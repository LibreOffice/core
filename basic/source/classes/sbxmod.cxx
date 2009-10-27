/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sbxmod.cxx,v $
 * $Revision: 1.44.10.1 $
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

#include <list>

#include <vos/macros.hxx>
#include <vcl/svapp.hxx>
#include <tools/stream.hxx>
#include <svtools/brdcst.hxx>
#include <tools/shl.hxx>
#include <basic/sbx.hxx>
#include "sb.hxx"
#include <sbjsmeth.hxx>
#include "sbjsmod.hxx"
#include "sbintern.hxx"
#include "image.hxx"
#include "opcodes.hxx"
#include "runtime.hxx"
#include "token.hxx"
#include "sbunoobj.hxx"


//#include <basic/hilight.hxx>
#include <svtools/syntaxhighlight.hxx>

#include <basic/basrdll.hxx>
#include <vos/mutex.hxx>


// for the bsearch
#ifdef WNT
#define CDECL _cdecl
#endif
#if defined(UNX) || defined(OS2)
#define CDECL
#endif
#ifdef UNX
#include <sys/resource.h>
#endif

#include <stdio.h>


TYPEINIT1(SbModule,SbxObject)
TYPEINIT1(SbMethod,SbxMethod)
TYPEINIT1(SbProperty,SbxProperty)
TYPEINIT1(SbProcedureProperty,SbxProperty)
TYPEINIT1(SbJScriptModule,SbModule)
TYPEINIT1(SbJScriptMethod,SbMethod)

SV_DECL_VARARR(SbiBreakpoints,USHORT,4,4)
SV_IMPL_VARARR(SbiBreakpoints,USHORT)


SV_IMPL_VARARR(HighlightPortions, HighlightPortion)


/////////////////////////////////////////////////////////////////////////////

// Ein BASIC-Modul hat EXTSEARCH gesetzt, damit die im Modul enthaltenen
// Elemente von anderen Modulen aus gefunden werden koennen.

SbModule::SbModule( const String& rName )
         : SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("StarBASICModule") ) ),
           pImage( NULL ), pBreaks( NULL ), pClassData( NULL )
{
    SetName( rName );
    SetFlag( SBX_EXTSEARCH | SBX_GBLSEARCH );
}

SbModule::~SbModule()
{
    if( pImage )
        delete pImage;
    if( pBreaks )
        delete pBreaks;
    if( pClassData )
        delete pClassData;
}

BOOL SbModule::IsCompiled() const
{
    return BOOL( pImage != 0 );
}

const SbxObject* SbModule::FindType( String aTypeName ) const
{
    return pImage ? pImage->FindType( aTypeName ) : NULL;
}


// Aus dem Codegenerator: Loeschen des Images und Invalidieren der Entries

void SbModule::StartDefinitions()
{
    delete pImage; pImage = NULL;
    if( pClassData )
        pClassData->clear();

    // Methoden und Properties bleiben erhalten, sind jedoch ungueltig
    // schliesslich sind ja u.U. die Infos belegt
    USHORT i;
    for( i = 0; i < pMethods->Count(); i++ )
    {
        SbMethod* p = PTR_CAST(SbMethod,pMethods->Get( i ) );
        if( p )
            p->bInvalid = TRUE;
    }
    for( i = 0; i < pProps->Count(); )
    {
        SbProperty* p = PTR_CAST(SbProperty,pProps->Get( i ) );
        if( p )
            pProps->Remove( i );
        else
            i++;
    }
}

// Methode anfordern/anlegen

SbMethod* SbModule::GetMethod( const String& rName, SbxDataType t )
{
    SbxVariable* p = pMethods->Find( rName, SbxCLASS_METHOD );
    SbMethod* pMeth = p ? PTR_CAST(SbMethod,p) : NULL;
    if( p && !pMeth )
        pMethods->Remove( p );
    if( !pMeth )
    {
        pMeth = new SbMethod( rName, t, this );
        pMeth->SetParent( this );
        pMeth->SetFlags( SBX_READ );
        pMethods->Put( pMeth, pMethods->Count() );
        StartListening( pMeth->GetBroadcaster(), TRUE );
    }
    // Per Default ist die Methode GUELTIG, da sie auch vom Compiler
    // (Codegenerator) erzeugt werden kann
    pMeth->bInvalid = FALSE;
    pMeth->ResetFlag( SBX_FIXED );
    pMeth->SetFlag( SBX_WRITE );
    pMeth->SetType( t );
    pMeth->ResetFlag( SBX_WRITE );
    if( t != SbxVARIANT )
        pMeth->SetFlag( SBX_FIXED );
    return pMeth;
}

// Property anfordern/anlegen

SbProperty* SbModule::GetProperty( const String& rName, SbxDataType t )
{
    SbxVariable* p = pProps->Find( rName, SbxCLASS_PROPERTY );
    SbProperty* pProp = p ? PTR_CAST(SbProperty,p) : NULL;
    if( p && !pProp )
        pProps->Remove( p );
    if( !pProp )
    {
        pProp = new SbProperty( rName, t, this );
        pProp->SetFlag( SBX_READWRITE );
        pProp->SetParent( this );
        pProps->Put( pProp, pProps->Count() );
        StartListening( pProp->GetBroadcaster(), TRUE );
    }
    return pProp;
}

SbProcedureProperty* SbModule::GetProcedureProperty
    ( const String& rName, SbxDataType t )
{
    SbxVariable* p = pProps->Find( rName, SbxCLASS_PROPERTY );
    SbProcedureProperty* pProp = p ? PTR_CAST(SbProcedureProperty,p) : NULL;
    if( p && !pProp )
        pProps->Remove( p );
    if( !pProp )
    {
        pProp = new SbProcedureProperty( rName, t );
        pProp->SetFlag( SBX_READWRITE );
        pProp->SetParent( this );
        pProps->Put( pProp, pProps->Count() );
        StartListening( pProp->GetBroadcaster(), TRUE );
    }
    return pProp;
}

