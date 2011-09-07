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
#include "precompiled_basic.hxx"
#include <tools/errcode.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbx.hxx>
#include <basic/sbxvar.hxx>
#include <vcl/msgbox.hxx>

#include "object.hxx"
#include "collelem.hxx"

// The sample-object has got the following elements:
// 1) Properties:
//    Name      the name
//    Value     a double-value, both already as default
// 2) Methods:
//    Create    creating a new sub-element
//    Display   display a text
//    Square    Argument * Argument
//    Event     calling a basic event handler
// 3) Subobjects:
//    With Create() a new sub-element can be established, which
//    can be indexed if multiple objects of the same name exist.
// This implementation is an example for a table-controlled version,
// which can contain a lot of elements. The elements are taken from
// the table into the object as required.
// The collection can be found in COLLECTN.*, the collection's
// objects in COLLELEM.*

// The sample-object is in ..\app\mybasic.cxx implemented as follows
// in StarBASIC:


// The nArgs-field of a table entry is encrypted as follows:

#define _ARGSMASK   0x00FF  // up to 255 arguments
#define _RWMASK     0x0F00  // mask for R/W-bits
#define _TYPEMASK   0xF000  // mask for the entry's type

#define _READ       0x0100  // can be read
#define _BWRITE     0x0200  // can be used as Lvalue
#define _LVALUE     _BWRITE // can be used as Lvalue
#define _READWRITE  0x0300  // both
#define _OPT        0x0400  // sal_True: optional parameter
#define _METHOD     0x1000  // mask-bit for a method
#define _PROPERTY   0x2000  // mask-bit for a property
#define _COLL       0x4000  // mask-bit for a collection
                            // combination of the bits above:
#define _FUNCTION   0x1100  // mask for function
#define _LFUNCTION  0x1300  // mask for function that also works as Lvalue
#define _ROPROP     0x2100  // mask Read Only-Property
#define _WOPROP     0x2200  // mask Write Only-Property
#define _RWPROP     0x2300  // mask Read/Write-Property
#define _COLLPROP   0x4100  // mask Read-Collection-Element

#define COLLNAME    "Elements"  // the collection's name, wired hard here

SampleObject::Methods SampleObject::aMethods[] = {
// a sample-method (the return value is SbxNULL)
{ "Display", SbxEMPTY, &SampleObject::Display, 1 | _FUNCTION },
    // a named parameter
    { "message", SbxSTRING, NULL, 0 },
// a sample-function
{ "Square", SbxDOUBLE, &SampleObject::Square, 1 | _FUNCTION },
    // a named parameter
    { "value", SbxDOUBLE, NULL, 0 },
//  Basic-Callback
{ "Event", SbxEMPTY, &SampleObject::Event, 1 | _FUNCTION },
    // a named parameter
    { "event", SbxSTRING, NULL, 0 },
//  create element
{ "Create", SbxEMPTY, &SampleObject::Create, 1 | _FUNCTION },
    // a named parameter
    { "name", SbxSTRING, NULL, 0 },

{ NULL, SbxNULL, NULL, -1 }};  // end of the table

SampleObject::SampleObject( const String& rClass ) : SbxObject( rClass )
{
    SetName( String( RTL_CONSTASCII_USTRINGPARAM("Sample") ) );
    PutDouble( 1.0 );
}

// Finding an element:
// It goes linearly through the method table until an adequate
// method is found.
// If the method/property hasn't been found, return only NULL
// without error code so that a whole chain of objects can be
// asked for their method/property.

