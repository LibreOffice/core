/*************************************************************************
 *
 *  $RCSfile: breakiterator_ctl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 06:26:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
            BreakIterator_Unicode::previousCharacters(Text, nStartPos, rLocale,
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
            BreakIterator_Unicode::nextCharacters(Text, nStartPos, rLocale,
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

} } } }
