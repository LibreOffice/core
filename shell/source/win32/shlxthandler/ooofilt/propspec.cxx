/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propspec.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:01:28 $
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

//+-------------------------------------------------------------------------
//
//  File:       propspec.cxx
//
//  Contents:   C++ wrappers for FULLPROPSPEC
//
//--------------------------------------------------------------------------
#include <windows.h>
#pragma warning(disable: 4512)
#include <filter.h>
#pragma hdrstop

#include "propspec.hxx"

//GUID CLSID_Storage = PSGUID_STORAGE;
//
//
//refer to ms-help://MS.VSCC/MS.MSDNVS.2052/com/stgasstg_7agk.htm
//FMTID_SummaryInformation
//
//GUID CLSID_SummaryInforation = {
//    0xF29F85E0,
//    0x4FF9,
//    0x1068,
//    { 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 }
//};
//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::CFullPropSpec, public
//
//  Synopsis:   Default constructor
//
//  Effects:    Defines property with null guid and propid 0
//
//--------------------------------------------------------------------------

CFullPropSpec::CFullPropSpec()
{
    memset( &_guidPropSet, 0, sizeof( _guidPropSet ) );
    _psProperty.ulKind = PRSPEC_PROPID;
    _psProperty.propid = 0;
}
//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::CFullPropSpec, public
//
//  Synopsis:   Construct propid based propspec
//
//  Arguments:  [guidPropSet]  -- Property set
//              [pidProperty] -- Property
//
//--------------------------------------------------------------------------
CFullPropSpec::CFullPropSpec( GUID const & guidPropSet, PROPID pidProperty ) :
    _guidPropSet( guidPropSet )
{
    _psProperty.ulKind = PRSPEC_PROPID;
    _psProperty.propid = pidProperty;
}
//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::CFullPropSpec, public
//
//  Synopsis:   Construct name based propspec
//
//  Arguments:  [guidPropSet] -- Property set
//              [wcsProperty] -- Property
//
//--------------------------------------------------------------------------
CFullPropSpec::CFullPropSpec( GUID const & guidPropSet,
                              WCHAR const * wcsProperty ) :
    _guidPropSet( guidPropSet )
{
    _psProperty.ulKind = PRSPEC_PROPID;
    SetProperty( wcsProperty );
}
//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::CFullPropSpec, public
//
//  Synopsis:   Copy constructor
//
//  Arguments:  [src] -- Source property spec
//
//--------------------------------------------------------------------------
CFullPropSpec::CFullPropSpec( CFullPropSpec const & src ) :
    _guidPropSet( src._guidPropSet )
{
    _psProperty.ulKind = src._psProperty.ulKind;
    if ( _psProperty.ulKind == PRSPEC_LPWSTR )
    {
        if ( src._psProperty.lpwstr )
        {
            _psProperty.ulKind = PRSPEC_PROPID;
            SetProperty( src._psProperty.lpwstr );
        }
        else
            _psProperty.lpwstr = 0;
    }
    else
    {
        _psProperty.propid = src._psProperty.propid;
    }
}
inline void * operator new( size_t /*size*/, void * p )
{
    return( p );
}
//+-------------------------------------------------------------------------
//
//  Member:     CFullPropSpec::operator=, public
//
//  Synopsis:   Assignment operator
//
//  Arguments:  [Property] -- Source property
//
//--------------------------------------------------------------------------
CFullPropSpec & CFullPropSpec::operator=( CFullPropSpec const & Property )
{
    // Clean up.
    CFullPropSpec::~CFullPropSpec();
#pragma warning( disable : 4291 )           // unmatched operator new
    new (this) CFullPropSpec( Property );
#pragma warning( default : 4291 )
    return *this;
}
CFullPropSpec::~CFullPropSpec()
{
    if ( _psProperty.ulKind == PRSPEC_LPWSTR &&
         _psProperty.lpwstr )
    {
        CoTaskMemFree( _psProperty.lpwstr );
    }
}
void CFullPropSpec::SetProperty( PROPID pidProperty )
{
    if ( _psProperty.ulKind == PRSPEC_LPWSTR &&
         0 != _psProperty.lpwstr )
    {
        CoTaskMemFree( _psProperty.lpwstr );
    }
    _psProperty.ulKind = PRSPEC_PROPID;
    _psProperty.propid = pidProperty;
}
BOOL CFullPropSpec::SetProperty( WCHAR const * wcsProperty )
{
    if ( _psProperty.ulKind == PRSPEC_LPWSTR &&
         0 != _psProperty.lpwstr )
    {
        CoTaskMemFree( _psProperty.lpwstr );
    }
    _psProperty.ulKind = PRSPEC_LPWSTR;
    int len = (int) ( (wcslen( wcsProperty ) + 1) * sizeof( WCHAR ) );
    _psProperty.lpwstr = (WCHAR *)CoTaskMemAlloc( len );
    if ( 0 != _psProperty.lpwstr )
    {
        memcpy( _psProperty.lpwstr,
                wcsProperty,
                len );
        return( TRUE );
    }
    else
    {
        _psProperty.lpwstr = 0;
        return( FALSE );
    }
}
int CFullPropSpec::operator==( CFullPropSpec const & prop ) const
{
    if ( memcmp( &prop._guidPropSet,
                 &_guidPropSet,
                 sizeof( _guidPropSet ) ) != 0 ||
         prop._psProperty.ulKind != _psProperty.ulKind )
    {
        return( 0 );
    }
    switch( _psProperty.ulKind )
    {
    case PRSPEC_LPWSTR:
        return( _wcsicmp( GetPropertyName(), prop.GetPropertyName() ) == 0 );
        break;
    case PRSPEC_PROPID:
        return( GetPropertyPropid() == prop.GetPropertyPropid() );
        break;
    default:
        return( 0 );
        break;
    }
}
int CFullPropSpec::operator!=( CFullPropSpec const & prop ) const
{
    if (*this == prop)
        return( 0 );
    else
        return( 1 );
}

