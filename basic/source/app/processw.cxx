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
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <basic/sbxvar.hxx>
#endif

//#include <osl/thread.h>
#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#include "processw.hxx"

// Process has the following elements:
// 1) Properties:
//    none
// 2) Methods:
//    SetImage( Filename )
//    sal_Bool Start
//    sal_uInt16 GetExitCode
//    sal_Bool IsRunning
//    sal_Bool WasGPF


// This implementation is a sample for a table driven version that
// can contain lots of elements. The elements are taken into the object
// when they are needed.

// The nArgs field of a table entry is scrambled as follows:
#define _ARGSMASK   0x00FF  // Up to 255 arguments
#define _RWMASK     0x0F00  // Mask for R/W-Bits
#define _TYPEMASK   0xF000  // Mask for entry type

#define _READ       0x0100  // can be read
#define _BWRITE     0x0200  // can be used as Lvaluen
#define _LVALUE     _BWRITE
#define _READWRITE  0x0300  // can read and written
#define _OPT        0x0400  // sal_True: optional parameter
#define _METHOD     0x1000  // Mask-Bit for a method
#define _PROPERTY   0x2000  // Mask-Bit for a property
#define _COLL       0x4000  // Mask-Bit for a collection

// Combination of the bits above:
#define _FUNCTION   0x1100  // Mask for a Function
#define _LFUNCTION  0x1300  // Mask for a Function, that can be uses as Lvalue
#define _ROPROP     0x2100  // Mask Read Only-Property
#define _WOPROP     0x2200  // Mask Write Only-Property
#define _RWPROP     0x2300  // Mask Read/Write-Property
#define _COLLPROP   0x4100  // Mask Read-Collection-Element

#define COLLNAME    "Elements"  // Name of the collection, hard coded


ProcessWrapper::Methods ProcessWrapper::aProcessMethods[] = {
// Imagefile of the Executable
{ "SetImage", SbxEMPTY, &ProcessWrapper::PSetImage, 1 | _FUNCTION },
    // Two Named Parameter
    { "Filename", SbxSTRING, NULL, 0 },
    { "Params", SbxSTRING, NULL, _OPT },
// Program is started
{ "Start", SbxBOOL, &ProcessWrapper::PStart, 0 | _FUNCTION },
// ExitCode of the program
{ "GetExitCode", SbxULONG, &ProcessWrapper::PGetExitCode, 0 | _FUNCTION },
// Program is running
{ "IsRunning", SbxBOOL, &ProcessWrapper::PIsRunning, 0 | _FUNCTION },
// Program has faulted with GPF etc.
{ "WasGPF", SbxBOOL, &ProcessWrapper::PWasGPF, 0 | _FUNCTION },

{ NULL, SbxNULL, NULL, -1 }};  // End of table


ProcessWrapper::ProcessWrapper() : SbxObject( CUniString("Process") )
{
    pProcess = new Process();
    SetName( CUniString("Process") );
    pMethods = &aProcessMethods[0];
}

ProcessWrapper::~ProcessWrapper()
{
    delete pProcess;
}

// Search for an element:
// Linear search through the method table until an appropriate one
// can be found.
// If the the method/property cannot be found, NULL is return
// without an error code, so that we can ask the whole
// chain of objects for the method/property.
SbxVariable* ProcessWrapper::Find( const String& rName, SbxClassType t )
{
    // Is the element already available?
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( !pRes && t != SbxCLASS_OBJECT )
    {
        // otherwise search
        Methods* p = pMethods;
        short nIndex = 0;
        sal_Bool bFound = sal_False;
        while( p->nArgs != -1 )
        {
            if( rName.EqualsIgnoreCaseAscii( p->pName ) )
            {
                bFound = sal_True; break;
            }
            nIndex += ( p->nArgs & _ARGSMASK ) + 1;
            p = pMethods + nIndex;
        }
        if( bFound )
        {
            // isolate Args fields:
            short nAccess = ( p->nArgs & _RWMASK ) >> 8;
            short nType   = ( p->nArgs & _TYPEMASK );
            String aMethodName( p->pName, RTL_TEXTENCODING_ASCII_US );
            SbxClassType eCT = SbxCLASS_OBJECT;
            if( nType & _PROPERTY )
                eCT = SbxCLASS_PROPERTY;
            else if( nType & _METHOD )
                eCT = SbxCLASS_METHOD;
            pRes = Make( aMethodName, eCT, p->eType );
            // We set array index + 1 because there are other
            // default properties that must be activated
            pRes->SetUserData( nIndex + 1 );
            pRes->SetFlags( nAccess );
        }
    }
    return pRes;
}

