/*************************************************************************
#*
#* Copyright (c) 2002 Sun Microsystems Inc.
#*
#* Bugs! - Contact Prabhat.Hegde@sun.com
#************************************************************************/

#include <breakiterator_hi.hxx>

#include <string.h> // for memset

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class Breakiterator_hi
//  ----------------------------------------------------;
BreakIterator_hi::BreakIterator_hi()
{
    cBreakIterator = "com.sun.star.i18n.BreakIterator_hi";
}

BreakIterator_hi::~BreakIterator_hi()
{
}

/*
 * Devanagari character class Definitions
 */
#define _NP      1L
#define _UP      (1L<<1)
#define _IV      (1L<<2)
#define _CN      (1L<<3)
#define _CK      (1L<<4)
#define _RC      (1L<<5)
#define _NM      (1L<<6)
#define _IM      (1L<<7)
#define _HL      (1L<<8)
#define _NK      (1L<<9)
#define _VD      (1L<<10)
#define _HD      (1L<<11)
#define _II_M    (1L<<12)
#define _EY_M    (1L<<13)
#define _AI_M    (1L<<14)
#define _OW1_M (1L<<15)
#define _OW2_M (1L<<16)
#define _MS      (1L<<17)
#define _AYE_M (1L<<18)
#define _EE_M    (1L<<19)
#define _AWE_M (1L<<20)
#define _O_M     (1L<<21)
#define _RM      (_II_M|_EY_M|_AI_M|_OW1_M|_OW2_M|_AYE_M|_EE_M|_AWE_M|_O_M)

/* Non-Defined Class type */
#define __ND        0

/*
 * Devanagari character type definitions
 */
#define __UP  1  // ChandraBindu & Anuswar
#define __NP  2  // Visarg
#define __IV    3  // Independant Vowels
#define __CN    4  // Consonants except _CK & _RC
#define __CK    5  // Consonants that can be followed by Nukta
#define __RC    6  // Ra
#define __NM    7  // Matra
#define __RM    8  // Ra + HAL
#define __IM    9  // Choti I Matra
#define __HL    10 // HAL
#define __NK    11 // Nukta
#define __VD    12 // Vedic
#define __HD    13 // Hindu Numerals

/*
 * Devanagari character type table
 */
static const sal_uInt16 devaCT[128] = {
/*         0,    1,    2,    3,    4,    5,    6,    7,
           8,    9,    A,    B,    C,    D,    E,    F, */
/* 0900 */ __ND, __UP, __UP, __NP, __ND, __IV, __IV, __IV,
           __IV, __IV, __IV, __IV, __IV, __IV, __IV, __IV,
/* 0910 */ __IV, __IV, __IV, __IV, __IV, __CK, __CK, __CK,
           __CN, __CN, __CN, __CN, __CK, __CN, __CN, __CN,
/* 0920 */ __CN, __CK, __CK, __CN, __CN, __CN, __CN, __CN,
           __CN, __CN, __CN, __CK, __CN, __CN, __CN, __CN,
/* 0930 */ __RC, __CN, __CN, __CN, __CN, __CN, __CN, __CN,
           __CN, __CN, __ND, __ND, __NK, __VD, __NM, __IM,
/* 0940 */ __RM, __NM, __NM, __NM, __NM, __RM, __RM, __RM,
           __RM, __RM, __RM, __RM, __RM, __HL, __ND, __ND,
/* 0950 */ __ND, __VD, __VD, __VD, __VD, __ND, __ND, __ND,
           __CN, __CN, __CN, __CN, __CN, __CN, __CN, __CN,
/* 0960 */ __IV, __IV, __NM, __NM, __ND, __ND, __HD, __HD,
           __HD, __HD, __HD, __HD, __HD, __HD, __HD, __HD,
/* 0970 */ __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
           __ND, __ND, __ND, __ND, __ND, __ND, __ND, __ND,
};


#define getCharType(x) \
    ((x >= 0x0900 && x < 0x097f) ? devaCT[x - 0x0900] : __ND)

/*
 * Devanagari character composition table
 */
static const sal_uInt16 devaCompRel[14][14] = {
  /*        ND, UP, NP, IV, CN, CK, RC, NM, RM, IM, HL, NK, VD, HD, */
  /* 0  */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* ND */
  /* 1  */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* UP */
  /* 2  */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* NP */
  /* 3  */ { 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* IV */
  /* 4  */ { 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0 }, /* CN */
  /* 5  */ { 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  1,  0,  0 }, /* CK */
  /* 6  */ { 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0 }, /* RC */
  /* 7  */ { 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* NM */
  /* 8  */ { 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* RM */
  /* 9  */ { 0,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* IM */
  /* 10 */ { 0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0 }, /* HL */
  /* 11 */ { 0,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0 }, /* NK */
  /* 12 */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, /* VD */
  /* 13 */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }  /* HD */
};

static sal_uInt16 SAL_CALL
getCombState(const sal_Unicode *text, sal_Int32 pos)
{
    sal_uInt16 ch1 = getCharType(text[pos]);
    sal_uInt16 ch2 = getCharType(text[pos+1]);

    return devaCompRel[ch1][ch2];
}

static sal_uInt32 SAL_CALL
getACell(const sal_Unicode *text, sal_Int32 pos, sal_Int32 len)
{
    sal_uInt32 curr = 1;
    for (; pos + 1 < len && getCombState(text, pos) == 1; curr++, pos++) {}
    return curr;
}

#define is_Hindi(x) ((x >= 0x0900 && x < 0x097f) || (x >= 0x0905 && x < 0x0940) || \
                     (x >= 0x093c && x < 0x094e) || (x >= 0x0950 && x < 0x0954) || \
                     (x >= 0x0958 && x < 0x0971))

void SAL_CALL
i18n::BreakIterator_hi::makeIndex(const OUString& Text, sal_Int32 nStartPos)
    throw(uno::RuntimeException)
{
    if (Text != cachedText) {
    cachedText = Text;
    if (cellIndexSize < cachedText.getLength()) {
      cellIndexSize = cachedText.getLength();
      free(nextCellIndex);
      free(previousCellIndex);
      nextCellIndex = (sal_Int32*) calloc(cellIndexSize, sizeof(sal_Int32));
      previousCellIndex = (sal_Int32*) calloc(cellIndexSize, sizeof(sal_Int32));
    }
    // reset nextCell for new Text
    memset(nextCellIndex, 0, cellIndexSize * sizeof(sal_Int32));
    }
    else if (nextCellIndex[nStartPos] > 0 || ! is_Hindi(Text[nStartPos]))
    return;

    const sal_Unicode* str = cachedText.getStr();
    sal_Int32 len = cachedText.getLength(), startPos, endPos;

    startPos = nStartPos;
    while (startPos > 0 && is_Hindi(str[startPos-1])) startPos--;
    endPos = nStartPos+1;
    while (endPos < len && is_Hindi(str[endPos])) endPos++;

    sal_Int32 start, end, pos;
    pos = start = end = startPos;

    while (pos < endPos) {
    end += getACell(str, start, endPos);
    while (pos < end) {
      nextCellIndex[pos] = end;
      previousCellIndex[pos] = start;
      pos++;
    }
    start = end;
    }
}

} } } }
