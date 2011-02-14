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
#include <tools/fsys.hxx>
#include <vcl/svapp.hxx>
#include <tools/wldcrd.hxx>
#include <svl/zforlist.hxx>
#include <unotools/syslocale.hxx>
#include "runtime.hxx"
#include "sbintern.hxx"
#include "opcodes.hxx"
#include "codegen.hxx"
#include "iosys.hxx"
#include "image.hxx"
#include "ddectrl.hxx"
#include "dllmgr.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include "sbunoobj.hxx"
#include "errobject.hxx"
#include "sbtrace.hxx"
#include "comenumwrapper.hxx"

using namespace ::com::sun::star;

bool SbiRuntime::isVBAEnabled()
{
    bool result = false;
    SbiInstance* pInst = pINST;
    if ( pInst && pINST->pRun )
        result = pInst->pRun->bVBAEnabled;
    return result;
}

// #91147 Global reschedule flag
static sal_Bool bStaticGlobalEnableReschedule = sal_True;

void StarBASIC::StaticEnableReschedule( sal_Bool bReschedule )
{
    bStaticGlobalEnableReschedule = bReschedule;
}
void StarBASIC::SetVBAEnabled( sal_Bool bEnabled )
{
    if ( bDocBasic )
    {
        bVBAEnabled = bEnabled;
    }
}

sal_Bool StarBASIC::isVBAEnabled()
{
    if ( bDocBasic )
    {
        if( SbiRuntime::isVBAEnabled() )
            return sal_True;
        return bVBAEnabled;
    }
    return sal_False;
}


struct SbiArgvStack {                   // Argv stack:
    SbiArgvStack*  pNext;               // Stack Chain
    SbxArrayRef    refArgv;             // Argv
    short nArgc;                        // Argc
};

SbiRuntime::pStep0 SbiRuntime::aStep0[] = { // Alle Opcodes ohne Operanden
    &SbiRuntime::StepNOP,
    &SbiRuntime::StepEXP,
    &SbiRuntime::StepMUL,
    &SbiRuntime::StepDIV,
    &SbiRuntime::StepMOD,
    &SbiRuntime::StepPLUS,
    &SbiRuntime::StepMINUS,
    &SbiRuntime::StepNEG,
    &SbiRuntime::StepEQ,
    &SbiRuntime::StepNE,
    &SbiRuntime::StepLT,
    &SbiRuntime::StepGT,
    &SbiRuntime::StepLE,
    &SbiRuntime::StepGE,
    &SbiRuntime::StepIDIV,
    &SbiRuntime::StepAND,
    &SbiRuntime::StepOR,
    &SbiRuntime::StepXOR,
    &SbiRuntime::StepEQV,
    &SbiRuntime::StepIMP,
    &SbiRuntime::StepNOT,
    &SbiRuntime::StepCAT,

    &SbiRuntime::StepLIKE,
    &SbiRuntime::StepIS,
    // Laden/speichern
    &SbiRuntime::StepARGC,      // neuen Argv einrichten
    &SbiRuntime::StepARGV,      // TOS ==> aktueller Argv
    &SbiRuntime::StepINPUT,     // Input ==> TOS
    &SbiRuntime::StepLINPUT,        // Line Input ==> TOS
    &SbiRuntime::StepGET,        // TOS anfassen
    &SbiRuntime::StepSET,        // Speichern Objekt TOS ==> TOS-1
    &SbiRuntime::StepPUT,       // TOS ==> TOS-1
    &SbiRuntime::StepPUTC,      // TOS ==> TOS-1, dann ReadOnly
    &SbiRuntime::StepDIM,       // DIM
    &SbiRuntime::StepREDIM,         // REDIM
    &SbiRuntime::StepREDIMP,        // REDIM PRESERVE
    &SbiRuntime::StepERASE,         // TOS loeschen
    // Verzweigen
    &SbiRuntime::StepSTOP,          // Programmende
    &SbiRuntime::StepINITFOR,   // FOR-Variable initialisieren
    &SbiRuntime::StepNEXT,      // FOR-Variable inkrementieren
    &SbiRuntime::StepCASE,      // Anfang CASE
    &SbiRuntime::StepENDCASE,   // Ende CASE
    &SbiRuntime::StepSTDERROR,      // Standard-Fehlerbehandlung
    &SbiRuntime::StepNOERROR,   // keine Fehlerbehandlung
    &SbiRuntime::StepLEAVE,     // UP verlassen
    // E/A
    &SbiRuntime::StepCHANNEL,   // TOS = Kanalnummer
    &SbiRuntime::StepPRINT,     // print TOS
    &SbiRuntime::StepPRINTF,        // print TOS in field
    &SbiRuntime::StepWRITE,     // write TOS
    &SbiRuntime::StepRENAME,        // Rename Tos+1 to Tos
    &SbiRuntime::StepPROMPT,        // Input Prompt aus TOS definieren
    &SbiRuntime::StepRESTART,   // Set restart point
    &SbiRuntime::StepCHANNEL0,  // E/A-Kanal 0 einstellen
    &SbiRuntime::StepEMPTY,     // Leeren Ausdruck auf Stack
    &SbiRuntime::StepERROR,     // TOS = Fehlercode
    &SbiRuntime::StepLSET,      // Speichern Objekt TOS ==> TOS-1
    &SbiRuntime::StepRSET,      // Speichern Objekt TOS ==> TOS-1
    &SbiRuntime::StepREDIMP_ERASE,// Copy array object for REDIMP
    &SbiRuntime::StepINITFOREACH,// Init for each loop
    &SbiRuntime::StepVBASET,// vba-like set statement
    &SbiRuntime::StepERASE_CLEAR,// vba-like set statement
    &SbiRuntime::StepARRAYACCESS,// access TOS as array
    &SbiRuntime::StepBYVAL,     // access TOS as array
};

