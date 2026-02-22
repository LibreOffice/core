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

#include <svl/itemprop.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/util/Date.hpp>

#include <optuno.hxx>
#include <miscuno.hxx>
#include <docoptio.hxx>

using namespace com::sun::star;

bool ScDocOptionsHelper::setPropertyValue( ScDocOptions& rOptions,
                const SfxItemPropertyMap& rPropMap,
                std::u16string_view aPropertyName, const uno::Any& aValue )
{
    //! use map (with new identifiers)

    const SfxItemPropertyMapEntry* pEntry = rPropMap.getByName(aPropertyName );
    if( !pEntry || !pEntry->nWID )
        return false;
    switch( static_cast<ScPropUnoId>(pEntry->nWID) )
    {
        case ScPropUnoId::CalcAsShown:
            rOptions.SetCalcAsShown( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case ScPropUnoId::DefTabStop:
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
                rOptions.SetTabDistance( nIntVal );
        }
        break;
        case ScPropUnoId::IgnoreCase:
            rOptions.SetIgnoreCase( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case ScPropUnoId::IterEnabled:
            rOptions.SetIter( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case ScPropUnoId::IterCount:
        {
            sal_Int32 nIntVal = 0;
            if ( aValue >>= nIntVal )
                rOptions.SetIterCount( static_cast<sal_uInt16>(nIntVal) );
        }
        break;
        case ScPropUnoId::IterEpsilon:
        {
            double fDoubleVal = 0;
            if ( aValue >>= fDoubleVal )
                rOptions.SetIterEps( fDoubleVal );
        }
        break;
        case ScPropUnoId::LookupLabels:
            rOptions.SetLookUpColRowNames( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case ScPropUnoId::MatchWhole:
            rOptions.SetMatchWholeCell( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case ScPropUnoId::NullDate:
        {
            util::Date aDate;
            if ( aValue >>= aDate )
                rOptions.SetDate( aDate.Day, aDate.Month, aDate.Year );
        }
        break;
        case ScPropUnoId::StandardDec:
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
                rOptions.SetStdPrecision( nIntVal );
        }
        break;
        case ScPropUnoId::RegexEnabled:
            rOptions.SetFormulaRegexEnabled( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case ScPropUnoId::WildcardsEnabled:
            rOptions.SetFormulaWildcardsEnabled( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        default:;
    }
    return true;
}

uno::Any ScDocOptionsHelper::getPropertyValue(
                const ScDocOptions& rOptions,
                const SfxItemPropertyMap& rPropMap,
                std::u16string_view aPropertyName )
{
    uno::Any aRet;
    const SfxItemPropertyMapEntry* pEntry = rPropMap.getByName( aPropertyName );
    if( !pEntry || !pEntry->nWID )
        return aRet;
    switch( static_cast<ScPropUnoId>(pEntry->nWID) )
    {
        case ScPropUnoId::CalcAsShown:
            aRet <<= rOptions.IsCalcAsShown();
        break;
        case ScPropUnoId::DefTabStop:
            aRet <<= static_cast<sal_Int16>( rOptions.GetTabDistance() );
        break;
        case ScPropUnoId::IgnoreCase:
            aRet <<= rOptions.IsIgnoreCase();
        break;
        case ScPropUnoId::IterEnabled:
            aRet <<= rOptions.IsIter();
        break;
        case ScPropUnoId::IterCount:
            aRet <<= static_cast<sal_Int32>( rOptions.GetIterCount() );
        break;
        case ScPropUnoId::IterEpsilon:
            aRet <<= rOptions.GetIterEps();
        break;
        case ScPropUnoId::LookupLabels:
            aRet <<= rOptions.IsLookUpColRowNames();
        break;
        case ScPropUnoId::MatchWhole:
            aRet <<= rOptions.IsMatchWholeCell();
        break;
        case ScPropUnoId::NullDate:
        {
            sal_uInt16 nD, nM;
            sal_Int16 nY;
            rOptions.GetDate( nD, nM, nY );
            util::Date aDate( nD, nM, nY );
            aRet <<= aDate;
        }
        break;
        case ScPropUnoId::StandardDec:
            aRet <<= static_cast<sal_Int16>( rOptions.GetStdPrecision() );
        break;
        case ScPropUnoId::RegexEnabled:
            aRet <<= rOptions.IsFormulaRegexEnabled();
        break;
        case ScPropUnoId::WildcardsEnabled:
            aRet <<= rOptions.IsFormulaWildcardsEnabled();
        break;
        default:;
    }
    return aRet;
}

ScDocOptionsObj::ScDocOptionsObj( const ScDocOptions& rOpt ) :
    ScModelObj( nullptr ),
    aOptions( rOpt )
{
}

ScDocOptionsObj::~ScDocOptionsObj()
{
}

void SAL_CALL ScDocOptionsObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    bool bDone = ScDocOptionsHelper::setPropertyValue( aOptions, GetPropertySet().getPropertyMap(), aPropertyName, aValue );

    if (!bDone)
        ScModelObj::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL ScDocOptionsObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    uno::Any aRet(ScDocOptionsHelper::getPropertyValue( aOptions, GetPropertySet().getPropertyMap(), aPropertyName ));
    if ( !aRet.hasValue() )
        aRet =  ScModelObj::getPropertyValue( aPropertyName );

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
