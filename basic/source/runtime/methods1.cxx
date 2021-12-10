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

#include <config_features.h>

#include <sal/config.h>
#include <config_version.h>

#include <cstddef>

#include <stdlib.h>
#include <rtl/math.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/outdev.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <basic/sbxvar.hxx>
#include <basic/sbx.hxx>
#include <svl/zforlist.hxx>
#include <tools/urlobj.hxx>
#include <tools/fract.hxx>
#include <o3tl/temporary.hxx>
#include <osl/file.hxx>
#include <sbobjmod.hxx>
#include <basic/sbuno.hxx>

#include <date.hxx>
#include <sbintern.hxx>
#include <runtime.hxx>
#include <rtlproto.hxx>
#include "dllmgr.hxx"
#include <iosys.hxx>
#include <sbunoobj.hxx>
#include <propacc.hxx>
#include <sal/log.hxx>
#include <eventatt.hxx>
#include <rtl/math.h>
#include <svl/numformat.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/LocaleCalendar2.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>

#include <officecfg/Office/Scripting.hxx>

#include <memory>

using namespace comphelper;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;
using namespace com::sun::star::sheet;
using namespace com::sun::star::uno;

static Reference< XCalendar4 > const & getLocaleCalendar()
{
    static Reference< XCalendar4 > xCalendar = LocaleCalendar2::create(getProcessComponentContext());
    static css::lang::Locale aLastLocale;
    static bool bNeedsReload = true;

    css::lang::Locale aLocale = Application::GetSettings().GetLanguageTag().getLocale();
    bNeedsReload = bNeedsReload ||
           ( aLocale.Language != aLastLocale.Language ||
             aLocale.Country  != aLastLocale.Country ||
             aLocale.Variant  != aLastLocale.Variant );
    if( bNeedsReload )
    {
        bNeedsReload = false;
        aLastLocale = aLocale;
        xCalendar->loadDefaultCalendar( aLocale );
    }
    return xCalendar;
}

#if HAVE_FEATURE_SCRIPTING

void SbRtl_CallByName(StarBASIC *, SbxArray & rPar, bool)
{
    const sal_Int16 vbGet       = 2;
    const sal_Int16 vbLet       = 4;
    const sal_Int16 vbMethod    = 1;
    const sal_Int16 vbSet       = 8;

    // At least 3 parameter needed plus function itself -> 4
    sal_uInt32 nParCount = rPar.Count();
    if ( nParCount < 4 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // 1. parameter is object
    SbxBase* pObjVar = rPar.Get(1)->GetObject();
    SbxObject* pObj = nullptr;
    if( pObjVar )
        pObj = dynamic_cast<SbxObject*>( pObjVar );
    if( !pObj )
        if (auto pSbxVar = dynamic_cast<const SbxVariable*>( pObjVar))
            pObj = dynamic_cast<SbxObject*>( pSbxVar->GetObject() );
    if( !pObj )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_PARAMETER );
        return;
    }

    // 2. parameter is ProcedureName
    OUString aNameStr = rPar.Get(2)->GetOUString();

    // 3. parameter is CallType
    sal_Int16 nCallType = rPar.Get(3)->GetInteger();

    //SbxObject* pFindObj = NULL;
    SbxVariable* pFindVar = pObj->Find( aNameStr, SbxClassType::DontCare );
    if( pFindVar == nullptr )
    {
        StarBASIC::Error( ERRCODE_BASIC_PROC_UNDEFINED );
        return;
    }

    switch( nCallType )
    {
    case vbGet:
        {
            SbxValues aVals;
            aVals.eType = SbxVARIANT;
            pFindVar->Get( aVals );

            SbxVariableRef refVar = rPar.Get(0);
            refVar->Put( aVals );
        }
        break;
    case vbLet:
    case vbSet:
        {
            if ( nParCount != 5 )
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                return;
            }
            SbxVariableRef pValVar = rPar.Get(4);
            if( nCallType == vbLet )
            {
                SbxValues aVals;
                aVals.eType = SbxVARIANT;
                pValVar->Get( aVals );
                pFindVar->Put( aVals );
            }
            else
            {
                SbxVariableRef rFindVar = pFindVar;
                SbiInstance* pInst = GetSbData()->pInst;
                SbiRuntime* pRT = pInst ? pInst->pRun : nullptr;
                if( pRT != nullptr )
                {
                    pRT->StepSET_Impl( pValVar, rFindVar );
                }
            }
        }
        break;
    case vbMethod:
        {
            SbMethod* pMeth = dynamic_cast<SbMethod*>( pFindVar );
            if( pMeth == nullptr )
            {
                StarBASIC::Error( ERRCODE_BASIC_PROC_UNDEFINED );
                return;
            }

            // Setup parameters
            SbxArrayRef xArray;
            sal_uInt32 nMethParamCount = nParCount - 4;
            if( nMethParamCount > 0 )
            {
                xArray = new SbxArray;
                for( sal_uInt32 i = 0 ; i < nMethParamCount ; i++ )
                {
                    SbxVariable* pPar = rPar.Get(i + 4);
                    xArray->Put(pPar, i + 1);
                }
            }

            // Call method
            SbxVariableRef refVar = rPar.Get(0);
            if( xArray.is() )
                pMeth->SetParameters( xArray.get() );
            pMeth->Call( refVar.get() );
            pMeth->SetParameters( nullptr );
        }
        break;
    default:
        StarBASIC::Error( ERRCODE_BASIC_PROC_UNDEFINED );
    }
}

void SbRtl_CBool(StarBASIC *, SbxArray & rPar, bool) // JSM
{
    bool bVal = false;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        bVal = pSbxVariable->GetBool();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutBool(bVal);
}

void SbRtl_CByte(StarBASIC *, SbxArray & rPar, bool) // JSM
{
    sal_uInt8 nByte = 0;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        nByte = pSbxVariable->GetByte();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutByte(nByte);
}

void SbRtl_CCur(StarBASIC *, SbxArray & rPar, bool)
{
    sal_Int64 nCur = 0;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        nCur = pSbxVariable->GetCurrency();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutCurrency(nCur);
}

void SbRtl_CDec(StarBASIC *, SbxArray & rPar, bool)
{
#ifdef _WIN32
    SbxDecimal* pDec = nullptr;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        pDec = pSbxVariable->GetDecimal();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutDecimal(pDec);
#else
    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(ERRCODE_BASIC_NOT_IMPLEMENTED);
#endif
}

void SbRtl_CDate(StarBASIC *, SbxArray & rPar, bool) // JSM
{
    double nVal = 0.0;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetDate();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutDate(nVal);
}

void SbRtl_CDbl(StarBASIC *, SbxArray & rPar, bool)  // JSM
{
    double nVal = 0.0;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        if( pSbxVariable->GetType() == SbxSTRING )
        {
            // #41690
            OUString aScanStr = pSbxVariable->GetOUString();
            ErrCode Error = SbxValue::ScanNumIntnl( aScanStr, nVal );
            if( Error != ERRCODE_NONE )
            {
                StarBASIC::Error( Error );
            }
        }
        else
        {
            nVal = pSbxVariable->GetDouble();
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }

    rPar.Get(0)->PutDouble(nVal);
}

void SbRtl_CInt(StarBASIC *, SbxArray & rPar, bool)  // JSM
{
    sal_Int16 nVal = 0;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetInteger();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutInteger(nVal);
}

void SbRtl_CLng(StarBASIC *, SbxArray & rPar, bool)  // JSM
{
    sal_Int32 nVal = 0;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetLong();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutLong(nVal);
}

void SbRtl_CSng(StarBASIC *, SbxArray & rPar, bool)  // JSM
{
    float nVal = float(0.0);
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        if( pSbxVariable->GetType() == SbxSTRING )
        {
            // #41690
            double dVal = 0.0;
            OUString aScanStr = pSbxVariable->GetOUString();
            ErrCode Error = SbxValue::ScanNumIntnl( aScanStr, dVal, /*bSingle=*/true );
            if( SbxBase::GetError() == ERRCODE_NONE && Error != ERRCODE_NONE )
            {
                StarBASIC::Error( Error );
            }
            nVal = static_cast<float>(dVal);
        }
        else
        {
            nVal = pSbxVariable->GetSingle();
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutSingle(nVal);
}

void SbRtl_CStr(StarBASIC *, SbxArray & rPar, bool)  // JSM
{
    OUString aString;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        aString = pSbxVariable->GetOUString();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutString(aString);
}

void SbRtl_CVar(StarBASIC *, SbxArray & rPar, bool)  // JSM
{
    SbxValues aVals( SbxVARIANT );
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        pSbxVariable->Get( aVals );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->Put(aVals);
}

void SbRtl_CVErr(StarBASIC *, SbxArray & rPar, bool)
{
    sal_Int16 nErrCode = 0;
    if (rPar.Count() == 2)
    {
        SbxVariable* pSbxVariable = rPar.Get(1);
        nErrCode = pSbxVariable->GetInteger();
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutErr(nErrCode);
}

void SbRtl_Iif(StarBASIC *, SbxArray & rPar, bool) // JSM
{
    if (rPar.Count() == 4)
    {
        if (rPar.Get(1)->GetBool())
        {
            *rPar.Get(0) = *rPar.Get(2);
        }
        else
        {
            *rPar.Get(0) = *rPar.Get(3);
        }
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

void SbRtl_GetSystemType(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 1)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // Removed for SRC595
        rPar.Get(0)->PutInteger(-1);
    }
}

void SbRtl_GetGUIType(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 1)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // 17.7.2000 Make simple solution for testtool / fat office
#if   defined(_WIN32)
        rPar.Get(0)->PutInteger(1);
#elif defined(UNX)
        rPar.Get(0)->PutInteger(4);
#else
        rPar.Get(0)->PutInteger(-1);
#endif
    }
}

