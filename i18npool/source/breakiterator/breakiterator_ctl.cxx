/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <breakiterator_ctl.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {

/**
 * Constructor.
 */
BreakIterator_CTL::BreakIterator_CTL() :
    cachedText(),
    nextCellIndex( nullptr ),
    previousCellIndex( nullptr ),
    cellIndexSize( 512 )
{
    cBreakIterator = "com.sun.star.i18n.BreakIterator_CTL";
    // to improve performance, alloc big enough memory in construct.
    nextCellIndex = static_cast<sal_Int32*>(calloc(cellIndexSize, sizeof(sal_Int32)));
    previousCellIndex = static_cast<sal_Int32*>(calloc(cellIndexSize, sizeof(sal_Int32)));
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
    throw(RuntimeException, std::exception)
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
    throw(RuntimeException, std::exception)
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
void SAL_CALL BreakIterator_CTL::makeIndex(const OUString& /*text*/, sal_Int32 /*pos*/)
    throw(RuntimeException)
{
    throw RuntimeException();
}

// Make sure line is broken on cell boundary if we implement cell iterator.
LineBreakResults SAL_CALL BreakIterator_CTL::getLineBreak(
    const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale& rLocale, sal_Int32 nMinBreakPos,
    const LineBreakHyphenationOptions& hOptions,
    const LineBreakUserOptions& bOptions ) throw(RuntimeException, std::exception)
{
    LineBreakResults lbr = BreakIterator_Unicode::getLineBreak(Text, nStartPos,
                    rLocale, nMinBreakPos, hOptions, bOptions );
    if (lbr.breakIndex < Text.getLength()) {
        makeIndex(Text, lbr.breakIndex);
        lbr.breakIndex = previousCellIndex[ lbr.breakIndex ];
    }
    return lbr;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
