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

#include <stdlib.h> // getenv
#include <vcl/svapp.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/timer.hxx>
#include <basic/sbxvar.hxx>
#ifndef _SBX_HXX
#include <basic/sbx.hxx>
#endif
#include <svl/zforlist.hxx>
#include <tools/fsys.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>

#ifdef OS2
#define INCL_DOS
#define INCL_DOSPROCESS
#include <svpm.h>
#endif

#ifndef CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC
#endif

#include <vcl/jobset.hxx>
#include <basic/sbobjmod.hxx>

#include "sbintern.hxx"
#include "runtime.hxx"
#include "stdobj.hxx"
#include "rtlproto.hxx"
#include "dllmgr.hxx"
#include <iosys.hxx>
#include "sbunoobj.hxx"
#include "propacc.hxx"


#include <comphelper/processfactory.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/XCalendar.hpp>

using namespace comphelper;
using namespace com::sun::star::uno;
using namespace com::sun::star::i18n;


static Reference< XCalendar > getLocaleCalendar( void )
{
    static Reference< XCalendar > xCalendar;
    if( !xCalendar.is() )
    {
        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( xSMgr.is() )
        {
            xCalendar = Reference< XCalendar >( xSMgr->createInstance
                ( ::rtl::OUString::createFromAscii( "com.sun.star.i18n.LocaleCalendar" ) ), UNO_QUERY );
        }
    }

    static com::sun::star::lang::Locale aLastLocale;
    static bool bNeedsInit = true;

    com::sun::star::lang::Locale aLocale = Application::GetSettings().GetLocale();
    bool bNeedsReload = false;
    if( bNeedsInit )
    {
        bNeedsInit = false;
        bNeedsReload = true;
    }
    else if( aLocale.Language != aLastLocale.Language ||
             aLocale.Country  != aLastLocale.Country )
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

RTLFUNC(CallByName)
{
    (void)pBasic;
    (void)bWrite;

    const sal_Int16 vbGet       = 2;
    const sal_Int16 vbLet       = 4;
    const sal_Int16 vbMethod    = 1;
    const sal_Int16 vbSet       = 8;

    // At least 3 parameter needed plus function itself -> 4
    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount < 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // 1. parameter is object
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

    // 2. parameter is ProcedureName
    String aNameStr = rPar.Get(2)->GetString();

    // 3. parameter is CallType
    sal_Int16 nCallType = rPar.Get(3)->GetInteger();

    //SbxObject* pFindObj = NULL;
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
                    SbiInstance* pInst = pINST;
                    SbiRuntime* pRT = pInst ? pInst->pRun : NULL;
                    if( pRT != NULL )
                        pRT->StepSET_Impl( pValVar, rFindVar, false );
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

                // Setup parameters
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

                // Call method
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

RTLFUNC(CBool) // JSM
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutBool(bVal);
}

RTLFUNC(CByte) // JSM
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutByte(nByte);
}

RTLFUNC(CCur)  // JSM
{
    (void)pBasic;
    (void)bWrite;

    SbxINT64 nCur;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nCur = pSbxVariable->GetCurrency();
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutCurrency( nCur );
}

RTLFUNC(CDec)  // JSM
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutDecimal( pDec );
#else
    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
#endif
}

RTLFUNC(CDate) // JSM
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutDate(nVal);
}

RTLFUNC(CDbl)  // JSM
{
    (void)pBasic;
    (void)bWrite;

    double nVal = 0.0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        if( pSbxVariable->GetType() == SbxSTRING )
        {
            // AB #41690 , String holen
            String aScanStr = pSbxVariable->GetString();
            SbError Error = SbxValue::ScanNumIntnl( aScanStr, nVal );
            if( Error != SbxERR_OK )
                StarBASIC::Error( Error );
        }
        else
        {
            nVal = pSbxVariable->GetDouble();
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutDouble(nVal);
}

RTLFUNC(CInt)  // JSM
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutInteger(nVal);
}

RTLFUNC(CLng)  // JSM
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutLong(nVal);
}