void SbRtl_Red(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int32 nRGB = rPar.Get(1)->GetLong();
        nRGB &= 0x00FF0000;
        nRGB >>= 16;
        rPar.Get(0)->PutInteger(static_cast<sal_Int16>(nRGB));
    }
}

void SbRtl_Green(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int32 nRGB = rPar.Get(1)->GetLong();
        nRGB &= 0x0000FF00;
        nRGB >>= 8;
        rPar.Get(0)->PutInteger(static_cast<sal_Int16>(nRGB));
    }
}

void SbRtl_Blue(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        sal_Int32 nRGB = rPar.Get(1)->GetLong();
        nRGB &= 0x000000FF;
        rPar.Get(0)->PutInteger(static_cast<sal_Int16>(nRGB));
    }
}


void SbRtl_Switch(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nCount = rPar.Count();
    if( !(nCount & 0x0001 ))
    {
        // number of arguments must be odd
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    sal_uInt32 nCurExpr = 1;
    while( nCurExpr < (nCount-1) )
    {
        if (rPar.Get(nCurExpr)->GetBool())
        {
            (*rPar.Get(0)) = *(rPar.Get(nCurExpr + 1));
            return;
        }
        nCurExpr += 2;
    }
    rPar.Get(0)->PutNull();
}

//i#64882# Common wait impl for existing Wait and new WaitUntil
// rtl functions
void Wait_Impl( bool bDurationBased, SbxArray& rPar )
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    tools::Long nWait = 0;
    if ( bDurationBased )
    {
        double dWait = rPar.Get(1)->GetDouble();
        double dNow = Now_Impl();
        double dSecs = ( dWait - dNow ) * 24.0 * 3600.0;
        nWait = static_cast<tools::Long>( dSecs * 1000 ); // wait in thousands of sec
    }
    else
    {
        nWait = rPar.Get(1)->GetLong();
    }
    if( nWait < 0 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    Timer aTimer("basic Wait_Impl");
    aTimer.SetTimeout( nWait );
    aTimer.Start();
    while ( aTimer.IsActive() && !Application::IsQuit())
    {
        Application::Yield();
    }
}

//i#64882#
void SbRtl_Wait(StarBASIC *, SbxArray & rPar, bool)
{
    Wait_Impl( false, rPar );
}

//i#64882# add new WaitUntil ( for application.wait )
// share wait_impl with 'normal' oobasic wait
void SbRtl_WaitUntil(StarBASIC *, SbxArray & rPar, bool)
{
    Wait_Impl( true, rPar );
}

void SbRtl_DoEvents(StarBASIC *, SbxArray & rPar, bool)
{
// don't understand what upstream are up to
// we already process application events etc. in between
// basic runtime pcode ( on a timed basis )
    // always return 0
    rPar.Get(0)->PutInteger(0);
    Application::Reschedule( true );
}

void SbRtl_GetGUIVersion(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 1)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        // Removed for SRC595
        rPar.Get(0)->PutLong(-1);
    }
}

void SbRtl_Choose(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    sal_Int16 nIndex = rPar.Get(1)->GetInteger();
    sal_uInt32 nCount = rPar.Count();
    nCount--;
    if( nCount == 1 || nIndex > sal::static_int_cast<sal_Int16>(nCount-1) || nIndex < 1 )
    {
        rPar.Get(0)->PutNull();
        return;
    }
    (*rPar.Get(0)) = *(rPar.Get(nIndex + 1));
}


void SbRtl_Trim(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        OUString aStr(comphelper::string::strip(rPar.Get(1)->GetOUString(), ' '));
        rPar.Get(0)->PutString(aStr);
    }
}

void SbRtl_GetSolarVersion(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutLong(LIBO_VERSION_MAJOR * 10000 + LIBO_VERSION_MINOR * 100
                         + LIBO_VERSION_MICRO * 1);
}

void SbRtl_TwipsPerPixelX(StarBASIC *, SbxArray & rPar, bool)
{
    sal_Int32 nResult = 0;
    Size aSize( 100,0 );
    MapMode aMap( MapUnit::MapTwip );
    OutputDevice* pDevice = Application::GetDefaultDevice();
    if( pDevice )
    {
        aSize = pDevice->PixelToLogic( aSize, aMap );
        nResult = aSize.Width() / 100;
    }
    rPar.Get(0)->PutLong(nResult);
}

void SbRtl_TwipsPerPixelY(StarBASIC *, SbxArray & rPar, bool)
{
    sal_Int32 nResult = 0;
    Size aSize( 0,100 );
    MapMode aMap( MapUnit::MapTwip );
    OutputDevice* pDevice = Application::GetDefaultDevice();
    if( pDevice )
    {
        aSize = pDevice->PixelToLogic( aSize, aMap );
        nResult = aSize.Height() / 100;
    }
    rPar.Get(0)->PutLong(nResult);
}


void SbRtl_FreeLibrary(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    GetSbData()->pInst->GetDllMgr()->FreeDll(rPar.Get(1)->GetOUString());
}
bool IsBaseIndexOne()
{
    bool bResult = false;
    if ( GetSbData()->pInst && GetSbData()->pInst->pRun )
    {
        sal_uInt16 res = GetSbData()->pInst->pRun->GetBase();
        if ( res )
        {
            bResult = true;
        }
    }
    return bResult;
}

void SbRtl_Array(StarBASIC *, SbxArray & rPar, bool)
{
    SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
    sal_uInt32 nArraySize = rPar.Count() - 1;
    bool bIncIndex = IsBaseIndexOne();
    if( nArraySize )
    {
        if ( bIncIndex )
        {
            pArray->AddDim(1, sal::static_int_cast<sal_Int32>(nArraySize));
        }
        else
        {
            pArray->AddDim(0, sal::static_int_cast<sal_Int32>(nArraySize) - 1);
        }
    }
    else
    {
        pArray->unoAddDim(0, -1);
    }

    // insert parameters into the array
    for( sal_uInt32 i = 0 ; i < nArraySize ; i++ )
    {
        SbxVariable* pVar = rPar.Get(i + 1);
        SbxVariable* pNew = new SbxEnsureParentVariable(*pVar);
        pNew->SetFlag( SbxFlagBits::Write );
        sal_Int32 aIdx[1];
        aIdx[0] = static_cast<sal_Int32>(i);
        if ( bIncIndex )
        {
            ++aIdx[0];
        }
        pArray->Put(pNew, aIdx);
    }

    // return array
    SbxVariableRef refVar = rPar.Get(0);
    SbxFlagBits nFlags = refVar->GetFlags();
    refVar->ResetFlag( SbxFlagBits::Fixed );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( nullptr );
}


// Featurewish #57868
// The function returns a variant-array; if there are no parameters passed,
// an empty array is created (according to dim a(); equal to a sequence of
// the length 0 in Uno).
// If there are parameters passed, there's a dimension created for each of
// them; DimArray( 2, 2, 4 ) is equal to DIM a( 2, 2, 4 )
// the array is always of the type variant
void SbRtl_DimArray(StarBASIC *, SbxArray & rPar, bool)
{
    SbxDimArray * pArray = new SbxDimArray( SbxVARIANT );
    sal_uInt32 nArrayDims = rPar.Count() - 1;
    if( nArrayDims > 0 )
    {
        for( sal_uInt32 i = 0; i < nArrayDims ; i++ )
        {
            sal_Int32 ub = rPar.Get(i + 1)->GetLong();
            if( ub < 0 )
            {
                StarBASIC::Error( ERRCODE_BASIC_OUT_OF_RANGE );
                ub = 0;
            }
            pArray->AddDim(0, ub);
        }
    }
    else
    {
        pArray->unoAddDim(0, -1);
    }
    SbxVariableRef refVar = rPar.Get(0);
    SbxFlagBits nFlags = refVar->GetFlags();
    refVar->ResetFlag( SbxFlagBits::Fixed );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( nullptr );
}

/*
 * FindObject and FindPropertyObject make it possible to
 * address objects and properties of the type Object with
 * their name as string-parameters at the runtime.
 *
 * Example:
 * MyObj.Prop1.Bla = 5
 *
 * is equal to:
 * dim ObjVar as Object
 * dim ObjProp as Object
 * ObjName$ = "MyObj"
 * ObjVar = FindObject( ObjName$ )
 * PropName$ = "Prop1"
 * ObjProp = FindPropertyObject( ObjVar, PropName$ )
 * ObjProp.Bla = 5
 *
 * The names can be created dynamically at the runtime
 * so that e. g. via controls "TextEdit1" to "TextEdit5"
 * can be iterated in a dialog in a loop.
 */


// 1st parameter = the object's name as string
void SbRtl_FindObject(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    OUString aNameStr = rPar.Get(1)->GetOUString();

    SbxBase* pFind =  StarBASIC::FindSBXInCurrentScope( aNameStr );
    SbxObject* pFindObj = nullptr;
    if( pFind )
    {
        pFindObj = dynamic_cast<SbxObject*>( pFind );
    }
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( pFindObj );
}