SbIfaceMapperMethod* SbModule::GetIfaceMapperMethod
    ( const String& rName, SbMethod* pImplMeth )
{
    SbxVariable* p = pMethods->Find( rName, SbxCLASS_METHOD );
    SbIfaceMapperMethod* pMapperMethod = p ? PTR_CAST(SbIfaceMapperMethod,p) : NULL;
    if( p && !pMapperMethod )
        pMethods->Remove( p );
    if( !pMapperMethod )
    {
        pMapperMethod = new SbIfaceMapperMethod( rName, pImplMeth );
        pMapperMethod->SetParent( this );
        pMapperMethod->SetFlags( SBX_READ );
        pMethods->Put( pMapperMethod, pMethods->Count() );
    }
    pMapperMethod->bInvalid = FALSE;
    return pMapperMethod;
}

SbIfaceMapperMethod::~SbIfaceMapperMethod()
{
}

TYPEINIT1(SbIfaceMapperMethod,SbMethod)


// Aus dem Codegenerator: Ungueltige Eintraege entfernen

void SbModule::EndDefinitions( BOOL bNewState )
{
    for( USHORT i = 0; i < pMethods->Count(); )
    {
        SbMethod* p = PTR_CAST(SbMethod,pMethods->Get( i ) );
        if( p )
        {
            if( p->bInvalid )
                pMethods->Remove( p );
            else
            {
                p->bInvalid = bNewState;
                i++;
            }
        }
        else
            i++;
    }
    SetModified( TRUE );
}

void SbModule::Clear()
{
    delete pImage; pImage = NULL;
    if( pClassData )
        pClassData->clear();
    SbxObject::Clear();
}


SbxVariable* SbModule::Find( const XubString& rName, SbxClassType t )
{
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( !pRes && pImage )
    {
        SbiInstance* pInst = pINST;
        if( pInst && pInst->IsCompatibility() )
        {
            // Put enum types as objects into module,
            // allows MyEnum.First notation
            SbxArrayRef xArray = pImage->GetEnums();
            if( xArray.Is() )
            {
                SbxVariable* pEnumVar = xArray->Find( rName, SbxCLASS_DONTCARE );
                SbxObject* pEnumObject = PTR_CAST( SbxObject, pEnumVar );
                if( pEnumObject )
                {
                    bool bPrivate = pEnumObject->IsSet( SBX_PRIVATE );
                    String aEnumName = pEnumObject->GetName();

                    pRes = new SbxVariable( SbxOBJECT );
                    pRes->SetName( aEnumName );
                    pRes->SetParent( this );
                    pRes->SetFlag( SBX_READ );
                    if( bPrivate )
                        pRes->SetFlag( SBX_PRIVATE );
                    pRes->PutObject( pEnumObject );
                }
            }
        }
    }
    return pRes;
}

const ::rtl::OUString& SbModule::GetSource32() const
{
    return aOUSource;
}

const String& SbModule::GetSource() const
{
    static String aRetStr;
    aRetStr = aOUSource;
    return aRetStr;
}

// Parent und BASIC sind eins!

void SbModule::SetParent( SbxObject* p )
{
    // #118083: Assertion is not valid any more
    // DBG_ASSERT( !p || p->IsA( TYPE(StarBASIC) ), "SbModules nur in BASIC eintragen" );
    pParent = p;
}

void SbModule::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                           const SfxHint& rHint, const TypeId& rHintType )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbProperty* pProp = PTR_CAST(SbProperty,pVar);
        SbMethod* pMeth = PTR_CAST(SbMethod,pVar);
        if( pProp )
        {
            if( pProp->GetModule() != this )
                SetError( SbxERR_BAD_ACTION );
        }
        else if( pMeth )
        {
            if( pHint->GetId() == SBX_HINT_DATAWANTED )
            {
                if( pMeth->bInvalid && !Compile() )
                    // Auto-Compile hat nicht geklappt!
                    StarBASIC::Error( SbERR_BAD_PROP_VALUE );
                else
                {
                    // Aufruf eines Unterprogramms
                    SbModule* pOld = pMOD;
                    pMOD = this;
                    Run( (SbMethod*) pVar );
                    pMOD = pOld;
                }
            }
        }
        else
            SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
    }
}

// Das Setzen der Source macht das Image ungueltig
// und scant die Methoden-Definitionen neu ein

void SbModule::SetSource( const String& r )
{
    SetSource32( r );
}