RTLFUNC(CSng)  // JSM
{
    (void)pBasic;
    (void)bWrite;

    float nVal = (float)0.0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        if( pSbxVariable->GetType() == SbxSTRING )
        {
            // AB #41690 , String holen
            double dVal = 0.0;
            String aScanStr = pSbxVariable->GetString();
            SbError Error = SbxValue::ScanNumIntnl( aScanStr, dVal, /*bSingle=*/sal_True );
            if( SbxBase::GetError() == SbxERR_OK && Error != SbxERR_OK )
                StarBASIC::Error( Error );
            nVal = (float)dVal;
        }
        else
        {
            nVal = pSbxVariable->GetSingle();
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutSingle(nVal);
}

RTLFUNC(CStr)  // JSM
{
    (void)pBasic;
    (void)bWrite;

    String aString;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        aString = pSbxVariable->GetString();
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutString(aString);
}

RTLFUNC(CVar)  // JSM
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutErr( nErrCode );
}

RTLFUNC(Iif) // JSM
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 4 )
    {
        if (rPar.Get(1)->GetBool())
            *rPar.Get(0) = *rPar.Get(2);
        else
            *rPar.Get(0) = *rPar.Get(3);
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(GetSystemType)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
        // Removed for SRC595
        rPar.Get(0)->PutInteger( -1 );
}

RTLFUNC(GetGUIType)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // 17.7.2000 Make simple solution for testtool / fat office
#if defined (WNT)
        rPar.Get(0)->PutInteger( 1 );
#elif defined OS2
        rPar.Get(0)->PutInteger( 2 );
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
        // Anzahl der Argumente muss ungerade sein
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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

//i#64882# Common wait impl for existing Wait and new WaitUntil
// rtl functions
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
        nWait = (long)( dSecs * 1000 ); // wait in thousands of sec
    }
    else
        nWait = rPar.Get(1)->GetLong();
    if( nWait < 0 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    Timer aTimer;
    aTimer.SetTimeout( nWait );
    aTimer.Start();
    while ( aTimer.IsActive() )
        Application::Yield();
}

//i#64882#
RTLFUNC(Wait)
{
    (void)pBasic;
    (void)bWrite;
    Wait_Impl( false, rPar );
}

//i#64882# add new WaitUntil ( for application.wait )
// share wait_impl with 'normal' oobasic wait
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
    // Dummy implementation as the following code leads
    // to performance problems for unknown reasons
    //Timer aTimer;
    //aTimer.SetTimeout( 1 );
    //aTimer.Start();
    //while ( aTimer.IsActive() )
    //  Application::Reschedule();
    Application::Reschedule( true );
}

RTLFUNC(GetGUIVersion)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 1 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // Removed for SRC595
        rPar.Get(0)->PutLong( -1 );
    }
}

RTLFUNC(Choose)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr( rPar.Get(1)->GetString() );
        aStr.EraseLeadingChars();
        aStr.EraseTrailingChars();
        rPar.Get(0)->PutString( aStr );
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
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    pINST->GetDllMgr()->FreeDll( rPar.Get(1)->GetString() );
}
bool IsBaseIndexOne()
{
    bool result = false;
    if ( pINST && pINST->pRun )
    {
        sal_uInt16 res = pINST->pRun->GetBase();
        if ( res )
            result = true;
    }
    return result;
}

RTLFUNC(Array)
{
    (void)pBasic;
    (void)bWrite;

    SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
    sal_uInt16 nArraySize = rPar.Count() - 1;

    // Option Base zunaechst ignorieren (kennt leider nur der Compiler)
    bool bIncIndex = (IsBaseIndexOne() && SbiRuntime::isVBAEnabled() );
    if( nArraySize )
    {
        if ( bIncIndex )
            pArray->AddDim( 1, nArraySize );
        else
            pArray->AddDim( 0, nArraySize-1 );
    }
    else
    {
        pArray->unoAddDim( 0, -1 );
    }

    // Parameter ins Array uebernehmen
    // ATTENTION: Using type sal_uInt16 for loop variable is
    // mandatory to workaround a problem with the
    // Solaris Intel compiler optimizer! See i104354
    for( sal_uInt16 i = 0 ; i < nArraySize ; i++ )
    {
        SbxVariable* pVar = rPar.Get(i+1);
        SbxVariable* pNew = new SbxVariable( *pVar );
        pNew->SetFlag( SBX_WRITE );
        short index = static_cast< short >(i);
        if ( bIncIndex )
            ++index;
        pArray->Put( pNew, &index );
    }

    // Array zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    sal_uInt16 nFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( NULL );
}


// Featurewunsch #57868
// Die Funktion liefert ein Variant-Array, wenn keine Parameter angegeben
// werden, wird ein leeres Array erzeugt (entsprechend dim a(), entspricht
// einer Sequence der Laenge 0 in Uno).
// Wenn Parameter angegeben sind, wird fuer jeden eine Dimension erzeugt
// DimArray( 2, 2, 4 ) entspricht DIM a( 2, 2, 4 )
// Das Array ist immer vom Typ Variant
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
        pArray->unoAddDim( 0, -1 );

    // Array zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    sal_uInt16 nFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( NULL );
}