SbiRuntime::pStep1 SbiRuntime::aStep1[] = { // Alle Opcodes mit einem Operanden
    &SbiRuntime::StepLOADNC,        // Laden einer numerischen Konstanten (+ID)
    &SbiRuntime::StepLOADSC,        // Laden einer Stringkonstanten (+ID)
    &SbiRuntime::StepLOADI,     // Immediate Load (+Wert)
    &SbiRuntime::StepARGN,      // Speichern eines named Args in Argv (+StringID)
    &SbiRuntime::StepPAD,       // String auf feste Laenge bringen (+Laenge)
    // Verzweigungen
    &SbiRuntime::StepJUMP,      // Sprung (+Target)
    &SbiRuntime::StepJUMPT,     // TOS auswerten), bedingter Sprung (+Target)
    &SbiRuntime::StepJUMPF,     // TOS auswerten), bedingter Sprung (+Target)
    &SbiRuntime::StepONJUMP,        // TOS auswerten), Sprung in JUMP-Tabelle (+MaxVal)
    &SbiRuntime::StepGOSUB,     // UP-Aufruf (+Target)
    &SbiRuntime::StepRETURN,        // UP-Return (+0 oder Target)
    &SbiRuntime::StepTESTFOR,   // FOR-Variable testen), inkrementieren (+Endlabel)
    &SbiRuntime::StepCASETO,        // Tos+1 <= Case <= Tos), 2xremove (+Target)
    &SbiRuntime::StepERRHDL,        // Fehler-Handler (+Offset)
    &SbiRuntime::StepRESUME,        // Resume nach Fehlern (+0 or 1 or Label)
    // E/A
    &SbiRuntime::StepCLOSE,     // (+Kanal/0)
    &SbiRuntime::StepPRCHAR,        // (+char)
    // Verwaltung
    &SbiRuntime::StepSETCLASS,  // Set + Klassennamen testen (+StringId)
    &SbiRuntime::StepTESTCLASS, // Check TOS class (+StringId)
    &SbiRuntime::StepLIB,       // Lib fuer Declare-Call (+StringId)
    &SbiRuntime::StepBASED,     // TOS wird um BASE erhoeht, BASE davor gepusht
    &SbiRuntime::StepARGTYP,        // Letzten Parameter in Argv konvertieren (+Typ)
    &SbiRuntime::StepVBASETCLASS,// vba-like set statement
};

SbiRuntime::pStep2 SbiRuntime::aStep2[] = {// Alle Opcodes mit zwei Operanden
    &SbiRuntime::StepRTL,       // Laden aus RTL (+StringID+Typ)
    &SbiRuntime::StepFIND,      // Laden (+StringID+Typ)
    &SbiRuntime::StepELEM,          // Laden Element (+StringID+Typ)
    &SbiRuntime::StepPARAM,     // Parameter (+Offset+Typ)
    // Verzweigen
    &SbiRuntime::StepCALL,      // Declare-Call (+StringID+Typ)
    &SbiRuntime::StepCALLC,     // CDecl-Declare-Call (+StringID+Typ)
    &SbiRuntime::StepCASEIS,        // Case-Test (+Test-Opcode+False-Target)
    // Verwaltung
    &SbiRuntime::StepSTMNT,         // Beginn eines Statements (+Line+Col)
    // E/A
    &SbiRuntime::StepOPEN,          // (+SvStreamFlags+Flags)
    // Objekte
    &SbiRuntime::StepLOCAL,     // Lokale Variable definieren (+StringId+Typ)
    &SbiRuntime::StepPUBLIC,        // Modulglobale Variable (+StringID+Typ)
    &SbiRuntime::StepGLOBAL,        // Globale Variable definieren (+StringID+Typ)
    &SbiRuntime::StepCREATE,        // Objekt kreieren (+StringId+StringId)
    &SbiRuntime::StepSTATIC,     // Statische Variable (+StringId+StringId)
    &SbiRuntime::StepTCREATE,    // User Defined Objekte (+StringId+StringId)
    &SbiRuntime::StepDCREATE,    // Objekt-Array kreieren (+StringID+StringID)
    &SbiRuntime::StepGLOBAL_P,   // Globale Variable definieren, die beim Neustart
                                        // von Basic nicht ueberschrieben wird (+StringID+Typ)
    &SbiRuntime::StepFIND_G,        // Sucht globale Variable mit Spezialbehandlung wegen _GLOBAL_P
    &SbiRuntime::StepDCREATE_REDIMP, // Objekt-Array redimensionieren (+StringID+StringID)
    &SbiRuntime::StepFIND_CM,    // Search inside a class module (CM) to enable global search in time
    &SbiRuntime::StepPUBLIC_P,    // Search inside a class module (CM) to enable global search in time
    &SbiRuntime::StepFIND_STATIC,    // Search inside a class module (CM) to enable global search in time
};


//////////////////////////////////////////////////////////////////////////
//                              SbiRTLData                              //
//////////////////////////////////////////////////////////////////////////

SbiRTLData::SbiRTLData()
{
    pDir        = 0;
    nDirFlags   = 0;
    nCurDirPos  = 0;
    pWildCard   = NULL;
}

SbiRTLData::~SbiRTLData()
{
    delete pDir;
    pDir = 0;
    delete pWildCard;
}

//////////////////////////////////////////////////////////////////////////
//                              SbiInstance                             //
//////////////////////////////////////////////////////////////////////////

