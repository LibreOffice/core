/*************************************************************************
 *
 *  $RCSfile: sbxmod.cxx,v $
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

#if STLPORT_VERSION < 321
#include <tools/presys.h>
#include <list>
#include <tools/postsys.h>
#else
#include <list>
#endif

#include <vos/macros.hxx>
#include <vcl/svapp.hxx>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#pragma hdrstop
#include <svtools/sbx.hxx>
#include "sb.hxx"
#include <sbjsmeth.hxx>
#include "sbjsmod.hxx"
#include "sbintern.hxx"
#include "image.hxx"
#include "opcodes.hxx"
#include "runtime.hxx"
#include "token.hxx"
#include "sbunoobj.hxx"
#include <hilight.hxx>
#include <basrdll.hxx>


// for the bsearch
#ifdef WNT
#define CDECL _cdecl
#endif
#ifdef OS2
#define CDECL _Optlink
#endif
#if defined(UNX)  || defined(MAC)
#define CDECL
#endif


// TEST
#include <stdio.h>

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBASIC, SBASIC_CODE )

TYPEINIT1(SbModule,SbxObject)
TYPEINIT1(SbMethod,SbxMethod)
TYPEINIT1(SbProperty,SbxProperty)
TYPEINIT1(SbJScriptModule,SbModule)
TYPEINIT1(SbJScriptMethod,SbMethod)

SV_DECL_VARARR(SbiBreakpoints,USHORT,4,4)
SV_IMPL_VARARR(SbiBreakpoints,USHORT)


SV_IMPL_VARARR(HighlightPortions, HighlightPortion)

// ##########################################################################
// ACHTUNG!!!  Alle Woerter dieser Tabelle müssen KLEIN geschrieben werden!!!
// ##########################################################################
static char* strListBasicKeyWords[] = {
    "access",
    "alias",
    "and",
    "any",
    "append",
    "as",
    "base",
    "binary",
    "boolean",
    "byval",
    "call",
    "case",
    "cdecl",
    "close",
    "compare",
    "const",
    "currency",
    "date",
    "declare",
    "defbool",
    "defcur",
    "defdate",
    "defdbl",
    "deferr",
    "defint",
    "deflng",
    "defobj",
    "defsng",
    "defstr",
    "defvar",
    "dim",
    "do",
    "double",
    "each",
    "else",
    "elseif",
    "end",
    "end function",
    "end if",
    "end select",
    "end sub",
    "end type",
    "endif",
    "eqv",
    "erase",
    "error",
    "exit",
    "explicit",
    "for",
    "function",
    "global",
    "gosub",
    "goto",
    "if",
    "imp",
    "in",
    "input",
    "integer",
    "is",
    "let",
    "lib"
    "line",
    "line input",
    "local",
    "lock",
    "long",
    "loop",
    "lprint",
    "lset",
    "mod",
    "name",
    "new",
    "next",
    "not",
    "object",
    "on",
    "open",
    "option",
    "optional",
    "or",
    "output",
    "preserve",
    "print",
    "private",
    "public",
    "random",
    "read",
    "redim",
    "rem",
    "resume",
    "return",
    "rset",
    "select",
    "set",
    "shared",
    "single",
    "static",
    "step",
    "stop",
    "string",
    "sub",
    "system",
    "text",
    "then",
    "to",
    "type",
    "until",
    "variant",
    "wend",
    "while",
    "with",
    "write",
    "xor"
};

int CDECL compare_strings( const void *arg1, const void *arg2 )
{
    char* pCh = *(char**)arg2;
    return strcmp( (char *)arg1, *(char **)arg2 );
}



/////////////////////////////////////////////////////////////////////////////

// Ein BASIC-Modul hat EXTSEARCH gesetzt, damit die im Modul enthaltenen
// Elemente von anderen Modulen aus gefunden werden koennen.

SbModule::SbModule( const String& rName )
         : SbxObject( String( RTL_CONSTASCII_USTRINGPARAM("StarBASICModule") ) ),
           pImage( NULL ), pBreaks( NULL )
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
}

BOOL SbModule::IsCompiled() const
{
    return BOOL( pImage != 0 );
}

// Aus dem Codegenerator: Loeschen des Images und Invalidieren der Entries

void SbModule::StartDefinitions()
{
    delete pImage; pImage = NULL;
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
    }
    SetModified( TRUE );
}

void SbModule::Clear()
{
    delete pImage; pImage = NULL;
    SbxObject::Clear();
}

const String& SbModule::GetSource() const
{
    return aSource;
}

// Parent und BASIC sind eins!

void SbModule::SetParent( SbxObject* p )
{
    DBG_ASSERT( !p || p->IsA( TYPE(StarBASIC) ), "SbModules nur in BASIC eintragen" );
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
    aSource = r;
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
            }
            eLastTok = eCurTok;
        }
        // Definition der Methode
        SbMethod* pMeth;
        if( eEndTok != NIL )
        {
            USHORT nLine1 = aTok.GetLine();
            if( aTok.Next() == SYMBOL )
            {
                String aName( aTok.GetSym() );
                SbxDataType t = aTok.GetType();
                if( t == SbxVARIANT && eEndTok == ENDSUB )
                    t = SbxVOID;
                pMeth = GetMethod( aName, t );
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

    // Oberstes Basic suchen
    SbxObject* p = pBasic;
    while( p->GetParent() )
        p = p->GetParent();

    // Rekursiven Loeschvorgang ausloesen
    ClearUnoObjectsInRTL_Impl_Rek( (StarBASIC*)p );
}

// Ausfuehren eines BASIC-Unterprogramms
USHORT SbModule::Run( SbMethod* pMeth )
{
    USHORT nRes = 0;
    BOOL bDelInst = BOOL( pINST == NULL );
    StarBASICRef xBasic;
    if( bDelInst )
    {
        // #32779: Basic waehrend der Ausfuehrung festhalten
        xBasic = (StarBASIC*) GetParent();

        pINST = new SbiInstance( (StarBASIC*) GetParent() );

        // Error-Stack loeschen
        SbErrorStack*& rErrStack = GetSbData()->pErrStack;
        delete rErrStack;
        rErrStack = NULL;
    }
    // Rekursion zu tief?
    if( ++pINST->nCallLvl <= MAXRECURSION )
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
            pINST->pRun = pRt;
            while( pRt->Step() ) {}

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

                DBG_ASSERT(pINST->nCallLvl==0,"BASIC-Call-Level > 0")
                delete pINST, pINST = NULL, bDelInst = FALSE;
                SendHint( GetParent(), SBX_HINT_BASICSTOP, pMeth );
            }
        }
    }
    else
        StarBASIC::FatalError( SbERR_STACK_OVERFLOW );
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

        // RunInit ist nicht mehr aktiv
        GetSbData()->bRunInit = FALSE;
    }
}

// Mit private/dim deklarierte Variablen loeschen
void SbModule::ClearPrivateVars()
{
    for( int i = 0 ; i < pProps->Count() ; i++ )
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
                    for( int j = 0 ; j < pArray->Count() ; j++ )
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
    // Alle Objekte ueberpruefen, ob es sich um ein Basic handelt
    // Wenn ja, auch dort initialisieren
    for ( USHORT nObj = 0; nObj < pObjs->Count(); nObj++ )
    {
        SbxVariable* pVar = pObjs->Get( nObj );
        StarBASIC* pBasic = PTR_CAST(StarBASIC,pVar);
        if( pBasic )
            pBasic->ClearAllModuleVars();
    }

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

        SbxObject* pParent = pBasic->GetParent();
        if( pParent )
            pBasic = PTR_CAST(StarBASIC,pParent);
        if( pBasic )
            pBasic->InitAllModules();
    }
}

// Suche nach dem naechsten STMNT-Befehl im Code. Wird vom STMNT-
// Opcode verwendet, um die Endspalte zu setzen.

const BYTE* SbModule::FindNextStmnt( const BYTE* p, USHORT& nLine, USHORT& nCol ) const
{
    USHORT nPC = (USHORT) ( p - (const BYTE*) pImage->GetCode() );
    while( nPC < pImage->GetCodeSize() )
    {
        SbiOpcode eOp = (SbiOpcode ) ( *p++ );
        nPC++;
        if( eOp >= SbOP1_START && eOp <= SbOP1_END )
            p += 2, nPC += 2;
        else if( eOp == _STMNT )
        {
            USHORT nl, nc;
            nl = *p++; nl |= *p++ << 8;
            nc = *p++; nc |= *p++ << 8;
            nLine = nl; nCol = nc;
            return p;
        }
        else if( eOp >= SbOP2_START && eOp <= SbOP2_END )
            p += 4, nPC += 4;
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
        if( !p->Load( rStrm ) )
        {
            delete p;
            return FALSE;
        }
        aComment = p->aComment;
        SetName( p->aName );
        // Ist Code vorhanden?
        if( p->GetCodeSize() )
        {
            aSource = p->aSource;
            // Alte Version: Image weg
            if( nVer == 1 )
            {
                SetSource( p->aSource );
                delete p;
            }
            else
                pImage = p;
        }
        else
        {
            SetSource( p->aSource );
            delete p;
        }
    }
    return TRUE;
}

BOOL SbModule::StoreData( SvStream& rStrm ) const
{
    if( !SbxObject::StoreData( rStrm ) )
        return FALSE;
    if( pImage )
    {
        pImage->aSource = aSource;
        pImage->aComment = aComment;
        pImage->aName = GetName();
        rStrm << (BYTE) 1;
        return pImage->Save( rStrm );
    }
    else
    {
        SbiImage aImg;
        aImg.aSource = aSource;
        aImg.aComment = aComment;
        aImg.aName = GetName();
        rStrm << (BYTE) 1;
        return aImg.Save( rStrm );
    }
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
// Hilfsklasse zur Untersuchung von JavaScript-Modulen, zunaechst zum
// Heraussuchen der Funktionen, spaeter auch zum Syntax-Highlighting verwenden

//  Flags fuer Zeichen-Eigenschaften
#define CHAR_START_IDENTIFIER   0x0001
#define CHAR_IN_IDENTIFIER      0x0002
#define CHAR_START_NUMBER       0x0004
#define CHAR_IN_NUMBER          0x0008
#define CHAR_IN_HEX_NUMBER      0x0010
#define CHAR_IN_OCT_NUMBER      0x0020
#define CHAR_START_STRING       0x0040
#define CHAR_OPERATOR           0x0080
#define CHAR_SPACE              0x0100
#define CHAR_EOL                0x0200

#define CHAR_EOF                0x00


// Token-Typen TT_...

//enum TokenType
//{
//  TT_UNKNOWN,
//  TT_IDENTIFIER,
//  TT_WHITESPACE,
//  TT_NUMBER,
//  TT_STRING,
////    TT_HTMLSTRING,
////    TT_LONG,
////    TT_DOUBLE,
////    TT_BOOLEAN,
////    TT_NULLOBJECT,
////    TT_CHAR,
//  TT_EOL,
////    TT_LONG2DOUBLE,
//  TT_COMMENT,
////    TT_SKIP,
//  TT_ERROR,
//  TT_OPERATOR,
//  TT_KEYWORD
//};


class SimpleTokenizer_Impl
{
    // Zeichen-Info-Tabelle
    USHORT aCharTypeTab[256];

    const char* mpStringBegin;
    const char* mpActualPos;

    // Zeile und Spalte
    UINT32 nLine;
    UINT32 nCol;

    char peekChar( void )   { return *mpActualPos; }
    char getChar( void )    { nCol++; return *mpActualPos++; }

    // Hilfsfunktion: Zeichen-Flag Testen
    BOOL testCharFlags( unsigned char c, USHORT nTestFlags );

    // Neues Token holen, Leerstring == nix mehr da
    BOOL getNextToken( /*out*/TokenTypes& reType,
        /*out*/const char*& rpStartPos, /*out*/const char*& rpEndPos );

    String getTokStr( /*out*/const char* pStartPos, /*out*/const char* pEndPos );

    // TEST: Token ausgeben
    String getFullTokenStr( /*out*/TokenTypes eType,
        /*out*/const char* pStartPos, /*out*/const char* pEndPos );

    BOOL isBeginComment( UINT32 nLine );
    void setCommentState(UINT32 nLine, BOOL bCommentBegin, BOOL bCommentEnd);

    NAMESPACE_STD(list)<BOOL>* pCommentsBegin;
    NAMESPACE_STD(list)<BOOL>* pCommentsEnd;

    char** ppListKeyWords;
    UINT16 nKeyWordCount;
    BOOL bStarScriptMode;

    BOOL bLineHasCommentBegin;
    BOOL bLineHasCommentEnd;

