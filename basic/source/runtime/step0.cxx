/*************************************************************************
 *
 *  $RCSfile: step0.cxx,v $
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

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif

#include <svtools/sbx.hxx>
#include "runtime.hxx"
#pragma hdrstop
#include "sbintern.hxx"
#include "iosys.hxx"
#include <sb.hrc>
#include <basrid.hxx>
#include "sbunoobj.hxx"
#include <com/sun/star/uno/Any.hxx>

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )

void SbiRuntime::StepNOP()
{}

void SbiRuntime::StepArith( SbxOperator eOp )
{
    SbxVariableRef p1 = PopVar();
    TOSMakeTemp();
    SbxVariable* p2 = GetTOS();
    p2->ResetFlag( SBX_FIXED );
    p2->Compute( eOp, *p1 );
}

void SbiRuntime::StepUnary( SbxOperator eOp )
{
    TOSMakeTemp();
    SbxVariable* p = GetTOS();
    p->Compute( eOp, *p );
}

void SbiRuntime::StepCompare( SbxOperator eOp )
{
    SbxVariableRef p1 = PopVar();
    SbxVariableRef p2 = PopVar();
#ifndef WIN
    static SbxVariable* pTRUE = NULL;
    static SbxVariable* pFALSE = NULL;

    if( p2->Compare( eOp, *p1 ) )
    {
        if( !pTRUE )
        {
            pTRUE = new SbxVariable;
            pTRUE->PutBool( TRUE );
            pTRUE->AddRef();
        }
        PushVar( pTRUE );
    }
    else
    {
        if( !pFALSE )
        {
            pFALSE = new SbxVariable;
            pFALSE->PutBool( FALSE );
            pFALSE->AddRef();
        }
        PushVar( pFALSE );
    }
#else
    BOOL bRes = p2->Compare( eOp, *p1 );
    SbxVariable* pRes = new SbxVariable;
    pRes->PutBool( bRes );
    PushVar( pRes );
#endif
}

void SbiRuntime::StepEXP()      { StepArith( SbxEXP );      }
void SbiRuntime::StepMUL()      { StepArith( SbxMUL );      }
void SbiRuntime::StepDIV()      { StepArith( SbxDIV );      }
void SbiRuntime::StepIDIV()     { StepArith( SbxIDIV );     }
void SbiRuntime::StepMOD()      { StepArith( SbxMOD );      }
void SbiRuntime::StepPLUS()     { StepArith( SbxPLUS );     }
void SbiRuntime::StepMINUS()        { StepArith( SbxMINUS );    }
void SbiRuntime::StepCAT()      { StepArith( SbxCAT );      }
void SbiRuntime::StepAND()      { StepArith( SbxAND );      }
void SbiRuntime::StepOR()       { StepArith( SbxOR );       }
void SbiRuntime::StepXOR()      { StepArith( SbxXOR );      }
void SbiRuntime::StepEQV()      { StepArith( SbxEQV );      }
void SbiRuntime::StepIMP()      { StepArith( SbxIMP );      }

void SbiRuntime::StepNEG()      { StepUnary( SbxNEG );      }
void SbiRuntime::StepNOT()      { StepUnary( SbxNOT );      }

void SbiRuntime::StepEQ()       { StepCompare( SbxEQ );     }
void SbiRuntime::StepNE()       { StepCompare( SbxNE );     }
void SbiRuntime::StepLT()       { StepCompare( SbxLT );     }
void SbiRuntime::StepGT()       { StepCompare( SbxGT );     }
void SbiRuntime::StepLE()       { StepCompare( SbxLE );     }
void SbiRuntime::StepGE()       { StepCompare( SbxGE );     }

void SbiRuntime::StepLIKE()
{
    StarBASIC::FatalError( SbERR_NOT_IMPLEMENTED );
}

// TOS und TOS-1 sind beides Objektvariable und enthalten den selben Pointer

void SbiRuntime::StepIS()
{
    SbxVariableRef refVar1 = PopVar();
    SbxVariableRef refVar2 = PopVar();
    BOOL bRes = BOOL(
        refVar1->GetType() == SbxOBJECT
     && refVar2->GetType() == SbxOBJECT
     && refVar1->GetObject() == refVar2->GetObject() );
    SbxVariable* pRes = new SbxVariable;
    pRes->PutBool( bRes );
    PushVar( pRes );
}

// Aktualisieren des Wertes von TOS

void SbiRuntime::StepGET()
{
    SbxVariable* p = GetTOS();
    p->Broadcast( SBX_HINT_DATAWANTED );
}

// #67607 Uno-Structs kopieren
inline void checkUnoStructCopy( SbxVariableRef& refVal, SbxVariableRef& refVar )
{
    SbxDataType eVarType = refVar->GetType();
    if( eVarType == SbxOBJECT )
    {
        SbxObjectRef xVarObj = (SbxObject*)refVar->GetObject();
        SbxDataType eValType = refVal->GetType();
        if( eValType == SbxOBJECT && xVarObj == refVal->GetObject() )
        {
            SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*)xVarObj);
            if( pUnoObj )
            {
                Any aAny = pUnoObj->getUnoAny();
                if( aAny.getValueType().getTypeClass() == TypeClass_STRUCT )
                {
                    SbUnoObject* pNewUnoObj = new SbUnoObject( pUnoObj->GetName(), aAny );
                    // #70324: ClassName uebernehmen
                    pNewUnoObj->SetClassName( pUnoObj->GetClassName() );
                    refVar->PutObject( pNewUnoObj );
                }
            }
        }
    }
}

// Ablage von TOS in TOS-1

void SbiRuntime::StepPUT()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    // Store auf die eigene Methode (innerhalb einer Function)?
    BOOL bFlagsChanged = FALSE;
    USHORT n;
    if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
    {
        bFlagsChanged = TRUE;
        n = refVar->GetFlags();
        refVar->SetFlag( SBX_WRITE );
    }
    *refVar = *refVal;
    // #67607 Uno-Structs kopieren
    checkUnoStructCopy( refVal, refVar );
    if( bFlagsChanged )
        refVar->SetFlags( n );
}


// Speichern Objektvariable
// Nicht-Objekt-Variable fuehren zu Fehlern

void SbiRuntime::StepSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    // #67733 Typen mit Array-Flag sind auch ok
    SbxDataType eValType = refVal->GetType();
    SbxDataType eVarType = refVar->GetType();
    if( (eValType != SbxOBJECT && eValType != SbxEMPTY && !(eValType & SbxARRAY)) ||
        (eVarType != SbxOBJECT && !(eVarType & SbxARRAY) ) )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
    }
    else
    {
        // Auf refVal GetObject fuer Collections ausloesen
        SbxBase* pObjVarObj = refVal->GetObject();
        if( pObjVarObj )
        {
            SbxVariableRef refObjVal = PTR_CAST(SbxObject,pObjVarObj);

            // #67733 Typen mit Array-Flag sind auch ok
            if( refObjVal )
                refVal = refObjVal;
            else if( !(eValType & SbxARRAY) )
                refVal = NULL;
        }

        // #52896 Wenn Uno-Sequences bzw. allgemein Arrays einer als
        // Object deklarierten Variable zugewiesen werden, kann hier
        // refVal ungueltig sein!
        if( !refVal )
        {
            Error( SbERR_INVALID_USAGE_OBJECT );
        }
        else
        {
            // Store auf die eigene Methode (innerhalb einer Function)?
            BOOL bFlagsChanged = FALSE;
            USHORT n;
            if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
            {
                bFlagsChanged = TRUE;
                n = refVar->GetFlags();
                refVar->SetFlag( SBX_WRITE );
            }
            *refVar = *refVal;
            // #67607 Uno-Structs kopieren
            checkUnoStructCopy( refVal, refVar );
            if( bFlagsChanged )
                refVar->SetFlags( n );
        }
    }
}

// JSM 07.10.95
void SbiRuntime::StepLSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    if( refVar->GetType() != SbxSTRING
     || refVal->GetType() != SbxSTRING )
        Error( SbERR_INVALID_USAGE_OBJECT );
    else
    {
        // Store auf die eigene Methode (innerhalb einer Function)?
        USHORT n = refVar->GetFlags();
        if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
            refVar->SetFlag( SBX_WRITE );
        String aRefVarString = refVar->GetString();
        String aRefValString = refVal->GetString();

        if (aRefVarString.Len() > aRefValString.Len())
            aRefVarString.Fill(aRefVarString.Len(),' ');
        aRefVarString  = aRefValString.Copy( 0, aRefVarString.Len() );
        aRefVarString += aRefVarString.Copy( aRefValString.Len() );
        refVar->PutString(aRefVarString);

        refVar->SetFlags( n );
    }
}

// JSM 07.10.95
void SbiRuntime::StepRSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    if( refVar->GetType() != SbxSTRING
     || refVal->GetType() != SbxSTRING )
        Error( SbERR_INVALID_USAGE_OBJECT );
    else
    {
        // Store auf die eigene Methode (innerhalb einer Function)?
        USHORT n = refVar->GetFlags();
        if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
            refVar->SetFlag( SBX_WRITE );
        String aRefVarString = refVar->GetString();
        String aRefValString = refVal->GetString();

        USHORT nPos = 0;
        if (aRefVarString.Len() > aRefValString.Len())
        {
            aRefVarString.Fill(aRefVarString.Len(),' ');
            nPos = aRefVarString.Len() - aRefValString.Len();
        }
        aRefVarString  = aRefVarString.Copy( 0, nPos );
        aRefVarString += aRefValString.Copy( 0, aRefVarString.Len() - nPos );
        refVar->PutString(aRefVarString);

        refVar->SetFlags( n );
    }
}

// Ablage von TOS in TOS-1, dann ReadOnly-Bit setzen

void SbiRuntime::StepPUTC()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    refVar->SetFlag( SBX_WRITE );
    *refVar = *refVal;
    refVar->ResetFlag( SBX_WRITE );
    refVar->SetFlag( SBX_CONST );
}

// DIM
// TOS = Variable fuer das Array mit Dimensionsangaben als Parameter

void SbiRuntime::StepDIM()
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );
}

// #56204 DIM-Funktionalitaet in Hilfsmethode auslagern (step0.cxx)
void SbiRuntime::DimImpl( SbxVariableRef refVar )
{
    SbxArray* pDims = refVar->GetParameters();
    // Muss eine gerade Anzahl Argumente haben
    // Man denke daran, dass Arg[0] nicht zaehlt!
    if( pDims && !( pDims->Count() & 1 ) )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxDataType eType = refVar->IsFixed() ? refVar->GetType() : SbxVARIANT;
        SbxDimArray* pArray = new SbxDimArray( eType );
        // AB 2.4.1996, auch Arrays ohne Dimensionsangaben zulassen (VB-komp.)
        if( pDims )
        {
            for( USHORT i = 1; i < pDims->Count(); )
            {
                INT16 lb = pDims->Get( i++ )->GetInteger();
                INT16 ub = pDims->Get( i++ )->GetInteger();
                if( ub < lb )
                    Error( SbERR_OUT_OF_RANGE ), ub = lb;
                pArray->AddDim( lb, ub );
            }
        }
        else
        {
            // #62867 Beim Anlegen eines Arrays der Laenge 0 wie bei
            // Uno-Sequences der Laenge 0 eine Dimension anlegen
            pArray->unoAddDim( 0, -1 );
        }
        USHORT nFlags = refVar->GetFlags();
        refVar->ResetFlag( SBX_FIXED );
        refVar->PutObject( pArray );
        refVar->SetFlags( nFlags );
        refVar->SetParameters( NULL );
    }
}


// REDIM
// TOS  = Variable fuer das Array
// argv = Dimensionsangaben

void SbiRuntime::StepREDIM()
{
    // Im Moment ist es nichts anderes als Dim, da doppeltes Dim
    // bereits vom Compiler erkannt wird.
    StepDIM();
}

// REDIM PRESERVE
// TOS  = Variable fuer das Array
// argv = Dimensionsangaben

void SbiRuntime::StepREDIMP()
{
    StarBASIC::FatalError( SbERR_NOT_IMPLEMENTED );
}

// Variable loeschen
// TOS = Variable

void SbiRuntime::StepERASE()
{
    SbxVariableRef refVar = PopVar();
    SbxDataType eType = refVar->GetType();
    if( eType & SbxARRAY )
    {
        // AB 2.4.1996
        // Arrays haben bei Erase nach VB ein recht komplexes Verhalten. Hier
        // werden zunaechst nur die Typ-Probleme bei REDIM (#26295) beseitigt:
        // Typ hart auf den Array-Typ setzen, da eine Variable mit Array
        // SbxOBJECT ist. Bei REDIM entsteht dann ein SbxOBJECT-Array und
        // der ursruengliche Typ geht verloren -> Laufzeitfehler
        USHORT nFlags = refVar->GetFlags();
        refVar->ResetFlag( SBX_FIXED );
        refVar->SetType( SbxDataType(eType & 0x0FFF) );
        refVar->SetFlags( nFlags );
        refVar->Clear();
    }
    else
    if( refVar->IsFixed() )
        refVar->Clear();
    else
        refVar->SetType( SbxEMPTY );
}

// Einrichten eines Argvs
// nOp1 bleibt so -> 1. Element ist Returnwert

void SbiRuntime::StepARGC()
{
    PushArgv();
    refArgv = new SbxArray;
    nArgc = 1;
}

// Speichern eines Arguments in Argv

void SbiRuntime::StepARGV()
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        SbxVariableRef pVal = PopVar();
        if( pVal->ISA(SbxMethod) || pVal->ISA(SbxProperty) )
        {
            // Methoden und Properties evaluieren!
            SbxVariable* pRes = new SbxVariable( *pVal );
            pVal = pRes;
        }
        refArgv->Put( pVal, nArgc++ );
    }
}

// Input to Variable. Die Variable ist auf TOS und wird
// anschliessend entfernt.

void SbiRuntime::StepINPUT()
{
    String s;
    char ch;
    SbError err;
    // Skip whitespace
    while( ( err = pIosys->GetError() ) == 0 )
    {
        ch = pIosys->Read();
        if( ch != ' ' && ch != '\t' && ch != '\n' )
            break;
    }
    if( !err )
    {
        // Scan until comma or whitespace
        char sep = ( ch == '"' ) ? ch : 0;
        if( sep ) ch = pIosys->Read();
        while( ( err = pIosys->GetError() ) == 0 )
        {
            if( ch == sep )
            {
                ch = pIosys->Read();
                if( ch != sep )
                    break;
            }
            else if( !sep && (ch == ',' || ch == '\n') )
                break;
            s += ch;
            ch = pIosys->Read();
        }
        // skip whitespace
        if( ch == ' ' || ch == '\t' )
          while( ( err = pIosys->GetError() ) == 0 )
        {
            if( ch != ' ' && ch != '\t' && ch != '\n' )
                break;
            ch = pIosys->Read();
        }
    }
    if( !err )
    {
        SbxVariableRef pVar = GetTOS();
        // Zuerst versuchen, die Variable mit einem numerischen Wert
        // zu fuellen, dann mit einem Stringwert
        BOOL bSet = FALSE;
        if( !pVar->IsFixed() || pVar->IsNumeric() )
        {
            USHORT nLen = 0;
            if( !pVar->Scan( s, &nLen ) )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
            }
            // Der Wert muss komplett eingescant werden
            else if( nLen != s.Len() && !pVar->PutString( s ) )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
            }
            else if( nLen != s.Len() && pVar->IsNumeric() )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
                if( !err )
                    err = SbERR_CONVERSION;
            }
        }
        else
        {
            pVar->PutString( s );
            err = SbxBase::GetError();
            SbxBase::ResetError();
        }
    }
    if( err == SbERR_USER_ABORT )
        Error( err );
    else if( err )
    {
        if( pRestart && !pIosys->GetChannel() )
        {
            BasicResId aId( IDS_SBERR_START + 4 );
            String aMsg( aId );
            ErrorBox( NULL, WB_OK, aMsg ).Execute();
            pCode = pRestart;
        }
        else
            Error( err );
    }
    else
    {
        // pIosys->ResetChannel();
        PopVar();
    }
}

// Line Input to Variable. Die Variable ist auf TOS und wird
// anschliessend entfernt.

void SbiRuntime::StepLINPUT()
{
    ByteString aInput;
    pIosys->Read( aInput );
    Error( pIosys->GetError() );
    SbxVariableRef p = PopVar();
    p->PutString( String( aInput, gsl_getSystemTextEncoding() ) );
    // pIosys->ResetChannel();
}

// Programmende

void SbiRuntime::StepSTOP()
{
    pInst->Stop();
}

// FOR-Variable initialisieren

void SbiRuntime::StepINITFOR()
{
    PushFor();
}

// FOR-Variable inkrementieren

void SbiRuntime::StepNEXT()
{
    if( !pForStk )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
        pForStk->refVar->Compute( SbxPLUS, *pForStk->refInc );
}

// Anfang CASE: TOS in CASE-Stack

void SbiRuntime::StepCASE()
{
    if( !refCaseStk.Is() )
        refCaseStk = new SbxArray;
    SbxVariableRef xVar = PopVar();
    refCaseStk->Put( xVar, refCaseStk->Count() );
}

// Ende CASE: Variable freigeben

void SbiRuntime::StepENDCASE()
{
    if( !refCaseStk || !refCaseStk->Count() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
        refCaseStk->Remove( refCaseStk->Count() - 1 );
}

// Standard-Fehlerbehandlung

void SbiRuntime::StepSTDERROR()
{
    pError = NULL; bError = TRUE;
    pInst->aErrorMsg = String();
    pInst->nErr = 0L;
    pInst->nErl = 0;
    nError = 0L;
}

void SbiRuntime::StepNOERROR()
{
    pInst->aErrorMsg = String();
    pInst->nErr = 0L;
    pInst->nErl = 0;
    nError = 0L;
    bError = FALSE;
}

// UP verlassen

void SbiRuntime::StepLEAVE()
{
    bRun = FALSE;
}

void SbiRuntime::StepCHANNEL()      // TOS = Kanalnummer
{
    SbxVariableRef pChan = PopVar();
    short nChan = pChan->GetInteger();
    pIosys->SetChannel( nChan );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepCHANNEL0()
{
    pIosys->ResetChannel();
}

void SbiRuntime::StepPRINT()        // print TOS
{
    SbxVariableRef p = PopVar();
    String s1 = p->GetString();
    String s;
    if( p->GetType() >= SbxINTEGER && p->GetType() <= SbxDOUBLE )
        s = ' ';    // ein Blank davor
    s += s1;
    ByteString aByteStr( s, gsl_getSystemTextEncoding() );
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepPRINTF()       // print TOS in field
{
    SbxVariableRef p = PopVar();
    String s1 = p->GetString();
    String s;
    if( p->GetType() >= SbxINTEGER && p->GetType() <= SbxDOUBLE )
        s = ' ';    // ein Blank davor
    s += s1;
    s.Expand( 14, ' ' );
    ByteString aByteStr( s, gsl_getSystemTextEncoding() );
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepWRITE()        // write TOS
{
    SbxVariableRef p = PopVar();
    // Muss der String gekapselt werden?
    char ch = 0;
    switch (p->GetType() )
    {
        case SbxSTRING: ch = '"'; break;
        case SbxCURRENCY:
        case SbxBOOL:
        case SbxDATE: ch = '#'; break;
    }
    String s;
    if( ch )
        s += ch;
    s += p->GetString();
    if( ch )
        s += ch;
    ByteString aByteStr( s, gsl_getSystemTextEncoding() );
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepRENAME()       // Rename Tos+1 to Tos
{
    SbxVariableRef pTos1 = PopVar();
    SbxVariableRef pTos  = PopVar();
    String aDest = pTos1->GetString();
    String aSource = pTos->GetString();

    // <-- UCB
    if( hasUno() )
    {
        implStepRenameUCB( aSource, aDest );
    }
    else
    // --> UCB
    {
        DirEntry aSourceDirEntry( aSource );
        if( aSourceDirEntry.Exists() )
        {
            if( aSourceDirEntry.MoveTo( DirEntry(aDest) ) != FSYS_ERR_OK )
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
        }
        else
                StarBASIC::Error( SbERR_PATH_NOT_FOUND );
    }
}

// TOS = Prompt

void SbiRuntime::StepPROMPT()
{
    SbxVariableRef p = PopVar();
    ByteString aStr( p->GetString(), gsl_getSystemTextEncoding() );
    pIosys->SetPrompt( aStr );
}

// Set Restart point

void SbiRuntime::StepRESTART()
{
    pRestart = pCode;
}

// Leerer Ausdruck auf Stack fuer fehlenden Parameter

void SbiRuntime::StepEMPTY()
{
    // #57915 Die Semantik von StepEMPTY() ist die Repraesentation eines fehlenden
    // Arguments. Dies wird in VB durch ein durch den Wert 448 (SbERR_NAMED_NOT_FOUND)
    // vom Typ Error repraesentiert. StepEmpty jetzt muesste besser StepMISSING()
    // heissen, aber der Name wird der Einfachkeit halber beibehalten.
    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
    xVar->PutErr( 448 );
    PushVar( xVar );
    // ALT: PushVar( new SbxVariable( SbxEMPTY ) );
}

// TOS = Fehlercode

void SbiRuntime::StepERROR()
{
    SbxVariableRef refCode = PopVar();
    ULONG n = refCode->GetLong();
    Error( n );
}