// 16.10.96: #31460 Neues Konzept fuer StepInto/Over/Out
// Die Entscheidung, ob StepPoint aufgerufen werden soll, wird anhand des
// CallLevels getroffen. Angehalten wird, wenn der aktuelle CallLevel <=
// nBreakCallLvl ist. Der aktuelle CallLevel kann niemals kleiner als 1
// sein, da er beim Aufruf einer Methode (auch main) inkrementiert wird.
// Daher bedeutet ein BreakCallLvl von 0, dass das Programm gar nicht
// angehalten wird.
// (siehe auch step2.cxx, SbiRuntime::StepSTMNT() )

// Hilfsfunktion, um den BreakCallLevel gemaess der der Debug-Flags zu ermitteln
void SbiInstance::CalcBreakCallLevel( sal_uInt16 nFlags )
{
    // Break-Flag wegfiltern
    nFlags &= ~((sal_uInt16)SbDEBUG_BREAK);

    sal_uInt16 nRet;
    switch( nFlags )
    {
        case SbDEBUG_STEPINTO:
            nRet = nCallLvl + 1;    // CallLevel+1 wird auch angehalten
            break;
        case SbDEBUG_STEPOVER | SbDEBUG_STEPINTO:
            nRet = nCallLvl;        // Aktueller CallLevel wird angehalten
            break;
        case SbDEBUG_STEPOUT:
            nRet = nCallLvl - 1;    // Kleinerer CallLevel wird angehalten
            break;
        case SbDEBUG_CONTINUE:
        // Basic-IDE liefert 0 statt SbDEBUG_CONTINUE, also auch default=continue
        default:
            nRet = 0;               // CallLevel ist immer >0 -> kein StepPoint
    }
    nBreakCallLvl = nRet;           // Ergebnis uebernehmen
}

SbiInstance::SbiInstance( StarBASIC* p )
{
    pBasic   = p;
    pNext    = NULL;
    pRun     = NULL;
    pIosys   = new SbiIoSystem;
    pDdeCtrl = new SbiDdeControl;
    pDllMgr  = 0; // on demand
    pNumberFormatter = 0; // on demand
    nCallLvl = 0;
    nBreakCallLvl = 0;
    nErr     =
    nErl     = 0;
    bReschedule = sal_True;
    bCompatibility = sal_False;
}

SbiInstance::~SbiInstance()
{
    while( pRun )
    {
        SbiRuntime* p = pRun->pNext;
        delete pRun;
        pRun = p;
    }
    delete pIosys;
    delete pDdeCtrl;
    delete pDllMgr;
    delete pNumberFormatter;

    try
    {
        int nSize = ComponentVector.size();
        if( nSize )
        {
            for( int i = nSize - 1 ; i >= 0 ; --i )
            {
                Reference< XComponent > xDlgComponent = ComponentVector[i];
                if( xDlgComponent.is() )
                    xDlgComponent->dispose();
            }
        }
    }
    catch( const Exception& )
    {
        DBG_ERROR( "SbiInstance::~SbiInstance: caught an exception while disposing the components!" );
    }

    ComponentVector.clear();
}

SbiDllMgr* SbiInstance::GetDllMgr()
{
    if( !pDllMgr )
        pDllMgr = new SbiDllMgr;
    return pDllMgr;
}

// #39629 NumberFormatter jetzt ueber statische Methode anlegen
SvNumberFormatter* SbiInstance::GetNumberFormatter()
{
    LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();
    SvtSysLocale aSysLocale;
    DateFormat eDate = aSysLocale.GetLocaleData().getDateFormat();
    if( pNumberFormatter )
    {
        if( eLangType != meFormatterLangType ||
            eDate != meFormatterDateFormat )
        {
            delete pNumberFormatter;
            pNumberFormatter = NULL;
        }
    }
    meFormatterLangType = eLangType;
    meFormatterDateFormat = eDate;
    if( !pNumberFormatter )
        PrepareNumberFormatter( pNumberFormatter, nStdDateIdx, nStdTimeIdx, nStdDateTimeIdx,
        &meFormatterLangType, &meFormatterDateFormat );
    return pNumberFormatter;
}

// #39629 NumberFormatter auch statisch anbieten
void SbiInstance::PrepareNumberFormatter( SvNumberFormatter*& rpNumberFormatter,
    sal_uInt32 &rnStdDateIdx, sal_uInt32 &rnStdTimeIdx, sal_uInt32 &rnStdDateTimeIdx,
    LanguageType* peFormatterLangType, DateFormat* peFormatterDateFormat )
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
        xFactory = comphelper::getProcessServiceFactory();

    LanguageType eLangType;
    if( peFormatterLangType )
        eLangType = *peFormatterLangType;
    else
        eLangType = GetpApp()->GetSettings().GetLanguage();

    DateFormat eDate;
    if( peFormatterDateFormat )
        eDate = *peFormatterDateFormat;
    else
    {
        SvtSysLocale aSysLocale;
        eDate = aSysLocale.GetLocaleData().getDateFormat();
    }

    rpNumberFormatter = new SvNumberFormatter( xFactory, eLangType );

    xub_StrLen nCheckPos = 0; short nType;
    rnStdTimeIdx = rpNumberFormatter->GetStandardFormat( NUMBERFORMAT_TIME, eLangType );

    // Standard-Vorlagen des Formatters haben nur zweistellige
    // Jahreszahl. Deshalb eigenes Format registrieren

    // HACK, da der Numberformatter in PutandConvertEntry die Platzhalter
    // fuer Monat, Tag, Jahr nicht entsprechend der Systemeinstellung
    // austauscht. Problem: Print Year(Date) unter engl. BS
    // siehe auch svtools\source\sbx\sbxdate.cxx

    String aDateStr;
    switch( eDate )
    {
        case MDY: aDateStr = String( RTL_CONSTASCII_USTRINGPARAM("MM.TT.JJJJ") ); break;
        case DMY: aDateStr = String( RTL_CONSTASCII_USTRINGPARAM("TT.MM.JJJJ") ); break;
        case YMD: aDateStr = String( RTL_CONSTASCII_USTRINGPARAM("JJJJ.MM.TT") ); break;
        default:  aDateStr = String( RTL_CONSTASCII_USTRINGPARAM("MM.TT.JJJJ") );
    }
    String aStr( aDateStr );
    rpNumberFormatter->PutandConvertEntry( aStr, nCheckPos, nType,
        rnStdDateIdx, LANGUAGE_GERMAN, eLangType );
    nCheckPos = 0;
    String aStrHHMMSS( RTL_CONSTASCII_USTRINGPARAM(" HH:MM:SS") );
    aStr = aDateStr;
    aStr += aStrHHMMSS;
    rpNumberFormatter->PutandConvertEntry( aStr, nCheckPos, nType,
        rnStdDateTimeIdx, LANGUAGE_GERMAN, eLangType );
}



