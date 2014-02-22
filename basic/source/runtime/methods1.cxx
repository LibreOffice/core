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

#include <sal/config.h>

#include <cstddef>

#include <stdlib.h>
#include <vcl/svapp.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <basic/sbxvar.hxx>
#include <basic/sbx.hxx>
#include <svl/zforlist.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <vcl/jobset.hxx>
#include "sbobjmod.hxx"
#include <basic/sbuno.hxx>

#include "date.hxx"
#include "sbintern.hxx"
#include "runtime.hxx"
#include "stdobj.hxx"
#include "rtlproto.hxx"
#include "dllmgr.hxx"
#include <iosys.hxx>
#include "sbunoobj.hxx"
#include "propacc.hxx"
#include <sal/log.hxx>


#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/LocaleCalendar.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>

using namespace comphelper;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;
using namespace com::sun::star::sheet;
using namespace com::sun::star::uno;

static Reference< XCalendar3 > getLocaleCalendar( void )
{
    static Reference< XCalendar3 > xCalendar;
    if( !xCalendar.is() )
    {
        Reference< XComponentContext > xContext = getProcessComponentContext();
        xCalendar = LocaleCalendar::create(xContext);
    }

    static com::sun::star::lang::Locale aLastLocale;
    static bool bNeedsInit = true;

    com::sun::star::lang::Locale aLocale = Application::GetSettings().GetLanguageTag().getLocale();
    bool bNeedsReload = false;
    if( bNeedsInit )
    {
        bNeedsInit = false;
        bNeedsReload = true;
    }
    else if( aLocale.Language != aLastLocale.Language ||
             aLocale.Country  != aLastLocale.Country ||
             aLocale.Variant  != aLastLocale.Variant )
    {
        bNeedsReload = true;
    }
    if( bNeedsReload )
    {
        aLastLocale = aLocale;
        xCalendar->loadDefaultCalendar( aLocale );
    }
    return xCalendar;
}

#ifndef DISABLE_SCRIPTING

RTLFUNC(CallByName)
{
    (void)pBasic;
    (void)bWrite;

    const sal_Int16 vbGet       = 2;
    const sal_Int16 vbLet       = 4;
    const sal_Int16 vbMethod    = 1;
    const sal_Int16 vbSet       = 8;

    
    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount < 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    
    SbxBase* pObjVar = (SbxObject*)rPar.Get(1)->GetObject();
    SbxObject* pObj = NULL;
    if( pObjVar )
        pObj = PTR_CAST(SbxObject,pObjVar);
    if( !pObj && pObjVar && pObjVar->ISA(SbxVariable) )
    {
        SbxBase* pObjVarObj = ((SbxVariable*)pObjVar)->GetObject();
        pObj = PTR_CAST(SbxObject,pObjVarObj);
    }
    if( !pObj )
    {
        StarBASIC::Error( SbERR_BAD_PARAMETER );
        return;
    }

    
    OUString aNameStr = rPar.Get(2)->GetOUString();

    
    sal_Int16 nCallType = rPar.Get(3)->GetInteger();

    
    SbxVariable* pFindVar = pObj->Find( aNameStr, SbxCLASS_DONTCARE );
    if( pFindVar == NULL )
    {
        StarBASIC::Error( SbERR_PROC_UNDEFINED );
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
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
                SbiRuntime* pRT = pInst ? pInst->pRun : NULL;
                if( pRT != NULL )
                {
                    pRT->StepSET_Impl( pValVar, rFindVar, false );
                }
            }
        }
        break;
    case vbMethod:
        {
            SbMethod* pMeth = PTR_CAST(SbMethod,pFindVar);
            if( pMeth == NULL )
            {
                StarBASIC::Error( SbERR_PROC_UNDEFINED );
                return;
            }

            
            SbxArrayRef xArray;
            sal_uInt16 nMethParamCount = nParCount - 4;
            if( nMethParamCount > 0 )
            {
                xArray = new SbxArray;
                for( sal_uInt16 i = 0 ; i < nMethParamCount ; i++ )
                {
                    SbxVariable* pPar = rPar.Get( i + 4 );
                    xArray->Put( pPar, i + 1 );
                }
            }

            
            SbxVariableRef refVar = rPar.Get(0);
            if( xArray.Is() )
                pMeth->SetParameters( xArray );
            pMeth->Call( refVar );
            pMeth->SetParameters( NULL );
        }
        break;
    default:
        StarBASIC::Error( SbERR_PROC_UNDEFINED );
    }
}

RTLFUNC(CBool) 
{
    (void)pBasic;
    (void)bWrite;

    sal_Bool bVal = sal_False;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        bVal = pSbxVariable->GetBool();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutBool(bVal);
}

RTLFUNC(CByte) 
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt8 nByte = 0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nByte = pSbxVariable->GetByte();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutByte(nByte);
}

RTLFUNC(CCur)
{
    (void)pBasic;
    (void)bWrite;

    sal_Int64 nCur = 0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nCur = pSbxVariable->GetCurrency();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutCurrency( nCur );
}

RTLFUNC(CDec)
{
    (void)pBasic;
    (void)bWrite;

#ifdef WNT
    SbxDecimal* pDec = NULL;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        pDec = pSbxVariable->GetDecimal();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutDecimal( pDec );
#else
    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
#endif
}

RTLFUNC(CDate) 
{
    (void)pBasic;
    (void)bWrite;

    double nVal = 0.0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetDate();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutDate(nVal);
}

RTLFUNC(CDbl)  
{
    (void)pBasic;
    (void)bWrite;

    double nVal = 0.0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        if( pSbxVariable->GetType() == SbxSTRING )
        {
            
            OUString aScanStr = pSbxVariable->GetOUString();
            SbError Error = SbxValue::ScanNumIntnl( aScanStr, nVal );
            if( Error != SbxERR_OK )
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }

    rPar.Get(0)->PutDouble(nVal);
}

RTLFUNC(CInt)  
{
    (void)pBasic;
    (void)bWrite;

    sal_Int16 nVal = 0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetInteger();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutInteger(nVal);
}

RTLFUNC(CLng)  
{
    (void)pBasic;
    (void)bWrite;

    sal_Int32 nVal = 0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetLong();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutLong(nVal);
}

RTLFUNC(CSng)  
{
    (void)pBasic;
    (void)bWrite;

    float nVal = (float)0.0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        if( pSbxVariable->GetType() == SbxSTRING )
        {
            
            double dVal = 0.0;
            OUString aScanStr = pSbxVariable->GetOUString();
            SbError Error = SbxValue::ScanNumIntnl( aScanStr, dVal, /*bSingle=*/true );
            if( SbxBase::GetError() == SbxERR_OK && Error != SbxERR_OK )
            {
                StarBASIC::Error( Error );
            }
            nVal = (float)dVal;
        }
        else
        {
            nVal = pSbxVariable->GetSingle();
        }
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutSingle(nVal);
}

RTLFUNC(CStr)  
{
    (void)pBasic;
    (void)bWrite;

    OUString aString;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        aString = pSbxVariable->GetOUString();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutString(aString);
}

RTLFUNC(CVar)  
{
    (void)pBasic;
    (void)bWrite;

    SbxValues aVals( SbxVARIANT );
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        pSbxVariable->Get( aVals );
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->Put( aVals );
}

