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

#include <stdlib.h>

#include <algorithm>

#include <boost/unordered_map.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/SearchOptions.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <sal/log.hxx>

#include <tools/wldcrd.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <rtl/instance.hxx>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

#include <svl/zforlist.hxx>

#include <unotools/syslocale.hxx>
#include <unotools/textsearch.hxx>

#include <basic/sbuno.hxx>

#include "basrid.hxx"
#include "codegen.hxx"
#include "comenumwrapper.hxx"
#include "ddectrl.hxx"
#include "dllmgr.hxx"
#include "errobject.hxx"
#include "image.hxx"
#include "iosys.hxx"
#include "opcodes.hxx"
#include "runtime.hxx"
#include "sb.hrc"
#include "sbintern.hxx"
#include "sbunoobj.hxx"
#include "basic/codecompletecache.hxx"

using com::sun::star::uno::Reference;

using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;

using namespace ::com::sun::star;

static void lcl_clearImpl( SbxVariableRef& refVar, SbxDataType& eType );
static void lcl_eraseImpl( SbxVariableRef& refVar, bool bVBAEnabled );

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
        SAL_WARN("basic", "SbiInstance::~SbiInstance: caught an exception while disposing the components!" );
    }

    ComponentVector.clear();
}

SbiDllMgr* SbiInstance::GetDllMgr()
{
    if( !pDllMgr )
    {
        pDllMgr = new SbiDllMgr;
    }
    return pDllMgr;
}

// #39629 create NumberFormatter with the help of a static method now
SvNumberFormatter* SbiInstance::GetNumberFormatter()
{
    LanguageType eLangType = GetpApp()->GetSettings().GetLanguageTag().getLanguageType();
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
    {
        PrepareNumberFormatter( pNumberFormatter, nStdDateIdx, nStdTimeIdx, nStdDateTimeIdx,
        &meFormatterLangType, &meFormatterDateFormat );
    }
    return pNumberFormatter;
}

// #39629 offer NumberFormatter static too
void SbiInstance::PrepareNumberFormatter( SvNumberFormatter*& rpNumberFormatter,
    sal_uInt32 &rnStdDateIdx, sal_uInt32 &rnStdTimeIdx, sal_uInt32 &rnStdDateTimeIdx,
    LanguageType* peFormatterLangType, DateFormat* peFormatterDateFormat )
{
    LanguageType eLangType;
    if( peFormatterLangType )
    {
        eLangType = *peFormatterLangType;
    }
    else
    {
        eLangType = GetpApp()->GetSettings().GetLanguageTag().getLanguageType();
    }
    DateFormat eDate;
    if( peFormatterDateFormat )
    {
        eDate = *peFormatterDateFormat;
    }
    else
    {
        SvtSysLocale aSysLocale;
        eDate = aSysLocale.GetLocaleData().getDateFormat();
    }

    rpNumberFormatter = new SvNumberFormatter( comphelper::getProcessComponentContext(), eLangType );

    sal_Int32 nCheckPos = 0; short nType;
    rnStdTimeIdx = rpNumberFormatter->GetStandardFormat( NUMBERFORMAT_TIME, eLangType );

    // the formatter's standard templates have only got a two-digit date
    // -> registering an own format

    // HACK, beause the numberformatter doesn't swap the place holders
    // for month, day and year according to the system setting.
    // Problem: Print Year(Date) under engl. BS
    // also have a look at: basic/source/sbx/sbxdate.cxx

    OUString aDateStr;
    switch( eDate )
    {
        default:
        case MDY: aDateStr = "MM/DD/YYYY"; break;
        case DMY: aDateStr = "DD/MM/YYYY"; break;
        case YMD: aDateStr = "YYYY/MM/DD"; break;
    }
    OUString aStr( aDateStr );      // PutandConvertEntry() modifies string!
    rpNumberFormatter->PutandConvertEntry( aStr, nCheckPos, nType,
        rnStdDateIdx, LANGUAGE_ENGLISH_US, eLangType );
    nCheckPos = 0;
    OUString aStrHHMMSS(" HH:MM:SS");
    aDateStr += aStrHHMMSS;
    aStr = aDateStr;
    rpNumberFormatter->PutandConvertEntry( aStr, nCheckPos, nType,
        rnStdDateTimeIdx, LANGUAGE_ENGLISH_US, eLangType );
}


// Let engine run. If Flags == SbDEBUG_CONTINUE, take Flags over

void SbiInstance::Stop()
{
    for( SbiRuntime* p = pRun; p; p = p->pNext )
    {
        p->Stop();
    }
}

// Allows Basic IDE to set watch mode to suppress errors
static bool bWatchMode = false;

void setBasicWatchMode( bool bOn )
{
    bWatchMode = bOn;
}

void SbiInstance::Error( SbError n )
{
    Error( n, OUString() );
}

void SbiInstance::Error( SbError n, const OUString& rMsg )
{
    if( !bWatchMode )
    {
        aErrorMsg = rMsg;
        pRun->Error( n );
    }
}

void SbiInstance::ErrorVB( sal_Int32 nVBNumber, const OUString& rMsg )
{
    if( !bWatchMode )
    {
        SbError n = StarBASIC::GetSfxFromVBError( static_cast< sal_uInt16 >( nVBNumber ) );
        if ( !n )
        {
            n = nVBNumber; // force orig number, probably should have a specific table of vb ( localized ) errors
        }
        aErrorMsg = rMsg;
        SbiRuntime::translateErrorToVba( n, aErrorMsg );

        bool bVBATranslationAlreadyDone = true;
        pRun->Error( SbERR_BASIC_COMPAT, bVBATranslationAlreadyDone );
    }
}

void SbiInstance::setErrorVB( sal_Int32 nVBNumber, const OUString& rMsg )
{
    SbError n = StarBASIC::GetSfxFromVBError( static_cast< sal_uInt16 >( nVBNumber ) );
    if( !n )
    {
        n = nVBNumber; // force orig number, probably should have a specific table of vb ( localized ) errors
    }
    aErrorMsg = rMsg;
    SbiRuntime::translateErrorToVba( n, aErrorMsg );

    nErr = n;
}


void SbiInstance::FatalError( SbError n )
{
    pRun->FatalError( n );
}

void SbiInstance::FatalError( SbError _errCode, const OUString& _details )
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
        {
            pCurBasic = (StarBASIC*)pParent;
        }
    }
    return pCurBasic;
}

SbModule* SbiInstance::GetActiveModule()
{
    if( pRun )
    {
        return pRun->GetModule();
    }
    else
    {
        return NULL;
    }
}

SbMethod* SbiInstance::GetCaller( sal_uInt16 nLevel )
{
    SbiRuntime* p = pRun;
    while( nLevel-- && p )
    {
        p = p->pNext;
    }
    return p ? p->GetCaller() : NULL;
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
#if defined __GNUC__
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
        {
            mpExtCaller = pMeth->mCaller;
        }
    }
    else
    {
        mpExtCaller = 0;
    }
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
                {
                    bByVal = sal_True;
                }

                bTargetTypeIsArray = (p->nUserData & PARAM_INFO_WITHBRACKETS) != 0;
            }
            if( bByVal )
            {
                if( bTargetTypeIsArray )
                {
                    t = SbxOBJECT;
                }
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
                    {
                        Error( SbERR_CONVERSION );
                    }
                    else
                    {
                        v->Convert( t );
                    }
                }
                refParams->Put( v, i );
            }
            if( p )
            {
                refParams->PutAlias( p->aName, i );
            }
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
            {
                Application::Reschedule();
            }
        }

        SbiOpcode eOp = (SbiOpcode ) ( *pCode++ );
        sal_uInt32 nOp1, nOp2;
        if (eOp <= SbOP0_END)
        {
            (this->*( aStep0[ eOp ] ) )();
        }
        else if (eOp >= SbOP1_START && eOp <= SbOP1_END)
        {
            nOp1 = *pCode++; nOp1 |= *pCode++ << 8; nOp1 |= *pCode++ << 16; nOp1 |= *pCode++ << 24;

            (this->*( aStep1[ eOp - SbOP1_START ] ) )( nOp1 );
        }
        else if (eOp >= SbOP2_START && eOp <= SbOP2_END)
        {
            nOp1 = *pCode++; nOp1 |= *pCode++ << 8; nOp1 |= *pCode++ << 16; nOp1 |= *pCode++ << 24;
            nOp2 = *pCode++; nOp2 |= *pCode++ << 8; nOp2 |= *pCode++ << 16; nOp2 |= *pCode++ << 24;
            (this->*( aStep2[ eOp - SbOP2_START ] ) )( nOp1, nOp2 );
        }
        else
        {
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        }

        SbError nSbError = SbxBase::GetError();
        Error( ERRCODE_TOERROR(nSbError) );

        // from 13.2.1997, new error handling:
        // ATTENTION: nError can be set already even if !nSbError
        // since nError can now also be set from other RT-instances

        if( nError )
        {
            SbxBase::ResetError();
        }

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
                {
                    StepRESUME( 1 );
                }
                else if( pError )       // On Error Goto ...
                {
                    pCode = pError;
                }
                else
                {
                    bLetParentHandleThis = true;
                }
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
                        {
                            pRt->bRun = false;
                        }
                        else
                        {
                            break;
                        }
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
            OUString aMsg = pInst->GetErrorMsg();
            sal_Int32 nVBAErrorNumber = translateErrorToVba( nError, aMsg );
            SbxVariable* pSbxErrObjVar = SbxErrObject::getErrObject();
            SbxErrObject* pGlobErr = static_cast< SbxErrObject* >( pSbxErrObjVar );
            if( pGlobErr != NULL )
            {
                pGlobErr->setNumberAndDescription( nVBAErrorNumber, aMsg );
            }
            pInst->aErrorMsg = aMsg;
            nError = SbERR_BASIC_COMPAT;
        }
    }
}

void SbiRuntime::Error( SbError _errCode, const OUString& _details )
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

void SbiRuntime::FatalError( SbError _errCode, const OUString& _details )
{
    StepSTDERROR();
    Error( _errCode, _details );
}

