/*************************************************************************
 *
 *  $RCSfile: sb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
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

#include <stdio.h>      // sprintf()

#pragma hdrstop
#include "sb.hxx"
#ifdef VCL
#include <vcl/rcid.h>
#include <vcl/config.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _TOOLS_RC_HXX //autogen
#include <tools/rc.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include "sbunoobj.hxx"
#include "sbjsmeth.hxx"
#include "sbjsmod.hxx"
#include "sbintern.hxx"
#include "disas.hxx"
#include "runtime.hxx"
#include "sbuno.hxx"
#include "stdobj.hxx"
#include "filefmt.hxx"
#include "sb.hrc"
#include <basrid.hxx>

#pragma SW_SEGMENT_CLASS( SBASIC, SBASIC_CODE )

SV_IMPL_VARARR(SbTextPortions,SbTextPortion)

TYPEINIT1(StarBASIC,SbxObject)

#define RTLNAME "@SBRTL"


//========================================================================
// Array zur Umrechnung SFX <-> VB-Fehlercodes anlegen

struct SFX_VB_ErrorItem
{
    USHORT  nErrorVB;
    SbError nErrorSFX;
};

const SFX_VB_ErrorItem __FAR_DATA SFX_VB_ErrorTab[] =
{
    { 2, SbERR_SYNTAX },
    { 3, SbERR_NO_GOSUB },
    { 4, SbERR_REDO_FROM_START },
    { 5, SbERR_BAD_ARGUMENT },
    { 6, SbERR_MATH_OVERFLOW },
    { 7, SbERR_NO_MEMORY },
    { 8, SbERR_ALREADY_DIM },
    { 9, SbERR_OUT_OF_RANGE },
    { 10, SbERR_DUPLICATE_DEF },
    { 11, SbERR_ZERODIV },
    { 12, SbERR_VAR_UNDEFINED },
    { 13, SbERR_CONVERSION },
    { 14, SbERR_BAD_PARAMETER },
    { 18, SbERR_USER_ABORT },
    { 20, SbERR_BAD_RESUME },
    { 28, SbERR_STACK_OVERFLOW },
    { 35, SbERR_PROC_UNDEFINED },
    { 48, SbERR_BAD_DLL_LOAD },
    { 49, SbERR_BAD_DLL_CALL },
    { 51, SbERR_INTERNAL_ERROR },
    { 52, SbERR_BAD_CHANNEL },
    { 53, SbERR_FILE_NOT_FOUND },
    { 54, SbERR_BAD_FILE_MODE },
    { 55, SbERR_FILE_ALREADY_OPEN },
    { 57, SbERR_IO_ERROR },
    { 58, SbERR_FILE_EXISTS },
    { 59, SbERR_BAD_RECORD_LENGTH },
    { 61, SbERR_DISK_FULL },
    { 62, SbERR_READ_PAST_EOF },
    { 63, SbERR_BAD_RECORD_NUMBER },
    { 67, SbERR_TOO_MANY_FILES },
    { 68, SbERR_NO_DEVICE },
    { 70, SbERR_ACCESS_DENIED },
    { 71, SbERR_NOT_READY },
    { 73, SbERR_NOT_IMPLEMENTED },
    { 74, SbERR_DIFFERENT_DRIVE },
    { 75, SbERR_ACCESS_ERROR },
    { 76, SbERR_PATH_NOT_FOUND },
    { 91, SbERR_NO_OBJECT },
    { 93, SbERR_BAD_PATTERN },
    { 94, SBERR_IS_NULL },
    { 250, SbERR_DDE_ERROR },
    { 280, SbERR_DDE_WAITINGACK },
    { 281, SbERR_DDE_OUTOFCHANNELS },
    { 282, SbERR_DDE_NO_RESPONSE },
    { 283, SbERR_DDE_MULT_RESPONSES },
    { 284, SbERR_DDE_CHANNEL_LOCKED },
    { 285, SbERR_DDE_NOTPROCESSED },
    { 286, SbERR_DDE_TIMEOUT },
    { 287, SbERR_DDE_USER_INTERRUPT },
    { 288, SbERR_DDE_BUSY },
    { 289, SbERR_DDE_NO_DATA },
    { 290, SbERR_DDE_WRONG_DATA_FORMAT },
    { 291, SbERR_DDE_PARTNER_QUIT },
    { 292, SbERR_DDE_CONV_CLOSED },
    { 293, SbERR_DDE_NO_CHANNEL },
    { 294, SbERR_DDE_INVALID_LINK },
    { 295, SbERR_DDE_QUEUE_OVERFLOW },
    { 296, SbERR_DDE_LINK_ALREADY_EST },
    { 297, SbERR_DDE_LINK_INV_TOPIC },
    { 298, SbERR_DDE_DLL_NOT_FOUND },
    { 323, SbERR_CANNOT_LOAD },
    { 341, SbERR_BAD_INDEX },
    { 366, SbERR_NO_ACTIVE_OBJECT },
    { 380, SbERR_BAD_PROP_VALUE },
    { 382, SbERR_PROP_READONLY },
    { 394, SbERR_PROP_WRITEONLY },
    { 420, SbERR_INVALID_OBJECT },
    { 423, SbERR_NO_METHOD },
    { 424, SbERR_NEEDS_OBJECT },
    { 425, SbERR_INVALID_USAGE_OBJECT },
    { 430, SbERR_NO_OLE },
    { 438, SbERR_BAD_METHOD },
    { 440, SbERR_OLE_ERROR },
    { 445, SbERR_BAD_ACTION },
    { 446, SbERR_NO_NAMED_ARGS },
    { 447, SbERR_BAD_LOCALE },
    { 448, SbERR_NAMED_NOT_FOUND },
    { 449, SbERR_NOT_OPTIONAL },
    { 450, SbERR_WRONG_ARGS },
    { 451, SbERR_NOT_A_COLL },
    { 452, SbERR_BAD_ORDINAL },
    { 453, SbERR_DLLPROC_NOT_FOUND },
    { 460, SbERR_BAD_CLIPBD_FORMAT },
    { 951, SbERR_UNEXPECTED },
    { 952, SbERR_EXPECTED },
    { 953, SbERR_SYMBOL_EXPECTED },
    { 954, SbERR_VAR_EXPECTED },
    { 955, SbERR_LABEL_EXPECTED },
    { 956, SbERR_LVALUE_EXPECTED },
    { 957, SbERR_VAR_DEFINED },
    { 958, SbERR_PROC_DEFINED },
    { 959, SbERR_LABEL_DEFINED },
    { 960, SbERR_UNDEF_VAR },
    { 961, SbERR_UNDEF_ARRAY },
    { 962, SbERR_UNDEF_PROC },
    { 963, SbERR_UNDEF_LABEL },
    { 964, SbERR_UNDEF_TYPE },
    { 965, SbERR_BAD_EXIT },
    { 966, SbERR_BAD_BLOCK },
    { 967, SbERR_BAD_BRACKETS },
    { 968, SbERR_BAD_DECLARATION },
    { 969, SbERR_BAD_PARAMETERS },
    { 970, SbERR_BAD_CHAR_IN_NUMBER },
    { 971, SbERR_MUST_HAVE_DIMS },
    { 972, SbERR_NO_IF },
    { 973, SbERR_NOT_IN_SUBR },
    { 974, SbERR_NOT_IN_MAIN },
    { 975, SbERR_WRONG_DIMS },
    { 976, SbERR_BAD_OPTION },
    { 977, SbERR_CONSTANT_REDECLARED },
    { 978, SbERR_PROG_TOO_LARGE },
    { 979, SbERR_NO_STRINGS_ARRAYS },
    { 1000, SbERR_PROPERTY_NOT_FOUND },
    { 1001, SbERR_METHOD_NOT_FOUND },
    { 1002, SbERR_ARG_MISSING },
    { 1003, SbERR_BAD_NUMBER_OF_ARGS },
    { 1004, SbERR_METHOD_FAILED },
    { 1005, SbERR_SETPROP_FAILED },
    { 1006, SbERR_GETPROP_FAILED },
    { 0xFFFF, 0xFFFFFFFFL }     // End-Marke
};

////////////////////////////////////////////////////////////////////////////

// Die StarBASIC-Factory hat einen Hack. Wenn ein SbModule eingerichtet wird,
// wird der Pointer gespeichert und an nachfolgende SbProperties/SbMethods
// uebergeben. Dadurch wird die Modul-Relationship wiederhergestellt. Das
// klappt aber nur, wenn ein Modul geladen wird. Fuer getrennt geladene
// Properties kann es Probleme geben!

SbxBase* SbiFactory::Create( UINT16 nSbxId, UINT32 nCreator )
{
    if( nCreator ==  SBXCR_SBX )
    {
        String aEmpty;
        switch( nSbxId )
        {
            case SBXID_BASIC:
                return new StarBASIC( NULL );
            case SBXID_BASICMOD:
                return new SbModule( aEmpty );
            case SBXID_BASICPROP:
                return new SbProperty( aEmpty, SbxVARIANT, NULL );
            case SBXID_BASICMETHOD:
                return new SbMethod( aEmpty, SbxVARIANT, NULL );
            case SBXID_JSCRIPTMOD:
                return new SbJScriptModule( aEmpty );
            case SBXID_JSCRIPTMETH:
                return new SbJScriptMethod( aEmpty, SbxVARIANT, NULL );
        }
    }
    return NULL;
}

SbxObject* SbiFactory::CreateObject( const String& rClass )
{
    if( rClass.EqualsIgnoreCaseAscii( "StarBASIC" ) )
        return new StarBASIC( NULL );
    else
    if( rClass.EqualsIgnoreCaseAscii( "StarBASICModule" ) )
    {
        String aEmpty;
        return new SbModule( aEmpty );
    }
    else
        return NULL;
}

////////////////////////////////////////////////////////////////////////////

StarBASIC::StarBASIC( StarBASIC* p )
    : SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("StarBASIC") ) )
{
    SetParent( p );
    pLibInfo = NULL;
    bNoRtl = bBreak = FALSE;
    pModules = new SbxArray;

#ifdef DBG_UTIL
    Config LangConfig( String( RTL_CONSTASCII_USTRINGPARAM("d:\\LANGUAGE.INI") ) );
    LangConfig.SetGroup( "main" );
    ByteString aStr = LangConfig.ReadKey( "language","basic" );
    if( aStr == "vbscript" )
        SetGlobalLanguageMode( SB_LANG_VBSCRIPT );
    else
    if( aStr == "javascript" )
        SetGlobalLanguageMode( SB_LANG_JAVASCRIPT );
#endif

    if( !GetSbData()->nInst++ )
    {
        pSBFAC = new SbiFactory;
        AddFactory( pSBFAC );
        pUNOFAC = new SbUnoFactory;
        AddFactory( pUNOFAC );
    }
    pRtl = new SbiStdObject( String( RTL_CONSTASCII_USTRINGPARAM(RTLNAME) ), this );
    // Suche ueber StarBASIC ist immer global
    SetFlag( SBX_GBLSEARCH );
}

// #51727 SetModified ueberladen, damit der Modified-
// Zustand nicht an den Parent weitergegeben wird.
void StarBASIC::SetModified( BOOL b )
{
    SbxBase::SetModified( b );
}

//***

StarBASIC::~StarBASIC()
{
    if( !--GetSbData()->nInst )
    {
        RemoveFactory( pSBFAC );
        pSBFAC = NULL;
        RemoveFactory( pUNOFAC );
        pUNOFAC = NULL;

#ifdef DBG_UTIL
    // SbiData braucht am Programm-Ende nicht abgeraeumt werden,
    // aber wir wollen keine MLK's beim Purify
    // Wo sollte es sonst geschehen???
    SbiGlobals** pp = (SbiGlobals**) ::GetAppData( SHL_SBC );
    SbiGlobals* p = *pp;
    if( p )
    {
        delete p;
        *pp = 0;
    }
#endif
    }
}

// operator new() wird hier versenkt, damit jeder eine Instanz
// anlegen kann, ohne neu zu bilden.

void* StarBASIC::operator new( size_t n )
{
    if( n < sizeof( StarBASIC ) )
    {
//      DBG_ASSERT( FALSE, "Warnung: inkompatibler BASIC-Stand!" );
        n = sizeof( StarBASIC );
    }
    return ::operator new( n );
}

void StarBASIC::operator delete( void* p )
{
    ::operator delete( p );
}

/**************************************************************************
*
*   Erzeugen/Verwalten von Modulen
*
**************************************************************************/