// Engine laufenlassen. Falls Flags == SbDEBUG_CONTINUE, Flags uebernehmen

void SbiInstance::Stop()
{
    for( SbiRuntime* p = pRun; p; p = p->pNext )
        p->Stop();
}

// Allows Basic IDE to set watch mode to suppress errors
static bool bWatchMode = false;

void setBasicWatchMode( bool bOn )
{
    bWatchMode = bOn;
}

void SbiInstance::Error( SbError n )
{
    Error( n, String() );
}

void SbiInstance::Error( SbError n, const String& rMsg )
{
    if( !bWatchMode )
    {
        aErrorMsg = rMsg;
        pRun->Error( n );
    }
}

void SbiInstance::ErrorVB( sal_Int32 nVBNumber, const String& rMsg )
{
    if( !bWatchMode )
    {
        SbError n = StarBASIC::GetSfxFromVBError( static_cast< sal_uInt16 >( nVBNumber ) );
        if ( !n )
            n = nVBNumber; // force orig number, probably should have a specific table of vb ( localized ) errors

        aErrorMsg = rMsg;
        SbiRuntime::translateErrorToVba( n, aErrorMsg );

        bool bVBATranslationAlreadyDone = true;
        pRun->Error( SbERR_BASIC_COMPAT, bVBATranslationAlreadyDone );
    }
}

void SbiInstance::setErrorVB( sal_Int32 nVBNumber, const String& rMsg )
{
    SbError n = StarBASIC::GetSfxFromVBError( static_cast< sal_uInt16 >( nVBNumber ) );
    if( !n )
        n = nVBNumber; // force orig number, probably should have a specific table of vb ( localized ) errors

    aErrorMsg = rMsg;
    SbiRuntime::translateErrorToVba( n, aErrorMsg );

    nErr = n;
}


void SbiInstance::FatalError( SbError n )
{
    pRun->FatalError( n );
}

void SbiInstance::FatalError( SbError _errCode, const String& _details )
{
    pRun->FatalError( _errCode, _details );
}

void SbiInstance::Abort()
{
    // Basic suchen, in dem der Fehler auftrat
    StarBASIC* pErrBasic = GetCurrentBasic( pBasic );
    pErrBasic->RTError( nErr, aErrorMsg, pRun->nLine, pRun->nCol1, pRun->nCol2 );
    pBasic->Stop();
}

// Hilfsfunktion, um aktives Basic zu finden, kann ungleich pRTBasic sein
StarBASIC* GetCurrentBasic( StarBASIC* pRTBasic )
{
    StarBASIC* pCurBasic = pRTBasic;
    SbModule* pActiveModule = pRTBasic->GetActiveModule();
    if( pActiveModule )
    {
        SbxObject* pParent = pActiveModule->GetParent();
        if( pParent && pParent->ISA(StarBASIC) )
            pCurBasic = (StarBASIC*)pParent;
    }
    return pCurBasic;
}

SbModule* SbiInstance::GetActiveModule()
{
    if( pRun )
        return pRun->GetModule();
    else
        return NULL;
}

SbMethod* SbiInstance::GetCaller( sal_uInt16 nLevel )
{
    SbiRuntime* p = pRun;
    while( nLevel-- && p )
        p = p->pNext;
    if( p )
        return p->GetCaller();
    else
        return NULL;
}

SbxArray* SbiInstance::GetLocals( SbMethod* pMeth )
{
    SbiRuntime* p = pRun;
    while( p && p->GetMethod() != pMeth )
        p = p->pNext;
    if( p )
        return p->GetLocals();
    else
        return NULL;
}

//////////////////////////////////////////////////////////////////////////
//                              SbiInstance                             //
//////////////////////////////////////////////////////////////////////////

// Achtung: pMeth kann auch NULL sein (beim Aufruf des Init-Codes)

SbiRuntime::SbiRuntime( SbModule* pm, SbMethod* pe, sal_uInt32 nStart )
         : rBasic( *(StarBASIC*)pm->pParent ), pInst( pINST ),
           pMod( pm ), pMeth( pe ), pImg( pMod->pImage ), m_nLastTime(0)
{
    nFlags    = pe ? pe->GetDebugFlags() : 0;
    pIosys    = pInst->pIosys;
    pArgvStk  = NULL;
    pGosubStk = NULL;
    pForStk   = NULL;
    pError    = NULL;
    pErrCode  =
    pErrStmnt =
    pRestart  = NULL;
    pNext     = NULL;
    pCode     =
    pStmnt    = (const sal_uInt8* ) pImg->GetCode() + nStart;
    bRun      =
    bError    = sal_True;
    bInError  = sal_False;
    bBlocked  = sal_False;
    nLine     = 0;
    nCol1     = 0;
    nCol2     = 0;
    nExprLvl  = 0;
    nArgc     = 0;
    nError    = 0;
    nGosubLvl = 0;
    nForLvl   = 0;
    nOps      = 0;
    refExprStk = new SbxArray;
    SetVBAEnabled( pMod->IsVBACompat() );
#if defined GCC
    SetParameters( pe ? pe->GetParameters() : (class SbxArray *)NULL );
#else
    SetParameters( pe ? pe->GetParameters() : NULL );
#endif
    pRefSaveList = NULL;
    pItemStoreList = NULL;
}