sal_Int32 SbiRuntime::translateErrorToVba( SbError nError, OUString& rMsg )
{
    // If a message is defined use that ( in preference to
    // the defined one for the error ) NB #TODO
    // if there is an error defined it more than likely
    // is not the one you want ( some are the same though )
    // we really need a new vba compatible error list
    if ( rMsg.isEmpty() )
    {
        // TEST, has to be vb here always
#ifdef DBG_UTIL
        SbError nTmp = StarBASIC::GetSfxFromVBError( (sal_uInt16)nError );
        SAL_WARN_IF( nTmp == 0, "basic", "No VB error!" );
#endif

        StarBASIC::MakeErrorText( nError, rMsg );
        rMsg = StarBASIC::GetErrorText();
        if ( rMsg.isEmpty() ) // no message for err no, need localized resource here
        {
            rMsg = "Internal Object Error:";
        }
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
    {
        refExprStk->Put( pVar, nExprLvl++ );
    }
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
    if ( xVar->GetName().equalsAscii( "Cells" ) )
        SAL_INFO("basic", "PopVar: Name equals 'Cells'" );
#endif
    // methods hold themselves in parameter 0
    if( xVar->IsA( TYPE(SbxMethod) ) )
    {
        xVar->SetParameters(0);
    }
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
    {
        p->Broadcast( SBX_HINT_DATAWANTED );
    }

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
    {
        StarBASIC::FatalError( SbERR_STACK_OVERFLOW );
    }
    SbiGosubStack* p = new SbiGosubStack;
    p->pCode  = pc;
    p->pNext  = pGosubStk;
    p->nStartForLvl = nForLvl;
    pGosubStk = p;
}

void SbiRuntime::PopGosub()
{
    if( !pGosubStk )
    {
        Error( SbERR_NO_GOSUB );
    }
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
    {
        pGosubStk = p->pNext, delete p;
    }
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
    {
        PopArgv();
    }
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
            {
                bError_ = true;
            }
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
    {
        PopFor();
    }
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
    ( const OUString& aFuncName,
      const OUString& aDLLName,
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
    {
        Error( nErr );
    }
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

void SbiRuntime::StepNOP()
{}

void SbiRuntime::StepArith( SbxOperator eOp )
{
    SbxVariableRef p1 = PopVar();
    TOSMakeTemp();
    SbxVariable* p2 = GetTOS();

    p2->ResetFlag( SBX_FIXED );
    p2->Compute( eOp, *p1 );

    checkArithmeticOverflow( p2 );
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

    // Make sure objects with default params have
    // values ( and type ) set as appropriate
    SbxDataType p1Type = p1->GetType();
    SbxDataType p2Type = p2->GetType();
    if ( p1Type == SbxEMPTY )
    {
        p1->Broadcast( SBX_HINT_DATAWANTED );
        p1Type = p1->GetType();
    }
    if ( p2Type == SbxEMPTY )
    {
        p2->Broadcast( SBX_HINT_DATAWANTED );
        p2Type = p2->GetType();
    }
    if ( p1Type == p2Type )
    {
        // if both sides are an object and have default props
        // then we need to use the default props
        // we don't need to worry if only one side ( lhs, rhs ) is an
        // object ( object side will get coerced to correct type in
        // Compare )
        if ( p1Type ==  SbxOBJECT )
        {
            SbxVariable* pDflt = getDefaultProp( p1 );
            if ( pDflt )
            {
                p1 = pDflt;
                p1->Broadcast( SBX_HINT_DATAWANTED );
            }
            pDflt = getDefaultProp( p2 );
            if ( pDflt )
            {
                p2 = pDflt;
                p2->Broadcast( SBX_HINT_DATAWANTED );
            }
        }

    }
    static SbxVariable* pTRUE = NULL;
    static SbxVariable* pFALSE = NULL;
    static SbxVariable* pNULL = NULL;
    // why do this on non-windows ?
    // why do this at all ?
    // I dumbly follow the pattern :-/
    if ( bVBAEnabled && ( p1->IsNull() || p2->IsNull() ) )
    {
        if( !pNULL )
        {
            pNULL = new SbxVariable;
            pNULL->PutNull();
            pNULL->AddRef();
        }
        PushVar( pNULL );
    }
    else if( p2->Compare( eOp, *p1 ) )
    {
        if( !pTRUE )
        {
            pTRUE = new SbxVariable;
            pTRUE->PutBool( sal_True );
            pTRUE->AddRef();
        }
        PushVar( pTRUE );
    }
    else
    {
        if( !pFALSE )
        {
            pFALSE = new SbxVariable;
            pFALSE->PutBool( sal_False );
            pFALSE->AddRef();
        }
        PushVar( pFALSE );
    }
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

namespace
{
    bool NeedEsc(sal_Unicode cCode)
    {
        if((cCode & 0xFF80))
        {
            return false;
        }
        switch((sal_uInt8)(cCode & 0x07F))
        {
        case '.':
        case '^':
        case '$':
        case '+':
        case '\\':
        case '|':
        case '{':
        case '}':
        case '(':
        case ')':
            return true;
        default:
            return false;
        }
    }

    OUString VBALikeToRegexp(const OUString &rIn)
    {
        OUStringBuffer sResult;
        const sal_Unicode *start = rIn.getStr();
        const sal_Unicode *end = start + rIn.getLength();

        int seenright = 0;

        sResult.append('^');

        while (start < end)
        {
            switch (*start)
            {
            case '?':
                sResult.append('.');
                start++;
                break;
            case '*':
                sResult.append(".*");
                start++;
                break;
            case '#':
                sResult.append("[0-9]");
                start++;
                break;
            case ']':
                sResult.append('\\');
                sResult.append(*start++);
                break;
            case '[':
                sResult.append(*start++);
                seenright = 0;
                while (start < end && !seenright)
                {
                    switch (*start)
                    {
                    case '[':
                    case '?':
                    case '*':
                        sResult.append('\\');
                        sResult.append(*start);
                        break;
                    case ']':
                        sResult.append(*start);
                        seenright = 1;
                        break;
                    case '!':
                        sResult.append('^');
                        break;
                    default:
                        if (NeedEsc(*start))
                        {
                            sResult.append('\\');
                        }
                        sResult.append(*start);
                        break;
                    }
                    start++;
                }
                break;
            default:
                if (NeedEsc(*start))
                {
                    sResult.append('\\');
                }
                sResult.append(*start++);
            }
        }

        sResult.append('$');

        return sResult.makeStringAndClear();
    }
}

void SbiRuntime::StepLIKE()
{
    SbxVariableRef refVar1 = PopVar();
    SbxVariableRef refVar2 = PopVar();

    OUString pattern = VBALikeToRegexp(refVar1->GetOUString());
    OUString value = refVar2->GetOUString();

    com::sun::star::util::SearchOptions aSearchOpt;

    aSearchOpt.algorithmType = com::sun::star::util::SearchAlgorithms_REGEXP;

    aSearchOpt.Locale = Application::GetSettings().GetLanguageTag().getLocale();
    aSearchOpt.searchString = pattern;

    int bTextMode(1);
    bool bCompatibility = ( GetSbData()->pInst && GetSbData()->pInst->IsCompatibility() );
    if( bCompatibility )
    {
        bTextMode = GetImageFlag( SBIMG_COMPARETEXT );
    }
    if( bTextMode )
    {
        aSearchOpt.transliterateFlags |= com::sun::star::i18n::TransliterationModules_IGNORE_CASE;
    }
    SbxVariable* pRes = new SbxVariable;
    utl::TextSearch aSearch(aSearchOpt);
    sal_Int32 nStart=0, nEnd=value.getLength();
    int bRes = aSearch.SearchForward(value, &nStart, &nEnd);
    pRes->PutBool( bRes != 0 );

    PushVar( pRes );
}

// TOS and TOS-1 are both object variables and contain the same pointer

void SbiRuntime::StepIS()
{
    SbxVariableRef refVar1 = PopVar();
    SbxVariableRef refVar2 = PopVar();

    SbxDataType eType1 = refVar1->GetType();
    SbxDataType eType2 = refVar2->GetType();
    if ( eType1 == SbxEMPTY )
    {
        refVar1->Broadcast( SBX_HINT_DATAWANTED );
        eType1 = refVar1->GetType();
    }
    if ( eType2 == SbxEMPTY )
    {
        refVar2->Broadcast( SBX_HINT_DATAWANTED );
        eType2 = refVar2->GetType();
    }

    sal_Bool bRes = sal_Bool( eType1 == SbxOBJECT && eType2 == SbxOBJECT );
    if ( bVBAEnabled  && !bRes )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
    }
    bRes = ( bRes && refVar1->GetObject() == refVar2->GetObject() );
    SbxVariable* pRes = new SbxVariable;
    pRes->PutBool( bRes );
    PushVar( pRes );
}

// update the value of TOS

void SbiRuntime::StepGET()
{
    SbxVariable* p = GetTOS();
    p->Broadcast( SBX_HINT_DATAWANTED );
}

// #67607 copy Uno-Structs
inline bool checkUnoStructCopy( bool bVBA, SbxVariableRef& refVal, SbxVariableRef& refVar )
{
    SbxDataType eVarType = refVar->GetType();
    SbxDataType eValType = refVal->GetType();

    if ( !( !bVBA|| ( bVBA && refVar->GetType() != SbxEMPTY ) ) || !refVar->CanWrite() )
        return false;

    if ( eValType != SbxOBJECT )
        return false;
    // we seem to be duplicating parts of SbxValue=operator, maybe we should just move this to
    // there :-/ not sure if for every '=' we would want struct handling
    if( eVarType != SbxOBJECT )
    {
        if ( refVar->IsFixed() )
            return false;
    }
    // #115826: Exclude ProcedureProperties to avoid call to Property Get procedure
    else if( refVar->ISA(SbProcedureProperty) )
        return false;

    SbxObjectRef xValObj = (SbxObject*)refVal->GetObject();
    if( !xValObj.Is() || xValObj->ISA(SbUnoAnyObject) )
        return false;

    SbUnoObject* pUnoVal =  PTR_CAST(SbUnoObject,(SbxObject*)xValObj);
    SbUnoStructRefObject* pUnoStructVal = PTR_CAST(SbUnoStructRefObject,(SbxObject*)xValObj);
    Any aAny;
    // make doubly sure value is either an Uno object or
    // an uno struct
    if ( pUnoVal || pUnoStructVal )
        aAny = pUnoVal ? pUnoVal->getUnoAny() : pUnoStructVal->getUnoAny();
    else
        return false;
    if (  aAny.getValueType().getTypeClass() == TypeClass_STRUCT )
    {
        refVar->SetType( SbxOBJECT );
        SbxError eOldErr = refVar->GetError();
        // There are some circumstances when calling GetObject
        // will trigger an error, we need to squash those here.
        // Alternatively it is possible that the same scenario
        // could overwrite and existing error. Lets prevent that
        SbxObjectRef xVarObj = (SbxObject*)refVar->GetObject();
        if ( eOldErr != SbxERR_OK )
            refVar->SetError( eOldErr );
        else
            refVar->ResetError();

        SbUnoStructRefObject* pUnoStructObj = PTR_CAST(SbUnoStructRefObject,(SbxObject*)xVarObj);

        OUString sClassName = pUnoVal ? pUnoVal->GetClassName() : pUnoStructVal->GetClassName();
        OUString sName = pUnoVal ? pUnoVal->GetName() : pUnoStructVal->GetName();

        if ( pUnoStructObj )
        {
            StructRefInfo aInfo = pUnoStructObj->getStructInfo();
            aInfo.setValue( aAny );
        }
        else
        {
            SbUnoObject* pNewUnoObj = new SbUnoObject( sName, aAny );
            // #70324: adopt ClassName
            pNewUnoObj->SetClassName( sClassName );
            refVar->PutObject( pNewUnoObj );
        }
        return true;
    }
    return false;
}


// laying down TOS in TOS-1

void SbiRuntime::StepPUT()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    // store on its own method (inside a function)?
    bool bFlagsChanged = false;
    sal_uInt16 n = 0;
    if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
    {
        bFlagsChanged = true;
        n = refVar->GetFlags();
        refVar->SetFlag( SBX_WRITE );
    }

    // if left side arg is an object or variant and right handside isn't
    // either an object or a variant then try and see if a default
    // property exists.
    // to use e.g. Range{"A1") = 34
    // could equate to Range("A1").Value = 34
    if ( bVBAEnabled )
    {
        // yet more hacking at this, I feel we don't quite have the correct
        // heuristics for dealing with obj1 = obj2 ( where obj2 ( and maybe
        // obj1 ) has default member/property ) ) It seems that default props
        // aren't dealt with if the object is a member of some parent object
        bool bObjAssign = false;
        if ( refVar->GetType() == SbxEMPTY )
            refVar->Broadcast( SBX_HINT_DATAWANTED );
        if ( refVar->GetType() == SbxOBJECT )
        {
            if  ( refVar->IsA( TYPE(SbxMethod) ) || ! refVar->GetParent() )
            {
                SbxVariable* pDflt = getDefaultProp( refVar );

                if ( pDflt )
                    refVar = pDflt;
            }
            else
                bObjAssign = true;
        }
        if (  refVal->GetType() == SbxOBJECT  && !bObjAssign && ( refVal->IsA( TYPE(SbxMethod) ) || ! refVal->GetParent() ) )
        {
            SbxVariable* pDflt = getDefaultProp( refVal );
            if ( pDflt )
                refVal = pDflt;
        }
    }

    if ( !checkUnoStructCopy( bVBAEnabled, refVal, refVar ) )
        *refVar = *refVal;

    if( bFlagsChanged )
        refVar->SetFlags( n );
}


// VBA Dim As New behavior handling, save init object information
struct DimAsNewRecoverItem
{
    OUString        m_aObjClass;
    OUString        m_aObjName;
    SbxObject*      m_pObjParent;
    SbModule*       m_pClassModule;

    DimAsNewRecoverItem( void )
        : m_pObjParent( NULL )
        , m_pClassModule( NULL )
    {}

    DimAsNewRecoverItem( const OUString& rObjClass, const OUString& rObjName,
                         SbxObject* pObjParent, SbModule* pClassModule )
            : m_aObjClass( rObjClass )
            , m_aObjName( rObjName )
            , m_pObjParent( pObjParent )
            , m_pClassModule( pClassModule )
    {}

};


struct SbxVariablePtrHash
{
    size_t operator()( SbxVariable* pVar ) const
        { return (size_t)pVar; }
};

typedef boost::unordered_map< SbxVariable*, DimAsNewRecoverItem,
                              SbxVariablePtrHash >  DimAsNewRecoverHash;

class GaDimAsNewRecoverHash : public rtl::Static<DimAsNewRecoverHash, GaDimAsNewRecoverHash> {};

void removeDimAsNewRecoverItem( SbxVariable* pVar )
{
    DimAsNewRecoverHash &rDimAsNewRecoverHash = GaDimAsNewRecoverHash::get();
    DimAsNewRecoverHash::iterator it = rDimAsNewRecoverHash.find( pVar );
    if( it != rDimAsNewRecoverHash.end() )
    {
        rDimAsNewRecoverHash.erase( it );
    }
}


// saving object variable
// not-object variables will cause errors

static const char pCollectionStr[] = "Collection";

void SbiRuntime::StepSET_Impl( SbxVariableRef& refVal, SbxVariableRef& refVar, bool bHandleDefaultProp )
{
    // #67733 types with array-flag are OK too

    // Check var, !object is no error for sure if, only if type is fixed
    SbxDataType eVarType = refVar->GetType();
    if( !bHandleDefaultProp && eVarType != SbxOBJECT && !(eVarType & SbxARRAY) && refVar->IsFixed() )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
        return;
    }

    // Check value, !object is no error for sure if, only if type is fixed
    SbxDataType eValType = refVal->GetType();
    if( !bHandleDefaultProp && eValType != SbxOBJECT && !(eValType & SbxARRAY) && refVal->IsFixed() )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
        return;
    }

    // Getting in here causes problems with objects with default properties
    // if they are SbxEMPTY I guess
    if ( !bHandleDefaultProp || ( bHandleDefaultProp && eValType == SbxOBJECT ) )
    {
    // activate GetOject for collections on refVal
        SbxBase* pObjVarObj = refVal->GetObject();
        if( pObjVarObj )
        {
            SbxVariableRef refObjVal = PTR_CAST(SbxObject,pObjVarObj);

            if( refObjVal )
            {
                refVal = refObjVal;
            }
            else if( !(eValType & SbxARRAY) )
            {
                refVal = NULL;
            }
        }
    }

    // #52896 refVal can be invalid here, if uno-sequences - or more
    // general arrays - are assigned to variables that are declared
    // as an object!
    if( !refVal )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
    }
    else
    {
        bool bFlagsChanged = false;
        sal_uInt16 n = 0;
        if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
        {
            bFlagsChanged = true;
            n = refVar->GetFlags();
            refVar->SetFlag( SBX_WRITE );
        }
        SbProcedureProperty* pProcProperty = PTR_CAST(SbProcedureProperty,(SbxVariable*)refVar);
        if( pProcProperty )
        {
            pProcProperty->setSet( true );
        }
        if ( bHandleDefaultProp )
        {
            // get default properties for lhs & rhs where necessary
            // SbxVariable* defaultProp = NULL; unused variable
            // LHS try determine if a default prop exists
            // again like in StepPUT (see there too ) we are tweaking the
            // heursitics again for when to assign an object reference or
            // use default memebers if they exists
            // #FIXME we really need to get to the bottom of this mess
            bool bObjAssign = false;
            if ( refVar->GetType() == SbxOBJECT )
            {
                if ( refVar->IsA( TYPE(SbxMethod) ) || ! refVar->GetParent() )
                {
                    SbxVariable* pDflt = getDefaultProp( refVar );
                    if ( pDflt )
                    {
                        refVar = pDflt;
                    }
                }
                else
                    bObjAssign = true;
            }
            // RHS only get a default prop is the rhs has one
            if (  refVal->GetType() == SbxOBJECT )
            {
                // check if lhs is a null object
                // if it is then use the object not the default property
                SbxObject* pObj = NULL;


                pObj = PTR_CAST(SbxObject,(SbxVariable*)refVar);

                // calling GetObject on a SbxEMPTY variable raises
                // object not set errors, make sure its an Object
                if ( !pObj && refVar->GetType() == SbxOBJECT )
                {
                    SbxBase* pObjVarObj = refVar->GetObject();
                    pObj = PTR_CAST(SbxObject,pObjVarObj);
                }
                SbxVariable* pDflt = NULL;
                if ( pObj && !bObjAssign )
                {
                    // lhs is either a valid object || or has a defaultProp
                    pDflt = getDefaultProp( refVal );
                }
                if ( pDflt )
                {
                    refVal = pDflt;
                }
            }
        }

        // Handle Dim As New
        bool bDimAsNew = bVBAEnabled && refVar->IsSet( SBX_DIM_AS_NEW );
        SbxBaseRef xPrevVarObj;
        if( bDimAsNew )
        {
            xPrevVarObj = refVar->GetObject();
        }
        // Handle withevents
        sal_Bool bWithEvents = refVar->IsSet( SBX_WITH_EVENTS );
        if ( bWithEvents )
        {
            Reference< XInterface > xComListener;

            SbxBase* pObj = refVal->GetObject();
            SbUnoObject* pUnoObj = (pObj != NULL) ? PTR_CAST(SbUnoObject,pObj) : NULL;
            if( pUnoObj != NULL )
            {
                Any aControlAny = pUnoObj->getUnoAny();
                OUString aDeclareClassName = refVar->GetDeclareClassName();
                OUString aVBAType = aDeclareClassName;
                OUString aPrefix = refVar->GetName();
                SbxObjectRef xScopeObj = refVar->GetParent();
                xComListener = createComListener( aControlAny, aVBAType, aPrefix, xScopeObj );

                refVal->SetDeclareClassName( aDeclareClassName );
                refVal->SetComListener( xComListener, &rBasic );        // Hold reference
            }

        }

        // lhs is a property who's value is currently (Empty e.g. no broadcast yet)
        // in this case if there is a default prop involved the value of the
        // default property may infact be void so the type will also be SbxEMPTY
        // in this case we do not want to call checkUnoStructCopy 'cause that will
        // cause an error also
        if ( !checkUnoStructCopy( bHandleDefaultProp, refVal, refVar ) )
        {
            *refVar = *refVal;
        }
        if ( bDimAsNew )
        {
            if( !refVar->ISA(SbxObject) )
            {
                SbxBase* pValObjBase = refVal->GetObject();
                if( pValObjBase == NULL )
                {
                    if( xPrevVarObj.Is() )
                    {
                        // Object is overwritten with NULL, instantiate init object
                        DimAsNewRecoverHash &rDimAsNewRecoverHash = GaDimAsNewRecoverHash::get();
                        DimAsNewRecoverHash::iterator it = rDimAsNewRecoverHash.find( refVar );
                        if( it != rDimAsNewRecoverHash.end() )
                        {
                            const DimAsNewRecoverItem& rItem = it->second;
                            if( rItem.m_pClassModule != NULL )
                            {
                                SbClassModuleObject* pNewObj = new SbClassModuleObject( rItem.m_pClassModule );
                                pNewObj->SetName( rItem.m_aObjName );
                                pNewObj->SetParent( rItem.m_pObjParent );
                                refVar->PutObject( pNewObj );
                            }
                            else if( rItem.m_aObjClass.equalsIgnoreAsciiCaseAscii( pCollectionStr ) )
                            {
                                BasicCollection* pNewCollection = new BasicCollection( OUString(pCollectionStr) );
                                pNewCollection->SetName( rItem.m_aObjName );
                                pNewCollection->SetParent( rItem.m_pObjParent );
                                refVar->PutObject( pNewCollection );
                            }
                        }
                    }
                }
                else
                {
                    // Does old value exist?
                    bool bFirstInit = !xPrevVarObj.Is();
                    if( bFirstInit )
                    {
                        // Store information to instantiate object later
                        SbxObject* pValObj = PTR_CAST(SbxObject,pValObjBase);
                        if( pValObj != NULL )
                        {
                            OUString aObjClass = pValObj->GetClassName();

                            SbClassModuleObject* pClassModuleObj = PTR_CAST(SbClassModuleObject,pValObjBase);
                            DimAsNewRecoverHash &rDimAsNewRecoverHash = GaDimAsNewRecoverHash::get();
                            if( pClassModuleObj != NULL )
                            {
                                SbModule* pClassModule = pClassModuleObj->getClassModule();
                                rDimAsNewRecoverHash[refVar] =
                                    DimAsNewRecoverItem( aObjClass, pValObj->GetName(), pValObj->GetParent(), pClassModule );
                            }
                            else if( aObjClass.equalsIgnoreAsciiCase( "Collection" ) )
                            {
                                rDimAsNewRecoverHash[refVar] =
                                    DimAsNewRecoverItem( aObjClass, pValObj->GetName(), pValObj->GetParent(), NULL );
                            }
                        }
                    }
                }
            }
        }

        if( bFlagsChanged )
        {
            refVar->SetFlags( n );
        }
    }
}