// Activation of an element or request for an info block
void ProcessWrapper::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCT,
                             const SfxHint& rHint, const TypeId& rHT )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pNotifyPar = pVar->GetParameters();
        sal_uInt16 nIndex = (sal_uInt16) pVar->GetUserData();
        // No index: put through
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
                    // Parameter-Test for methods:
                    sal_uInt16 nPar = pMethods[ --nIndex ].nArgs & 0x00FF;
                    // Element 0 is the return value
                    if( ( !pNotifyPar && nPar )
                     || ( pNotifyPar && pNotifyPar->Count() < nPar+1 ) )
                        SetError( SbxERR_WRONG_ARGS );
                    // ready for call
                    else
                    {
                        (this->*(pMethods[ nIndex ].pFunc))( pVar, pNotifyPar, bWrite );
                    }
                }
            }
        }
        SbxObject::SFX_NOTIFY( rBC, rBCT, rHint, rHT );
    }
}

// Building the info struct for single elements
SbxInfo* ProcessWrapper::GetInfo( short nIdx )
{
    Methods* p = &pMethods[ nIdx ];
    // Wenn mal eine Hilfedatei zur Verfuegung steht:
    // SbxInfo* pResultInfo = new SbxInfo( Hilfedateiname, p->nHelpId );
    SbxInfo* pResultInfo = new SbxInfo;
    short nPar = p->nArgs & _ARGSMASK;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        String aMethodName( p->pName, RTL_TEXTENCODING_ASCII_US );
        sal_uInt16 nInfoFlags = ( p->nArgs >> 8 ) & 0x03;
        if( p->nArgs & _OPT )
            nInfoFlags |= SBX_OPTIONAL;
        pResultInfo->AddParam( aMethodName, p->eType, nInfoFlags );
    }
    return pResultInfo;
}


////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////

// Properties and methods save the return value in argv[0] (Get, bPut = sal_False)
// and store the value from argv[0] (Put, bPut = sal_True)

void ProcessWrapper::PSetImage( SbxVariable* pVar, SbxArray* pMethodePar, sal_Bool bWriteIt )
{ // Imagefile of the executable
    (void) pVar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    if ( pMethodePar->Count() >= 2 )
        pProcess->SetImage(pMethodePar->Get( 1 )->GetString(), pMethodePar->Get( 2 )->GetString() );
    else
        pProcess->SetImage(pMethodePar->Get( 1 )->GetString(), String() );
}

void ProcessWrapper::PStart( SbxVariable* pVar, SbxArray* pMethodePar, sal_Bool bWriteIt )
{ // Program is started
    (void) pMethodePar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    pVar->PutBool( pProcess->Start() );
}

void ProcessWrapper::PGetExitCode( SbxVariable* pVar, SbxArray* pMethodePar, sal_Bool bWriteIt )
{ // ExitCode of the program after it was finished
    (void) pMethodePar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    pVar->PutULong( pProcess->GetExitCode() );
}

void ProcessWrapper::PIsRunning( SbxVariable* pVar, SbxArray* pMethodePar, sal_Bool bWriteIt )
{ // Program is still running
    (void) pMethodePar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    pVar->PutBool( pProcess->IsRunning() );
}

void ProcessWrapper::PWasGPF( SbxVariable* pVar, SbxArray* pMethodePar, sal_Bool bWriteIt )
{ // Program faulted with GPF etc.
    (void) pMethodePar; /* avoid warning about unused parameter */
    (void) bWriteIt; /* avoid warning about unused parameter */
    pVar->PutBool( pProcess->WasGPF() );
}






// The factory creates our object
SbxObject* ProcessFactory::CreateObject( const String& rClass )
{
    if( rClass.CompareIgnoreCaseToAscii( "Process" ) == COMPARE_EQUAL )
        return new ProcessWrapper();
    return NULL;
}

