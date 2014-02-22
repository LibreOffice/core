/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <vcl/settings.hxx>

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
#include <basic/codecompletecache.hxx>

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


struct SbiArgvStack {                   
    SbiArgvStack*  pNext;               
    SbxArrayRef    refArgv;             
    short nArgc;                        
};

SbiRuntime::pStep0 SbiRuntime::aStep0[] = { 
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
    
    &SbiRuntime::StepARGC,      
    &SbiRuntime::StepARGV,      
    &SbiRuntime::StepINPUT,     
    &SbiRuntime::StepLINPUT,        
    &SbiRuntime::StepGET,        
    &SbiRuntime::StepSET,        
    &SbiRuntime::StepPUT,       
    &SbiRuntime::StepPUTC,      
    &SbiRuntime::StepDIM,       
    &SbiRuntime::StepREDIM,         
    &SbiRuntime::StepREDIMP,        
    &SbiRuntime::StepERASE,         
    
    &SbiRuntime::StepSTOP,          
    &SbiRuntime::StepINITFOR,   
    &SbiRuntime::StepNEXT,      
    &SbiRuntime::StepCASE,      
    &SbiRuntime::StepENDCASE,   
    &SbiRuntime::StepSTDERROR,      
    &SbiRuntime::StepNOERROR,   
    &SbiRuntime::StepLEAVE,     
    
    &SbiRuntime::StepCHANNEL,   
    &SbiRuntime::StepPRINT,     
    &SbiRuntime::StepPRINTF,        
    &SbiRuntime::StepWRITE,     
    &SbiRuntime::StepRENAME,        
    &SbiRuntime::StepPROMPT,        
    &SbiRuntime::StepRESTART,   
    &SbiRuntime::StepCHANNEL0,  
    &SbiRuntime::StepEMPTY,     
    &SbiRuntime::StepERROR,     
    &SbiRuntime::StepLSET,      
    &SbiRuntime::StepRSET,      
    &SbiRuntime::StepREDIMP_ERASE,
    &SbiRuntime::StepINITFOREACH,
    &SbiRuntime::StepVBASET,
    &SbiRuntime::StepERASE_CLEAR,
    &SbiRuntime::StepARRAYACCESS,
    &SbiRuntime::StepBYVAL,     
};

SbiRuntime::pStep1 SbiRuntime::aStep1[] = { 
    &SbiRuntime::StepLOADNC,        
    &SbiRuntime::StepLOADSC,        
    &SbiRuntime::StepLOADI,     
    &SbiRuntime::StepARGN,      
    &SbiRuntime::StepPAD,       
    
    &SbiRuntime::StepJUMP,      
    &SbiRuntime::StepJUMPT,     
    &SbiRuntime::StepJUMPF,     
    &SbiRuntime::StepONJUMP,        
    &SbiRuntime::StepGOSUB,     
    &SbiRuntime::StepRETURN,        
    &SbiRuntime::StepTESTFOR,   
    &SbiRuntime::StepCASETO,        
    &SbiRuntime::StepERRHDL,        
    &SbiRuntime::StepRESUME,        
    
    &SbiRuntime::StepCLOSE,     
    &SbiRuntime::StepPRCHAR,        
    
    &SbiRuntime::StepSETCLASS,  
    &SbiRuntime::StepTESTCLASS, 
    &SbiRuntime::StepLIB,       
    &SbiRuntime::StepBASED,     
    &SbiRuntime::StepARGTYP,        
    &SbiRuntime::StepVBASETCLASS,
};

SbiRuntime::pStep2 SbiRuntime::aStep2[] = {
    &SbiRuntime::StepRTL,       
    &SbiRuntime::StepFIND,      
    &SbiRuntime::StepELEM,          
    &SbiRuntime::StepPARAM,     
    
    &SbiRuntime::StepCALL,      
    &SbiRuntime::StepCALLC,     
    &SbiRuntime::StepCASEIS,        
    
    &SbiRuntime::StepSTMNT,         
    
    &SbiRuntime::StepOPEN,          
    
    &SbiRuntime::StepLOCAL,     
    &SbiRuntime::StepPUBLIC,        
    &SbiRuntime::StepGLOBAL,        
    &SbiRuntime::StepCREATE,        
    &SbiRuntime::StepSTATIC,     
    &SbiRuntime::StepTCREATE,    
    &SbiRuntime::StepDCREATE,    
    &SbiRuntime::StepGLOBAL_P,   
                                 
    &SbiRuntime::StepFIND_G,        
    &SbiRuntime::StepDCREATE_REDIMP, 
    &SbiRuntime::StepFIND_CM,    
    &SbiRuntime::StepPUBLIC_P,    
    &SbiRuntime::StepFIND_STATIC,    
};




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