void SbiRuntime::StepSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    StepSET_Impl( refVal, refVar, bVBAEnabled ); // this is really assigment
}

void SbiRuntime::StepVBASET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    // don't handle default property
    StepSET_Impl( refVal, refVar, false ); // set obj = something
}


void SbiRuntime::StepLSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    if( refVar->GetType() != SbxSTRING ||
        refVal->GetType() != SbxSTRING )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
    }
    else
    {
        sal_uInt16 n = refVar->GetFlags();
        if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
        {
            refVar->SetFlag( SBX_WRITE );
        }
        OUString aRefVarString = refVar->GetOUString();
        OUString aRefValString = refVal->GetOUString();

        sal_Int32 nVarStrLen = aRefVarString.getLength();
        sal_Int32 nValStrLen = aRefValString.getLength();
        OUStringBuffer aNewStr;
        if( nVarStrLen > nValStrLen )
        {
            aNewStr.append(aRefValString);
            comphelper::string::padToLength(aNewStr, nVarStrLen, ' ');
        }
        else
        {
            aNewStr = aRefValString.copy( 0, nVarStrLen );
        }

        refVar->PutString(aNewStr.makeStringAndClear());
        refVar->SetFlags( n );
    }
}

void SbiRuntime::StepRSET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    if( refVar->GetType() != SbxSTRING || refVal->GetType() != SbxSTRING )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
    }
    else
    {
        sal_uInt16 n = refVar->GetFlags();
        if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
        {
            refVar->SetFlag( SBX_WRITE );
        }
        OUString aRefVarString = refVar->GetOUString();
        OUString aRefValString = refVal->GetOUString();
        sal_Int32 nVarStrLen = aRefVarString.getLength();
        sal_Int32 nValStrLen = aRefValString.getLength();

        OUStringBuffer aNewStr(nVarStrLen);
        if (nVarStrLen > nValStrLen)
        {
            comphelper::string::padToLength(aNewStr, nVarStrLen - nValStrLen, ' ');
            aNewStr.append(aRefValString);
        }
        else
        {
            aNewStr.append(aRefValString.copy(0, nVarStrLen));
        }
        refVar->PutString(aNewStr.makeStringAndClear());

        refVar->SetFlags( n );
    }
}

// laying down TOS in TOS-1, then set ReadOnly-Bit

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
// TOS = variable for the array with dimension information as parameter

void SbiRuntime::StepDIM()
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );
}

// #56204 swap out DIM-functionality into a help method (step0.cxx)
void SbiRuntime::DimImpl( SbxVariableRef refVar )
{
    // If refDim then this DIM statement is terminating a ReDIM and
    // previous StepERASE_CLEAR for an array, the following actions have
    // been delayed from ( StepERASE_CLEAR ) 'till here
    if ( refRedim )
    {
        if ( !refRedimpArray ) // only erase the array not ReDim Preserve
        {
            lcl_eraseImpl( refVar, bVBAEnabled );
        }
        SbxDataType eType = refVar->GetType();
        lcl_clearImpl( refVar, eType );
        refRedim = NULL;
    }
    SbxArray* pDims = refVar->GetParameters();
    // must have an even number of arguments
    // have in mind that Arg[0] does not count!
    if( pDims && !( pDims->Count() & 1 ) )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    }
    else
    {
        SbxDataType eType = refVar->IsFixed() ? refVar->GetType() : SbxVARIANT;
        SbxDimArray* pArray = new SbxDimArray( eType );
        // allow arrays without dimension information, too (VB-compatible)
        if( pDims )
        {
            refVar->ResetFlag( SBX_VAR_TO_DIM );

            for( sal_uInt16 i = 1; i < pDims->Count(); )
            {
                sal_Int32 lb = pDims->Get( i++ )->GetLong();
                sal_Int32 ub = pDims->Get( i++ )->GetLong();
                if( ub < lb )
                {
                    Error( SbERR_OUT_OF_RANGE ), ub = lb;
                }
                pArray->AddDim32( lb, ub );
                if ( lb != ub )
                {
                    pArray->setHasFixedSize( true );
                }
            }
        }
        else
        {
            // #62867 On creating an array of the length 0, create
            // a dimension (like for Uno-Sequences of the length 0)
            pArray->unoAddDim( 0, -1 );
        }
        sal_uInt16 nSavFlags = refVar->GetFlags();
        refVar->ResetFlag( SBX_FIXED );
        refVar->PutObject( pArray );
        refVar->SetFlags( nSavFlags );
        refVar->SetParameters( NULL );
    }
}

// REDIM
// TOS  = variable for the array
// argv = dimension information

void SbiRuntime::StepREDIM()
{
    // Nothing different than dim at the moment because
    // a double dim is already recognized by the compiler.
    StepDIM();
}


// Helper function for StepREDIMP
void implCopyDimArray( SbxDimArray* pNewArray, SbxDimArray* pOldArray, short nMaxDimIndex,
    short nActualDim, sal_Int32* pActualIndices, sal_Int32* pLowerBounds, sal_Int32* pUpperBounds )
{
    sal_Int32& ri = pActualIndices[nActualDim];
    for( ri = pLowerBounds[nActualDim] ; ri <= pUpperBounds[nActualDim] ; ri++ )
    {
        if( nActualDim < nMaxDimIndex )
        {
            implCopyDimArray( pNewArray, pOldArray, nMaxDimIndex, nActualDim + 1,
                pActualIndices, pLowerBounds, pUpperBounds );
        }
        else
        {
            SbxVariable* pSource = pOldArray->Get32( pActualIndices );
            SbxVariable* pDest   = pNewArray->Get32( pActualIndices );
            if( pSource && pDest )
            {
                *pDest = *pSource;
            }
        }
    }
}

// REDIM PRESERVE
// TOS  = variable for the array
// argv = dimension information

void SbiRuntime::StepREDIMP()
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );

    // Now check, if we can copy from the old array
    if( refRedimpArray.Is() )
    {
        SbxBase* pElemObj = refVar->GetObject();
        SbxDimArray* pNewArray = PTR_CAST(SbxDimArray,pElemObj);
        SbxDimArray* pOldArray = (SbxDimArray*)(SbxArray*)refRedimpArray;
        if( pNewArray )
        {
            short nDimsNew = pNewArray->GetDims();
            short nDimsOld = pOldArray->GetDims();
            short nDims = nDimsNew;

            if( nDimsOld != nDimsNew )
            {
                StarBASIC::Error( SbERR_OUT_OF_RANGE );
            }
            else if (nDims > 0)
            {
                // Store dims to use them for copying later
                sal_Int32* pLowerBounds = new sal_Int32[nDims];
                sal_Int32* pUpperBounds = new sal_Int32[nDims];
                sal_Int32* pActualIndices = new sal_Int32[nDims];

                // Compare bounds
                for( short i = 1 ; i <= nDims ; i++ )
                {
                    sal_Int32 lBoundNew, uBoundNew;
                    sal_Int32 lBoundOld, uBoundOld;
                    pNewArray->GetDim32( i, lBoundNew, uBoundNew );
                    pOldArray->GetDim32( i, lBoundOld, uBoundOld );
                    lBoundNew = std::max( lBoundNew, lBoundOld );
                    uBoundNew = std::min( uBoundNew, uBoundOld );
                    short j = i - 1;
                    pActualIndices[j] = pLowerBounds[j] = lBoundNew;
                    pUpperBounds[j] = uBoundNew;
                }
                // Copy data from old array by going recursively through all dimensions
                // (It would be faster to work on the flat internal data array of an
                // SbyArray but this solution is clearer and easier)
                implCopyDimArray( pNewArray, pOldArray, nDims - 1,
                                  0, pActualIndices, pLowerBounds, pUpperBounds );
                delete[] pUpperBounds;
                delete[] pLowerBounds;
                delete[] pActualIndices;
            }

            refRedimpArray = NULL;
        }
    }

}

// REDIM_COPY
// TOS  = Array-Variable, Reference to array is copied
//        Variable is cleared as in ERASE

