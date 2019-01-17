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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORTXT_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORTXT_HXX

#include "porlin.hxx"

class SwTextGuess;

/// This portion represents a part of the paragraph string.
class SwTextPortion : public SwLinePortion
{
    void BreakCut( SwTextFormatInfo &rInf, const SwTextGuess &rGuess );
    void BreakUnderflow( SwTextFormatInfo &rInf );
    bool Format_( SwTextFormatInfo &rInf );

public:
    SwTextPortion(){ SetWhichPor( PortionType::Text ); }
    static SwTextPortion * CopyLinePortion(const SwLinePortion &rPortion);
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) override;
    virtual TextFrameIndex GetCursorOfst(sal_uInt16 nOfst) const override;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual long CalcSpacing( long nSpaceAdd, const SwTextSizeInfo &rInf ) const override;

    // Counts the spaces for justified paragraph
    TextFrameIndex GetSpaceCnt(const SwTextSizeInfo &rInf, TextFrameIndex& rCnt) const;

    bool CreateHyphen( SwTextFormatInfo &rInf, SwTextGuess const &rGuess );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwTextInputFieldPortion : public SwTextPortion
{
public:
    SwTextInputFieldPortion();

    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
};

class SwHolePortion : public SwLinePortion
{
    sal_uInt16 nBlankWidth;
public:
    explicit SwHolePortion( const SwTextPortion &rPor );
    sal_uInt16 GetBlankWidth( ) const { return nBlankWidth; }
    void SetBlankWidth( const sal_uInt16 nNew ) { nBlankWidth = nNew; }
    virtual SwLinePortion *Compress() override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwFieldMarkPortion : public SwTextPortion
{
    public:
        SwFieldMarkPortion() : SwTextPortion()
            { }
        virtual void Paint( const SwTextPaintInfo &rInf ) const override;
        virtual bool Format( SwTextFormatInfo &rInf ) override;
};

class SwFieldFormCheckboxPortion : public SwTextPortion
{
public:
    SwFieldFormCheckboxPortion() : SwTextPortion()
    {
    }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