void SbModule::SetSource32( const ::rtl::OUString& r )
{
    aOUSource = r;
    StartDefinitions();
    SbiTokenizer aTok( r );
    while( !aTok.IsEof() )
    {
        SbiToken eEndTok = NIL;

        // Suchen nach SUB oder FUNCTION
        SbiToken eLastTok = NIL;
        while( !aTok.IsEof() )
        {
            // #32385: Nicht bei declare
            SbiToken eCurTok = aTok.Next();
            if( eLastTok != DECLARE )
            {
                if( eCurTok == SUB )
                {
                    eEndTok = ENDSUB; break;
                }
                if( eCurTok == FUNCTION )
                {
                    eEndTok = ENDFUNC; break;
                }
                if( eCurTok == PROPERTY )
                {
                    eEndTok = ENDPROPERTY; break;
                }
                if( eCurTok == OPTION )
                {
                    eCurTok = aTok.Next();
                    if( eCurTok == COMPATIBLE
                    || ( ( eCurTok == VBASUPPORT ) && ( aTok.Next() == NUMBER ) && ( aTok.GetDbl()== 1 ) ) )
                        aTok.SetCompatible( true );
                }
            }
            eLastTok = eCurTok;
        }
        // Definition der Methode
        SbMethod* pMeth = NULL;
        if( eEndTok != NIL )
        {
            USHORT nLine1 = aTok.GetLine();
            if( aTok.Next() == SYMBOL )
            {
                String aName_( aTok.GetSym() );
                SbxDataType t = aTok.GetType();
                if( t == SbxVARIANT && eEndTok == ENDSUB )
                    t = SbxVOID;
                pMeth = GetMethod( aName_, t );
                pMeth->nLine1 = pMeth->nLine2 = nLine1;
                // Die Methode ist erst mal GUELTIG
                pMeth->bInvalid = FALSE;
            }
            else
                eEndTok = NIL;
        }
        // Skip bis END SUB/END FUNCTION
        if( eEndTok != NIL )
        {
            while( !aTok.IsEof() )
            {
                if( aTok.Next() == eEndTok )
                {
                    pMeth->nLine2 = aTok.GetLine();
                    break;
                }
            }
            if( aTok.IsEof() )
                pMeth->nLine2 = aTok.GetLine();
        }
    }
    EndDefinitions( TRUE );
}

void SbModule::SetComment( const String& r )
{
    aComment = r;
    SetModified( TRUE );
}

SbMethod* SbModule::GetFunctionForLine( USHORT nLine )
{
    for( USHORT i = 0; i < pMethods->Count(); i++ )
    {
        SbMethod* p = (SbMethod*) pMethods->Get( i );
        if( p->GetSbxId() == SBXID_BASICMETHOD )
        {
            if( nLine >= p->nLine1 && nLine <= p->nLine2 )
                return p;
        }
    }
    return NULL;
}

// Ausstrahlen eines Hints an alle Basics

static void _SendHint( SbxObject* pObj, ULONG nId, SbMethod* p )
{
    // Selbst ein BASIC?
    if( pObj->IsA( TYPE(StarBASIC) ) && pObj->IsBroadcaster() )
        pObj->GetBroadcaster().Broadcast( SbxHint( nId, p ) );
    // Dann die Unterobjekte fragen
    SbxArray* pObjs = pObj->GetObjects();
    for( USHORT i = 0; i < pObjs->Count(); i++ )
    {
        SbxVariable* pVar = pObjs->Get( i );
        if( pVar->IsA( TYPE(SbxObject) ) )
            _SendHint( PTR_CAST(SbxObject,pVar), nId, p );
    }
}

static void SendHint( SbxObject* pObj, ULONG nId, SbMethod* p )
{
    while( pObj->GetParent() )
        pObj = pObj->GetParent();
    _SendHint( pObj, nId, p );
}

// #57841 Uno-Objekte, die in RTL-Funktionen gehalten werden,
// beim Programm-Ende freigeben, damit nichts gehalten wird.
void ClearUnoObjectsInRTL_Impl_Rek( StarBASIC* pBasic )
{
    // return-Wert von CreateUnoService loeschen
    static String aName( RTL_CONSTASCII_USTRINGPARAM("CreateUnoService") );
    SbxVariable* pVar = pBasic->GetRtl()->Find( aName, SbxCLASS_METHOD );
    if( pVar )
        pVar->SbxValue::Clear();

    // return-Wert von CreateUnoDialog loeschen
    static String aName2( RTL_CONSTASCII_USTRINGPARAM("CreateUnoDialog") );
    pVar = pBasic->GetRtl()->Find( aName2, SbxCLASS_METHOD );
    if( pVar )
        pVar->SbxValue::Clear();

    // return-Wert von CDec loeschen
    static String aName3( RTL_CONSTASCII_USTRINGPARAM("CDec") );
    pVar = pBasic->GetRtl()->Find( aName3, SbxCLASS_METHOD );
    if( pVar )
        pVar->SbxValue::Clear();

    // return-Wert von CreateObject loeschen
    static String aName4( RTL_CONSTASCII_USTRINGPARAM("CreateObject") );
    pVar = pBasic->GetRtl()->Find( aName4, SbxCLASS_METHOD );
    if( pVar )
        pVar->SbxValue::Clear();

    // Ueber alle Sub-Basics gehen
    SbxArray* pObjs = pBasic->GetObjects();
    USHORT nCount = pObjs->Count();
    for( USHORT i = 0 ; i < nCount ; i++ )
    {
        SbxVariable* pObjVar = pObjs->Get( i );
        StarBASIC* pSubBasic = PTR_CAST( StarBASIC, pObjVar );
        if( pSubBasic )
            ClearUnoObjectsInRTL_Impl_Rek( pSubBasic );
    }
}

void ClearUnoObjectsInRTL_Impl( StarBASIC* pBasic )
{
    // #67781 Rueckgabewerte der Uno-Methoden loeschen
    clearUnoMethods();
    clearUnoServiceCtors();

    ClearUnoObjectsInRTL_Impl_Rek( pBasic );

    // Oberstes Basic suchen
    SbxObject* p = pBasic;
    while( p->GetParent() )
        p = p->GetParent();
    if( ((StarBASIC*)p) != pBasic )
        ClearUnoObjectsInRTL_Impl_Rek( (StarBASIC*)p );
}

