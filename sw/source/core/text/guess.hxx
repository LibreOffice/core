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
    css::uno::Reference< css::linguistic2::XHyphenatedWord >  m_xHyphWord;
    std::unique_ptr<SwHangingPortion> m_pHanging; // for hanging punctuation
    TextFrameIndex m_nCutPos;         // this character doesn't fit
    TextFrameIndex m_nBreakStart;     // start index of word containing line break
    TextFrameIndex m_nBreakPos;       // start index of break position
    TextFrameIndex m_nFieldDiff;      // absolute positions can be wrong if we
                               // a field in the text has been expanded
    SwTwips m_nBreakWidth; // width of the broken portion
    SwTwips m_nExtraBlankWidth; // width of spaces after the break
public:
    SwTextGuess(): m_nCutPos(0), m_nBreakStart(0),
                   m_nBreakPos(0), m_nFieldDiff(0), m_nBreakWidth(0), m_nExtraBlankWidth(0)
        { }

    // true, if current portion still fits to current line
    bool Guess( const SwTextPortion& rPor, SwTextFormatInfo &rInf,
                    const sal_uInt16 nHeight, sal_Int32 nSpacesInLine = 0 );
    bool AlternativeSpelling( const SwTextFormatInfo &rInf, const TextFrameIndex nPos );

    SwHangingPortion* GetHangingPortion() const { return m_pHanging.get(); }
    SwHangingPortion* ReleaseHangingPortion() { return m_pHanging.release(); }
    SwTwips BreakWidth() const { return m_nBreakWidth; }
    SwTwips ExtraBlankWidth() const { return m_nExtraBlankWidth; }
    TextFrameIndex CutPos() const { return m_nCutPos; }
    TextFrameIndex BreakStart() const { return m_nBreakStart; }
    TextFrameIndex BreakPos() const {return m_nBreakPos; }
    TextFrameIndex FieldDiff() const {return m_nFieldDiff; }
    const css::uno::Reference< css::linguistic2::XHyphenatedWord >& HyphWord() const
        { return m_xHyphWord; }
private:
    bool maybeAdjustPositionsForBlockAdjust(tools::Long& rMaxSizeDiff,
                                            SwTwips& rExtraAscent, SwTwips& rExtraDescent,
                                            const SwTextFormatInfo& rInf, const SwScriptInfo& rSI,
                                            sal_uInt16 maxComp,
                                            std::optional<SwLinePortionLayoutContext> nLayoutContext);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