void SbiRuntime::StepREDIMP_ERASE()
{
    SbxVariableRef refVar = PopVar();
    refRedim = refVar;
    SbxDataType eType = refVar->GetType();
    if( eType & SbxARRAY )
    {
        SbxBase* pElemObj = refVar->GetObject();
        SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
        if( pDimArray )
        {
            refRedimpArray = pDimArray;
        }

    }
    else if( refVar->IsFixed() )
    {
        refVar->Clear();
    }
    else
    {
        refVar->SetType( SbxEMPTY );
    }
}

static void lcl_clearImpl( SbxVariableRef& refVar, SbxDataType& eType )
{
    sal_uInt16 nSavFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->SetType( SbxDataType(eType & 0x0FFF) );
    refVar->SetFlags( nSavFlags );
    refVar->Clear();
}

static void lcl_eraseImpl( SbxVariableRef& refVar, bool bVBAEnabled )
{
    SbxDataType eType = refVar->GetType();
    if( eType & SbxARRAY )
    {
        if ( bVBAEnabled )
        {
            SbxBase* pElemObj = refVar->GetObject();
            SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
            bool bClearValues = true;
            if( pDimArray )
            {
                if ( pDimArray->hasFixedSize() )
                {
                    // Clear all Value(s)
                    pDimArray->SbxArray::Clear();
                    bClearValues = false;
                }
                else
                {
                    pDimArray->Clear(); // clear Dims
                }
            }
            if ( bClearValues )
            {
                SbxArray* pArray = PTR_CAST(SbxArray,pElemObj);
                if ( pArray )
                {
                    pArray->Clear();
                }
            }
        }
        else
        {
            // Arrays have on an erase to VB quite a complex behaviour. Here are
            // only the type problems at REDIM (#26295) removed at first:
            // Set type hard onto the array-type, because a variable with array is
            // SbxOBJECT. At REDIM there's an SbxOBJECT-array generated then and
            // the original type is lost -> runtime error
            lcl_clearImpl( refVar, eType );
        }
    }
    else if( refVar->IsFixed() )
    {
        refVar->Clear();
    }
    else
    {
        refVar->SetType( SbxEMPTY );
    }
}

// delete variable
// TOS = variable

void SbiRuntime::StepERASE()
{
    SbxVariableRef refVar = PopVar();
    lcl_eraseImpl( refVar, bVBAEnabled );
}

void SbiRuntime::StepERASE_CLEAR()
{
    refRedim = PopVar();
}

void SbiRuntime::StepARRAYACCESS()
{
    if( !refArgv )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    }
    SbxVariableRef refVar = PopVar();
    refVar->SetParameters( refArgv );
    PopArgv();
    PushVar( CheckArray( refVar ) );
}

void SbiRuntime::StepBYVAL()
{
    // Copy variable on stack to break call by reference
    SbxVariableRef pVar = PopVar();
    SbxDataType t = pVar->GetType();

    SbxVariable* pCopyVar = new SbxVariable( t );
    pCopyVar->SetFlag( SBX_READWRITE );
    *pCopyVar = *pVar;

    PushVar( pCopyVar );
}

// establishing an argv
// nOp1 stays as it is -> 1st element is the return value

void SbiRuntime::StepARGC()
{
    PushArgv();
    refArgv = new SbxArray;
    nArgc = 1;
}

// storing an argument in Argv

void SbiRuntime::StepARGV()
{
    if( !refArgv )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    }
    else
    {
        SbxVariableRef pVal = PopVar();

        // Before fix of #94916:
        if( pVal->ISA(SbxMethod) || pVal->ISA(SbUnoProperty) || pVal->ISA(SbProcedureProperty) )
        {
            // evaluate methods and properties!
            SbxVariable* pRes = new SbxVariable( *pVal );
            pVal = pRes;
        }
        refArgv->Put( pVal, nArgc++ );
    }
}

// Input to Variable. The variable is on TOS and is
// is removed afterwards.
void SbiRuntime::StepINPUT()
{
    OUStringBuffer sin;
    OUString s;
    char ch = 0;
    SbError err;
    // Skip whitespace
    while( ( err = pIosys->GetError() ) == 0 )
    {
        ch = pIosys->Read();
        if( ch != ' ' && ch != '\t' && ch != '\n' )
        {
            break;
        }
    }
    if( !err )
    {
        // Scan until comma or whitespace
        char sep = ( ch == '"' ) ? ch : 0;
        if( sep )
        {
            ch = pIosys->Read();
        }
        while( ( err = pIosys->GetError() ) == 0 )
        {
            if( ch == sep )
            {
                ch = pIosys->Read();
                if( ch != sep )
                {
                    break;
                }
            }
            else if( !sep && (ch == ',' || ch == '\n') )
            {
                break;
            }
            sin.append( ch );
            ch = pIosys->Read();
        }
        // skip whitespace
        if( ch == ' ' || ch == '\t' )
        {
            while( ( err = pIosys->GetError() ) == 0 )
            {
                if( ch != ' ' && ch != '\t' && ch != '\n' )
                {
                    break;
                }
                ch = pIosys->Read();
            }
        }
    }
    if( !err )
    {
        s = sin.makeStringAndClear();
        SbxVariableRef pVar = GetTOS();
        // try to fill the variable with a numeric value first,
        // then with a string value
        if( !pVar->IsFixed() || pVar->IsNumeric() )
        {
            sal_uInt16 nLen = 0;
            if( !pVar->Scan( s, &nLen ) )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
            }
            // the value has to be scanned in completely
            else if( nLen != s.getLength() && !pVar->PutString( s ) )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
            }
            else if( nLen != s.getLength() && pVar->IsNumeric() )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
                if( !err )
                {
                    err = SbERR_CONVERSION;
                }
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
    {
        Error( err );
    }
    else if( err )
    {
        if( pRestart && !pIosys->GetChannel() )
        {
            pCode = pRestart;
        }
        else
        {
            Error( err );
        }
    }
    else
    {
        PopVar();
    }
}

// Line Input to Variable. The variable is on TOS and is
// deleted afterwards.

void SbiRuntime::StepLINPUT()
{
    OString aInput;
    pIosys->Read( aInput );
    Error( pIosys->GetError() );
    SbxVariableRef p = PopVar();
    p->PutString(OStringToOUString(aInput, osl_getThreadTextEncoding()));
}

// end of program

void SbiRuntime::StepSTOP()
{
    pInst->Stop();
}


void SbiRuntime::StepINITFOR()
{
    PushFor();
}

void SbiRuntime::StepINITFOREACH()
{
    PushForEach();
}

// increment FOR-variable

void SbiRuntime::StepNEXT()
{
    if( !pForStk )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        return;
    }
    if( pForStk->eForType == FOR_TO )
    {
        pForStk->refVar->Compute( SbxPLUS, *pForStk->refInc );
    }
}

// beginning CASE: TOS in CASE-stack

void SbiRuntime::StepCASE()
{
    if( !refCaseStk.Is() )
    {
        refCaseStk = new SbxArray;
    }
    SbxVariableRef xVar = PopVar();
    refCaseStk->Put( xVar, refCaseStk->Count() );
}

// end CASE: free variable

void SbiRuntime::StepENDCASE()
{
    if( !refCaseStk || !refCaseStk->Count() )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    }
    else
    {
        refCaseStk->Remove( refCaseStk->Count() - 1 );
    }
}


void SbiRuntime::StepSTDERROR()
{
    pError = NULL; bError = true;
    pInst->aErrorMsg = OUString();
    pInst->nErr = 0L;
    pInst->nErl = 0;
    nError = 0L;
    SbxErrObject::getUnoErrObject()->Clear();
}

void SbiRuntime::StepNOERROR()
{
    pInst->aErrorMsg = OUString();
    pInst->nErr = 0L;
    pInst->nErl = 0;
    nError = 0L;
    SbxErrObject::getUnoErrObject()->Clear();
    bError = false;
}

// leave UP

void SbiRuntime::StepLEAVE()
{
    bRun = false;
        // If VBA and we are leaving an ErrorHandler then clear the error ( it's been processed )
    if ( bInError && pError )
    {
        SbxErrObject::getUnoErrObject()->Clear();
    }
}

void SbiRuntime::StepCHANNEL()      // TOS = channel number
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
    OUString s1 = p->GetOUString();
    OUString s;
    if( p->GetType() >= SbxINTEGER && p->GetType() <= SbxDOUBLE )
    {
        s = " ";    // one blank before
    }
    s += s1;
    OString aByteStr(OUStringToOString(s, osl_getThreadTextEncoding()));
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepPRINTF()       // print TOS in field
{
    SbxVariableRef p = PopVar();
    OUString s1 = p->GetOUString();
    OUStringBuffer s;
    if( p->GetType() >= SbxINTEGER && p->GetType() <= SbxDOUBLE )
    {
        s.append(' ');
    }
    s.append(s1);
    comphelper::string::padToLength(s, 14, ' ');
    OString aByteStr(OUStringToOString(s.makeStringAndClear(), osl_getThreadTextEncoding()));
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepWRITE()        // write TOS
{
    SbxVariableRef p = PopVar();
    // Does the string have to be encapsulated?
    char ch = 0;
    switch (p->GetType() )
    {
    case SbxSTRING: ch = '"'; break;
    case SbxCURRENCY:
    case SbxBOOL:
    case SbxDATE: ch = '#'; break;
    default: break;
    }
    OUString s;
    if( ch )
    {
        s += OUString(ch);
    }
    s += p->GetOUString();
    if( ch )
    {
        s += OUString(ch);
    }
    OString aByteStr(OUStringToOString(s, osl_getThreadTextEncoding()));
    pIosys->Write( aByteStr );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepRENAME()       // Rename Tos+1 to Tos
{
    SbxVariableRef pTos1 = PopVar();
    SbxVariableRef pTos  = PopVar();
    OUString aDest = pTos1->GetOUString();
    OUString aSource = pTos->GetOUString();

    if( hasUno() )
    {
        implStepRenameUCB( aSource, aDest );
    }
    else
    {
        implStepRenameOSL( aSource, aDest );
    }
}

// TOS = Prompt

void SbiRuntime::StepPROMPT()
{
    SbxVariableRef p = PopVar();
    OString aStr(OUStringToOString(p->GetOUString(), osl_getThreadTextEncoding()));
    pIosys->SetPrompt( aStr );
}

// Set Restart point

void SbiRuntime::StepRESTART()
{
    pRestart = pCode;
}

// empty expression on stack for missing parameter

void SbiRuntime::StepEMPTY()
{
    // #57915 The semantics of StepEMPTY() is the representation of a missing argument.
    // This is represented by the value 448 (SbERR_NAMED_NOT_FOUND) of the type error
    // in VB. StepEmpty should now rather be named StepMISSING() but the name is kept
    // to simplify matters.
    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
    xVar->PutErr( 448 );
    PushVar( xVar );
}

// TOS = error code

void SbiRuntime::StepERROR()
{
    SbxVariableRef refCode = PopVar();
    sal_uInt16 n = refCode->GetUShort();
    SbError error = StarBASIC::GetSfxFromVBError( n );
    if ( bVBAEnabled )
    {
        pInst->Error( error );
    }
    else
    {
        Error( error );
    }
}

// loading a numeric constant (+ID)

void SbiRuntime::StepLOADNC( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable( SbxDOUBLE );

    // #57844 use localized function
    OUString aStr = pImg->GetString( static_cast<short>( nOp1 ) );
    // also allow , !!!
    sal_Int32 iComma = aStr.indexOf((sal_Unicode)',');
    if( iComma >= 0 )
    {
        aStr = aStr.replaceAt(iComma, 1, OUString("."));
    }
    double n = ::rtl::math::stringToDouble( aStr, '.', ',', NULL, NULL );

    p->PutDouble( n );
    PushVar( p );
}

// loading a string constant (+ID)

void SbiRuntime::StepLOADSC( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable;
    p->PutString( pImg->GetString( static_cast<short>( nOp1 ) ) );
    PushVar( p );
}

// Immediate Load (+Wert)

void SbiRuntime::StepLOADI( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable;
    p->PutInteger( static_cast<sal_Int16>( nOp1 ) );
    PushVar( p );
}

// stora a named argument in Argv (+Arg-no. from 1!)

void SbiRuntime::StepARGN( sal_uInt32 nOp1 )
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        OUString aAlias( pImg->GetString( static_cast<short>( nOp1 ) ) );
        SbxVariableRef pVal = PopVar();
        if( bVBAEnabled && ( pVal->ISA(SbxMethod) || pVal->ISA(SbUnoProperty) || pVal->ISA(SbProcedureProperty) ) )
        {
            // named variables ( that are Any especially properties ) can be empty at this point and need a broadcast
            if ( pVal->GetType() == SbxEMPTY )
                pVal->Broadcast( SBX_HINT_DATAWANTED );
            // evaluate methods and properties!
            SbxVariable* pRes = new SbxVariable( *pVal );
            pVal = pRes;
        }
        refArgv->Put( pVal, nArgc );
        refArgv->PutAlias( aAlias, nArgc++ );
    }
}

// converting the type of an argument in Argv for DECLARE-Fkt. (+type)