// address object-property in an object
// 1st parameter = object
// 2nd parameter = the property's name as string
void SbRtl_FindPropertyObject(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() < 3)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    SbxBase* pObjVar = rPar.Get(1)->GetObject();
    SbxObject* pObj = nullptr;
    if( pObjVar )
    {
        pObj = dynamic_cast<SbxObject*>( pObjVar );
    }
    if( !pObj )
        if (auto pSbxVar = dynamic_cast<const SbxVariable*>( pObjVar))
            pObj = dynamic_cast<SbxObject*>( pSbxVar->GetObject() );

    OUString aNameStr = rPar.Get(2)->GetOUString();

    SbxObject* pFindObj = nullptr;
    if( pObj )
    {
        SbxVariable* pFindVar = pObj->Find( aNameStr, SbxClassType::Object );
        pFindObj = dynamic_cast<SbxObject*>( pFindVar );
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_PARAMETER );
    }

    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( pFindObj );
}


static bool lcl_WriteSbxVariable( const SbxVariable& rVar, SvStream* pStrm,
                                      bool bBinary, short nBlockLen, bool bIsArray )
{
    sal_uInt64 const nFPos = pStrm->Tell();

    bool bIsVariant = !rVar.IsFixed();
    SbxDataType eType = rVar.GetType();

    switch( eType )
    {
    case SbxBOOL:
    case SbxCHAR:
    case SbxBYTE:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( SbxBYTE ); // VarType Id
        }
        pStrm->WriteUChar( rVar.GetByte() );
        break;

    case SbxEMPTY:
    case SbxNULL:
    case SbxVOID:
    case SbxINTEGER:
    case SbxUSHORT:
    case SbxINT:
    case SbxUINT:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( SbxINTEGER ); // VarType Id
        }
        pStrm->WriteInt16( rVar.GetInteger() );
        break;

    case SbxLONG:
    case SbxULONG:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( SbxLONG ); // VarType Id
        }
        pStrm->WriteInt32( rVar.GetLong() );
        break;
    case SbxSALINT64:
    case SbxSALUINT64:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( SbxSALINT64 ); // VarType Id
        }
        pStrm->WriteUInt64( rVar.GetInt64() );
        break;
    case SbxSINGLE:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( eType ); // VarType Id
        }
        pStrm->WriteFloat( rVar.GetSingle() );
        break;

    case SbxDOUBLE:
    case SbxCURRENCY:
    case SbxDATE:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( eType ); // VarType Id
        }
        pStrm->WriteDouble( rVar.GetDouble() );
        break;

    case SbxSTRING:
    case SbxLPSTR:
        {
            const OUString& rStr = rVar.GetOUString();
            if( !bBinary || bIsArray )
            {
                if( bIsVariant )
                {
                    pStrm->WriteUInt16( SbxSTRING );
                }
                pStrm->WriteUniOrByteString( rStr, osl_getThreadTextEncoding() );
            }
            else
            {
                // without any length information! without end-identifier!
                // What does that mean for Unicode?! Choosing conversion to ByteString...
                OString aByteStr(OUStringToOString(rStr, osl_getThreadTextEncoding()));
                pStrm->WriteOString( aByteStr );
            }
        }
        break;

    default:
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return false;
    }

    if( nBlockLen )
    {
        pStrm->Seek( nFPos + nBlockLen );
    }
    return pStrm->GetErrorCode() == ERRCODE_NONE;
}

static bool lcl_ReadSbxVariable( SbxVariable& rVar, SvStream* pStrm,
                                     bool bBinary, short nBlockLen )
{
    double aDouble;

    sal_uInt64 const nFPos = pStrm->Tell();

    bool bIsVariant = !rVar.IsFixed();
    SbxDataType eVarType = rVar.GetType();

    SbxDataType eSrcType = eVarType;
    if( bIsVariant )
    {
        sal_uInt16 nTemp;
        pStrm->ReadUInt16( nTemp );
        eSrcType = static_cast<SbxDataType>(nTemp);
    }

    switch( eSrcType )
    {
    case SbxBOOL:
    case SbxCHAR:
    case SbxBYTE:
        {
            sal_uInt8 aByte;
            pStrm->ReadUChar( aByte );

            if( bBinary && SbiRuntime::isVBAEnabled() && aByte == 1 && pStrm->eof() )
            {
                aByte = 0;
            }
            rVar.PutByte( aByte );
        }
        break;

    case SbxEMPTY:
    case SbxNULL:
    case SbxVOID:
    case SbxINTEGER:
    case SbxUSHORT:
    case SbxINT:
    case SbxUINT:
        {
            sal_Int16 aInt;
            pStrm->ReadInt16( aInt );
            rVar.PutInteger( aInt );
        }
        break;

    case SbxLONG:
    case SbxULONG:
        {
            sal_Int32 aInt;
            pStrm->ReadInt32( aInt );
            rVar.PutLong( aInt );
        }
        break;
    case SbxSALINT64:
    case SbxSALUINT64:
        {
            sal_uInt32 aInt;
            pStrm->ReadUInt32( aInt );
            rVar.PutInt64( static_cast<sal_Int64>(aInt) );
        }
        break;
    case SbxSINGLE:
        {
            float nS;
            pStrm->ReadFloat( nS );
            rVar.PutSingle( nS );
        }
        break;

    case SbxDOUBLE:
    case SbxCURRENCY:
        {
            pStrm->ReadDouble( aDouble );
            rVar.PutDouble( aDouble );
        }
        break;

    case SbxDATE:
        {
            pStrm->ReadDouble( aDouble );
            rVar.PutDate( aDouble );
        }
        break;

    case SbxSTRING:
    case SbxLPSTR:
        {
            OUString aStr = pStrm->ReadUniOrByteString(osl_getThreadTextEncoding());
            rVar.PutString( aStr );
        }
        break;

    default:
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return false;
    }

    if( nBlockLen )
    {
        pStrm->Seek( nFPos + nBlockLen );
    }
    return pStrm->GetErrorCode() == ERRCODE_NONE;
}


// nCurDim = 1...n
static bool lcl_WriteReadSbxArray( SbxDimArray& rArr, SvStream* pStrm,
    bool bBinary, sal_Int32 nCurDim, sal_Int32* pOtherDims, bool bWrite )
{
    SAL_WARN_IF( nCurDim <= 0,"basic", "Bad Dim");
    sal_Int32 nLower, nUpper;
    if (!rArr.GetDim(nCurDim, nLower, nUpper))
        return false;
    for(sal_Int32 nCur = nLower; nCur <= nUpper; nCur++ )
    {
        pOtherDims[ nCurDim-1 ] = nCur;
        if( nCurDim != 1 )
            lcl_WriteReadSbxArray(rArr, pStrm, bBinary, nCurDim-1, pOtherDims, bWrite);
        else
        {
            SbxVariable* pVar = rArr.Get(pOtherDims);
            bool bRet;
            if( bWrite )
                bRet = lcl_WriteSbxVariable(*pVar, pStrm, bBinary, 0, true );
            else
                bRet = lcl_ReadSbxVariable(*pVar, pStrm, bBinary, 0 );
            if( !bRet )
                return false;
        }
    }
    return true;
}

static void PutGet( SbxArray& rPar, bool bPut )
{
    if (rPar.Count() != 4)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nFileNo = rPar.Get(1)->GetInteger();
    SbxVariable* pVar2 = rPar.Get(2);
    SbxDataType eType2 = pVar2->GetType();
    bool bHasRecordNo = (eType2 != SbxEMPTY && eType2 != SbxERROR);
    tools::Long nRecordNo = pVar2->GetLong();
    if ( nFileNo < 1 || ( bHasRecordNo && nRecordNo < 1 ) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    nRecordNo--;
    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
    SbiStream* pSbStrm = pIO->GetStream( nFileNo );

    if ( !pSbStrm || !(pSbStrm->GetMode() & (SbiStreamFlags::Binary | SbiStreamFlags::Random)) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
        return;
    }

    SvStream* pStrm = pSbStrm->GetStrm();
    bool bRandom = pSbStrm->IsRandom();
    short nBlockLen = bRandom ? pSbStrm->GetBlockLen() : 0;

    if( bPut )
    {
        pSbStrm->ExpandFile();
    }

    if( bHasRecordNo )
    {
        sal_uInt64 const nFilePos = bRandom
            ? static_cast<sal_uInt64>(nBlockLen * nRecordNo)
            : static_cast<sal_uInt64>(nRecordNo);
        pStrm->Seek( nFilePos );
    }

    SbxDimArray* pArr = nullptr;
    SbxVariable* pVar = rPar.Get(3);
    if( pVar->GetType() & SbxARRAY )
    {
        SbxBase* pParObj = pVar->GetObject();
        pArr = dynamic_cast<SbxDimArray*>( pParObj );
    }

    bool bRet;

    if( pArr )
    {
        sal_uInt64 const nFPos = pStrm->Tell();
        sal_Int32 nDims = pArr->GetDims();
        std::unique_ptr<sal_Int32[]> pDims(new sal_Int32[ nDims ]);
        bRet = lcl_WriteReadSbxArray(*pArr,pStrm,!bRandom,nDims,pDims.get(),bPut);
        pDims.reset();
        if( nBlockLen )
            pStrm->Seek( nFPos + nBlockLen );
    }
    else
    {
        if( bPut )
            bRet = lcl_WriteSbxVariable(*pVar, pStrm, !bRandom, nBlockLen, false);
        else
            bRet = lcl_ReadSbxVariable(*pVar, pStrm, !bRandom, nBlockLen);
    }
    if( !bRet || pStrm->GetErrorCode() )
        StarBASIC::Error( ERRCODE_BASIC_IO_ERROR );
}

void SbRtl_Put(StarBASIC *, SbxArray & rPar, bool)
{
    PutGet( rPar, true );
}

void SbRtl_Get(StarBASIC *, SbxArray & rPar, bool)
{
    PutGet( rPar, false );
}

void SbRtl_Environ(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    OUString aResult;
    // should be ANSI but that's not possible under Win16 in the DLL
    OString aByteStr(OUStringToOString(rPar.Get(1)->GetOUString(),
                                                 osl_getThreadTextEncoding()));
    const char* pEnvStr = getenv(aByteStr.getStr());
    if ( pEnvStr )
    {
        aResult = OUString(pEnvStr, strlen(pEnvStr), osl_getThreadTextEncoding());
    }
    rPar.Get(0)->PutString(aResult);
}

static double GetDialogZoomFactor( bool bX, tools::Long nValue )
{
    OutputDevice* pDevice = Application::GetDefaultDevice();
    double nResult = 0;
    if( pDevice )
    {
        Size aRefSize( nValue, nValue );
        Fraction aFracX( 1, 26 );
        Fraction aFracY( 1, 24 );
        MapMode aMap( MapUnit::MapAppFont, Point(), aFracX, aFracY );
        Size aScaledSize = pDevice->LogicToPixel( aRefSize, aMap );
        aRefSize = pDevice->LogicToPixel( aRefSize, MapMode(MapUnit::MapTwip) );

        double nRef, nScaled;
        if( bX )
        {
            nRef = aRefSize.Width();
            nScaled = aScaledSize.Width();
        }
        else
        {
            nRef = aRefSize.Height();
            nScaled = aScaledSize.Height();
        }
        nResult = nScaled / nRef;
    }
    return nResult;
}


void SbRtl_GetDialogZoomFactorX(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutDouble(GetDialogZoomFactor(true, rPar.Get(1)->GetLong()));
}

void SbRtl_GetDialogZoomFactorY(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutDouble(GetDialogZoomFactor(false, rPar.Get(1)->GetLong()));
}


void SbRtl_EnableReschedule(StarBASIC *, SbxArray & rPar, bool)
{
    rPar.Get(0)->PutEmpty();
    if (rPar.Count() != 2)
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    if( GetSbData()->pInst )
        GetSbData()->pInst->EnableReschedule(rPar.Get(1)->GetBool());
}

void SbRtl_GetSystemTicks(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 1)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutLong(tools::Time::GetSystemTicks());
}