SbiRuntime::~SbiRuntime()
{
    ClearGosubStack();
    ClearArgvStack();
    ClearForStack();

    // #74254 Items zum Sichern temporaere Referenzen freigeben
    ClearRefs();
    while( pItemStoreList )
    {
        RefSaveItem* pToDeleteItem = pItemStoreList;
        pItemStoreList = pToDeleteItem->pNext;
        delete pToDeleteItem;
    }
}

void SbiRuntime::SetVBAEnabled(bool bEnabled )
{
    bVBAEnabled = bEnabled;
}

// Aufbau der Parameterliste. Alle ByRef-Parameter werden direkt
// uebernommen; von ByVal-Parametern werden Kopien angelegt. Falls
// ein bestimmter Datentyp verlangt wird, wird konvertiert.

void SbiRuntime::SetParameters( SbxArray* pParams )
{
    refParams = new SbxArray;
    // fuer den Returnwert
    refParams->Put( pMeth, 0 );

    SbxInfo* pInfo = pMeth ? pMeth->GetInfo() : NULL;
    sal_uInt16 nParamCount = pParams ? pParams->Count() : 1;
    if( nParamCount > 1 )
    {
        for( sal_uInt16 i = 1 ; i < nParamCount ; i++ )
        {
            const SbxParamInfo* p = pInfo ? pInfo->GetParam( i ) : NULL;

            // #111897 ParamArray
            if( p && (p->nUserData & PARAM_INFO_PARAMARRAY) != 0 )
            {
                SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
                sal_uInt16 nParamArrayParamCount = nParamCount - i;
                pArray->unoAddDim( 0, nParamArrayParamCount - 1 );
                for( sal_uInt16 j = i ; j < nParamCount ; j++ )
                {
                    SbxVariable* v = pParams->Get( j );
                    short nDimIndex = j - i;
                    pArray->Put( v, &nDimIndex );
                }
                SbxVariable* pArrayVar = new SbxVariable( SbxVARIANT );
                pArrayVar->SetFlag( SBX_READWRITE );
                pArrayVar->PutObject( pArray );
                refParams->Put( pArrayVar, i );

                // Block ParamArray for missing parameter
                pInfo = NULL;
                break;
            }

            SbxVariable* v = pParams->Get( i );
            // Methoden sind immer byval!
            sal_Bool bByVal = v->IsA( TYPE(SbxMethod) );
            SbxDataType t = v->GetType();
            if( p )
            {
                bByVal |= sal_Bool( ( p->eType & SbxBYREF ) == 0 );
                t = (SbxDataType) ( p->eType & 0x0FFF );

                if( !bByVal && t != SbxVARIANT &&
                    (!v->IsFixed() || (SbxDataType)(v->GetType() & 0x0FFF ) != t) )
                        bByVal = sal_True;
            }
            if( bByVal )
            {
                SbxVariable* v2 = new SbxVariable( t );
                v2->SetFlag( SBX_READWRITE );
                *v2 = *v;
                refParams->Put( v2, i );
            }
            else
            {
                if( t != SbxVARIANT && t != ( v->GetType() & 0x0FFF ) )
                {
                    // Array konvertieren??
                    if( p && (p->eType & SbxARRAY) )
                        Error( SbERR_CONVERSION );
                    else
                        v->Convert( t );
                }
                refParams->Put( v, i );
            }
            if( p )
                refParams->PutAlias( p->aName, i );
        }
    }

    // ParamArray for missing parameter
    if( pInfo )
    {
        // #111897 Check first missing parameter for ParamArray
        const SbxParamInfo* p = pInfo->GetParam( nParamCount );
        if( p && (p->nUserData & PARAM_INFO_PARAMARRAY) != 0 )
        {
            SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
            pArray->unoAddDim( 0, -1 );
            SbxVariable* pArrayVar = new SbxVariable( SbxVARIANT );
            pArrayVar->SetFlag( SBX_READWRITE );
            pArrayVar->PutObject( pArray );
            refParams->Put( pArrayVar, nParamCount );
        }
    }
}


// Einen P-Code ausfuehren

