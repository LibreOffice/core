/*************************************************************************
 *
 *  $RCSfile: breakiterator_ctl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:38:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <breakiterator_ctl.hxx>

#include <string.h> // for memset

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

/**
 * Constructor.
 */
BreakIterator_CTL::BreakIterator_CTL()
{
    cBreakIterator = "com.sun.star.i18n.BreakIterator_CTL";
    // to improve performance, alloc big enough memory in construct.
    cellIndexSize = 512;
    nextCellIndex = (sal_Int32*) calloc(cellIndexSize, sizeof(sal_Int32));
    previousCellIndex = (sal_Int32*) calloc(cellIndexSize, sizeof(sal_Int32));
    memset(nextCellIndex, 0, cellIndexSize * sizeof(sal_Int32));
}

/**
 * Deconstructor.
 */
BreakIterator_CTL::~BreakIterator_CTL()
{
    free(nextCellIndex);
    free(previousCellIndex);
}

sal_Int32 SAL_CALL BreakIterator_CTL::previousCharacters( const OUString& Text,
    sal_Int32 nStartPos, const lang::Locale& rLocale,
    sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
    throw(RuntimeException)
{
    if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) {
        nDone = 0;
        if (nStartPos > 0) {    // for others to skip cell.
        makeIndex(Text, nStartPos);

        if (nextCellIndex[nStartPos-1] == 0) // not a CTL character
            return BreakIterator_Unicode::previousCharacters(Text, nStartPos, rLocale,
                nCharacterIteratorMode, nCount, nDone);
        else while (nCount > 0 && nextCellIndex[nStartPos - 1] > 0) {
            nCount--; nDone++;
            nStartPos = previousCellIndex[nStartPos - 1];
        }
        } else
        nStartPos = 0;
    } else { // for BS to delete one char.
        nDone = (nStartPos > nCount) ? nCount : nStartPos;
        nStartPos -= nDone;
    }

    return nStartPos;
}

sal_Int32 SAL_CALL BreakIterator_CTL::nextCharacters(const OUString& Text,
    sal_Int32 nStartPos, const lang::Locale& rLocale,
    sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone)
    throw(RuntimeException)
{
    sal_Int32 len = Text.getLength();
    if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) {
        nDone = 0;
        if (nStartPos < len) {
        makeIndex(Text, nStartPos);

        if (nextCellIndex[nStartPos] == 0) // not a CTL character
            return BreakIterator_Unicode::nextCharacters(Text, nStartPos, rLocale,
                nCharacterIteratorMode, nCount, nDone);
        else while (nCount > 0 && nextCellIndex[nStartPos] > 0) {
            nCount--; nDone++;
            nStartPos = nextCellIndex[nStartPos];
        }
        } else
        nStartPos = len;
    } else {
        nDone = (len - nStartPos > nCount) ? nCount : len - nStartPos;
        nStartPos += nDone;
    }

    return nStartPos;
}

// This method should be overwritten by derived language specific class.
void SAL_CALL BreakIterator_CTL::makeIndex(const OUString& text, sal_Int32 pos)
    throw(RuntimeException)
{
    throw RuntimeException();
}

// Make sure line is broken on cell boundary if we implement cell iterator.
LineBreakResults SAL_CALL BreakIterator_CTL::getLineBreak(
    const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale& rLocale, sal_Int32 nMinBreakPos,
    const LineBreakHyphenationOptions& hOptions,
    const LineBreakUserOptions& bOptions ) throw(RuntimeException)
{
    LineBreakResults lbr = BreakIterator_Unicode::getLineBreak(Text, nStartPos,
                    rLocale, nMinBreakPos, hOptions, bOptions );
    makeIndex(Text, nStartPos);
    lbr.breakIndex = previousCellIndex[ lbr.breakIndex ];
    return lbr;
}

} } } }