void SbRtl_GetPathSeparator(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 1)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutString(OUString(SAL_PATHDELIMITER));
}

void SbRtl_ResolvePath(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() == 2)
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        rPar.Get(0)->PutString(aStr);
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

void SbRtl_TypeLen(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        SbxDataType eType = rPar.Get(1)->GetType();
        sal_Int16 nLen = 0;
        switch( eType )
        {
        case SbxEMPTY:
        case SbxNULL:
        case SbxVECTOR:
        case SbxARRAY:
        case SbxBYREF:
        case SbxVOID:
        case SbxHRESULT:
        case SbxPOINTER:
        case SbxDIMARRAY:
        case SbxCARRAY:
        case SbxUSERDEF:
            nLen = 0;
            break;

        case SbxINTEGER:
        case SbxERROR:
        case SbxUSHORT:
        case SbxINT:
        case SbxUINT:
            nLen = 2;
            break;

        case SbxLONG:
        case SbxSINGLE:
        case SbxULONG:
            nLen = 4;
            break;

        case SbxDOUBLE:
        case SbxCURRENCY:
        case SbxDATE:
        case SbxSALINT64:
        case SbxSALUINT64:
            nLen = 8;
            break;

        case SbxOBJECT:
        case SbxVARIANT:
        case SbxDATAOBJECT:
            nLen = 0;
            break;

        case SbxCHAR:
        case SbxBYTE:
        case SbxBOOL:
            nLen = 1;
                break;

        case SbxLPSTR:
        case SbxLPWSTR:
        case SbxCoreSTRING:
        case SbxSTRING:
            nLen = static_cast<sal_Int16>(rPar.Get(1)->GetOUString().getLength());
            break;

        default:
            nLen = 0;
            break;
        }
        rPar.Get(0)->PutInteger(nLen);
    }
}


// 1st parameter == class name, other parameters for initialisation
void SbRtl_CreateUnoStruct(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_CreateUnoStruct( rPar );
}


// 1st parameter == service-name
void SbRtl_CreateUnoService(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_CreateUnoService( rPar );
}

void SbRtl_CreateUnoServiceWithArguments(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_CreateUnoServiceWithArguments( rPar );
}


void SbRtl_CreateUnoValue(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_CreateUnoValue( rPar );
}


// no parameters
void SbRtl_GetProcessServiceManager(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_GetProcessServiceManager( rPar );
}


// 1st parameter == Sequence<PropertyValue>
void SbRtl_CreatePropertySet(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_CreatePropertySet( rPar );
}


// multiple interface-names as parameters
void SbRtl_HasUnoInterfaces(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_HasInterfaces( rPar );
}


void SbRtl_IsUnoStruct(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_IsUnoStruct( rPar );
}


void SbRtl_EqualUnoObjects(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_EqualUnoObjects( rPar );
}

void SbRtl_CreateUnoDialog(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_CreateUnoDialog( rPar );
}

// Return the application standard lib as root scope
void SbRtl_GlobalScope(StarBASIC * pBasic, SbxArray & rPar, bool)
{
    SbxObject* p = pBasic;
    while( p->GetParent() )
    {
        p = p->GetParent();
    }
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( p );
}

// Helper functions to convert Url from/to system paths
void SbRtl_ConvertToUrl(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() == 2)
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        INetURLObject aURLObj( aStr, INetProtocol::File );
        OUString aFileURL = aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
        if( aFileURL.isEmpty() )
        {
            osl::File::getFileURLFromSystemPath(aStr, aFileURL);
        }
        if( aFileURL.isEmpty() )
        {
            aFileURL = aStr;
        }
        rPar.Get(0)->PutString(aFileURL);
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

void SbRtl_ConvertFromUrl(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() == 2)
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        OUString aSysPath;
        ::osl::File::getSystemPathFromFileURL( aStr, aSysPath );
        if( aSysPath.isEmpty() )
        {
            aSysPath = aStr;
        }
        rPar.Get(0)->PutString(aSysPath);
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}


// Provide DefaultContext
void SbRtl_GetDefaultContext(StarBASIC *, SbxArray & rPar, bool)
{
    RTL_Impl_GetDefaultContext( rPar );
}

void SbRtl_Join(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if ( nParCount != 3 && nParCount != 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    SbxBase* pParObj = rPar.Get(1)->GetObject();
    SbxDimArray* pArr = dynamic_cast<SbxDimArray*>( pParObj );
    if( pArr )
    {
        if (pArr->GetDims() != 1)
        {
            StarBASIC::Error( ERRCODE_BASIC_WRONG_DIMS );   // Syntax Error?!
            return;
        }
        OUString aDelim;
        if( nParCount == 3 )
        {
            aDelim = rPar.Get(2)->GetOUString();
        }
        else
        {
            aDelim = " ";
        }
        OUStringBuffer aRetStr(32);
        sal_Int32 nLower, nUpper;
        pArr->GetDim(1, nLower, nUpper);
        sal_Int32 aIdx[1];
        for (aIdx[0] = nLower; aIdx[0] <= nUpper; ++aIdx[0])
        {
            OUString aStr = pArr->Get(aIdx)->GetOUString();
            aRetStr.append(aStr);
            if (aIdx[0] != nUpper)
            {
                aRetStr.append(aDelim);
            }
        }
        rPar.Get(0)->PutString(aRetStr.makeStringAndClear());
    }
    else
    {
        StarBASIC::Error( ERRCODE_BASIC_MUST_HAVE_DIMS );
    }
}


void SbRtl_Split(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if ( nParCount < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    OUString aExpression = rPar.Get(1)->GetOUString();
    sal_Int32 nArraySize = 0;
    std::vector< OUString > vRet;
    if( !aExpression.isEmpty() )
    {
        OUString aDelim;
        if( nParCount >= 3 )
        {
            aDelim = rPar.Get(2)->GetOUString();
        }
        else
        {
            aDelim = " ";
        }

        sal_Int32 nCount = -1;
        if( nParCount == 4 )
        {
            nCount = rPar.Get(3)->GetLong();
        }
        sal_Int32 nDelimLen = aDelim.getLength();
        if( nDelimLen )
        {
            sal_Int32 iSearch = -1;
            sal_Int32 iStart = 0;
            do
            {
                bool bBreak = false;
                if( nCount >= 0 && nArraySize == nCount - 1 )
                {
                    bBreak = true;
                }
                iSearch = aExpression.indexOf( aDelim, iStart );
                OUString aSubStr;
                if( iSearch >= 0 && !bBreak )
                {
                    aSubStr = aExpression.copy( iStart, iSearch - iStart );
                    iStart = iSearch + nDelimLen;
                }
                else
                {
                    aSubStr = aExpression.copy( iStart );
                }
                vRet.push_back( aSubStr );
                nArraySize++;

                if( bBreak )
                {
                    break;
                }
            }
            while( iSearch >= 0 );
        }
        else
        {
            vRet.push_back( aExpression );
            nArraySize = 1;
        }
    }

    // tdf#123025 - split returns an array of substrings
    SbxDimArray* pArray = new SbxDimArray( SbxSTRING );
    pArray->unoAddDim(0, nArraySize - 1);

    // insert parameter(s) into the array
    const bool bIsVBAInterOp = SbiRuntime::isVBAEnabled();
    for(sal_Int32 i = 0 ; i < nArraySize ; i++ )
    {
        // tdf#123025 - split returns an array of substrings
        SbxVariableRef xVar = new SbxVariable( SbxSTRING );
        xVar->PutString( vRet[i] );
        // tdf#144924 - allow the assignment of different data types to the individual elements
        if (!bIsVBAInterOp)
        {
            xVar->ResetFlag(SbxFlagBits::Fixed);
        }
        pArray->Put(xVar.get(), &i);
    }

    // return array
    SbxVariableRef refVar = rPar.Get(0);
    SbxFlagBits nFlags = refVar->GetFlags();
    refVar->ResetFlag( SbxFlagBits::Fixed );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( nullptr );
}

// MonthName(month[, abbreviate])
void SbRtl_MonthName(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if( nParCount != 2 && nParCount != 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    const Reference< XCalendar4 >& xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );
        return;
    }
    Sequence< CalendarItem2 > aMonthSeq = xCalendar->getMonths2();
    sal_Int32 nMonthCount = aMonthSeq.getLength();

    sal_Int16 nVal = rPar.Get(1)->GetInteger();
    if( nVal < 1 || nVal > nMonthCount )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    bool bAbbreviate = false;
    if( nParCount == 3 )
        bAbbreviate = rPar.Get(2)->GetBool();

    const CalendarItem2* pCalendarItems = aMonthSeq.getConstArray();
    const CalendarItem2& rItem = pCalendarItems[nVal - 1];

    OUString aRetStr = ( bAbbreviate ? rItem.AbbrevName : rItem.FullName );
    rPar.Get(0)->PutString(aRetStr);
}

// WeekdayName(weekday, abbreviate, firstdayofweek)
void SbRtl_WeekdayName(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if( nParCount < 2 || nParCount > 4 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    const Reference< XCalendar4 >& xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );
        return;
    }

    Sequence< CalendarItem2 > aDaySeq = xCalendar->getDays2();
    sal_Int16 nDayCount = static_cast<sal_Int16>(aDaySeq.getLength());
    sal_Int16 nDay = rPar.Get(1)->GetInteger();
    sal_Int16 nFirstDay = 0;
    if( nParCount == 4 )
    {
        nFirstDay = rPar.Get(3)->GetInteger();
        if( nFirstDay < 0 || nFirstDay > 7 )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
            return;
        }
    }
    if( nFirstDay == 0 )
    {
        nFirstDay = sal_Int16( xCalendar->getFirstDayOfWeek() + 1 );
    }
    nDay = 1 + (nDay + nDayCount + nFirstDay - 2) % nDayCount;
    if( nDay < 1 || nDay > nDayCount )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    bool bAbbreviate = false;
    if( nParCount >= 3 )
    {
        SbxVariable* pPar2 = rPar.Get(2);
        if( !pPar2->IsErr() )
        {
            bAbbreviate = pPar2->GetBool();
        }
    }

    const CalendarItem2* pCalendarItems = aDaySeq.getConstArray();
    const CalendarItem2& rItem = pCalendarItems[nDay - 1];

    OUString aRetStr = ( bAbbreviate ? rItem.AbbrevName : rItem.FullName );
    rPar.Get(0)->PutString(aRetStr);
}

