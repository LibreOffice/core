/*************************************************************************
 *
 *  $RCSfile: step1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdlib.h>
#include <svtools/sbx.hxx>
#ifndef _TOOLS_SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif
#ifndef _TOOLS_INTN_HXX //autogen wg. International
#include <tools/intn.hxx>
#endif
#include "runtime.hxx"
#pragma hdrstop
#include "sbintern.hxx"
#include "iosys.hxx"
#include "image.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )

// Laden einer numerischen Konstanten (+ID)

void SbiRuntime::StepLOADNC( USHORT nOp1 )
{
    static International aEnglischIntn( LANGUAGE_ENGLISH_US, LANGUAGE_ENGLISH_US );

    SbxVariable* p = new SbxVariable( SbxDOUBLE );

    // #57844 Lokalisierte Funktion benutzen
    int nErrno;
    String aStr = pImg->GetString( nOp1 );
    // Auch , zulassen !!!
    USHORT iComma = aStr.Search( ',' );
    if( iComma != STRING_NOTFOUND )
    {
        String aStr1 = aStr.Copy( 0, iComma );
        String aStr2 = aStr.Copy( iComma + 1 );
        aStr = aStr1;
        aStr += '.';
        aStr += aStr2;
    }
    double n = SolarMath::StringToDouble( aStr.GetBuffer(), aEnglischIntn, nErrno );
    //ALT: double n = atof( pImg->GetString( nOp1 ) );

    p->PutDouble( n );
    PushVar( p );
}

// Laden einer Stringkonstanten (+ID)

void SbiRuntime::StepLOADSC( USHORT nOp1 )
{
    SbxVariable* p = new SbxVariable;
    p->PutString( pImg->GetString( nOp1 ) );
    PushVar( p );
}

// Immediate Load (+Wert)

void SbiRuntime::StepLOADI( USHORT nOp1 )
{
    SbxVariable* p = new SbxVariable;
    p->PutInteger( nOp1 );
    PushVar( p );
}

// Speichern eines named Arguments in Argv (+Arg-Nr ab 1!)

void SbiRuntime::StepARGN( USHORT nOp1 )
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        String aAlias( pImg->GetString( nOp1 ) );
        SbxVariableRef pVal = PopVar();
        refArgv->Put( pVal, nArgc );
        refArgv->PutAlias( aAlias, nArgc++ );
    }
}

// Konvertierung des Typs eines Arguments in Argv fuer DECLARE-Fkt. (+Typ)

void SbiRuntime::StepARGTYP( USHORT nOp1 )
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        BOOL bByVal = (nOp1 & 0x8000) != 0;         // Ist BYVAL verlangt?
        SbxDataType t = (SbxDataType) (nOp1 & 0x7FFF);
        SbxVariable* pVar = refArgv->Get( refArgv->Count() - 1 );   // letztes Arg

        // BYVAL prüfen
        if( pVar->GetRefCount() > 2 )       // 2 ist normal für BYVAL
        {
            // Parameter ist eine Referenz
            if( bByVal )
            {
                // Call by Value ist verlangt -> Kopie anlegen
                pVar = new SbxVariable( *pVar );
                pVar->SetFlag( SBX_READWRITE );
                refExprStk->Put( pVar, refArgv->Count() - 1 );
            }
            else
                pVar->SetFlag( SBX_REFERENCE );     // Ref-Flag für DllMgr
        }
        else
        {
            // Parameter ist KEINE Referenz
            if( bByVal )
                pVar->ResetFlag( SBX_REFERENCE );   // Keine Referenz -> OK
            else
                Error( SbERR_BAD_PARAMETERS );      // Referenz verlangt
        }

        if( pVar->GetType() != t )
        {
            // Variant, damit richtige Konvertierung
            // Ausserdem Fehler, wenn SbxBYREF
            pVar->Convert( SbxVARIANT );
            pVar->Convert( t );
        }
    }
}

// String auf feste Laenge bringen (+Laenge)

void SbiRuntime::StepPAD( USHORT nOp1 )
{
    SbxVariable* p = GetTOS();
    String& s = (String&)(const String&) *p;
    if( s.Len() > nOp1 )
        s.Erase( nOp1 );
    else
        s.Expand( nOp1, ' ' );
}

// Sprung (+Target)

void SbiRuntime::StepJUMP( USHORT nOp1 )
{
#ifndef PRODUCT
    if( nOp1 >= pImg->GetCodeSize() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
#endif
    pCode = (const BYTE*) pImg->GetCode() + nOp1;
}

// TOS auswerten, bedingter Sprung (+Target)

void SbiRuntime::StepJUMPT( USHORT nOp1 )
{
    SbxVariableRef p = PopVar();
    if( p->GetBool() )
        StepJUMP( nOp1 );
}

// TOS auswerten, bedingter Sprung (+Target)

void SbiRuntime::StepJUMPF( USHORT nOp1 )
{
    SbxVariableRef p = PopVar();
    if( !p->GetBool() )
        StepJUMP( nOp1 );
}

// TOS auswerten, Sprung in JUMP-Tabelle (+MaxVal)
// Sieht so aus:
// ONJUMP 2
// JUMP target1
// JUMP target2
// ...
//Falls im Operanden 0x8000 gesetzt ist, Returnadresse pushen (ON..GOSUB)

void SbiRuntime::StepONJUMP( USHORT nOp1 )
{
    SbxVariableRef p = PopVar();
    INT16 n = p->GetInteger();
    if( nOp1 & 0x8000 )
    {
        nOp1 &= 0x7FFF;
        PushGosub( pCode + 3 * nOp1 );
    }
    if( n < 1 || n > (short) nOp1 )
        n = nOp1 + 1;
    nOp1 = (USHORT) ( (const char*) pCode - pImg->GetCode() ) + 3 * --n;
    StepJUMP( nOp1 );
}

// UP-Aufruf (+Target)

void SbiRuntime::StepGOSUB( USHORT nOp1 )
{
    PushGosub( pCode );
    if( nOp1 >= pImg->GetCodeSize() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    pCode = (const BYTE*) pImg->GetCode() + nOp1;
}

// UP-Return (+0 oder Target)

void SbiRuntime::StepRETURN( USHORT nOp1 )
{
    PopGosub();
    if( nOp1 )
        StepJUMP( nOp1 );
}

// FOR-Variable testen (+Endlabel)

void SbiRuntime::StepTESTFOR( USHORT nOp1 )
{
    if( !pForStk )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxOperator eOp = ( pForStk->refInc->GetDouble() < 0 ) ? SbxLT : SbxGT;
        if( pForStk->refVar->Compare( eOp, *pForStk->refEnd ) )
        {
            PopFor();
            StepJUMP( nOp1 );
        }
    }
}

// Tos+1 <= Tos+2 <= Tos, 2xremove (+Target)

void SbiRuntime::StepCASETO( USHORT nOp1 )
{
    if( !refCaseStk || !refCaseStk->Count() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxVariableRef xTo   = PopVar();
        SbxVariableRef xFrom = PopVar();
        SbxVariableRef xCase = refCaseStk->Get( refCaseStk->Count() - 1 );
        if( *xCase >= *xFrom && *xCase <= *xTo )
            StepJUMP( nOp1 );
    }
}

// Fehler-Handler

void SbiRuntime::StepERRHDL( USHORT nOp1 )
{
    const BYTE* p = pCode;
    StepJUMP( nOp1 );
    pError = pCode;
    pCode = p;
    pInst->aErrorMsg = String();
    pInst->nErr =
    pInst->nErl =
    nError = 0;
}

// Resume nach Fehlern (+0=statement, 1=next or Label)

void SbiRuntime::StepRESUME( USHORT nOp1 )
{
    // AB #32714 Resume ohne Error? -> Fehler
    if( !bInError )
    {
        Error( SbERR_BAD_RESUME );
        return;
    }
    if( nOp1 )
    {
        // Code-Zeiger auf naechstes Statement setzen
        USHORT n1, n2;
        pCode = pMod->FindNextStmnt( pErrCode, n1, n2 );
    }
    else
        pCode = pErrStmnt;

    if( nOp1 > 1 )
        StepJUMP( nOp1 );
    pInst->aErrorMsg = String();
    pInst->nErr =
    pInst->nErl =
    nError = 0;
    bInError = FALSE;

    // Error-Stack loeschen
    SbErrorStack*& rErrStack = GetSbData()->pErrStack;
    delete rErrStack;
    rErrStack = NULL;
}

// Kanal schliessen (+Kanal, 0=Alle)
void SbiRuntime::StepCLOSE( USHORT nOp1 )
{
    short err;
    if( !nOp1 )
        pIosys->Shutdown();
    else
    {
        err = pIosys->GetError();
        if( !err )
        {
            pIosys->Close();
        }
    }
    err = pIosys->GetError();
    Error( err );
}

// Zeichen ausgeben (+char)

void SbiRuntime::StepPRCHAR( USHORT nOp1 )
{
    ByteString s( (char) nOp1 );
    pIosys->Write( s );
    Error( pIosys->GetError() );
}

// Check, ob TOS eine bestimmte Objektklasse ist (+StringID)

void SbiRuntime::StepCLASS( USHORT nOp1 )
{
    String aClass( pImg->GetString( nOp1 ) );
    SbxVariable* pVar = GetTOS();
    if( pVar->GetType() != SbxOBJECT )
        Error( SbERR_NEEDS_OBJECT );
    else
    {
        SbxObject* pObj;
        if( pVar->IsA( TYPE(SbxObject) ) )
            pObj = (SbxObject*) pVar;
        else
        {
            pObj = (SbxObject*) pVar->GetObject();
            if( pObj && !pObj->IsA( TYPE(SbxObject) ) )
                pObj = NULL;
        }
        if( !pObj || !pObj->IsClass( aClass ) )
            Error( SbERR_INVALID_USAGE_OBJECT );
    }
}

// Library fuer anschliessenden Declare-Call definieren

void SbiRuntime::StepLIB( USHORT nOp1 )
{
    aLibName = pImg->GetString( nOp1 );
}

// TOS wird um BASE erhoeht, BASE davor gepusht (+BASE)
// Dieser Opcode wird vor DIM/REDIM-Anweisungen gepusht,
// wenn nur ein Index angegeben wurde.

void SbiRuntime::StepBASED( USHORT nOp1 )
{
    SbxVariable* p1 = new SbxVariable;
    SbxVariableRef x2 = PopVar();
    p1->PutInteger( nOp1 );
    x2->Compute( SbxPLUS, *p1 );
    PushVar( x2 );  // erst die Expr
    PushVar( p1 );  // dann die Base
}