void SbiInstance::CalcBreakCallLevel( sal_uInt16 nFlags )
{

    nFlags &= ~((sal_uInt16)SbDEBUG_BREAK);

    sal_uInt16 nRet;
    switch( nFlags )
    {
    case SbDEBUG_STEPINTO:
        nRet = nCallLvl + 1;    
        break;
    case SbDEBUG_STEPOVER | SbDEBUG_STEPINTO:
        nRet = nCallLvl;        
        break;
    case SbDEBUG_STEPOUT:
        nRet = nCallLvl - 1;    
        break;
    case SbDEBUG_CONTINUE:
        
    default:
        nRet = 0;               
    }
    nBreakCallLvl = nRet;           
}

SbiInstance::SbiInstance( StarBASIC* p )
    : pIosys(new SbiIoSystem)
    , pDdeCtrl(new SbiDdeControl)
    , pDllMgr(0) 
    , pBasic(p)
    , pNumberFormatter(0)
    , meFormatterLangType(LANGUAGE_DONTKNOW)
    , meFormatterDateFormat(YMD)
    , nStdDateIdx(0)
    , nStdTimeIdx(0)
    , nStdDateTimeIdx(0)
    , nErr(0)
    , nErl(0)
    , bReschedule(true)
    , bCompatibility(false)
    , pRun(NULL)
    , pNext(NULL)
    , nCallLvl(0)
    , nBreakCallLvl(0)
{
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

    
    

    
    
    
    

    OUString aDateStr;
    switch( eDate )
    {
        default:
        case MDY: aDateStr = "MM/DD/YYYY"; break;
        case DMY: aDateStr = "DD/MM/YYYY"; break;
        case YMD: aDateStr = "YYYY/MM/DD"; break;
    }
    OUString aStr( aDateStr );      
    rpNumberFormatter->PutandConvertEntry( aStr, nCheckPos, nType,
        rnStdDateIdx, LANGUAGE_ENGLISH_US, eLangType );
    nCheckPos = 0;
    OUString aStrHHMMSS(" HH:MM:SS");
    aDateStr += aStrHHMMSS;
    aStr = aDateStr;
    rpNumberFormatter->PutandConvertEntry( aStr, nCheckPos, nType,
        rnStdDateTimeIdx, LANGUAGE_ENGLISH_US, eLangType );
}




void SbiInstance::Stop()
{
    for( SbiRuntime* p = pRun; p; p = p->pNext )
    {
        p->Stop();
    }
}


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
            n = nVBNumber; 
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
        n = nVBNumber; 
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