void SbRtl_Weekday(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if ( nParCount < 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
    else
    {
        double aDate = rPar.Get(1)->GetDate();

        bool bFirstDay = false;
        sal_Int16 nFirstDay = 0;
        if ( nParCount > 2 )
        {
            nFirstDay = rPar.Get(2)->GetInteger();
            bFirstDay = true;
        }
        sal_Int16 nDay = implGetWeekDay( aDate, bFirstDay, nFirstDay );
        rPar.Get(0)->PutInteger(nDay);
    }
}

namespace {

enum Interval
{
    INTERVAL_YYYY,
    INTERVAL_Q,
    INTERVAL_M,
    INTERVAL_Y,
    INTERVAL_D,
    INTERVAL_W,
    INTERVAL_WW,
    INTERVAL_H,
    INTERVAL_N,
    INTERVAL_S
};

struct IntervalInfo
{
    Interval    meInterval;
    char const * mStringCode;
    double      mdValue;
    bool        mbSimple;
};

}

static IntervalInfo const * getIntervalInfo( const OUString& rStringCode )
{
    static IntervalInfo const aIntervalTable[] =
    {
        { INTERVAL_YYYY, "yyyy", 0.0,           false }, // Year
        { INTERVAL_Q,    "q",    0.0,           false }, // Quarter
        { INTERVAL_M,    "m",    0.0,           false }, // Month
        { INTERVAL_Y,    "y",    1.0,           true  }, // Day of year
        { INTERVAL_D,    "d",    1.0,           true  }, // Day
        { INTERVAL_W,    "w",    1.0,           true  }, // Weekday
        { INTERVAL_WW,   "ww",   7.0,           true  }, // Week
        { INTERVAL_H,    "h",    1.0 /    24.0, true  }, // Hour
        { INTERVAL_N,    "n",    1.0 /  1440.0, true  }, // Minute
        { INTERVAL_S,    "s",    1.0 / 86400.0, true  }  // Second
    };
    for( std::size_t i = 0; i != SAL_N_ELEMENTS(aIntervalTable); ++i )
    {
        if( rStringCode.equalsIgnoreAsciiCaseAscii(
                aIntervalTable[i].mStringCode ) )
        {
            return &aIntervalTable[i];
        }
    }
    return nullptr;
}

static void implGetDayMonthYear( sal_Int16& rnYear, sal_Int16& rnMonth, sal_Int16& rnDay, double dDate )
{
    rnDay   = implGetDateDay( dDate );
    rnMonth = implGetDateMonth( dDate );
    rnYear  = implGetDateYear( dDate );
}

/** Limits a date to valid dates within tools' class Date capabilities.

    @return the year number, truncated if necessary and in that case also
            rMonth and rDay adjusted.
 */
static sal_Int16 limitDate( sal_Int32 n32Year, sal_Int16& rMonth, sal_Int16& rDay )
{
    if( n32Year > SAL_MAX_INT16 )
    {
        n32Year = SAL_MAX_INT16;
        rMonth = 12;
        rDay = 31;
    }
    else if( n32Year < SAL_MIN_INT16 )
    {
        n32Year = SAL_MIN_INT16;
        rMonth = 1;
        rDay = 1;
    }
    return static_cast<sal_Int16>(n32Year);
}

void SbRtl_DateAdd(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if( nParCount != 4 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    OUString aStringCode = rPar.Get(1)->GetOUString();
    IntervalInfo const * pInfo = getIntervalInfo( aStringCode );
    if( !pInfo )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    sal_Int32 lNumber = rPar.Get(2)->GetLong();
    double dDate = rPar.Get(3)->GetDate();
    double dNewDate = 0;
    if( pInfo->mbSimple )
    {
        double dAdd = pInfo->mdValue * lNumber;
        dNewDate = dDate + dAdd;
    }
    else
    {
        // Keep hours, minutes, seconds
        double dHoursMinutesSeconds = dDate - floor( dDate );

        bool bOk = true;
        sal_Int16 nYear, nMonth, nDay;
        sal_Int16 nTargetYear16 = 0, nTargetMonth = 0;
        implGetDayMonthYear( nYear, nMonth, nDay, dDate );
        switch( pInfo->meInterval )
        {
            case INTERVAL_YYYY:
            {
                sal_Int32 nTargetYear = lNumber + nYear;
                nTargetYear16 = limitDate( nTargetYear, nMonth, nDay );
                /* TODO: should the result be error if the date was limited? It never was. */
                nTargetMonth = nMonth;
                bOk = implDateSerial( nTargetYear16, nTargetMonth, nDay, false, SbDateCorrection::TruncateToMonth, dNewDate );
                break;
            }
            case INTERVAL_Q:
            case INTERVAL_M:
            {
                bool bNeg = (lNumber < 0);
                if( bNeg )
                    lNumber = -lNumber;
                sal_Int32 nYearsAdd;
                sal_Int16 nMonthAdd;
                if( pInfo->meInterval == INTERVAL_Q )
                {
                    nYearsAdd = lNumber / 4;
                    nMonthAdd = static_cast<sal_Int16>( 3 * (lNumber % 4) );
                }
                else
                {
                    nYearsAdd = lNumber / 12;
                    nMonthAdd = static_cast<sal_Int16>( lNumber % 12 );
                }

                sal_Int32 nTargetYear;
                if( bNeg )
                {
                    nTargetMonth = nMonth - nMonthAdd;
                    if( nTargetMonth <= 0 )
                    {
                        nTargetMonth += 12;
                        nYearsAdd++;
                    }
                    nTargetYear = static_cast<sal_Int32>(nYear) - nYearsAdd;
                }
                else
                {
                    nTargetMonth = nMonth + nMonthAdd;
                    if( nTargetMonth > 12 )
                    {
                        nTargetMonth -= 12;
                        nYearsAdd++;
                    }
                    nTargetYear = static_cast<sal_Int32>(nYear) + nYearsAdd;
                }
                nTargetYear16 = limitDate( nTargetYear, nTargetMonth, nDay );
                /* TODO: should the result be error if the date was limited? It never was. */
                bOk = implDateSerial( nTargetYear16, nTargetMonth, nDay, false, SbDateCorrection::TruncateToMonth, dNewDate );
                break;
            }
            default: break;
        }

        if( bOk )
            dNewDate += dHoursMinutesSeconds;
    }

    rPar.Get(0)->PutDate(dNewDate);
}

static double RoundImpl( double d )
{
    return ( d >= 0 ) ? floor( d + 0.5 ) : -floor( -d + 0.5 );
}

void SbRtl_DateDiff(StarBASIC *, SbxArray & rPar, bool)
{
    // DateDiff(interval, date1, date2[, firstdayofweek[, firstweekofyear]])

    sal_uInt32 nParCount = rPar.Count();
    if( nParCount < 4 || nParCount > 6 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    OUString aStringCode = rPar.Get(1)->GetOUString();
    IntervalInfo const * pInfo = getIntervalInfo( aStringCode );
    if( !pInfo )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    double dDate1 = rPar.Get(2)->GetDate();
    double dDate2 = rPar.Get(3)->GetDate();

    double dRet = 0.0;
    switch( pInfo->meInterval )
    {
        case INTERVAL_YYYY:
        {
            sal_Int16 nYear1 = implGetDateYear( dDate1 );
            sal_Int16 nYear2 = implGetDateYear( dDate2 );
            dRet = nYear2 - nYear1;
            break;
        }
        case INTERVAL_Q:
        {
            sal_Int16 nYear1 = implGetDateYear( dDate1 );
            sal_Int16 nYear2 = implGetDateYear( dDate2 );
            sal_Int16 nQ1 = 1 + (implGetDateMonth( dDate1 ) - 1) / 3;
            sal_Int16 nQ2 = 1 + (implGetDateMonth( dDate2 ) - 1) / 3;
            sal_Int16 nQGes1 = 4 * nYear1 + nQ1;
            sal_Int16 nQGes2 = 4 * nYear2 + nQ2;
            dRet = nQGes2 - nQGes1;
            break;
        }
        case INTERVAL_M:
        {
            sal_Int16 nYear1 = implGetDateYear( dDate1 );
            sal_Int16 nYear2 = implGetDateYear( dDate2 );
            sal_Int16 nMonth1 = implGetDateMonth( dDate1 );
            sal_Int16 nMonth2 = implGetDateMonth( dDate2 );
            sal_Int16 nMonthGes1 = 12 * nYear1 + nMonth1;
            sal_Int16 nMonthGes2 = 12 * nYear2 + nMonth2;
            dRet = nMonthGes2 - nMonthGes1;
            break;
        }
        case INTERVAL_Y:
        case INTERVAL_D:
        {
            double dDays1 = floor( dDate1 );
            double dDays2 = floor( dDate2 );
            dRet = dDays2 - dDays1;
            break;
        }
        case INTERVAL_W:
        case INTERVAL_WW:
        {
            double dDays1 = floor( dDate1 );
            double dDays2 = floor( dDate2 );
            if( pInfo->meInterval == INTERVAL_WW )
            {
                sal_Int16 nFirstDay = 1;    // Default
                if( nParCount >= 5 )
                {
                    nFirstDay = rPar.Get(4)->GetInteger();
                    if( nFirstDay < 0 || nFirstDay > 7 )
                    {
                        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                        return;
                    }
                    if( nFirstDay == 0 )
                    {
                        const Reference< XCalendar4 >& xCalendar = getLocaleCalendar();
                        if( !xCalendar.is() )
                        {
                            StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );
                            return;
                        }
                        nFirstDay = sal_Int16( xCalendar->getFirstDayOfWeek() + 1 );
                    }
                }
                sal_Int16 nDay1 = implGetWeekDay( dDate1 );
                sal_Int16 nDay1_Diff = nDay1 - nFirstDay;
                if( nDay1_Diff < 0 )
                    nDay1_Diff += 7;
                dDays1 -= nDay1_Diff;

                sal_Int16 nDay2 = implGetWeekDay( dDate2 );
                sal_Int16 nDay2_Diff = nDay2 - nFirstDay;
                if( nDay2_Diff < 0 )
                    nDay2_Diff += 7;
                dDays2 -= nDay2_Diff;
            }

            double dDiff = dDays2 - dDays1;
            dRet = ( dDiff >= 0 ) ? floor( dDiff / 7.0 ) : -floor( -dDiff / 7.0 );
            break;
        }
        case INTERVAL_H:
        {
            dRet = RoundImpl( 24.0 * (dDate2 - dDate1) );
            break;
        }
        case INTERVAL_N:
        {
            dRet = RoundImpl( 1440.0 * (dDate2 - dDate1) );
            break;
        }
        case INTERVAL_S:
        {
            dRet = RoundImpl( 86400.0 * (dDate2 - dDate1) );
            break;
        }
    }
    rPar.Get(0)->PutDouble(dRet);
}

static double implGetDateOfFirstDayInFirstWeek
    ( sal_Int16 nYear, sal_Int16& nFirstDay, sal_Int16& nFirstWeek, bool* pbError = nullptr )
{
    ErrCode nError = ERRCODE_NONE;
    if( nFirstDay < 0 || nFirstDay > 7 )
        nError = ERRCODE_BASIC_BAD_ARGUMENT;

    if( nFirstWeek < 0 || nFirstWeek > 3 )
        nError = ERRCODE_BASIC_BAD_ARGUMENT;

    Reference< XCalendar4 > xCalendar;
    if( nFirstDay == 0 || nFirstWeek == 0 )
    {
        xCalendar = getLocaleCalendar();
        if( !xCalendar.is() )
            nError = ERRCODE_BASIC_BAD_ARGUMENT;
    }

    if( nError != ERRCODE_NONE )
    {
        StarBASIC::Error( nError );
        if( pbError )
            *pbError = true;
        return 0.0;
    }

    if( nFirstDay == 0 )
        nFirstDay = sal_Int16( xCalendar->getFirstDayOfWeek() + 1 );

    sal_Int16 nFirstWeekMinDays = 0;    // Not used for vbFirstJan1 = default
    if( nFirstWeek == 0 )
    {
        nFirstWeekMinDays = xCalendar->getMinimumNumberOfDaysForFirstWeek();
        if( nFirstWeekMinDays == 1 )
        {
            nFirstWeekMinDays = 0;
            nFirstWeek = 1;
        }
        else if( nFirstWeekMinDays == 4 )
            nFirstWeek = 2;
        else if( nFirstWeekMinDays == 7 )
            nFirstWeek = 3;
    }
    else if( nFirstWeek == 2 )
        nFirstWeekMinDays = 4;      // vbFirstFourDays
    else if( nFirstWeek == 3 )
        nFirstWeekMinDays = 7;      // vbFirstFourDays

    double dBaseDate;
    implDateSerial( nYear, 1, 1, false, SbDateCorrection::None, dBaseDate );

    sal_Int16 nWeekDay0101 = implGetWeekDay( dBaseDate );
    sal_Int16 nDayDiff = nWeekDay0101 - nFirstDay;
    if( nDayDiff < 0 )
        nDayDiff += 7;

    if( nFirstWeekMinDays )
    {
        sal_Int16 nThisWeeksDaysInYearCount = 7 - nDayDiff;
        if( nThisWeeksDaysInYearCount < nFirstWeekMinDays )
            nDayDiff -= 7;
    }
    double dRetDate = dBaseDate - nDayDiff;
    return dRetDate;
}

void SbRtl_DatePart(StarBASIC *, SbxArray & rPar, bool)
{
    // DatePart(interval, date[,firstdayofweek[, firstweekofyear]])

    sal_uInt32 nParCount = rPar.Count();
    if( nParCount < 3 || nParCount > 5 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    OUString aStringCode = rPar.Get(1)->GetOUString();
    IntervalInfo const * pInfo = getIntervalInfo( aStringCode );
    if( !pInfo )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    double dDate = rPar.Get(2)->GetDate();

    sal_Int32 nRet = 0;
    switch( pInfo->meInterval )
    {
        case INTERVAL_YYYY:
        {
            nRet = implGetDateYear( dDate );
            break;
        }
        case INTERVAL_Q:
        {
            nRet = 1 + (implGetDateMonth( dDate ) - 1) / 3;
            break;
        }
        case INTERVAL_M:
        {
            nRet = implGetDateMonth( dDate );
            break;
        }
        case INTERVAL_Y:
        {
            sal_Int16 nYear = implGetDateYear( dDate );
            double dBaseDate;
            implDateSerial( nYear, 1, 1, false, SbDateCorrection::None, dBaseDate );
            nRet = 1 + sal_Int32( dDate - dBaseDate );
            break;
        }
        case INTERVAL_D:
        {
            nRet = implGetDateDay( dDate );
            break;
        }
        case INTERVAL_W:
        {
            bool bFirstDay = false;
            sal_Int16 nFirstDay = 1;    // Default
            if( nParCount >= 4 )
            {
                nFirstDay = rPar.Get(3)->GetInteger();
                bFirstDay = true;
            }
            nRet = implGetWeekDay( dDate, bFirstDay, nFirstDay );
            break;
        }
        case INTERVAL_WW:
        {
            sal_Int16 nFirstDay = 1;    // Default
            if( nParCount >= 4 )
                nFirstDay = rPar.Get(3)->GetInteger();

            sal_Int16 nFirstWeek = 1;   // Default
            if( nParCount == 5 )
                nFirstWeek = rPar.Get(4)->GetInteger();

            sal_Int16 nYear = implGetDateYear( dDate );
            bool bError = false;
            double dYearFirstDay = implGetDateOfFirstDayInFirstWeek( nYear, nFirstDay, nFirstWeek, &bError );
            if( !bError )
            {
                if( dYearFirstDay > dDate )
                {
                    // Date belongs to last year's week
                    dYearFirstDay = implGetDateOfFirstDayInFirstWeek( nYear - 1, nFirstDay, nFirstWeek );
                }
                else if( nFirstWeek != 1 )
                {
                    // Check if date belongs to next year
                    double dNextYearFirstDay = implGetDateOfFirstDayInFirstWeek( nYear + 1, nFirstDay, nFirstWeek );
                    if( dDate >= dNextYearFirstDay )
                        dYearFirstDay = dNextYearFirstDay;
                }

                // Calculate week
                double dDiff = dDate - dYearFirstDay;
                nRet = 1 + sal_Int32( dDiff / 7 );
            }
            break;
        }
        case INTERVAL_H:
        {
            nRet = implGetHour( dDate );
            break;
        }
        case INTERVAL_N:
        {
            nRet = implGetMinute( dDate );
            break;
        }
        case INTERVAL_S:
        {
            nRet = implGetSecond( dDate );
            break;
        }
    }
    rPar.Get(0)->PutLong(nRet);
}

// FormatDateTime(Date[,NamedFormat])
void SbRtl_FormatDateTime(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if( nParCount < 2 || nParCount > 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    double dDate = rPar.Get(1)->GetDate();
    sal_Int16 nNamedFormat = 0;
    if( nParCount > 2 )
    {
        nNamedFormat = rPar.Get(2)->GetInteger();
        if( nNamedFormat < 0 || nNamedFormat > 4 )
        {
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
            return;
        }
    }

    const Reference< XCalendar4 >& xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );
        return;
    }

    OUString aRetStr;
    SbxVariableRef pSbxVar = new SbxVariable( SbxSTRING );
    switch( nNamedFormat )
    {
        // GeneralDate:
        // Display a date and/or time. If there is a date part,
        // display it as a short date. If there is a time part,
        // display it as a long time. If present, both parts are displayed.

        // 12/21/2004 11:24:50 AM
        // 21.12.2004 12:13:51
    case 0:
        pSbxVar->PutDate( dDate );
        aRetStr = pSbxVar->GetOUString();
        break;

        // LongDate: Display a date using the long date format specified
        // in your computer's regional settings.
        // Tuesday, December 21, 2004
        // Dienstag, 21. December 2004
    case 1:
        {
            std::shared_ptr<SvNumberFormatter> pFormatter;
            if( GetSbData()->pInst )
            {
                pFormatter = GetSbData()->pInst->GetNumberFormatter();
            }
            else
            {
                sal_uInt32 n;   // Dummy
                pFormatter = SbiInstance::PrepareNumberFormatter( n, n, n );
            }

            LanguageType eLangType = Application::GetSettings().GetLanguageTag().getLanguageType();
            const sal_uInt32 nIndex = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_LONG, eLangType );
            const Color* pCol;
            pFormatter->GetOutputString( dDate, nIndex, aRetStr, &pCol );
            break;
        }

        // ShortDate: Display a date using the short date format specified
        // in your computer's regional settings.
        // 21.12.2004
    case 2:
        pSbxVar->PutDate( floor(dDate) );
        aRetStr = pSbxVar->GetOUString();
        break;

        // LongTime: Display a time using the time format specified
        // in your computer's regional settings.
        // 11:24:50 AM
        // 12:13:51
    case 3:
        // ShortTime: Display a time using the 24-hour format (hh:mm).
        // 11:24
    case 4:
        double dTime = modf( dDate, &o3tl::temporary(double()) );
        pSbxVar->PutDate( dTime );
        if( nNamedFormat == 3 )
        {
            aRetStr = pSbxVar->GetOUString();
        }
        else
        {
            aRetStr = pSbxVar->GetOUString().copy( 0, 5 );
        }
        break;
    }

    rPar.Get(0)->PutString(aRetStr);
}

void SbRtl_Frac(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if( nParCount != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    SbxVariable* pSbxVariable = rPar.Get(1);
    double dVal = pSbxVariable->GetDouble();
    if(dVal >= 0)
        rPar.Get(0)->PutDouble(dVal - ::rtl::math::approxFloor(dVal));
    else
        rPar.Get(0)->PutDouble(dVal - ::rtl::math::approxCeil(dVal));
}

void SbRtl_Round(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nParCount = rPar.Count();
    if( nParCount != 2 && nParCount != 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    SbxVariable* pSbxVariable = rPar.Get(1);
    double dVal = pSbxVariable->GetDouble();
    double dRes = 0.0;
    if( dVal != 0.0 )
    {
        sal_Int16 numdecimalplaces = 0;
        if( nParCount == 3 )
        {
            numdecimalplaces = rPar.Get(2)->GetInteger();
            if( numdecimalplaces < 0 || numdecimalplaces > 22 )
            {
                StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
                return;
            }
        }

        dRes = rtl_math_round(dVal, numdecimalplaces, rtl_math_RoundingMode_HalfEven);
    }
    rPar.Get(0)->PutDouble(dRes);
}

static void CallFunctionAccessFunction( const Sequence< Any >& aArgs, const OUString& sFuncName, SbxVariable* pRet )
{
    static Reference< XFunctionAccess > xFunc;
    try
    {
        if ( !xFunc.is() )
        {
            Reference< XMultiServiceFactory > xFactory( getProcessServiceFactory() );
            if( xFactory.is() )
            {
                xFunc.set( xFactory->createInstance("com.sun.star.sheet.FunctionAccess"), UNO_QUERY_THROW);
            }
        }
        Any aRet = xFunc->callFunction( sFuncName, aArgs );

        unoToSbxValue( pRet, aRet );

    }
    catch(const Exception& )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
    }
}

void SbRtl_SYD(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 4 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // retrieve non-optional params

    Sequence< Any > aParams
    {
        makeAny(rPar.Get(1)->GetDouble()),
        makeAny(rPar.Get(2)->GetDouble()),
        makeAny(rPar.Get(3)->GetDouble()),
        makeAny(rPar.Get(4)->GetDouble())
    };

    CallFunctionAccessFunction(aParams, "SYD", rPar.Get(0));
}

void SbRtl_SLN(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // retrieve non-optional params

    Sequence< Any > aParams
    {
        makeAny(rPar.Get(1)->GetDouble()),
        makeAny(rPar.Get(2)->GetDouble()),
        makeAny(rPar.Get(3)->GetDouble())
    };

    CallFunctionAccessFunction(aParams, "SLN", rPar.Get(0));
}

void SbRtl_Pmt(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 3 || nArgCount > 5 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params

    double rate = rPar.Get(1)->GetDouble();
    double nper = rPar.Get(2)->GetDouble();
    double pmt = rPar.Get(3)->GetDouble();

    // set default values for Optional args
    double fv = 0;
    double type = 0;

    // fv
    if ( nArgCount >= 4 )
    {
        if (rPar.Get(4)->GetType() != SbxEMPTY)
            fv = rPar.Get(4)->GetDouble();
    }
    // type
    if ( nArgCount >= 5 )
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
            type = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams
    {
        makeAny(rate),
        makeAny(nper),
        makeAny(pmt),
        makeAny(fv),
        makeAny(type)
    };

    CallFunctionAccessFunction(aParams, "Pmt", rPar.Get(0));
}

void SbRtl_PPmt(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 4 || nArgCount > 6 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params

    double rate = rPar.Get(1)->GetDouble();
    double per = rPar.Get(2)->GetDouble();
    double nper = rPar.Get(3)->GetDouble();
    double pv = rPar.Get(4)->GetDouble();

    // set default values for Optional args
    double fv = 0;
    double type = 0;

    // fv
    if ( nArgCount >= 5 )
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
            fv = rPar.Get(5)->GetDouble();
    }
    // type
    if ( nArgCount >= 6 )
    {
        if (rPar.Get(6)->GetType() != SbxEMPTY)
            type = rPar.Get(6)->GetDouble();
    }

    Sequence< Any > aParams
    {
        makeAny(rate),
        makeAny(per),
        makeAny(nper),
        makeAny(pv),
        makeAny(fv),
        makeAny(type)
    };

    CallFunctionAccessFunction(aParams, "PPmt", rPar.Get(0));
}

void SbRtl_PV(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 3 || nArgCount > 5 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params

    double rate = rPar.Get(1)->GetDouble();
    double nper = rPar.Get(2)->GetDouble();
    double pmt = rPar.Get(3)->GetDouble();

    // set default values for Optional args
    double fv = 0;
    double type = 0;

    // fv
    if ( nArgCount >= 4 )
    {
        if (rPar.Get(4)->GetType() != SbxEMPTY)
            fv = rPar.Get(4)->GetDouble();
    }
    // type
    if ( nArgCount >= 5 )
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
            type = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams
    {
        makeAny(rate),
        makeAny(nper),
        makeAny(pmt),
        makeAny(fv),
        makeAny(type)
    };

    CallFunctionAccessFunction(aParams, "PV", rPar.Get(0));
}

void SbRtl_NPV(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 1 || nArgCount > 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    Any aValues = sbxToUnoValue(rPar.Get(2),
                cppu::UnoType<Sequence<double>>::get() );

    // convert for calc functions
    Sequence< Sequence< double > > sValues(1);
    aValues >>= sValues.getArray()[ 0 ];
    aValues <<= sValues;

    Sequence< Any > aParams
    {
        makeAny(rPar.Get(1)->GetDouble()),
        aValues
    };

    CallFunctionAccessFunction(aParams, "NPV", rPar.Get(0));
}

void SbRtl_NPer(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 3 || nArgCount > 5 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params

    double rate = rPar.Get(1)->GetDouble();
    double pmt = rPar.Get(2)->GetDouble();
    double pv = rPar.Get(3)->GetDouble();

    // set default values for Optional args
    double fv = 0;
    double type = 0;

    // fv
    if ( nArgCount >= 4 )
    {
        if (rPar.Get(4)->GetType() != SbxEMPTY)
            fv = rPar.Get(4)->GetDouble();
    }
    // type
    if ( nArgCount >= 5 )
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
            type = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams
    {
        makeAny(rate),
        makeAny(pmt),
        makeAny(pv),
        makeAny(fv),
        makeAny(type)
    };

    CallFunctionAccessFunction(aParams, "NPer", rPar.Get(0));
}

void SbRtl_MIRR(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 3 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    // retrieve non-optional params

    Any aValues = sbxToUnoValue(rPar.Get(1),
                cppu::UnoType<Sequence<double>>::get() );

    // convert for calc functions
    Sequence< Sequence< double > > sValues(1);
    aValues >>= sValues.getArray()[ 0 ];
    aValues <<= sValues;

    Sequence< Any > aParams
    {
        aValues,
        makeAny(rPar.Get(2)->GetDouble()),
        makeAny(rPar.Get(3)->GetDouble())
    };

    CallFunctionAccessFunction(aParams, "MIRR", rPar.Get(0));
}

void SbRtl_IRR(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 1 || nArgCount > 2 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params
    Any aValues = sbxToUnoValue(rPar.Get(1),
                cppu::UnoType<Sequence<double>>::get() );

    // convert for calc functions
    Sequence< Sequence< double > > sValues(1);
    aValues >>= sValues.getArray()[ 0 ];
    aValues <<= sValues;

    // set default values for Optional args
    double guess = 0.1;
    // guess
    if ( nArgCount >= 2 )
    {
        if (rPar.Get(2)->GetType() != SbxEMPTY)
            guess = rPar.Get(2)->GetDouble();
    }

    Sequence< Any > aParams
    {
        aValues,
        makeAny(guess)
    };

    CallFunctionAccessFunction(aParams, "IRR", rPar.Get(0));
}

void SbRtl_IPmt(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 4 || nArgCount > 6 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params

    double rate = rPar.Get(1)->GetDouble();
    double per = rPar.Get(2)->GetInteger();
    double nper = rPar.Get(3)->GetDouble();
    double pv = rPar.Get(4)->GetDouble();

    // set default values for Optional args
    double fv = 0;
    double type = 0;

    // fv
    if ( nArgCount >= 5 )
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
            fv = rPar.Get(5)->GetDouble();
    }
    // type
    if ( nArgCount >= 6 )
    {
        if (rPar.Get(6)->GetType() != SbxEMPTY)
            type = rPar.Get(6)->GetDouble();
    }

    Sequence< Any > aParams
    {
        makeAny(rate),
        makeAny(per),
        makeAny(nper),
        makeAny(pv),
        makeAny(fv),
        makeAny(type)
    };

    CallFunctionAccessFunction(aParams, "IPmt", rPar.Get(0));
}

