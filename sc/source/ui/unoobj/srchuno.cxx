/*************************************************************************
 *
 *  $RCSfile: srchuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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

#include "scitems.hxx"
#include <svx/srchitem.hxx>
#include <osl/mutex.hxx>
#include <rtl/uuid.h>

#include "srchuno.hxx"
#include "docsh.hxx"
#include "undoblk.hxx"
#include "hints.hxx"
#include "markdata.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"
#include "unonames.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//! SearchWords sucht in ganzen Zellen - umbenennen ???

//  SfxItemPropertyMap nur fuer GetPropertySetInfo

const SfxItemPropertyMap* lcl_GetSearchPropertyMap()
{
    static SfxItemPropertyMap aSearchPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNO_SRCHBACK),     0,      &getBooleanCppuType(),       0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHBYROW),    0,      &getBooleanCppuType(),       0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHCASE),     0,      &getBooleanCppuType(),       0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHREGEXP),   0,      &getBooleanCppuType(),       0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHSIM),      0,      &getBooleanCppuType(),       0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHSIMADD),   0,      &getCppuType((sal_Int16*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHSIMEX),    0,      &getCppuType((sal_Int16*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHSIMREL),   0,      &getBooleanCppuType(),       0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHSIMREM),   0,      &getCppuType((sal_Int16*)0), 0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHSTYLES),   0,      &getBooleanCppuType(),       0, 0},
        {MAP_CHAR_LEN(SC_UNO_SRCHTYPE),     0,      &getCppuType((sal_Int16*)0), 0, 0}, // enum TableSearch ist weg
        {MAP_CHAR_LEN(SC_UNO_SRCHWORDS),    0,      &getBooleanCppuType(),       0, 0},
        {0,0,0,0}
    };
    return aSearchPropertyMap_Impl;
}

//------------------------------------------------------------------------

#define SCSEARCHDESCRIPTOR_SERVICE      "com.sun.star.util.SearchDescriptor"
#define SCREPLACEDESCRIPTOR_SERVICE     "com.sun.star.util.ReplaceDescriptor"

//------------------------------------------------------------------------

ScCellSearchObj::ScCellSearchObj() :
    aPropSet(lcl_GetSearchPropertyMap())
{
    pSearchItem = new SvxSearchItem;
    //  Defaults:
    pSearchItem->SetWordOnly(FALSE);
    pSearchItem->SetExact(FALSE);
    pSearchItem->SetBackward(FALSE);
    pSearchItem->SetSelection(FALSE);
    pSearchItem->SetRegExp(FALSE);
    pSearchItem->SetPattern(FALSE);
    pSearchItem->SetLevenshtein(FALSE);
    pSearchItem->SetLEVRelaxed(FALSE);
    pSearchItem->SetLEVOther(2);
    pSearchItem->SetLEVShorter(2);
    pSearchItem->SetLEVLonger(2);
    //  Calc-Flags
    pSearchItem->SetRowDirection(FALSE);
    pSearchItem->SetCellType(SVX_SEARCHIN_FORMULA);

    //  Selection-Flag wird beim Aufruf gesetzt
}

ScCellSearchObj::~ScCellSearchObj()
{
    delete pSearchItem;
}

// XSearchDescriptor

rtl::OUString SAL_CALL ScCellSearchObj::getSearchString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pSearchItem->GetSearchString();
}

void SAL_CALL ScCellSearchObj::setSearchString( const rtl::OUString& aString )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    pSearchItem->SetSearchString( aString );
}

// XReplaceDescriptor

rtl::OUString SAL_CALL ScCellSearchObj::getReplaceString() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return pSearchItem->GetReplaceString();
}

void SAL_CALL ScCellSearchObj::setReplaceString( const rtl::OUString& aReplaceString )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    pSearchItem->SetReplaceString( aReplaceString );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellSearchObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScCellSearchObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aPropertyName;

    if (aString.EqualsAscii( SC_UNO_SRCHBACK ))        pSearchItem->SetBackward( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHBYROW ))  pSearchItem->SetRowDirection( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHCASE ))   pSearchItem->SetExact( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHREGEXP )) pSearchItem->SetRegExp( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHSIM ))    pSearchItem->SetLevenshtein( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHSIMREL )) pSearchItem->SetLEVRelaxed( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHSTYLES )) pSearchItem->SetPattern( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHWORDS ))  pSearchItem->SetWordOnly( ScUnoHelpFunctions::GetBoolFromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHSIMADD )) pSearchItem->SetLEVLonger( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHSIMEX ))  pSearchItem->SetLEVOther( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHSIMREM )) pSearchItem->SetLEVShorter( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
    else if (aString.EqualsAscii( SC_UNO_SRCHTYPE ))   pSearchItem->SetCellType( ScUnoHelpFunctions::GetInt16FromAny( aValue ) );
}

uno::Any SAL_CALL ScCellSearchObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aPropertyName;
    uno::Any aRet;

    if (aString.EqualsAscii( SC_UNO_SRCHBACK ))        ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetBackward() );
    else if (aString.EqualsAscii( SC_UNO_SRCHBYROW ))  ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetRowDirection() );
    else if (aString.EqualsAscii( SC_UNO_SRCHCASE ))   ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetExact() );
    else if (aString.EqualsAscii( SC_UNO_SRCHREGEXP )) ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetRegExp() );
    else if (aString.EqualsAscii( SC_UNO_SRCHSIM ))    ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->IsLevenshtein() );
    else if (aString.EqualsAscii( SC_UNO_SRCHSIMREL )) ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->IsLEVRelaxed() );
    else if (aString.EqualsAscii( SC_UNO_SRCHSTYLES )) ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetPattern() );
    else if (aString.EqualsAscii( SC_UNO_SRCHWORDS ))  ScUnoHelpFunctions::SetBoolInAny( aRet, pSearchItem->GetWordOnly() );
    else if (aString.EqualsAscii( SC_UNO_SRCHSIMADD )) aRet <<= (sal_Int16) pSearchItem->GetLEVLonger();
    else if (aString.EqualsAscii( SC_UNO_SRCHSIMEX ))  aRet <<= (sal_Int16) pSearchItem->GetLEVOther();
    else if (aString.EqualsAscii( SC_UNO_SRCHSIMREM )) aRet <<= (sal_Int16) pSearchItem->GetLEVShorter();
    else if (aString.EqualsAscii( SC_UNO_SRCHTYPE ))   aRet <<= (sal_Int16) pSearchItem->GetCellType();

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScCellSearchObj )

// XServiceInfo

rtl::OUString SAL_CALL ScCellSearchObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScCellSearchObj" );
}

sal_Bool SAL_CALL ScCellSearchObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr = rServiceName;
    return aServiceStr.EqualsAscii( SCSEARCHDESCRIPTOR_SERVICE ) ||
           aServiceStr.EqualsAscii( SCREPLACEDESCRIPTOR_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellSearchObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( SCSEARCHDESCRIPTOR_SERVICE );
    pArray[1] = rtl::OUString::createFromAscii( SCREPLACEDESCRIPTOR_SERVICE );
    return aRet;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScCellSearchObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScCellSearchObj::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScCellSearchObj* ScCellSearchObj::getImplementation(
                                const uno::Reference<util::XSearchDescriptor> xObj )
{
    ScCellSearchObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScCellSearchObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}


//------------------------------------------------------------------------