// Ausfuehren eines BASIC-Unterprogramms
USHORT SbModule::Run( SbMethod* pMeth )
{
    static USHORT nMaxCallLevel = 0;
    static String aMSOMacroRuntimeLibName = String::CreateFromAscii( "Launcher" );
    static String aMSOMacroRuntimeAppSymbol = String::CreateFromAscii( "Application" );

    USHORT nRes = 0;
    BOOL bDelInst = BOOL( pINST == NULL );
    StarBASICRef xBasic;
    if( bDelInst )
    {
        // #32779: Basic waehrend der Ausfuehrung festhalten
        xBasic = (StarBASIC*) GetParent();

        pINST = new SbiInstance( (StarBASIC*) GetParent() );

        // Launcher problem
        // i80726 The Find below will genarate an error in Testtool so we reset it unless there was one before already
        BOOL bWasError = SbxBase::GetError() != 0;
        SbxVariable* pMSOMacroRuntimeLibVar = Find( aMSOMacroRuntimeLibName, SbxCLASS_OBJECT );
        if ( !bWasError && (SbxBase::GetError() == SbxERR_PROC_UNDEFINED) )
            SbxBase::ResetError();
        if( pMSOMacroRuntimeLibVar )
        {
            StarBASIC* pMSOMacroRuntimeLib = PTR_CAST(StarBASIC,pMSOMacroRuntimeLibVar);
            if( pMSOMacroRuntimeLib )
            {
                USHORT nGblFlag = pMSOMacroRuntimeLib->GetFlags() & SBX_GBLSEARCH;
                pMSOMacroRuntimeLib->ResetFlag( SBX_GBLSEARCH );
                SbxVariable* pAppSymbol = pMSOMacroRuntimeLib->Find( aMSOMacroRuntimeAppSymbol, SbxCLASS_METHOD );
                pMSOMacroRuntimeLib->SetFlag( nGblFlag );
                if( pAppSymbol )
                {
                    pMSOMacroRuntimeLib->SetFlag( SBX_EXTSEARCH );      // Could have been disabled before
                    GetSbData()->pMSOMacroRuntimLib = pMSOMacroRuntimeLib;
                }
            }
        }

        // Error-Stack loeschen
        SbErrorStack*& rErrStack = GetSbData()->pErrStack;
        delete rErrStack;
        rErrStack = NULL;

        if( nMaxCallLevel == 0 )
        {
#ifdef UNX
          struct rlimit rl;
          getrlimit ( RLIMIT_STACK, &rl );
          // printf( "RLIMIT_STACK = %ld\n", rl.rlim_cur );
#endif
#if defined LINUX
          // Empiric value, 900 = needed bytes/Basic call level
          // for Linux including 10% safety margin
          nMaxCallLevel = rl.rlim_cur / 900;
#elif defined SOLARIS
          // Empiric value, 1650 = needed bytes/Basic call level
          // for Solaris including 10% safety margin
          nMaxCallLevel = rl.rlim_cur / 1650;
#elif defined WIN32
          nMaxCallLevel = 5800;
#else
          nMaxCallLevel = MAXRECURSION;
#endif
        }
    }

    // Rekursion zu tief?
    if( ++pINST->nCallLvl <= nMaxCallLevel )
    {
        // Globale Variable in allen Mods definieren
        GlobalRunInit( /* bBasicStart = */ bDelInst );

        // Trat ein Compiler-Fehler auf? Dann starten wir nicht
        if( GetSbData()->bGlobalInitErr == FALSE )
        {
            if( bDelInst )
            {
                SendHint( GetParent(), SBX_HINT_BASICSTART, pMeth );

                // 16.10.96: #31460 Neues Konzept fuer StepInto/Over/Out
                // Erklaerung siehe runtime.cxx bei SbiInstance::CalcBreakCallLevel()
                // BreakCallLevel ermitteln
                pINST->CalcBreakCallLevel( pMeth->GetDebugFlags() );
            }

            SbModule* pOldMod = pMOD;
            pMOD = this;
            SbiRuntime* pRt = new SbiRuntime( this, pMeth, pMeth->nStart );
            pRt->pNext = pINST->pRun;
            if( pRt->pNext )
                pRt->pNext->block();
            pINST->pRun = pRt;
            if ( SbiRuntime ::isVBAEnabled() )
                        {
                pINST->EnableCompatibility( TRUE );
                pRt->SetVBAEnabled( true );
                        }
            while( pRt->Step() ) {}
            if( pRt->pNext )
                pRt->pNext->unblock();

            // #63710 Durch ein anderes Thread-Handling bei Events kann es passieren,
            // dass show-Aufruf an einem Dialog zurueckkehrt (durch schliessen des
            // Dialogs per UI), BEVOR ein per Event ausgeloester weitergehender Call,
            // der in Basic weiter oben im Stack steht und auf einen Basic-Breakpoint
            // gelaufen ist, zurueckkehrt. Dann wird unten die Instanz zerstoert und
            // wenn das noch im Call stehende Basic weiterlaeuft, gibt es einen GPF.
            // Daher muss hier gewartet werden, bis andere Call zurueckkehrt.
            if( bDelInst )
            {
                // Hier mit 1 statt 0 vergleichen, da vor nCallLvl--
                while( pINST->nCallLvl != 1 )
                    GetpApp()->Yield();
            }

            nRes = TRUE;
            pINST->pRun = pRt->pNext;
            pINST->nCallLvl--;          // Call-Level wieder runter

            // Gibt es eine uebergeordnete Runtime-Instanz?
            // Dann SbDEBUG_BREAK uebernehmen, wenn gesetzt
            SbiRuntime* pRtNext = pRt->pNext;
            if( pRtNext && (pRt->GetDebugFlags() & SbDEBUG_BREAK) )
                pRtNext->SetDebugFlags( SbDEBUG_BREAK );

            delete pRt;
            pMOD = pOldMod;
            if( bDelInst )
            {
                // #57841 Uno-Objekte, die in RTL-Funktionen gehalten werden,
                // beim Programm-Ende freigeben, damit nichts gehalten wird.
                ClearUnoObjectsInRTL_Impl( xBasic );

                DBG_ASSERT(pINST->nCallLvl==0,"BASIC-Call-Level > 0");
                delete pINST, pINST = NULL, bDelInst = FALSE;

                // #i30690
                vos::OGuard aSolarGuard( Application::GetSolarMutex() );
                SendHint( GetParent(), SBX_HINT_BASICSTOP, pMeth );

                GlobalRunDeInit();
            }
        }
        else
               pINST->nCallLvl--;           // Call-Level wieder runter
    }
    else
    {
        pINST->nCallLvl--;          // Call-Level wieder runter
        StarBASIC::FatalError( SbERR_STACK_OVERFLOW );
    }
    if( bDelInst )
    {
        // #57841 Uno-Objekte, die in RTL-Funktionen gehalten werden,
        // beim Programm-Ende freigeben, damit nichts gehalten wird.
        ClearUnoObjectsInRTL_Impl( xBasic );

        delete pINST;
        pINST = NULL;
    }
    return nRes;
}