public:
    SimpleTokenizer_Impl( void );
    ~SimpleTokenizer_Impl( void );

    UINT16 parseLine( UINT32 nLine, const String* aSource );
    void getHighlightPortions( UINT32 nParseLine, const String& rLine,
                                                    /*out*/HighlightPortions& portions );
    void addLines(UINT32 nLine, INT32 nCount);
    void outCommentList();
    void setKeyWords( char** ppKeyWords, UINT16 nCount );
};

// Hilfsfunktion: Zeichen-Flag Testen
BOOL SimpleTokenizer_Impl::testCharFlags( unsigned char c, USHORT nTestFlags )
{
    if( c != 0 )
        return ( (aCharTypeTab[c] & nTestFlags) != 0 );
    return FALSE;
}

void SimpleTokenizer_Impl::setKeyWords( char** ppKeyWords, UINT16 nCount )
{
    ppListKeyWords = ppKeyWords;
    nKeyWordCount = nCount;
}

// Neues Token holen
BOOL SimpleTokenizer_Impl::getNextToken( /*out*/TokenTypes& reType,
    /*out*/const char*& rpStartPos, /*out*/const char*& rpEndPos )
{
    reType = TT_UNKNOWN;

    // Position merken
    rpStartPos = mpActualPos;

    // Zeichen untersuchen
    char c = peekChar();
    if( c == CHAR_EOF )
        return FALSE;

    // Zeichen lesen
    getChar();

    //*** Alle Moeglichkeiten durchgehen ***
    // Spce?
    if ( (testCharFlags( c, CHAR_SPACE ) == TRUE) && (!bLineHasCommentBegin) )
    {
        while( testCharFlags( peekChar(), CHAR_SPACE ) == TRUE )
            getChar();

        reType = TT_WHITESPACE;
    }

    // Identifier?
    else if ( (testCharFlags( c, CHAR_START_IDENTIFIER ) == TRUE) && (!bLineHasCommentBegin) )
    {
        BOOL bIdentifierChar;
        int nPos = 0;
        do
        {
            // Naechstes Zeichen holen
            c = peekChar();
            bIdentifierChar = testCharFlags( c, CHAR_IN_IDENTIFIER );
            if( bIdentifierChar )
                getChar();
        }
        while( bIdentifierChar );

        reType = TT_IDENTIFIER;

        // Schluesselwort-Tabelle
        if (ppListKeyWords != NULL)
        {
            ByteString aByteStr(rpStartPos, mpActualPos-rpStartPos);
            if ( !bStarScriptMode )
                aByteStr.ToLowerAscii();

            if ( bsearch( aByteStr.GetBuffer(), ppListKeyWords, nKeyWordCount, sizeof( char* ),
                                                                    compare_strings ) )
            {
                reType = TT_KEYWORD;

                if ( (!bStarScriptMode) && (aByteStr.Equals( "rem" )) )
                {
                    // Alle Zeichen bis Zeilen-Ende oder EOF entfernen
                    char cPeek = peekChar();
                    while( cPeek != CHAR_EOF && testCharFlags( cPeek, CHAR_EOL ) == FALSE )
                    {
                        c = getChar();
                        cPeek = peekChar();
                    }

                    reType = TT_COMMENT;
                }
            }
        }
    }

    // Operator?
    else if ( (testCharFlags( c, CHAR_OPERATOR ) == TRUE) || bLineHasCommentBegin
        || ((!bStarScriptMode) && (c == '\'')) )
    {
        // Kommentar ?
        if ( (( c == '/' ) || bLineHasCommentBegin) || ((!bStarScriptMode) && (c == '\'')) )
        {
            char cNext = peekChar();
            if ( cNext == '/' || ( bStarScriptMode && (cNext == '*' || bLineHasCommentBegin))
                || ((!bStarScriptMode) && (c == '\'')) )    // Kommentar
            {
                if ((c == '*') && (cNext == '/'))   // Kommentarende am Zeilenanfang
                {
                    getChar();                  // Zeichen entfernen

                    bLineHasCommentEnd = TRUE;
                    bLineHasCommentBegin = FALSE;

                    reType = TT_COMMENT;
                }
                else if ( (cNext == '/' && (!bStarScriptMode || !bLineHasCommentBegin))
                    || ((!bStarScriptMode) && (c == '\'')) )// C++ - Kommentar
                {
                    c = getChar();  // '/' entfernen

                    // Alle Zeichen bis Zeilen-Ende oder EOF entfernen
                    char cPeek = peekChar();
                    while( cPeek != CHAR_EOF && testCharFlags( cPeek, CHAR_EOL ) == FALSE )
                    {
                        c = getChar();
                        cPeek = peekChar();

                        if (c == '*' && cPeek == '/')
                        {
                            bLineHasCommentEnd = TRUE;
                        }
                    }

                    reType = TT_COMMENT;
                }
                else if (( cNext == '*' ) || bLineHasCommentBegin)      // C -Kommentar
                {
                    bLineHasCommentBegin = !bLineHasCommentBegin;

                    // Alle Zeichen bis */ entfernen
                    do
                    {
                        c = getChar();
                        cNext = peekChar();

                        // Zeilennummer auch im Kommentar pflegen
                        if( testCharFlags( c, CHAR_EOL ) == TRUE )
                        {
                            // Doppelt-EOL rausschmeissen (CR/LF)
                            if( cNext != c && testCharFlags( cNext, CHAR_EOL ) == TRUE )
                            {
                                c = getChar();
                                cNext = peekChar();
                            }

                            setCommentState(nLine, bLineHasCommentBegin, bLineHasCommentEnd);
                            bLineHasCommentBegin = FALSE;
                            bLineHasCommentEnd = FALSE;

                            // Positions-Daten auf Zeilen-Beginn setzen
                            nCol = 0;
                            nLine++;
                        }
                        else if (c == '*' && cNext == '/')  // am Kommentarende
                        {
                            if (bLineHasCommentBegin)   // das Ende ist in der gleichen Zeile
                            {                           // wie der Anfang des Kommentars
                                bLineHasCommentBegin = FALSE;   // also zurücksetzen
                            }
                            else
                            {
                                bLineHasCommentEnd = TRUE;
                            }
                        }
                    }
                    while( cNext != CHAR_EOF && ( c != '*' || cNext != '/' ) );

                    // Alles ausser EOF lesen
                    if( cNext != CHAR_EOF )
                        getChar();

                    reType = TT_COMMENT;
                }
            }
        }
        // HTML-Kommentar
        else if( c == '<' )
        {
            char cNext = peekChar();
            if( cNext == '!' )
            {
                getChar();     // '!' ist verloren, wenn nicht wirklich Tag

                cNext = peekChar();
                if( cNext == '-' )
                {
                    getChar();     // '-' ist verloren, wenn nicht wirklich Tag

                    cNext = peekChar();
                    if( cNext == '-' )
                    {
                        getChar();

                        // HTML-Kommentar: Alle Zeichen bis Zeilen-Ende oder EOF entfernen
                        char cPeek = peekChar();
                        while( cPeek != CHAR_EOF && testCharFlags( cPeek, CHAR_EOL ) == FALSE )
                        {
                            c = getChar();
                            cPeek = peekChar();
                        }

                        reType = TT_COMMENT;
                    }
                    else
                    {
                        // Verlorene Zeichen nachliefern
                        mpActualPos -= 2;
                    }
                }
                else
                {
                    // Verlorenes Zeichen nachliefern
                    mpActualPos--;
                }
            }
        }

        // Echter Operator, kann hier einfach behandelt werden,
        // da nicht der wirkliche Operator, wie z.B. += interessiert,
        // sondern nur die Tatsache, dass es sich um einen handelt.
        if( reType != TT_COMMENT )
        {
            reType = TT_OPERATOR;
        }
    }

    // Objekt-Trenner? Muss vor Number abgehandelt werden
    else if( c == '.' && ( peekChar() < '0' || peekChar() > '9' ) )
    {
        reType = TT_OPERATOR;
    }

    // Zahl?
    else if( testCharFlags( c, CHAR_START_NUMBER ) == TRUE )
    {
        // Buffer-Position initialisieren
        int nPos = 0;

        // Zahlensystem, 10 = normal, wird bei Oct/Hex geaendert
        int nRadix = 10;

        // Ist es eine Hex- oder Oct-Zahl?
        if( c == '0' )
        {
            // Octal?
            // Java-Script geht von einem Octal-Wert aus, wenn nach 0 eine
            // Ziffer im oktalen Ziffernbereich folgt
            if( testCharFlags( peekChar(), CHAR_IN_OCT_NUMBER ) )
            {
                nRadix = 8;     // Octal-Basis

                // Alle Ziffern einlesen
                while( testCharFlags( peekChar(), CHAR_IN_OCT_NUMBER ) )
                    c = getChar();
            }

            // Dementsprechend wird bei 0x Hex geparsed
            else if( peekChar() == 'x' || peekChar() == 'X' )
            {
                // x entfernen
                getChar();
                nRadix = 16;     // Hex-Basis

                // Alle Ziffern einlesen und puffern
                while( testCharFlags( peekChar(), CHAR_IN_HEX_NUMBER ) )
                    c = getChar();
            }
        }

        // Wenn nicht Oct oder Hex als double ansehen
        if( nRadix == 10 )
        {
            // Flag, ob das letzte Zeichen ein Exponent war
            BOOL bAfterExpChar = FALSE;

            // Alle Ziffern einlesen
            while( testCharFlags( peekChar(), CHAR_IN_NUMBER ) ||
                    (bAfterExpChar && peekChar() == '+' ) ||
                    (bAfterExpChar && peekChar() == '-' ) )
                    // Nach Exponent auch +/- OK
            {
                c = getChar();                  // Zeichen lesen
                bAfterExpChar = ( c == 'e' || c == 'E' );
            }
        }

        reType = TT_NUMBER;
    }

    // String?
    else if( testCharFlags( c, CHAR_START_STRING ) == TRUE )
    {
        // Merken, welches Zeichen den String eroeffnet hat
        char cEndString = c;

        // Alle Ziffern einlesen und puffern
        while( peekChar() != cEndString )
        {
            // #58846 EOF vor getChar() abfangen, damit EOF micht verloren geht
            if( peekChar() == CHAR_EOF )
            {
                // ERROR: unterminated string literal
                reType = TT_ERROR;
                break;
            }
            c = getChar();
            if( testCharFlags( c, CHAR_EOL ) == TRUE )
            {
                // ERROR: unterminated string literal
                reType = TT_ERROR;
                break;
            }
            // Escape-Character untersuchen
            else if ( (c == '\\') && (bStarScriptMode) )
            {
                // Kann hier ganz einfach gehandelt werden:
                // Einfach ein weiteres Zeichen lesen
                char cNext = getChar();
            }
        }

        //  Zeichen lesen
        if( reType != TT_ERROR )
        {
            getChar();
            reType = TT_STRING;
        }
    }

    // Zeilenende?
    else if( testCharFlags( c, CHAR_EOL ) == TRUE )
    {
        // Falls ein weiteres anderes EOL-Char folgt, weg damit
        char cNext = peekChar();
        if( cNext != c && testCharFlags( cNext, CHAR_EOL ) == TRUE )
            getChar();

        setCommentState(nLine, bLineHasCommentBegin, bLineHasCommentEnd);
        bLineHasCommentBegin = FALSE;
        bLineHasCommentEnd = FALSE;

        // Positions-Daten auf Zeilen-Beginn setzen
        nCol = 0;
        nLine++;

        reType = TT_EOL;
    }

    // Alles andere bleibt TT_UNKNOWN


    // End-Position eintragen
    rpEndPos = mpActualPos;
    return TRUE;
}