sal_Bool SbiRuntime::Step()
{
    if( bRun )
    {
        // Unbedingt gelegentlich die Kontrolle abgeben!
        if( !( ++nOps & 0xF ) && pInst->IsReschedule() && bStaticGlobalEnableReschedule )
        {
            sal_uInt32 nTime = osl_getGlobalTimer();
            if (nTime - m_nLastTime > 5 ) // 20 ms
            {
                Application::Reschedule();
                m_nLastTime = nTime;
            }
        }

        // #i48868 blocked by next call level?
        while( bBlocked )
        {
            if( pInst->IsReschedule() && bStaticGlobalEnableReschedule )
                Application::Reschedule();
        }

#ifdef DBG_TRACE_BASIC
        sal_uInt32 nPC = ( pCode - (const sal_uInt8* )pImg->GetCode() );
        dbg_traceStep( pMod, nPC, pINST->nCallLvl );
#endif

        SbiOpcode eOp = (SbiOpcode ) ( *pCode++ );
        sal_uInt32 nOp1, nOp2;
        if( eOp <= SbOP0_END )
        {
            (this->*( aStep0[ eOp ] ) )();
        }
        else if( eOp >= SbOP1_START && eOp <= SbOP1_END )
        {
            nOp1 = *pCode++; nOp1 |= *pCode++ << 8; nOp1 |= *pCode++ << 16; nOp1 |= *pCode++ << 24;

            (this->*( aStep1[ eOp - SbOP1_START ] ) )( nOp1 );
        }
        else if( eOp >= SbOP2_START && eOp <= SbOP2_END )
        {
            nOp1 = *pCode++; nOp1 |= *pCode++ << 8; nOp1 |= *pCode++ << 16; nOp1 |= *pCode++ << 24;
            nOp2 = *pCode++; nOp2 |= *pCode++ << 8; nOp2 |= *pCode++ << 16; nOp2 |= *pCode++ << 24;
            (this->*( aStep2[ eOp - SbOP2_START ] ) )( nOp1, nOp2 );
        }
        else
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );

        // SBX-Fehler aufgetreten?
        SbError nSbError = SbxBase::GetError();
        Error( ERRCODE_TOERROR(nSbError) );         // Warnings rausfiltern

        // AB 13.2.1997, neues Error-Handling:
        // ACHTUNG: Hier kann nError auch dann gesetzt sein, wenn !nSbError,
        // da nError jetzt auch von anderen RT-Instanzen gesetzt werden kann

        if( nError )
            SbxBase::ResetError();

        // AB,15.3.96: Fehler nur anzeigen, wenn BASIC noch aktiv
        // (insbesondere nicht nach Compiler-Fehlern zur Laufzeit)
        if( nError && bRun )
        {
#ifdef DBG_TRACE_BASIC
            SbError nTraceErr = nError;
            String aTraceErrMsg = GetSbData()->aErrMsg;
            bool bTraceErrHandled = true;
#endif
            SbError err = nError;
            ClearExprStack();
            nError = 0;
            pInst->nErr = err;
            pInst->nErl = nLine;
            pErrCode    = pCode;
            pErrStmnt   = pStmnt;
            // An error occured in an error handler
            // force parent handler ( if there is one )
            // to handle the error
            bool bLetParentHandleThis = false;

            // Im Error Handler? Dann Std-Error
            if ( !bInError )
            {
                bInError = sal_True;

                if( !bError )           // On Error Resume Next
                    StepRESUME( 1 );
                else if( pError )       // On Error Goto ...
                    pCode = pError;
                else
                    bLetParentHandleThis = true;
            }
            else
            {
                bLetParentHandleThis = true;
                pError = NULL; //terminate the handler
            }
            if ( bLetParentHandleThis )
            {
                // AB 13.2.1997, neues Error-Handling:
                // Uebergeordnete Error-Handler beruecksichtigen

                // Wir haben keinen Error-Handler -> weiter oben suchen
                SbiRuntime* pRtErrHdl = NULL;
                SbiRuntime* pRt = this;
                while( NULL != (pRt = pRt->pNext) )
                {
                    // Gibt es einen Error-Handler?
                    if( pRt->bError == sal_False || pRt->pError != NULL )
                    {
                        pRtErrHdl = pRt;
                        break;
                    }
                }

                // Error-Hdl gefunden?
                if( pRtErrHdl )
                {
                    // (Neuen) Error-Stack anlegen
                    SbErrorStack*& rErrStack = GetSbData()->pErrStack;
                    if( rErrStack )
                        delete rErrStack;
                    rErrStack = new SbErrorStack();

                    // Alle im Call-Stack darunter stehenden RTs manipulieren
                    pRt = this;
                    do
                    {
                        // Fehler setzen
                        pRt->nError = err;
                        if( pRt != pRtErrHdl )
                            pRt->bRun = sal_False;

                        // In Error-Stack eintragen
                        SbErrorStackEntry *pEntry = new SbErrorStackEntry
                            ( pRt->pMeth, pRt->nLine, pRt->nCol1, pRt->nCol2 );
                        rErrStack->C40_INSERT(SbErrorStackEntry, pEntry, rErrStack->Count() );

                        // Nach RT mit Error-Handler aufhoeren
                        if( pRt == pRtErrHdl )
                            break;
                           pRt = pRt->pNext;
                    }
                    while( pRt );
                }
                // Kein Error-Hdl gefunden -> altes Vorgehen
                else
                {
#ifdef DBG_TRACE_BASIC
                    bTraceErrHandled = false;
#endif
                    pInst->Abort();
                }

                // ALT: Nur
                // pInst->Abort();
            }

#ifdef DBG_TRACE_BASIC
            dbg_traceNotifyError( nTraceErr, aTraceErrMsg, bTraceErrHandled, pINST->nCallLvl );
#endif
        }
    }
    return bRun;
}

void SbiRuntime::Error( SbError n, bool bVBATranslationAlreadyDone )
{
    if( n )
    {
        nError = n;
        if( isVBAEnabled() && !bVBATranslationAlreadyDone )
        {
            String aMsg = pInst->GetErrorMsg();
            sal_Int32 nVBAErrorNumber = translateErrorToVba( nError, aMsg );
            SbxVariable* pSbxErrObjVar = SbxErrObject::getErrObject();
            SbxErrObject* pGlobErr = static_cast< SbxErrObject* >( pSbxErrObjVar );
            if( pGlobErr != NULL )
                pGlobErr->setNumberAndDescription( nVBAErrorNumber, aMsg );

            pInst->aErrorMsg = aMsg;
            nError = SbERR_BASIC_COMPAT;
        }
    }
}