void SbiRuntime::StepARGTYP( sal_uInt32 nOp1 )
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        bool bByVal = (nOp1 & 0x8000) != 0;         // Ist BYVAL requested?
        SbxDataType t = (SbxDataType) (nOp1 & 0x7FFF);
        SbxVariable* pVar = refArgv->Get( refArgv->Count() - 1 );   // last Arg

        // check BYVAL
        if( pVar->GetRefCount() > 2 )       // 2 is normal for BYVAL
        {
            // parameter is a reference
            if( bByVal )
            {
                // Call by Value is requested -> create a copy
                pVar = new SbxVariable( *pVar );
                pVar->SetFlag( SBX_READWRITE );
                refExprStk->Put( pVar, refArgv->Count() - 1 );
            }
            else
                pVar->SetFlag( SBX_REFERENCE );     // Ref-Flag for DllMgr
        }
        else
        {
            // parameter is NO reference
            if( bByVal )
                pVar->ResetFlag( SBX_REFERENCE );   // no reference -> OK
            else
                Error( SbERR_BAD_PARAMETERS );      // reference needed
        }

        if( pVar->GetType() != t )
        {
            // variant for correct conversion
            // besides error, if SbxBYREF
            pVar->Convert( SbxVARIANT );
            pVar->Convert( t );
        }
    }
}

// bring string to a definite length (+length)

void SbiRuntime::StepPAD( sal_uInt32 nOp1 )
{
    SbxVariable* p = GetTOS();
    OUString s = p->GetOUString();
    sal_Int32 nLen(nOp1);
    if( s.getLength() != nLen )
    {
        OUStringBuffer aBuf(s);
        if (aBuf.getLength() > nLen)
        {
            comphelper::string::truncateToLength(aBuf, nLen);
        }
        else
        {
            comphelper::string::padToLength(aBuf, nLen, ' ');
        }
        s = aBuf.makeStringAndClear();
    }
}

// jump (+target)

void SbiRuntime::StepJUMP( sal_uInt32 nOp1 )
{
#ifdef DBG_UTIL
    // #QUESTION shouln't this be
    // if( (sal_uInt8*)( nOp1+pImagGetCode() ) >= pImg->GetCodeSize() )
    if( nOp1 >= pImg->GetCodeSize() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
#endif
    pCode = (const sal_uInt8*) pImg->GetCode() + nOp1;
}

// evaluate TOS, conditional jump (+target)

void SbiRuntime::StepJUMPT( sal_uInt32 nOp1 )
{
    SbxVariableRef p = PopVar();
    if( p->GetBool() )
        StepJUMP( nOp1 );
}

// evaluate TOS, conditional jump (+target)

void SbiRuntime::StepJUMPF( sal_uInt32 nOp1 )
{
    SbxVariableRef p = PopVar();
    // In a test e.g. If Null then
        // will evaluate Null will act as if False
    if( ( bVBAEnabled && p->IsNull() ) || !p->GetBool() )
        StepJUMP( nOp1 );
}

// evaluate TOS, jump into JUMP-table (+MaxVal)
// looks like this:
// ONJUMP 2
// JUMP target1
// JUMP target2
// ...
// if 0x8000 is set in the operand, push the return address (ON..GOSUB)

void SbiRuntime::StepONJUMP( sal_uInt32 nOp1 )
{
    SbxVariableRef p = PopVar();
    sal_Int16 n = p->GetInteger();
    if( nOp1 & 0x8000 )
    {
        nOp1 &= 0x7FFF;
        PushGosub( pCode + 5 * nOp1 );
    }
    if( n < 1 || static_cast<sal_uInt32>(n) > nOp1 )
        n = static_cast<sal_Int16>( nOp1 + 1 );
    nOp1 = (sal_uInt32) ( (const char*) pCode - pImg->GetCode() ) + 5 * --n;
    StepJUMP( nOp1 );
}

// UP-call (+target)

void SbiRuntime::StepGOSUB( sal_uInt32 nOp1 )
{
    PushGosub( pCode );
    if( nOp1 >= pImg->GetCodeSize() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    pCode = (const sal_uInt8*) pImg->GetCode() + nOp1;
}

// UP-return (+0 or target)

void SbiRuntime::StepRETURN( sal_uInt32 nOp1 )
{
    PopGosub();
    if( nOp1 )
        StepJUMP( nOp1 );
}

// check FOR-variable (+Endlabel)

void SbiRuntime::StepTESTFOR( sal_uInt32 nOp1 )
{
    if( !pForStk )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        return;
    }

    bool bEndLoop = false;
    switch( pForStk->eForType )
    {
        case FOR_TO:
        {
            SbxOperator eOp = ( pForStk->refInc->GetDouble() < 0 ) ? SbxLT : SbxGT;
            if( pForStk->refVar->Compare( eOp, *pForStk->refEnd ) )
                bEndLoop = true;
            break;
        }
        case FOR_EACH_ARRAY:
        {
            SbiForStack* p = pForStk;
            if( p->pArrayCurIndices == NULL )
            {
                bEndLoop = true;
            }
            else
            {
                SbxDimArray* pArray = (SbxDimArray*)(SbxVariable*)p->refEnd;
                short nDims = pArray->GetDims();

                // Empty array?
                if( nDims == 1 && p->pArrayLowerBounds[0] > p->pArrayUpperBounds[0] )
                {
                    bEndLoop = true;
                    break;
                }
                SbxVariable* pVal = pArray->Get32( p->pArrayCurIndices );
                *(p->refVar) = *pVal;

                bool bFoundNext = false;
                for( short i = 0 ; i < nDims ; i++ )
                {
                    if( p->pArrayCurIndices[i] < p->pArrayUpperBounds[i] )
                    {
                        bFoundNext = true;
                        p->pArrayCurIndices[i]++;
                        for( short j = i - 1 ; j >= 0 ; j-- )
                            p->pArrayCurIndices[j] = p->pArrayLowerBounds[j];
                        break;
                    }
                }
                if( !bFoundNext )
                {
                    delete[] p->pArrayCurIndices;
                    p->pArrayCurIndices = NULL;
                }
            }
            break;
        }
        case FOR_EACH_COLLECTION:
        {
            BasicCollection* pCollection = (BasicCollection*)(SbxVariable*)pForStk->refEnd;
            SbxArrayRef xItemArray = pCollection->xItemArray;
            sal_Int32 nCount = xItemArray->Count32();
            if( pForStk->nCurCollectionIndex < nCount )
            {
                SbxVariable* pRes = xItemArray->Get32( pForStk->nCurCollectionIndex );
                pForStk->nCurCollectionIndex++;
                (*pForStk->refVar) = *pRes;
            }
            else
            {
                bEndLoop = true;
            }
            break;
        }
        case FOR_EACH_XENUMERATION:
        {
            SbiForStack* p = pForStk;
            if( p->xEnumeration->hasMoreElements() )
            {
                Any aElem = p->xEnumeration->nextElement();
                SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
                unoToSbxValue( (SbxVariable*)xVar, aElem );
                (*pForStk->refVar) = *xVar;
            }
            else
            {
                bEndLoop = true;
            }
            break;
        }
    }
    if( bEndLoop )
    {
        PopFor();
        StepJUMP( nOp1 );
    }
}

// Tos+1 <= Tos+2 <= Tos, 2xremove (+Target)

void SbiRuntime::StepCASETO( sal_uInt32 nOp1 )
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


void SbiRuntime::StepERRHDL( sal_uInt32 nOp1 )
{
    const sal_uInt8* p = pCode;
    StepJUMP( nOp1 );
    pError = pCode;
    pCode = p;
    pInst->aErrorMsg = OUString();
    pInst->nErr = 0;
    pInst->nErl = 0;
    nError = 0;
    SbxErrObject::getUnoErrObject()->Clear();
}

// Resume after errors (+0=statement, 1=next or Label)

void SbiRuntime::StepRESUME( sal_uInt32 nOp1 )
{
    // #32714 Resume without error? -> error
    if( !bInError )
    {
        Error( SbERR_BAD_RESUME );
        return;
    }
    if( nOp1 )
    {
        // set Code-pointer to the next statement
        sal_uInt16 n1, n2;
        pCode = pMod->FindNextStmnt( pErrCode, n1, n2, sal_True, pImg );
    }
    else
        pCode = pErrStmnt;
    if ( pError ) // current in error handler ( and got a Resume Next statement )
        SbxErrObject::getUnoErrObject()->Clear();

    if( nOp1 > 1 )
        StepJUMP( nOp1 );
    pInst->aErrorMsg = OUString();
    pInst->nErr = 0;
    pInst->nErl = 0;
    nError = 0;
    bInError = false;
}

// close channel (+channel, 0=all)
void SbiRuntime::StepCLOSE( sal_uInt32 nOp1 )
{
    SbError err;
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

// output character (+char)

void SbiRuntime::StepPRCHAR( sal_uInt32 nOp1 )
{
    OString s(static_cast<sal_Char>(nOp1));
    pIosys->Write( s );
    Error( pIosys->GetError() );
}

// check whether TOS is a certain object class (+StringID)

bool SbiRuntime::implIsClass( SbxObject* pObj, const OUString& aClass )
{
    bool bRet = true;

    if( !aClass.isEmpty() )
    {
        bRet = pObj->IsClass( aClass );
        if( !bRet )
            bRet = aClass.equalsIgnoreAsciiCase( "object" );
        if( !bRet )
        {
            OUString aObjClass = pObj->GetClassName();
            SbModule* pClassMod = GetSbData()->pClassFac->FindClass( aObjClass );
            SbClassData* pClassData;
            if( pClassMod && (pClassData=pClassMod->pClassData) != NULL )
            {
                SbxVariable* pClassVar = pClassData->mxIfaces->Find( aClass, SbxCLASS_DONTCARE );
                bRet = (pClassVar != NULL);
            }
        }
    }
    return bRet;
}

bool SbiRuntime::checkClass_Impl( const SbxVariableRef& refVal,
    const OUString& aClass, bool bRaiseErrors, bool bDefault )
{
    bool bOk = bDefault;

    SbxDataType t = refVal->GetType();
    SbxVariable* pVal = (SbxVariable*)refVal;
    // we don't know the type of uno properties that are (maybevoid)
    if ( t == SbxEMPTY && refVal->ISA(SbUnoProperty) )
    {
        SbUnoProperty* pProp = (SbUnoProperty*)pVal;
        t = pProp->getRealType();
    }
    if( t == SbxOBJECT )
    {
        SbxObject* pObj;
        if( pVal->IsA( TYPE(SbxObject) ) )
            pObj = (SbxObject*) pVal;
        else
        {
            pObj = (SbxObject*) refVal->GetObject();
            if( pObj && !pObj->IsA( TYPE(SbxObject) ) )
                pObj = NULL;
        }
        if( pObj )
        {
            if( !implIsClass( pObj, aClass ) )
            {
                if ( ( bVBAEnabled || CodeCompleteOptions::IsExtendedTypeDeclaration() ) && pObj->IsA( TYPE(SbUnoObject) ) )
                {
                    SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
                    bOk = checkUnoObjectType( pUnoObj, aClass );
                }
                else
                    bOk = false;
                if ( !bOk )
                {
                    if( bRaiseErrors )
                        Error( SbERR_INVALID_USAGE_OBJECT );
                }
            }
            else
            {
                bOk = true;

                SbClassModuleObject* pClassModuleObject = PTR_CAST(SbClassModuleObject,pObj);
                if( pClassModuleObject != NULL )
                    pClassModuleObject->triggerInitializeEvent();
            }
        }
    }
    else
    {
        if ( !bVBAEnabled )
        {
            if( bRaiseErrors )
                Error( SbERR_NEEDS_OBJECT );
            bOk = false;
        }
    }
    return bOk;
}

void SbiRuntime::StepSETCLASS_impl( sal_uInt32 nOp1, bool bHandleDflt )
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    OUString aClass( pImg->GetString( static_cast<short>( nOp1 ) ) );

    bool bOk = checkClass_Impl( refVal, aClass, true );
    if( bOk )
    {
        StepSET_Impl( refVal, refVar, bHandleDflt ); // don't do handle dflt prop for a "proper" set
    }
}

void SbiRuntime::StepVBASETCLASS( sal_uInt32 nOp1 )
{
    StepSETCLASS_impl( nOp1, false );
}

void SbiRuntime::StepSETCLASS( sal_uInt32 nOp1 )
{
    StepSETCLASS_impl( nOp1, true );
}

void SbiRuntime::StepTESTCLASS( sal_uInt32 nOp1 )
{
    SbxVariableRef xObjVal = PopVar();
    OUString aClass( pImg->GetString( static_cast<short>( nOp1 ) ) );
    bool bDefault = !bVBAEnabled;
    bool bOk = checkClass_Impl( xObjVal, aClass, false, bDefault );

    SbxVariable* pRet = new SbxVariable;
    pRet->PutBool( bOk );
    PushVar( pRet );
}

// define library for following declare-call

void SbiRuntime::StepLIB( sal_uInt32 nOp1 )
{
    aLibName = pImg->GetString( static_cast<short>( nOp1 ) );
}

// TOS is incremented by BASE, BASE is pushed before (+BASE)
// This opcode is pushed before DIM/REDIM-commands,
// if there's been only one index named.

void SbiRuntime::StepBASED( sal_uInt32 nOp1 )
{
    SbxVariable* p1 = new SbxVariable;
    SbxVariableRef x2 = PopVar();

    // #109275 Check compatiblity mode
    bool bCompatible = ((nOp1 & 0x8000) != 0);
    sal_uInt16 uBase = static_cast<sal_uInt16>(nOp1 & 1);       // Can only be 0 or 1
    p1->PutInteger( uBase );
    if( !bCompatible )
        x2->Compute( SbxPLUS, *p1 );
    PushVar( x2 );  // first the Expr
    PushVar( p1 );  // then the Base
}

// the bits in the String-ID:
// 0x8000 - Argv is reserved

SbxVariable* SbiRuntime::FindElement( SbxObject* pObj, sal_uInt32 nOp1, sal_uInt32 nOp2,
                                      SbError nNotFound, bool bLocal, bool bStatic )
{
    bool bIsVBAInterOp = SbiRuntime::isVBAEnabled();
    if( bIsVBAInterOp )
    {
        StarBASIC* pMSOMacroRuntimeLib = GetSbData()->pMSOMacroRuntimLib;
        if( pMSOMacroRuntimeLib != NULL )
        {
            pMSOMacroRuntimeLib->ResetFlag( SBX_EXTSEARCH );
        }
    }

    SbxVariable* pElem = NULL;
    if( !pObj )
    {
        Error( SbERR_NO_OBJECT );
        pElem = new SbxVariable;
    }
    else
    {
        bool bFatalError = false;
        SbxDataType t = (SbxDataType) nOp2;
        OUString aName( pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) ) );
        // Hacky capture of Evaluate [] syntax
        // this should be tackled I feel at the pcode level
        if ( bIsVBAInterOp && aName.indexOf((sal_Unicode)'[') == 0 )
        {
            // emulate pcode here
            StepARGC();
            // psuedo StepLOADSC
            OUString sArg = aName.copy( 1, aName.getLength() - 2 );
            SbxVariable* p = new SbxVariable;
            p->PutString( sArg );
            PushVar( p );
            StepARGV();
            nOp1 = nOp1 | 0x8000; // indicate params are present
            aName = OUString("Evaluate");
        }
        if( bLocal )
        {
            if ( bStatic )
            {
                if ( pMeth )
                {
                    pElem = pMeth->GetStatics()->Find( aName, SbxCLASS_DONTCARE );
                }
            }

            if ( !pElem )
            {
                pElem = refLocals->Find( aName, SbxCLASS_DONTCARE );
            }
        }
        if( !pElem )
        {
            bool bSave = rBasic.bNoRtl;
            rBasic.bNoRtl = true;
            pElem = pObj->Find( aName, SbxCLASS_DONTCARE );

            // #110004, #112015: Make private really private
            if( bLocal && pElem )   // Local as flag for global search
            {
                if( pElem->IsSet( SBX_PRIVATE ) )
                {
                    SbiInstance* pInst_ = GetSbData()->pInst;
                    if( pInst_ && pInst_->IsCompatibility() && pObj != pElem->GetParent() )
                    {
                        pElem = NULL;   // Found but in wrong module!
                    }
                    // Interfaces: Use SBX_EXTFOUND
                }
            }
            rBasic.bNoRtl = bSave;

            // is it a global uno-identifier?
            if( bLocal && !pElem )
            {
                bool bSetName = true; // preserve normal behaviour

                // i#i68894# if VBAInterOp favour searching vba globals
                // over searching for uno classess
                if ( bVBAEnabled )
                {
                    // Try Find in VBA symbols space
                    pElem = rBasic.VBAFind( aName, SbxCLASS_DONTCARE );
                    if ( pElem )
                    {
                        bSetName = false; // don't overwrite uno name
                    }
                    else
                    {
                        pElem = VBAConstantHelper::instance().getVBAConstant( aName );
                    }
                }

                if( !pElem )
                {
                    // #72382 ATTENTION! ALWAYS returns a result now
                    // because of unknown modules!
                    SbUnoClass* pUnoClass = findUnoClass( aName );
                    if( pUnoClass )
                    {
                        pElem = new SbxVariable( t );
                        SbxValues aRes( SbxOBJECT );
                        aRes.pObj = pUnoClass;
                        pElem->SbxVariable::Put( aRes );
                    }
                }

                // #62939 If an uno-class has been found, the wrapper
                // object has to be held, because the uno-class, e. g.
                // "stardiv", has to be read out of the registry
                // every time again otherwise
                if( pElem )
                {
                    // #63774 May not be saved too!!!
                    pElem->SetFlag( SBX_DONTSTORE );
                    pElem->SetFlag( SBX_NO_MODIFY);

                    // #72382 save locally, all variables that have been declared
                    // implicit would become global automatically otherwise!
                    if ( bSetName )
                    {
                        pElem->SetName( aName );
                    }
                    refLocals->Put( pElem, refLocals->Count() );
                }
            }

            if( !pElem )
            {
                // not there and not in the object?
                // don't establish if that thing has parameters!
                if( nOp1 & 0x8000 )
                {
                    bFatalError = true;
                }

                // else, if there are parameters, use different error code
                if( !bLocal || pImg->GetFlag( SBIMG_EXPLICIT ) )
                {
                    // #39108 if explicit and as ELEM always a fatal error
                    bFatalError = true;


                    if( !( nOp1 & 0x8000 ) && nNotFound == SbERR_PROC_UNDEFINED )
                    {
                        nNotFound = SbERR_VAR_UNDEFINED;
                    }
                }
                if( bFatalError )
                {
                    // #39108 use dummy variable instead of fatal error
                    if( !xDummyVar.Is() )
                    {
                        xDummyVar = new SbxVariable( SbxVARIANT );
                    }
                    pElem = xDummyVar;

                    ClearArgvStack();

                    Error( nNotFound, aName );
                }
                else
                {
                    if ( bStatic )
                    {
                        pElem = StepSTATIC_Impl( aName, t );
                    }
                    if ( !pElem )
                    {
                        pElem = new SbxVariable( t );
                        if( t != SbxVARIANT )
                        {
                            pElem->SetFlag( SBX_FIXED );
                        }
                        pElem->SetName( aName );
                        refLocals->Put( pElem, refLocals->Count() );
                    }
                }
            }
        }
        // #39108 Args can already be deleted!
        if( !bFatalError )
        {
            SetupArgs( pElem, nOp1 );
        }
        // because a particular call-type is requested
        if( pElem->IsA( TYPE(SbxMethod) ) )
        {
            // shall the type be converted?
            SbxDataType t2 = pElem->GetType();
            bool bSet = false;
            if( !( pElem->GetFlags() & SBX_FIXED ) )
            {
                if( t != SbxVARIANT && t != t2 &&
                    t >= SbxINTEGER && t <= SbxSTRING )
                {
                    pElem->SetType( t ), bSet = true;
                }
            }
            // assign pElem to a Ref, to delete a temp-var if applicable
            SbxVariableRef refTemp = pElem;

            // remove potential rests of the last call of the SbxMethod
            // free Write before, so that there's no error
            sal_uInt16 nSavFlags = pElem->GetFlags();
            pElem->SetFlag( SBX_READWRITE | SBX_NO_BROADCAST );
            pElem->SbxValue::Clear();
            pElem->SetFlags( nSavFlags );

            // don't touch before setting, as e. g. LEFT()
            // has to know the difference between Left$() and Left()

            // because the methods' parameters are cut away in PopVar()
            SbxVariable* pNew = new SbxMethod( *((SbxMethod*)pElem) );
            //OLD: SbxVariable* pNew = new SbxVariable( *pElem );

            pElem->SetParameters(0);
            pNew->SetFlag( SBX_READWRITE );

            if( bSet )
            {
                pElem->SetType( t2 );
            }
            pElem = pNew;
        }
        // consider index-access for UnoObjects
        // definitely we want this for VBA where properties are often
        // collections ( which need index access ), but lets only do
        // this if we actually have params following
        else if( bVBAEnabled && pElem->ISA(SbUnoProperty) && pElem->GetParameters() )
        {
            SbxVariableRef refTemp = pElem;

            // dissolve the notify while copying variable
            SbxVariable* pNew = new SbxVariable( *((SbxVariable*)pElem) );
            pElem->SetParameters( NULL );
            pElem = pNew;
        }
    }
    return CheckArray( pElem );
}

