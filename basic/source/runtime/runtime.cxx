/*************************************************************************
 *
 *  $RCSfile: runtime.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ab $ $Date: 2000-11-23 17:11:50 $
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

#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif

#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#include <svtools/sbx.hxx>
#include "runtime.hxx"
#pragma hdrstop
#include "sbintern.hxx"
#include "opcodes.hxx"
#include "iosys.hxx"
#include "image.hxx"
#include "ddectrl.hxx"
#include "dllmgr.hxx"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

// Makro MEMBER()
#include <macfix.hxx>

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )

struct SbiGosubStack {                  // GOSUB-Stack:
    SbiGosubStack* pNext;               // Chain
    const BYTE* pCode;                  // Return-Pointer
};

struct SbiArgvStack {                   // Argv stack:
    SbiArgvStack*  pNext;               // Stack Chain
    SbxArrayRef    refArgv;             // Argv
    short nArgc;                        // Argc
};

SbiRuntime::pStep0 SbiRuntime::aStep0[] = { // Alle Opcodes ohne Operanden
    MEMBER(SbiRuntime::StepNOP),
    MEMBER(SbiRuntime::StepEXP),
    MEMBER(SbiRuntime::StepMUL),
    MEMBER(SbiRuntime::StepDIV),
    MEMBER(SbiRuntime::StepMOD),
    MEMBER(SbiRuntime::StepPLUS),
    MEMBER(SbiRuntime::StepMINUS),
    MEMBER(SbiRuntime::StepNEG),
    MEMBER(SbiRuntime::StepEQ),
    MEMBER(SbiRuntime::StepNE),
    MEMBER(SbiRuntime::StepLT),
    MEMBER(SbiRuntime::StepGT),
    MEMBER(SbiRuntime::StepLE),
    MEMBER(SbiRuntime::StepGE),
    MEMBER(SbiRuntime::StepIDIV),
    MEMBER(SbiRuntime::StepAND),
    MEMBER(SbiRuntime::StepOR),
    MEMBER(SbiRuntime::StepXOR),
    MEMBER(SbiRuntime::StepEQV),
    MEMBER(SbiRuntime::StepIMP),
    MEMBER(SbiRuntime::StepNOT),
    MEMBER(SbiRuntime::StepCAT),

    MEMBER(SbiRuntime::StepLIKE),
    MEMBER(SbiRuntime::StepIS),
    // Laden/speichern
    MEMBER(SbiRuntime::StepARGC),       // neuen Argv einrichten
    MEMBER(SbiRuntime::StepARGV),       // TOS ==> aktueller Argv
    MEMBER(SbiRuntime::StepINPUT),      // Input ==> TOS
    MEMBER(SbiRuntime::StepLINPUT),     // Line Input ==> TOS
    MEMBER(SbiRuntime::StepGET),        // TOS anfassen
    MEMBER(SbiRuntime::StepSET),        // Speichern Objekt TOS ==> TOS-1
    MEMBER(SbiRuntime::StepPUT),        // TOS ==> TOS-1
    MEMBER(SbiRuntime::StepPUTC),       // TOS ==> TOS-1, dann ReadOnly
    MEMBER(SbiRuntime::StepDIM),        // DIM
    MEMBER(SbiRuntime::StepREDIM),      // REDIM
    MEMBER(SbiRuntime::StepREDIMP),     // REDIM PRESERVE
    MEMBER(SbiRuntime::StepERASE),      // TOS loeschen
    // Verzweigen
    MEMBER(SbiRuntime::StepSTOP),       // Programmende
    MEMBER(SbiRuntime::StepINITFOR),    // FOR-Variable initialisieren
    MEMBER(SbiRuntime::StepNEXT),       // FOR-Variable inkrementieren
    MEMBER(SbiRuntime::StepCASE),       // Anfang CASE
    MEMBER(SbiRuntime::StepENDCASE),    // Ende CASE
    MEMBER(SbiRuntime::StepSTDERROR),   // Standard-Fehlerbehandlung
    MEMBER(SbiRuntime::StepNOERROR),    // keine Fehlerbehandlung
    MEMBER(SbiRuntime::StepLEAVE),      // UP verlassen
    // E/A
    MEMBER(SbiRuntime::StepCHANNEL),    // TOS = Kanalnummer
    MEMBER(SbiRuntime::StepPRINT),      // print TOS
    MEMBER(SbiRuntime::StepPRINTF),     // print TOS in field
    MEMBER(SbiRuntime::StepWRITE),      // write TOS
    MEMBER(SbiRuntime::StepRENAME),     // Rename Tos+1 to Tos
    MEMBER(SbiRuntime::StepPROMPT),     // Input Prompt aus TOS definieren
    MEMBER(SbiRuntime::StepRESTART),    // Set restart point
    MEMBER(SbiRuntime::StepCHANNEL0),   // E/A-Kanal 0 einstellen
    MEMBER(SbiRuntime::StepEMPTY),      // Leeren Ausdruck auf Stack
    MEMBER(SbiRuntime::StepERROR),      // TOS = Fehlercode
    MEMBER(SbiRuntime::StepLSET),       // Speichern Objekt TOS ==> TOS-1
    MEMBER(SbiRuntime::StepRSET),       // Speichern Objekt TOS ==> TOS-1
    MEMBER(SbiRuntime::StepREDIMP_ERASE)// Copy array object for REDIMP
};

SbiRuntime::pStep1 SbiRuntime::aStep1[] = { // Alle Opcodes mit einem Operanden
    MEMBER(SbiRuntime::StepLOADNC),     // Laden einer numerischen Konstanten (+ID)
    MEMBER(SbiRuntime::StepLOADSC),     // Laden einer Stringkonstanten (+ID)
    MEMBER(SbiRuntime::StepLOADI),      // Immediate Load (+Wert)
    MEMBER(SbiRuntime::StepARGN),       // Speichern eines named Args in Argv (+StringID)
    MEMBER(SbiRuntime::StepPAD),        // String auf feste Laenge bringen (+Laenge)
    // Verzweigungen
    MEMBER(SbiRuntime::StepJUMP),       // Sprung (+Target)
    MEMBER(SbiRuntime::StepJUMPT),      // TOS auswerten), bedingter Sprung (+Target)
    MEMBER(SbiRuntime::StepJUMPF),      // TOS auswerten), bedingter Sprung (+Target)
    MEMBER(SbiRuntime::StepONJUMP),     // TOS auswerten), Sprung in JUMP-Tabelle (+MaxVal)
    MEMBER(SbiRuntime::StepGOSUB),      // UP-Aufruf (+Target)
    MEMBER(SbiRuntime::StepRETURN),     // UP-Return (+0 oder Target)
    MEMBER(SbiRuntime::StepTESTFOR),    // FOR-Variable testen), inkrementieren (+Endlabel)
    MEMBER(SbiRuntime::StepCASETO),     // Tos+1 <= Case <= Tos), 2xremove (+Target)
    MEMBER(SbiRuntime::StepERRHDL),     // Fehler-Handler (+Offset)
    MEMBER(SbiRuntime::StepRESUME),     // Resume nach Fehlern (+0 or 1 or Label)
    // E/A
    MEMBER(SbiRuntime::StepCLOSE),      // (+Kanal/0)
    MEMBER(SbiRuntime::StepPRCHAR),     // (+char)
    // Verwaltung
    MEMBER(SbiRuntime::StepCLASS),      // Klassennamen testen (+StringId)
    MEMBER(SbiRuntime::StepLIB),        // Lib fuer Declare-Call (+StringId)
    MEMBER(SbiRuntime::StepBASED),      // TOS wird um BASE erhoeht, BASE davor gepusht
    MEMBER(SbiRuntime::StepARGTYP),     // Letzten Parameter in Argv konvertieren (+Typ)
};

SbiRuntime::pStep2 SbiRuntime::aStep2[] = {// Alle Opcodes mit zwei Operanden
    MEMBER(SbiRuntime::StepRTL),        // Laden aus RTL (+StringID+Typ)
    MEMBER(SbiRuntime::StepFIND),       // Laden (+StringID+Typ)
    MEMBER(SbiRuntime::StepELEM),       // Laden Element (+StringID+Typ)
    MEMBER(SbiRuntime::StepPARAM),      // Parameter (+Offset+Typ)
    // Verzweigen
    MEMBER(SbiRuntime::StepCALL),       // Declare-Call (+StringID+Typ)
    MEMBER(SbiRuntime::StepCALLC),      // CDecl-Declare-Call (+StringID+Typ)
    MEMBER(SbiRuntime::StepCASEIS),     // Case-Test (+Test-Opcode+False-Target)
    // Verwaltung
    MEMBER(SbiRuntime::StepSTMNT),      // Beginn eines Statements (+Line+Col)
    // E/A
    MEMBER(SbiRuntime::StepOPEN),       // (+SvStreamFlags+Flags)
    // Objekte
    MEMBER(SbiRuntime::StepLOCAL),      // Lokale Variable definieren (+StringId+Typ)
    MEMBER(SbiRuntime::StepPUBLIC),     // Modulglobale Variable (+StringID+Typ)
    MEMBER(SbiRuntime::StepGLOBAL),     // Globale Variable definieren (+StringID+Typ)
    MEMBER(SbiRuntime::StepCREATE),     // Objekt kreieren (+StringId+StringId)
    MEMBER(SbiRuntime::StepSTATIC),     // Statische Variable (+StringId+StringId)
    MEMBER(SbiRuntime::StepTCREATE),    // User Defined Objekte (+StringId+StringId)
    MEMBER(SbiRuntime::StepDCREATE),    // Objekt-Array kreieren (+StringID+StringID)
};

//////////////////////////////////////////////////////////////////////////
//                              SbiRTLData                              //
//////////////////////////////////////////////////////////////////////////

SbiRTLData::SbiRTLData()
{
    pDir        = 0;
    nDirFlags   = 0;
    nCurDirPos  = 0;
}

SbiRTLData::~SbiRTLData()
{
    delete pDir;
    pDir = 0;
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
void SbiInstance::CalcBreakCallLevel( USHORT nFlags )
{
    // Break-Flag wegfiltern
    nFlags &= ~((USHORT)SbDEBUG_BREAK);

    USHORT nRet;
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
    bReschedule = TRUE;
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
    if( !pNumberFormatter )
        PrepareNumberFormatter( pNumberFormatter, nStdDateIdx, nStdTimeIdx, nStdDateTimeIdx );
    return pNumberFormatter;
}

// #39629 NumberFormatter auch statisch anbieten
void SbiInstance::PrepareNumberFormatter( SvNumberFormatter*& rpNumberFormatter,
    ULONG &rnStdDateIdx, ULONG &rnStdTimeIdx, ULONG &rnStdDateTimeIdx )
{
    const International& rInter = GetpApp()->GetAppInternational();
    LanguageType eLangType = rInter.GetLanguage();

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
        xFactory = comphelper::getProcessServiceFactory();
    if ( xFactory.is() )
        rpNumberFormatter = new SvNumberFormatter( xFactory, eLangType );
    else
        rpNumberFormatter = new SvNumberFormatter( eLangType );
    xub_StrLen nCheckPos = 0; short nType;
    rnStdTimeIdx = rpNumberFormatter->GetStandardFormat( NUMBERFORMAT_TIME, eLangType );

    // Standard-Vorlagen des Formatters haben nur zweistellige
    // Jahreszahl. Deshalb eigenes Format registrieren

    // HACK, da der Numberformatter in PutandConvertEntry die Platzhalter
    // fuer Monat, Tag, Jahr nicht entsprechend der Systemeinstellung
    // austauscht. Problem: Print Year(Date) unter engl. BS
    // siehe auch svtools\source\sbx\sbxdate.cxx

    DateFormat eDate = rInter.GetDateFormat();
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

void SbiInstance::Error( SbError n )
{
    Error( n, String() );
}

void SbiInstance::Error( SbError n, const String& rMsg )
{
    aErrorMsg = rMsg;
    pRun->Error( n );
}

void SbiInstance::FatalError( SbError n )
{
    pRun->FatalError( n );
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

SbMethod* SbiInstance::GetCaller( USHORT nLevel )
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

SbiRuntime::SbiRuntime( SbModule* pm, SbMethod* pe, USHORT nStart )
         : pMeth( pe ), pMod( pm ), pImg( pMod->pImage ),
           rBasic( *(StarBASIC*)pm->pParent ), pInst( pINST )
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
    pStmnt    = (const BYTE* ) pImg->GetCode() + nStart;
    bRun      =
    bError    = TRUE;
    bInError  = FALSE;
    nLine     =
    nCol1     =
    nCol2     =
    nExprLvl  =
    nArgc     =
    nError    =
    nGosubLvl =
    nOps      = 0;
    refExprStk = new SbxArray;
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

// Aufbau der Parameterliste. Alle ByRef-Parameter werden direkt
// uebernommen; von ByVal-Parametern werden Kopien angelegt. Falls
// ein bestimmter Datentyp verlangt wird, wird konvertiert.

void SbiRuntime::SetParameters( SbxArray* pParams )
{
    refParams = new SbxArray;
    // fuer den Returnwert
    refParams->Put( pMeth, 0 );
    if( pParams )
    {
        SbxInfo* pInfo = pMeth->GetInfo();
        for( USHORT i = 1; i < pParams->Count(); i++ )
        {
            const SbxParamInfo* p = pInfo ? pInfo->GetParam( i ) : NULL;
            SbxVariable* v = pParams->Get( i );
            // Methoden sind immer byval!
            BOOL bByVal = v->IsA( TYPE(SbxMethod) );
            SbxDataType t = v->GetType();
            if( p )
            {
                bByVal |= BOOL( ( p->eType & SbxBYREF ) == 0 );
                t = (SbxDataType) ( p->eType & 0x0FFF );
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
}

// Einen P-Code ausfuehren

BOOL SbiRuntime::Step()
{
    if( bRun )
    {
        // Unbedingt gelegentlich die Kontrolle abgeben!
        if( pInst->IsReschedule() && !( ++nOps & 0x1F ) )
            Application::Reschedule();

        SbiOpcode eOp = (SbiOpcode ) ( *pCode++ );
        USHORT nOp1, nOp2;
        if( eOp <= SbOP0_END )
        {
            (this->*( aStep0[ eOp ] ) )();
        }
        else if( eOp >= SbOP1_START && eOp <= SbOP1_END )
        {
            nOp1 = *pCode++; nOp1 |= *pCode++ << 8;
            (this->*( aStep1[ eOp - SbOP1_START ] ) )( nOp1 );
        }
        else if( eOp >= SbOP2_START && eOp <= SbOP2_END )
        {
            nOp1 = *pCode++; nOp1 |= *pCode++ << 8;
            nOp2 = *pCode++; nOp2 |= *pCode++ << 8;
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
            SbError err = nError;
            ClearExprStack();
            nError = 0;
            // Im Error Handler? Dann Std-Error
            if( bInError )
            {
                StepSTDERROR();
                pInst->Abort();
            }
            else
            {
                bInError = TRUE;

                pInst->nErr = err;
                pInst->nErl = nLine;
                pErrCode    = pCode;
                pErrStmnt   = pStmnt;
                if( !bError )           // On Error Resume Next
                    StepRESUME( 1 );
                else if( pError )       // On Error Goto ...
                    pCode = pError;
                else                    // Standard-Fehlerbehandlung
                {
                    // AB 13.2.1997, neues Error-Handling:
                    // Uebergeordnete Error-Handler beruecksichtigen

                    // Wir haben keinen Error-Handler -> weiter oben suchen
                    SbiRuntime* pRtErrHdl = NULL;
                    SbiRuntime* pRt = this;
                    while( NULL != (pRt = pRt->pNext) )
                    {
                        // Gibt es einen Error-Handler?
                        if( pRt->bError == FALSE || pRt->pError != NULL )
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
                                pRt->bRun = FALSE;

                            // In Error-Stack eintragen
                            SbErrorStackEntry *pEntry = new SbErrorStackEntry
                                ( pRt->pMeth, pRt->nLine, pRt->nCol1, pRt->nCol2 );
                            rErrStack->C40_INSERT(SbErrorStackEntry, pEntry, rErrStack->Count() );

                            // Nach RT mit Error-Handler aufhoeren
                            if( pRt == pRtErrHdl )
                                break;
                        }
                        while( pRt = pRt->pNext );
                    }
                    // Kein Error-Hdl gefunden -> altes Vorgehen
                    else
                    {
                        pInst->Abort();
                    }

                    // ALT: Nur
                    // pInst->Abort();
                }
            }
        }
    }
    return bRun;
}

void SbiRuntime::Error( SbError n )
{
    if( n )
        nError = n;
}

void SbiRuntime::FatalError( SbError n )
{
    StepSTDERROR();
    Error( n );
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
#ifndef PRODUCT
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

BOOL SbiRuntime::ClearExprStack()
{
    // #74732 Hier kann ein Fehler gesetzt werden
    BOOL bErrorSet = FALSE;

    // Achtung: Clear() reicht nicht, da Methods geloescht werden muessen
    while ( nExprLvl )
    {
        SbxVariableRef xVar = PopVar();
        if( !nError && xVar->ISA( UnoClassMemberVariable ) )
        {
            Error( SbERR_NO_METHOD );
            bErrorSet = TRUE;
        }
    }
    refExprStk->Clear();
    return bErrorSet;
}

// Variable auf dem Expression-Stack holen, ohne sie zu entfernen
// n zaehlt ab 0.

SbxVariable* SbiRuntime::GetTOS( short n )
{
    n = nExprLvl - n - 1;
#ifndef PRODUCT
    if( n < 0 )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        return new SbxVariable;
    }
#endif
    return refExprStk->Get( (USHORT) n );
}

// Sicherstellen, dass TOS eine temporaere Variable ist

void SbiRuntime::TOSMakeTemp()
{
    SbxVariable* p = refExprStk->Get( nExprLvl - 1 );
    if( p->GetRefCount() != 1 )
    {
        // #74573 UnoClassSbxVariable spezialbehandeln
        SbxVariable* pNew;
        if( p->ISA( UnoClassSbxVariable ) )
            pNew = new UnoClassSbxVariable( *(UnoClassSbxVariable*)p );
        else
            pNew = new SbxVariable( *p );
        pNew->SetFlag( SBX_READWRITE );
        refExprStk->Put( pNew, nExprLvl - 1 );
    }
}

// Der GOSUB-Stack nimmt Returnadressen fuer GOSUBs auf

void SbiRuntime::PushGosub( const BYTE* pc )
{
    if( ++nGosubLvl > MAXRECURSION )
        StarBASIC::FatalError( SbERR_STACK_OVERFLOW );
    SbiGosubStack* p = new SbiGosubStack;
    p->pCode  = pc;
    p->pNext  = pGosubStk;
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
    p->pNext = pForStk;
    pForStk = p;
    // Der Stack ist wie folgt aufgebaut:
    p->refInc = PopVar();
    p->refEnd = PopVar();
    SbxVariableRef xBgn = PopVar();
    p->refVar = PopVar();
    *(p->refVar) = *xBgn;
}

// Poppen des FOR-Stacks

void SbiRuntime::PopFor()
{
    if( pForStk )
    {
        SbiForStack* p = pForStk;
        pForStk = p->pNext;
        delete p;
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
      BOOL bCDecl )             // TRUE: nach C-Konventionen
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
    ByteString aByteFuncName( aFuncName, gsl_getSystemTextEncoding() );
    ByteString aByteDLLName( aDLLName, gsl_getSystemTextEncoding() );
    SbError nErr = pDllMgr->Call( aByteFuncName.GetBuffer(), aByteDLLName.GetBuffer(), pArgs, *pRes, bCDecl );
    if( nErr )
        Error( nErr );
    PushVar( pRes );
}

