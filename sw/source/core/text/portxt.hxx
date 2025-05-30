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
    virtual TextFrameIndex GetModelPositionForViewPoint(SwTwips nOfst) const override;
    virtual SwPositiveSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual SwTwips CalcSpacing( tools::Long nSpaceAdd, const SwTextSizeInfo &rInf ) const override;

    // Counts the spaces for justified paragraph
    TextFrameIndex GetSpaceCnt(const SwTextSizeInfo &rInf, TextFrameIndex& rCnt) const;

    bool CreateHyphen( SwTextFormatInfo &rInf, SwTextGuess const &rGuess );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;

    sal_uInt16 GetMaxComp(const SwTextFormatInfo &rInf) const;
};

class SwTextInputFieldPortion : public SwTextPortion
{
public:
    SwTextInputFieldPortion();

    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual SwPositiveSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
};

class SwHolePortion : public SwLinePortion
{
    SwTwips m_nBlankWidth;
    bool m_bShowUnderline;

public:
    explicit SwHolePortion(const SwTextPortion& rPor, bool bShowUnderline = false);
    SwTwips GetBlankWidth() const { return m_nBlankWidth; }
    void SetBlankWidth(const SwTwips nNew) { m_nBlankWidth = nNew; }
    virtual SwLinePortion *Compress() override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual SwPositiveSize GetTextSize(const SwTextSizeInfo& rInfo) const override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;

    void dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText,
                   TextFrameIndex& nOffset) const override;
};

class SwFieldMarkPortion : public SwTextPortion
{
    public:
        SwFieldMarkPortion() : SwTextPortion()
        {
            SetWhichPor(PortionType::FieldMark);
        }
        virtual void Paint( const SwTextPaintInfo &rInf ) const override;
        virtual bool Format( SwTextFormatInfo &rInf ) override;
};

class SwFieldFormCheckboxPortion : public SwTextPortion
{
public:
    SwFieldFormCheckboxPortion() : SwTextPortion()
    {
        SetWhichPor(PortionType::FieldFormCheckbox);
    }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
