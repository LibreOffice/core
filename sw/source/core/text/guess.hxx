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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_GUESS_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_GUESS_HXX
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>

#include "swtypes.hxx"
#include "breakit.hxx"
#include "porrst.hxx"

class SwTextFormatInfo;

class SwTextGuess
{
    css::uno::Reference< css::linguistic2::XHyphenatedWord >  xHyphWord;
    SwHangingPortion *pHanging; // for hanging punctuation
    sal_Int32 nCutPos;         // this character doesn't fit
    sal_Int32 nBreakStart;     // start index of word containing line break
    sal_Int32 nBreakPos;       // start index of break position
    sal_Int32 nFieldDiff;      // absolut positions can be wrong if we
                               // a field in the text has been expanded
    sal_uInt16 nBreakWidth;    // width of the broken portion
public:
    inline SwTextGuess(): pHanging( NULL ), nCutPos(0), nBreakStart(0),
                        nBreakPos(0), nFieldDiff(0), nBreakWidth(0)
        { }
    ~SwTextGuess() { delete pHanging; }

    // true, if current portion still fits to current line
    bool Guess( const SwTextPortion& rPor, SwTextFormatInfo &rInf,
                    const sal_uInt16 nHeight );
    bool AlternativeSpelling( const SwTextFormatInfo &rInf, const sal_Int32 nPos );

    inline SwHangingPortion* GetHangingPortion() const { return pHanging; }
    inline void ClearHangingPortion() { pHanging = NULL; }
    inline sal_uInt16 BreakWidth() const { return nBreakWidth; }
    inline sal_Int32 CutPos() const { return nCutPos; }
    inline sal_Int32 BreakStart() const { return nBreakStart; }
    inline sal_Int32 BreakPos() const {return nBreakPos; }
    inline sal_Int32 FieldDiff() const {return nFieldDiff; }
    inline css::uno::Reference< css::linguistic2::XHyphenatedWord > HyphWord() const
        { return xHyphWord; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