void SbiRuntime::SetParameters( SbxArray* pParams )
{
    refParams = new SbxArray;
    
    refParams->Put( pMeth, 0 );

    SbxInfo* pInfo = pMeth ? pMeth->GetInfo() : NULL;
    sal_uInt16 nParamCount = pParams ? pParams->Count() : 1;
    if( nParamCount > 1 )
    {
        for( sal_uInt16 i = 1 ; i < nParamCount ; i++ )
        {
            const SbxParamInfo* p = pInfo ? pInfo->GetParam( i ) : NULL;

            
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

                
                pInfo = NULL;
                break;
            }

            SbxVariable* v = pParams->Get( i );
            
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

    
    if( pInfo )
    {
        
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




bool SbiRuntime::Step()
{
    if( bRun )
    {
        
        if( !( ++nOps & 0xF ) && pInst->IsReschedule() )
        {
            sal_uInt32 nTime = osl_getGlobalTimer();
            if (nTime - m_nLastTime > 5 ) 
            {
                Application::Reschedule();
                m_nLastTime = nTime;
            }
        }

        
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

        
        
        

        if( nError )
        {
            SbxBase::ResetError();
        }

        
        
        if( nError && bRun )
        {
            SbError err = nError;
            ClearExprStack();
            nError = 0;
            pInst->nErr = err;
            pInst->nErl = nLine;
            pErrCode    = pCode;
            pErrStmnt   = pStmnt;
            
            
            
            bool bLetParentHandleThis = false;

            
            if ( !bInError )
            {
                bInError = true;

                if( !bError )           
                {
                    StepRESUME( 1 );
                }
                else if( pError )       
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
                pError = NULL; 
            }
            if ( bLetParentHandleThis )
            {
                
                

                
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
        
        
        if ( pInst->pRun == this )
        {
            pInst->Error( _errCode, _details );
            
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
    
    
    
    
    
    if ( rMsg.isEmpty() )
    {
        
#ifdef DBG_UTIL
        SbError nTmp = StarBASIC::GetSfxFromVBError( (sal_uInt16)nError );
        SAL_WARN_IF( nTmp == 0, "basic", "No VB error!" );
#endif

        StarBASIC::MakeErrorText( nError, rMsg );
        rMsg = StarBASIC::GetErrorText();
        if ( rMsg.isEmpty() ) 
        {
            rMsg = "Internal Object Error:";
        }
    }
    
    sal_uInt16 nVBErrorCode = StarBASIC::GetVBErrorCode( nError );
    sal_Int32 nVBAErrorNumber = ( nVBErrorCode == 0 ) ? nError : nVBErrorCode;
    return nVBAErrorNumber;
}



SbMethod* SbiRuntime::GetCaller()
{
    return pMeth;
}






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
    
    if( xVar->IsA( TYPE(SbxMethod) ) )
    {
        xVar->SetParameters(0);
    }
    return xVar;
}

bool SbiRuntime::ClearExprStack()
{
    
    while ( nExprLvl )
    {
        PopVar();
    }
    refExprStk->Clear();
    return false;
}




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



//


void SbiRuntime::DllCall
    ( const OUString& aFuncName,
      const OUString& aDLLName,
      SbxArray* pArgs,          
      SbxDataType eResType,     
      bool bCDecl )         
{
    
    if( needSecurityRestrictions() )
    {
        StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
        return;
    }

    

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
    bool bRes = aSearch.SearchForward(value, &nStart, &nEnd);
    pRes->PutBool( bRes );

    PushVar( pRes );
}



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



void SbiRuntime::StepGET()
{
    SbxVariable* p = GetTOS();
    p->Broadcast( SBX_HINT_DATAWANTED );
}


inline bool checkUnoStructCopy( bool bVBA, SbxVariableRef& refVal, SbxVariableRef& refVar )
{
    SbxDataType eVarType = refVar->GetType();
    SbxDataType eValType = refVal->GetType();

    if ( !( !bVBA|| ( bVBA && refVar->GetType() != SbxEMPTY ) ) || !refVar->CanWrite() )
        return false;

    if ( eValType != SbxOBJECT )
        return false;
    
    
    if( eVarType != SbxOBJECT )
    {
        if ( refVar->IsFixed() )
            return false;
    }
    
    else if( refVar->ISA(SbProcedureProperty) )
        return false;

    SbxObjectRef xValObj = (SbxObject*)refVal->GetObject();
    if( !xValObj.Is() || xValObj->ISA(SbUnoAnyObject) )
        return false;

    SbUnoObject* pUnoVal =  PTR_CAST(SbUnoObject,(SbxObject*)xValObj);
    SbUnoStructRefObject* pUnoStructVal = PTR_CAST(SbUnoStructRefObject,(SbxObject*)xValObj);
    Any aAny;
    
    
    if ( pUnoVal || pUnoStructVal )
        aAny = pUnoVal ? pUnoVal->getUnoAny() : pUnoStructVal->getUnoAny();
    else
        return false;
    if (  aAny.getValueType().getTypeClass() == TypeClass_STRUCT )
    {
        refVar->SetType( SbxOBJECT );
        SbxError eOldErr = refVar->GetError();
        
        
        
        
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
            
            pNewUnoObj->SetClassName( sClassName );
            refVar->PutObject( pNewUnoObj );
        }
        return true;
    }
    return false;
}




void SbiRuntime::StepPUT()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    
    bool bFlagsChanged = false;
    sal_uInt16 n = 0;
    if( (SbxVariable*) refVar == (SbxVariable*) pMeth )
    {
        bFlagsChanged = true;
        n = refVar->GetFlags();
        refVar->SetFlag( SBX_WRITE );
    }

    
    
    
    
    
    if ( bVBAEnabled )
    {
        
        
        
        
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





static const char pCollectionStr[] = "Collection";

void SbiRuntime::StepSET_Impl( SbxVariableRef& refVal, SbxVariableRef& refVar, bool bHandleDefaultProp )
{
    

    
    SbxDataType eVarType = refVar->GetType();
    if( !bHandleDefaultProp && eVarType != SbxOBJECT && !(eVarType & SbxARRAY) && refVar->IsFixed() )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
        return;
    }

    
    SbxDataType eValType = refVal->GetType();
    if( !bHandleDefaultProp && eValType != SbxOBJECT && !(eValType & SbxARRAY) && refVal->IsFixed() )
    {
        Error( SbERR_INVALID_USAGE_OBJECT );
        return;
    }

    
    
    if ( !bHandleDefaultProp || ( bHandleDefaultProp && eValType == SbxOBJECT ) )
    {
    
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
            
            if (  refVal->GetType() == SbxOBJECT )
            {
                
                
                SbxObject* pObj = NULL;


                pObj = PTR_CAST(SbxObject,(SbxVariable*)refVar);

                
                
                if ( !pObj && refVar->GetType() == SbxOBJECT )
                {
                    SbxBase* pObjVarObj = refVar->GetObject();
                    pObj = PTR_CAST(SbxObject,pObjVarObj);
                }
                SbxVariable* pDflt = NULL;
                if ( pObj && !bObjAssign )
                {
                    
                    pDflt = getDefaultProp( refVal );
                }
                if ( pDflt )
                {
                    refVal = pDflt;
                }
            }
        }

        
        bool bDimAsNew = bVBAEnabled && refVar->IsSet( SBX_DIM_AS_NEW );
        SbxBaseRef xPrevVarObj;
        if( bDimAsNew )
        {
            xPrevVarObj = refVar->GetObject();
        }
        
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
                refVal->SetComListener( xComListener, &rBasic );        
            }

        }

        
        
        
        
        
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
                    
                    bool bFirstInit = !xPrevVarObj.Is();
                    if( bFirstInit )
                    {
                        
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
    StepSET_Impl( refVal, refVar, bVBAEnabled ); 
}

void SbiRuntime::StepVBASET()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    
    StepSET_Impl( refVal, refVar, false ); 
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



void SbiRuntime::StepPUTC()
{
    SbxVariableRef refVal = PopVar();
    SbxVariableRef refVar = PopVar();
    refVar->SetFlag( SBX_WRITE );
    *refVar = *refVal;
    refVar->ResetFlag( SBX_WRITE );
    refVar->SetFlag( SBX_CONST );
}




void SbiRuntime::StepDIM()
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );
}


void SbiRuntime::DimImpl( SbxVariableRef refVar )
{
    
    
    
    if ( refRedim )
    {
        if ( !refRedimpArray ) 
        {
            lcl_eraseImpl( refVar, bVBAEnabled );
        }
        SbxDataType eType = refVar->GetType();
        lcl_clearImpl( refVar, eType );
        refRedim = NULL;
    }
    SbxArray* pDims = refVar->GetParameters();
    
    
    if( pDims && !( pDims->Count() & 1 ) )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    }
    else
    {
        SbxDataType eType = refVar->IsFixed() ? refVar->GetType() : SbxVARIANT;
        SbxDimArray* pArray = new SbxDimArray( eType );
        
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
            
            
            pArray->unoAddDim( 0, -1 );
        }
        sal_uInt16 nSavFlags = refVar->GetFlags();
        refVar->ResetFlag( SBX_FIXED );
        refVar->PutObject( pArray );
        refVar->SetFlags( nSavFlags );
        refVar->SetParameters( NULL );
    }
}





