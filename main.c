#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define m_set_bit(word, index) ((word) |= (1ULL << (U64)(index)))
#define m_get_bit(word, index) ((word) & (1ULL << (U64)(index)))
#define m_pop_bit(word, index) ((word) &= ~(1ULL << (U64)(index)))
#define m_zero_least(word) ((word) &= (word - 1ULL))
#define m_encode_move(source, dest) (U16)(source) | (U16)((dest) << 6U)
#define m_decode_move(source, dest, move) source = (int)((move) & 0x3fU); dest = (int)(((move) >> 6U) & 0x3fU)
#define U64 unsigned long long
#define U16 unsigned short

typedef struct{
    U64 bits[12];
    int side;
} Position;
typedef struct{
    U16 moves[100];
    int count;
} Move_tab;

const char *index_to_coord[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};
enum
{
    white, black,
};
enum
{
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
};
enum
{
    P,
    N,
    B,
    R,
    Q,
    K,
    p,
    n,
    b,
    r,
    q,
    k,
    empty,
};
char* unicode_pieces[12] = {"♟︎ ", "♞ ", "♝ ", "♜ ", "♛ ", "♚ ", "♙ ", "♘ ", "♗ ", "♖ ", "♕ ", "♔ "};
char* unicode_numbers[8] = {"１","２","３","４","５","６","７","８"};
char* unicode_letters[8] = {"Ａ","Ｂ","Ｃ","Ｄ","Ｅ","Ｆ","Ｇ","Ｈ"};