// for current scope (e. g. query from BASIC-IDE)
SbxBase* SbiRuntime::FindElementExtern( const OUString& rName )
{
    // don't expect pMeth to be != 0, as there are none set
    // in the RunInit yet

    SbxVariable* pElem = NULL;
    if( !pMod || rName.isEmpty() )
    {
        return NULL;
    }
    if( refLocals )
    {
        pElem = refLocals->Find( rName, SbxCLASS_DONTCARE );
    }
    if ( !pElem && pMeth )
    {
        // for statics, set the method's name in front
        OUString aMethName = pMeth->GetName();
        aMethName += ":";
        aMethName += rName;
        pElem = pMod->Find(aMethName, SbxCLASS_DONTCARE);
    }

    // search in parameter list
    if( !pElem && pMeth )
    {
        SbxInfo* pInfo = pMeth->GetInfo();
        if( pInfo && refParams )
        {
            sal_uInt16 nParamCount = refParams->Count();
            sal_uInt16 j = 1;
            const SbxParamInfo* pParam = pInfo->GetParam( j );
            while( pParam )
            {
                if( pParam->aName.equalsIgnoreAsciiCase( rName ) )
                {
                    if( j >= nParamCount )
                    {
                        // Parameter is missing
                        pElem = new SbxVariable( SbxSTRING );
                        pElem->PutString( OUString("<missing parameter>"));
                    }
                    else
                    {
                        pElem = refParams->Get( j );
                    }
                    break;
                }
                pParam = pInfo->GetParam( ++j );
            }
        }
    }

    // search in module
    if( !pElem )
    {
        bool bSave = rBasic.bNoRtl;
        rBasic.bNoRtl = true;
        pElem = pMod->Find( rName, SbxCLASS_DONTCARE );
        rBasic.bNoRtl = bSave;
    }
    return pElem;
}



void SbiRuntime::SetupArgs( SbxVariable* p, sal_uInt32 nOp1 )
{
    if( nOp1 & 0x8000 )
    {
        if( !refArgv )
        {
            StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
        }
        bool bHasNamed = false;
        sal_uInt16 i;
        sal_uInt16 nArgCount = refArgv->Count();
        for( i = 1 ; i < nArgCount ; i++ )
        {
            if( !refArgv->GetAlias(i).isEmpty() )
            {
                bHasNamed = true; break;
            }
        }
        if( bHasNamed )
        {
            SbxInfo* pInfo = p->GetInfo();
            if( !pInfo )
            {
                bool bError_ = true;

                SbUnoMethod* pUnoMethod = PTR_CAST(SbUnoMethod,p);
                SbUnoProperty* pUnoProperty = PTR_CAST(SbUnoProperty,p);
                if( pUnoMethod || pUnoProperty )
                {
                    SbUnoObject* pParentUnoObj = PTR_CAST( SbUnoObject,p->GetParent() );
                    if( pParentUnoObj )
                    {
                        Any aUnoAny = pParentUnoObj->getUnoAny();
                        Reference< XInvocation > xInvocation;
                        aUnoAny >>= xInvocation;
                        if( xInvocation.is() )  // TODO: if( xOLEAutomation.is() )
                        {
                            bError_ = false;

                            sal_uInt16 nCurPar = 1;
                            AutomationNamedArgsSbxArray* pArg =
                                new AutomationNamedArgsSbxArray( nArgCount );
                            OUString* pNames = pArg->getNames().getArray();
                            for( i = 1 ; i < nArgCount ; i++ )
                            {
                                SbxVariable* pVar = refArgv->Get( i );
                                const OUString& rName = refArgv->GetAlias( i );
                                if( !rName.isEmpty() )
                                {
                                    pNames[i] = rName;
                                }
                                pArg->Put( pVar, nCurPar++ );
                            }
                            refArgv = pArg;
                        }
                    }
                }
                else if( bVBAEnabled && p->GetType() == SbxOBJECT && (!p->ISA(SbxMethod) || !p->IsBroadcaster()) )
                {
                    // Check for default method with named parameters
                    SbxBaseRef pObj = (SbxBase*)p->GetObject();
                    if( pObj && pObj->ISA(SbUnoObject) )
                    {
                        SbUnoObject* pUnoObj = (SbUnoObject*)(SbxBase*)pObj;
                        Any aAny = pUnoObj->getUnoAny();

                        if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE )
                        {
                            Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();
                            Reference< XDefaultMethod > xDfltMethod( x, UNO_QUERY );

                            OUString sDefaultMethod;
                            if ( xDfltMethod.is() )
                            {
                                sDefaultMethod = xDfltMethod->getDefaultMethodName();
                            }
                            if ( !sDefaultMethod.isEmpty() )
                            {
                                SbxVariable* meth = pUnoObj->Find( sDefaultMethod, SbxCLASS_METHOD );
                                if( meth != NULL )
                                {
                                    pInfo = meth->GetInfo();
                                }
                                if( pInfo )
                                {
                                    bError_ = false;
                                }
                            }
                        }
                    }
                }
                if( bError_ )
                {
                    Error( SbERR_NO_NAMED_ARGS );
                }
            }
            else
            {
                sal_uInt16 nCurPar = 1;
                SbxArray* pArg = new SbxArray;
                for( i = 1 ; i < nArgCount ; i++ )
                {
                    SbxVariable* pVar = refArgv->Get( i );
                    const OUString& rName = refArgv->GetAlias( i );
                    if( !rName.isEmpty() )
                    {
                        // nCurPar is set to the found parameter
                        sal_uInt16 j = 1;
                        const SbxParamInfo* pParam = pInfo->GetParam( j );
                        while( pParam )
                        {
                            if( pParam->aName.equalsIgnoreAsciiCase( rName ) )
                            {
                                nCurPar = j;
                                break;
                            }
                            pParam = pInfo->GetParam( ++j );
                        }
                        if( !pParam )
                        {
                            Error( SbERR_NAMED_NOT_FOUND ); break;
                        }
                    }
                    pArg->Put( pVar, nCurPar++ );
                }
                refArgv = pArg;
            }
        }
        // own var as parameter 0
        refArgv->Put( p, 0 );
        p->SetParameters( refArgv );
        PopArgv();
    }
    else
    {
        p->SetParameters( NULL );
    }
}

// getting an array element

