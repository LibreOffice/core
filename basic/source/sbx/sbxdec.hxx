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


#ifndef INCLUDED_BASIC_SOURCE_SBX_SBXDEC_HXX
#define INCLUDED_BASIC_SOURCE_SBX_SBXDEC_HXX

#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#include <comutil.h>
#include <oleauto.h>
#endif

#endif

#include <basic/sbx.hxx>

#include <com/sun/star/bridge/oleautomation/Decimal.hpp>


// Decimal support
// Implementation only for windows

class SbxDecimal
{
    friend void releaseDecimalPtr( SbxDecimal*& rpDecimal );

#ifdef _WIN32
    DECIMAL     maDec;
#endif
    sal_Int32       mnRefCount;

public:
    SbxDecimal();
    SbxDecimal( const SbxDecimal& rDec );
    explicit SbxDecimal( const css::bridge::oleautomation::Decimal& rAutomationDec );

    ~SbxDecimal();

    void addRef()
        { mnRefCount++; }

    void fillAutomationDecimal( css::bridge::oleautomation::Decimal& rAutomationDec );

    void setChar( sal_Unicode val );
    void setByte( sal_uInt8 val );
    void setShort( sal_Int16 val );
    void setLong( sal_Int32 val );
    void setUShort( sal_uInt16 val );
    void setULong( sal_uInt32 val );
    bool setSingle( float val );
    bool setDouble( double val );
    void setInt( int val );
    void setUInt( unsigned int val );
    bool setString( OUString* pOUString );
    void setDecimal( SbxDecimal* pDecimal )
    {
#ifdef _WIN32
        if( pDecimal )
            maDec = pDecimal->maDec;
#else
        (void)pDecimal;
#endif
    }

    bool getChar( sal_Unicode& rVal );
    bool getShort( sal_Int16& rVal );
    bool getLong( sal_Int32& rVal );
    bool getUShort( sal_uInt16& rVal );
    bool getULong( sal_uInt32& rVal );
    bool getSingle( float& rVal );
    bool getDouble( double& rVal );
    void getString( OUString& rString );

    bool operator -= ( const SbxDecimal &r );
    bool operator += ( const SbxDecimal &r );
    bool operator /= ( const SbxDecimal &r );
    bool operator *= ( const SbxDecimal &r );
    bool neg();

    bool isZero();

    enum CmpResult { LT, EQ, GT };
    friend CmpResult compare( const SbxDecimal &rLeft, const SbxDecimal &rRight );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