void SbiRuntime::Error( SbError _errCode, const String& _details )
{
    if ( _errCode )
    {
        // Not correct for class module usage, remove for now
        //OSL_ENSURE( pInst->pRun == this, "SbiRuntime::Error: can't propagate the error message details!" );
        if ( pInst->pRun == this )
        {
            pInst->Error( _errCode, _details );
            //OSL_POSTCOND( nError == _errCode, "SbiRuntime::Error: the instance is expecte to propagate the error code back to me!" );
        }
        else
        {
            nError = _errCode;
        }
    }
}

void SbiRuntime::FatalError( SbError n )
{
    StepSTDERROR();
    Error( n );
}

void SbiRuntime::FatalError( SbError _errCode, const String& _details )
{
    StepSTDERROR();
    Error( _errCode, _details );
}

sal_Int32 SbiRuntime::translateErrorToVba( SbError nError, String& rMsg )
{
    // If a message is defined use that ( in preference to
    // the defined one for the error ) NB #TODO
    // if there is an error defined it more than likely
    // is not the one you want ( some are the same though )
    // we really need a new vba compatible error list
    if ( !rMsg.Len() )
    {
        // TEST, has to be vb here always
#ifdef DBG_UTIL
        SbError nTmp = StarBASIC::GetSfxFromVBError( (sal_uInt16)nError );
        DBG_ASSERT( nTmp, "No VB error!" );
#endif

        StarBASIC::MakeErrorText( nError, rMsg );
        rMsg = StarBASIC::GetErrorText();
        if ( !rMsg.Len() ) // no message for err no, need localized resource here
            rMsg = String( RTL_CONSTASCII_USTRINGPARAM("Internal Object Error:") );
    }
    // no num? most likely then it *is* really a vba err
    sal_uInt16 nVBErrorCode = StarBASIC::GetVBErrorCode( nError );
    sal_Int32 nVBAErrorNumber = ( nVBErrorCode == 0 ) ? nError : nVBErrorCode;
    return nVBAErrorNumber;
}

//////////////////////////////////////////////////////////////////////////
//
//  Parameter, Locals, Caller
//
//////////////////////////////////////////////////////////////////////////

SbMethod* SbiRuntime::GetCaller()
{
    return pMeth;
}

SbxArray* SbiRuntime::GetLocals()
{
    return refLocals;
}

SbxArray* SbiRuntime::GetParams()
{
    return refParams;
}

//////////////////////////////////////////////////////////////////////////
//
//  Stacks
//
//////////////////////////////////////////////////////////////////////////

// Der Expression-Stack steht fuer die laufende Auswertung von Expressions
// zur Verfuegung.

void SbiRuntime::PushVar( SbxVariable* pVar )
{
    if( pVar )
        refExprStk->Put( pVar, nExprLvl++ );
}

SbxVariableRef SbiRuntime::PopVar()
{
#ifdef DBG_UTIL
    if( !nExprLvl )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        return new SbxVariable;
    }
#endif
    SbxVariableRef xVar = refExprStk->Get( --nExprLvl );
#ifdef DBG_UTIL
    if ( xVar->GetName().EqualsAscii( "Cells" ) )
        DBG_TRACE( "" );
#endif
    // Methods halten im 0.Parameter sich selbst, also weghauen
    if( xVar->IsA( TYPE(SbxMethod) ) )
        xVar->SetParameters(0);
    return xVar;
}

sal_Bool SbiRuntime::ClearExprStack()
{
    // Achtung: Clear() reicht nicht, da Methods geloescht werden muessen
    while ( nExprLvl )
    {
        PopVar();
    }
    refExprStk->Clear();
    return sal_False;
}

// Variable auf dem Expression-Stack holen, ohne sie zu entfernen
// n zaehlt ab 0.

SbxVariable* SbiRuntime::GetTOS( short n )
{
    n = nExprLvl - n - 1;
#ifdef DBG_UTIL
    if( n < 0 )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        return new SbxVariable;
    }
#endif
    return refExprStk->Get( (sal_uInt16) n );
}

// Sicherstellen, dass TOS eine temporaere Variable ist

void SbiRuntime::TOSMakeTemp()
{
    SbxVariable* p = refExprStk->Get( nExprLvl - 1 );
    if( p->GetRefCount() != 1 )
    {
        SbxVariable* pNew = new SbxVariable( *p );
        pNew->SetFlag( SBX_READWRITE );
        refExprStk->Put( pNew, nExprLvl - 1 );
    }
}

// Der GOSUB-Stack nimmt Returnadressen fuer GOSUBs auf

void SbiRuntime::PushGosub( const sal_uInt8* pc )
{
    if( ++nGosubLvl > MAXRECURSION )
        StarBASIC::FatalError( SbERR_STACK_OVERFLOW );
    SbiGosubStack* p = new SbiGosubStack;
    p->pCode  = pc;
    p->pNext  = pGosubStk;
    p->nStartForLvl = nForLvl;
    pGosubStk = p;
}

void SbiRuntime::PopGosub()
{
    if( !pGosubStk )
        Error( SbERR_NO_GOSUB );
    else
    {
        SbiGosubStack* p = pGosubStk;
        pCode = p->pCode;
        pGosubStk = p->pNext;
        delete p;
        nGosubLvl--;
    }
}

// Entleeren des GOSUB-Stacks

void SbiRuntime::ClearGosubStack()
{
    SbiGosubStack* p;
    while(( p = pGosubStk ) != NULL )
        pGosubStk = p->pNext, delete p;
    nGosubLvl = 0;
}

// Der Argv-Stack nimmt aktuelle Argument-Vektoren auf

void SbiRuntime::PushArgv()
{
    SbiArgvStack* p = new SbiArgvStack;
    p->refArgv = refArgv;
    p->nArgc = nArgc;
    nArgc = 1;
    refArgv.Clear();
    p->pNext = pArgvStk;
    pArgvStk = p;
}