/*
 * FindObject und FindPropertyObject ermoeglichen es,
 * Objekte und Properties vom Typ Objekt zur Laufzeit
 * ueber ihren Namen als String-Parameter anzusprechen.
 *
 * Bsp.:
 * MyObj.Prop1.Bla = 5
 *
 * entspricht:
 * dim ObjVar as Object
 * dim ObjProp as Object
 * ObjName$ = "MyObj"
 * ObjVar = FindObject( ObjName$ )
 * PropName$ = "Prop1"
 * ObjProp = FindPropertyObject( ObjVar, PropName$ )
 * ObjProp.Bla = 5
 *
 * Dabei koennen die Namen zur Laufzeit dynamisch
 * erzeugt werden und, so dass z.B. ueber Controls
 * "TextEdit1" bis "TextEdit5" in einem Dialog in
 * einer Schleife iteriert werden kann.
 */

// Objekt ueber den Namen ansprechen
// 1. Parameter = Name des Objekts als String
RTLFUNC(FindObject)
{
    (void)pBasic;
    (void)bWrite;

    // Wir brauchen einen Parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // 1. Parameter ist der Name
    String aNameStr = rPar.Get(1)->GetString();

    // Basic-Suchfunktion benutzen
    SbxBase* pFind =  StarBASIC::FindSBXInCurrentScope( aNameStr );
    SbxObject* pFindObj = NULL;
    if( pFind )
        pFindObj = PTR_CAST(SbxObject,pFind);
    /*
    if( !pFindObj )
    {
        StarBASIC::Error( SbERR_VAR_UNDEFINED );
        return;
    }
    */

    // Objekt zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( pFindObj );
}

// Objekt-Property in einem Objekt ansprechen
// 1. Parameter = Objekt
// 2. Parameter = Name der Property als String
RTLFUNC(FindPropertyObject)
{
    (void)pBasic;
    (void)bWrite;

    // Wir brauchen 2 Parameter
    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // 1. Parameter holen, muss Objekt sein
    SbxBase* pObjVar = (SbxObject*)rPar.Get(1)->GetObject();
    SbxObject* pObj = NULL;
    if( pObjVar )
        pObj = PTR_CAST(SbxObject,pObjVar);
    if( !pObj && pObjVar && pObjVar->ISA(SbxVariable) )
    {
        SbxBase* pObjVarObj = ((SbxVariable*)pObjVar)->GetObject();
        pObj = PTR_CAST(SbxObject,pObjVarObj);
    }
    /*
    if( !pObj )
    {
        StarBASIC::Error( SbERR_VAR_UNDEFINED );
        return;
    }
    */

    // 2. Parameter ist der Name
    String aNameStr = rPar.Get(2)->GetString();

    // Jetzt muss ein Objekt da sein, sonst Error
    SbxObject* pFindObj = NULL;
    if( pObj )
    {
        // Im Objekt nach Objekt suchen
        SbxVariable* pFindVar = pObj->Find( aNameStr, SbxCLASS_OBJECT );
        pFindObj = PTR_CAST(SbxObject,pFindVar);
    }
    else
        StarBASIC::Error( SbERR_BAD_PARAMETER );

    // Objekt zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( pFindObj );
}