// Ausfuehren der Init-Methode eines Moduls nach dem Laden
// oder der Compilation

void SbModule::RunInit()
{
    if( pImage
     && !pImage->bInit
     && pImage->GetFlag( SBIMG_INITCODE ) )
    {
        // Flag setzen, dass RunInit aktiv ist (Testtool)
        GetSbData()->bRunInit = TRUE;

        // BOOL bDelInst = BOOL( pINST == NULL );
        // if( bDelInst )
            // pINST = new SbiInstance( (StarBASIC*) GetParent() );
        SbModule* pOldMod = pMOD;
        pMOD = this;
        // Der Init-Code beginnt immer hier
        SbiRuntime* pRt = new SbiRuntime( this, NULL, 0 );
        pRt->pNext = pINST->pRun;
        pINST->pRun = pRt;
        while( pRt->Step() ) {}
        pINST->pRun = pRt->pNext;
        delete pRt;
        pMOD = pOldMod;
        // if( bDelInst )
            // delete pINST, pINST = NULL;
        pImage->bInit = TRUE;
        pImage->bFirstInit = FALSE;

        // RunInit ist nicht mehr aktiv
        GetSbData()->bRunInit = FALSE;
    }
}

// Mit private/dim deklarierte Variablen loeschen
void SbModule::ClearPrivateVars()
{
    for( USHORT i = 0 ; i < pProps->Count() ; i++ )
    {
        SbProperty* p = PTR_CAST(SbProperty,pProps->Get( i ) );
        if( p )
        {
            // Arrays nicht loeschen, sondern nur deren Inhalt
            if( p->GetType() & SbxARRAY )
            {
                SbxArray* pArray = PTR_CAST(SbxArray,p->GetObject());
                if( pArray )
                {
                    for( USHORT j = 0 ; j < pArray->Count() ; j++ )
                    {
                        SbxVariable* pj = PTR_CAST(SbxVariable,pArray->Get( j ));
                        pj->SbxValue::Clear();
                        /*
                        USHORT nFlags = pj->GetFlags();
                        pj->SetFlags( (nFlags | SBX_WRITE) & (~SBX_FIXED) );
                        pj->PutEmpty();
                        pj->SetFlags( nFlags );
                        */
                    }
                }
            }
            else
            {
                p->SbxValue::Clear();
                /*
                USHORT nFlags = p->GetFlags();
                p->SetFlags( (nFlags | SBX_WRITE) & (~SBX_FIXED) );
                p->PutEmpty();
                p->SetFlags( nFlags );
                */
            }
        }
    }
}

// Zunaechst in dieses Modul, um 358-faehig zu bleiben
// (Branch in sb.cxx vermeiden)
void StarBASIC::ClearAllModuleVars( void )
{
    // Eigene Module initialisieren
    for ( USHORT nMod = 0; nMod < pModules->Count(); nMod++ )
    {
        SbModule* pModule = (SbModule*)pModules->Get( nMod );
        // Nur initialisieren, wenn der Startcode schon ausgefuehrt wurde
        if( pModule->pImage && pModule->pImage->bInit )
            pModule->ClearPrivateVars();
    }

    /* #88042 This code can delete already used public vars during runtime!
    // Alle Objekte ueberpruefen, ob es sich um ein Basic handelt
    // Wenn ja, auch dort initialisieren
    for ( USHORT nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
        if( pBasic )
            pBasic->ClearAllModuleVars();
    }
    */
}

// Ausfuehren des Init-Codes aller Module
void SbModule::GlobalRunInit( BOOL bBasicStart )
{
    // Wenn kein Basic-Start, nur initialisieren, wenn Modul uninitialisiert
    if( !bBasicStart )
        if( !(pImage && !pImage->bInit) )
            return;

    // GlobalInitErr-Flag fuer Compiler-Error initialisieren
    // Anhand dieses Flags kann in SbModule::Run() nach dem Aufruf
    // von GlobalRunInit festgestellt werden, ob beim initialisieren
    // der Module ein Fehler auftrat. Dann wird nicht gestartet.
    GetSbData()->bGlobalInitErr = FALSE;

    // Parent vom Modul ist ein Basic
    StarBASIC *pBasic = PTR_CAST(StarBASIC,GetParent());
    if( pBasic )
    {
        pBasic->InitAllModules();

        SbxObject* pParent_ = pBasic->GetParent();
        if( pParent_ )
        {
            StarBASIC * pParentBasic = PTR_CAST(StarBASIC,pParent_);
            if( pParentBasic )
            {
                pParentBasic->InitAllModules( pBasic );

                // #109018 Parent can also have a parent (library in doc)
                SbxObject* pParentParent = pParentBasic->GetParent();
                if( pParentParent )
                {
                    StarBASIC * pParentParentBasic = PTR_CAST(StarBASIC,pParentParent);
                    if( pParentParentBasic )
                        pParentParentBasic->InitAllModules( pParentBasic );
                }
            }
        }
    }
}

