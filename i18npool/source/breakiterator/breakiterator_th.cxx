/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: breakiterator_th.cxx,v $
 * $Revision: 1.11 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"
#include <breakiterator_th.hxx>
#include <wtt.h>

#include <string.h> // for memset

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class Breakiterator_th
//  ----------------------------------------------------;
BreakIterator_th::BreakIterator_th()
{
    cBreakIterator = "com.sun.star.i18n.BreakIterator_th";
    wordRule=lineRule=NULL;
}

BreakIterator_th::~BreakIterator_th()
{
}

#define SARA_AM 0x0E33

/*
 * cell composition states
 */

#define ST_COM  1   // Compose the following character with leading char and display in the same cell
#define ST_NXT  2   // display the following character in the next cell
#define ST_NDP  3   // non-display

static const sal_Int16 thaiCompRel[MAX_CT][MAX_CT] = {
    //  C  N  C  L  F  F  F  B  B  B  T  A  A  A  A  A  A
    //  T  O  O  V  V  V  V  V  V  D  O  D  D  D  V  V  V
    //  R  N  N     1  2  3  1  2     N  1  2  3  1  2  3
    //  L     S                       E
    //  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // CTRL 0
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // NON  1
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_COM, ST_COM, ST_COM, ST_COM, ST_COM, ST_COM, ST_COM, ST_COM, ST_COM, ST_COM   }, // CONS 2
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // LV   3
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // FV1  4
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // FV2  5
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // FV3  6
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_COM, ST_COM, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // BV1  7
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_COM, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // BV2  8
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // BD   9
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // TONE 10
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // AD1  11
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // AD2  12
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // AD3  13
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_COM, ST_COM, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // AV1  14
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_COM, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT   }, // AV2  15
    {   ST_NDP, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_NXT, ST_COM, ST_NXT, ST_COM, ST_NXT, ST_NXT, ST_NXT, ST_NXT   } // AV3  16

};

const sal_uInt32 is_ST_COM = (1<<CT_CTRL)|(1<<CT_NON)|(1<<CT_CONS)|(1<<CT_TONE);

static sal_uInt16 SAL_CALL getCombState(const sal_Unicode *text, sal_Int32 pos)
{
    sal_uInt16 ch1 = getCharType(text[pos]);
    sal_uInt16 ch2 = getCharType(text[pos+1]);

    if (text[pos+1] == SARA_AM) {
        if ((1 << ch1) & is_ST_COM)
        return  ST_COM;
        else
        ch2 = CT_AD1;
    }

    return thaiCompRel[ch1][ch2];
}


static sal_Int32 SAL_CALL getACell(const sal_Unicode *text, sal_Int32 pos, sal_Int32 len)
{
    sal_uInt32 curr = 1;
    for (; pos + 1 < len && getCombState(text, pos) == ST_COM; curr++, pos++) {}
    return curr;
}

#define is_Thai(c)  (0x0e00 <= c && c <= 0x0e7f) // Unicode definition for Thai

void SAL_CALL BreakIterator_th::makeIndex(const OUString& Text, sal_Int32 nStartPos)
    throw(RuntimeException)
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
    else if (nextCellIndex[nStartPos] > 0 || ! is_Thai(Text[nStartPos]))
        return;

    const sal_Unicode* str = cachedText.getStr();
    sal_Int32 len = cachedText.getLength(), startPos, endPos;

    startPos = nStartPos;
    while (startPos > 0 && is_Thai(str[startPos-1])) startPos--;
    endPos = nStartPos+1;
    while (endPos < len && is_Thai(str[endPos])) endPos++;

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
