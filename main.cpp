#include <xxhash.h>
#include <array>
#include <functional>
#include <cstdio>
#include <vector>
#include <optional>
#include <ctime>
#include <unordered_map>

struct board_tile_t {
	int8_t owner;
	int8_t size;
};

struct game_state_t {
	board_tile_t board[9];
	int remaining_moves[2][3];
	bool player_yielded[2];

	bool operator==(const game_state_t &other) const {
		return memcmp((int8_t *) this, (int8_t *) &other, sizeof(game_state_t)) == 0;
	}
};

namespace std {
	template<>
	struct hash<game_state_t> {
		std::size_t operator()(const game_state_t &k) const {
			int8_t *data = (int8_t *) &k;

			XXH64_hash_t hash = XXH64(data, sizeof(game_state_t), 1);
			return hash;
		}
	};
}


struct move_t {
	int player;
	int size;
	int position;
	bool yield;
};


bool is_valid_move(game_state_t const &state, move_t const &move) {
	if (state.remaining_moves[move.player][move.size] > 0) {
		if (move.position >= 0 && move.position < 9 && move.size >= 0 && move.size < 3) {
			if (state.board[move.position].owner != move.player && state.board[move.position].size < move.size) {
				return true;
			}
		}
	}

	return false;
}

std::optional<game_state_t> perform_move(game_state_t const &state, move_t const &move) {
	if (move.yield) {
		game_state_t out_state = state;
		out_state.player_yielded[move.player] = true;
		return out_state;
	}
	if (is_valid_move(state, move)) {
		game_state_t out_state = state;
		out_state.board[move.position].owner = move.player;
		out_state.board[move.position].size = move.size;
		out_state.remaining_moves[move.player][move.size]--;
		return out_state;
	}

	return std::nullopt;
}

int get_victor(game_state_t const &state) {
	int victory_conditions[][3] = {
			{0, 1, 2},
			{3, 4, 5},
			{6, 7, 8},

			{0, 3, 6},
			{1, 4, 7},
			{2, 5, 8},

			{0, 4, 8},
			{6, 4, 2},
	};

	bool out_of_moves = true;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (state.remaining_moves[i][j] > 0) {
				out_of_moves = false;
			}
		}
	}
	if (out_of_moves) {
		return 2;
	}

	for (int i = 0; i < 2; ++i) {
		if (state.player_yielded[i]) {
			return 1 - i;
		}
	}

	for (auto condition: victory_conditions) {
		int owner = -1;
		bool victory = true;

		owner = state.board[condition[0]].owner;
		if (owner == -1) continue;
		for (int i = 0; i < 3; ++i) {
			if (state.board[condition[i]].owner != owner) {
				victory = false;
				break;
			}
		}

		if (victory) {
			return owner;
		}
	}

	return -1;
}

game_state_t get_clear_game_state() {
	game_state_t state;
	for (int i = 0; i < 9; ++i) {
		state.board[i].owner = -1;
		state.board[i].size = -1;
	}

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 3; ++j) {
			state.remaining_moves[i][j] = 2;
		}
	}

	state.player_yielded[0] = false;
	state.player_yielded[1] = false;

	return state;
}

std::vector<move_t> get_valid_moves(game_state_t const &state, int player) {
	std::vector<move_t> moves;
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 3; ++j) {
			move_t move;
			move.yield = false;
			move.player = player;
			move.position = i;
			move.size = j;
			if (is_valid_move(state, move)) {
				moves.emplace_back(move);
			}
		}
	}

	return moves;
}

move_t get_move_player(game_state_t const &state, int player) {
	printf("Enter move in format X Y Z (X, Y, and Size, in numbers from 1 to 3\n");

	char x_str[256], y_str[256], z_str[256];
	scanf("%s %s %s", x_str, y_str, z_str);
	int x = atoi(x_str) - 1;
	int y = atoi(y_str) - 1;
	int z = atoi(z_str) - 1;

	int pos = y * 3 + x;

	return {player, z, pos, false};
}

move_t get_move_random_ai(game_state_t const &state, int player) {
	auto moves = get_valid_moves(state, player);
//    printf("Valid moves: %i\n", (int) moves.size());
	if (moves.empty()) {
		return {player, -1, -1, true};
	}
	return moves[rand() % moves.size()];
}

void play_game(std::array<std::function<move_t(game_state_t const &, int)>, 2> brains) {
	char size_array[] = {'.', 'x', 'X'};
	game_state_t state = get_clear_game_state();
	bool running = true;
	while (running) {
		for (int player = 0; player < brains.size(); ++player) {
			printf("-----------------\n");
			printf("Player 1: ");
			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < state.remaining_moves[0][i]; ++j) {
					printf("%c", size_array[i]);
				}
			}
			printf("\nPlayer 2: ");
			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < state.remaining_moves[1][i]; ++j) {
					printf("%c", size_array[i]);
				}
			}
			printf("\n");
			printf("\n");

			for (int y = 0; y < 3; ++y) {
				for (int x = 0; x < 3; ++x) {
					printf("%c|%c\t", state.board[y * 3 + x].owner + '1', state.board[y * 3 + x].size + '1');
				}
				printf("\n");
			}

			int winner = get_victor(state);
			if (winner != -1) {
				if (winner <= 1) {
					printf("Player %i won!\n", winner + 1);
				} else {
					printf("Draw!\n");
				}
				running = false;
				break;
			}

			printf("-----------------\n\n\n");


			std::optional<game_state_t> result;
			do {
				move_t move = brains[player](state, player);
				result = perform_move(state, move);
			} while (!result);
			state = *result;
		}
	}
}

int get_score_for_moves_left(game_state_t const &state, int player) {
	int total = 0;
	for (int i = 0; i < 3; ++i) {
		total += state.remaining_moves[player][i] * (i * 5);
	}

	return total;
}

int get_score(game_state_t const &state, int player) {
	int victory = get_victor(state);
	if (victory == -1) {
		return get_score_for_moves_left(state, player);
	}
	if (victory == player)
		return INT32_MAX;
	if (victory == 1 - player)
		return INT32_MIN;

	return 0;
}

std::unordered_map<game_state_t, std::pair<move_t, int>> cache;

std::pair<move_t, int> maximize(game_state_t const &state, int player, int depth_left) {
	auto ret = cache.find(state);
	if ( ret != cache.end()) {
		return ret->second;
	}

	auto moves = get_valid_moves(state, player);
	std::pair<move_t, int> best_result = {{}, INT32_MIN};
	for (auto const &move: moves) {
		auto new_state = perform_move(state, move);

		int state_score = get_score(*new_state, player);
		if (state_score == INT32_MAX) {
			return {move, state_score};
		}

		if (state_score == -INT32_MIN) {
			return {move, state_score};
		}

		if (depth_left <= 0) {
			return {move, state_score};
		}


		auto res = maximize(*new_state, 1 - player, depth_left - 1);
		int others_score = -res.second / 2;
		if (others_score > best_result.second) {
			best_result.first = move;
			best_result.second = others_score;
		}
	}

	cache.insert_or_assign(state, best_result);
	return best_result;
}

move_t smarter_ai(game_state_t const &state, int player) {
	cache.clear();
	return maximize(state, player, 7).first;
}

int main() {
	srand((unsigned int) std::time(nullptr));

	play_game({smarter_ai, get_move_player});
	return 0;
}