void SbRtl_FV(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 3 || nArgCount > 5 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params

    double rate = rPar.Get(1)->GetDouble();
    double nper = rPar.Get(2)->GetDouble();
    double pmt = rPar.Get(3)->GetDouble();

    // set default values for Optional args
    double pv = 0;
    double type = 0;

    // pv
    if ( nArgCount >= 4 )
    {
        if (rPar.Get(4)->GetType() != SbxEMPTY)
            pv = rPar.Get(4)->GetDouble();
    }
    // type
    if ( nArgCount >= 5 )
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
            type = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams
    {
        makeAny(rate),
        makeAny(nper),
        makeAny(pmt),
        makeAny(pv),
        makeAny(type)
    };

    CallFunctionAccessFunction(aParams, "FV", rPar.Get(0));
}

void SbRtl_DDB(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 4 || nArgCount > 5 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params

    double cost = rPar.Get(1)->GetDouble();
    double salvage = rPar.Get(2)->GetDouble();
    double life = rPar.Get(3)->GetDouble();
    double period = rPar.Get(4)->GetDouble();

    // set default values for Optional args
    double factor = 2;

    // factor
    if ( nArgCount >= 5 )
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
            factor = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams
    {
        makeAny(cost),
        makeAny(salvage),
        makeAny(life),
        makeAny(period),
        makeAny(factor)
    };

    CallFunctionAccessFunction(aParams, "DDB", rPar.Get(0));
}