SbModule* StarBASIC::MakeModule( const String& rName, const String& rSrc )
{
    SbModule* p = new SbModule( rName );
    p->SetSource( rSrc );
    p->SetParent( this );
    pModules->Insert( p, pModules->Count() );
    SetModified( TRUE );
    return p;
}

void StarBASIC::Insert( SbxVariable* pVar )
{
    if( pVar->IsA( TYPE(SbModule) ) )
    {
        pModules->Insert( pVar, pModules->Count() );
        pVar->SetParent( this );
        StartListening( pVar->GetBroadcaster(), TRUE );
    }
    else
    {
        BOOL bWasModified = IsModified();
        SbxObject::Insert( pVar );
        if( !bWasModified && pVar->IsSet( SBX_DONTSTORE ) )
            SetModified( FALSE );
    }
}

void StarBASIC::Remove( SbxVariable* pVar )
{
    if( pVar->IsA( TYPE(SbModule) ) )
    {
        pModules->Remove( pVar );
        pVar->SetParent( 0 );
        EndListening( pVar->GetBroadcaster() );
    }
    else
        SbxObject::Remove( pVar );
}

BOOL StarBASIC::Compile( SbModule* pMod )
{
    return pMod ? pMod->Compile() : FALSE;
}

BOOL StarBASIC::Disassemble( SbModule* pMod, String& rText )
{
    rText.Erase();
    if( pMod )
        pMod->Disassemble( rText );
    return BOOL( rText.Len() != 0 );
}