void SbModule::GlobalRunDeInit( void )
{
    StarBASIC *pBasic = PTR_CAST(StarBASIC,GetParent());
    if( pBasic )
    {
        pBasic->DeInitAllModules();

        SbxObject* pParent_ = pBasic->GetParent();
        if( pParent_ )
            pBasic = PTR_CAST(StarBASIC,pParent_);
        if( pBasic )
            pBasic->DeInitAllModules();
    }
}

// Suche nach dem naechsten STMNT-Befehl im Code. Wird vom STMNT-
// Opcode verwendet, um die Endspalte zu setzen.

const BYTE* SbModule::FindNextStmnt( const BYTE* p, USHORT& nLine, USHORT& nCol ) const
{
    return FindNextStmnt( p, nLine, nCol, FALSE );
}

const BYTE* SbModule::FindNextStmnt( const BYTE* p, USHORT& nLine, USHORT& nCol,
    BOOL bFollowJumps, const SbiImage* pImg ) const
{
    UINT32 nPC = (UINT32) ( p - (const BYTE*) pImage->GetCode() );
    while( nPC < pImage->GetCodeSize() )
    {
        SbiOpcode eOp = (SbiOpcode ) ( *p++ );
        nPC++;
        if( bFollowJumps && eOp == _JUMP && pImg )
        {
            DBG_ASSERT( pImg, "FindNextStmnt: pImg==NULL with FollowJumps option" );
            UINT32 nOp1 = *p++; nOp1 |= *p++ << 8;
            nOp1 |= *p++ << 16; nOp1 |= *p++ << 24;
            p = (const BYTE*) pImg->GetCode() + nOp1;
        }
        else if( eOp >= SbOP1_START && eOp <= SbOP1_END )
            p += 4, nPC += 4;
        else if( eOp == _STMNT )
        {
            UINT32 nl, nc;
            nl = *p++; nl |= *p++ << 8;
            nl |= *p++ << 16 ; nl |= *p++ << 24;
            nc = *p++; nc |= *p++ << 8;
            nc |= *p++ << 16 ; nc |= *p++ << 24;
            nLine = (USHORT)nl; nCol = (USHORT)nc;
            return p;
        }
        else if( eOp >= SbOP2_START && eOp <= SbOP2_END )
            p += 8, nPC += 8;
        else if( !( eOp >= SbOP0_START && eOp <= SbOP0_END ) )
        {
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
            break;
        }
    }
    return NULL;
}

// Testen, ob eine Zeile STMNT-Opcodes enthaelt

BOOL SbModule::IsBreakable( USHORT nLine ) const
{
    if( !pImage )
        return FALSE;
    const BYTE* p = (const BYTE* ) pImage->GetCode();
    USHORT nl, nc;
    while( ( p = FindNextStmnt( p, nl, nc ) ) != NULL )
        if( nl == nLine )
            return TRUE;
    return FALSE;
}

USHORT SbModule::GetBPCount() const
{
    return pBreaks ? pBreaks->Count() : 0;
}

USHORT SbModule::GetBP( USHORT n ) const
{
    if( pBreaks && n < pBreaks->Count() )
        return pBreaks->GetObject( n );
    else
        return 0;
}

BOOL SbModule::IsBP( USHORT nLine ) const
{
    if( pBreaks )
    {
        const USHORT* p = pBreaks->GetData();
        USHORT n = pBreaks->Count();
        for( USHORT i = 0; i < n; i++, p++ )
        {
            USHORT b = *p;
            if( b == nLine )
                return TRUE;
            if( b < nLine )
                break;
        }
    }
    return FALSE;
}

BOOL SbModule::SetBP( USHORT nLine )
{
    if( !IsBreakable( nLine ) )
        return FALSE;
    if( !pBreaks )
        pBreaks = new SbiBreakpoints;
    const USHORT* p = pBreaks->GetData();
    USHORT n = pBreaks->Count();
    USHORT i;
    for( i = 0; i < n; i++, p++ )
    {
        USHORT b = *p;
        if( b == nLine )
            return TRUE;
        if( b < nLine )
            break;
    }
    pBreaks->Insert( &nLine, 1, i );

    // #38568: Zur Laufzeit auch hier SbDEBUG_BREAK setzen
    if( pINST && pINST->pRun )
        pINST->pRun->SetDebugFlags( SbDEBUG_BREAK );

    return IsBreakable( nLine );
}

BOOL SbModule::ClearBP( USHORT nLine )
{
    BOOL bRes = FALSE;
    if( pBreaks )
    {
        const USHORT* p = pBreaks->GetData();
        USHORT n = pBreaks->Count();
        for( USHORT i = 0; i < n; i++, p++ )
        {
            USHORT b = *p;
            if( b == nLine )
            {
                pBreaks->Remove( i, 1 ); bRes = TRUE; break;
            }
            if( b < nLine )
                break;
        }
        if( !pBreaks->Count() )
            delete pBreaks, pBreaks = NULL;
    }
    return bRes;
}

void SbModule::ClearAllBP()
{
    delete pBreaks; pBreaks = NULL;
}

void
SbModule::fixUpMethodStart( bool bCvtToLegacy, SbiImage* pImg ) const
{
        if ( !pImg )
            pImg = pImage;
        for( UINT32 i = 0; i < pMethods->Count(); i++ )
        {
            SbMethod* pMeth = PTR_CAST(SbMethod,pMethods->Get( (USHORT)i ) );
            if( pMeth )
            {
                //fixup method start positions
                if ( bCvtToLegacy )
                    pMeth->nStart = pImg->CalcLegacyOffset( pMeth->nStart );
                else
                    pMeth->nStart = pImg->CalcNewOffset( (USHORT)pMeth->nStart );
            }
        }

}

