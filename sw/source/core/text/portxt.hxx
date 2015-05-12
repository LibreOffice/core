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

class SwTxtGuess;

/// This portion represents a part of the paragraph string.
class SwTxtPortion : public SwLinePortion
{
    void BreakCut( SwTxtFormatInfo &rInf, const SwTxtGuess &rGuess );
    void BreakUnderflow( SwTxtFormatInfo &rInf );
    bool _Format( SwTxtFormatInfo &rInf );

public:
    inline SwTxtPortion(){ SetWhichPor( POR_TXT ); }
    static SwTxtPortion * CopyLinePortion(const SwLinePortion &rPortion);
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void FormatEOL( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
    virtual sal_Int32 GetCrsrOfst( const sal_uInt16 nOfst ) const SAL_OVERRIDE;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const SAL_OVERRIDE;
    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const SAL_OVERRIDE;
    virtual long CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo &rInf ) const SAL_OVERRIDE;

    // Counts the spaces for justified paragraph
    sal_Int32 GetSpaceCnt( const SwTxtSizeInfo &rInf, sal_Int32& rCnt ) const;

    bool CreateHyphen( SwTxtFormatInfo &rInf, SwTxtGuess &rGuess );

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtPortion)
};

class SwTxtInputFldPortion : public SwTxtPortion
{
public:
    SwTxtInputFldPortion();

    virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual sal_uInt16 GetViewWidth( const SwTxtSizeInfo &rInf ) const SAL_OVERRIDE;
    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const SAL_OVERRIDE;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const SAL_OVERRIDE;

private:
    bool mbContainsInputFieldStart;
    bool mbContainsInputFieldEnd;
    bool ContainsOnlyDummyChars() const;
};

class SwHolePortion : public SwLinePortion
{
    sal_uInt16 nBlankWidth;
public:
            SwHolePortion( const SwTxtPortion &rPor );
    inline sal_uInt16 GetBlankWidth( ) const { return nBlankWidth; }
    inline void SetBlankWidth( const sal_uInt16 nNew ) { nBlankWidth = nNew; }
    virtual SwLinePortion *Compress() SAL_OVERRIDE;
    virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const SAL_OVERRIDE;

    OUTPUT_OPERATOR_OVERRIDE
    DECL_FIXEDMEMPOOL_NEWDEL(SwHolePortion)
};

class SwFieldMarkPortion : public SwTxtPortion
{
    public:
        inline SwFieldMarkPortion() : SwTxtPortion()
            { }
        virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
        virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
};

class SwFieldFormCheckboxPortion : public SwTxtPortion
{
public:
    SwFieldFormCheckboxPortion() : SwTxtPortion()
    {
    }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const SAL_OVERRIDE;
    virtual bool Format( SwTxtFormatInfo &rInf ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
