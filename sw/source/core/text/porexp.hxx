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

#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_POREXP_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_POREXP_HXX

#include "portxt.hxx"

class SwExpandPortion : public SwTextPortion
{
public:
    SwExpandPortion() { SetWhichPor( PortionType::Expand ); }
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual TextFrameIndex GetCursorOfst(sal_uInt16 nOfst) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwBlankPortion : public SwExpandPortion
{
    sal_Unicode cChar;
    bool const bMulti;        // For multiportion brackets
public:
    SwBlankPortion( sal_Unicode cCh, bool bMult = false )
        : cChar( cCh ), bMulti( bMult )
        { cChar = cCh; SetLen(TextFrameIndex(1)); SetWhichPor( PortionType::Blank ); }

    virtual SwLinePortion *Compress() override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    static sal_uInt16 MayUnderflow(const SwTextFormatInfo &rInf, TextFrameIndex nIdx,
        bool bUnderflow );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwPostItsPortion : public SwExpandPortion
{
    bool const    bScript;
public:
    explicit SwPostItsPortion( bool bScrpt );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    bool IsScript() const { return bScript; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