BOOL SbModule::LoadData( SvStream& rStrm, USHORT nVer )
{
    Clear();
    if( !SbxObject::LoadData( rStrm, 1 ) )
        return FALSE;
    // Sicherheitshalber...
    SetFlag( SBX_EXTSEARCH | SBX_GBLSEARCH );
    BYTE bImage;
    rStrm >> bImage;
    if( bImage )
    {
        SbiImage* p = new SbiImage;
        UINT32 nImgVer = 0;

        if( !p->Load( rStrm, nImgVer ) )
        {
            delete p;
            return FALSE;
        }
        // If the image is in old format, we fix up the method start offsets
        if ( nImgVer < B_EXT_IMG_VERSION )
        {
            fixUpMethodStart( false, p );
            p->ReleaseLegacyBuffer();
        }
        aComment = p->aComment;
        SetName( p->aName );
        if( p->GetCodeSize() )
        {
            aOUSource = p->aOUSource;
            // Alte Version: Image weg
            if( nVer == 1 )
            {
                SetSource32( p->aOUSource );
                delete p;
            }
            else
                pImage = p;
        }
        else
        {
            SetSource32( p->aOUSource );
            delete p;
        }
    }
    return TRUE;
}

BOOL SbModule::StoreData( SvStream& rStrm ) const
{
    BOOL bFixup = ( pImage && !pImage->ExceedsLegacyLimits() );
    if ( bFixup )
        fixUpMethodStart( true );
    BOOL bRet = SbxObject::StoreData( rStrm );
    if ( !bRet )
        return FALSE;

    if( pImage )
    {
        pImage->aOUSource = aOUSource;
        pImage->aComment = aComment;
        pImage->aName = GetName();
        rStrm << (BYTE) 1;
        // # PCode is saved only for legacy formats only
        // It should be noted that it probably isn't necessary
        // It would be better not to store the image ( more flexible with
        // formats )
        bool bRes = pImage->Save( rStrm, B_LEGACYVERSION );
        if ( bFixup )
            fixUpMethodStart( false ); // restore method starts
        return bRes;

    }
    else
    {
        SbiImage aImg;
        aImg.aOUSource = aOUSource;
        aImg.aComment = aComment;
        aImg.aName = GetName();
        rStrm << (BYTE) 1;
        return aImg.Save( rStrm );
    }
}

BOOL SbModule::ExceedsLegacyModuleSize()
{
    if ( !IsCompiled() )
        Compile();
    if ( pImage && pImage->ExceedsLegacyLimits() )
        return true;
    return false;
}


// Store only image, no source
BOOL SbModule::StoreBinaryData( SvStream& rStrm )
{
    return StoreBinaryData( rStrm, 0 );
}

BOOL SbModule::StoreBinaryData( SvStream& rStrm, USHORT nVer )
{
    BOOL bRet = Compile();
    if( bRet )
    {
        BOOL bFixup = ( !nVer && !pImage->ExceedsLegacyLimits() );// save in old image format, fix up method starts

        if ( bFixup ) // save in old image format, fix up method starts
            fixUpMethodStart( true );
         bRet = SbxObject::StoreData( rStrm );
        if( bRet )
        {
            pImage->aOUSource = ::rtl::OUString();
            pImage->aComment = aComment;
            pImage->aName = GetName();

            rStrm << (BYTE) 1;
                    if ( nVer )
                        bRet = pImage->Save( rStrm, B_EXT_IMG_VERSION );
                    else
                        bRet = pImage->Save( rStrm, B_LEGACYVERSION );
                    if ( bFixup )
                        fixUpMethodStart( false ); // restore method starts

            pImage->aOUSource = aOUSource;
        }
    }
    return bRet;
}

// Called for >= OO 1.0 passwd protected libraries only
//

BOOL SbModule::LoadBinaryData( SvStream& rStrm )
{
    ::rtl::OUString aKeepSource = aOUSource;
    bool bRet = LoadData( rStrm, 2 );
    LoadCompleted();
    aOUSource = aKeepSource;
    return bRet;
}