void SbiRuntime::PopArgv()
{
    if( pArgvStk )
    {
        SbiArgvStack* p = pArgvStk;
        pArgvStk = p->pNext;
        refArgv = p->refArgv;
        nArgc = p->nArgc;
        delete p;
    }
}

// Entleeren des Argv-Stacks

void SbiRuntime::ClearArgvStack()
{
    while( pArgvStk )
        PopArgv();
}

// Push des For-Stacks. Der Stack hat Inkrement, Ende, Beginn und Variable.
// Nach Aufbau des Stack-Elements ist der Stack leer.

void SbiRuntime::PushFor()
{
    SbiForStack* p = new SbiForStack;
    p->eForType = FOR_TO;
    p->pNext = pForStk;
    pForStk = p;
    // Der Stack ist wie folgt aufgebaut:
    p->refInc = PopVar();
    p->refEnd = PopVar();
    SbxVariableRef xBgn = PopVar();
    p->refVar = PopVar();
    *(p->refVar) = *xBgn;
    nForLvl++;
}

void SbiRuntime::PushForEach()
{
    SbiForStack* p = new SbiForStack;
    p->pNext = pForStk;
    pForStk = p;

    SbxVariableRef xObjVar = PopVar();
    SbxBase* pObj = xObjVar.Is() ? xObjVar->GetObject() : NULL;
    if( pObj == NULL )
    {
        Error( SbERR_NO_OBJECT );
        return;
    }

    bool bError_ = false;
    BasicCollection* pCollection;
    SbxDimArray* pArray;
    SbUnoObject* pUnoObj;
    if( (pArray = PTR_CAST(SbxDimArray,pObj)) != NULL )
    {
        p->eForType = FOR_EACH_ARRAY;
        p->refEnd = (SbxVariable*)pArray;

        short nDims = pArray->GetDims();
        p->pArrayLowerBounds = new sal_Int32[nDims];
        p->pArrayUpperBounds = new sal_Int32[nDims];
        p->pArrayCurIndices  = new sal_Int32[nDims];
        sal_Int32 lBound, uBound;
        for( short i = 0 ; i < nDims ; i++ )
        {
            pArray->GetDim32( i+1, lBound, uBound );
            p->pArrayCurIndices[i] = p->pArrayLowerBounds[i] = lBound;
            p->pArrayUpperBounds[i] = uBound;
        }
    }
    else if( (pCollection = PTR_CAST(BasicCollection,pObj)) != NULL )
    {
        p->eForType = FOR_EACH_COLLECTION;
        p->refEnd = pCollection;
        p->nCurCollectionIndex = 0;
    }
    else if( (pUnoObj = PTR_CAST(SbUnoObject,pObj)) != NULL )
    {
        // XEnumerationAccess?
        Any aAny = pUnoObj->getUnoAny();
        Reference< XEnumerationAccess > xEnumerationAccess;
        if( (aAny >>= xEnumerationAccess) )
        {
            p->xEnumeration = xEnumerationAccess->createEnumeration();
            p->eForType = FOR_EACH_XENUMERATION;
        }
        else if ( isVBAEnabled() && pUnoObj->isNativeCOMObject() )
        {
            uno::Reference< script::XInvocation > xInvocation;
            if ( ( aAny >>= xInvocation ) && xInvocation.is() )
            {
                try
                {
                    p->xEnumeration = new ComEnumerationWrapper( xInvocation );
                    p->eForType = FOR_EACH_XENUMERATION;
                }
                catch( uno::Exception& )
                {}
            }

            if ( !p->xEnumeration.is() )
                bError_ = true;
        }
        else
        {
            bError_ = true;
        }
    }
    else
    {
        bError_ = true;
    }

    if( bError_ )
    {
        Error( SbERR_CONVERSION );
        return;
    }

    // Container variable
    p->refVar = PopVar();
    nForLvl++;
}

// Poppen des FOR-Stacks

void SbiRuntime::PopFor()
{
    if( pForStk )
    {
        SbiForStack* p = pForStk;
        pForStk = p->pNext;
        delete p;
        nForLvl--;
    }
}

// Entleeren des FOR-Stacks

void SbiRuntime::ClearForStack()
{
    while( pForStk )
        PopFor();
}

//////////////////////////////////////////////////////////////////////////
//
//  DLL-Aufrufe
//
//////////////////////////////////////////////////////////////////////////

void SbiRuntime::DllCall
    ( const String& aFuncName,  // Funktionsname
      const String& aDLLName,   // Name der DLL
      SbxArray* pArgs,          // Parameter (ab Index 1, kann NULL sein)
      SbxDataType eResType,     // Returnwert
      sal_Bool bCDecl )             // sal_True: nach C-Konventionen
{
    // No DllCall for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    // MUSS NOCH IMPLEMENTIERT WERDEN
    /*
    String aMsg;
    aMsg = "FUNC=";
    aMsg += pFunc;
    aMsg += " DLL=";
    aMsg += pDLL;
    MessBox( NULL, WB_OK, String( "DLL-CALL" ), aMsg ).Execute();
    Error( SbERR_NOT_IMPLEMENTED );
    */

    SbxVariable* pRes = new SbxVariable( eResType );
    SbiDllMgr* pDllMgr = pInst->GetDllMgr();
    SbError nErr = pDllMgr->Call( aFuncName, aDLLName, pArgs, *pRes, bCDecl );
    if( nErr )
        Error( nErr );
    PushVar( pRes );
}

sal_uInt16 SbiRuntime::GetImageFlag( sal_uInt16 n ) const
{
    return pImg->GetFlag( n );
}

sal_uInt16 SbiRuntime::GetBase()
{
    return pImg->GetBase();
}
