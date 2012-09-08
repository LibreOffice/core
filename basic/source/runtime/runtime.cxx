/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

#include "comenumwrapper.hxx"

SbxVariable* getDefaultProp( SbxVariable* pRef );

using namespace ::com::sun::star;

bool SbiRuntime::isVBAEnabled()
{
    bool result = false;
    SbiInstance* pInst = GetSbData()->pInst;
    if ( pInst && GetSbData()->pInst->pRun )
        result = pInst->pRun->bVBAEnabled;
    return result;
}

void StarBASIC::SetVBAEnabled( bool bEnabled )
{
    if ( bDocBasic )
    {
        bVBAEnabled = bEnabled;
    }
}

bool StarBASIC::isVBAEnabled()
{
    if ( bDocBasic )
    {
        if( SbiRuntime::isVBAEnabled() )
            return true;
        return bVBAEnabled;
    }
    return false;
}


struct SbiArgvStack {                   // Argv stack:
    SbiArgvStack*  pNext;               // Stack Chain
    SbxArrayRef    refArgv;             // Argv
    short nArgc;                        // Argc
};

SbiRuntime::pStep0 SbiRuntime::aStep0[] = { // all opcodes without operands
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
    // load/save
    &SbiRuntime::StepARGC,      // establish new Argv
    &SbiRuntime::StepARGV,      // TOS ==> current Argv
    &SbiRuntime::StepINPUT,     // Input ==> TOS
    &SbiRuntime::StepLINPUT,        // Line Input ==> TOS
    &SbiRuntime::StepGET,        // touch TOS
    &SbiRuntime::StepSET,        // save object TOS ==> TOS-1
    &SbiRuntime::StepPUT,       // TOS ==> TOS-1
    &SbiRuntime::StepPUTC,      // TOS ==> TOS-1, then ReadOnly
    &SbiRuntime::StepDIM,       // DIM
    &SbiRuntime::StepREDIM,         // REDIM
    &SbiRuntime::StepREDIMP,        // REDIM PRESERVE
    &SbiRuntime::StepERASE,         // delete TOS
    // branch
    &SbiRuntime::StepSTOP,          // program end
    &SbiRuntime::StepINITFOR,   // intitialize FOR-Variable
    &SbiRuntime::StepNEXT,      // increment FOR-Variable
    &SbiRuntime::StepCASE,      // beginning CASE
    &SbiRuntime::StepENDCASE,   // end CASE
    &SbiRuntime::StepSTDERROR,      // standard error handling
    &SbiRuntime::StepNOERROR,   // no error handling
    &SbiRuntime::StepLEAVE,     // leave UP
    // E/A
    &SbiRuntime::StepCHANNEL,   // TOS = channel number
    &SbiRuntime::StepPRINT,     // print TOS
    &SbiRuntime::StepPRINTF,        // print TOS in field
    &SbiRuntime::StepWRITE,     // write TOS
    &SbiRuntime::StepRENAME,        // Rename Tos+1 to Tos
    &SbiRuntime::StepPROMPT,        // define Input Prompt from TOS
    &SbiRuntime::StepRESTART,   // Set restart point
    &SbiRuntime::StepCHANNEL0,  // set E/A-channel 0
    &SbiRuntime::StepEMPTY,     // empty expression on stack
    &SbiRuntime::StepERROR,     // TOS = error code
    &SbiRuntime::StepLSET,      // save object TOS ==> TOS-1
    &SbiRuntime::StepRSET,      // save object TOS ==> TOS-1
    &SbiRuntime::StepREDIMP_ERASE,// Copy array object for REDIMP
    &SbiRuntime::StepINITFOREACH,// Init for each loop
    &SbiRuntime::StepVBASET,// vba-like set statement
    &SbiRuntime::StepERASE_CLEAR,// vba-like set statement
    &SbiRuntime::StepARRAYACCESS,// access TOS as array
    &SbiRuntime::StepBYVAL,     // access TOS as array
};