void StarBASIC::Clear()
{
    while( pModules->Count() )
        pModules->Remove( pModules->Count() - 1 );
}

SbModule* StarBASIC::FindModule( const String& rName )
{
    for( USHORT i = 0; i < pModules->Count(); i++ )
    {
        SbModule* p = (SbModule*) pModules->Get( i );
        if( p->GetName().EqualsIgnoreCaseAscii( rName ) )
            return p;
    }
    return NULL;
}

// Init-Code aller Module ausfuehren (auch in inserteten Bibliotheken)
void StarBASIC::InitAllModules( void )
{
    // Eigene Module initialisieren
    for ( USHORT nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = (SbModule*)pModules->Get( nMod );
        if( !pModule->IsCompiled() )
            pModule->Compile();
        pModule->RunInit();
    }
    // Alle Objekte ueberpruefen, ob es sich um ein Basic handelt
    // Wenn ja, auch dort initialisieren
    for ( USHORT nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
        if( pBasic )
            pBasic->InitAllModules();
    }
}

// #43011 Fuer das TestTool, um globale Variablen loeschen zu koennen
void StarBASIC::ClearGlobalVars( void )
{
    SbxArrayRef xProps( GetProperties() );
    USHORT nPropCount = xProps->Count();
    for ( USHORT nProp = 0 ; nProp < nPropCount ; ++nProp )
    {
        SbxBase* pVar = xProps->Get( nProp );
        pVar->Clear();
    }
    SetModified( TRUE );
}


