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
#include <osl/mutex.hxx>
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

//! SearchWords sucht in ganzen Zellen - umbenennen ???

//  SfxItemPropertyMapEntry nur fuer GetPropertySetInfo

static const SfxItemPropertyMapEntry* lcl_GetSearchPropertyMap()
{
    static const SfxItemPropertyMapEntry aSearchPropertyMap_Impl[] =
    {
        {OUString(SC_UNO_SRCHBACK),     0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHBYROW),    0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHCASE),     0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHREGEXP),   0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHSIM),      0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHSIMADD),   0,      cppu::UnoType<sal_Int16>::get(), 0, 0},
        {OUString(SC_UNO_SRCHSIMEX),    0,      cppu::UnoType<sal_Int16>::get(), 0, 0},
        {OUString(SC_UNO_SRCHSIMREL),   0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHSIMREM),   0,      cppu::UnoType<sal_Int16>::get(), 0, 0},
        {OUString(SC_UNO_SRCHSTYLES),   0,      cppu::UnoType<bool>::get(),       0, 0},
        {OUString(SC_UNO_SRCHTYPE),     0,      cppu::UnoType<sal_Int16>::get(), 0, 0}, // enum TableSearch ist weg
        {OUString(SC_UNO_SRCHWORDS),    0,      cppu::UnoType<bool>::get(),       0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aSearchPropertyMap_Impl;
}

#define SCSEARCHDESCRIPTOR_SERVICE      "com.sun.star.util.SearchDescriptor"
#define SCREPLACEDESCRIPTOR_SERVICE     "com.sun.star.util.ReplaceDescriptor"

ScCellSearchObj::ScCellSearchObj() :
    aPropSet(lcl_GetSearchPropertyMap())
{
    pSearchItem = new SvxSearchItem( SCITEM_SEARCHDATA );
    //  Defaults:
    pSearchItem->SetWordOnly(false);
    pSearchItem->SetExact(false);
    pSearchItem->SetMatchFullHalfWidthForms(false);
    pSearchItem->SetUseAsianOptions(false);     // or all asian bits would have to be handled
    pSearchItem->SetBackward(false);
    pSearchItem->SetSelection(false);
    pSearchItem->SetRegExp(false);
    pSearchItem->SetPattern(false);
    pSearchItem->SetLevenshtein(false);
    pSearchItem->SetLEVRelaxed(false);
    pSearchItem->SetLEVOther(2);
    pSearchItem->SetLEVShorter(2);
    pSearchItem->SetLEVLonger(2);
    //  Calc-Flags
    pSearchItem->SetRowDirection(false);
    pSearchItem->SetCellType(SvxSearchCellType::FORMULA);

    //  Selection-Flag wird beim Aufruf gesetzt
}

ScCellSearchObj::~ScCellSearchObj()
{
    delete pSearchItem;
}

// XSearchDescriptor

OUString SAL_CALL ScCellSearchObj::getSearchString() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return pSearchItem->GetSearchString();
}

void SAL_CALL ScCellSearchObj::setSearchString( const OUString& aString )
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    pSearchItem->SetSearchString( aString );
}

// XReplaceDescriptor

OUString SAL_CALL ScCellSearchObj::getReplaceString() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return pSearchItem->GetReplaceString();
}

void SAL_CALL ScCellSearchObj::setReplaceString( const OUString& aReplaceString )
                                                    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    pSearchItem->SetReplaceString( aReplaceString );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellSearchObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScCellSearchObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aString(aPropertyName);

    if (aString == SC_UNO_SRCHBACK)        pSearchItem->SetBackward( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString == SC_UNO_SRCHBYROW)  pSearchItem->SetRowDirection( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString == SC_UNO_SRCHCASE)   pSearchItem->SetExact( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString == SC_UNO_SRCHREGEXP) pSearchItem->SetRegExp( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString == SC_UNO_SRCHSIM)    pSearchItem->SetLevenshtein( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString == SC_UNO_SRCHSIMREL) pSearchItem->SetLEVRelaxed( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString == SC_UNO_SRCHSTYLES) pSearchItem->SetPattern( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString == SC_UNO_SRCHWORDS)  pSearchItem->SetWordOnly( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString == SC_UNO_SRCHSIMADD) pSearchItem->SetLEVLonger( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aString == SC_UNO_SRCHSIMEX)  pSearchItem->SetLEVOther( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aString == SC_UNO_SRCHSIMREM) pSearchItem->SetLEVShorter( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aString == SC_UNO_SRCHTYPE)   pSearchItem->SetCellType( static_cast<SvxSearchCellType>(ScUnoHelpFunctions::GetInt16FromAny( aValue )) );
    else if (aString == SC_UNO_SRCHFILTERED) pSearchItem->SetSearchFiltered( ScUnoHelpFunctions::GetBoolFromAny(aValue) );
    else if (aString == SC_UNO_SRCHFORMATTED) pSearchItem->SetSearchFormatted( ScUnoHelpFunctions::GetBoolFromAny(aValue) );
}

uno::Any SAL_CALL ScCellSearchObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aString(aPropertyName);
    uno::Any aRet;

    if (aString == SC_UNO_SRCHBACK)        ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetBackward() );
    else if (aString == SC_UNO_SRCHBYROW)  ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetRowDirection() );
    else if (aString == SC_UNO_SRCHCASE)   ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetExact() );
    else if (aString == SC_UNO_SRCHREGEXP) ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetRegExp() );
    else if (aString == SC_UNO_SRCHSIM)    ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->IsLevenshtein() );
    else if (aString == SC_UNO_SRCHSIMREL) ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->IsLEVRelaxed() );
    else if (aString == SC_UNO_SRCHSTYLES) ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetPattern() );
    else if (aString == SC_UNO_SRCHWORDS)  ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetWordOnly() );
    else if (aString == SC_UNO_SRCHSIMADD) aRet <<= (sal_Int16) pSearchItem->GetLEVLonger();
    else if (aString == SC_UNO_SRCHSIMEX)  aRet <<= (sal_Int16) pSearchItem->GetLEVOther();
    else if (aString == SC_UNO_SRCHSIMREM) aRet <<= (sal_Int16) pSearchItem->GetLEVShorter();
    else if (aString == SC_UNO_SRCHTYPE)   aRet <<= (sal_Int16) pSearchItem->GetCellType();
    else if (aString == SC_UNO_SRCHFILTERED) ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->IsSearchFiltered() );
    else if (aString == SC_UNO_SRCHFORMATTED) ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->IsSearchFormatted() );

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScCellSearchObj )

// XServiceInfo

OUString SAL_CALL ScCellSearchObj::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString( "ScCellSearchObj" );
}

sal_Bool SAL_CALL ScCellSearchObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScCellSearchObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = SCSEARCHDESCRIPTOR_SERVICE;
    pArray[1] = SCREPLACEDESCRIPTOR_SERVICE;
    return aRet;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScCellSearchObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
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
