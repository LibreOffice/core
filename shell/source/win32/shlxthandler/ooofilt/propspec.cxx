/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

