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
    inline  SwExpandPortion() { SetWhichPor( POR_EXP ); }
    virtual bool Format( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
    virtual sal_Int32 GetCrsrOfst( const sal_uInt16 nOfst ) const SAL_OVERRIDE;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const SAL_OVERRIDE;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const SAL_OVERRIDE;
    virtual void Paint( const SwTextPaintInfo &rInf ) const SAL_OVERRIDE;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
};

class SwBlankPortion : public SwExpandPortion
{
    sal_Unicode cChar;
    bool bMulti;        // For multiportion brackets
public:
    inline  SwBlankPortion( sal_Unicode cCh, bool bMult = false )
        : cChar( cCh ), bMulti( bMult )
        { cChar = cCh; SetLen(1); SetWhichPor( POR_BLANK ); }

    bool IsMulti() const { return bMulti; }
    void SetMulti( bool bNew ) { bMulti = bNew; }

    virtual SwLinePortion *Compress() SAL_OVERRIDE;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const SAL_OVERRIDE;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
    virtual bool Format( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void Paint( const SwTextPaintInfo &rInf ) const SAL_OVERRIDE;
    static sal_uInt16 MayUnderflow( const SwTextFormatInfo &rInf, sal_Int32 nIdx,
        bool bUnderflow );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
};

class SwPostItsPortion : public SwExpandPortion
{
    bool    bScript;
public:
    explicit SwPostItsPortion( bool bScrpt );
    virtual void Paint( const SwTextPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual bool Format( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const SAL_OVERRIDE;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const SAL_OVERRIDE;
    bool IsScript() const { return bScript; }
    OUTPUT_OPERATOR_OVERRIDE
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