SbxVariable* SbiRuntime::CheckArray( SbxVariable* pElem )
{
    SbxArray* pPar;
    if( ( pElem->GetType() & SbxARRAY ) && (SbxVariable*)refRedim != pElem )
    {
        SbxBase* pElemObj = pElem->GetObject();
        SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
        pPar = pElem->GetParameters();
        if( pDimArray )
        {
            // parameters may be missing, if an array is
            // passed as an argument
            if( pPar )
                pElem = pDimArray->Get( pPar );
        }
        else
        {
            SbxArray* pArray = PTR_CAST(SbxArray,pElemObj);
            if( pArray )
            {
                if( !pPar )
                {
                    Error( SbERR_OUT_OF_RANGE );
                    pElem = new SbxVariable;
                }
                else
                {
                    pElem = pArray->Get( pPar->Get( 1 )->GetInteger() );
                }
            }
        }

        // #42940, set parameter 0 to NULL so that var doesn't contain itself
        if( pPar )
        {
            pPar->Put( NULL, 0 );
        }
    }
    // consider index-access for UnoObjects
    else if( pElem->GetType() == SbxOBJECT && !pElem->ISA(SbxMethod) && ( !bVBAEnabled || ( bVBAEnabled && !pElem->ISA(SbxProperty) ) ) )
    {
        pPar = pElem->GetParameters();
        if ( pPar )
        {
            // is it an uno-object?
            SbxBaseRef pObj = (SbxBase*)pElem->GetObject();
            if( pObj )
            {
                if( pObj->ISA(SbUnoObject) )
                {
                    SbUnoObject* pUnoObj = (SbUnoObject*)(SbxBase*)pObj;
                    Any aAny = pUnoObj->getUnoAny();

                    if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE )
                    {
                        Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();
                        Reference< XIndexAccess > xIndexAccess( x, UNO_QUERY );
                        if ( !bVBAEnabled )
                        {
                            if( xIndexAccess.is() )
                            {
                                sal_uInt32 nParamCount = (sal_uInt32)pPar->Count() - 1;
                                if( nParamCount != 1 )
                                {
                                    StarBASIC::Error( SbERR_BAD_ARGUMENT );
                                    return pElem;
                                }

                                // get index
                                sal_Int32 nIndex = pPar->Get( 1 )->GetLong();
                                Reference< XInterface > xRet;
                                try
                                {
                                    Any aAny2 = xIndexAccess->getByIndex( nIndex );
                                    TypeClass eType = aAny2.getValueType().getTypeClass();
                                    if( eType == TypeClass_INTERFACE )
                                    {
                                        xRet = *(Reference< XInterface >*)aAny2.getValue();
                                    }
                                }
                                catch (const IndexOutOfBoundsException&)
                                {
                                    // usually expect converting problem
                                    StarBASIC::Error( SbERR_OUT_OF_RANGE );
                                }

                                // #57847 always create a new variable, else error
                                // due to PutObject(NULL) at ReadOnly-properties
                                pElem = new SbxVariable( SbxVARIANT );
                                if( xRet.is() )
                                {
                                    aAny <<= xRet;

                                    // #67173 don't specify a name so that the real class name is entered
                                    OUString aName;
                                    SbxObjectRef xWrapper = (SbxObject*)new SbUnoObject( aName, aAny );
                                    pElem->PutObject( xWrapper );
                                }
                                else
                                {
                                    pElem->PutObject( NULL );
                                }
                            }
                        }
                        else
                        {
                            // check if there isn't a default member between the current variable
                            // and the params, e.g.
                            //   Dim rst1 As New ADODB.Recordset
                            //      "
                            //   val = rst1("FirstName")
                            // has the default 'Fields' member between rst1 and '("FirstName")'
                            SbxVariable* pDflt = getDefaultProp( pElem );
                            if ( pDflt )
                            {
                                pDflt->Broadcast( SBX_HINT_DATAWANTED );
                                SbxBaseRef pDfltObj = (SbxBase*)pDflt->GetObject();
                                if( pDfltObj )
                                {
                                    if( pDfltObj->ISA(SbUnoObject) )
                                    {
                                        pUnoObj = (SbUnoObject*)(SbxBase*)pDfltObj;
                                        Any aUnoAny = pUnoObj->getUnoAny();

                                        if( aUnoAny.getValueType().getTypeClass() == TypeClass_INTERFACE )
                                            x = *(Reference< XInterface >*)aUnoAny.getValue();
                                        pElem = pDflt;
                                    }
                                }
                            }
                            OUString sDefaultMethod;

                            Reference< XDefaultMethod > xDfltMethod( x, UNO_QUERY );

                            if ( xDfltMethod.is() )
                            {
                                sDefaultMethod = xDfltMethod->getDefaultMethodName();
                            }
                            else if( xIndexAccess.is() )
                            {
                                sDefaultMethod = OUString( "getByIndex" );
                            }
                            if ( !sDefaultMethod.isEmpty() )
                            {
                                SbxVariable* meth = pUnoObj->Find( sDefaultMethod, SbxCLASS_METHOD );
                                SbxVariableRef refTemp = meth;
                                if ( refTemp )
                                {
                                    meth->SetParameters( pPar );
                                    SbxVariable* pNew = new SbxMethod( *(SbxMethod*)meth );
                                    pElem = pNew;
                                }
                            }
                        }
                    }

                    // #42940, set parameter 0 to NULL so that var doesn't contain itself
                    pPar->Put( NULL, 0 );
                }
                else if( pObj->ISA(BasicCollection) )
                {
                    BasicCollection* pCol = (BasicCollection*)(SbxBase*)pObj;
                    pElem = new SbxVariable( SbxVARIANT );
                    pPar->Put( pElem, 0 );
                    pCol->CollItem( pPar );
                }
            }
            else if( bVBAEnabled )  // !pObj
            {
                SbxArray* pParam = pElem->GetParameters();
                if( pParam != NULL && !pElem->IsSet( SBX_VAR_TO_DIM ) )
                {
                    Error( SbERR_NO_OBJECT );
                }
            }
        }
    }

    return pElem;
}

// loading an element from the runtime-library (+StringID+type)

void SbiRuntime::StepRTL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    PushVar( FindElement( rBasic.pRtl, nOp1, nOp2, SbERR_PROC_UNDEFINED, false ) );
}

void SbiRuntime::StepFIND_Impl( SbxObject* pObj, sal_uInt32 nOp1, sal_uInt32 nOp2,
                                SbError nNotFound, bool bLocal, bool bStatic )
{
    if( !refLocals )
    {
        refLocals = new SbxArray;
    }
    PushVar( FindElement( pObj, nOp1, nOp2, nNotFound, bLocal, bStatic ) );
}
// loading a local/global variable (+StringID+type)

void SbiRuntime::StepFIND( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, true );
}

// Search inside a class module (CM) to enable global search in time
void SbiRuntime::StepFIND_CM( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{

    SbClassModuleObject* pClassModuleObject = PTR_CAST(SbClassModuleObject,pMod);
    if( pClassModuleObject )
    {
        pMod->SetFlag( SBX_GBLSEARCH );
    }
    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, true );

    if( pClassModuleObject )
    {
        pMod->ResetFlag( SBX_GBLSEARCH );
    }
}

void SbiRuntime::StepFIND_STATIC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, true, true );
}

// loading an object-element (+StringID+type)
// the object lies on TOS

void SbiRuntime::StepELEM( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    SbxVariableRef pObjVar = PopVar();

    SbxObject* pObj = PTR_CAST(SbxObject,(SbxVariable*) pObjVar);
    if( !pObj )
    {
        SbxBase* pObjVarObj = pObjVar->GetObject();
        pObj = PTR_CAST(SbxObject,pObjVarObj);
    }

    // #56368 save reference at StepElem, otherwise objects could
    // lose their reference too early in qualification chains like
    // ActiveComponent.Selection(0).Text
    // #74254 now per list
    if( pObj )
    {
        SaveRef( (SbxVariable*)pObj );
    }
    PushVar( FindElement( pObj, nOp1, nOp2, SbERR_NO_METHOD, false ) );
}

// loading a parameter (+offset+type)
// If the data type is wrong, create a copy.
// The data type SbxEMPTY shows that no parameters are given.
// Get( 0 ) may be EMPTY

void SbiRuntime::StepPARAM( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    sal_uInt16 i = static_cast<sal_uInt16>( nOp1 & 0x7FFF );
    SbxDataType t = (SbxDataType) nOp2;
    SbxVariable* p;

    // #57915 solve missing in a cleaner way
    sal_uInt16 nParamCount = refParams->Count();
    if( i >= nParamCount )
    {
        sal_Int16 iLoop = i;
        while( iLoop >= nParamCount )
        {
            p = new SbxVariable();

            if( SbiRuntime::isVBAEnabled() &&
                (t == SbxOBJECT || t == SbxSTRING) )
            {
                if( t == SbxOBJECT )
                {
                    p->PutObject( NULL );
                }
                else
                {
                    p->PutString( OUString() );
                }
            }
            else
            {
                p->PutErr( 448 );       // like in VB: Error-Code 448 (SbERR_NAMED_NOT_FOUND)
            }
            refParams->Put( p, iLoop );
            iLoop--;
        }
    }
    p = refParams->Get( i );

    if( p->GetType() == SbxERROR && ( i ) )
    {
        // if there's a parameter missing, it can be OPTIONAL
        bool bOpt = false;
        if( pMeth )
        {
            SbxInfo* pInfo = pMeth->GetInfo();
            if ( pInfo )
            {
                const SbxParamInfo* pParam = pInfo->GetParam( i );
                if( pParam && ( (pParam->nFlags & SBX_OPTIONAL) != 0 ) )
                {
                    // Default value?
                    sal_uInt16 nDefaultId = (sal_uInt16)(pParam->nUserData & 0x0ffff);
                    if( nDefaultId > 0 )
                    {
                        OUString aDefaultStr = pImg->GetString( nDefaultId );
                        p = new SbxVariable();
                        p->PutString( aDefaultStr );
                        refParams->Put( p, i );
                    }
                    bOpt = true;
                }
            }
        }
        if( !bOpt )
        {
            Error( SbERR_NOT_OPTIONAL );
        }
    }
    else if( t != SbxVARIANT && (SbxDataType)(p->GetType() & 0x0FFF ) != t )
    {
        SbxVariable* q = new SbxVariable( t );
        SaveRef( q );
        *q = *p;
        p = q;
        if ( i )
        {
            refParams->Put( p, i );
        }
    }
    SetupArgs( p, nOp1 );
    PushVar( CheckArray( p ) );
}

// Case-Test (+True-Target+Test-Opcode)

void SbiRuntime::StepCASEIS( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( !refCaseStk || !refCaseStk->Count() )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    }
    else
    {
        SbxVariableRef xComp = PopVar();
        SbxVariableRef xCase = refCaseStk->Get( refCaseStk->Count() - 1 );
        if( xCase->Compare( (SbxOperator) nOp2, *xComp ) )
        {
            StepJUMP( nOp1 );
        }
    }
}

// call of a DLL-procedure (+StringID+type)
// the StringID's MSB shows that Argv is occupied

void SbiRuntime::StepCALL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    OUString aName = pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
    {
        pArgs = refArgv;
    }
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, false );
    aLibName = OUString();
    if( nOp1 & 0x8000 )
    {
        PopArgv();
    }
}

// call of a DLL-procedure after CDecl (+StringID+type)

void SbiRuntime::StepCALLC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    OUString aName = pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
    {
        pArgs = refArgv;
    }
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, true );
    aLibName = OUString();
    if( nOp1 & 0x8000 )
    {
        PopArgv();
    }
}


// beginning of a statement (+Line+Col)

void SbiRuntime::StepSTMNT( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    // If the Expr-Stack at the beginning of a statement constains a variable,
    // some fool has called X as a function, although it's a variable!
    bool bFatalExpr = false;
    OUString sUnknownMethodName;
    if( nExprLvl > 1 )
    {
        bFatalExpr = true;
    }
    else if( nExprLvl )
    {
        SbxVariable* p = refExprStk->Get( 0 );
        if( p->GetRefCount() > 1 &&
            refLocals.Is() && refLocals->Find( p->GetName(), p->GetClass() ) )
        {
            sUnknownMethodName = p->GetName();
            bFatalExpr = true;
        }
    }

    ClearExprStack();

    ClearRefs();

    // We have to cancel hard here because line and column
    // would be wrong later otherwise!
    if( bFatalExpr)
    {
        StarBASIC::FatalError( SbERR_NO_METHOD, sUnknownMethodName );
        return;
    }
    pStmnt = pCode - 9;
    sal_uInt16 nOld = nLine;
    nLine = static_cast<short>( nOp1 );

    // #29955 & 0xFF, to filter out for-loop-level
    nCol1 = static_cast<short>( nOp2 & 0xFF );

    // find the next STMNT-command to set the final column
    // of this statement

    nCol2 = 0xffff;
    sal_uInt16 n1, n2;
    const sal_uInt8* p = pMod->FindNextStmnt( pCode, n1, n2 );
    if( p )
    {
        if( n1 == nOp1 )
        {
            // #29955 & 0xFF, to filter out for-loop-level
            nCol2 = (n2 & 0xFF) - 1;
        }
    }

    // #29955 correct for-loop-level, #67452 NOT in the error-handler
    if( !bInError )
    {
        // (there's a difference here in case of a jump out of a loop)
        sal_uInt16 nExspectedForLevel = static_cast<sal_uInt16>( nOp2 / 0x100 );
        if( pGosubStk )
        {
            nExspectedForLevel = nExspectedForLevel + pGosubStk->nStartForLvl;
        }

        // if the actual for-level is too small it'd jump out
        // of a loop -> corrected
        while( nForLvl > nExspectedForLevel )
        {
            PopFor();
        }
    }

    // 16.10.96: #31460 new concept for StepInto/Over/Out
    // see explanation at _ImplGetBreakCallLevel
    if( pInst->nCallLvl <= pInst->nBreakCallLvl )
    {
        StarBASIC* pStepBasic = GetCurrentBasic( &rBasic );
        sal_uInt16 nNewFlags = pStepBasic->StepPoint( nLine, nCol1, nCol2 );

        pInst->CalcBreakCallLevel( nNewFlags );
    }

    // break points only at STMNT-commands in a new line!
    else if( ( nOp1 != nOld )
        && ( nFlags & SbDEBUG_BREAK )
        && pMod->IsBP( static_cast<sal_uInt16>( nOp1 ) ) )
    {
        StarBASIC* pBreakBasic = GetCurrentBasic( &rBasic );
        sal_uInt16 nNewFlags = pBreakBasic->BreakPoint( nLine, nCol1, nCol2 );

        pInst->CalcBreakCallLevel( nNewFlags );
    }
}