// Diese Implementation sucht erst innerhalb der Runtime-Library, dann
// nach einem Element innerhalb eines Moduls. Dieses Element kann eine
// Public-Variable oder ein Entrypoint sein. Wenn nicht gefunden, wird,
// falls nach einer Methode gesucht wird und ein Modul mit dem angege-
// benen Namen gefunden wurde, der Entrypoint "Main" gesucht. Wenn das
// auch nicht klappt, laeuft die traditionelle Suche ueber Objekte an.

SbxVariable* StarBASIC::Find( const String& rName, SbxClassType t )
{
    SbxVariable* pRes = NULL;
    SbModule* pNamed = NULL;
    // "Extended" search in Runtime Lib
    // aber nur, wenn SbiRuntime nicht das Flag gesetzt hat
    if( !bNoRtl )
    {
        if( t == SbxCLASS_DONTCARE || t == SbxCLASS_OBJECT )
        {
            if( rName.EqualsIgnoreCaseAscii( RTLNAME ) )
                pRes = pRtl;
        }
        if( !pRes )
            pRes = ((SbiStdObject*) (SbxObject*) pRtl)->Find( rName, t );
        if( pRes )
            pRes->SetFlag( SBX_EXTFOUND );
    }
    // Module durchsuchen
    if( !pRes )
      for( USHORT i = 0; i < pModules->Count(); i++ )
    {
        SbModule* p = (SbModule*) pModules->Get( i );
        if( p->IsVisible() )
        {
            // Modul merken fuer Main()-Aufruf
            // oder stimmt etwa der Name ueberein?!?
            if( p->GetName().EqualsIgnoreCaseAscii( rName ) )
            {
                if( t == SbxCLASS_OBJECT || t == SbxCLASS_DONTCARE )
                {
                    pRes = p; break;
                }
                pNamed = p;
            }
            // Sonst testen, ob das Element vorhanden ist
            // GBLSEARCH-Flag rausnehmen (wg. Rekursion)
            USHORT nGblFlag = p->GetFlags() & SBX_GBLSEARCH;
            p->ResetFlag( SBX_GBLSEARCH );
            pRes = p->Find( rName, t );
            p->SetFlag( nGblFlag );
            if( pRes )
                break;
        }
    }
    if( !pRes && pNamed && ( t == SbxCLASS_METHOD || t == SbxCLASS_DONTCARE ) )
        pRes = pNamed->Find( String( RTL_CONSTASCII_USTRINGPARAM("Main") ), SbxCLASS_METHOD );
    if( !pRes )
        pRes = SbxObject::Find( rName, t );
    return pRes;
}