SbxVariable* SampleObject::Find( const String& rName, SbxClassType t )
{
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( !pRes && t != SbxCLASS_OBJECT )
    {
        Methods* p = aMethods;
        short nIndex = 0;
        sal_Bool bFound = sal_False;
        while( p->nArgs != -1 )
        {
            if( rName.EqualsIgnoreCaseAscii( p->pName ) )
            {
                bFound = sal_True; break;
            }
            nIndex += ( p->nArgs & _ARGSMASK ) + 1;
            p = aMethods + nIndex;
        }
        if( bFound )
        {
            // isolate args-fields:
            short nAccess = ( p->nArgs & _RWMASK ) >> 8;
            short nType   = ( p->nArgs & _TYPEMASK );
            String aName_ = String::CreateFromAscii( p->pName );
            SbxClassType eCT = SbxCLASS_OBJECT;
            if( nType & _PROPERTY )
                eCT = SbxCLASS_PROPERTY;
            else if( nType & _METHOD )
                eCT = SbxCLASS_METHOD;
            pRes = Make( aName_, eCT, p->eType );
            // We set the array-index + 1 because there are other standard-
            // properties existing which have to activated too.
            pRes->SetUserData( nIndex + 1 );
            pRes->SetFlags( nAccess );
        }
    }
    return pRes;
}

// activation of an element or ask for an info block

void SampleObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCT,
                             const SfxHint& rHint, const TypeId& rHT )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pPar_ = pVar->GetParameters();
        sal_uInt16 nIndex = (sal_uInt16) pVar->GetUserData();
        // no index: hand on!
        if( nIndex )
        {
            sal_uIntPtr t = pHint->GetId();
            if( t == SBX_HINT_INFOWANTED )
                pVar->SetInfo( GetInfo( (short) pVar->GetUserData() ) );
            else
            {
                sal_Bool bWrite = sal_False;
                if( t == SBX_HINT_DATACHANGED )
                    bWrite = sal_True;
                if( t == SBX_HINT_DATAWANTED || bWrite )
                {
                    // parameter-test for methods:
                    sal_uInt16 nPar = aMethods[ --nIndex ].nArgs & 0x00FF;
                    // element 0 is the return value
                    if( ( !pPar_ && nPar )
                     || ( pPar_->Count() != nPar+1 ) )
                        SetError( SbxERR_WRONG_ARGS );
                    // alright, the call can be done
                    else
                    {
                        (this->*(aMethods[ nIndex ].pFunc))( pVar, pPar_, bWrite );
                    }
                }
            }
        }
        SbxObject::SFX_NOTIFY( rBC, rBCT, rHint, rHT );
    }
}

// construction of the info structure for single elements

SbxInfo* SampleObject::GetInfo( short nIdx )
{
    Methods* p = &aMethods[ nIdx ];
    SbxInfo* pInfo_ = new SbxInfo;
    short nPar = p->nArgs & _ARGSMASK;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        String aName_ = String::CreateFromAscii( p->pName );
        sal_uInt16 nFlags_ = ( p->nArgs >> 8 ) & 0x03;
        if( p->nArgs & _OPT )
            nFlags_ |= SBX_OPTIONAL;
        pInfo_->AddParam( aName_, p->eType, nFlags_ );
    }
    return pInfo_;
}


// Properties and methods lay down the return value in element 0 of the
// Argv at Get (bPut = sal_False); at Put (bPut = sal_True) the value from
// element 0 is saved.


void SampleObject::Display( SbxVariable*, SbxArray* pPar_, sal_Bool )
{
    // GetString() might perhaps cause an error!
    String s( pPar_->Get( 1 )->GetString() );
    if( !IsError() )
        InfoBox( NULL, s ).Execute();
}

void SampleObject::Square( SbxVariable* pVar, SbxArray* pPar_, sal_Bool )
{
    double n = pPar_->Get( 1 )->GetDouble();
    pVar->PutDouble( n * n );
}

// Callback to BASIC:

void SampleObject::Event( SbxVariable*, SbxArray* pPar_, sal_Bool )
{
    Call( pPar_->Get( 1 )->GetString(), NULL );
}


void SampleObject::Create( SbxVariable* pVar, SbxArray* pPar_, sal_Bool )
{
    pVar->PutObject(
        MakeObject( pPar_->Get( 1 )->GetString(), String( RTL_CONSTASCII_USTRINGPARAM("SampleElement") ) ) );
}


SbxObject* SampleObjectFac::CreateObject( const String& rClass )
{
    if( rClass.EqualsIgnoreCaseAscii( "SampleObject" ) )
        return new SampleObject( rClass );
    if( rClass.EqualsIgnoreCaseAscii( "SampleElement" ) )
        return new SampleElement( rClass );
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