int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k,
};
//evals
const int piece_val[12] = 
{
    1, 3, 3, 5, 9, 200, 1, 3, 3, 5, 9, 200,
};
const int piece_square_eval[12][64] = 
{
    //P
    {
        10,10,10,10,10,10,10,10,
        20,20,20,20,20,20,20,20,
        15,15,12,12,12,12,15,15,
        10,10,10,12,12,10,10,10,
        10,10,13,13,13,10,10,10,
        10,12,11, 8, 8,10,10,11,
        10,10,10, 6, 6,12,12,10,
        10,10,10,10,10,10,10,10,
    },
    //N
    {
        6, 7, 9, 9, 9, 9, 7, 6,
        6, 8,10,10,10,10, 8, 8,
        6,10,11,11,11,11,10,10,
        6,10,12,12,12,12,10,10,
        6,10,10,11,11,10,10, 7,
        6, 8,11, 9, 9,11, 8, 6,
        6, 7, 7,10,10, 7, 7, 6,
        4, 8, 6, 6, 6, 6, 8, 4,
    },
    //B
    {
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,12,10,10,10,10,12,10,
        12,10,12,10,10,12,10,12,
        10,10,10,10,10,10,11,10,
         9,13,10,11,11,10,13, 9,
        10,10,10,10,10,10,10,10,
    },
    //R
    {
        12,12,12,12,12,12,12,12,
        12,12,12,12,12,12,12,12,
        11,11,11,11,11,11,11,11,
         9, 9, 9, 9, 9, 9, 9, 9,
         9, 9, 9, 9, 9 ,9 ,9, 9,
         9, 9, 9, 9, 9, 9, 9, 9,
        10,10,10,10,10, 9, 9, 9,
        10,10,11,11,11,11, 9, 9,
    },
    //Q
    {
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,11,10,10,
        10,11,10,10,10,10,10,10,
        10,10,12,10,10,10,10,10,
         9, 9,10,10,10,10, 9, 9,
    },
    //K
    {
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        12,12,10,10,10,10,12,12,
    },
    //p
    {
        10,10,10,10,10,10,10,10,
        10,10,10, 6, 6,12,12,10,
        10,12,11, 8, 8,10,10,11,
        10,10,13,13,13,10,10,10,
        10,10,10,12,12,10,10,10,
        15,15,12,12,12,12,15,15,
        20,20,20,20,20,20,20,20,
        10,10,10,10,10,10,10,10,
    },
    //n
    {

         4, 8, 6, 6, 6, 6, 8, 4,
         6, 7, 7,10,10, 7, 7, 6,
         6, 8,11, 9, 9,11, 8, 6,
         6,10,10,11,11,10,10, 7,
         6,10,12,12,12,12,10,10,
         6,10,11,11,11,11,10,10,
         6, 8,10,10,10,10, 8, 8,
         6, 7, 9, 9, 9, 9, 7, 6,
    },
    //b
    {

        10,10,10,10,10,10,10,10,
         9,13,10,11,11,10,13, 9,
        10,10,10,10,10,10,11,10,
        12,10,12,10,10,12,10,12,
        10,12,10,10,10,10,12,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
    },
    //r
    {
        10,10,11,11,11,11, 9, 9,
        10,10,10,10,10, 9, 9, 9,
         9, 9, 9, 9, 9, 9, 9, 9,
         9, 9, 9, 9, 9 ,9 ,9, 9,
         9, 9, 9, 9, 9, 9, 9, 9,
        11,11,11,11,11,11,11,11,
        12,12,12,12,12,12,12,12,
        12,12,12,12,12,12,12,12,
    },
    //q
    {
         9, 9,10,10,10,10, 9, 9,
        10,10,12,10,10,10,10,10,
        10,11,10,10,10,10,10,10,
        10,10,10,10,10,11,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
    },
    //k
    {
        12,12,10,10,10,10,12,12,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,
    },
};
//attack tables
const U64 pawn_attacks[2][64] =
{
    {
        0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 
        0x2, 0x5, 0xa, 0x14, 
        0x28, 0x50, 0xa0, 0x40, 
        0x200, 0x500, 0xa00, 0x1400, 
        0x2800, 0x5000, 0xa000, 0x4000, 
        0x20000, 0x50000, 0xa0000, 0x140000, 
        0x280000, 0x500000, 0xa00000, 0x400000, 
        0x2000000, 0x5000000, 0xa000000, 0x14000000, 
        0x28000000, 0x50000000, 0xa0000000, 0x40000000, 
        0x200000000, 0x500000000, 0xa00000000, 0x1400000000, 
        0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000, 
        0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000, 
        0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000, 
        0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000, 
        0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
    },
    { 
        0x200, 0x500, 0xa00, 0x1400, 
        0x2800, 0x5000, 0xa000, 0x4000, 
        0x20000, 0x50000, 0xa0000, 0x140000, 
        0x280000, 0x500000, 0xa00000, 0x400000, 
        0x2000000, 0x5000000, 0xa000000, 0x14000000, 
        0x28000000, 0x50000000, 0xa0000000, 0x40000000, 
        0x200000000, 0x500000000, 0xa00000000, 0x1400000000, 
        0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000, 
        0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000, 
        0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000, 
        0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000, 
        0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000, 
        0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000, 
        0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000, 
        0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0,
    },
};
const U64 knight_attacks[64] = 
{
    0x20400, 0x50800, 0xa1100, 0x142200, 
    0x284400, 0x508800, 0xa01000, 0x402000, 
    0x2040004, 0x5080008, 0xa110011, 0x14220022, 
    0x28440044, 0x50880088, 0xa0100010, 0x40200020, 
    0x204000402, 0x508000805, 0xa1100110a, 0x1422002214, 
    0x2844004428, 0x5088008850, 0xa0100010a0, 0x4020002040, 
    0x20400040200, 0x50800080500, 0xa1100110a00, 0x142200221400, 
    0x284400442800, 0x508800885000, 0xa0100010a000, 0x402000204000, 
    0x2040004020000, 0x5080008050000, 0xa1100110a0000, 0x14220022140000, 
    0x28440044280000, 0x50880088500000, 0xa0100010a00000, 0x40200020400000, 
    0x204000402000000, 0x508000805000000, 0xa1100110a000000, 0x1422002214000000, 
    0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000, 
    0x400040200000000, 0x800080500000000, 0x1100110a00000000, 0x2200221400000000, 
    0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000, 
    0x4020000000000, 0x8050000000000, 0x110a0000000000, 0x22140000000000, 
    0x44280000000000, 0x88500000000000, 0x10a00000000000, 0x20400000000000,
};
const U64 king_attacks[64] = 
{
    0x302, 0x705, 0xe0a, 0x1c14, 
    0x3828, 0x7050, 0xe0a0, 0xc040, 
    0x30203, 0x70507, 0xe0a0e, 0x1c141c, 
    0x382838, 0x705070, 0xe0a0e0, 0xc040c0, 
    0x3020300, 0x7050700, 0xe0a0e00, 0x1c141c00, 
    0x38283800, 0x70507000, 0xe0a0e000, 0xc040c000, 
    0x302030000, 0x705070000, 0xe0a0e0000, 0x1c141c0000, 
    0x3828380000, 0x7050700000, 0xe0a0e00000, 0xc040c00000, 
    0x30203000000, 0x70507000000, 0xe0a0e000000, 0x1c141c000000, 
    0x382838000000, 0x705070000000, 0xe0a0e0000000, 0xc040c0000000, 
    0x3020300000000, 0x7050700000000, 0xe0a0e00000000, 0x1c141c00000000, 
    0x38283800000000, 0x70507000000000, 0xe0a0e000000000, 0xc040c000000000, 
    0x302030000000000, 0x705070000000000, 0xe0a0e0000000000, 0x1c141c0000000000, 
    0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000, 
    0x203000000000000, 0x507000000000000, 0xa0e000000000000, 0x141c000000000000, 
    0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000,
};
const U64 rook_possible_attacks[64] = 
{
    0x1010101010101fe, 0x2020202020202fd, 0x4040404040404fb, 0x8080808080808f7, 0x10101010101010ef, 0x20202020202020df, 0x40404040404040bf, 0x808080808080807f, 
    0x10101010101fe01, 0x20202020202fd02, 0x40404040404fb04, 0x80808080808f708, 0x101010101010ef10, 0x202020202020df20, 0x404040404040bf40, 0x8080808080807f80, 
    0x101010101fe0101, 0x202020202fd0202, 0x404040404fb0404, 0x808080808f70808, 0x1010101010ef1010, 0x2020202020df2020, 0x4040404040bf4040, 0x80808080807f8080, 
    0x1010101fe010101, 0x2020202fd020202, 0x4040404fb040404, 0x8080808f7080808, 0x10101010ef101010, 0x20202020df202020, 0x40404040bf404040, 0x808080807f808080, 
    0x10101fe01010101, 0x20202fd02020202, 0x40404fb04040404, 0x80808f708080808, 0x101010ef10101010, 0x202020df20202020, 0x404040bf40404040, 0x8080807f80808080, 
    0x101fe0101010101, 0x202fd0202020202, 0x404fb0404040404, 0x808f70808080808, 0x1010ef1010101010, 0x2020df2020202020, 0x4040bf4040404040, 0x80807f8080808080, 
    0x1fe010101010101, 0x2fd020202020202, 0x4fb040404040404, 0x8f7080808080808, 0x10ef101010101010, 0x20df202020202020, 0x40bf404040404040, 0x807f808080808080, 
    0xfe01010101010101, 0xfd02020202020202, 0xfb04040404040404, 0xf708080808080808, 0xef10101010101010, 0xdf20202020202020, 0xbf40404040404040, 0x7f80808080808080,
};
const U64 bishop_possible_attacks[64] = 
{
    0x8040201008040200, 0x80402010080500, 0x804020110a00, 0x8041221400, 0x182442800, 0x10204885000, 0x102040810a000, 0x102040810204000, 
    0x4020100804020002, 0x8040201008050005, 0x804020110a000a, 0x804122140014, 0x18244280028, 0x1020488500050, 0x102040810a000a0, 0x204081020400040, 
    0x2010080402000204, 0x4020100805000508, 0x804020110a000a11, 0x80412214001422, 0x1824428002844, 0x102048850005088, 0x2040810a000a010, 0x408102040004020, 
    0x1008040200020408, 0x2010080500050810, 0x4020110a000a1120, 0x8041221400142241, 0x182442800284482, 0x204885000508804, 0x40810a000a01008, 0x810204000402010, 
    0x804020002040810, 0x1008050005081020, 0x20110a000a112040, 0x4122140014224180, 0x8244280028448201, 0x488500050880402, 0x810a000a0100804, 0x1020400040201008, 
    0x402000204081020, 0x805000508102040, 0x110a000a11204080, 0x2214001422418000, 0x4428002844820100, 0x8850005088040201, 0x10a000a010080402, 0x2040004020100804, 
    0x200020408102040, 0x500050810204080, 0xa000a1120408000, 0x1400142241800000, 0x2800284482010000, 0x5000508804020100, 0xa000a01008040201, 0x4000402010080402, 
    0x2040810204080, 0x5081020408000, 0xa112040800000, 0x14224180000000, 0x28448201000000, 0x50880402010000, 0xa0100804020100, 0x40201008040201, 
};
const U64 rook_occupancy[64] = 
{
    0x101010101017e, 0x202020202027c, 0x404040404047a, 0x8080808080876, 0x1010101010106e, 0x2020202020205e, 0x4040404040403e, 0x8080808080807e, 
    0x1010101017e00, 0x2020202027c00, 0x4040404047a00, 0x8080808087600, 0x10101010106e00, 0x20202020205e00, 0x40404040403e00, 0x80808080807e00, 
    0x10101017e0100, 0x20202027c0200, 0x40404047a0400, 0x8080808760800, 0x101010106e1000, 0x202020205e2000, 0x404040403e4000, 0x808080807e8000, 
    0x101017e010100, 0x202027c020200, 0x404047a040400, 0x8080876080800, 0x1010106e101000, 0x2020205e202000, 0x4040403e404000, 0x8080807e808000, 
    0x1017e01010100, 0x2027c02020200, 0x4047a04040400, 0x8087608080800, 0x10106e10101000, 0x20205e20202000, 0x40403e40404000, 0x80807e80808000, 
    0x17e0101010100, 0x27c0202020200, 0x47a0404040400, 0x8760808080800, 0x106e1010101000, 0x205e2020202000, 0x403e4040404000, 0x807e8080808000, 
    0x7e010101010100, 0x7c020202020200, 0x7a040404040400, 0x76080808080800, 0x6e101010101000, 0x5e202020202000, 0x3e404040404000, 0x7e808080808000, 
    0x7e01010101010100, 0x7c02020202020200, 0x7a04040404040400, 0x7608080808080800, 0x6e10101010101000, 0x5e20202020202000, 0x3e40404040404000, 0x7e80808080808000, 

};
const U64 bishop_occupancy[64] = 
{
    0x40201008040200, 0x402010080400, 0x4020100a00, 0x40221400, 0x2442800, 0x204085000, 0x20408102000, 0x2040810204000, 
    0x20100804020000, 0x40201008040000, 0x4020100a0000, 0x4022140000, 0x244280000, 0x20408500000, 0x2040810200000, 0x4081020400000, 
    0x10080402000200, 0x20100804000400, 0x4020100a000a00, 0x402214001400, 0x24428002800, 0x2040850005000, 0x4081020002000, 0x8102040004000, 
    0x8040200020400, 0x10080400040800, 0x20100a000a1000, 0x40221400142200, 0x2442800284400, 0x4085000500800, 0x8102000201000, 0x10204000402000, 
    0x4020002040800, 0x8040004081000, 0x100a000a102000, 0x22140014224000, 0x44280028440200, 0x8500050080400, 0x10200020100800, 0x20400040201000, 
    0x2000204081000, 0x4000408102000, 0xa000a10204000, 0x14001422400000, 0x28002844020000, 0x50005008040200, 0x20002010080400, 0x40004020100800, 
    0x20408102000, 0x40810204000, 0xa1020400000, 0x142240000000, 0x284402000000, 0x500804020000, 0x201008040200, 0x402010080400, 
    0x2040810204000, 0x4081020400000, 0xa102040000000, 0x14224000000000, 0x28440200000000, 0x50080402000000, 0x20100804020000, 0x40201008040200,
};
static inline U64 get_color_bitposition(Position* position, int color){
    U64 temp = 0ULL;
    for(int i = 0; i < 6; i++) temp |= position->bits[i + color * 6];
    return temp;
}
static inline U64 get_sum_bitposition(Position* position){
    U64 temp = 0ULL;
    for(int i = 0; i < 12; i++) temp |= position->bits[i];
    return temp;
}
static inline int get_population_count(U64 bitposition)
{
    int count = 0;
    while (bitposition){
        count++;
        bitposition &= bitposition - 1;
    }
    return count;
}