RTLFUNC(CVErr)
{
    (void)pBasic;
    (void)bWrite;

    sal_Int16 nErrCode = 0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nErrCode = pSbxVariable->GetInteger();
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    rPar.Get(0)->PutErr( nErrCode );
}

RTLFUNC(Iif) 
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 4 )
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
}

RTLFUNC(GetSystemType)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    else
    {
        
        rPar.Get(0)->PutInteger( -1 );
    }
}

RTLFUNC(GetGUIType)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    else
    {
        
#if defined (WNT)
        rPar.Get(0)->PutInteger( 1 );
#elif defined UNX
        rPar.Get(0)->PutInteger( 4 );
#else
        rPar.Get(0)->PutInteger( -1 );
#endif
    }
}

RTLFUNC(Red)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    else
    {
        sal_uIntPtr nRGB = (sal_uIntPtr)rPar.Get(1)->GetLong();
        nRGB &= 0x00FF0000;
        nRGB >>= 16;
        rPar.Get(0)->PutInteger( (sal_Int16)nRGB );
    }
}

RTLFUNC(Green)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    else
    {
        sal_uIntPtr nRGB = (sal_uIntPtr)rPar.Get(1)->GetLong();
        nRGB &= 0x0000FF00;
        nRGB >>= 8;
        rPar.Get(0)->PutInteger( (sal_Int16)nRGB );
    }
}

RTLFUNC(Blue)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    else
    {
        sal_uIntPtr nRGB = (sal_uIntPtr)rPar.Get(1)->GetLong();
        nRGB &= 0x000000FF;
        rPar.Get(0)->PutInteger( (sal_Int16)nRGB );
    }
}


RTLFUNC(Switch)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nCount = rPar.Count();
    if( !(nCount & 0x0001 ))
    {
        
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    sal_uInt16 nCurExpr = 1;
    while( nCurExpr < (nCount-1) )
    {
        if( rPar.Get( nCurExpr )->GetBool())
        {
            (*rPar.Get(0)) = *(rPar.Get(nCurExpr+1));
            return;
        }
        nCurExpr += 2;
    }
    rPar.Get(0)->PutNull();
}



void Wait_Impl( bool bDurationBased, SbxArray& rPar )
{
    if( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    long nWait = 0;
    if ( bDurationBased )
    {
        double dWait = rPar.Get(1)->GetDouble();
        double dNow = Now_Impl();
         double dSecs = (double)( ( dWait - dNow ) * (double)( 24.0*3600.0) );
        nWait = (long)( dSecs * 1000 ); 
    }
    else
    {
        nWait = rPar.Get(1)->GetLong();
    }
    if( nWait < 0 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    Timer aTimer;
    aTimer.SetTimeout( nWait );
    aTimer.Start();
    while ( aTimer.IsActive() )
    {
        Application::Yield();
    }
}


RTLFUNC(Wait)
{
    (void)pBasic;
    (void)bWrite;
    Wait_Impl( false, rPar );
}



RTLFUNC(WaitUntil)
{
    (void)pBasic;
    (void)bWrite;
    Wait_Impl( true, rPar );
}

RTLFUNC(DoEvents)
{
    (void)pBasic;
    (void)bWrite;
    (void)rPar;



    
    rPar.Get(0)->PutInteger( 0 );
    Application::Reschedule( true );
}

RTLFUNC(GetGUIVersion)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    else
    {
        
        rPar.Get(0)->PutLong( -1 );
    }
}

RTLFUNC(Choose)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    sal_Int16 nIndex = rPar.Get(1)->GetInteger();
    sal_uInt16 nCount = rPar.Count();
    nCount--;
    if( nCount == 1 || nIndex > (nCount-1) || nIndex < 1 )
    {
        rPar.Get(0)->PutNull();
        return;
    }
    (*rPar.Get(0)) = *(rPar.Get(nIndex+1));
}


RTLFUNC(Trim)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    else
    {
        OUString aStr(comphelper::string::strip(rPar.Get(1)->GetOUString(), ' '));
        rPar.Get(0)->PutString(aStr);
    }
}

RTLFUNC(GetSolarVersion)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutLong( (sal_Int32)SUPD );
}

RTLFUNC(TwipsPerPixelX)
{
    (void)pBasic;
    (void)bWrite;

    sal_Int32 nResult = 0;
    Size aSize( 100,0 );
    MapMode aMap( MAP_TWIP );
    OutputDevice* pDevice = Application::GetDefaultDevice();
    if( pDevice )
    {
        aSize = pDevice->PixelToLogic( aSize, aMap );
        nResult = aSize.Width() / 100;
    }
    rPar.Get(0)->PutLong( nResult );
}

RTLFUNC(TwipsPerPixelY)
{
    (void)pBasic;
    (void)bWrite;

    sal_Int32 nResult = 0;
    Size aSize( 0,100 );
    MapMode aMap( MAP_TWIP );
    OutputDevice* pDevice = Application::GetDefaultDevice();
    if( pDevice )
    {
        aSize = pDevice->PixelToLogic( aSize, aMap );
        nResult = aSize.Height() / 100;
    }
    rPar.Get(0)->PutLong( nResult );
}


RTLFUNC(FreeLibrary)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
    GetSbData()->pInst->GetDllMgr()->FreeDll( rPar.Get(1)->GetOUString() );
}
bool IsBaseIndexOne()
{
    bool result = false;
    if ( GetSbData()->pInst && GetSbData()->pInst->pRun )
    {
        sal_uInt16 res = GetSbData()->pInst->pRun->GetBase();
        if ( res )
        {
            result = true;
        }
    }
    return result;
}

RTLFUNC(Array)
{
    (void)pBasic;
    (void)bWrite;

    SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
    sal_uInt16 nArraySize = rPar.Count() - 1;

    
    bool bIncIndex = (IsBaseIndexOne() && SbiRuntime::isVBAEnabled() );
    if( nArraySize )
    {
        if ( bIncIndex )
        {
            pArray->AddDim( 1, nArraySize );
        }
        else
        {
            pArray->AddDim( 0, nArraySize-1 );
        }
    }
    else
    {
        pArray->unoAddDim( 0, -1 );
    }

    
    
    
    
    for( sal_uInt16 i = 0 ; i < nArraySize ; i++ )
    {
        SbxVariable* pVar = rPar.Get(i+1);
        SbxVariable* pNew = new SbxVariable( *pVar );
        pNew->SetFlag( SBX_WRITE );
        short index = static_cast< short >(i);
        if ( bIncIndex )
        {
            ++index;
        }
        pArray->Put( pNew, &index );
    }

    
    SbxVariableRef refVar = rPar.Get(0);
    sal_uInt16 nFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( NULL );
}









RTLFUNC(DimArray)
{
    (void)pBasic;
    (void)bWrite;

    SbxDimArray * pArray = new SbxDimArray( SbxVARIANT );
    sal_uInt16 nArrayDims = rPar.Count() - 1;
    if( nArrayDims > 0 )
    {
        for( sal_uInt16 i = 0; i < nArrayDims ; i++ )
        {
            sal_Int32 ub = rPar.Get(i+1)->GetLong();
            if( ub < 0 )
            {
                StarBASIC::Error( SbERR_OUT_OF_RANGE );
                ub = 0;
            }
            pArray->AddDim32( 0, ub );
        }
    }
    else
    {
        pArray->unoAddDim( 0, -1 );
    }
    SbxVariableRef refVar = rPar.Get(0);
    sal_uInt16 nFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( NULL );
}