void SbRtl_Rate(StarBASIC *, SbxArray & rPar, bool)
{
    sal_uInt32 nArgCount = rPar.Count() - 1;

    if ( nArgCount < 3 || nArgCount > 6 )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }
    // retrieve non-optional params

    double nper = 0;
    double pmt = 0;
    double pv = 0;

    nper = rPar.Get(1)->GetDouble();
    pmt = rPar.Get(2)->GetDouble();
    pv = rPar.Get(3)->GetDouble();

    // set default values for Optional args
    double fv = 0;
    double type = 0;
    double guess = 0.1;

    // fv
    if ( nArgCount >= 4 )
    {
        if (rPar.Get(4)->GetType() != SbxEMPTY)
            fv = rPar.Get(4)->GetDouble();
    }

    // type
    if ( nArgCount >= 5 )
    {
        if (rPar.Get(5)->GetType() != SbxEMPTY)
            type = rPar.Get(5)->GetDouble();
    }

    // guess
    if ( nArgCount >= 6 )
    {
        if (rPar.Get(6)->GetType() != SbxEMPTY)
            guess = rPar.Get(6)->GetDouble();
    }

    Sequence< Any > aParams
    {
        makeAny(nper),
        makeAny(pmt),
        makeAny(pv),
        makeAny(fv),
        makeAny(type),
        makeAny(guess)
    };

    CallFunctionAccessFunction(aParams, "Rate", rPar.Get(0));
}