void SbiRuntime::StepREDIM()
{
    
    
    StepDIM();
}



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





void SbiRuntime::StepREDIMP()
{
    SbxVariableRef refVar = PopVar();
    DimImpl( refVar );

    
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
                
                sal_Int32* pLowerBounds = new sal_Int32[nDims];
                sal_Int32* pUpperBounds = new sal_Int32[nDims];
                sal_Int32* pActualIndices = new sal_Int32[nDims];

                
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
                    
                    pDimArray->SbxArray::Clear();
                    bClearValues = false;
                }
                else
                {
                    pDimArray->Clear(); 
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
    
    SbxVariableRef pVar = PopVar();
    SbxDataType t = pVar->GetType();

    SbxVariable* pCopyVar = new SbxVariable( t );
    pCopyVar->SetFlag( SBX_READWRITE );
    *pCopyVar = *pVar;

    PushVar( pCopyVar );
}




void SbiRuntime::StepARGC()
{
    PushArgv();
    refArgv = new SbxArray;
    nArgc = 1;
}



void SbiRuntime::StepARGV()
{
    if( !refArgv )
    {
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    }
    else
    {
        SbxVariableRef pVal = PopVar();

        
        if( pVal->ISA(SbxMethod) || pVal->ISA(SbUnoProperty) || pVal->ISA(SbProcedureProperty) )
        {
            
            SbxVariable* pRes = new SbxVariable( *pVal );
            pVal = pRes;
        }
        refArgv->Put( pVal, nArgc++ );
    }
}