static inline int get_least_bit_index(U64 bitposition)
{
    if (bitposition) return get_population_count((bitposition & -bitposition) - 1);
    return -1;
}

void print_bitposition(U64 word){
    for(int i = 0; i < 8; i++){
        printf("\n\t");
        for(int j = 0; j < 8; j++){
            printf("%s", m_get_bit(word, i * 8 + j) ? "x " : ". ");
        }
    }
    printf("\n0x%llx\n", word);
}
void print_position(Position* pos){
    for (int i = 0; i < 8; i++)
    {
        printf("\n%s", unicode_numbers[7 - i]);
        for (int j = 0; j < 8; j++)
        {
            if(m_get_bit(get_sum_bitposition(pos), i*8 + j)){
                for(int piece = P; piece <= k; piece++)
                {
                    printf("%s", m_get_bit(pos->bits[piece], i*8+j) ? unicode_pieces[piece] : "");
                }
            }
            else printf("  ");
        }
    }
    printf("\n  ");
    for (int i = 0; i < 8; i++)
    {
        printf("%s", unicode_letters[i]);
    }
    printf("\nSide to move: %s", pos->side == white ? "white" : "black");
}
static inline U64 generate_bishop_attacks(int square, U64 blocker){
    U64 temp = 0;
    int i = square / 8;
    int j = square & 0x7;
    int rank = i;
    int file = j;
    while(rank < 7 && file < 7){
        m_set_bit(temp, (++rank)*8+(++file));
        if(m_get_bit(blocker, rank*8+file)) break;
    }
    rank = i;
    file = j;
    while(rank > 0 && file < 7){
        m_set_bit(temp, (--rank)*8+(++file));
        if(m_get_bit(blocker, rank*8+file)) break;
    } 
    rank = i;
    file = j;
    while(rank > 0 && file > 0){
        m_set_bit(temp, (--rank)*8+(--file));
        if(m_get_bit(blocker, rank*8+file)) break;
    }
    rank = i;
    file = j;
    while(rank < 7 && file > 0){
        m_set_bit(temp, (++rank)*8+(--file));
        if(m_get_bit(blocker, rank*8+file)) break;
    } 
    return temp;
}
static inline U64 generate_rook_attacks(int square, U64 blocker){
    U64 temp = 0;
    int i = square / 8;
    int j = square & 0x7;
    int rank = i;
    int file = j;
    while(rank < 7){
        m_set_bit(temp, (++rank)*8+(file));
        if(m_get_bit(blocker, rank*8+file)) break;
    }
    rank = i;
    file = j;
    while(rank > 0){
        m_set_bit(temp, (--rank)*8+(file));
        if(m_get_bit(blocker, rank*8+file)) break;
    } 
    rank = i;
    file = j;
    while(file > 0){
        m_set_bit(temp, (rank)*8+(--file));
        if(m_get_bit(blocker, rank*8+file)) break;
    }
    rank = i;
    file = j;
    while(file < 7){
        m_set_bit(temp, (rank)*8+(++file));
        if(m_get_bit(blocker, rank*8+file)) break;
    } 
    return temp;
}
static inline U64 generate_pawn_moves(int square, U64 blocker, int color){
    U64 temp = 0ULL;
    if(color == white){
        if(square / 8 > 0 && !m_get_bit(blocker, square - 8)){
            m_set_bit(temp, square - 8);
            if(square / 8 == 6 && !m_get_bit(blocker, square - 16)){
                m_set_bit(temp, square - 16);
            }
        }
    }
    else{
        if(square / 8 < 7 && !m_get_bit(blocker, square + 8)){
            m_set_bit(temp, square + 8);
            if(square / 8 == 1 && !m_get_bit(blocker, square + 16)){
                m_set_bit(temp, square + 16);
            }
        }
    }
    return temp;
}
void set_standard_position(Position* pos){
    pos->bits[p] = 0x000000000000ff00ULL;
    pos->bits[P] = 0x00ff000000000000ULL;
    pos->bits[k] = 0x0000000000000010ULL;
    pos->bits[K] = 0x1000000000000000ULL;
    pos->bits[q] = 0x0000000000000008ULL;
    pos->bits[Q] = 0x0800000000000000ULL;
    pos->bits[r] = 0x0000000000000081ULL;
    pos->bits[R] = 0x8100000000000000ULL;
    pos->bits[b] = 0x0000000000000024ULL;
    pos->bits[B] = 0x2400000000000000ULL;
    pos->bits[n] = 0x0000000000000042ULL;
    pos->bits[N] = 0x4200000000000000ULL;
    pos->side = white;
}
Move_tab move_generator(Position* position){
    Move_tab tab;
    tab.count = 0;
    int ally = position->side == white ? white : black;
    int enemy = position->side == white ? black : white;
    U64 dest_candidate;
    int candidate_count, source, dest;

    //knights
    U64 knights = position->bits[ally * 6 + N];
    int knight_count = get_population_count(knights);
    for(int i = 0; i < knight_count; i++){
        source = get_least_bit_index(knights);
        m_zero_least(knights);
        dest_candidate = knight_attacks[source];
        candidate_count = get_population_count(dest_candidate);
        for(int j = 0; j < candidate_count; j++){
            dest = get_least_bit_index(dest_candidate);
            m_zero_least(dest_candidate);
            if(m_get_bit(get_color_bitposition(position, ally), dest) == 0ULL){
                tab.moves[tab.count++] = m_encode_move(source, dest);
            }
        }
    }
    //pawns
    U64 pawns = position->bits[ally * 6 + P];
    int pawn_count = get_population_count(pawns);
    for(int i = 0; i < pawn_count; i++){
        source = get_least_bit_index(pawns);
        m_zero_least(pawns);
        //attacks
        dest_candidate = pawn_attacks[ally][source];
        candidate_count = get_population_count(dest_candidate);
        for(int j = 0; j < candidate_count; j++){
            dest = get_least_bit_index(dest_candidate);
            m_zero_least(dest_candidate);
            if(m_get_bit(get_color_bitposition(position, enemy), dest)){
                tab.moves[tab.count++] = m_encode_move(source, dest);
            }
        }
        //moves
        dest_candidate = generate_pawn_moves(source, get_sum_bitposition(position), ally);
        candidate_count = get_population_count(dest_candidate);
        for(int j = 0; j < candidate_count; j++){
            dest = get_least_bit_index(dest_candidate);
            m_zero_least(dest_candidate);
            tab.moves[tab.count++] = m_encode_move(source, dest);
        }
    }
    //kings
    U64 kings = position->bits[ally * 6 + K];
    int king_count = get_population_count(kings);
    for(int i = 0; i < king_count; i++){
        source = get_least_bit_index(kings);
        m_zero_least(kings);
        dest_candidate = king_attacks[source];
        candidate_count = get_population_count(dest_candidate);
        for(int j = 0; j < candidate_count; j++){
            dest = get_least_bit_index(dest_candidate);
            m_zero_least(dest_candidate);
            if(m_get_bit(get_color_bitposition(position, ally), dest) == 0ULL){
                tab.moves[tab.count++] = m_encode_move(source, dest);
            }
        }
    }
    //bishops
    U64 bishops = position->bits[ally * 6 + B];
    int bishop_count = get_population_count(bishops);
    for(int i = 0; i < bishop_count; i++){
        source = get_least_bit_index(bishops);
        m_zero_least(bishops);
        dest_candidate = generate_bishop_attacks(source, get_sum_bitposition(position));
        candidate_count = get_population_count(dest_candidate);
        for(int j = 0; j < candidate_count; j++){
            dest = get_least_bit_index(dest_candidate);
            m_zero_least(dest_candidate);
            if(m_get_bit(get_color_bitposition(position, ally), dest) == 0ULL){
                tab.moves[tab.count++] = m_encode_move(source, dest);
            }
        }
    }
    //rooks
    U64 rooks = position->bits[ally * 6 + R];
    int rook_count = get_population_count(rooks);
    for(int i = 0; i < rook_count; i++){
        source = get_least_bit_index(rooks);
        m_zero_least(rooks);
        dest_candidate = generate_rook_attacks(source, get_sum_bitposition(position));
        candidate_count = get_population_count(dest_candidate);
        for(int j = 0; j < candidate_count; j++){
            dest = get_least_bit_index(dest_candidate);
            m_zero_least(dest_candidate);
            if(m_get_bit(get_color_bitposition(position, ally), dest) == 0ULL){
                tab.moves[tab.count++] = m_encode_move(source, dest);
            }
        }
    }
    //queens
    U64 queens = position->bits[ally * 6 + Q];
    int queen_count = get_population_count(queens);
    for(int i = 0; i < queen_count; i++){
        source = get_least_bit_index(queens);
        m_zero_least(queens);
        dest_candidate = generate_rook_attacks(source, get_sum_bitposition(position)) | generate_bishop_attacks(source, get_sum_bitposition(position));
        candidate_count = get_population_count(dest_candidate);
        for(int j = 0; j < candidate_count; j++){
            dest = get_least_bit_index(dest_candidate);
            m_zero_least(dest_candidate);
            if(m_get_bit(get_color_bitposition(position, ally), dest) == 0ULL){
                tab.moves[tab.count++] = m_encode_move(source, dest);
            }
        }
    }
    return tab;
}
static inline int evaluate_position(Position* position){
    int score = 0;
    int ally = position->side == white ? white : black;
    int enemy = position->side == white ? black : white;
    score += 200*(2 * get_population_count(position->bits[ally * 6 + K]) - get_population_count(position->bits[enemy * 6 + K]));
    score += 9  *(get_population_count(position->bits[ally * 6 + Q]) - get_population_count(position->bits[enemy * 6 + Q]));
    score += 5  *(get_population_count(position->bits[ally * 6 + R]) - get_population_count(position->bits[enemy * 6 + R]));
    score += 4  *(get_population_count(position->bits[ally * 6 + B]) - get_population_count(position->bits[enemy * 6 + B]));
    score += 3  *(get_population_count(position->bits[ally * 6 + N]) - get_population_count(position->bits[enemy * 6 + N]));
    score += 1  *(get_population_count(position->bits[ally * 6 + P]) - get_population_count(position->bits[enemy * 6 + P]));
    return score;
}
static inline int evaluate_position2(Position* position){
    int score = 0;
    int ally = position->side == white ? white : black;
    int enemy = position->side == white ? black : white;
    int piece_count;
    U64 pieces;
    if(get_population_count(position->bits[ally * 6 + K] == 0ULL)) score -= 100000;
    for(int i = P; i <= K; i++){
        pieces = position->bits[ally * 6 + i];
        piece_count = get_population_count(position->bits[ally * 6 + i]);
        for(int j = 0; j < piece_count; j++){
            int index = get_least_bit_index(pieces);
            m_zero_least(pieces);
            score += piece_val[ally * 6 + i] * 100 + piece_square_eval[ally * 6 + i][index];
        }
        pieces = position->bits[enemy * 6 + i];
        piece_count = get_population_count(position->bits[enemy * 6 + i]);
        for(int j = 0; j < piece_count; j++){
            int index = get_least_bit_index(pieces);
            m_zero_least(pieces);
            score += -piece_val[enemy * 6 + i] * 100 - piece_square_eval[enemy * 6 + i][index];
        }
    }
    return score;
}
void make_move(Position* position, U16 move){
    int source;
    int dest;
    m_decode_move(source, dest, move);
    for(int i = P; i <= k; i++){
        m_pop_bit(position->bits[i], dest);
    }
    for(int i = P; i <= k; i++){
        if(m_get_bit(position->bits[i], source)){
            m_pop_bit(position->bits[i], source);
            if((i == p || i == P) && (dest < 8 || dest > 55)) m_set_bit(position->bits[i + Q - P], dest);
            else m_set_bit(position->bits[i], dest);
        }
    }
    position->side = !position->side;
}
int negamax(Position* position, int alpha, int beta, int depth, int depth_start, U16* best, int* nodes){
    Move_tab tab = move_generator(position);
    if(depth <= 0){
        (*nodes)++;
        return evaluate_position2(position);
    }
    for(int i = 0; i < tab.count; i++){
        Position copy;
        memcpy(&copy, position, sizeof(Position));
        make_move(&copy, tab.moves[i]);
        int score = -negamax(&copy, -beta, -alpha, depth - 1, depth_start, best, nodes);
        if(score > beta) return beta;
        if(score > alpha){
            alpha = score;
            if(depth == depth_start) *best = tab.moves[i];
        }
    }
    return alpha;
}
void engine_move(Position* position){
    int nodes = 0, score;
    int depth = 1;
    U16 move;
    while(nodes < 0xffff){
        nodes = 0;
        score = -negamax(position, -0xffffff, 0xffffff, depth, depth, &move, &nodes);
        //engine evaluations/debug
        //printf("\nScore: %d Depth: %d Nodes: %d\n", score, depth, nodes);
        depth++;
    }
    make_move(position, move);
}
void read_move(U16 word){
    int source;
    int dest;
    m_decode_move(source, dest, word);
    printf("%s%s\n", index_to_coord[source], index_to_coord[dest]);
}
U16 parse_move(){
    int source;
    int dest;
    char buff[100];
    U16 move;
    fgets(buff, sizeof(buff), stdin);
    source = (int)(buff[0] - 'a') + 8 * (7 - (int)buff[1] + '1');
    dest = (int)(buff[2] - 'a') + 8 * (7 - (int)buff[3] + '1');
    move = m_encode_move(source, dest);
    return move;
}
U16 query_for_move(Position* position){
    Move_tab tab = move_generator(position);
    while(1){
        U16 move = parse_move();
        for(int i = 0; i < tab.count; i++){
            if(move == tab.moves[i]){
                return move;
            }
        }
        print_position(position);
        printf("\nhere are possible moves you dummy\n");
        for(int i = 0; i < tab.count; i++){
            read_move(tab.moves[i]);
        }
    }
}

int main(){
    Position position;
    memset(&position, 0, sizeof(Position));
    set_standard_position(&position);
    print_position(&position);
    for(int i = 0; i < 100; i++){
        make_move(&position, query_for_move(&position));
        print_position(&position);
        engine_move(&position);
        print_position(&position);
    }
    return 0;
}