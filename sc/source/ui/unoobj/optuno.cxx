/*************************************************************************
 *
 *  $RCSfile: optuno.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-12 10:18:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <svtools/itemprop.hxx>

#include <com/sun/star/util/Date.hpp>

#include "optuno.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"
#include "docoptio.hxx"
#include "unoguard.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

// static
const SfxItemPropertyMap* ScDocOptionsHelper::GetPropertyMap()
{
    static SfxItemPropertyMap aMap[] =
    {
        {MAP_CHAR_LEN(SC_UNO_CALCASSHOWN),  0,  &getBooleanCppuType(),                                    0},
        {MAP_CHAR_LEN(SC_UNO_DEFTABSTOP),   0,  &getCppuType((sal_Int16*)0),                              0},
        {MAP_CHAR_LEN(SC_UNO_IGNORECASE),   0,  &getBooleanCppuType(),                                    0},
        {MAP_CHAR_LEN(SC_UNO_ITERENABLED),  0,  &getBooleanCppuType(),                                    0},
        {MAP_CHAR_LEN(SC_UNO_ITERCOUNT),    0,  &getCppuType((sal_Int32*)0),                              0},
        {MAP_CHAR_LEN(SC_UNO_ITEREPSILON),  0,  &getCppuType((double*)0),                                 0},
        {MAP_CHAR_LEN(SC_UNO_LOOKUPLABELS), 0,  &getBooleanCppuType(),                                    0},
        {MAP_CHAR_LEN(SC_UNO_MATCHWHOLE),   0,  &getBooleanCppuType(),                                    0},
        {MAP_CHAR_LEN(SC_UNO_NULLDATE),     0,  &getCppuType((util::Date*)0),                             0},
        {MAP_CHAR_LEN(SC_UNO_SPELLONLINE),  0,  &getBooleanCppuType(),                                    0},
        {MAP_CHAR_LEN(SC_UNO_STANDARDDEC),  0,  &getCppuType((sal_Int16*)0),                              0},
        {0,0,0,0}
    };
    return aMap;
}

// static
sal_Bool ScDocOptionsHelper::setPropertyValue( ScDocOptions& rOptions,
                const rtl::OUString& aPropertyName, const uno::Any& aValue )
{
    //! use map (with new identifiers)

    sal_Bool bKnown = sal_True;
    String aString = aPropertyName;

    if ( aString.EqualsAscii( SC_UNO_CALCASSHOWN ) )
        rOptions.SetCalcAsShown( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aString.EqualsAscii( SC_UNO_IGNORECASE ) )
        rOptions.SetIgnoreCase( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aString.EqualsAscii( SC_UNO_ITERENABLED ) )
        rOptions.SetIter( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aString.EqualsAscii( SC_UNO_LOOKUPLABELS ) )
        rOptions.SetLookUpColRowNames( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aString.EqualsAscii( SC_UNO_MATCHWHOLE ) )
        rOptions.SetMatchWholeCell( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aString.EqualsAscii( SC_UNO_SPELLONLINE ) )
        rOptions.SetAutoSpell( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if ( aString.EqualsAscii( SC_UNO_DEFTABSTOP ) )
    {
        sal_Int16 nIntVal;
        if ( aValue >>= nIntVal )
            rOptions.SetTabDistance( nIntVal );
    }
    else if ( aString.EqualsAscii( SC_UNO_ITERCOUNT ) )
    {
        sal_Int32 nIntVal;
        if ( aValue >>= nIntVal )
            rOptions.SetIterCount( nIntVal );
    }
    else if ( aString.EqualsAscii( SC_UNO_STANDARDDEC ) )
    {
        sal_Int16 nIntVal;
        if ( aValue >>= nIntVal )
            rOptions.SetStdPrecision( nIntVal );
    }
    else if ( aString.EqualsAscii( SC_UNO_ITEREPSILON ) )
    {
        double fDoubleVal;
        if ( aValue >>= fDoubleVal )
            rOptions.SetIterEps( fDoubleVal );
    }
    else if ( aString.EqualsAscii( SC_UNO_NULLDATE ) )
    {
        util::Date aDate;
        if ( aValue >>= aDate )
            rOptions.SetDate( aDate.Day, aDate.Month, aDate.Year );
    }
    else
        bKnown = sal_False;

    return bKnown;
}

// static
uno::Any ScDocOptionsHelper::getPropertyValue(
                const ScDocOptions& rOptions, const rtl::OUString& aPropertyName )
{
    //! use map (with new identifiers)

    String aString = aPropertyName;
    uno::Any aRet;

    if ( aString.EqualsAscii( SC_UNO_CALCASSHOWN ) )
        ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsCalcAsShown() );
    else if ( aString.EqualsAscii( SC_UNO_IGNORECASE ) )
        ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsIgnoreCase() );
    else if ( aString.EqualsAscii( SC_UNO_ITERENABLED ) )
        ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsIter() );
    else if ( aString.EqualsAscii( SC_UNO_LOOKUPLABELS ) )
        ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsLookUpColRowNames() );
    else if ( aString.EqualsAscii( SC_UNO_MATCHWHOLE ) )
        ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsMatchWholeCell() );
    else if ( aString.EqualsAscii( SC_UNO_SPELLONLINE ) )
        ScUnoHelpFunctions::SetBoolInAny( aRet, rOptions.IsAutoSpell() );
    else if ( aString.EqualsAscii( SC_UNO_DEFTABSTOP ) )
        aRet <<= (sal_Int16)( rOptions.GetTabDistance() );
    else if ( aString.EqualsAscii( SC_UNO_ITERCOUNT ) )
        aRet <<= (sal_Int32)( rOptions.GetIterCount() );
    else if ( aString.EqualsAscii( SC_UNO_STANDARDDEC ) )
        aRet <<= (sal_Int16)( rOptions.GetStdPrecision() );
    else if ( aString.EqualsAscii( SC_UNO_ITEREPSILON ) )
        aRet <<= (double)( rOptions.GetIterEps() );
    else if ( aString.EqualsAscii( SC_UNO_NULLDATE ) )
    {
        USHORT nD, nM, nY;
        rOptions.GetDate( nD, nM, nY );
        util::Date aDate( nD, nM, nY );
        aRet <<= aDate;
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

    BOOL bDone = ScDocOptionsHelper::setPropertyValue( aOptions, aPropertyName, aValue );

    if (!bDone)
        ScModelObj::setPropertyValue( aPropertyName, aValue );
}

uno::Any SAL_CALL ScDocOptionsObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;

    uno::Any aRet = ScDocOptionsHelper::getPropertyValue( aOptions, aPropertyName );
    if ( !aRet.hasValue() )
        aRet =  ScModelObj::getPropertyValue( aPropertyName );

    return aRet;
}