void SbiRuntime::StepINPUT()
{
    OUStringBuffer sin;
    OUString s;
    char ch = 0;
    SbError err;
    
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
        
        
        if( !pVar->IsFixed() || pVar->IsNumeric() )
        {
            sal_uInt16 nLen = 0;
            if( !pVar->Scan( s, &nLen ) )
            {
                err = SbxBase::GetError();
                SbxBase::ResetError();
            }
            
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




void SbiRuntime::StepLINPUT()
{
    OString aInput;
    pIosys->Read( aInput );
    Error( pIosys->GetError() );
    SbxVariableRef p = PopVar();
    p->PutString(OStringToOUString(aInput, osl_getThreadTextEncoding()));
}



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



void SbiRuntime::StepCASE()
{
    if( !refCaseStk.Is() )
    {
        refCaseStk = new SbxArray;
    }
    SbxVariableRef xVar = PopVar();
    refCaseStk->Put( xVar, refCaseStk->Count() );
}



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
    pInst->aErrorMsg = "";
    pInst->nErr = 0L;
    pInst->nErl = 0;
    nError = 0L;
    SbxErrObject::getUnoErrObject()->Clear();
}

void SbiRuntime::StepNOERROR()
{
    pInst->aErrorMsg = "";
    pInst->nErr = 0L;
    pInst->nErl = 0;
    nError = 0L;
    SbxErrObject::getUnoErrObject()->Clear();
    bError = false;
}



void SbiRuntime::StepLEAVE()
{
    bRun = false;
        
    if ( bInError && pError )
    {
        SbxErrObject::getUnoErrObject()->Clear();
    }
}

void SbiRuntime::StepCHANNEL()      
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

void SbiRuntime::StepPRINT()        
{
    SbxVariableRef p = PopVar();
    OUString s1 = p->GetOUString();
    OUString s;
    if( p->GetType() >= SbxINTEGER && p->GetType() <= SbxDOUBLE )
    {
        s = " ";    
    }
    s += s1;
    pIosys->Write( s );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepPRINTF()       
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
    pIosys->Write( s.makeStringAndClear() );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepWRITE()        
{
    SbxVariableRef p = PopVar();
    
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
    pIosys->Write( s );
    Error( pIosys->GetError() );
}

void SbiRuntime::StepRENAME()       
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



void SbiRuntime::StepPROMPT()
{
    SbxVariableRef p = PopVar();
    OString aStr(OUStringToOString(p->GetOUString(), osl_getThreadTextEncoding()));
    pIosys->SetPrompt( aStr );
}



void SbiRuntime::StepRESTART()
{
    pRestart = pCode;
}



void SbiRuntime::StepEMPTY()
{
    
    
    
    
    SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
    xVar->PutErr( 448 );
    PushVar( xVar );
}



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



void SbiRuntime::StepLOADNC( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable( SbxDOUBLE );

    
    OUString aStr = pImg->GetString( static_cast<short>( nOp1 ) );
    
    sal_Int32 iComma = aStr.indexOf((sal_Unicode)',');
    if( iComma >= 0 )
    {
        aStr = aStr.replaceAt(iComma, 1, OUString("."));
    }
    double n = ::rtl::math::stringToDouble( aStr, '.', ',', NULL, NULL );

    p->PutDouble( n );
    PushVar( p );
}



void SbiRuntime::StepLOADSC( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable;
    p->PutString( pImg->GetString( static_cast<short>( nOp1 ) ) );
    PushVar( p );
}



void SbiRuntime::StepLOADI( sal_uInt32 nOp1 )
{
    SbxVariable* p = new SbxVariable;
    p->PutInteger( static_cast<sal_Int16>( nOp1 ) );
    PushVar( p );
}



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
            
            if ( pVal->GetType() == SbxEMPTY )
                pVal->Broadcast( SBX_HINT_DATAWANTED );
            
            SbxVariable* pRes = new SbxVariable( *pVal );
            pVal = pRes;
        }
        refArgv->Put( pVal, nArgc );
        refArgv->PutAlias( aAlias, nArgc++ );
    }
}



void SbiRuntime::StepARGTYP( sal_uInt32 nOp1 )
{
    if( !refArgv )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    else
    {
        bool bByVal = (nOp1 & 0x8000) != 0;         
        SbxDataType t = (SbxDataType) (nOp1 & 0x7FFF);
        SbxVariable* pVar = refArgv->Get( refArgv->Count() - 1 );   

        
        if( pVar->GetRefCount() > 2 )       
        {
            
            if( bByVal )
            {
                
                pVar = new SbxVariable( *pVar );
                pVar->SetFlag( SBX_READWRITE );
                refExprStk->Put( pVar, refArgv->Count() - 1 );
            }
            else
                pVar->SetFlag( SBX_REFERENCE );     
        }
        else
        {
            
            if( bByVal )
                pVar->ResetFlag( SBX_REFERENCE );   
            else
                Error( SbERR_BAD_PARAMETERS );      
        }

        if( pVar->GetType() != t )
        {
            
            
            pVar->Convert( SbxVARIANT );
            pVar->Convert( t );
        }
    }
}



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