SbiRuntime::pStep1 SbiRuntime::aStep1[] = { // all opcodes with one operand
    &SbiRuntime::StepLOADNC,        // loading a numeric constant (+ID)
    &SbiRuntime::StepLOADSC,        // loading a string constant (+ID)
    &SbiRuntime::StepLOADI,     // Immediate Load (+Wert)
    &SbiRuntime::StepARGN,      // save a named Args in Argv (+StringID)
    &SbiRuntime::StepPAD,       // bring string to a definite length (+length)
    // branches
    &SbiRuntime::StepJUMP,      // jump (+Target)
    &SbiRuntime::StepJUMPT,     // evaluate TOS, conditional jump (+Target)
    &SbiRuntime::StepJUMPF,     // evaluate TOS, conditional jump (+Target)
    &SbiRuntime::StepONJUMP,        // evaluate TOS, jump into JUMP-table (+MaxVal)
    &SbiRuntime::StepGOSUB,     // UP-call (+Target)
    &SbiRuntime::StepRETURN,        // UP-return (+0 or Target)
    &SbiRuntime::StepTESTFOR,   // check FOR-variable, increment (+Endlabel)
    &SbiRuntime::StepCASETO,        // Tos+1 <= Case <= Tos), 2xremove (+Target)
    &SbiRuntime::StepERRHDL,        // error handler (+Offset)
    &SbiRuntime::StepRESUME,        // resume after errors (+0 or 1 or Label)
    // E/A
    &SbiRuntime::StepCLOSE,     // (+channel/0)
    &SbiRuntime::StepPRCHAR,        // (+char)
    // management
    &SbiRuntime::StepSETCLASS,  // check set + class names (+StringId)
    &SbiRuntime::StepTESTCLASS, // Check TOS class (+StringId)
    &SbiRuntime::StepLIB,       // lib for declare-call (+StringId)
    &SbiRuntime::StepBASED,     // TOS is incremented by BASE, BASE is pushed before
    &SbiRuntime::StepARGTYP,        // convert last parameter in Argv (+Type)
    &SbiRuntime::StepVBASETCLASS,// vba-like set statement
};

SbiRuntime::pStep2 SbiRuntime::aStep2[] = {// all opcodes with two operands
    &SbiRuntime::StepRTL,       // load from RTL (+StringID+Typ)
    &SbiRuntime::StepFIND,      // load (+StringID+Typ)
    &SbiRuntime::StepELEM,          // load element (+StringID+Typ)
    &SbiRuntime::StepPARAM,     // Parameter (+Offset+Typ)
    // Verzweigen
    &SbiRuntime::StepCALL,      // Declare-Call (+StringID+Typ)
    &SbiRuntime::StepCALLC,     // CDecl-Declare-Call (+StringID+Typ)
    &SbiRuntime::StepCASEIS,        // Case-Test (+Test-Opcode+False-Target)
    // Verwaltung
    &SbiRuntime::StepSTMNT,         // beginning of a statement (+Line+Col)
    // E/A
    &SbiRuntime::StepOPEN,          // (+SvStreamFlags+Flags)
    // Objects
    &SbiRuntime::StepLOCAL,     // define local variable (+StringId+Typ)
    &SbiRuntime::StepPUBLIC,        // module global variable (+StringID+Typ)
    &SbiRuntime::StepGLOBAL,        // define global variable (+StringID+Typ)
    &SbiRuntime::StepCREATE,        // create object (+StringId+StringId)
    &SbiRuntime::StepSTATIC,     // static variable (+StringId+StringId)
    &SbiRuntime::StepTCREATE,    // user-defined objects (+StringId+StringId)
    &SbiRuntime::StepDCREATE,    // create object-array (+StringID+StringID)
    &SbiRuntime::StepGLOBAL_P,   // define global variable which is not overwritten
                                 // by the Basic on a restart (+StringID+Typ)
    &SbiRuntime::StepFIND_G,        // finds global variable with special treatment because of _GLOBAL_P
    &SbiRuntime::StepDCREATE_REDIMP, // redimension object array (+StringID+StringID)
    &SbiRuntime::StepFIND_CM,    // Search inside a class module (CM) to enable global search in time
    &SbiRuntime::StepPUBLIC_P,    // Search inside a class module (CM) to enable global search in time
    &SbiRuntime::StepFIND_STATIC,    // Search inside a class module (CM) to enable global search in time
};