sal_Bool lcl_WriteSbxVariable( const SbxVariable& rVar, SvStream* pStrm,
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
                    *pStrm << (sal_uInt16)SbxBYTE; // VarType Id
                *pStrm << rVar.GetByte();
                break;

        case SbxEMPTY:
        case SbxNULL:
        case SbxVOID:
        case SbxINTEGER:
        case SbxUSHORT:
        case SbxINT:
        case SbxUINT:
                if( bIsVariant )
                    *pStrm << (sal_uInt16)SbxINTEGER; // VarType Id
                *pStrm << rVar.GetInteger();
                break;

        case SbxLONG:
        case SbxULONG:
        case SbxLONG64:
        case SbxULONG64:
                if( bIsVariant )
                    *pStrm << (sal_uInt16)SbxLONG; // VarType Id
                *pStrm << rVar.GetLong();
                break;

        case SbxSINGLE:
                if( bIsVariant )
                    *pStrm << (sal_uInt16)eType; // VarType Id
                *pStrm << rVar.GetSingle();
                break;

        case SbxDOUBLE:
        case SbxCURRENCY:
        case SbxDATE:
                if( bIsVariant )
                    *pStrm << (sal_uInt16)eType; // VarType Id
                *pStrm << rVar.GetDouble();
                break;

        case SbxSTRING:
        case SbxLPSTR:
                {
                const String& rStr = rVar.GetString();
                if( !bBinary || bIsArray )
                {
                    if( bIsVariant )
                        *pStrm << (sal_uInt16)SbxSTRING;
                    pStrm->WriteByteString( rStr, gsl_getSystemTextEncoding() );
                    //*pStrm << rStr;
                }
                else
                {
                    // ohne Laengenangabe! ohne Endekennung!
                    // What does that mean for Unicode?! Choosing conversion to ByteString...
                    ByteString aByteStr( rStr, gsl_getSystemTextEncoding() );
                    *pStrm << (const char*)aByteStr.GetBuffer();
                    //*pStrm << (const char*)rStr.GetStr();
                }
                }
                break;

        default:
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return sal_False;
    }

    if( nBlockLen )
        pStrm->Seek( nFPos + nBlockLen );
    return pStrm->GetErrorCode() ? sal_False : sal_True;
}

sal_Bool lcl_ReadSbxVariable( SbxVariable& rVar, SvStream* pStrm,
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
        *pStrm >> nTemp;
        eSrcType = (SbxDataType)nTemp;
    }

    switch( eSrcType )
    {
        case SbxBOOL:
        case SbxCHAR:
        case SbxBYTE:
                {
                sal_uInt8 aByte;
                *pStrm >> aByte;

                if( bBinary && SbiRuntime::isVBAEnabled() && aByte == 1 && pStrm->IsEof() )
                    aByte = 0;

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
                *pStrm >> aInt;
                rVar.PutInteger( aInt );
                }
                break;

        case SbxLONG:
        case SbxULONG:
        case SbxLONG64:
        case SbxULONG64:
                {
                sal_Int32 aInt;
                *pStrm >> aInt;
                rVar.PutLong( aInt );
                }
                break;

        case SbxSINGLE:
                {
                float nS;
                *pStrm >> nS;
                rVar.PutSingle( nS );
                }
                break;

        case SbxDOUBLE:
        case SbxCURRENCY:
                {
                *pStrm >> aDouble;
                rVar.PutDouble( aDouble );
                }
                break;

        case SbxDATE:
                {
                *pStrm >> aDouble;
                rVar.PutDate( aDouble );
                }
                break;

        case SbxSTRING:
        case SbxLPSTR:
                {
                String aStr;
                pStrm->ReadByteString( aStr, gsl_getSystemTextEncoding() );
                rVar.PutString( aStr );
                }
                break;

        default:
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return sal_False;
    }

    if( nBlockLen )
        pStrm->Seek( nFPos + nBlockLen );
    return pStrm->GetErrorCode() ? sal_False : sal_True;
}


// nCurDim = 1...n
sal_Bool lcl_WriteReadSbxArray( SbxDimArray& rArr, SvStream* pStrm,
    sal_Bool bBinary, short nCurDim, short* pOtherDims, sal_Bool bWrite )
{
    DBG_ASSERT( nCurDim > 0,"Bad Dim");
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
    // Wir brauchen 3 Parameter
    if ( rPar.Count() != 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    sal_Int16 nFileNo = rPar.Get(1)->GetInteger();
    SbxVariable* pVar2 = rPar.Get(2);
    SbxDataType eType2 = pVar2->GetType();
    sal_Bool bHasRecordNo = (sal_Bool)(eType2 != SbxEMPTY && eType2 != SbxERROR);
    long nRecordNo = pVar2->GetLong();
    if ( nFileNo < 1 || ( bHasRecordNo && nRecordNo < 1 ) )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    nRecordNo--; // wir moegen's ab 0!
    SbiIoSystem* pIO = pINST->GetIoSystem();
    SbiStream* pSbStrm = pIO->GetStream( nFileNo );
    // das File muss Random (feste Record-Laenge) oder Binary sein
    if ( !pSbStrm || !(pSbStrm->GetMode() & (SBSTRM_BINARY | SBSTRM_RANDOM)) )
    {
        StarBASIC::Error( SbERR_BAD_CHANNEL );
        return;
    }

    SvStream* pStrm = pSbStrm->GetStrm();
    sal_Bool bRandom = pSbStrm->IsRandom();
    short nBlockLen = bRandom ? pSbStrm->GetBlockLen() : 0;

    if( bPut )
    {
        // Datei aufplustern, falls jemand uebers Dateiende hinaus geseekt hat
        pSbStrm->ExpandFile();
    }

    // auf die Startposition seeken
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
    String aResult;
    // sollte ANSI sein, aber unter Win16 in DLL nicht moeglich
    ByteString aByteStr( rPar.Get(1)->GetString(), gsl_getSystemTextEncoding() );
    const char* pEnvStr = getenv( aByteStr.GetBuffer() );
    if ( pEnvStr )
        aResult = String::CreateFromAscii( pEnvStr );
    rPar.Get(0)->PutString( aResult );
}

static double GetDialogZoomFactor( sal_Bool bX, long nValue )
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
    rPar.Get(0)->PutDouble( GetDialogZoomFactor( sal_True, rPar.Get(1)->GetLong() ));
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
    rPar.Get(0)->PutDouble( GetDialogZoomFactor( sal_False, rPar.Get(1)->GetLong()));
}


