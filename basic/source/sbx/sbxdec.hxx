/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef __SBX_SBX_DEC_HXX
#define __SBX_SBX_DEC_HXX

#ifdef WIN32

#undef WB_LEFT
#undef WB_RIGHT
#include <prewin.h>
}   // close extern "C" {

#ifndef __MINGW32__
#include <comutil.h>
#endif
#include <oleauto.h>

extern "C" {    // reopen extern "C" {
#include <postwin.h>

#endif
#endif
#include <basic/sbx.hxx>

#include <com/sun/star/bridge/oleautomation/Decimal.hpp>


// Decimal support
// Implementation only for windows

class SbxDecimal
{
    friend void releaseDecimalPtr( SbxDecimal*& rpDecimal );

#ifdef WIN32
    DECIMAL     maDec;
#endif
    sal_Int32       mnRefCount;

public:
    SbxDecimal( void );
    SbxDecimal( const SbxDecimal& rDec );
    SbxDecimal( const com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );

    ~SbxDecimal();

    void addRef( void )
        { mnRefCount++; }

    void fillAutomationDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );

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
    bool setString( ::rtl::OUString* pOUString );
    void setDecimal( SbxDecimal* pDecimal )
    {
#ifdef WIN32
        if( pDecimal )
            maDec = pDecimal->maDec;
#else
        (void)pDecimal;
#endif
    }

    bool getChar( sal_Unicode& rVal );
    bool getByte( sal_uInt8& rVal );
    bool getShort( sal_Int16& rVal );
    bool getLong( sal_Int32& rVal );
    bool getUShort( sal_uInt16& rVal );
    bool getULong( sal_uInt32& rVal );
    bool getSingle( float& rVal );
    bool getDouble( double& rVal );
    bool getInt( int& rVal );
    bool getUInt( unsigned int& rVal );
    bool getString( ::rtl::OUString& rString );

    bool operator -= ( const SbxDecimal &r );
    bool operator += ( const SbxDecimal &r );
    bool operator /= ( const SbxDecimal &r );
    bool operator *= ( const SbxDecimal &r );
    bool neg( void );

    bool isZero( void );

    enum CmpResult { LT, EQ, GT };
    friend CmpResult compare( const SbxDecimal &rLeft, const SbxDecimal &rRight );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