BOOL StarBASIC::Call( const String& rName, SbxArray* pParam )
{
    BOOL bRes = SbxObject::Call( rName, pParam );
    if( !bRes )
    {
        SbxError eErr = SbxBase::GetError();
        SbxBase::ResetError();
        if( eErr != SbxERR_OK )
            RTError( (SbError)eErr, 0, 0, 0 );
    }
    return bRes;
}

// Find-Funktion ueber Name (z.B. Abfrage aus BASIC-IDE)
SbxBase* StarBASIC::FindSBXInCurrentScope( const String& rName )
{
    if( !pINST )
        return NULL;
    if( !pINST->pRun )
        return NULL;
    return pINST->pRun->FindElementExtern( rName );
}

// Alte Schnittstelle vorerst erhalten
SbxVariable* StarBASIC::FindVarInCurrentScopy
( const String& rName, USHORT& rStatus )
{
    rStatus = 1;            // Annahme: Nichts gefunden
    SbxVariable* pVar = NULL;
    SbxBase* pSbx = FindSBXInCurrentScope( rName );
    if( pSbx )
    {
        if( !pSbx->ISA(SbxMethod) && !pSbx->ISA(SbxObject) )
            pVar = PTR_CAST(SbxVariable,pSbx);
    }
    if( pVar )
        rStatus = 0;        // doch gefunden
    return pVar;
}

void StarBASIC::Stop()
{
    SbiInstance* p = pINST;
    while( p )
    {
        p->Stop();
        p = p->pNext;
    }
}

BOOL StarBASIC::IsRunning()
{
    return BOOL( pINST != NULL );
}

/**************************************************************************
*
*   Objekt-Factories etc.
*
**************************************************************************/

// Aktivierung eines Objekts. Aktive Objekte muessen nicht mehr
// von BASIC aus ueber den Namen angesprochen werden. Ist
// NULL angegeben, wird alles aktiviert.

void StarBASIC::ActivateObject( const String* pName, BOOL bActivate )
{
    if( pName )
    {
        SbxObject* p = (SbxObject*) SbxObject::Find( *pName, SbxCLASS_OBJECT );
        if( p )
            if( bActivate )
                p->SetFlag( SBX_EXTSEARCH );
            else
                p->ResetFlag( SBX_EXTSEARCH );
    }
    else
    {
        for( USHORT i = 0; i < GetObjects()->Count(); i++ )
        {
            SbxObject* p = (SbxObject*) GetObjects()->Get( i );
            if( bActivate )
                p->SetFlag( SBX_EXTSEARCH );
            else
                p->ResetFlag( SBX_EXTSEARCH );
        }
    }
}

/**************************************************************************
*
*   Debugging und Fehlerbehandlung
*
**************************************************************************/

SbMethod* StarBASIC::GetActiveMethod( USHORT nLevel )
{
    if( pINST )
        return pINST->GetCaller( nLevel );
    else
        return NULL;
}