void SbRtl_StrReverse(StarBASIC *, SbxArray & rPar, bool)
{
    if (rPar.Count() != 2)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    SbxVariable* pSbxVariable = rPar.Get(1);
    if( pSbxVariable->IsNull() )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    OUString aStr = comphelper::string::reverseString(pSbxVariable->GetOUString());
    rPar.Get(0)->PutString(aStr);
}

void SbRtl_CompatibilityMode(StarBASIC *, SbxArray & rPar, bool)
{
    bool bEnabled = false;
    sal_uInt32 nCount = rPar.Count();
    if ( nCount != 1 && nCount != 2 )
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );

    SbiInstance* pInst = GetSbData()->pInst;
    if( pInst )
    {
        if ( nCount == 2 )
        {
            pInst->EnableCompatibility(rPar.Get(1)->GetBool());
        }
        bEnabled = pInst->IsCompatibility();
    }
    rPar.Get(0)->PutBool(bEnabled);
}

void SbRtl_Input(StarBASIC *, SbxArray & rPar, bool)
{
    // 2 parameters needed
    if (rPar.Count() < 3)
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
        return;
    }

    sal_uInt16 nByteCount = rPar.Get(1)->GetUShort();
    sal_Int16 nFileNumber = rPar.Get(2)->GetInteger();

    SbiIoSystem* pIosys = GetSbData()->pInst->GetIoSystem();
    SbiStream* pSbStrm = pIosys->GetStream( nFileNumber );
    if ( !pSbStrm || !(pSbStrm->GetMode() & (SbiStreamFlags::Binary | SbiStreamFlags::Input)) )
    {
        StarBASIC::Error( ERRCODE_BASIC_BAD_CHANNEL );
        return;
    }

    OString aByteBuffer;
    ErrCode err = pSbStrm->Read( aByteBuffer, nByteCount, true );
    if( !err )
        err = pIosys->GetError();

    if( err )
    {
        StarBASIC::Error( err );
        return;
    }
    rPar.Get(0)->PutString(OStringToOUString(aByteBuffer, osl_getThreadTextEncoding()));
}

void SbRtl_Me(StarBASIC *, SbxArray & rPar, bool)
{
    SbModule* pActiveModule = GetSbData()->pInst->GetActiveModule();
    SbClassModuleObject* pClassModuleObject = dynamic_cast<SbClassModuleObject*>( pActiveModule );
    SbxVariableRef refVar = rPar.Get(0);
    if( pClassModuleObject == nullptr )
    {
        SbObjModule* pMod = dynamic_cast<SbObjModule*>( pActiveModule );
        if ( pMod )
            refVar->PutObject( pMod );
        else
            StarBASIC::Error( ERRCODE_BASIC_INVALID_USAGE_OBJECT );
    }
    else
        refVar->PutObject( pClassModuleObject );
}

#endif

bool LibreOffice6FloatingPointMode()
{
    static bool bMode = std::getenv("LIBREOFFICE6FLOATINGPOINTMODE") != nullptr;

    return bMode || officecfg::Office::Scripting::Basic::Compatibility::UseLibreOffice6FloatingPointConversion::get();
}

sal_Int16 implGetWeekDay( double aDate, bool bFirstDayParam, sal_Int16 nFirstDay )
{
    Date aRefDate( 1,1,1900 );
    sal_Int32 nDays = static_cast<sal_Int32>(aDate);
    nDays -= 2; // normalize: 1.1.1900 => 0
    aRefDate.AddDays( nDays);
    DayOfWeek aDay = aRefDate.GetDayOfWeek();
    sal_Int16 nDay;
    if ( aDay != SUNDAY )
        nDay = static_cast<sal_Int16>(aDay) + 2;
    else
        nDay = 1;   // 1 == Sunday

    // #117253 optional 2nd parameter "firstdayofweek"
    if( bFirstDayParam )
    {
        if( nFirstDay < 0 || nFirstDay > 7 )
        {
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::Error( ERRCODE_BASIC_BAD_ARGUMENT );
#endif
            return 0;
        }
        if( nFirstDay == 0 )
        {
            const Reference< XCalendar4 >& xCalendar = getLocaleCalendar();
            if( !xCalendar.is() )
            {
#if HAVE_FEATURE_SCRIPTING
                StarBASIC::Error( ERRCODE_BASIC_INTERNAL_ERROR );
#endif
                return 0;
            }
            nFirstDay = sal_Int16( xCalendar->getFirstDayOfWeek() + 1 );
        }
        nDay = 1 + (nDay + 7 - nFirstDay) % 7;
    }
    return nDay;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
