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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

//+-------------------------------------------------------------------------
//
//  File:       propspec.cxx
//
//  Contents:   C++ wrappers for FULLPROPSPEC
//
//--------------------------------------------------------------------------
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(disable: 4512)
#endif
#include <filter.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

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
    this->CFullPropSpec::~CFullPropSpec();

#ifdef _MSC_VER
#pragma warning( disable : 4291 )           // unmatched operator new
#endif
    new (this) CFullPropSpec( Property );
#ifdef _MSC_VER
#pragma warning( default : 4291 )
#endif
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