SbModule* StarBASIC::GetActiveModule()
{
    if( pINST && !IsCompilerError() )
        return pINST->GetActiveModule();
    else
        return pCMOD;
}

USHORT StarBASIC::BreakPoint( USHORT l, USHORT c1, USHORT c2 )
{
    SetErrorData( 0, l, c1, c2 );
    bBreak = TRUE;
    if( GetSbData()->aBreakHdl.IsSet() )
        return (USHORT) GetSbData()->aBreakHdl.Call( this );
    else
        return BreakHdl();
}

USHORT StarBASIC::StepPoint( USHORT l, USHORT c1, USHORT c2 )
{
    SetErrorData( 0, l, c1, c2 );
    bBreak = FALSE;
    if( GetSbData()->aBreakHdl.IsSet() )
        return (USHORT) GetSbData()->aBreakHdl.Call( this );
    else
        return BreakHdl();
}

USHORT __EXPORT StarBASIC::BreakHdl()
{
    return (USHORT) ( aBreakHdl.IsSet()
        ? aBreakHdl.Call( this ) : SbDEBUG_CONTINUE );
}

// Abfragen fuer den Error-Handler und den Break-Handler:
USHORT StarBASIC::GetLine()     { return GetSbData()->nLine; }
USHORT StarBASIC::GetCol1()     { return GetSbData()->nCol1; }
USHORT StarBASIC::GetCol2()     { return GetSbData()->nCol2; }

// Spezifisch fuer den Error-Handler:
SbError StarBASIC::GetErrorCode()       { return GetSbData()->nCode; }
const String& StarBASIC::GetErrorText() { return GetSbData()->aErrMsg; }
BOOL StarBASIC::IsCompilerError()       { return GetSbData()->bCompiler; }
void StarBASIC::SetGlobalLanguageMode( SbLanguageMode eLanguageMode )
{
    GetSbData()->eLanguageMode = eLanguageMode;
}
SbLanguageMode StarBASIC::GetGlobalLanguageMode()
{
    return GetSbData()->eLanguageMode;
}
// Lokale Einstellung
SbLanguageMode StarBASIC::GetLanguageMode()
{
    // Globale Einstellung nehmen?
    if( eLanguageMode == SB_LANG_GLOBAL )
        return GetSbData()->eLanguageMode;
    else
        return eLanguageMode;
}

// AB: 29.3.96
// Das Mapping zwischen alten und neuen Fehlercodes erfolgt, indem die Tabelle
// SFX_VB_ErrorTab[] durchsucht wird. Dies ist zwar nicht besonders performant,
// verbraucht aber viel weniger Speicher als entsprechende switch-Bloecke.
// Die Umrechnung von Fehlercodes muss nicht schnell sein, daher auch keine
// binaere Suche bei VB-Error -> SFX-Error.

// Neue Fehler-Codes auf alte, Sbx-Kompatible zurueckmappen
USHORT StarBASIC::GetVBErrorCode( SbError nError )
{
    USHORT nRet = 0;

    // Suchschleife
    const SFX_VB_ErrorItem* pErrItem;
    USHORT nIndex = 0;
    do
    {
        pErrItem = SFX_VB_ErrorTab + nIndex;
        if( pErrItem->nErrorSFX == nError )
        {
            nRet = pErrItem->nErrorVB;
            break;
        }
        nIndex++;
    }
    while( pErrItem->nErrorVB != 0xFFFF );      // bis End-Marke
    return nRet;
}

SbError StarBASIC::GetSfxFromVBError( USHORT nError )
{
    SbError nRet = 0L;

    // Suchschleife
    const SFX_VB_ErrorItem* pErrItem;
    USHORT nIndex = 0;
    do
    {
        pErrItem = SFX_VB_ErrorTab + nIndex;
        if( pErrItem->nErrorVB == nError )
        {
            nRet = pErrItem->nErrorSFX;
            break;
        }
        else if( pErrItem->nErrorVB > nError )
            break;              // kann nicht mehr gefunden werden

        nIndex++;
    }
    while( pErrItem->nErrorVB != 0xFFFF );      // bis End-Marke
    return nRet;
}

