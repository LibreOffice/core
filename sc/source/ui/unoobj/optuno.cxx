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
#include "precompiled_sc.hxx"



#include <svl/itemprop.hxx>

#include <com/sun/star/util/Date.hpp>

#include "optuno.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"
#include "docoptio.hxx"
#include "unoguard.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------
// static
const SfxItemPropertyMapEntry* ScDocOptionsHelper::GetPropertyMap()
{
    static SfxItemPropertyMapEntry aMap[] =
    {
        {MAP_CHAR_LEN(SC_UNO_CALCASSHOWN),  PROP_UNO_CALCASSHOWN ,  &getBooleanCppuType(),          0, 0},
        {MAP_CHAR_LEN(SC_UNO_DEFTABSTOP),   PROP_UNO_DEFTABSTOP  ,  &getCppuType((sal_Int16*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNO_IGNORECASE),   PROP_UNO_IGNORECASE  ,  &getBooleanCppuType(),          0, 0},
        {MAP_CHAR_LEN(SC_UNO_ITERENABLED),  PROP_UNO_ITERENABLED ,  &getBooleanCppuType(),          0, 0},
        {MAP_CHAR_LEN(SC_UNO_ITERCOUNT),    PROP_UNO_ITERCOUNT   ,  &getCppuType((sal_Int32*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNO_ITEREPSILON),  PROP_UNO_ITEREPSILON ,  &getCppuType((double*)0),       0, 0},
        {MAP_CHAR_LEN(SC_UNO_LOOKUPLABELS), PROP_UNO_LOOKUPLABELS,  &getBooleanCppuType(),          0, 0},
        {MAP_CHAR_LEN(SC_UNO_MATCHWHOLE),   PROP_UNO_MATCHWHOLE  ,  &getBooleanCppuType(),          0, 0},
        {MAP_CHAR_LEN(SC_UNO_NULLDATE),     PROP_UNO_NULLDATE    ,  &getCppuType((util::Date*)0),   0, 0},
        {MAP_CHAR_LEN(SC_UNO_SPELLONLINE),  PROP_UNO_SPELLONLINE ,  &getBooleanCppuType(),          0, 0},
        {MAP_CHAR_LEN(SC_UNO_STANDARDDEC),  PROP_UNO_STANDARDDEC ,  &getCppuType((sal_Int16*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNO_REGEXENABLED), PROP_UNO_REGEXENABLED,  &getBooleanCppuType(),          0, 0},
        {0,0,0,0,0,0}
    };
    return aMap;
}

// static
sal_Bool ScDocOptionsHelper::setPropertyValue( ScDocOptions& rOptions,
                const SfxItemPropertyMap& rPropMap,
                const rtl::OUString& aPropertyName, const uno::Any& aValue )
{
    //! use map (with new identifiers)

    const SfxItemPropertySimpleEntry* pEntry = rPropMap.getByName(aPropertyName );
    if( !pEntry || !pEntry->nWID )
        return sal_False;
    switch( pEntry->nWID )
    {
        case PROP_UNO_CALCASSHOWN :
            rOptions.SetCalcAsShown( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case PROP_UNO_DEFTABSTOP  :
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
                rOptions.SetTabDistance( nIntVal );
        }
        break;
        case PROP_UNO_IGNORECASE  :
            rOptions.SetIgnoreCase( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case PROP_UNO_ITERENABLED:
            rOptions.SetIter( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case PROP_UNO_ITERCOUNT   :
        {
            sal_Int32 nIntVal = 0;
            if ( aValue >>= nIntVal )
                rOptions.SetIterCount( (sal_uInt16)nIntVal );
        }
        break;
        case PROP_UNO_ITEREPSILON :
        {
            double fDoubleVal = 0;
            if ( aValue >>= fDoubleVal )
                rOptions.SetIterEps( fDoubleVal );
        }
        break;
        case PROP_UNO_LOOKUPLABELS :
            rOptions.SetLookUpColRowNames( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case PROP_UNO_MATCHWHOLE  :
            rOptions.SetMatchWholeCell( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case PROP_UNO_NULLDATE:
        {
            util::Date aDate;
            if ( aValue >>= aDate )
                rOptions.SetDate( aDate.Day, aDate.Month, aDate.Year );
        }
        break;
        case PROP_UNO_SPELLONLINE:
            rOptions.SetAutoSpell( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        case PROP_UNO_STANDARDDEC:
        {
            sal_Int16 nIntVal = 0;
            if ( aValue >>= nIntVal )
                rOptions.SetStdPrecision( nIntVal );
        }
        break;
        case PROP_UNO_REGEXENABLED:
            rOptions.SetFormulaRegexEnabled( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
        break;
        default:;
    }
    return sal_True;
}

// static
uno::Any ScDocOptionsHelper::getPropertyValue(
                const ScDocOptions& rOptions,
                const SfxItemPropertyMap& rPropMap,
                const rtl::OUString& aPropertyName )
{
    uno::Any aRet;
    const SfxItemPropertySimpleEntry* pEntry = rPropMap.getByName( aPropertyName );
    if( !pEntry || !pEntry->nWID )
        return aRet;
    switch( pEntry->nWID )
    {
        case PROP_UNO_CALCASSHOWN :
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsCalcAsShown() );
        break;
        case PROP_UNO_DEFTABSTOP :
            aRet <<= (sal_Int16)( rOptions.GetTabDistance() );
        break;
        case PROP_UNO_IGNORECASE :
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsIgnoreCase() );
        break;
        case PROP_UNO_ITERENABLED:
        ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsIter() );
        break;
        case PROP_UNO_ITERCOUNT:
            aRet <<= (sal_Int32)( rOptions.GetIterCount() );
        break;
        case PROP_UNO_ITEREPSILON:
            aRet <<= (double)( rOptions.GetIterEps() );
        break;
        case PROP_UNO_LOOKUPLABELS:
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsLookUpColRowNames() );
        break;
        case PROP_UNO_MATCHWHOLE:
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsMatchWholeCell() );
        break;
        case PROP_UNO_NULLDATE:
        {
            sal_uInt16 nD, nM, nY;
            rOptions.GetDate( nD, nM, nY );
            util::Date aDate( nD, nM, nY );
            aRet <<= aDate;
        }
        break;
        case PROP_UNO_SPELLONLINE:
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsAutoSpell() );
        break;
        case PROP_UNO_STANDARDDEC :
            aRet <<= (sal_Int16)( rOptions.GetStdPrecision() );
        break;
        case PROP_UNO_REGEXENABLED:
            ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsFormulaRegexEnabled() );
        break;
        default:;
    }
    return aRet;
}

//------------------------------------------------------------------------

ScDocOptionsObj::ScDocOptionsObj( const ScDocOptions& rOpt ) :
    ScModelObj( NULL ),
    aOptions( rOpt )
{
}

ScDocOptionsObj::~ScDocOptionsObj()
{
}

void SAL_CALL ScDocOptionsObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    sal_Bool bDone = ScDocOptionsHelper::setPropertyValue( aOptions, *GetPropertySet().getPropertyMap(), aPropertyName, aValue );

    if (!bDone)
        ScModelObj::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL ScDocOptionsObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Any aRet(ScDocOptionsHelper::getPropertyValue( aOptions, *GetPropertySet().getPropertyMap(), aPropertyName ));
    if ( !aRet.hasValue() )
        aRet =  ScModelObj::getPropertyValue( aPropertyName );

    return aRet;
}