BOOL SbModule::LoadCompleted()
{
    SbxArray* p = GetMethods();
    USHORT i;
    for( i = 0; i < p->Count(); i++ )
    {
        SbMethod* q = PTR_CAST(SbMethod,p->Get( i ) );
        if( q )
            q->pMod = this;
    }
    p = GetProperties();
    for( i = 0; i < p->Count(); i++ )
    {
        SbProperty* q = PTR_CAST(SbProperty,p->Get( i ) );
        if( q )
            q->pMod = this;
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
// Implementation SbJScriptModule (Basic-Modul fuer JavaScript-Sourcen)
SbJScriptModule::SbJScriptModule( const String& rName )
    :SbModule( rName )
{
}

BOOL SbJScriptModule::LoadData( SvStream& rStrm, USHORT nVer )
{
    (void)nVer;

    Clear();
    if( !SbxObject::LoadData( rStrm, 1 ) )
        return FALSE;

    // Source-String holen
    String aTmp;
    rStrm.ReadByteString( aTmp, gsl_getSystemTextEncoding() );
    aOUSource = aTmp;
    //rStrm >> aSource;
    return TRUE;
}

BOOL SbJScriptModule::StoreData( SvStream& rStrm ) const
{
    if( !SbxObject::StoreData( rStrm ) )
        return FALSE;

    // Source-String schreiben
    String aTmp = aOUSource;
    rStrm.WriteByteString( aTmp, gsl_getSystemTextEncoding() );
    //rStrm << aSource;
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////

SbMethod::SbMethod( const String& r, SbxDataType t, SbModule* p )
        : SbxMethod( r, t ), pMod( p )
{
    bInvalid     = TRUE;
    nStart       =
    nDebugFlags  =
    nLine1       =
    nLine2       = 0;
    refStatics = new SbxArray;
    // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
    SetFlag( SBX_NO_MODIFY );
}

SbMethod::SbMethod( const SbMethod& r )
    : SvRefBase( r ), SbxMethod( r )
{
    pMod         = r.pMod;
    bInvalid     = r.bInvalid;
    nStart       = r.nStart;
    nDebugFlags  = r.nDebugFlags;
    nLine1       = r.nLine1;
    nLine2       = r.nLine2;
        refStatics = r.refStatics;
    SetFlag( SBX_NO_MODIFY );
}

SbMethod::~SbMethod()
{
}

SbxArray* SbMethod::GetLocals()
{
    if( pINST )
        return pINST->GetLocals( this );
    else
        return NULL;
}

void SbMethod::ClearStatics()
{
    refStatics = new SbxArray;

}
SbxArray* SbMethod::GetStatics()
{
    return refStatics;
}

BOOL SbMethod::LoadData( SvStream& rStrm, USHORT nVer )
{
    if( !SbxMethod::LoadData( rStrm, 1 ) )
        return FALSE;
    INT16 n;
    rStrm >> n;
    INT16 nTempStart = (INT16)nStart;
    // nDebugFlags = n;     // AB 16.1.96: Nicht mehr uebernehmen
    if( nVer == 2 )
        rStrm >> nLine1 >> nLine2 >> nTempStart >> bInvalid;
    // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
    SetFlag( SBX_NO_MODIFY );
    nStart = nTempStart;
    return TRUE;
}

BOOL SbMethod::StoreData( SvStream& rStrm ) const
{
    if( !SbxMethod::StoreData( rStrm ) )
        return FALSE;
    rStrm << (INT16) nDebugFlags
          << (INT16) nLine1
          << (INT16) nLine2
          << (INT16) nStart
          << (BYTE)  bInvalid;
    return TRUE;
}

void SbMethod::GetLineRange( USHORT& l1, USHORT& l2 )
{
    l1 = nLine1; l2 = nLine2;
}

// Kann spaeter mal weg

SbxInfo* SbMethod::GetInfo()
{
    return pInfo;
}

// Schnittstelle zum Ausfuehren einer Methode aus den Applikationen
// #34191# Mit speziellem RefCounting, damit das Basic nicht durch CloseDocument()
// abgeschossen werden kann. Rueckgabewert wird als String geliefert.
ErrCode SbMethod::Call( SbxValue* pRet )
{
    // RefCount vom Modul hochzaehlen
    SbModule* pMod_ = (SbModule*)GetParent();
    pMod_->AddRef();

    // RefCount vom Basic hochzaehlen
    StarBASIC* pBasic = (StarBASIC*)pMod_->GetParent();
    pBasic->AddRef();

    // Values anlegen, um Return-Wert zu erhalten
    SbxValues aVals;
    aVals.eType = SbxVARIANT;

    // #104083: Compile BEFORE get
    if( bInvalid && !pMod_->Compile() )
        StarBASIC::Error( SbERR_BAD_PROP_VALUE );

    Get( aVals );
    if ( pRet )
        pRet->Put( aVals );

    // Gab es einen Error
    ErrCode nErr = SbxBase::GetError();
    SbxBase::ResetError();

    // Objekte freigeben
    pMod_->ReleaseRef();
    pBasic->ReleaseRef();

    return nErr;
}


// #100883 Own Broadcast for SbMethod
void SbMethod::Broadcast( ULONG nHintId )
{
    if( pCst && !IsSet( SBX_NO_BROADCAST ) && StaticIsEnabledBroadcasting() )
    {
        // Da die Methode von aussen aufrufbar ist, hier noch einmal
        // die Berechtigung testen
        if( nHintId & SBX_HINT_DATAWANTED )
            if( !CanRead() )
                return;
        if( nHintId & SBX_HINT_DATACHANGED )
            if( !CanWrite() )
                return;

        if( pMod && !pMod->IsCompiled() )
            pMod->Compile();

        // Block broadcasts while creating new method
        SfxBroadcaster* pSave = pCst;
        pCst = NULL;
        SbMethod* pThisCopy = new SbMethod( *this );
        SbMethodRef xHolder = pThisCopy;
        if( mpPar.Is() )
        {
            // this, als Element 0 eintragen, aber den Parent nicht umsetzen!
            if( GetType() != SbxVOID )
                mpPar->PutDirect( pThisCopy, 0 );
               SetParameters( NULL );
        }

        pCst = pSave;
        pSave->Broadcast( SbxHint( nHintId, pThisCopy ) );

        USHORT nSaveFlags = GetFlags();
        SetFlag( SBX_READWRITE );
        pCst = NULL;
        Put( pThisCopy->GetValues_Impl() );
        pCst = pSave;
        SetFlags( nSaveFlags );
    }
}

/////////////////////////////////////////////////////////////////////////

// Implementation SbJScriptMethod (Method-Klasse als Wrapper fuer JavaScript-Funktionen)

SbJScriptMethod::SbJScriptMethod( const String& r, SbxDataType t, SbModule* p )
        : SbMethod( r, t, p )
{
}

SbJScriptMethod::~SbJScriptMethod()
{}


/////////////////////////////////////////////////////////////////////////

SbProperty::SbProperty( const String& r, SbxDataType t, SbModule* p )
        : SbxProperty( r, t ), pMod( p )
{
    bInvalid = FALSE;
}

SbProperty::~SbProperty()
{}

/////////////////////////////////////////////////////////////////////////

SbProcedureProperty::~SbProcedureProperty()
{}