// Error- / Break-Daten setzen
void StarBASIC::SetErrorData
( SbError nCode, USHORT nLine, USHORT nCol1, USHORT nCol2 )
{
    SbiGlobals& aGlobals = *GetSbData();
    aGlobals.nCode = nCode;
    aGlobals.nLine = nLine;
    aGlobals.nCol1 = nCol1;
    aGlobals.nCol2 = nCol2;
}

//----------------------------------------------------------------
// Hilfsklasse zum Zugriff auf String SubResourcen einer Resource.
// Quelle: sfx2\source\doc\docfile.cxx (TLX)
struct BasicStringList_Impl : private Resource
{
    ResId aResId;

    BasicStringList_Impl( ResId& rErrIdP,  USHORT nId)
        : Resource( rErrIdP ),aResId(nId){}
    ~BasicStringList_Impl() { FreeResource(); }

    String GetString(){ return String( aResId ); }
    BOOL IsErrorTextAvailable( void )
        { return IsAvailableRes(aResId.SetRT(RSC_STRING)); }
};
//----------------------------------------------------------------

// #60175 Flag, das bei Basic-Fehlern das Anziehen der SFX-Resourcen verhindert
static BOOL bStaticSuppressSfxResource = FALSE;

void StarBASIC::StaticSuppressSfxResource( BOOL bSuppress )
{
    bStaticSuppressSfxResource = bSuppress;
}

void StarBASIC::MakeErrorText( SbError nId, const String& aMsg )
{
    if( bStaticSuppressSfxResource )
    {
        GetSbData()->aErrMsg = String( RTL_CONSTASCII_USTRINGPARAM("No resource: Error message not available") );
        return;
    }

    USHORT nOldID = GetVBErrorCode( nId );

    // Hilfsklasse instanzieren
    BasicResId aId( RID_BASIC_START );
    BasicStringList_Impl aMyStringList( aId, USHORT(nId & ERRCODE_RES_MASK) );

    if( aMyStringList.IsErrorTextAvailable() )
    {
        // Merge Message mit Zusatztext
        String aMsg1 = aMyStringList.GetString();
        // Argument-Platzhalter durch %s ersetzen
        String aSrgStr( RTL_CONSTASCII_USTRINGPARAM("$(ARG1)") );
        USHORT nResult = aMsg1.Search( aSrgStr );

        if( nResult != STRING_NOTFOUND )
        {
            aMsg1.Erase( nResult, aSrgStr.Len() );
            aMsg1.Insert( aMsg, nResult );
        }
        GetSbData()->aErrMsg = aMsg1;
    }
    else
    {
        String aStdMsg( RTL_CONSTASCII_USTRINGPARAM("Fehler ") );
        aStdMsg += String::CreateFromInt32( nOldID);
        aStdMsg += String( RTL_CONSTASCII_USTRINGPARAM(": Kein Fehlertext verfuegbar!") );
        GetSbData()->aErrMsg = aStdMsg;
    }
}

BOOL StarBASIC::CError
    ( SbError code, const String& rMsg, USHORT l, USHORT c1, USHORT c2 )
{
    // Compiler-Fehler waehrend der Laufzeit -> Programm anhalten
    if( IsRunning() )
        Stop();

    // #45741# Falls der Wait-Cursor gesetzt ist, jetzt zuruecksetzen
    if( GetSbData()->bCompWait )
    {
        Application::LeaveWait();
        GetSbData()->bCompWait = FALSE;
    }

    // Flag setzen, damit GlobalRunInit den Fehler mitbekommt
    GetSbData()->bGlobalInitErr = TRUE;

    // Fehlertext basteln
    MakeErrorText( code, rMsg );

    // Umsetzung des Codes fuer String-Transport in SFX-Error
    if( rMsg.Len() )
        code = (ULONG)*new StringErrorInfo( code, String(rMsg) );

    SetErrorData( code, l, c1, c2 );
    GetSbData()->bCompiler = TRUE;
    BOOL bRet;
    if( GetSbData()->aErrHdl.IsSet() )
        bRet = (BOOL) GetSbData()->aErrHdl.Call( this );
    else
        bRet = ErrorHdl();
    GetSbData()->bCompiler = FALSE;     // nur TRUE fuer Error-Handler
    return bRet;
}

BOOL StarBASIC::RTError
    ( SbError code, USHORT l, USHORT c1, USHORT c2 )
{
    return RTError( code, String(), l, c1, c2 );
}