void SbiRuntime::StepJUMP( sal_uInt32 nOp1 )
{
#ifdef DBG_UTIL
    
    
    if( nOp1 >= pImg->GetCodeSize() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
#endif
    pCode = (const sal_uInt8*) pImg->GetCode() + nOp1;
}



void SbiRuntime::StepJUMPT( sal_uInt32 nOp1 )
{
    SbxVariableRef p = PopVar();
    if( p->GetBool() )
        StepJUMP( nOp1 );
}



void SbiRuntime::StepJUMPF( sal_uInt32 nOp1 )
{
    SbxVariableRef p = PopVar();
    
        
    if( ( bVBAEnabled && p->IsNull() ) || !p->GetBool() )
        StepJUMP( nOp1 );
}









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



void SbiRuntime::StepGOSUB( sal_uInt32 nOp1 )
{
    PushGosub( pCode );
    if( nOp1 >= pImg->GetCodeSize() )
        StarBASIC::FatalError( SbERR_INTERNAL_ERROR );
    pCode = (const sal_uInt8*) pImg->GetCode() + nOp1;
}



void SbiRuntime::StepRETURN( sal_uInt32 nOp1 )
{
    PopGosub();
    if( nOp1 )
        StepJUMP( nOp1 );
}



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
    pInst->aErrorMsg = "";
    pInst->nErr = 0;
    pInst->nErl = 0;
    nError = 0;
    SbxErrObject::getUnoErrObject()->Clear();
}



void SbiRuntime::StepRESUME( sal_uInt32 nOp1 )
{
    
    if( !bInError )
    {
        Error( SbERR_BAD_RESUME );
        return;
    }
    if( nOp1 )
    {
        
        sal_uInt16 n1, n2;
        pCode = pMod->FindNextStmnt( pErrCode, n1, n2, sal_True, pImg );
    }
    else
        pCode = pErrStmnt;
    if ( pError ) 
        SbxErrObject::getUnoErrObject()->Clear();

    if( nOp1 > 1 )
        StepJUMP( nOp1 );
    pInst->aErrorMsg = "";
    pInst->nErr = 0;
    pInst->nErl = 0;
    nError = 0;
    bInError = false;
}


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



void SbiRuntime::StepPRCHAR( sal_uInt32 nOp1 )
{
    OUString s(static_cast<sal_Unicode>(nOp1));
    pIosys->Write( s );
    Error( pIosys->GetError() );
}



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
        StepSET_Impl( refVal, refVar, bHandleDflt ); 
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



void SbiRuntime::StepLIB( sal_uInt32 nOp1 )
{
    aLibName = pImg->GetString( static_cast<short>( nOp1 ) );
}