//                              SbiRTLData                              //

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

//                              SbiInstance                             //

// 16.10.96: #31460 new concept for StepInto/Over/Out
// The decision whether StepPoint shall be called is done with the help of
// the CallLevel. It's stopped when the current CallLevel is <= nBreakCallLvl.
// The current CallLevel can never be smaller than 1, as it's also incremented
// during the call of a method (also main). Therefore a BreakCallLvl from 0
// means that the program isn't stopped at all.
// (also have a look at: step2.cxx, SbiRuntime::StepSTMNT() )


void SbiInstance::CalcBreakCallLevel( sal_uInt16 nFlags )
{

    nFlags &= ~((sal_uInt16)SbDEBUG_BREAK);

    sal_uInt16 nRet;
    switch( nFlags )
    {
        case SbDEBUG_STEPINTO:
            nRet = nCallLvl + 1;    // CallLevel+1 is also stopped
            break;
        case SbDEBUG_STEPOVER | SbDEBUG_STEPINTO:
            nRet = nCallLvl;        // current CallLevel is stopped
            break;
        case SbDEBUG_STEPOUT:
            nRet = nCallLvl - 1;    // smaller CallLevel is stopped
            break;
        case SbDEBUG_CONTINUE:
        // Basic-IDE returns 0 instead of SbDEBUG_CONTINUE, so also default=continue
        default:
            nRet = 0;               // CallLevel is always > 0 -> no StepPoint
    }
    nBreakCallLvl = nRet;           // take result
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
        OSL_FAIL( "SbiInstance::~SbiInstance: caught an exception while disposing the components!" );
    }

    ComponentVector.clear();
}

SbiDllMgr* SbiInstance::GetDllMgr()
{
    if( !pDllMgr )
        pDllMgr = new SbiDllMgr;
    return pDllMgr;
}

// #39629 create NumberFormatter with the help of a static method now
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

// #39629 offer NumberFormatter static too
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

    // the formatter's standard templates have only got a two-digit date
    // -> registering an own format

    // HACK, beause the numberformatter doesn't swap the place holders
    // for month, day and year according to the system setting.
    // Problem: Print Year(Date) under engl. BS
    // also have a look at: svtools\source\sbx\sbxdate.cxx

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



// Let engine run. If Flags == SbDEBUG_CONTINUE, take Flags over

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
    StarBASIC* pErrBasic = GetCurrentBasic( pBasic );
    pErrBasic->RTError( nErr, aErrorMsg, pRun->nLine, pRun->nCol1, pRun->nCol2 );
    pBasic->Stop();
}

// can be unequal to pRTBasic
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

//                              SbiInstance                             //

// Attention: pMeth can also be NULL (on a call of the init-code)

SbiRuntime::SbiRuntime( SbModule* pm, SbMethod* pe, sal_uInt32 nStart )
         : rBasic( *(StarBASIC*)pm->pParent ), pInst( GetSbData()->pInst ),
           pMod( pm ), pMeth( pe ), pImg( pMod->pImage ), mpExtCaller(0), m_nLastTime(0)
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
    bError    = true;
    bInError  = false;
    bBlocked  = false;
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

    // #74254 free items for saving temporary references
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
    if ( bVBAEnabled )
    {
        if ( pMeth )
            mpExtCaller = pMeth->mCaller;
    }
    else
        mpExtCaller = 0;
}

// Construction of the parameter list. All ByRef-parameters are directly
// taken over; copies of ByVal-parameters are created. If a particular
// data type is requested, it is converted.

