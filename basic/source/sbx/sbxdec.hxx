/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxdec.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:49:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef __SBX_SBX_DEC_HXX
#define __SBX_SBX_DEC_HXX

#ifdef WIN32

#include <tools/prewin.h>
}   // close extern "C" {

#include <comutil.h>
#include <oleauto.h>

extern "C" {    // reopen extern "C" {
#include <tools/postwin.h>

#endif
#endif

#ifndef _SBXCLASS_HXX
#include "sbx.hxx"
#endif

#include <com/sun/star/bridge/oleautomation/Decimal.hpp>


// Decimal support
// Implementation only for windows

class SbxDecimal
{
    friend void releaseDecimalPtr( SbxDecimal*& rpDecimal );

#ifdef WIN32
    DECIMAL     maDec;
#endif
    INT32       mnRefCount;

public:
    SbxDecimal( void );
    SbxDecimal( const SbxDecimal& rDec );
    SbxDecimal( const com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );

    ~SbxDecimal();

    void addRef( void )
        { mnRefCount++; }

    void fillAutomationDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec );

    void setChar( sal_Unicode val );
    void setByte( BYTE val );
    void setShort( INT16 val );
    void setLong( INT32 val );
    void setUShort( UINT16 val );
    void setULong( UINT32 val );
    bool setSingle( float val );
    bool setDouble( double val );
    void setInt( int val );
    void setUInt( unsigned int val );
    bool setString( String* pString );
    void setDecimal( SbxDecimal* pDecimal )
    {
#ifdef WIN32
        if( pDecimal )
            maDec = pDecimal->maDec;
#endif
    }

    bool getChar( sal_Unicode& rVal );
    bool getByte( BYTE& rVal );
    bool getShort( INT16& rVal );
    bool getLong( INT32& rVal );
    bool getUShort( UINT16& rVal );
    bool getULong( UINT32& rVal );
    bool getSingle( float& rVal );
    bool getDouble( double& rVal );
    bool getInt( int& rVal );
    bool getUInt( unsigned int& rVal );
    bool getString( String& rString );

    bool operator -= ( const SbxDecimal &r );
    bool operator += ( const SbxDecimal &r );
    bool operator /= ( const SbxDecimal &r );
    bool operator *= ( const SbxDecimal &r );
    bool neg( void );

    bool isZero( void );

    enum CmpResult { LT, EQ, GT };
    friend CmpResult compare( const SbxDecimal &rLeft, const SbxDecimal &rRight );
};