BOOL StarBASIC::RTError( SbError code, const String& rMsg, USHORT l, USHORT c1, USHORT c2 )
{
    SbError c = code;
    if( (c & ERRCODE_CLASS_MASK) == ERRCODE_CLASS_COMPILER )
        c = 0;
    MakeErrorText( c, rMsg );

    // Umsetzung des Codes fuer String-Transport in SFX-Error
    if( rMsg.Len() )
        code = (ULONG)*new StringErrorInfo( code, String(rMsg) );

    SetErrorData( code, l, c1, c2 );
    if( GetSbData()->aErrHdl.IsSet() )
        return (BOOL) GetSbData()->aErrHdl.Call( this );
    else
        return ErrorHdl();
}

void StarBASIC::Error( SbError n )
{
    Error( n, String() );
}

void StarBASIC::Error( SbError n, const String& rMsg )
{
    if( pINST )
        pINST->Error( n, rMsg );
}

void StarBASIC::FatalError( SbError n )
{
    if( pINST )
        pINST->FatalError( n );
}

SbError StarBASIC::GetErr()
{
    if( pINST )
        return pINST->GetErr();
    else
        return 0;
}

// #66536 Zusatz-Message fuer RTL-Funktion Error zugreifbar machen
String StarBASIC::GetErrorMsg()
{
    if( pINST )
        return pINST->GetErrorMsg();
    else
        return String();
}

USHORT StarBASIC::GetErl()
{
    if( pINST )
        return pINST->GetErl();
    else
        return 0;
}

BOOL __EXPORT StarBASIC::ErrorHdl()
{
    return (BOOL) ( aErrorHdl.IsSet()
        ? aErrorHdl.Call( this ) : FALSE );
}

Link StarBASIC::GetGlobalErrorHdl()
{
    return GetSbData()->aErrHdl;
}

void StarBASIC::SetGlobalErrorHdl( const Link& rLink )
{
    GetSbData()->aErrHdl = rLink;
}


Link StarBASIC::GetGlobalBreakHdl()
{
    return GetSbData()->aBreakHdl;
}

void StarBASIC::SetGlobalBreakHdl( const Link& rLink )
{
    GetSbData()->aBreakHdl = rLink;
}

/**************************************************************************
*
*   Laden und Speichern
*
**************************************************************************/

BOOL StarBASIC::LoadData( SvStream& r, USHORT nVer )
{
    if( !SbxObject::LoadData( r, nVer ) )
        return FALSE;
    UINT16 nMod;
    pModules->Clear();
    r >> nMod;
    for( USHORT i = 0; i < nMod; i++ )
    {
        SbModule* pMod = (SbModule*) SbxBase::Load( r );
        if( !pMod )
            return FALSE;
        else if( pMod->ISA(SbJScriptModule) )
        {
            // Ref zuweisen, damit pMod deleted wird
            SbModuleRef xRef = pMod;
        }
        else
        {
            pMod->SetParent( this );
            pModules->Put( pMod, i );
        }
    }
    // HACK fuer SFX-Mist!
    SbxVariable* p = Find( String( RTL_CONSTASCII_USTRINGPARAM("FALSE") ), SbxCLASS_PROPERTY );
    if( p )
        Remove( p );
    p = Find( String( RTL_CONSTASCII_USTRINGPARAM("TRUE") ), SbxCLASS_PROPERTY );
    if( p )
        Remove( p );
    // Ende des Hacks!
    // Suche ueber StarBASIC ist immer global
    DBG_ASSERT( IsSet( SBX_GBLSEARCH ), "Basic ohne GBLSEARCH geladen" );
    SetFlag( SBX_GBLSEARCH );
    return TRUE;
}

BOOL StarBASIC::StoreData( SvStream& r ) const
{
    if( !SbxObject::StoreData( r ) )
        return FALSE;
    r << (UINT16) pModules->Count();
    for( USHORT i = 0; i < pModules->Count(); i++ )
    {
        SbModule* p = (SbModule*) pModules->Get( i );
        if( !p->Store( r ) )
            return FALSE;
    }
    return TRUE;
}

BOOL StarBASIC::LoadOldModules( SvStream& r )
{
    return FALSE;
}