void SimpleTokenizer_Impl::setCommentState(UINT32 nLine, BOOL bCommentBegin, BOOL bCommentEnd)
{
    while (pCommentsBegin->size() <= nLine)
        pCommentsBegin->push_back(FALSE);

    while (pCommentsEnd->size() <= nLine)
        pCommentsEnd->push_back(FALSE);

    NAMESPACE_STD(list)<BOOL>::iterator posBegins, posEnds;
    UINT32 nCounter = 0;

    posBegins = pCommentsBegin->begin();
    posEnds = pCommentsEnd->begin();

    while (nCounter < nLine)
    {
        posBegins++;
        posEnds++;
        nCounter++;
    }

    *posBegins = bCommentBegin;
    *posEnds = bCommentEnd;
}

void SimpleTokenizer_Impl::addLines(UINT32 nLine, INT32 nCount)
{
    NAMESPACE_STD(list)<BOOL>::iterator posBegins, posEnds;
    UINT32 nCounter = 0;

    if (!pCommentsBegin->empty())
    {
        posBegins = pCommentsBegin->begin();
        posEnds = pCommentsEnd->begin();

        while (nCounter < nLine)
        {
            posBegins++;
            posEnds++;
            nCounter++;
        }

        INT32 nDiff = nCount;
        while (nDiff != 0)
        {
            if (nDiff > 0)
            {
                pCommentsBegin->insert(posBegins, FALSE);
                pCommentsEnd->insert(posEnds, FALSE);
                nDiff--;
            }
            else
            {
                pCommentsBegin->erase(posBegins++);
                pCommentsEnd->erase(posEnds++);
                UINT16 dummy = pCommentsBegin->size();

                nDiff++;
            }
        }
    }
    else if (nCount > 0)
    {
        INT32 nDiff = nCount;
        while (nDiff != 0)
        {
            pCommentsBegin->push_back(FALSE);
            pCommentsEnd->push_back(FALSE);
            nDiff--;
        }
    }
}

