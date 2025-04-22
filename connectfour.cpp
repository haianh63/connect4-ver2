#include <iostream>
#include <cmath>
#include <cstdint>

const int BOARD_ROW = 6;
const int BOARD_COL = 7;
const int INAROW = 4;
const int BOARD_SIZE = BOARD_ROW * BOARD_COL;

class ConnectFourAgent
{
private:
    int AI_PIECE;
    int HUMAN_PIECE;

    // Winning patterns (giữ nguyên)
    const uint64_t GAME_OVER[69] = {15, 2113665, 16843009, 2130440, 30, 4227330, 33686018, 4260880, 60, 8454660, 67372036, 8521760, 120, 16909320, 134744072, 17043520, 33818640, 67637280, 135274560, 1920, 270549120, 2155905152, 272696320, 3840, 541098240, 4311810304, 545392640, 7680, 1082196480, 8623620608, 1090785280, 15360, 2164392960, 17247241216, 2181570560, 4328785920, 8657571840, 17315143680, 245760, 34630287360, 275955859456, 34905128960, 491520, 69260574720, 551911718912, 69810257920, 983040, 138521149440, 1103823437824, 139620515840, 1966080, 277042298880, 2207646875648, 279241031680, 554084597760, 1108169195520, 2216338391040, 31457280, 62914560, 125829120, 251658240, 4026531840, 8053063680, 16106127360, 32212254720, 515396075520, 1030792151040, 2061584302080, 4123168604160};

    const uint64_t IS_LEGAL_MOVE_MASK = (1ULL << BOARD_COL) - 1;

    // Kiểm tra vị trí hợp lệ
    bool is_valid_position(int col, int row) {
        return col >= 0 && col < BOARD_COL && row >= 0 && row < BOARD_ROW;
    }

    // Kiểm tra ô có khả dụng để đặt quân
    bool is_square_available(const uint64_t bitboard[2], int col, int row) {
        if (!is_valid_position(col, row)) return false;
        int index = col + row * BOARD_COL;
        if ((bitboard[0] >> index) & 1) return false; // Ô đã chiếm
        // Trong Connect-4, quân chỉ đặt được ở ô trống thấp nhất
        if (row > 0 && !((bitboard[0] >> (col + (row-1) * BOARD_COL)) & 1)) return false;
        return true;
    }

    // Kiểm tra length quân nối theo hướng (dc, dr) cho người chơi piece
    bool check_connected(const uint64_t bitboard[2], int col, int row, int dc, int dr, int piece, int length) {
        for (int i = 0; i < length; ++i) {
            int c = col + i * dc, r = row + i * dr;
            if (!is_valid_position(c, r)) return false;
            int index = c + r * BOARD_COL;
            bool is_played = (bitboard[0] >> index) & 1;
            bool is_ai = (bitboard[1] >> index) & 1;
            if (!is_played || (piece == AI_PIECE && !is_ai) || (piece == HUMAN_PIECE && is_ai)) return false;
        }
        return true;
    }

    // Kiểm tra tính khả dụng của các ô liền kề
    std::pair<bool, bool> get_adjacent_availability(const uint64_t bitboard[2], int col, int row, int dc, int dr, int length) {
        int left_col = col - dc, left_row = row - dr;
        int right_col = col + length * dc, right_row = row + length * dr;
        bool left_available = is_square_available(bitboard, left_col, left_row);
        bool right_available = is_square_available(bitboard, right_col, right_row);
        return {left_available, right_available};
    }

    // Đếm số ô khả dụng theo hướng cho đến khi gặp ô không khả dụng
    int count_available_squares(const uint64_t bitboard[2], int col, int row, int dc, int dr) {
        int count = 0;
        int c = col + dc, r = row + dr;
        while (is_valid_position(c, r)) {
            if (!is_square_available(bitboard, c, r)) break;
            ++count;
            c += dc; r += dr;
        }
        return count;
    }

    // Kiểm tra trường hợp đặc biệt của Feature 2
    bool check_special_case_feature2(const uint64_t bitboard[2], int col, int row, int dc, int dr, int piece) {
        if (!check_connected(bitboard, col, row, dc, dr, piece, 2)) return false;
        int c = col + 2 * dc, r = row + 2 * dr;
        if (!is_valid_position(c, r)) return false;
        int index = c + r * BOARD_COL;
        bool is_played = (bitboard[0] >> index) & 1;
        bool is_ai = (bitboard[1] >> index) & 1;
        return is_played && ((piece == AI_PIECE && is_ai) || (piece == HUMAN_PIECE && !is_ai));
    }