void SbiRuntime::StepBASED( sal_uInt32 nOp1 )
{
    SbxVariable* p1 = new SbxVariable;
    SbxVariableRef x2 = PopVar();

    
    bool bCompatible = ((nOp1 & 0x8000) != 0);
    sal_uInt16 uBase = static_cast<sal_uInt16>(nOp1 & 1);       
    p1->PutInteger( uBase );
    if( !bCompatible )
        x2->Compute( SbxPLUS, *p1 );
    PushVar( x2 );  
    PushVar( p1 );  
}




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
        
        
        if ( bIsVBAInterOp && aName.startsWith("[") )
        {
            
            StepARGC();
            
            OUString sArg = aName.copy( 1, aName.getLength() - 2 );
            SbxVariable* p = new SbxVariable;
            p->PutString( sArg );
            PushVar( p );
            StepARGV();
            nOp1 = nOp1 | 0x8000; 
            aName = "Evaluate";
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

            
            if( bLocal && pElem )   
            {
                if( pElem->IsSet( SBX_PRIVATE ) )
                {
                    SbiInstance* pInst_ = GetSbData()->pInst;
                    if( pInst_ && pInst_->IsCompatibility() && pObj != pElem->GetParent() )
                    {
                        pElem = NULL;   
                    }
                    
                }
            }
            rBasic.bNoRtl = bSave;

            
            if( bLocal && !pElem )
            {
                bool bSetName = true; 

                
                
                if ( bVBAEnabled )
                {
                    
                    pElem = rBasic.VBAFind( aName, SbxCLASS_DONTCARE );
                    if ( pElem )
                    {
                        bSetName = false; 
                    }
                    else
                    {
                        pElem = VBAConstantHelper::instance().getVBAConstant( aName );
                    }
                }

                if( !pElem )
                {
                    
                    
                    SbUnoClass* pUnoClass = findUnoClass( aName );
                    if( pUnoClass )
                    {
                        pElem = new SbxVariable( t );
                        SbxValues aRes( SbxOBJECT );
                        aRes.pObj = pUnoClass;
                        pElem->SbxVariable::Put( aRes );
                    }
                }

                
                
                
                
                if( pElem )
                {
                    
                    pElem->SetFlag( SBX_DONTSTORE );
                    pElem->SetFlag( SBX_NO_MODIFY);

                    
                    
                    if ( bSetName )
                    {
                        pElem->SetName( aName );
                    }
                    refLocals->Put( pElem, refLocals->Count() );
                }
            }

            if( !pElem )
            {
                
                
                if( nOp1 & 0x8000 )
                {
                    bFatalError = true;
                }

                
                if( !bLocal || pImg->GetFlag( SBIMG_EXPLICIT ) )
                {
                    
                    bFatalError = true;


                    if( !( nOp1 & 0x8000 ) && nNotFound == SbERR_PROC_UNDEFINED )
                    {
                        nNotFound = SbERR_VAR_UNDEFINED;
                    }
                }
                if( bFatalError )
                {
                    
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
        
        if( !bFatalError )
        {
            SetupArgs( pElem, nOp1 );
        }
        
        if( pElem->IsA( TYPE(SbxMethod) ) )
        {
            
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
            
            SbxVariableRef refTemp = pElem;

            
            
            sal_uInt16 nSavFlags = pElem->GetFlags();
            pElem->SetFlag( SBX_READWRITE | SBX_NO_BROADCAST );
            pElem->SbxValue::Clear();
            pElem->SetFlags( nSavFlags );

            
            

            
            SbxVariable* pNew = new SbxMethod( *((SbxMethod*)pElem) );
            

            pElem->SetParameters(0);
            pNew->SetFlag( SBX_READWRITE );

            if( bSet )
            {
                pElem->SetType( t2 );
            }
            pElem = pNew;
        }
        
        
        
        
        else if( bVBAEnabled && pElem->ISA(SbUnoProperty) && pElem->GetParameters() )
        {
            SbxVariableRef refTemp = pElem;

            
            SbxVariable* pNew = new SbxVariable( *((SbxVariable*)pElem) );
            pElem->SetParameters( NULL );
            pElem = pNew;
        }
    }
    return CheckArray( pElem );
}


SbxBase* SbiRuntime::FindElementExtern( const OUString& rName )
{
    
    

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
        
        OUString aMethName = pMeth->GetName();
        aMethName += ":";
        aMethName += rName;
        pElem = pMod->Find(aMethName, SbxCLASS_DONTCARE);
    }

    
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
                        if( xInvocation.is() )  
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
        
        refArgv->Put( p, 0 );
        p->SetParameters( refArgv );
        PopArgv();
    }
    else
    {
        p->SetParameters( NULL );
    }
}



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

        
        if( pPar )
        {
            pPar->Put( NULL, 0 );
        }
    }
    
    else if( pElem->GetType() == SbxOBJECT && !pElem->ISA(SbxMethod) && ( !bVBAEnabled || ( bVBAEnabled && !pElem->ISA(SbxProperty) ) ) )
    {
        pPar = pElem->GetParameters();
        if ( pPar )
        {
            
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
                                    
                                    StarBASIC::Error( SbERR_OUT_OF_RANGE );
                                }

                                
                                
                                pElem = new SbxVariable( SbxVARIANT );
                                if( xRet.is() )
                                {
                                    aAny <<= xRet;

                                    
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
                                sDefaultMethod = "getByIndex";
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
            else if( bVBAEnabled )  
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


void SbiRuntime::StepFIND( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    StepFIND_Impl( pMod, nOp1, nOp2, SbERR_PROC_UNDEFINED, true );
}


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




void SbiRuntime::StepELEM( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    SbxVariableRef pObjVar = PopVar();

    SbxObject* pObj = PTR_CAST(SbxObject,(SbxVariable*) pObjVar);
    if( !pObj )
    {
        SbxBase* pObjVarObj = pObjVar->GetObject();
        pObj = PTR_CAST(SbxObject,pObjVarObj);
    }

    
    
    
    
    if( pObj )
    {
        SaveRef( (SbxVariable*)pObj );
    }
    PushVar( FindElement( pObj, nOp1, nOp2, SbERR_NO_METHOD, false ) );
}






void SbiRuntime::StepPARAM( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    sal_uInt16 i = static_cast<sal_uInt16>( nOp1 & 0x7FFF );
    SbxDataType t = (SbxDataType) nOp2;
    SbxVariable* p;

    
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
                p->PutErr( 448 );       
            }
            refParams->Put( p, iLoop );
            iLoop--;
        }
    }
    p = refParams->Get( i );

    if( p->GetType() == SbxERROR && ( i ) )
    {
        
        bool bOpt = false;
        if( pMeth )
        {
            SbxInfo* pInfo = pMeth->GetInfo();
            if ( pInfo )
            {
                const SbxParamInfo* pParam = pInfo->GetParam( i );
                if( pParam && ( (pParam->nFlags & SBX_OPTIONAL) != 0 ) )
                {
                    
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




void SbiRuntime::StepCALL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    OUString aName = pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
    {
        pArgs = refArgv;
    }
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, false );
    aLibName = "";
    if( nOp1 & 0x8000 )
    {
        PopArgv();
    }
}



void SbiRuntime::StepCALLC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    OUString aName = pImg->GetString( static_cast<short>( nOp1 & 0x7FFF ) );
    SbxArray* pArgs = NULL;
    if( nOp1 & 0x8000 )
    {
        pArgs = refArgv;
    }
    DllCall( aName, aLibName, pArgs, (SbxDataType) nOp2, true );
    aLibName = "";
    if( nOp1 & 0x8000 )
    {
        PopArgv();
    }
}




void SbiRuntime::StepSTMNT( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    
    
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

    
    
    if( bFatalExpr)
    {
        StarBASIC::FatalError( SbERR_NO_METHOD, sUnknownMethodName );
        return;
    }
    pStmnt = pCode - 9;
    sal_uInt16 nOld = nLine;
    nLine = static_cast<short>( nOp1 );

    
    nCol1 = static_cast<short>( nOp2 & 0xFF );

    
    

    nCol2 = 0xffff;
    sal_uInt16 n1, n2;
    const sal_uInt8* p = pMod->FindNextStmnt( pCode, n1, n2 );
    if( p )
    {
        if( n1 == nOp1 )
        {
            
            nCol2 = (n2 & 0xFF) - 1;
        }
    }

    
    if( !bInError )
    {
        
        sal_uInt16 nExspectedForLevel = static_cast<sal_uInt16>( nOp2 / 0x100 );
        if( pGosubStk )
        {
            nExspectedForLevel = nExspectedForLevel + pGosubStk->nStartForLvl;
        }

        
        
        while( nForLvl > nExspectedForLevel )
        {
            PopFor();
        }
    }

    
    
    if( pInst->nCallLvl <= pInst->nBreakCallLvl )
    {
        StarBASIC* pStepBasic = GetCurrentBasic( &rBasic );
        sal_uInt16 nNewFlags = pStepBasic->StepPoint( nLine, nCol1, nCol2 );

        pInst->CalcBreakCallLevel( nNewFlags );
    }

    
    else if( ( nOp1 != nOld )
        && ( nFlags & SbDEBUG_BREAK )
        && pMod->IsBP( static_cast<sal_uInt16>( nOp1 ) ) )
    {
        StarBASIC* pBreakBasic = GetCurrentBasic( &rBasic );
        sal_uInt16 nNewFlags = pBreakBasic->BreakPoint( nLine, nCol1, nCol2 );

        pInst->CalcBreakCallLevel( nNewFlags );
    }
}






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


void SbiRuntime::StepDCREATE_IMPL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    SbxVariableRef refVar = PopVar();

    DimImpl( refVar );

    
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

        
        sal_Int32* pLowerBounds = new sal_Int32[nDims];
        sal_Int32* pUpperBounds = new sal_Int32[nDims];
        sal_Int32* pActualIndices = new sal_Int32[nDims];
        if( nDimsOld != nDimsNew )
        {
            bRangeError = true;
        }
        else
        {
            
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
            
            
            
            implCopyDimArray_DCREATE( pArray, pOldArray, nDims - 1,
                0, pActualIndices, pLowerBounds, pUpperBounds );
        }
        delete [] pUpperBounds;
        delete [] pLowerBounds;
        delete [] pActualIndices;
        refRedimpArray = NULL;
    }
}



