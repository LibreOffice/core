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

#include <appluno.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>
#include <formula/funcvarargs.h>

#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <miscuno.hxx>
#include <scmod.hxx>
#include <appoptio.hxx>
#include <inputopt.hxx>
#include <printopt.hxx>
#include <userlist.hxx>
#include <scdll.hxx>
#include <unonames.hxx>
#include <funcdesc.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/sheet/FunctionArgument.hpp>
#include <memory>

using namespace com::sun::star;

//  Special value for zoom
//! somewhere central
#define SC_ZOOMVAL_OPTIMAL      (-1)
#define SC_ZOOMVAL_WHOLEPAGE    (-2)
#define SC_ZOOMVAL_PAGEWIDTH    (-3)

//  Number of PropertyValues in a function description
#define SC_FUNCDESC_PROPCOUNT   5

//  everything without Which-ID, map only for PropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetSettingsPropertyMap()
{
    static const SfxItemPropertyMapEntry aSettingsPropertyMap_Impl[] =
    {
        {u"" SC_UNONAME_DOAUTOCP, 0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_ENTERED,  0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_EXPREF,   0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_EXTFMT,   0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_LINKUPD,  0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        {u"" SC_UNONAME_MARKHDR,  0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_METRIC,   0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        {u"" SC_UNONAME_MOVEDIR,  0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        {u"" SC_UNONAME_MOVESEL,  0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_PRALLSH,  0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_PREMPTY,  0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_RANGEFIN, 0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_SCALE,    0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        {u"" SC_UNONAME_STBFUNC,  0,  cppu::UnoType<sal_Int16>::get(),        0, 0},
        {u"" SC_UNONAME_ULISTS,   0,  cppu::UnoType<uno::Sequence<OUString>>::get(), 0, 0},
        {u"" SC_UNONAME_PRMETRICS,0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_USETABCOL,0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"" SC_UNONAME_REPLWARN, 0,  cppu::UnoType<bool>::get(),              0, 0},
        {u"", 0, css::uno::Type(), 0, 0 }
    };
    return aSettingsPropertyMap_Impl;
}

#define SCFUNCTIONLISTOBJ_SERVICE       "com.sun.star.sheet.FunctionDescriptions"
#define SCRECENTFUNCTIONSOBJ_SERVICE    "com.sun.star.sheet.RecentFunctions"
#define SCSPREADSHEETSETTINGS_SERVICE   "com.sun.star.sheet.GlobalSheetSettings"

SC_SIMPLE_SERVICE_INFO( ScFunctionListObj, "stardiv.StarCalc.ScFunctionListObj", SCFUNCTIONLISTOBJ_SERVICE )
SC_SIMPLE_SERVICE_INFO( ScRecentFunctionsObj, "stardiv.StarCalc.ScRecentFunctionsObj", SCRECENTFUNCTIONSOBJ_SERVICE )
SC_SIMPLE_SERVICE_INFO( ScSpreadsheetSettings, "stardiv.StarCalc.ScSpreadsheetSettings", SCSPREADSHEETSETTINGS_SERVICE )


ScSpreadsheetSettings::ScSpreadsheetSettings() :
    aPropSet( lcl_GetSettingsPropertyMap() )
{
}

ScSpreadsheetSettings::~ScSpreadsheetSettings()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Calc_ScSpreadsheetSettings_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    return cppu::acquire(new ScSpreadsheetSettings());
}


bool ScSpreadsheetSettings::getPropertyBool(const OUString& aPropertyName)
{
   uno::Any any = getPropertyValue(aPropertyName);
   bool b = false;
   any >>= b;
   return b;
}

sal_Int16 ScSpreadsheetSettings::getPropertyInt16(const OUString& aPropertyName)
{
   uno::Any any = getPropertyValue(aPropertyName);
   sal_Int16 b = 0;
   any >>= b;
   return b;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScSpreadsheetSettings::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScSpreadsheetSettings::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    ScModule* pScMod = SC_MOD();
    ScAppOptions   aAppOpt(pScMod->GetAppOptions());
    ScInputOptions aInpOpt(pScMod->GetInputOptions());
    bool bSaveApp = false;
    bool bSaveInp = false;
    // print options aren't loaded until needed

    if (aPropertyName == SC_UNONAME_DOAUTOCP)
    {
        aAppOpt.SetAutoComplete( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveApp = true;
    }
    else if (aPropertyName == SC_UNONAME_ENTERED)
    {
        aInpOpt.SetEnterEdit( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_EXPREF)
    {
        aInpOpt.SetExpandRefs( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_EXTFMT)
    {
        aInpOpt.SetExtendFormat( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_LINKUPD)
    {
        // XXX NOTE: this is not css::document::Settings property
        // LinkUpdateMode but css::sheet::XGlobalSheetSettings attribute
        // LinkUpdateMode.
        sal_Int16 n;
        if (!(aValue >>= n) || n < 0 || n >= ScLkUpdMode::LM_UNKNOWN)
        {
            throw css::lang::IllegalArgumentException(
                ("LinkUpdateMode property value must be a SHORT with a value in the range of 0--2"
                 " as documented for css::sheet::XGlobalSheetSettings attribute LinkUpdateMode"),
                css::uno::Reference<css::uno::XInterface>(), -1);
        }
        aAppOpt.SetLinkMode( static_cast<ScLkUpdMode>(n) );
        bSaveApp = true;
    }
    else if (aPropertyName == SC_UNONAME_MARKHDR)
    {
        aInpOpt.SetMarkHeader( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_MOVESEL)
    {
        aInpOpt.SetMoveSelection( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_RANGEFIN)
    {
        aInpOpt.SetRangeFinder( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_USETABCOL)
    {
        aInpOpt.SetUseTabCol( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_PRMETRICS)
    {
        aInpOpt.SetTextWysiwyg( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_REPLWARN)
    {
        aInpOpt.SetReplaceCellsWarn( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_METRIC)
    {
        aAppOpt.SetAppMetric( static_cast<FieldUnit>(ScUnoHelpFunctions::GetInt16FromAny( aValue )) );
        bSaveApp = true;
    }
    else if (aPropertyName == SC_UNONAME_MOVEDIR)
    {
        aInpOpt.SetMoveDir( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveInp = true;
    }
    else if (aPropertyName == SC_UNONAME_SCALE)
    {
        short nVal = ScUnoHelpFunctions::GetInt16FromAny( aValue );
        if ( nVal < 0 )
        {
            SvxZoomType eType = SvxZoomType::PERCENT;
            switch (nVal)
            {
                case SC_ZOOMVAL_OPTIMAL:    eType = SvxZoomType::OPTIMAL;   break;
                case SC_ZOOMVAL_WHOLEPAGE:  eType = SvxZoomType::WHOLEPAGE; break;
                case SC_ZOOMVAL_PAGEWIDTH:  eType = SvxZoomType::PAGEWIDTH; break;
            }
            aAppOpt.SetZoomType( eType );
        }
        else if ( nVal >= MINZOOM && nVal <= MAXZOOM )
        {
            aAppOpt.SetZoom( nVal );
            aAppOpt.SetZoomType( SvxZoomType::PERCENT );
        }
        bSaveApp = true;
    }
    else if (aPropertyName == SC_UNONAME_STBFUNC)
    {
        aAppOpt.SetStatusFunc( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
        bSaveApp = true;
    }
    else if (aPropertyName == SC_UNONAME_ULISTS)
    {
        ScUserList* pUserList = ScGlobal::GetUserList();
        uno::Sequence<OUString> aSeq;
        if ( pUserList && ( aValue >>= aSeq ) )
        {
            //  directly change the active list
            //  ScGlobal::SetUseTabCol does not do much else

            pUserList->clear();
            for (const OUString& aEntry : std::as_const(aSeq))
            {
                ScUserListData* pData = new ScUserListData(aEntry);
                pUserList->push_back(pData);
            }
            bSaveApp = true;    // List with App-Options are saved
        }
    }
    else if (aPropertyName == SC_UNONAME_PRALLSH)
    {
        ScPrintOptions aPrintOpt(pScMod->GetPrintOptions());
        aPrintOpt.SetAllSheets( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        pScMod->SetPrintOptions( aPrintOpt );
    }
    else if (aPropertyName == SC_UNONAME_PREMPTY)
    {
        ScPrintOptions aPrintOpt(pScMod->GetPrintOptions());
        aPrintOpt.SetSkipEmpty( !ScUnoHelpFunctions::GetBoolFromAny( aValue ) );    // reversed
        pScMod->SetPrintOptions( aPrintOpt );
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScPrintOptions ) );    // update previews
    }

    if ( bSaveApp )
        pScMod->SetAppOptions( aAppOpt );
    if ( bSaveInp )
        pScMod->SetInputOptions( aInpOpt );
}

uno::Any SAL_CALL ScSpreadsheetSettings::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    ScModule* pScMod = SC_MOD();
    ScAppOptions   aAppOpt = pScMod->GetAppOptions();
    ScInputOptions aInpOpt = pScMod->GetInputOptions();
    // print options aren't loaded until needed

    if (aPropertyName == SC_UNONAME_DOAUTOCP) aRet <<= aAppOpt.GetAutoComplete();
    else if (aPropertyName == SC_UNONAME_ENTERED ) aRet <<= aInpOpt.GetEnterEdit();
    else if (aPropertyName == SC_UNONAME_EXPREF ) aRet <<= aInpOpt.GetExpandRefs();
    else if (aPropertyName == SC_UNONAME_EXTFMT ) aRet <<= aInpOpt.GetExtendFormat();
    else if (aPropertyName == SC_UNONAME_LINKUPD ) aRet <<= static_cast<sal_Int16>(aAppOpt.GetLinkMode());
    else if (aPropertyName == SC_UNONAME_MARKHDR ) aRet <<= aInpOpt.GetMarkHeader();
    else if (aPropertyName == SC_UNONAME_MOVESEL ) aRet <<= aInpOpt.GetMoveSelection();
    else if (aPropertyName == SC_UNONAME_RANGEFIN ) aRet <<= aInpOpt.GetRangeFinder();
    else if (aPropertyName == SC_UNONAME_USETABCOL ) aRet <<= aInpOpt.GetUseTabCol();
    else if (aPropertyName == SC_UNONAME_PRMETRICS ) aRet <<= aInpOpt.GetTextWysiwyg();
    else if (aPropertyName == SC_UNONAME_REPLWARN ) aRet <<= aInpOpt.GetReplaceCellsWarn();
    else if (aPropertyName == SC_UNONAME_METRIC )  aRet <<= static_cast<sal_Int16>(aAppOpt.GetAppMetric());
    else if (aPropertyName == SC_UNONAME_MOVEDIR ) aRet <<= static_cast<sal_Int16>(aInpOpt.GetMoveDir());
    else if (aPropertyName == SC_UNONAME_STBFUNC ) aRet <<= static_cast<sal_Int16>(aAppOpt.GetStatusFunc());
    else if (aPropertyName == SC_UNONAME_SCALE )
    {
        sal_Int16 nZoomVal = 0;
        switch ( aAppOpt.GetZoomType() )
        {
            case SvxZoomType::PERCENT:   nZoomVal = aAppOpt.GetZoom();    break;
            case SvxZoomType::OPTIMAL:   nZoomVal = SC_ZOOMVAL_OPTIMAL;   break;
            case SvxZoomType::WHOLEPAGE: nZoomVal = SC_ZOOMVAL_WHOLEPAGE; break;
            case SvxZoomType::PAGEWIDTH: nZoomVal = SC_ZOOMVAL_PAGEWIDTH; break;
            default:
            {
                // added to avoid warnings
            }
        }
        aRet <<= nZoomVal;
    }
    else if (aPropertyName == SC_UNONAME_ULISTS )
    {
        ScUserList* pUserList = ScGlobal::GetUserList();
        if (pUserList)
        {
            size_t nCount = pUserList->size();
            uno::Sequence<OUString> aSeq(nCount);
            OUString* pAry = aSeq.getArray();
            for (size_t i=0; i<nCount; ++i)
            {
                OUString aEntry((*pUserList)[i].GetString());
                pAry[i] = aEntry;
            }
            aRet <<= aSeq;
        }
    }
    else if (aPropertyName == SC_UNONAME_PRALLSH )
        aRet <<= pScMod->GetPrintOptions().GetAllSheets();
    else if (aPropertyName == SC_UNONAME_PREMPTY )
        aRet <<= !pScMod->GetPrintOptions().GetSkipEmpty();    // reversed

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScSpreadsheetSettings )

ScRecentFunctionsObj::ScRecentFunctionsObj()
{
}

ScRecentFunctionsObj::~ScRecentFunctionsObj()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ScRecentFunctionsObj_get_implementation(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    return cppu::acquire(new ScRecentFunctionsObj());
}

// XRecentFunctions

uno::Sequence<sal_Int32> SAL_CALL ScRecentFunctionsObj::getRecentFunctionIds()
{
    SolarMutexGuard aGuard;
    const ScAppOptions& rOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nCount = rOpt.GetLRUFuncListCount();
    const sal_uInt16* pFuncs = rOpt.GetLRUFuncList();
    if (pFuncs)
    {
        uno::Sequence<sal_Int32> aSeq(nCount);
        sal_Int32* pAry = aSeq.getArray();
        for (sal_uInt16 i=0; i<nCount; i++)
            pAry[i] = pFuncs[i];
        return aSeq;
    }
    return uno::Sequence<sal_Int32>(0);
}

void SAL_CALL ScRecentFunctionsObj::setRecentFunctionIds(
                    const uno::Sequence<sal_Int32>& aRecentFunctionIds )
{
    SolarMutexGuard aGuard;
    sal_uInt16 nCount = static_cast<sal_uInt16>(std::min( aRecentFunctionIds.getLength(), sal_Int32(LRU_MAX) ));
    const sal_Int32* pAry = aRecentFunctionIds.getConstArray();

    std::unique_ptr<sal_uInt16[]> pFuncs(nCount ? new sal_uInt16[nCount] : nullptr);
    for (sal_uInt16 i=0; i<nCount; i++)
        pFuncs[i] = static_cast<sal_uInt16>(pAry[i]);        //! check for valid values?

    ScModule* pScMod = SC_MOD();
    ScAppOptions aNewOpts(pScMod->GetAppOptions());
    aNewOpts.SetLRUFuncList(pFuncs.get(), nCount);
    pScMod->SetAppOptions(aNewOpts);
}

sal_Int32 SAL_CALL ScRecentFunctionsObj::getMaxRecentFunctions()
{
    return LRU_MAX;
}

ScFunctionListObj::ScFunctionListObj()
{
}

ScFunctionListObj::~ScFunctionListObj()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ScFunctionListObj_get_implementation(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;
    ScDLL::Init();
    return cppu::acquire(new ScFunctionListObj());
}

static void lcl_FillSequence( uno::Sequence<beans::PropertyValue>& rSequence, const ScFuncDesc& rDesc )
{
    rDesc.initArgumentInfo();   // full argument info is needed

    OSL_ENSURE( rSequence.getLength() == SC_FUNCDESC_PROPCOUNT, "Wrong count" );

    beans::PropertyValue* pArray = rSequence.getArray();

    pArray[0].Name = SC_UNONAME_ID;
    pArray[0].Value <<= static_cast<sal_Int32>(rDesc.nFIndex);

    pArray[1].Name = SC_UNONAME_CATEGORY;
    pArray[1].Value <<= static_cast<sal_Int32>(rDesc.nCategory);

    pArray[2].Name = SC_UNONAME_NAME;
    if (rDesc.mxFuncName)
        pArray[2].Value <<= *rDesc.mxFuncName;

    pArray[3].Name = SC_UNONAME_DESCRIPTION;
    if (rDesc.mxFuncDesc)
        pArray[3].Value <<= *rDesc.mxFuncDesc;

    pArray[4].Name = SC_UNONAME_ARGUMENTS;
    if (rDesc.maDefArgNames.empty() || rDesc.maDefArgDescs.empty() || !rDesc.pDefArgFlags)
        return;

    sal_uInt16 nCount = rDesc.nArgCount;
    if (nCount >= PAIRED_VAR_ARGS)
        nCount -= PAIRED_VAR_ARGS - 2;
    else if (nCount >= VAR_ARGS)
        nCount -= VAR_ARGS - 1;
    sal_uInt16 nSeqCount = rDesc.GetSuppressedArgCount();
    if (nSeqCount >= PAIRED_VAR_ARGS)
        nSeqCount -= PAIRED_VAR_ARGS - 2;
    else if (nSeqCount >= VAR_ARGS)
        nSeqCount -= VAR_ARGS - 1;

    if (!nSeqCount)
        return;

    uno::Sequence<sheet::FunctionArgument> aArgSeq(nSeqCount);
    sheet::FunctionArgument* pArgAry = aArgSeq.getArray();
    for (sal_uInt16 i=0, j=0; i<nCount; i++)
    {
        sheet::FunctionArgument aArgument;
        aArgument.Name        = rDesc.maDefArgNames[i];
        aArgument.Description = rDesc.maDefArgDescs[i];
        aArgument.IsOptional  = rDesc.pDefArgFlags[i].bOptional;
        pArgAry[j++] = aArgument;
    }
    pArray[4].Value <<= aArgSeq;
}

// XFunctionDescriptions

uno::Sequence<beans::PropertyValue> SAL_CALL ScFunctionListObj::getById( sal_Int32 nId )
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( !pFuncList )
        throw uno::RuntimeException();                  // should not happen

    sal_uInt16 nCount = static_cast<sal_uInt16>(pFuncList->GetCount());
    for (sal_uInt16 nIndex=0; nIndex<nCount; nIndex++)
    {
        const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
        if ( pDesc && pDesc->nFIndex == nId )
        {
            uno::Sequence<beans::PropertyValue> aSeq( SC_FUNCDESC_PROPCOUNT );
            lcl_FillSequence( aSeq, *pDesc );
            return aSeq;
        }
    }

    throw lang::IllegalArgumentException();         // not found
}

// XNameAccess

uno::Any SAL_CALL ScFunctionListObj::getByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( !pFuncList )
        throw uno::RuntimeException();                  // should not happen

    sal_uInt16 nCount = static_cast<sal_uInt16>(pFuncList->GetCount());
    for (sal_uInt16 nIndex=0; nIndex<nCount; nIndex++)
    {
        const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
        //! Case-insensitive???
        if ( pDesc && pDesc->mxFuncName && aName == *pDesc->mxFuncName )
        {
            uno::Sequence<beans::PropertyValue> aSeq( SC_FUNCDESC_PROPCOUNT );
            lcl_FillSequence( aSeq, *pDesc );
            return uno::makeAny(aSeq);
        }
    }

    throw container::NoSuchElementException();      // not found
}

// XIndexAccess

sal_Int32 SAL_CALL ScFunctionListObj::getCount()
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = 0;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
        nCount = static_cast<sal_Int32>(pFuncList->GetCount());
    return nCount;
}

uno::Any SAL_CALL ScFunctionListObj::getByIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( !pFuncList )
        throw uno::RuntimeException();                  // should not happen

    if ( nIndex >= 0 && nIndex < static_cast<sal_Int32>(pFuncList->GetCount()) )
    {
        const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
        if ( pDesc )
        {
            uno::Sequence<beans::PropertyValue> aSeq( SC_FUNCDESC_PROPCOUNT );
            lcl_FillSequence( aSeq, *pDesc );
            return uno::makeAny(aSeq);
        }
    }

    throw lang::IndexOutOfBoundsException();        // illegal index
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScFunctionListObj::createEnumeration()
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, "com.sun.star.sheet.FunctionDescriptionEnumeration");
}

// XElementAccess

uno::Type SAL_CALL ScFunctionListObj::getElementType()
{
    SolarMutexGuard aGuard;
    return cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get();
}

sal_Bool SAL_CALL ScFunctionListObj::hasElements()
{
    SolarMutexGuard aGuard;
    return ( getCount() > 0 );
}

uno::Sequence<OUString> SAL_CALL ScFunctionListObj::getElementNames()
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uInt32 nCount = pFuncList->GetCount();
        uno::Sequence<OUString> aSeq(nCount);
        OUString* pAry = aSeq.getArray();
        for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            if ( pDesc && pDesc->mxFuncName )
                pAry[nIndex] = *pDesc->mxFuncName;
        }
        return aSeq;
    }
    return uno::Sequence<OUString>(0);
}

sal_Bool SAL_CALL ScFunctionListObj::hasByName( const OUString& aName )
{
    SolarMutexGuard aGuard;
    const ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    if ( pFuncList )
    {
        sal_uInt32 nCount = pFuncList->GetCount();
        for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            const ScFuncDesc* pDesc = pFuncList->GetFunction(nIndex);
            //! Case-insensitive???
            if ( pDesc && pDesc->mxFuncName && aName == *pDesc->mxFuncName )
                return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
