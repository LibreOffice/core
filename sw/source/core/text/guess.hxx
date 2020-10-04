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

#pragma once
#include <memory>

#include "porrst.hxx"

#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::linguistic2 { class XHyphenatedWord; }

class SwTextFormatInfo;

class SwTextGuess
{
    css::uno::Reference< css::linguistic2::XHyphenatedWord >  xHyphWord;
    std::unique_ptr<SwHangingPortion> pHanging; // for hanging punctuation
    TextFrameIndex nCutPos;         // this character doesn't fit
    TextFrameIndex nBreakStart;     // start index of word containing line break
    TextFrameIndex nBreakPos;       // start index of break position
    TextFrameIndex nFieldDiff;      // absolute positions can be wrong if we
                               // a field in the text has been expanded
    sal_uInt16 nBreakWidth;    // width of the broken portion
public:
    SwTextGuess(): nCutPos(0), nBreakStart(0),
                   nBreakPos(0), nFieldDiff(0), nBreakWidth(0)
        { }

    // true, if current portion still fits to current line
    bool Guess( const SwTextPortion& rPor, SwTextFormatInfo &rInf,
                    const sal_uInt16 nHeight );
    bool AlternativeSpelling( const SwTextFormatInfo &rInf, const TextFrameIndex nPos );

    SwHangingPortion* GetHangingPortion() const { return pHanging.get(); }
    SwHangingPortion* ReleaseHangingPortion() { return pHanging.release(); }
    sal_uInt16 BreakWidth() const { return nBreakWidth; }
    TextFrameIndex CutPos() const { return nCutPos; }
    TextFrameIndex BreakStart() const { return nBreakStart; }
    TextFrameIndex BreakPos() const {return nBreakPos; }
    TextFrameIndex FieldDiff() const {return nFieldDiff; }
    const css::uno::Reference< css::linguistic2::XHyphenatedWord >& HyphWord() const
        { return xHyphWord; }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */