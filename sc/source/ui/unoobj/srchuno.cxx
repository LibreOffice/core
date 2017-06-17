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

#include "scitems.hxx"
#include <svl/srchitem.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "srchuno.hxx"
#include "docsh.hxx"
#include "undoblk.hxx"
#include "hints.hxx"
#include "markdata.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"

using namespace com::sun::star;

//! SearchWords searches in whole cells - rename it ???

//  SfxItemPropertyMapEntry only for GetPropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetSearchPropertyMap()
{
    static const SfxItemPropertyMapEntry aSearchPropertyMap_Impl[] =
    {
        {OUString(SC_UNO_SRCHBACK),     0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHBYROW),    0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHCASE),     0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHREGEXP),   0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHWILDCARD), 0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHSIM),      0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHSIMADD),   0,      cppu::UnoType<sal_Int16>::get(), 0, 0},
        {OUString(SC_UNO_SRCHSIMEX),    0,      cppu::UnoType<sal_Int16>::get(), 0, 0},
        {OUString(SC_UNO_SRCHSIMREL),   0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHSIMREM),   0,      cppu::UnoType<sal_Int16>::get(), 0, 0},
        {OUString(SC_UNO_SRCHSTYLES),   0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHTYPE),     0,      cppu::UnoType<sal_Int16>::get(), 0, 0}, // enum TableSearch is gone
        {OUString(SC_UNO_SRCHWORDS),    0,      cppu::UnoType<bool>::get(),       0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aSearchPropertyMap_Impl;
}

#define SCSEARCHDESCRIPTOR_SERVICE      "com.sun.star.util.SearchDescriptor"
#define SCREPLACEDESCRIPTOR_SERVICE     "com.sun.star.util.ReplaceDescriptor"

ScCellSearchObj::ScCellSearchObj() :
    aPropSet(lcl_GetSearchPropertyMap()),
    pSearchItem( new SvxSearchItem( SCITEM_SEARCHDATA ) )
{
    //  Defaults:
    pSearchItem->SetWordOnly(false);
    pSearchItem->SetExact(false);
    pSearchItem->SetMatchFullHalfWidthForms(false);
    pSearchItem->SetUseAsianOptions(false);     // or all asian bits would have to be handled
    pSearchItem->SetBackward(false);
    pSearchItem->SetSelection(false);
    pSearchItem->SetRegExp(false);
    pSearchItem->SetWildcard(false);
    pSearchItem->SetPattern(false);
    pSearchItem->SetLevenshtein(false);
    pSearchItem->SetLEVRelaxed(false);
    pSearchItem->SetLEVOther(2);
    pSearchItem->SetLEVShorter(2);
    pSearchItem->SetLEVLonger(2);
    //  Calc-Flags
    pSearchItem->SetRowDirection(false);
    pSearchItem->SetCellType(SvxSearchCellType::FORMULA);

    //  Selection-Flag will be set when this is called
}

ScCellSearchObj::~ScCellSearchObj()
{
}

// XSearchDescriptor

OUString SAL_CALL ScCellSearchObj::getSearchString()
{
    SolarMutexGuard aGuard;
    return pSearchItem->GetSearchString();
}

void SAL_CALL ScCellSearchObj::setSearchString( const OUString& aString )
{
    SolarMutexGuard aGuard;
    pSearchItem->SetSearchString( aString );
}

// XReplaceDescriptor

OUString SAL_CALL ScCellSearchObj::getReplaceString()
{
    SolarMutexGuard aGuard;
    return pSearchItem->GetReplaceString();
}

void SAL_CALL ScCellSearchObj::setReplaceString( const OUString& aReplaceString )
{
    SolarMutexGuard aGuard;
    pSearchItem->SetReplaceString( aReplaceString );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellSearchObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScCellSearchObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    if (aPropertyName == SC_UNO_SRCHBACK)        pSearchItem->SetBackward( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHBYROW)  pSearchItem->SetRowDirection( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHCASE)   pSearchItem->SetExact( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHREGEXP) pSearchItem->SetRegExp( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHWILDCARD) pSearchItem->SetWildcard( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHSIM)    pSearchItem->SetLevenshtein( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHSIMREL) pSearchItem->SetLEVRelaxed( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHSTYLES) pSearchItem->SetPattern( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHWORDS)  pSearchItem->SetWordOnly( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHSIMADD) pSearchItem->SetLEVLonger( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHSIMEX)  pSearchItem->SetLEVOther( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHSIMREM) pSearchItem->SetLEVShorter( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aPropertyName == SC_UNO_SRCHTYPE)   pSearchItem->SetCellType( static_cast<SvxSearchCellType>(ScUnoHelpFunctions::GetInt16FromAny( aValue )) );
    else if (aPropertyName == SC_UNO_SRCHFILTERED) pSearchItem->SetSearchFiltered( ScUnoHelpFunctions::GetBoolFromAny(aValue) );
    else if (aPropertyName == SC_UNO_SRCHFORMATTED) pSearchItem->SetSearchFormatted( ScUnoHelpFunctions::GetBoolFromAny(aValue) );
}

uno::Any SAL_CALL ScCellSearchObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    if (aPropertyName == SC_UNO_SRCHBACK)        aRet <<= pSearchItem->GetBackward();
    else if (aPropertyName == SC_UNO_SRCHBYROW)  aRet <<= pSearchItem->GetRowDirection();
    else if (aPropertyName == SC_UNO_SRCHCASE)   aRet <<= pSearchItem->GetExact();
    else if (aPropertyName == SC_UNO_SRCHREGEXP) aRet <<= pSearchItem->GetRegExp();
    else if (aPropertyName == SC_UNO_SRCHWILDCARD) aRet <<= pSearchItem->GetWildcard();
    else if (aPropertyName == SC_UNO_SRCHSIM)    aRet <<= pSearchItem->IsLevenshtein();
    else if (aPropertyName == SC_UNO_SRCHSIMREL) aRet <<= pSearchItem->IsLEVRelaxed();
    else if (aPropertyName == SC_UNO_SRCHSTYLES) aRet <<= pSearchItem->GetPattern();
    else if (aPropertyName == SC_UNO_SRCHWORDS)  aRet <<= pSearchItem->GetWordOnly();
    else if (aPropertyName == SC_UNO_SRCHSIMADD) aRet <<= (sal_Int16) pSearchItem->GetLEVLonger();
    else if (aPropertyName == SC_UNO_SRCHSIMEX)  aRet <<= (sal_Int16) pSearchItem->GetLEVOther();
    else if (aPropertyName == SC_UNO_SRCHSIMREM) aRet <<= (sal_Int16) pSearchItem->GetLEVShorter();
    else if (aPropertyName == SC_UNO_SRCHTYPE)   aRet <<= (sal_Int16) pSearchItem->GetCellType();
    else if (aPropertyName == SC_UNO_SRCHFILTERED) aRet <<= pSearchItem->IsSearchFiltered();
    else if (aPropertyName == SC_UNO_SRCHFORMATTED) aRet <<= pSearchItem->IsSearchFormatted();

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScCellSearchObj )

// XServiceInfo

OUString SAL_CALL ScCellSearchObj::getImplementationName()
{
    return OUString( "ScCellSearchObj" );
}

sal_Bool SAL_CALL ScCellSearchObj::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScCellSearchObj::getSupportedServiceNames()
{
    return {SCSEARCHDESCRIPTOR_SERVICE, SCREPLACEDESCRIPTOR_SERVICE};
}

// XUnoTunnel

sal_Int64 SAL_CALL ScCellSearchObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId )
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScCellSearchObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScCellSearchObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScCellSearchObj::getUnoTunnelId()
{
    return theScCellSearchObjUnoTunnelId::get().getSeq();
}

ScCellSearchObj* ScCellSearchObj::getImplementation(const uno::Reference<util::XSearchDescriptor>& rObj)
{
    ScCellSearchObj* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<ScCellSearchObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