String SimpleTokenizer_Impl::getTokStr
    ( /*out*/const char* pStartPos, /*out*/const char* pEndPos )
{
    return String( pStartPos, (USHORT)( pEndPos - pStartPos ) );
}

// TEST: Token ausgeben
String SimpleTokenizer_Impl::getFullTokenStr( /*out*/TokenTypes eType,
    /*out*/const char* pStartPos, /*out*/const char* pEndPos )
{
    String aOut;
    switch( eType )
    {
        case TT_UNKNOWN:    aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_UNKNOWN:") ); break;
        case TT_IDENTIFIER: aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_IDENTIFIER:") ); break;
        case TT_WHITESPACE: aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_WHITESPACE:") ); break;
        case TT_NUMBER:     aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_NUMBER:") ); break;
        case TT_STRING:     aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_STRING:") ); break;
        case TT_EOL:        aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_EOL:") ); break;
        case TT_COMMENT:    aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_COMMENT:") ); break;
        case TT_ERROR:      aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_ERROR:") ); break;
        case TT_OPERATOR:   aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_OPERATOR:") ); break;
        case TT_KEYWORD:    aOut = String( RTL_CONSTASCII_USTRINGPARAM("TT_KEYWORD:") ); break;
    }
    if( eType != TT_EOL )
    {
        aOut += String( pStartPos, (USHORT)( pEndPos - pStartPos ) );
    }
    aOut += String( RTL_CONSTASCII_USTRINGPARAM("\n") );
    return aOut;
}