RTLFUNC(EnableReschedule)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutEmpty();
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    if( pINST )
        pINST->EnableReschedule( rPar.Get(1)->GetBool() );
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
    rPar.Get(0)->PutString( DirEntry::GetAccessDelimiter() );
}

RTLFUNC(ResolvePath)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        String aStr = rPar.Get(1)->GetString();
        DirEntry aEntry( aStr );
        //if( aEntry.IsVirtual() )
            //aStr = aEntry.GetRealPathFromVirtualURL();
        rPar.Get(0)->PutString( aStr );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(TypeLen)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
            case SbxLONG64:
            case SbxULONG64:
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
                nLen = (sal_Int16)rPar.Get(1)->GetString().Len();
                break;

            default:
                nLen = 0;
        }
        rPar.Get(0)->PutInteger( nLen );
    }
}


// Uno-Struct eines beliebigen Typs erzeugen
// 1. Parameter == Klassename, weitere Parameter zur Initialisierung
RTLFUNC(CreateUnoStruct)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreateUnoStruct( pBasic, rPar, bWrite );
}

// Uno-Service erzeugen
// 1. Parameter == Service-Name
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


// ServiceManager liefern (keine Parameter)
RTLFUNC(GetProcessServiceManager)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_GetProcessServiceManager( pBasic, rPar, bWrite );
}

// PropertySet erzeugen
// 1. Parameter == Sequence<PropertyValue>
RTLFUNC(CreatePropertySet)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreatePropertySet( pBasic, rPar, bWrite );
}

// Abfragen, ob ein Interface unterstuetzt wird
// Mehrere Interface-Namen als Parameter
RTLFUNC(HasUnoInterfaces)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_HasInterfaces( pBasic, rPar, bWrite );
}

// Abfragen, ob ein Basic-Objekt ein Uno-Struct repraesentiert
RTLFUNC(IsUnoStruct)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_IsUnoStruct( pBasic, rPar, bWrite );
}

// Abfragen, ob zwei Uno-Objekte identisch sind
RTLFUNC(EqualUnoObjects)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_EqualUnoObjects( pBasic, rPar, bWrite );
}

// Instanciate "com.sun.star.awt.UnoControlDialog" on basis
// of a DialogLibrary entry: Convert from XML-ByteSequence
// and attach events. Implemented in classes\eventatt.cxx
void RTL_Impl_CreateUnoDialog( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );

RTLFUNC(CreateUnoDialog)
{
    (void)pBasic;
    (void)bWrite;

    RTL_Impl_CreateUnoDialog( pBasic, rPar, bWrite );
}

// Return the application standard lib as root scope
RTLFUNC(GlobalScope)
{
    (void)pBasic;
    (void)bWrite;

    SbxObject* p = pBasic;
    while( p->GetParent() )
        p = p->GetParent();

    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( p );
}

// Helper functions to convert Url from/to system paths
RTLFUNC(ConvertToUrl)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        String aStr = rPar.Get(1)->GetString();
        INetURLObject aURLObj( aStr, INET_PROT_FILE );
        ::rtl::OUString aFileURL = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
        if( !aFileURL.getLength() )
            ::osl::File::getFileURLFromSystemPath( aFileURL, aFileURL );
        if( !aFileURL.getLength() )
            aFileURL = aStr;
        rPar.Get(0)->PutString( String(aFileURL) );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(ConvertFromUrl)
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() == 2 )
    {
        String aStr = rPar.Get(1)->GetString();
        ::rtl::OUString aSysPath;
        ::osl::File::getSystemPathFromFileURL( aStr, aSysPath );
        if( !aSysPath.getLength() )
            aSysPath = aStr;
        rPar.Get(0)->PutString( String(aSysPath) );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}


// Provide DefaultContext
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
            StarBASIC::Error( SbERR_WRONG_DIMS );   // Syntax Error?!

        String aDelim;
        if( nParCount == 3 )
            aDelim = rPar.Get(2)->GetString();
        else
            aDelim = String::CreateFromAscii( " " );

        String aRetStr;
        short nLower, nUpper;
        pArr->GetDim( 1, nLower, nUpper );
        for( short i = nLower ; i <= nUpper ; ++i )
        {
            String aStr = pArr->Get( &i )->GetString();
            aRetStr += aStr;
            if( i != nUpper )
                aRetStr += aDelim;
        }
        rPar.Get(0)->PutString( aRetStr );
    }
    else
        StarBASIC::Error( SbERR_MUST_HAVE_DIMS );
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

    String aExpression = rPar.Get(1)->GetString();
    short nArraySize = 0;
    StringVector vRet;
    if( aExpression.Len() )
    {
        String aDelim;
        if( nParCount >= 3 )
            aDelim = rPar.Get(2)->GetString();
        else
            aDelim = String::CreateFromAscii( " " );

        sal_Int32 nCount = -1;
        if( nParCount == 4 )
            nCount = rPar.Get(3)->GetLong();

        xub_StrLen nDelimLen = aDelim.Len();
        if( nDelimLen )
        {
            xub_StrLen iSearch = STRING_NOTFOUND;
            xub_StrLen iStart = 0;
            do
            {
                bool bBreak = false;
                if( nCount >= 0 && nArraySize == nCount - 1 )
                    bBreak = true;

                iSearch = aExpression.Search( aDelim, iStart );
                String aSubStr;
                if( iSearch != STRING_NOTFOUND && !bBreak )
                {
                    aSubStr = aExpression.Copy( iStart, iSearch - iStart );
                    iStart = iSearch + nDelimLen;
                }
                else
                {
                    aSubStr = aExpression.Copy( iStart );
                }
                vRet.push_back( aSubStr );
                nArraySize++;

                if( bBreak )
                    break;
            }
            while( iSearch != STRING_NOTFOUND );
        }
        else
        {
            vRet.push_back( aExpression );
            nArraySize = 1;
        }
    }

    SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
    pArray->unoAddDim( 0, nArraySize-1 );

    // Parameter ins Array uebernehmen
    for( short i = 0 ; i < nArraySize ; i++ )
    {
        SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
        xVar->PutString( vRet[i] );
        pArray->Put( (SbxVariable*)xVar, &i );
    }

    // Array zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    sal_uInt16 nFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( NULL );
}

// MonthName(month[, abbreviate])
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

    Reference< XCalendar > xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( SbERR_INTERNAL_ERROR );
        return;
    }
    Sequence< CalendarItem > aMonthSeq = xCalendar->getMonths();
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

    const CalendarItem* pCalendarItems = aMonthSeq.getConstArray();
    const CalendarItem& rItem = pCalendarItems[nVal - 1];

    ::rtl::OUString aRetStr = ( bAbbreviate ? rItem.AbbrevName : rItem.FullName );
    rPar.Get(0)->PutString( String(aRetStr) );
}

// WeekdayName(weekday, abbreviate, firstdayofweek)
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

    Reference< XCalendar > xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( SbERR_INTERNAL_ERROR );
        return;
    }

    Sequence< CalendarItem > aDaySeq = xCalendar->getDays();
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
        nFirstDay = sal_Int16( xCalendar->getFirstDayOfWeek() + 1 );

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
            bAbbreviate = pPar2->GetBool();
    }

    const CalendarItem* pCalendarItems = aDaySeq.getConstArray();
    const CalendarItem& rItem = pCalendarItems[nDay - 1];

    ::rtl::OUString aRetStr = ( bAbbreviate ? rItem.AbbrevName : rItem.FullName );
    rPar.Get(0)->PutString( String(aRetStr) );
}