void SbiRuntime::SetParameters( SbxArray* pParams )
{
    refParams = new SbxArray;
    // for the return value
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
            // methods are always byval!
            sal_Bool bByVal = v->IsA( TYPE(SbxMethod) );
            SbxDataType t = v->GetType();
            bool bTargetTypeIsArray = false;
            if( p )
            {
                bByVal |= sal_Bool( ( p->eType & SbxBYREF ) == 0 );
                t = (SbxDataType) ( p->eType & 0x0FFF );

                if( !bByVal && t != SbxVARIANT &&
                    (!v->IsFixed() || (SbxDataType)(v->GetType() & 0x0FFF ) != t) )
                        bByVal = sal_True;

                bTargetTypeIsArray = (p->nUserData & PARAM_INFO_WITHBRACKETS) != 0;
            }
            if( bByVal )
            {
                if( bTargetTypeIsArray )
                    t = SbxOBJECT;
                SbxVariable* v2 = new SbxVariable( t );
                v2->SetFlag( SBX_READWRITE );
                *v2 = *v;
                refParams->Put( v2, i );
            }
            else
            {
                if( t != SbxVARIANT && t != ( v->GetType() & 0x0FFF ) )
                {
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


// execute a P-Code

bool SbiRuntime::Step()
{
    if( bRun )
    {
        // in any case check casually!
        if( !( ++nOps & 0xF ) && pInst->IsReschedule() )
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
            if( pInst->IsReschedule() )
                Application::Reschedule();
        }

        SbiOpcode eOp = (SbiOpcode ) ( *pCode++ );
        sal_uInt32 nOp1, nOp2;
        if (eOp < SbOP0_END)
        {
            (this->*( aStep0[ eOp ] ) )();
        }
        else if (eOp >= SbOP1_START && eOp < SbOP1_END)
        {
            nOp1 = *pCode++; nOp1 |= *pCode++ << 8; nOp1 |= *pCode++ << 16; nOp1 |= *pCode++ << 24;

            (this->*( aStep1[ eOp - SbOP1_START ] ) )( nOp1 );
        }
        else if (eOp >= SbOP2_START && eOp < SbOP2_END)
        {
            nOp1 = *pCode++; nOp1 |= *pCode++ << 8; nOp1 |= *pCode++ << 16; nOp1 |= *pCode++ << 24;
            nOp2 = *pCode++; nOp2 |= *pCode++ << 8; nOp2 |= *pCode++ << 16; nOp2 |= *pCode++ << 24;
            (this->*( aStep2[ eOp - SbOP2_START ] ) )( nOp1, nOp2 );
        }
        else
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );


        SbError nSbError = SbxBase::GetError();
        Error( ERRCODE_TOERROR(nSbError) );

        // from 13.2.1997, new error handling:
        // ATTENTION: nError can be set already even if !nSbError
        // since nError can now also be set from other RT-instances

        if( nError )
            SbxBase::ResetError();

        // from 15.3.96: display errors only if BASIC is still active
        // (especially not after compiler errors at the runtime)
        if( nError && bRun )
        {
            SbError err = nError;
            ClearExprStack();
            nError = 0;
            pInst->nErr = err;
            pInst->nErl = nLine;
            pErrCode    = pCode;
            pErrStmnt   = pStmnt;
            // An error occurred in an error handler
            // force parent handler ( if there is one )
            // to handle the error
            bool bLetParentHandleThis = false;

            // in the error handler? so std-error
            if ( !bInError )
            {
                bInError = true;

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
                // from 13.2.1997, new error handling:
                // consider superior error handlers

                // there's no error handler -> find one farther above
                SbiRuntime* pRtErrHdl = NULL;
                SbiRuntime* pRt = this;
                while( NULL != (pRt = pRt->pNext) )
                {
                    if( !pRt->bError || pRt->pError != NULL )
                    {
                        pRtErrHdl = pRt;
                        break;
                    }
                }


                if( pRtErrHdl )
                {
                    // manipulate all the RTs that are below in the call-stack
                    pRt = this;
                    do
                    {
                        pRt->nError = err;
                        if( pRt != pRtErrHdl )
                            pRt->bRun = false;
                        else
                            break;
                        pRt = pRt->pNext;
                    }
                    while( pRt );
                }
                // no error-hdl found -> old behaviour
                else
                {
                    pInst->Abort();
                }

            }
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

//  Parameter, Locals, Caller

SbMethod* SbiRuntime::GetCaller()
{
    return pMeth;
}

//  Stacks

// The expression-stack is available for the continous evaluation
// of expressions.

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
        OSL_TRACE( "" );
#endif
    // methods hold themselves in parameter 0
    if( xVar->IsA( TYPE(SbxMethod) ) )
        xVar->SetParameters(0);
    return xVar;
}

bool SbiRuntime::ClearExprStack()
{
    // Attention: Clear() doesn't suffice as methods must be deleted
    while ( nExprLvl )
    {
        PopVar();
    }
    refExprStk->Clear();
    return false;
}

// Take variable from the expression-stack without removing it
// n counts from 0

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


void SbiRuntime::TOSMakeTemp()
{
    SbxVariable* p = refExprStk->Get( nExprLvl - 1 );
    if ( p->GetType() == SbxEMPTY )
        p->Broadcast( SBX_HINT_DATAWANTED );

    SbxVariable* pDflt = NULL;
    if ( bVBAEnabled &&  ( p->GetType() == SbxOBJECT || p->GetType() == SbxVARIANT  ) && ((pDflt = getDefaultProp(p)) != NULL) )
    {
        pDflt->Broadcast( SBX_HINT_DATAWANTED );
        // replacing new p on stack causes object pointed by
        // pDft->pParent to be deleted, when p2->Compute() is
        // called below pParent is accessed ( but its deleted )
        // so set it to NULL now
        pDflt->SetParent( NULL );
        p = new SbxVariable( *pDflt );
        p->SetFlag( SBX_READWRITE );
        refExprStk->Put( p, nExprLvl - 1 );
    }

    else if( p->GetRefCount() != 1 )
    {
        SbxVariable* pNew = new SbxVariable( *p );
        pNew->SetFlag( SBX_READWRITE );
        refExprStk->Put( pNew, nExprLvl - 1 );
    }
}

// the GOSUB-stack collects return-addresses for GOSUBs
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


void SbiRuntime::ClearGosubStack()
{
    SbiGosubStack* p;
    while(( p = pGosubStk ) != NULL )
        pGosubStk = p->pNext, delete p;
    nGosubLvl = 0;
}

// the Argv-stack collects current argument-vectors

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


void SbiRuntime::ClearArgvStack()
{
    while( pArgvStk )
        PopArgv();
}

// Push of the for-stack. The stack has increment, end, begin and variable.
// After the creation of the stack-element the stack's empty.

void SbiRuntime::PushFor()
{
    SbiForStack* p = new SbiForStack;
    p->eForType = FOR_TO;
    p->pNext = pForStk;
    pForStk = p;

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
                catch(const uno::Exception& )
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


void SbiRuntime::ClearForStack()
{
    while( pForStk )
        PopFor();
}

SbiForStack* SbiRuntime::FindForStackItemForCollection( class BasicCollection* pCollection )
{
    for (SbiForStack *p = pForStk; p; p = p->pNext)
    {
        SbxVariable* pVar = p->refEnd.Is() ? (SbxVariable*)p->refEnd : NULL;
        if( p->eForType == FOR_EACH_COLLECTION && pVar != NULL &&
            PTR_CAST(BasicCollection,pVar) == pCollection )
        {
            return p;
        }
    }

    return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//  DLL-calls

void SbiRuntime::DllCall
    ( const String& aFuncName,
      const String& aDLLName,
      SbxArray* pArgs,          // parameter (from index 1, can be NULL)
      SbxDataType eResType,     // return value
      bool bCDecl )         // true: according to C-conventions
{
    // No DllCall for "virtual" portal users
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    // NOT YET IMPLEMENTED

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