SimpleTokenizer_Impl::SimpleTokenizer_Impl( void )
{
    memset( aCharTypeTab, 0, sizeof( aCharTypeTab ) );

    // Zeichen-Tabelle fuellen
    USHORT i;

    // Zulaessige Zeichen fuer Identifier
    USHORT nHelpMask = (USHORT)( CHAR_START_IDENTIFIER | CHAR_IN_IDENTIFIER );
    for( i = 'a' ; i <= 'z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    for( i = 'A' ; i <= 'Z' ; i++ )
        aCharTypeTab[i] |= nHelpMask;
    // '_' extra eintragen
    aCharTypeTab['_'] |= nHelpMask;
    // AB 23.6.97: '$' ist auch erlaubt
    aCharTypeTab['$'] |= nHelpMask;

    // Ziffern (Identifier und Number ist moeglich)
    nHelpMask = (USHORT)( CHAR_IN_IDENTIFIER | CHAR_START_NUMBER |
                         CHAR_IN_NUMBER | CHAR_IN_HEX_NUMBER );
    for( i = '0' ; i <= '9' ; i++ )
        aCharTypeTab[i] |= nHelpMask;

    // e und E sowie . von Hand ergaenzen
    aCharTypeTab['e'] |=    CHAR_IN_NUMBER;
    aCharTypeTab['E'] |=    CHAR_IN_NUMBER;
    aCharTypeTab['.'] |=  (USHORT)( CHAR_IN_NUMBER | CHAR_START_NUMBER );

    // Hex-Ziffern
    for( i = 'a' ; i <= 'f' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_HEX_NUMBER;
    for( i = 'A' ; i <= 'F' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_HEX_NUMBER;

    // Oct-Ziffern
    for( i = '0' ; i <= '7' ; i++ )
        aCharTypeTab[i] |= CHAR_IN_OCT_NUMBER;

    // String-Beginn/End-Zeichen
    aCharTypeTab['\''] |= CHAR_START_STRING;
    aCharTypeTab['\"'] |= CHAR_START_STRING;

    // Operator-Zeichen
    aCharTypeTab['!'] |= CHAR_OPERATOR;
    aCharTypeTab['%'] |= CHAR_OPERATOR;
    aCharTypeTab['&'] |= CHAR_OPERATOR;
    aCharTypeTab['('] |= CHAR_OPERATOR;
    aCharTypeTab[')'] |= CHAR_OPERATOR;
    aCharTypeTab['*'] |= CHAR_OPERATOR;
    aCharTypeTab['+'] |= CHAR_OPERATOR;
    aCharTypeTab[','] |= CHAR_OPERATOR;
    aCharTypeTab['-'] |= CHAR_OPERATOR;
    aCharTypeTab['/'] |= CHAR_OPERATOR;
    aCharTypeTab[':'] |= CHAR_OPERATOR;
    aCharTypeTab['<'] |= CHAR_OPERATOR;
    aCharTypeTab['='] |= CHAR_OPERATOR;
    aCharTypeTab['>'] |= CHAR_OPERATOR;
    aCharTypeTab['?'] |= CHAR_OPERATOR;
    aCharTypeTab['^'] |= CHAR_OPERATOR;
    aCharTypeTab['|'] |= CHAR_OPERATOR;
    aCharTypeTab['~'] |= CHAR_OPERATOR;
    aCharTypeTab['{'] |= CHAR_OPERATOR;
    aCharTypeTab['}'] |= CHAR_OPERATOR;
    aCharTypeTab['['] |= CHAR_OPERATOR;
    aCharTypeTab[']'] |= CHAR_OPERATOR;
    aCharTypeTab[';'] |= CHAR_OPERATOR;

    // Space
    aCharTypeTab[' ' ] |= CHAR_SPACE;
    aCharTypeTab['\t'] |= CHAR_SPACE;

    // Zeilen-Ende-Zeichen
    aCharTypeTab['\r'] |= CHAR_EOL;
    aCharTypeTab['\n'] |= CHAR_EOL;

    // fuer Syntax Highlighting
    pCommentsBegin = new NAMESPACE_STD(list)<BOOL>();
    pCommentsEnd = new NAMESPACE_STD(list)<BOOL>();

    bStarScriptMode = FALSE;
    ppListKeyWords = NULL;
}

SimpleTokenizer_Impl::~SimpleTokenizer_Impl( void )
{
    delete(pCommentsBegin);
    delete(pCommentsEnd);
}

SimpleTokenizer_Impl* getSimpleTokenizer( void )
{
    static SimpleTokenizer_Impl* pSimpleTokenizer = NULL;
    if( !pSimpleTokenizer )
        pSimpleTokenizer = new SimpleTokenizer_Impl();
    return pSimpleTokenizer;
}

// Heraussuchen der jeweils naechsten Funktion aus einem JavaScript-Modul
UINT16 SimpleTokenizer_Impl::parseLine( UINT32 nParseLine, const String* aSource )
{
    ByteString aByteSource( *aSource, gsl_getSystemTextEncoding() );

    // Position auf den Anfang des Source-Strings setzen
    mpStringBegin = mpActualPos = aByteSource.GetBuffer();
    bLineHasCommentBegin = isBeginComment( nParseLine );
    bLineHasCommentEnd = FALSE;

    // Zeile und Spalte initialisieren
    nLine = nParseLine;
    nCol = 0L;

    // Variablen fuer die Out-Parameter
    TokenTypes eType;
    const char* pStartPos;
    const char* pEndPos;

    // Schleife ueber alle Tokens
    UINT16 nTokenCount = 0;
    while( getNextToken( eType, pStartPos, pEndPos ) )
        nTokenCount++;

    // die Endzustaende der Zeilen in die Listen eintragen
    setCommentState(nParseLine, bLineHasCommentBegin, bLineHasCommentEnd);

    return nTokenCount;
}

void SimpleTokenizer_Impl::getHighlightPortions( UINT32 nParseLine, const String& rLine,
                                                    /*out*/HighlightPortions& portions  )
{
    ByteString aByteLine( rLine, gsl_getSystemTextEncoding() );

    // Position auf den Anfang des Source-Strings setzen
    mpStringBegin = mpActualPos = aByteLine.GetBuffer();
    bLineHasCommentBegin = isBeginComment( nParseLine );
    bLineHasCommentEnd = FALSE;

    // Zeile und Spalte initialisieren
    nLine = nParseLine;
    nCol = 0L;

    // Variablen fuer die Out-Parameter
    TokenTypes eType;
    const char* pStartPos;
    const char* pEndPos;

    // Schleife ueber alle Tokens
    while( getNextToken( eType, pStartPos, pEndPos ) )
    {
        HighlightPortion portion;

        portion.nBegin = (UINT16)(pStartPos - mpStringBegin);
        portion.nEnd = (UINT16)(pEndPos - mpStringBegin);
        portion.tokenType = eType;

        portions.Insert(portion, portions.Count());
    }
}

BOOL SimpleTokenizer_Impl::isBeginComment( UINT32 nLine )
{
    NAMESPACE_STD(list)<BOOL>::const_iterator posBegin, posEnd;
    BOOL bCommentStart = FALSE;

    UINT32 i = 0;
    posBegin=pCommentsBegin->begin();
    posEnd=pCommentsEnd->begin();

    while ((i < nLine) && (posBegin != pCommentsBegin->end()) && (posEnd != pCommentsEnd->end()))
    {
        if (bCommentStart && *posEnd)
            bCommentStart = FALSE;
        if ((!bCommentStart) && *posBegin)
            bCommentStart = TRUE;

        posBegin++;
        posEnd++;
        i++;
    }

    return bCommentStart;
}

void SimpleTokenizer_Impl::outCommentList()
{
    NAMESPACE_STD(list)<BOOL>::const_iterator posBegin, posEnd;
    BOOL bCommentStart = FALSE;

    UINT32 i = 0;
    posBegin=pCommentsBegin->begin();
    posEnd=pCommentsEnd->begin();

    printf("\nComments:\n");
    while (posBegin != pCommentsBegin->end())
    {
        printf("line: %2d   beginComment: %d   endComment: %d\n", i, *posBegin, *posEnd);

        posBegin++;
        posEnd++;
        i++;
    }

}

//////////////////////////////////////////////////////////////////////////
// Implementierung des SyntaxHighlighter

SyntaxHighlighter::SyntaxHighlighter()
{
    m_pSimpleTokenizer = new SimpleTokenizer_Impl();
    m_pKeyWords = NULL;
    m_nKeyWordCount = 0;
}

SyntaxHighlighter::~SyntaxHighlighter()
{
    delete(m_pSimpleTokenizer);
    delete(m_pKeyWords);
}

void SyntaxHighlighter::initialize( HighlighterLanguage eLanguage_ )
{
    eLanguage = eLanguage_;
    delete(m_pSimpleTokenizer);
    m_pSimpleTokenizer = new SimpleTokenizer_Impl();

    if (eLanguage == HIGHLIGHT_BASIC)
    {
        m_pSimpleTokenizer->setKeyWords( strListBasicKeyWords,
                                            sizeof( strListBasicKeyWords ) / sizeof( char* ));
    }
    else
    {
        m_pSimpleTokenizer->setKeyWords( NULL, 0 );
    }
}

const Range SyntaxHighlighter::notifyChange( UINT32 nLine, INT32 nLineCountDifference,
                                const String* pChangedLines, UINT32 nArrayLength)
{
    if (nLineCountDifference != 0)
        m_pSimpleTokenizer->addLines(nLine, nLineCountDifference);

    for (INT32 i=0; i<nArrayLength; i++)
        m_pSimpleTokenizer->parseLine(nLine+i, &pChangedLines[i]);

    return Range(nLine, nLine+nArrayLength-1);
}

void SyntaxHighlighter::getHighlightPortions( UINT32 nLine, const String& rLine,
                                            /*out*/HighlightPortions& portions )
{
    m_pSimpleTokenizer->getHighlightPortions( nLine, rLine, portions );
}


/////////////////////////////////////////////////////////////////////////
// Implementation SbJScriptModule (Basic-Modul fuer JavaScript-Sourcen)
SbJScriptModule::SbJScriptModule( const String& rName )
    :SbModule( rName )
{
}

BOOL SbJScriptModule::LoadData( SvStream& rStrm, USHORT nVer )
{
    Clear();
    if( !SbxObject::LoadData( rStrm, 1 ) )
        return FALSE;

    // Source-String holen
    rStrm.ReadByteString( aSource, gsl_getSystemTextEncoding() );
    //rStrm >> aSource;
    return TRUE;
}

BOOL SbJScriptModule::StoreData( SvStream& rStrm ) const
{
    if( !SbxObject::StoreData( rStrm ) )
        return FALSE;

    // Source-String schreiben
    rStrm.WriteByteString( aSource, gsl_getSystemTextEncoding() );
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
    // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
    SetFlag( SBX_NO_MODIFY );
}

SbMethod::~SbMethod()
{}

SbxArray* SbMethod::GetLocals()
{
    if( pINST )
        return pINST->GetLocals( this );
    else
        return NULL;
}

SbxArray* SbMethod::GetStatics()
{
    DBG_ERROR( "SbMethod::GetStatics() invalid, AB fragen" )
    return NULL;
}

BOOL SbMethod::LoadData( SvStream& rStrm, USHORT nVer )
{
    if( !SbxMethod::LoadData( rStrm, 1 ) )
        return FALSE;
    INT16 n;
    rStrm >> n;
    // nDebugFlags = n;     // AB 16.1.96: Nicht mehr uebernehmen
    if( nVer == 2 )
        rStrm >> nLine1 >> nLine2 >> nStart >> bInvalid;
    // AB: 2.7.1996: HACK wegen 'Referenz kann nicht gesichert werden'
    SetFlag( SBX_NO_MODIFY );
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
    SbModule* pMod = (SbModule*)GetParent();
    pMod->AddRef();

    // RefCount vom Basic hochzaehlen
    StarBASIC* pBasic = (StarBASIC*)pMod->GetParent();
    pBasic->AddRef();

    // Values anlegen, um Return-Wert zu erhalten
    SbxValues aVals;
    aVals.eType = SbxVARIANT;
    Get( aVals );
    if ( pRet )
        pRet->Put( aVals );

    // Gab es einen Error
    ErrCode nErr = SbxBase::GetError();
    SbxBase::ResetError();

    // Objekte freigeben
    pMod->ReleaseRef();
    pBasic->ReleaseRef();

    return nErr;
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