sal_Int16 implGetWeekDay( double aDate, bool bFirstDayParam = false, sal_Int16 nFirstDay = 0 )
{
    Date aRefDate( 1,1,1900 );
    long nDays = (long) aDate;
    nDays -= 2; // normieren: 1.1.1900 => 0
    aRefDate += nDays;
    DayOfWeek aDay = aRefDate.GetDayOfWeek();
    sal_Int16 nDay;
    if ( aDay != SUNDAY )
        nDay = (sal_Int16)aDay + 2;
    else
        nDay = 1;   // 1==Sonntag

    // #117253 Optional 2. parameter "firstdayofweek"
    if( bFirstDayParam )
    {
        if( nFirstDay < 0 || nFirstDay > 7 )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return 0;
        }
        if( nFirstDay == 0 )
        {
            Reference< XCalendar > xCalendar = getLocaleCalendar();
            if( !xCalendar.is() )
            {
                StarBASIC::Error( SbERR_INTERNAL_ERROR );
                return 0;
            }
            nFirstDay = sal_Int16( xCalendar->getFirstDayOfWeek() + 1 );
        }
        nDay = 1 + (nDay + 7 - nFirstDay) % 7;
    }
    return nDay;
}

RTLFUNC(Weekday)
{
    (void)pBasic;
    (void)bWrite;

    sal_uInt16 nParCount = rPar.Count();
    if ( nParCount < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
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
    INTERVAL_NONE,
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
    const char* mpStringCode;
    double      mdValue;
    bool        mbSimple;

    IntervalInfo( Interval eInterval, const char* pStringCode, double dValue, bool bSimple )
        : meInterval( eInterval )
        , mpStringCode( pStringCode )
        , mdValue( dValue )
        , mbSimple( bSimple )
    {}
};

static IntervalInfo pIntervalTable[] =
{
    IntervalInfo( INTERVAL_YYYY,    "yyyy",      0.0,               false ),    // Year
    IntervalInfo( INTERVAL_Q,       "q",         0.0,               false ),    // Quarter
    IntervalInfo( INTERVAL_M,       "m",         0.0,               false ),    // Month
    IntervalInfo( INTERVAL_Y,       "y",         1.0,               true ),     // Day of year
    IntervalInfo( INTERVAL_D,       "d",         1.0,               true ),     // Day
    IntervalInfo( INTERVAL_W,       "w",         1.0,               true ),     // Weekday
    IntervalInfo( INTERVAL_WW,      "ww",        7.0,               true ),     // Week
    IntervalInfo( INTERVAL_H,       "h",        (1.0 /    24.0),    true ),     // Hour
    IntervalInfo( INTERVAL_N,       "n",        (1.0 /  1440.0),    true),      // Minute
    IntervalInfo( INTERVAL_S,       "s",        (1.0 / 86400.0),    true ),     // Second
    IntervalInfo( INTERVAL_NONE, NULL, 0.0, false )
};

IntervalInfo* getIntervalInfo( const String& rStringCode )
{
    IntervalInfo* pInfo = NULL;
    sal_Int16 i = 0;
    while( (pInfo = pIntervalTable + i)->mpStringCode != NULL )
    {
        if( rStringCode.EqualsIgnoreCaseAscii( pInfo->mpStringCode ) )
            break;
        i++;
    }
    return pInfo;
}

// From methods.cxx
sal_Bool implDateSerial( sal_Int16 nYear, sal_Int16 nMonth, sal_Int16 nDay, double& rdRet );
sal_Int16 implGetDateDay( double aDate );
sal_Int16 implGetDateMonth( double aDate );
sal_Int16 implGetDateYear( double aDate );

sal_Int16 implGetHour( double dDate );
sal_Int16 implGetMinute( double dDate );
sal_Int16 implGetSecond( double dDate );


inline void implGetDayMonthYear( sal_Int16& rnYear, sal_Int16& rnMonth, sal_Int16& rnDay, double dDate )
{
    rnDay   = implGetDateDay( dDate );
    rnMonth = implGetDateMonth( dDate );
    rnYear  = implGetDateYear( dDate );
}

inline sal_Int16 limitToINT16( sal_Int32 n32 )
{
    if( n32 > 32767 )
        n32 = 32767;
    else if( n32 < -32768 )
        n32 = -32768;
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

    String aStringCode = rPar.Get(1)->GetString();
    IntervalInfo* pInfo = getIntervalInfo( aStringCode );
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
        // Keep hours, minutes, seconds
        double dHoursMinutesSeconds = dDate - floor( dDate );

        sal_Bool bOk = sal_True;
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
            // Overflow?
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

    // DateDiff(interval, date1, date2[, firstdayofweek[, firstweekofyear]])

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount < 4 || nParCount > 6 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    String aStringCode = rPar.Get(1)->GetString();
    IntervalInfo* pInfo = getIntervalInfo( aStringCode );
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
                sal_Int16 nFirstDay = 1;    // Default
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
                        Reference< XCalendar > xCalendar = getLocaleCalendar();
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
        case INTERVAL_NONE:
            break;
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

    Reference< XCalendar > xCalendar;
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

    // DatePart(interval, date[,firstdayofweek[, firstweekofyear]])

    sal_uInt16 nParCount = rPar.Count();
    if( nParCount < 3 || nParCount > 5 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    String aStringCode = rPar.Get(1)->GetString();
    IntervalInfo* pInfo = getIntervalInfo( aStringCode );
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
        case INTERVAL_NONE:
            break;
    }
    rPar.Get(0)->PutLong( nRet );
}

// FormatDateTime(Date[,NamedFormat])
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

    Reference< XCalendar > xCalendar = getLocaleCalendar();
    if( !xCalendar.is() )
    {
        StarBASIC::Error( SbERR_INTERNAL_ERROR );
        return;
    }

    String aRetStr;
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
            aRetStr = pSbxVar->GetString();
            break;

        // LongDate: Display a date using the long date format specified
        // in your computer's regional settings.
        // Tuesday, December 21, 2004
        // Dienstag, 21. December 2004
        case 1:
        {
            SvNumberFormatter* pFormatter = NULL;
            if( pINST )
                pFormatter = pINST->GetNumberFormatter();
            else
            {
                sal_uInt32 n;   // Dummy
                SbiInstance::PrepareNumberFormatter( pFormatter, n, n, n );
            }

            LanguageType eLangType = GetpApp()->GetSettings().GetLanguage();
            sal_uIntPtr nIndex = pFormatter->GetFormatIndex( NF_DATE_SYSTEM_LONG, eLangType );
            Color* pCol;
            pFormatter->GetOutputString( dDate, nIndex, aRetStr, &pCol );

            if( !pINST )
                delete pFormatter;

            break;
        }

        // ShortDate: Display a date using the short date format specified
        // in your computer's regional settings.
        // 12/21/2004
        // 21.12.2004
        case 2:
            pSbxVar->PutDate( floor(dDate) );
            aRetStr = pSbxVar->GetString();
            break;

        // LongTime: Display a time using the time format specified
        // in your computer's regional settings.
        // 11:24:50 AM
        // 12:13:51
        case 3:
        // ShortTime: Display a time using the 24-hour format (hh:mm).
        // 11:24
        case 4:
            double n;
            double dTime = modf( dDate, &n );
            pSbxVar->PutDate( dTime );
            if( nNamedFormat == 3 )
                aRetStr = pSbxVar->GetString();
            else
                aRetStr = pSbxVar->GetString().Copy( 0, 5 );
            break;
    }

    rPar.Get(0)->PutString( aRetStr );
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

    String aStr = pSbxVariable->GetString();
    aStr.Reverse();
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

    SbiInstance* pInst = pINST;
    if( pInst )
    {
        if ( nCount == 2 )
            pInst->EnableCompatibility( rPar.Get(1)->GetBool() );

        bEnabled = pInst->IsCompatibility();
    }
    rPar.Get(0)->PutBool( bEnabled );
}

RTLFUNC(Input)
{
    (void)pBasic;
    (void)bWrite;

    // 2 parameters needed
    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    sal_uInt16 nByteCount  = rPar.Get(1)->GetUShort();
    sal_Int16  nFileNumber = rPar.Get(2)->GetInteger();

    SbiIoSystem* pIosys = pINST->GetIoSystem();
    SbiStream* pSbStrm = pIosys->GetStream( nFileNumber );
    if ( !pSbStrm || !(pSbStrm->GetMode() & (SBSTRM_BINARY | SBSTRM_INPUT)) )
    {
        StarBASIC::Error( SbERR_BAD_CHANNEL );
        return;
    }

    ByteString aByteBuffer;
    SbError err = pSbStrm->Read( aByteBuffer, nByteCount, true );
    if( !err )
        err = pIosys->GetError();

    if( err )
    {
        StarBASIC::Error( err );
        return;
    }
    rPar.Get(0)->PutString( String( aByteBuffer, gsl_getSystemTextEncoding() ) );
}

// #115824
RTLFUNC(Me)
{
    (void)pBasic;
    (void)bWrite;

    SbModule* pActiveModule = pINST->GetActiveModule();
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

