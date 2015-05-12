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

#include <tools/mempool.hxx>

#include "porlin.hxx"

class SwTextGuess;

/// This portion represents a part of the paragraph string.
class SwTextPortion : public SwLinePortion
{
    void BreakCut( SwTextFormatInfo &rInf, const SwTextGuess &rGuess );
    void BreakUnderflow( SwTextFormatInfo &rInf );
    bool _Format( SwTextFormatInfo &rInf );

public:
    inline SwTextPortion(){ SetWhichPor( POR_TXT ); }
    SwTextPortion( const SwLinePortion &rPortion );
    virtual void Paint( const SwTextPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual bool Format( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
    virtual sal_Int32 GetCrsrOfst( const sal_uInt16 nOfst ) const SAL_OVERRIDE;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const SAL_OVERRIDE;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const SAL_OVERRIDE;
    virtual long CalcSpacing( long nSpaceAdd, const SwTextSizeInfo &rInf ) const SAL_OVERRIDE;

    // Counts the spaces for justified paragraph
    sal_Int32 GetSpaceCnt( const SwTextSizeInfo &rInf, sal_Int32& rCnt ) const;

    bool CreateHyphen( SwTextFormatInfo &rInf, SwTextGuess &rGuess );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
    DECL_FIXEDMEMPOOL_NEWDEL(SwTextPortion)
};

class SwTextInputFieldPortion : public SwTextPortion
{
public:
    SwTextInputFieldPortion();

    virtual bool Format( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void Paint( const SwTextPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const SAL_OVERRIDE;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const SAL_OVERRIDE;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const SAL_OVERRIDE;

private:
    bool mbContainsInputFieldStart;
    bool mbContainsInputFieldEnd;
    bool ContainsOnlyDummyChars() const;
};

class SwHolePortion : public SwLinePortion
{
    sal_uInt16 nBlankWidth;
public:
            SwHolePortion( const SwTextPortion &rPor );
    inline sal_uInt16 GetBlankWidth( ) const { return nBlankWidth; }
    inline void SetBlankWidth( const sal_uInt16 nNew ) { nBlankWidth = nNew; }
    virtual SwLinePortion *Compress() SAL_OVERRIDE;
    virtual bool Format( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void Paint( const SwTextPaintInfo &rInf ) const SAL_OVERRIDE;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
    DECL_FIXEDMEMPOOL_NEWDEL(SwHolePortion)
};

class SwFieldMarkPortion : public SwTextPortion
{
    public:
        inline SwFieldMarkPortion() : SwTextPortion()
            { }
        virtual void Paint( const SwTextPaintInfo &rInf ) const SAL_OVERRIDE;
        virtual bool Format( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
};

class SwFieldFormCheckboxPortion : public SwTextPortion
{
public:
    SwFieldFormCheckboxPortion() : SwTextPortion()
    {
    }
    virtual void Paint( const SwTextPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual bool Format( SwTextFormatInfo &rInf ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