SbxObject* createUserTypeImpl( const OUString& rClassName );  

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
        sal_uInt16 nCount = static_cast<sal_uInt16>( nOp2 >> 17 );      
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
    
    
    if( pMod->pImage->bFirstInit )
    {
        bool bUsedForClassModule = pImg->GetFlag( SBIMG_CLASSMODULE );
        StepPUBLIC_Impl( nOp1, nOp2, bUsedForClassModule );
    }
}



void SbiRuntime::StepGLOBAL( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( pImg->GetFlag( SBIMG_CLASSMODULE ) )
    {
        StepPUBLIC_Impl( nOp1, nOp2, true );
    }
    OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType)(nOp2 & 0xffff);

    
    
    
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
        
        p->SetFlag( SBX_NO_MODIFY);
    }
}





void SbiRuntime::StepGLOBAL_P( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( pMod->pImage->bFirstInit )
    {
        StepGLOBAL( nOp1, nOp2 );
    }
}





void SbiRuntime::StepFIND_G( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    if( pMod->pImage->bFirstInit )
    {
        
        StepFIND( nOp1, nOp2 );
    }
    else
    {
        
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

void SbiRuntime::StepSTATIC( sal_uInt32 nOp1, sal_uInt32 nOp2 )
{
    OUString aName( pImg->GetString( static_cast<short>( nOp1 ) ) );
    SbxDataType t = (SbxDataType) nOp2;
    StepSTATIC_Impl( aName, t );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