// (+SvStreamFlags+Flags)
// Stack: block length
//        channel number
//        file name

void SbiRuntime::StepOPEN( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    SbxVariableRef pName = PopVar();
    SbxVariableRef pChan = PopVar();
    SbxVariableRef pLen  = PopVar();
    short nBlkLen = pLen->GetInteger();
    short nChan   = pChan->GetInteger();
    OString aName(OUStringToOString(pName->GetOUString(), osl_getThreadTextEncoding()));
    pIosys->Open( nChan, aName, static_cast<short>( nOp1 ),
                  static_cast<short>( nOp2 ), nBlkLen );
    Error( pIosys->GetError() );
}

// create object (+StringID+StringID)

void SbiRuntime::StepCREATE( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    OUString aClass( pImg->GetString( static_cast<short>( nOp2 ) ) );
    SbxObject *pObj = SbxBase::CreateObject( aClass );
    if( !pObj )
    {
        Error( SbERR_INVALID_OBJECT );
    }
    else
    {
        OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
        pObj->SetName( aName );
        // the object must be able to call the BASIC
        pObj->SetParent( &rBasic );
        SbxVariable* pNew = new SbxVariable;
        pNew->PutObject( pObj );
        PushVar( pNew );
    }
}

void SbiRuntime::StepDCREATE( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepDCREATE_IMPL( nOp1, nOp2 );
}

void SbiRuntime::StepDCREATE_REDIMP( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepDCREATE_IMPL( nOp1, nOp2 );
}


// Helper function for StepDCREATE_IMPL / bRedimp = true
void implCopyDimArray_DCREATE( SbxDimArray* pNewArray, SbxDimArray* pOldArray, short nMaxDimIndex,
    short nActualDim, sal_Int32* pActualIndices, sal_Int32* pLowerBounds, sal_Int32* pUpperBounds )
{
    sal_Int32& ri = pActualIndices[nActualDim];
    for( ri = pLowerBounds[nActualDim] ; ri <= pUpperBounds[nActualDim] ; ri++ )
    {
        if( nActualDim < nMaxDimIndex )
        {
            implCopyDimArray_DCREATE( pNewArray, pOldArray, nMaxDimIndex, nActualDim + 1,
                pActualIndices, pLowerBounds, pUpperBounds );
        }
        else
        {
            SbxVariable* pSource = pOldArray->Get32( pActualIndices );
            pNewArray->Put32( pSource, pActualIndices );
        }
    }
}

// #56204 create object array (+StringID+StringID), DCREATE == Dim-Create
void SbiRuntime::StepDCREATE_IMPL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    SbxVariableRef refVar = PopVar();

    DimImpl( refVar );

    // fill the array with instances of the requested class
    SbxBaseRef xObj = (SbxBase*)refVar->GetObject();
    if( !xObj )
    {
        StarBASIC::Error( SbERR_INVALID_OBJECT );
        return;
    }

    SbxDimArray* pArray = 0;
    if( xObj->ISA(SbxDimArray) )
    {
        SbxBase* pObj = (SbxBase*)xObj;
        pArray = (SbxDimArray*)pObj;

        short nDims = pArray->GetDims();
        sal_Int32 nTotalSize = 0;

        // must be a one-dimensional array
        sal_Int32 nLower, nUpper, nSize;
        sal_Int32 i;
        for( i = 0 ; i < nDims ; i++ )
        {
            pArray->GetDim32( i+1, nLower, nUpper );
            nSize = nUpper - nLower + 1;
            if( i == 0 )
            {
                nTotalSize = nSize;
            }
            else
            {
                nTotalSize *= nSize;
            }
        }

        // create objects and insert them into the array
        OUString aClass( pImg->GetString( static_cast<short>( nOp2 ) ) );
        for( i = 0 ; i < nTotalSize ; i++ )
        {
            SbxObject *pClassObj = SbxBase::CreateObject( aClass );
            if( !pClassObj )
            {
                Error( SbERR_INVALID_OBJECT );
                break;
            }
            else
            {
                OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
                pClassObj->SetName( aName );
                // the object must be able to call the basic
                pClassObj->SetParent( &rBasic );
                pArray->SbxArray::Put32( pClassObj, i );
            }
        }
    }

    SbxDimArray* pOldArray = (SbxDimArray*)(SbxArray*)refRedimpArray;
    if( pArray && pOldArray )
    {
        short nDimsNew = pArray->GetDims();
        short nDimsOld = pOldArray->GetDims();
        short nDims = nDimsNew;
        bool bRangeError = false;

        // Store dims to use them for copying later
        sal_Int32* pLowerBounds = new sal_Int32[nDims];
        sal_Int32* pUpperBounds = new sal_Int32[nDims];
        sal_Int32* pActualIndices = new sal_Int32[nDims];
        if( nDimsOld != nDimsNew )
        {
            bRangeError = true;
        }
        else
        {
            // Compare bounds
            for( short i = 1 ; i <= nDims ; i++ )
            {
                sal_Int32 lBoundNew, uBoundNew;
                sal_Int32 lBoundOld, uBoundOld;
                pArray->GetDim32( i, lBoundNew, uBoundNew );
                pOldArray->GetDim32( i, lBoundOld, uBoundOld );

                lBoundNew = std::max( lBoundNew, lBoundOld );
                uBoundNew = std::min( uBoundNew, uBoundOld );
                short j = i - 1;
                pActualIndices[j] = pLowerBounds[j] = lBoundNew;
                pUpperBounds[j] = uBoundNew;
            }
        }

        if( bRangeError )
        {
            StarBASIC::Error( SbERR_OUT_OF_RANGE );
        }
        else
        {
            // Copy data from old array by going recursively through all dimensions
            // (It would be faster to work on the flat internal data array of an
            // SbyArray but this solution is clearer and easier)
            implCopyDimArray_DCREATE( pArray, pOldArray, nDims - 1,
                0, pActualIndices, pLowerBounds, pUpperBounds );
        }
        delete [] pUpperBounds;
        delete [] pLowerBounds;
        delete [] pActualIndices;
        refRedimpArray = NULL;
    }
}

// create object from user-type  (+StringID+StringID)

SbxObject* createUserTypeImpl( const OUString& rClassName );  // sb.cxx

void SbiRuntime::StepTCREATE( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    OUString aClass( pImg->GetString( static_cast<short>( nOp2 ) ) );

    SbxObject* pCopyObj = createUserTypeImpl( aClass );
    if( pCopyObj )
    {
        pCopyObj->SetName( aName );
    }
    SbxVariable* pNew = new SbxVariable;
    pNew->PutObject( pCopyObj );
    pNew->SetDeclareClassName( aClass );
    PushVar( pNew );
}

void SbiRuntime::implHandleSbxFlags( SbxVariable* pVar, SbxDataType t, sal_uInt32 nOp2 )
{
    bool bWithEvents = ((t & 0xff) == SbxOBJECT && (nOp2 & SBX_TYPE_WITH_EVENTS_FLAG) != 0);
    if( bWithEvents )
    {
        pVar->SetFlag( SBX_WITH_EVENTS );
    }
    bool bDimAsNew = ((nOp2 & SBX_TYPE_DIM_AS_NEW_FLAG) != 0);
    if( bDimAsNew )
    {
        pVar->SetFlag( SBX_DIM_AS_NEW );
    }
    bool bFixedString = ((t & 0xff) == SbxSTRING && (nOp2 & SBX_FIXED_LEN_STRING_FLAG) != 0);
    if( bFixedString )
    {
        sal_uInt16 nCount = static_cast<sal_uInt16>( nOp2 >> 17 );      // len = all bits above 0x10000
        OUStringBuffer aBuf;
        comphelper::string::padToLength(aBuf, nCount, 0);
        pVar->PutString(aBuf.makeStringAndClear());
    }

    bool bVarToDim = ((nOp2 & SBX_TYPE_VAR_TO_DIM_FLAG) != 0);
    if( bVarToDim )
    {
        pVar->SetFlag( SBX_VAR_TO_DIM );
    }
}

// establishing a local variable (+StringID+type)

void SbiRuntime::StepLOCAL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( !refLocals.Is() )
    {
        refLocals = new SbxArray;
    }
    OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    if( refLocals->Find( aName, SbxCLASS_DONTCARE ) == NULL )
    {
        SbxDataType t = (SbxDataType)(nOp2 & 0xffff);
        SbxVariable* p = new SbxVariable( t );
        p->SetName( aName );
        implHandleSbxFlags( p, t, nOp2 );
        refLocals->Put( p, refLocals->Count() );
    }
}

// establishing a module-global variable (+StringID+type)

void SbiRuntime::StepPUBLIC_Impl( sal_uInt32 nOp1, sal_uInt32 nOp2, bool bUsedForClassModule )
{
    OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType)(SbxDataType)(nOp2 & 0xffff);;
    sal_Bool bFlag = pMod->IsSet( SBX_NO_MODIFY );
    pMod->SetFlag( SBX_NO_MODIFY );
    SbxVariableRef p = pMod->Find( aName, SbxCLASS_PROPERTY );
    if( p.Is() )
    {
        pMod->Remove (p);
    }
    SbProperty* pProp = pMod->GetProperty( aName, t );
    if( !bUsedForClassModule )
    {
        pProp->SetFlag( SBX_PRIVATE );
    }
    if( !bFlag )
    {
        pMod->ResetFlag( SBX_NO_MODIFY );
    }
    if( pProp )
    {
        pProp->SetFlag( SBX_DONTSTORE );
        // from 2.7.1996: HACK because of 'reference can't be saved'
        pProp->SetFlag( SBX_NO_MODIFY);

        implHandleSbxFlags( pProp, t, nOp2 );
    }
}

void SbiRuntime::StepPUBLIC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepPUBLIC_Impl( nOp1, nOp2, false );
}

void SbiRuntime::StepPUBLIC_P( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    // Creates module variable that isn't reinitialised when
    // between invocations ( for VBASupport & document basic only )
    if( pMod->pImage->bFirstInit )
    {
        bool bUsedForClassModule = pImg->GetFlag( SBIMG_CLASSMODULE );
        StepPUBLIC_Impl( nOp1, nOp2, bUsedForClassModule );
    }
}

// establishing a global variable (+StringID+type)

void SbiRuntime::StepGLOBAL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( pImg->GetFlag( SBIMG_CLASSMODULE ) )
    {
        StepPUBLIC_Impl( nOp1, nOp2, true );
    }
    OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType)(nOp2 & 0xffff);

    // Store module scope variables at module scope
    // in non vba mode these are stored at the library level :/
    // not sure if this really should not be enabled for ALL basic
    SbxObject* pStorage = &rBasic;
    if ( SbiRuntime::isVBAEnabled() )
    {
        pStorage = pMod;
        pMod->AddVarName( aName );
    }

    sal_Bool bFlag = pStorage->IsSet( SBX_NO_MODIFY );
    rBasic.SetFlag( SBX_NO_MODIFY );
    SbxVariableRef p = pStorage->Find( aName, SbxCLASS_PROPERTY );
    if( p.Is() )
    {
        pStorage->Remove (p);
    }
    p = pStorage->Make( aName, SbxCLASS_PROPERTY, t );
    if( !bFlag )
    {
        pStorage->ResetFlag( SBX_NO_MODIFY );
    }
    if( p )
    {
        p->SetFlag( SBX_DONTSTORE );
        // from 2.7.1996: HACK because of 'reference can't be saved'
        p->SetFlag( SBX_NO_MODIFY);
    }
}


// Creates global variable that isn't reinitialised when
// basic is restarted, P=PERSIST (+StringID+Typ)

void SbiRuntime::StepGLOBAL_P( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( pMod->pImage->bFirstInit )
    {
        StepGLOBAL( nOp1, nOp2 );
    }
}


// Searches for global variable, behavior depends on the fact
// if the variable is initialised for the first time

void SbiRuntime::StepFIND_G( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( pMod->pImage->bFirstInit )
    {
        // Behave like always during first init
        StepFIND( nOp1, nOp2 );
    }
    else
    {
        // Return dummy variable
        SbxDataType t = (SbxDataType) nOp2;
        OUString aName( pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) ) );

        SbxVariable* pDummyVar = new SbxVariable( t );
        pDummyVar->SetName( aName );
        PushVar( pDummyVar );
    }
}


SbxVariable* SbiRuntime::StepSTATIC_Impl( OUString& aName, SbxDataType& t )
{
    SbxVariable* p = NULL;
    if ( pMeth )
    {
        SbxArray* pStatics = pMeth->GetStatics();
        if( pStatics && ( pStatics->Find( aName, SbxCLASS_DONTCARE ) == NULL ) )
        {
            p = new SbxVariable( t );
            if( t != SbxVARIANT )
            {
                p->SetFlag( SBX_FIXED );
            }
            p->SetName( aName );
            pStatics->Put( p, pStatics->Count() );
        }
    }
    return p;
}
// establishing a static variable (+StringID+type)
void SbiRuntime::StepSTATIC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType) nOp2;
    StepSTATIC_Impl( aName, t );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