/*
 * FindObject and FindPropertyObject make it possible to
 * address objects and properties of the type Object with
 * their name as string-pararmeters at the runtime.
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



RTLFUNC(FindObject)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    OUString aNameStr = rPar.Get(1)->GetOUString();

    SbxBase* pFind =  StarBASIC::FindSBXInCurrentScope( aNameStr );
    SbxObject* pFindObj = NULL;
    if( pFind )
    {
        pFindObj = PTR_CAST(SbxObject,pFind);
    }
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( pFindObj );
}




RTLFUNC(FindPropertyObject)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    SbxBase* pObjVar = (SbxObject*)rPar.Get(1)->GetObject();
    SbxObject* pObj = NULL;
    if( pObjVar )
    {
        pObj = PTR_CAST(SbxObject,pObjVar);
    }
    if( !pObj && pObjVar && pObjVar->ISA(SbxVariable) )
    {
        SbxBase* pObjVarObj = ((SbxVariable*)pObjVar)->GetObject();
        pObj = PTR_CAST(SbxObject,pObjVarObj);
    }

    OUString aNameStr = rPar.Get(2)->GetOUString();

    SbxObject* pFindObj = NULL;
    if( pObj )
    {
        SbxVariable* pFindVar = pObj->Find( aNameStr, SbxCLASS_OBJECT );
        pFindObj = PTR_CAST(SbxObject,pFindVar);
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_PARAMETER );
    }

    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( pFindObj );
}



static sal_Bool lcl_WriteSbxVariable( const SbxVariable& rVar, SvStream* pStrm,
                                      sal_Bool bBinary, short nBlockLen, sal_Bool bIsArray )
{
    sal_uIntPtr nFPos = pStrm->Tell();

    sal_Bool bIsVariant = !rVar.IsFixed();
    SbxDataType eType = rVar.GetType();

    switch( eType )
    {
    case SbxBOOL:
    case SbxCHAR:
    case SbxBYTE:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( (sal_uInt16)SbxBYTE ); 
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
            pStrm->WriteUInt16( (sal_uInt16)SbxINTEGER ); 
        }
        pStrm->WriteInt16( rVar.GetInteger() );
        break;

    case SbxLONG:
    case SbxULONG:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( (sal_uInt16)SbxLONG ); 
        }
        pStrm->WriteInt32( rVar.GetLong() );
        break;
    case SbxSALINT64:
    case SbxSALUINT64:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( (sal_uInt16)SbxSALINT64 ); 
        }
        pStrm->WriteUInt64( (sal_uInt64)rVar.GetInt64() );
        break;
    case SbxSINGLE:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( (sal_uInt16)eType ); 
        }
        pStrm->WriteFloat( rVar.GetSingle() );
        break;

    case SbxDOUBLE:
    case SbxCURRENCY:
    case SbxDATE:
        if( bIsVariant )
        {
            pStrm->WriteUInt16( (sal_uInt16)eType ); 
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
                    pStrm->WriteUInt16( (sal_uInt16)SbxSTRING );
                }
                pStrm->WriteUniOrByteString( rStr, osl_getThreadTextEncoding() );
            }
            else
            {
                
                
                OString aByteStr(OUStringToOString(rStr, osl_getThreadTextEncoding()));
                pStrm->WriteCharPtr( (const char*)aByteStr.getStr() );
            }
        }
        break;

    default:
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return sal_False;
    }

    if( nBlockLen )
    {
        pStrm->Seek( nFPos + nBlockLen );
    }
    return pStrm->GetErrorCode() ? sal_False : sal_True;
}

static sal_Bool lcl_ReadSbxVariable( SbxVariable& rVar, SvStream* pStrm,
                                     sal_Bool bBinary, short nBlockLen, sal_Bool bIsArray )
{
    (void)bBinary;
    (void)bIsArray;

    double aDouble;

    sal_uIntPtr nFPos = pStrm->Tell();

    sal_Bool bIsVariant = !rVar.IsFixed();
    SbxDataType eVarType = rVar.GetType();

    SbxDataType eSrcType = eVarType;
    if( bIsVariant )
    {
        sal_uInt16 nTemp;
        pStrm->ReadUInt16( nTemp );
        eSrcType = (SbxDataType)nTemp;
    }

    switch( eSrcType )
    {
    case SbxBOOL:
    case SbxCHAR:
    case SbxBYTE:
        {
            sal_uInt8 aByte;
            pStrm->ReadUChar( aByte );

            if( bBinary && SbiRuntime::isVBAEnabled() && aByte == 1 && pStrm->IsEof() )
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
            rVar.PutInt64( (sal_Int64)aInt );
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return sal_False;
    }

    if( nBlockLen )
    {
        pStrm->Seek( nFPos + nBlockLen );
    }
    return pStrm->GetErrorCode() ? sal_False : sal_True;
}



static sal_Bool lcl_WriteReadSbxArray( SbxDimArray& rArr, SvStream* pStrm,
    sal_Bool bBinary, short nCurDim, short* pOtherDims, sal_Bool bWrite )
{
    SAL_WARN_IF( nCurDim <= 0,"basic", "Bad Dim");
    short nLower, nUpper;
    if( !rArr.GetDim( nCurDim, nLower, nUpper ) )
        return sal_False;
    for( short nCur = nLower; nCur <= nUpper; nCur++ )
    {
        pOtherDims[ nCurDim-1 ] = nCur;
        if( nCurDim != 1 )
            lcl_WriteReadSbxArray(rArr, pStrm, bBinary, nCurDim-1, pOtherDims, bWrite);
        else
        {
            SbxVariable* pVar = rArr.Get( (const short*)pOtherDims );
            sal_Bool bRet;
            if( bWrite )
                bRet = lcl_WriteSbxVariable(*pVar, pStrm, bBinary, 0, sal_True );
            else
                bRet = lcl_ReadSbxVariable(*pVar, pStrm, bBinary, 0, sal_True );
            if( !bRet )
                return sal_False;
        }
    }
    return sal_True;
}

void PutGet( SbxArray& rPar, sal_Bool bPut )
{
    if ( rPar.Count() != 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nFileNo = rPar.Get(1)->GetInteger();
    SbxVariable* pVar2 = rPar.Get(2);
    SbxDataType eType2 = pVar2->GetType();
    bool bHasRecordNo = (eType2 != SbxEMPTY && eType2 != SbxERROR);
    long nRecordNo = pVar2->GetLong();
    if ( nFileNo < 1 || ( bHasRecordNo && nRecordNo < 1 ) )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    nRecordNo--;
    SbiIoSystem* pIO = GetSbData()->pInst->GetIoSystem();
    SbiStream* pSbStrm = pIO->GetStream( nFileNo );

    if ( !pSbStrm || !(pSbStrm->GetMode() & (SBSTRM_BINARY | SBSTRM_RANDOM)) )
    {
        StarBASIC::Error( SbERR_BAD_CHANNEL );
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
        sal_uIntPtr nFilePos = bRandom ? (sal_uIntPtr)(nBlockLen*nRecordNo) : (sal_uIntPtr)nRecordNo;
        pStrm->Seek( nFilePos );
    }

    SbxDimArray* pArr = 0;
    SbxVariable* pVar = rPar.Get(3);
    if( pVar->GetType() & SbxARRAY )
    {
        SbxBase* pParObj = pVar->GetObject();
        pArr = PTR_CAST(SbxDimArray,pParObj);
    }

    sal_Bool bRet;

    if( pArr )
    {
        sal_uIntPtr nFPos = pStrm->Tell();
        short nDims = pArr->GetDims();
        short* pDims = new short[ nDims ];
        bRet = lcl_WriteReadSbxArray(*pArr,pStrm,!bRandom,nDims,pDims,bPut);
        delete [] pDims;
        if( nBlockLen )
            pStrm->Seek( nFPos + nBlockLen );
    }
    else
    {
        if( bPut )
            bRet = lcl_WriteSbxVariable(*pVar, pStrm, !bRandom, nBlockLen, sal_False);
        else
            bRet = lcl_ReadSbxVariable(*pVar, pStrm, !bRandom, nBlockLen, sal_False);
    }
    if( !bRet || pStrm->GetErrorCode() )
        StarBASIC::Error( SbERR_IO_ERROR );
}

RTLFUNC(Put)
{
    (void)pBasic;
    (void)bWrite;

    PutGet( rPar, sal_True );
}

RTLFUNC(Get)
{
    (void)pBasic;
    (void)bWrite;

    PutGet( rPar, sal_False );
}

RTLFUNC(Environ)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    OUString aResult;
    
    OString aByteStr(OUStringToOString(rPar.Get(1)->GetOUString(),
                                                 osl_getThreadTextEncoding()));
    const char* pEnvStr = getenv(aByteStr.getStr());
    if ( pEnvStr )
    {
        aResult = OUString(pEnvStr, strlen(pEnvStr), osl_getThreadTextEncoding());
    }
    rPar.Get(0)->PutString( aResult );
}

static double GetDialogZoomFactor( bool bX, long nValue )
{
    OutputDevice* pDevice = Application::GetDefaultDevice();
    double nResult = 0;
    if( pDevice )
    {
        Size aRefSize( nValue, nValue );
        Fraction aFracX( 1, 26 );
        Fraction aFracY( 1, 24 );
        MapMode aMap( MAP_APPFONT, Point(), aFracX, aFracY );
        Size aScaledSize = pDevice->LogicToPixel( aRefSize, aMap );
        aRefSize = pDevice->LogicToPixel( aRefSize, MapMode(MAP_TWIP) );

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


RTLFUNC(GetDialogZoomFactorX)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutDouble( GetDialogZoomFactor( true, rPar.Get(1)->GetLong() ));
}

RTLFUNC(GetDialogZoomFactorY)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutDouble( GetDialogZoomFactor( false, rPar.Get(1)->GetLong()));
}


RTLFUNC(EnableReschedule)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    if( GetSbData()->pInst )
        GetSbData()->pInst->EnableReschedule( rPar.Get(1)->GetBool() );
}

RTLFUNC(GetSystemTicks)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutLong( Time::GetSystemTicks() );
}

RTLFUNC(GetPathSeparator)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutString( OUString( SAL_PATHDELIMITER ) );
}

RTLFUNC(ResolvePath)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        rPar.Get(0)->PutString( aStr );
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
}

RTLFUNC(TypeLen)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
            nLen = (sal_Int16)rPar.Get(1)->GetOUString().getLength();
            break;

        default:
            nLen = 0;
            break;
        }
        rPar.Get(0)->PutInteger( nLen );
    }
}



RTLFUNC(CreateUnoStruct)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreateUnoStruct( pBasic, rPar, bWrite );
}



RTLFUNC(CreateUnoService)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreateUnoService( pBasic, rPar, bWrite );
}

RTLFUNC(CreateUnoServiceWithArguments)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreateUnoServiceWithArguments( pBasic, rPar, bWrite );
}


RTLFUNC(CreateUnoValue)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreateUnoValue( pBasic, rPar, bWrite );
}



RTLFUNC(GetProcessServiceManager)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_GetProcessServiceManager( pBasic, rPar, bWrite );
}



RTLFUNC(CreatePropertySet)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreatePropertySet( pBasic, rPar, bWrite );
}



RTLFUNC(HasUnoInterfaces)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_HasInterfaces( pBasic, rPar, bWrite );
}


RTLFUNC(IsUnoStruct)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_IsUnoStruct( pBasic, rPar, bWrite );
}


RTLFUNC(EqualUnoObjects)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_EqualUnoObjects( pBasic, rPar, bWrite );
}




void RTL_Impl_CreateUnoDialog( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );

RTLFUNC(CreateUnoDialog)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreateUnoDialog( pBasic, rPar, bWrite );
}


RTLFUNC(GlobalScope)
{
    (void)pBasic;
    (void)bWrite;

    SbxObject* p = pBasic;
    while( p->GetParent() )
    {
        p = p->GetParent();
    }
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( p );
}


RTLFUNC(ConvertToUrl)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        OUString aStr = rPar.Get(1)->GetOUString();
        INetURLObject aURLObj( aStr, INET_PROT_FILE );
        OUString aFileURL = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
        if( aFileURL.isEmpty() )
        {
            ::osl::File::getFileURLFromSystemPath( aFileURL, aFileURL );
        }
        if( aFileURL.isEmpty() )
        {
            aFileURL = aStr;
        }
        rPar.Get(0)->PutString(aFileURL);
    }
    else
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
}

RTLFUNC(ConvertFromUrl)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
}



RTLFUNC(GetDefaultContext)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_GetDefaultContext( pBasic, rPar, bWrite );
}

#ifdef DBG_TRACE_BASIC
RTLFUNC(TraceCommand)
{
    RTL_Impl_TraceCommand( pBasic, rPar, bWrite );
}
#endif

RTLFUNC(Join)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount != 3 && nParCount != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    SbxBase* pParObj = rPar.Get(1)->GetObject();
    SbxDimArray* pArr = PTR_CAST(SbxDimArray,pParObj);
    if( pArr )
    {
        if( pArr->GetDims() != 1 )
        {
            StarBASIC::Error( SbERR_WRONG_DIMS );   
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
        OUString aRetStr;
        short nLower, nUpper;
        pArr->GetDim( 1, nLower, nUpper );
        for( short i = nLower ; i <= nUpper ; ++i )
        {
            OUString aStr = pArr->Get( &i )->GetOUString();
            aRetStr += aStr;
            if( i != nUpper )
            {
                aRetStr += aDelim;
            }
        }
        rPar.Get(0)->PutString( aRetStr );
    }
    else
    {
        StarBASIC::Error( SbERR_MUST_HAVE_DIMS );
    }
}


RTLFUNC(Split)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    OUString aExpression = rPar.Get(1)->GetOUString();
    short nArraySize = 0;
    StringVector vRet;
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

    SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
    pArray->unoAddDim( 0, nArraySize-1 );

    
    for( short i = 0 ; i < nArraySize ; i++ )
    {
        SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
        xVar->PutString( vRet[i] );
        pArray->Put( (SbxVariable*)xVar, &i );
    }

    
    SbxVariableRef refVar = rPar.Get(0);
    sal_uInt16 nFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( NULL );
}


RTLFUNC(MonthName)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount != 2 && nParCount != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    Reference< XCalendar3 > xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( SbERR_INTERNAL_ERROR );
        return;
    }
    Sequence< CalendarItem2 > aMonthSeq = xCalendar->getMonths2();
    sal_Int32 nMonthCount = aMonthSeq.getLength();

    sal_Int16 nVal = rPar.Get(1)->GetInteger();
    if( nVal < 1 || nVal > nMonthCount )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    sal_Bool bAbbreviate = false;
    if( nParCount == 3 )
        bAbbreviate = rPar.Get(2)->GetBool();

    const CalendarItem2* pCalendarItems = aMonthSeq.getConstArray();
    const CalendarItem2& rItem = pCalendarItems[nVal - 1];

    OUString aRetStr = ( bAbbreviate ? rItem.AbbrevName : rItem.FullName );
    rPar.Get(0)->PutString(aRetStr);
}


RTLFUNC(WeekdayName)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount < 2 || nParCount > 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    Reference< XCalendar3 > xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( SbERR_INTERNAL_ERROR );
        return;
    }

    Sequence< CalendarItem2 > aDaySeq = xCalendar->getDays2();
    sal_Int16 nDayCount = (sal_Int16)aDaySeq.getLength();
    sal_Int16 nDay = rPar.Get(1)->GetInteger();
    sal_Int16 nFirstDay = 0;
    if( nParCount == 4 )
    {
        nFirstDay = rPar.Get(3)->GetInteger();
        if( nFirstDay < 0 || nFirstDay > 7 )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    sal_Bool bAbbreviate = false;
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
    rPar.Get(0)->PutString( aRetStr );
}

RTLFUNC(Weekday)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
        rPar.Get(0)->PutInteger( nDay );
    }
}


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

IntervalInfo const * getIntervalInfo( const OUString& rStringCode )
{
    static IntervalInfo const aIntervalTable[] =
    {
        { INTERVAL_YYYY, "yyyy", 0.0,           false }, 
        { INTERVAL_Q,    "q",    0.0,           false }, 
        { INTERVAL_M,    "m",    0.0,           false }, 
        { INTERVAL_Y,    "y",    1.0,           true  }, 
        { INTERVAL_D,    "d",    1.0,           true  }, 
        { INTERVAL_W,    "w",    1.0,           true  }, 
        { INTERVAL_WW,   "ww",   7.0,           true  }, 
        { INTERVAL_H,    "h",    1.0 /    24.0, true  }, 
        { INTERVAL_N,    "n",    1.0 /  1440.0, true  }, 
        { INTERVAL_S,    "s",    1.0 / 86400.0, true  }  
    };
    for( std::size_t i = 0; i != SAL_N_ELEMENTS(aIntervalTable); ++i )
    {
        if( rStringCode.equalsIgnoreAsciiCaseAscii(
                aIntervalTable[i].mStringCode ) )
        {
            return &aIntervalTable[i];
        }
    }
    return NULL;
}

inline void implGetDayMonthYear( sal_Int16& rnYear, sal_Int16& rnMonth, sal_Int16& rnDay, double dDate )
{
    rnDay   = implGetDateDay( dDate );
    rnMonth = implGetDateMonth( dDate );
    rnYear  = implGetDateYear( dDate );
}

inline sal_Int16 limitToINT16( sal_Int32 n32 )
{
    if( n32 > 32767 )
    {
        n32 = 32767;
    }
    else if( n32 < -32768 )
    {
        n32 = -32768;
    }
    return (sal_Int16)n32;
}

RTLFUNC(DateAdd)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount != 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    OUString aStringCode = rPar.Get(1)->GetOUString();
    IntervalInfo const * pInfo = getIntervalInfo( aStringCode );
    if( !pInfo )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
                nTargetYear16 = limitToINT16( nTargetYear );
                nTargetMonth = nMonth;
                bOk = implDateSerial( nTargetYear16, nTargetMonth, nDay, dNewDate );
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
                    nMonthAdd = (sal_Int16)( 3 * (lNumber % 4) );
                }
                else
                {
                    nYearsAdd = lNumber / 12;
                    nMonthAdd = (sal_Int16)( lNumber % 12 );
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
                    nTargetYear = (sal_Int32)nYear - nYearsAdd;
                }
                else
                {
                    nTargetMonth = nMonth + nMonthAdd;
                    if( nTargetMonth > 12 )
                    {
                        nTargetMonth -= 12;
                        nYearsAdd++;
                    }
                    nTargetYear = (sal_Int32)nYear + nYearsAdd;
                }
                nTargetYear16 = limitToINT16( nTargetYear );
                bOk = implDateSerial( nTargetYear16, nTargetMonth, nDay, dNewDate );
                break;
            }
            default: break;
        }

        if( bOk )
        {
            
            sal_Int16 nNewYear, nNewMonth, nNewDay;
            implGetDayMonthYear( nNewYear, nNewMonth, nNewDay, dNewDate );
            if( nNewYear > 9999 || nNewYear < 100 )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return;
            }
            sal_Int16 nCorrectionDay = nDay;
            while( nNewMonth > nTargetMonth )
            {
                nCorrectionDay--;
                implDateSerial( nTargetYear16, nTargetMonth, nCorrectionDay, dNewDate );
                implGetDayMonthYear( nNewYear, nNewMonth, nNewDay, dNewDate );
            }
            dNewDate += dHoursMinutesSeconds;
        }
    }

    rPar.Get(0)->PutDate( dNewDate );
}

inline double RoundImpl( double d )
{
    return ( d >= 0 ) ? floor( d + 0.5 ) : -floor( -d + 0.5 );
}

RTLFUNC(DateDiff)
{
    (void)pBasic;
    (void)bWrite;

    

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount < 4 || nParCount > 6 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    OUString aStringCode = rPar.Get(1)->GetOUString();
    IntervalInfo const * pInfo = getIntervalInfo( aStringCode );
    if( !pInfo )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
                sal_Int16 nFirstDay = 1;    
                if( nParCount >= 5 )
                {
                    nFirstDay = rPar.Get(4)->GetInteger();
                    if( nFirstDay < 0 || nFirstDay > 7 )
                    {
                        StarBASIC::Error( SbERR_BAD_ARGUMENT );
                        return;
                    }
                    if( nFirstDay == 0 )
                    {
                        Reference< XCalendar3 > xCalendar = getLocaleCalendar();
                        if( !xCalendar.is() )
                        {
                            StarBASIC::Error( SbERR_INTERNAL_ERROR );
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
            double dFactor = 24.0;
            dRet = RoundImpl( dFactor * (dDate2 - dDate1) );
            break;
        }
        case INTERVAL_N:
        {
            double dFactor =1440.0;
            dRet = RoundImpl( dFactor * (dDate2 - dDate1) );
            break;
        }
        case INTERVAL_S:
        {
            double dFactor = 86400.0;
            dRet = RoundImpl( dFactor * (dDate2 - dDate1) );
            break;
        }
    }
    rPar.Get(0)->PutDouble( dRet );
}

double implGetDateOfFirstDayInFirstWeek
    ( sal_Int16 nYear, sal_Int16& nFirstDay, sal_Int16& nFirstWeek, bool* pbError = NULL )
{
    SbError nError = 0;
    if( nFirstDay < 0 || nFirstDay > 7 )
        nError = SbERR_BAD_ARGUMENT;

    if( nFirstWeek < 0 || nFirstWeek > 3 )
        nError = SbERR_BAD_ARGUMENT;

    Reference< XCalendar3 > xCalendar;
    if( nFirstDay == 0 || nFirstWeek == 0 )
    {
        xCalendar = getLocaleCalendar();
        if( !xCalendar.is() )
            nError = SbERR_BAD_ARGUMENT;
    }

    if( nError != 0 )
    {
        StarBASIC::Error( nError );
        if( pbError )
            *pbError = true;
        return 0.0;
    }

    if( nFirstDay == 0 )
        nFirstDay = sal_Int16( xCalendar->getFirstDayOfWeek() + 1 );

    sal_Int16 nFirstWeekMinDays = 0;    
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
        nFirstWeekMinDays = 4;      
    else if( nFirstWeek == 3 )
        nFirstWeekMinDays = 7;      

    double dBaseDate;
    implDateSerial( nYear, 1, 1, dBaseDate );
    double dRetDate = dBaseDate;

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
    dRetDate = dBaseDate - nDayDiff;
    return dRetDate;
}

RTLFUNC(DatePart)
{
    (void)pBasic;
    (void)bWrite;

    

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount < 3 || nParCount > 5 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    OUString aStringCode = rPar.Get(1)->GetOUString();
    IntervalInfo const * pInfo = getIntervalInfo( aStringCode );
    if( !pInfo )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
            implDateSerial( nYear, 1, 1, dBaseDate );
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
            sal_Int16 nFirstDay = 1;    
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
            sal_Int16 nFirstDay = 1;    
            if( nParCount >= 4 )
                nFirstDay = rPar.Get(3)->GetInteger();

            sal_Int16 nFirstWeek = 1;   
            if( nParCount == 5 )
                nFirstWeek = rPar.Get(4)->GetInteger();

            sal_Int16 nYear = implGetDateYear( dDate );
            bool bError = false;
            double dYearFirstDay = implGetDateOfFirstDayInFirstWeek( nYear, nFirstDay, nFirstWeek, &bError );
            if( !bError )
            {
                if( dYearFirstDay > dDate )
                {
                    
                    dYearFirstDay = implGetDateOfFirstDayInFirstWeek( nYear - 1, nFirstDay, nFirstWeek );
                }
                else if( nFirstWeek != 1 )
                {
                    
                    double dNextYearFirstDay = implGetDateOfFirstDayInFirstWeek( nYear + 1, nFirstDay, nFirstWeek );
                    if( dDate >= dNextYearFirstDay )
                        dYearFirstDay = dNextYearFirstDay;
                }

                
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
    rPar.Get(0)->PutLong( nRet );
}


RTLFUNC(FormatDateTime)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount < 2 || nParCount > 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    double dDate = rPar.Get(1)->GetDate();
    sal_Int16 nNamedFormat = 0;
    if( nParCount > 2 )
    {
        nNamedFormat = rPar.Get(2)->GetInteger();
        if( nNamedFormat < 0 || nNamedFormat > 4 )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return;
        }
    }

    Reference< XCalendar3 > xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( SbERR_INTERNAL_ERROR );
        return;
    }

    OUString aRetStr;
    SbxVariableRef pSbxVar = new SbxVariable( SbxSTRING );
    switch( nNamedFormat )
    {
        
        
        
        

        
        
    case 0:
        pSbxVar->PutDate( dDate );
        aRetStr = pSbxVar->GetOUString();
        break;

        
        
        
        
    case 1:
        {
            SvNumberFormatter* pFormatter = NULL;
            if( GetSbData()->pInst )
            {
                pFormatter = GetSbData()->pInst->GetNumberFormatter();
            }
            else
            {
                sal_uInt32 n;   
                SbiInstance::PrepareNumberFormatter( pFormatter, n, n, n );
            }

            LanguageType eLangType = GetpApp()->GetSettings().GetLanguageTag().getLanguageType();
            sal_uIntPtr nIndex = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_LONG, eLangType );
            Color* pCol;
            pFormatter->GetOutputString( dDate, nIndex, aRetStr, &pCol );

            if( !GetSbData()->pInst )
            {
                delete pFormatter;
            }
            break;
        }

        
        
        
    case 2:
        pSbxVar->PutDate( floor(dDate) );
        aRetStr = pSbxVar->GetOUString();
        break;

        
        
        
        
    case 3:
        
        
    case 4:
        double n;
        double dTime = modf( dDate, &n );
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

    rPar.Get(0)->PutString( aRetStr );
}

RTLFUNC(Frac)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount != 2)
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    SbxVariable *pSbxVariable = rPar.Get(1);
    double dVal = pSbxVariable->GetDouble();
    if(dVal >= 0)
        rPar.Get(0)->PutDouble(dVal - ::rtl::math::approxFloor(dVal));
    else
        rPar.Get(0)->PutDouble(dVal - ::rtl::math::approxCeil(dVal));
}

RTLFUNC(Round)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount != 2 && nParCount != 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    SbxVariable *pSbxVariable = rPar.Get(1);
    double dVal = pSbxVariable->GetDouble();
    double dRes = 0.0;
    if( dVal != 0.0 )
    {
        bool bNeg = false;
        if( dVal < 0.0 )
        {
            bNeg = true;
            dVal = -dVal;
        }

        sal_Int16 numdecimalplaces = 0;
        if( nParCount == 3 )
        {
            numdecimalplaces = rPar.Get(2)->GetInteger();
            if( numdecimalplaces < 0 || numdecimalplaces > 22 )
            {
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return;
            }
        }

        if( numdecimalplaces == 0 )
        {
            dRes = floor( dVal + 0.5 );
        }
        else
        {
            double dFactor = pow( 10.0, numdecimalplaces );
            dVal *= dFactor;
            dRes = floor( dVal + 0.5 );
            dRes /= dFactor;
        }

        if( bNeg )
            dRes = -dRes;
    }
    rPar.Get(0)->PutDouble( dRes );
}

void CallFunctionAccessFunction( const Sequence< Any >& aArgs, const OUString& sFuncName, SbxVariable* pRet )
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    }
}

RTLFUNC(SYD)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    

    Sequence< Any > aParams( 4 );
    aParams[ 0 ] <<= makeAny( rPar.Get(1)->GetDouble() );
    aParams[ 1 ] <<= makeAny( rPar.Get(2)->GetDouble() );
    aParams[ 2 ] <<= makeAny( rPar.Get(3)->GetDouble() );
    aParams[ 3 ] <<= makeAny( rPar.Get(4)->GetDouble() );

    CallFunctionAccessFunction( aParams, "SYD", rPar.Get( 0 ) );
}

RTLFUNC(SLN)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    

    Sequence< Any > aParams( 3 );
    aParams[ 0 ] <<= makeAny( rPar.Get(1)->GetDouble() );
    aParams[ 1 ] <<= makeAny( rPar.Get(2)->GetDouble() );
    aParams[ 2 ] <<= makeAny( rPar.Get(3)->GetDouble() );

    CallFunctionAccessFunction( aParams, "SLN", rPar.Get( 0 ) );
}

RTLFUNC(Pmt)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 3 || nArgCount > 5 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    

    double rate = rPar.Get(1)->GetDouble();
    double nper = rPar.Get(2)->GetDouble();
    double pmt = rPar.Get(3)->GetDouble();

    
    double fv = 0;
    double type = 0;

    
    if ( nArgCount >= 4 )
    {
        if( rPar.Get(4)->GetType() != SbxEMPTY )
            fv = rPar.Get(4)->GetDouble();
    }
    
    if ( nArgCount >= 5 )
    {
        if( rPar.Get(5)->GetType() != SbxEMPTY )
            type = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams( 5 );
    aParams[ 0 ] <<= rate;
    aParams[ 1 ] <<= nper;
    aParams[ 2 ] <<= pmt;
    aParams[ 3 ] <<= fv;
    aParams[ 4 ] <<= type;

    CallFunctionAccessFunction( aParams, "Pmt", rPar.Get( 0 ) );
}

RTLFUNC(PPmt)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 4 || nArgCount > 6 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    

    double rate = rPar.Get(1)->GetDouble();
    double per = rPar.Get(2)->GetDouble();
    double nper = rPar.Get(3)->GetDouble();
    double pv = rPar.Get(4)->GetDouble();

    
    double fv = 0;
    double type = 0;

    
    if ( nArgCount >= 5 )
    {
        if( rPar.Get(5)->GetType() != SbxEMPTY )
            fv = rPar.Get(5)->GetDouble();
    }
    
    if ( nArgCount >= 6 )
    {
        if( rPar.Get(6)->GetType() != SbxEMPTY )
            type = rPar.Get(6)->GetDouble();
    }

    Sequence< Any > aParams( 6 );
    aParams[ 0 ] <<= rate;
    aParams[ 1 ] <<= per;
    aParams[ 2 ] <<= nper;
    aParams[ 3 ] <<= pv;
    aParams[ 4 ] <<= fv;
    aParams[ 5 ] <<= type;

    CallFunctionAccessFunction( aParams, "PPmt", rPar.Get( 0 ) );
}

RTLFUNC(PV)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 3 || nArgCount > 5 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    

    double rate = rPar.Get(1)->GetDouble();
    double nper = rPar.Get(2)->GetDouble();
    double pmt = rPar.Get(3)->GetDouble();

    
    double fv = 0;
    double type = 0;

    
    if ( nArgCount >= 4 )
    {
        if( rPar.Get(4)->GetType() != SbxEMPTY )
            fv = rPar.Get(4)->GetDouble();
    }
    
    if ( nArgCount >= 5 )
    {
        if( rPar.Get(5)->GetType() != SbxEMPTY )
            type = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams( 5 );
    aParams[ 0 ] <<= rate;
    aParams[ 1 ] <<= nper;
    aParams[ 2 ] <<= pmt;
    aParams[ 3 ] <<= fv;
    aParams[ 4 ] <<= type;

    CallFunctionAccessFunction( aParams, "PV", rPar.Get( 0 ) );
}

RTLFUNC(NPV)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 1 || nArgCount > 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    Sequence< Any > aParams( 2 );
    aParams[ 0 ] <<= makeAny( rPar.Get(1)->GetDouble() );
    Any aValues = sbxToUnoValue( rPar.Get(2),
                getCppuType( (Sequence<double>*)0 ) );

    
    Sequence< Sequence< double > > sValues(1);
    aValues >>= sValues[ 0 ];
    aValues <<= sValues;

    aParams[ 1 ] <<= aValues;

    CallFunctionAccessFunction( aParams, "NPV", rPar.Get( 0 ) );
}

RTLFUNC(NPer)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 3 || nArgCount > 5 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    

    double rate = rPar.Get(1)->GetDouble();
    double pmt = rPar.Get(2)->GetDouble();
    double pv = rPar.Get(3)->GetDouble();

    
    double fv = 0;
    double type = 0;

    
    if ( nArgCount >= 4 )
    {
        if( rPar.Get(4)->GetType() != SbxEMPTY )
            fv = rPar.Get(4)->GetDouble();
    }
    
    if ( nArgCount >= 5 )
    {
        if( rPar.Get(5)->GetType() != SbxEMPTY )
            type = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams( 5 );
    aParams[ 0 ] <<= rate;
    aParams[ 1 ] <<= pmt;
    aParams[ 2 ] <<= pv;
    aParams[ 3 ] <<= fv;
    aParams[ 4 ] <<= type;

    CallFunctionAccessFunction( aParams, "NPer", rPar.Get( 0 ) );
}

RTLFUNC(MIRR)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    

    Sequence< Any > aParams( 3 );
    Any aValues = sbxToUnoValue( rPar.Get(1),
                getCppuType( (Sequence<double>*)0 ) );

    
    Sequence< Sequence< double > > sValues(1);
    aValues >>= sValues[ 0 ];
    aValues <<= sValues;

    aParams[ 0 ] <<= aValues;
    aParams[ 1 ] <<= makeAny( rPar.Get(2)->GetDouble() );
    aParams[ 2 ] <<= makeAny( rPar.Get(3)->GetDouble() );

    CallFunctionAccessFunction( aParams, "MIRR", rPar.Get( 0 ) );
}

RTLFUNC(IRR)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 1 || nArgCount > 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    
    Any aValues = sbxToUnoValue( rPar.Get(1),
                getCppuType( (Sequence<double>*)0 ) );

    
    Sequence< Sequence< double > > sValues(1);
    aValues >>= sValues[ 0 ];
    aValues <<= sValues;

    
    double guess = 0.1;
    
    if ( nArgCount >= 2 )
    {
        if( rPar.Get(2)->GetType() != SbxEMPTY )
            guess = rPar.Get(2)->GetDouble();
    }

    Sequence< Any > aParams( 2 );
    aParams[ 0 ] <<= aValues;
    aParams[ 1 ] <<= guess;

    CallFunctionAccessFunction( aParams, "IRR", rPar.Get( 0 ) );
}

RTLFUNC(IPmt)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 4 || nArgCount > 6 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    

    double rate = rPar.Get(1)->GetDouble();
    double per = rPar.Get(2)->GetInteger();
    double nper = rPar.Get(3)->GetDouble();
    double pv = rPar.Get(4)->GetDouble();

    
    double fv = 0;
    double type = 0;

    
    if ( nArgCount >= 5 )
    {
        if( rPar.Get(5)->GetType() != SbxEMPTY )
            fv = rPar.Get(5)->GetDouble();
    }
    
    if ( nArgCount >= 6 )
    {
        if( rPar.Get(6)->GetType() != SbxEMPTY )
            type = rPar.Get(6)->GetDouble();
    }

    Sequence< Any > aParams( 6 );
    aParams[ 0 ] <<= rate;
    aParams[ 1 ] <<= per;
    aParams[ 2 ] <<= nper;
    aParams[ 3 ] <<= pv;
    aParams[ 4 ] <<= fv;
    aParams[ 5 ] <<= type;

    CallFunctionAccessFunction( aParams, "IPmt", rPar.Get( 0 ) );
}

RTLFUNC(FV)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 3 || nArgCount > 5 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    

    double rate = rPar.Get(1)->GetDouble();
    double nper = rPar.Get(2)->GetDouble();
    double pmt = rPar.Get(3)->GetDouble();

    
    double pv = 0;
    double type = 0;

    
    if ( nArgCount >= 4 )
    {
        if( rPar.Get(4)->GetType() != SbxEMPTY )
            pv = rPar.Get(4)->GetDouble();
    }
    
    if ( nArgCount >= 5 )
    {
        if( rPar.Get(5)->GetType() != SbxEMPTY )
            type = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams( 5 );
    aParams[ 0 ] <<= rate;
    aParams[ 1 ] <<= nper;
    aParams[ 2 ] <<= pmt;
    aParams[ 3 ] <<= pv;
    aParams[ 4 ] <<= type;

    CallFunctionAccessFunction( aParams, "FV", rPar.Get( 0 ) );
}

RTLFUNC(DDB)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 4 || nArgCount > 5 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    

    double cost = rPar.Get(1)->GetDouble();
    double salvage = rPar.Get(2)->GetDouble();
    double life = rPar.Get(3)->GetDouble();
    double period = rPar.Get(4)->GetDouble();

    
    double factor = 2;

    
    if ( nArgCount >= 5 )
    {
        if( rPar.Get(5)->GetType() != SbxEMPTY )
            factor = rPar.Get(5)->GetDouble();
    }

    Sequence< Any > aParams( 5 );
    aParams[ 0 ] <<= cost;
    aParams[ 1 ] <<= salvage;
    aParams[ 2 ] <<= life;
    aParams[ 3 ] <<= period;
    aParams[ 4 ] <<= factor;

    CallFunctionAccessFunction( aParams, "DDB", rPar.Get( 0 ) );
}

RTLFUNC(Rate)
{
    (void)pBasic;
    (void)bWrite;

    sal_uLong nArgCount = rPar.Count()-1;

    if ( nArgCount < 3 || nArgCount > 6 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    

    double nper = 0;
    double pmt = 0;
    double pv = 0;

    nper = rPar.Get(1)->GetDouble();
    pmt = rPar.Get(2)->GetDouble();
    pv = rPar.Get(3)->GetDouble();

    
    double fv = 0;
    double type = 0;
    double guess = 0.1;

    
    if ( nArgCount >= 4 )
    {
        if( rPar.Get(4)->GetType() != SbxEMPTY )
            fv = rPar.Get(4)->GetDouble();
    }

    
    if ( nArgCount >= 5 )
    {
        if( rPar.Get(5)->GetType() != SbxEMPTY )
            type = rPar.Get(5)->GetDouble();
    }

    
    if ( nArgCount >= 6 )
    {
        if( rPar.Get(6)->GetType() != SbxEMPTY )
            type = rPar.Get(6)->GetDouble();
    }

    Sequence< Any > aParams( 6 );
    aParams[ 0 ] <<= nper;
    aParams[ 1 ] <<= pmt;
    aParams[ 2 ] <<= pv;
    aParams[ 3 ] <<= fv;
    aParams[ 4 ] <<= type;
    aParams[ 5 ] <<= guess;

    CallFunctionAccessFunction( aParams, "Rate", rPar.Get( 0 ) );
}

RTLFUNC(StrReverse)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    SbxVariable *pSbxVariable = rPar.Get(1);
    if( pSbxVariable->IsNull() )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    OUString aStr = comphelper::string::reverseString(pSbxVariable->GetOUString());
    rPar.Get(0)->PutString( aStr );
}

RTLFUNC(CompatibilityMode)
{
    (void)pBasic;
    (void)bWrite;

    bool bEnabled = false;
    sal_uInt16 nCount = rPar.Count();
    if ( nCount != 1 && nCount != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    SbiInstance* pInst = GetSbData()->pInst;
    if( pInst )
    {
        if ( nCount == 2 )
        {
            pInst->EnableCompatibility( rPar.Get(1)->GetBool() );
        }
        bEnabled = pInst->IsCompatibility();
    }
    rPar.Get(0)->PutBool( bEnabled );
}

RTLFUNC(Input)
{
    (void)pBasic;
    (void)bWrite;

    
    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    sal_uInt16 nByteCount  = rPar.Get(1)->GetUShort();
    sal_Int16  nFileNumber = rPar.Get(2)->GetInteger();

    SbiIoSystem* pIosys = GetSbData()->pInst->GetIoSystem();
    SbiStream* pSbStrm = pIosys->GetStream( nFileNumber );
    if ( !pSbStrm || !(pSbStrm->GetMode() & (SBSTRM_BINARY | SBSTRM_INPUT)) )
    {
        StarBASIC::Error( SbERR_BAD_CHANNEL );
        return;
    }

    OString aByteBuffer;
    SbError err = pSbStrm->Read( aByteBuffer, nByteCount, true );
    if( !err )
        err = pIosys->GetError();

    if( err )
    {
        StarBASIC::Error( err );
        return;
    }
    rPar.Get(0)->PutString(OStringToOUString(aByteBuffer, osl_getThreadTextEncoding()));
}

RTLFUNC(Me)
{
    (void)pBasic;
    (void)bWrite;

    SbModule* pActiveModule = GetSbData()->pInst->GetActiveModule();
    SbClassModuleObject* pClassModuleObject = PTR_CAST(SbClassModuleObject,pActiveModule);
    SbxVariableRef refVar = rPar.Get(0);
    if( pClassModuleObject == NULL )
    {
        SbObjModule* pMod = PTR_CAST(SbObjModule,pActiveModule);
        if ( pMod )
            refVar->PutObject( pMod );
        else
            StarBASIC::Error( SbERR_INVALID_USAGE_OBJECT );
    }
    else
        refVar->PutObject( pClassModuleObject );
}

#endif

sal_Int16 implGetWeekDay( double aDate, bool bFirstDayParam, sal_Int16 nFirstDay )
{
    Date aRefDate( 1,1,1900 );
    long nDays = (long) aDate;
    nDays -= 2; 
    aRefDate += nDays;
    DayOfWeek aDay = aRefDate.GetDayOfWeek();
    sal_Int16 nDay;
    if ( aDay != SUNDAY )
        nDay = (sal_Int16)aDay + 2;
    else
        nDay = 1;   

    
    if( bFirstDayParam )
    {
        if( nFirstDay < 0 || nFirstDay > 7 )
        {
#ifndef DISABLE_SCRIPTING
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
#endif
            return 0;
        }
        if( nFirstDay == 0 )
        {
            Reference< XCalendar3 > xCalendar = getLocaleCalendar();
            if( !xCalendar.is() )
            {
#ifndef DISABLE_SCRIPTING
                StarBASIC::Error( SbERR_INTERNAL_ERROR );
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