    // Đánh giá Feature 1: Bốn quân nối
    double evaluate_feature1(const uint64_t bitboard[2], int col, int row, int piece) {
        int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}}; // Ngang, dọc, chéo, chéo ngược
        for (const auto& dir : directions) {
            int dc = dir[0], dr = dir[1];
            if (check_connected(bitboard, col, row, dc, dr, piece, 4)) {
                return (piece == AI_PIECE) ? INFINITY : -INFINITY;
            }
        }
        return 0;
    }

    // Đánh giá Feature 2: Ba quân nối
    double evaluate_feature2(const uint64_t bitboard[2], int col, int row, int piece) {
        int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
        double score = 0;
        for (const auto& dir : directions) {
            int dc = dir[0], dr = dir[1];
            if (check_connected(bitboard, col, row, dc, dr, piece, 3)) {
                auto [left_available, right_available] = get_adjacent_availability(bitboard, col, row, dc, dr, 3);
                if (left_available && right_available) {
                    score += (piece == AI_PIECE) ? INFINITY : -INFINITY;
                } else if (left_available || right_available) {
                    score += (piece == AI_PIECE) ? 900000 : -900000;
                }
            }
            if (check_special_case_feature2(bitboard, col, row, dc, dr, piece)) {
                score += (piece == AI_PIECE) ? 900000 : -900000;
            }
        }
        return score;
    }

    // Đánh giá Feature 3: Hai quân nối
    double evaluate_feature3(const uint64_t bitboard[2], int col, int row, int piece) {
        int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
        double score = 0;
        for (const auto& dir : directions) {
            int dc = dir[0], dr = dir[1];
            if (check_connected(bitboard, col, row, dc, dr, piece, 2)) {
                auto [left_available, right_available] = get_adjacent_availability(bitboard, col, row, dc, dr, 2);
                if (left_available && right_available) {
                    score += (piece == AI_PIECE) ? 50000 : -50000;
                } else if (left_available) {
                    int count = count_available_squares(bitboard, col, row, -dc, -dr);
                    score += (piece == AI_PIECE) ? count : -count;
                } else if (right_available) {
                    int count = count_available_squares(bitboard, col + 2 * dc, row + 2 * dr, dc, dr);
                    score += (piece == AI_PIECE) ? count : -count;
                }
            }
        }
        return score;
    }

    // Đánh giá Feature 4: Quân đơn
    double evaluate_feature4(int col, int piece) {
        if (col == 3) return (piece == AI_PIECE) ? 200 : -200; // Cột d
        if (col == 2 || col == 4) return (piece == AI_PIECE) ? 120 : -120; // Cột c, e
        if (col == 1 || col == 5) return (piece == AI_PIECE) ? 70 : -70; // Cột b, f
        if (col == 0 || col == 6) return (piece == AI_PIECE) ? 40 : -40; // Cột a, g
        return 0;
    }

    // Hàm heuristic mới thay thế evaluatingFunction
    double evaluatingFunction(const uint64_t bitboard[2]) {
        double score = 0;
        for (int row = 0; row < BOARD_ROW; ++row) {
            for (int col = 0; col < BOARD_COL; ++col) {
                int index = col + row * BOARD_COL;
                if ((bitboard[0] >> index) & 1) { // Ô đã chơi
                    int piece = (bitboard[1] >> index) & 1 ? AI_PIECE : HUMAN_PIECE;
                    // Feature 1
                    double feature1_score = evaluate_feature1(bitboard, col, row, piece);
                    if (std::abs(feature1_score) == INFINITY) return feature1_score;
                    score += feature1_score;
                    // Feature 2
                    score += evaluate_feature2(bitboard, col, row, piece);
                    // Feature 3
                    score += evaluate_feature3(bitboard, col, row, piece);
                    // Feature 4
                    score += evaluate_feature4(col, piece);
                }
            }
        }
        return score;
    }

    void list_to_bitboard(const int listboard[BOARD_SIZE], uint64_t bitboard[2])
    {
        bitboard[0] = 0; // played
        bitboard[1] = 0; // player
        for (int n = 0; n < BOARD_SIZE; n++)
        {
            if (listboard[n] != 0)
            {
                bitboard[0] |= (1ULL << n);
                if (listboard[n] == AI_PIECE)
                {
                    bitboard[1] |= (1ULL << n);
                }
            }
        }
    }

    void bitboard_to_numpy2d(const uint64_t bitboard[2], int8_t output[BOARD_ROW][BOARD_COL])
    {
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            int row = i / BOARD_COL;
            int col = i % BOARD_COL;
            output[row][col] = 0;
            bool is_played = (bitboard[0] >> i) & 1;
            if (is_played)
            {
                bool player = (bitboard[1] >> i) & 1;
                output[row][col] = player ? AI_PIECE : HUMAN_PIECE;
            }
        }
    }

    bool isRunOutMove(const uint64_t bitboard[2])
    {
        uint64_t mask_board = (1ULL << (BOARD_COL * BOARD_ROW)) - 1;
        return bitboard[0] == mask_board;
    }

    bool isWinning(const uint64_t bitboard[2], int piece)
    {
        for (int i = 0; i < 69; i++)
        {
            if (piece == AI_PIECE)
            {
                if ((bitboard[0] & bitboard[1] & GAME_OVER[i]) == GAME_OVER[i])
                    return true;
            }
            else
            {
                if ((bitboard[0] & ~bitboard[1] & GAME_OVER[i]) == GAME_OVER[i])
                    return true;
            }
        }
        return false;
    }

    bool is_legal_move(const uint64_t bitboard[2], int action)
    {
        uint64_t bits = bitboard[0] & IS_LEGAL_MOVE_MASK;
        return ((bits >> action) & 1) == 0;
    }

    void getValidPositions(const uint64_t bitboard[2], int validPositions[BOARD_COL], int &count)
    {
        count = 0;
        const int exploration_order[7] = {3, 2, 4, 1, 5, 0, 6};
        for (int i = 0; i < BOARD_COL; i++)
        {
            int col = exploration_order[i];
            if (is_legal_move(bitboard, col))
            {
                validPositions[count++] = col;
            }
        }
    }

    int get_next_index(const uint64_t bitboard[2], int action)
    {
        for (int row = BOARD_ROW - 1; row >= 0; row--)
        {
            int index = action + (row * BOARD_COL);
            if (((bitboard[0] >> index) & 1) == 0)
            {
                return index;
            }
        }
        return action;
    }

    void dropPiece(const uint64_t bitboard[2], int col, int piece, uint64_t output[2])
    {
        int index = get_next_index(bitboard, col);
        uint64_t mark = (piece == HUMAN_PIECE) ? 0 : 1;
        output[0] = bitboard[0] | (1ULL << index);
        output[1] = bitboard[1] | (mark << index);
    }

    void minimax(const uint64_t bitboard[2], int depth, double alpha, double beta,
                 bool maximizingPlayer, int &bestMove, double &bestScore)
    {
        if (depth == 0)
        {
            bestMove = 0;
            bestScore = evaluatingFunction(bitboard);
            return;
        }
        if (isWinning(bitboard, AI_PIECE))
        {
            bestMove = 0;
            bestScore = INFINITY;
            return;
        }
        if (isWinning(bitboard, HUMAN_PIECE))
        {
            bestMove = 0;
            bestScore = -INFINITY;
            return;
        }
        if (isRunOutMove(bitboard))
        {
            bestMove = 0;
            bestScore = 0;
            return;
        }

        int validPositions[BOARD_COL];
        int count;
        getValidPositions(bitboard, validPositions, count);

        if (maximizingPlayer)
        {
            double maxEval = -INFINITY;
            bestMove = validPositions[0];
            for (int i = 0; i < count; i++)
            {
                int col = validPositions[i];
                uint64_t new_bitboard[2];
                dropPiece(bitboard, col, AI_PIECE, new_bitboard);
                int nextMove;
                double eval;
                minimax(new_bitboard, depth - 1, alpha, beta, false, nextMove, eval);
                if (eval > maxEval)
                {
                    maxEval = eval;
                    bestMove = col;
                }
                alpha = (alpha > maxEval) ? alpha : maxEval;
                if (beta <= alpha)
                    break;
            }
            bestScore = maxEval;
        }
        else
        {
            double minEval = INFINITY;
            bestMove = validPositions[0];
            for (int i = 0; i < count; i++)
            {
                int col = validPositions[i];
                uint64_t new_bitboard[2];
                dropPiece(bitboard, col, HUMAN_PIECE, new_bitboard);
                int nextMove;
                double eval;
                minimax(new_bitboard, depth - 1, alpha, beta, true, nextMove, eval);
                if (eval < minEval)
                {
                    minEval = eval;
                    bestMove = col;
                }
                beta = (beta < minEval) ? beta : minEval;
                if (beta <= alpha)
                    break;
            }
            bestScore = minEval;
        }
    }

public:
    ConnectFourAgent(int mark)
    {
        AI_PIECE = mark;
        HUMAN_PIECE = (AI_PIECE == 1) ? 2 : 1;
    }

    int my_agent(const int board[BOARD_SIZE])
    {
        uint64_t bitboard[2];
        list_to_bitboard(board, bitboard);
        int bestMove;
        double bestScore;
        minimax(bitboard, 9, -INFINITY, INFINITY, true, bestMove, bestScore);
        return bestMove;
    }
};

extern "C"
{
    int call_connect_four_agent(int *board, int size, int mark)
    {
        if (size != BOARD_SIZE)
        {
            return -1; // Error: invalid board size
        }
        ConnectFourAgent agent(mark);
        return agent.my_agent(board);
    }
}
