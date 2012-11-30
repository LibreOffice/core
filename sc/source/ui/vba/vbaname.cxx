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
#include <vbahelper/helperdecl.hxx>

#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>

#include "vbaname.hxx"
#include "vbarange.hxx"
#include "vbaglobals.hxx"
#include <vector>
#include <rangenam.hxx>
#include <vcl/msgbox.hxx>
#include "tabvwsh.hxx"
#include "viewdata.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaName::ScVbaName(const css::uno::Reference< ov::XHelperInterface >& xParent,
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Reference< css::sheet::XNamedRange >& xName,
            const css::uno::Reference< css::sheet::XNamedRanges >& xNames,
            const css::uno::Reference< css::frame::XModel >& xModel ):
            NameImpl_BASE(  xParent , xContext ),
            mxModel( xModel ),
            mxNamedRange( xName ),
            mxNames( xNames )
{
}

ScVbaName::~ScVbaName()
{
}

css::uno::Reference< ov::excel::XWorksheet >
ScVbaName::getWorkSheet() throw (css::uno::RuntimeException)
{
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    return xApplication->getActiveSheet();
}

::rtl::OUString
ScVbaName::getName() throw (css::uno::RuntimeException)
{
    String sName;
    sName += UniString ( mxNamedRange->getName() );
    return ::rtl::OUString( sName );
}

void
ScVbaName::setName( const ::rtl::OUString & rName ) throw (css::uno::RuntimeException)
{
    mxNamedRange->setName( rName );
}

::rtl::OUString
ScVbaName::getNameLocal() throw (css::uno::RuntimeException)
{
    return getName();
}

void
ScVbaName::setNameLocal( const ::rtl::OUString & rName ) throw (css::uno::RuntimeException)
{
    setName( rName );
}

sal_Bool
ScVbaName::getVisible() throw (css::uno::RuntimeException)
{
    return true;
}

void
ScVbaName::setVisible( sal_Bool /*bVisible*/ ) throw (css::uno::RuntimeException)
{
}

::rtl::OUString
ScVbaName::getValue() throw (css::uno::RuntimeException)
{
    ::rtl::OUString sValue = mxNamedRange->getContent();
    ::rtl::OUString sSheetName = getWorkSheet()->getName();
    ::rtl::OUString sSegmentation = ::rtl::OUString::createFromAscii( ";" );
    ::rtl::OUString sNewSegmentation = ::rtl::OUString::createFromAscii( "," );
    ::rtl::OUString sResult;
    sal_Int32 nFrom = 0;
    sal_Int32 nTo = 0;
    nTo = sValue.indexOf( sSegmentation, nFrom );
    while ( nTo != -1 )
    {
        ::rtl::OUString sTmpValue = sValue.copy( nFrom, nTo - nFrom );
        if ( sTmpValue.toChar() == '$' )
        {
            ::rtl::OUString sTmp = sTmpValue.copy( 1 );
            sTmp = sTmp.replaceAt(0, OUString(sSheetName + ::rtl::OUString::createFromAscii(".")).getLength(), sSheetName + ::rtl::OUString::createFromAscii("!"));
            sResult += sTmp;
            sResult += sNewSegmentation;
        }
        nFrom = nTo + 1;
        nTo = sValue.indexOf( sSegmentation, nFrom );
    }
    ::rtl::OUString sTmpValue = sValue.copy( nFrom );
    if ( sTmpValue.toChar() == '$' )
    {
        ::rtl::OUString sTmp = sTmpValue.copy(1);
        sTmp = sTmp.replaceAt(0, OUString(sSheetName + ::rtl::OUString::createFromAscii(".")).getLength(), sSheetName + ::rtl::OUString::createFromAscii("!"));
        sResult += sTmp;
    }
    if (sResult.indexOf('=') != 0)
    {
        sResult = ::rtl::OUString::createFromAscii("=") + sResult;
    }
    return sResult;
}

void
ScVbaName::setValue( const ::rtl::OUString & rValue ) throw (css::uno::RuntimeException)
{
    ::rtl::OUString sSheetName = getWorkSheet()->getName();
    ::rtl::OUString sValue = rValue;
    ::rtl::OUString sSegmentation = ::rtl::OUString::createFromAscii( "," );
    ::rtl::OUString sNewSegmentation = ::rtl::OUString::createFromAscii( ";" );
    ::rtl::OUString sResult;
    sal_Int32 nFrom = 0;
    sal_Int32 nTo = 0;
    if (sValue.indexOf('=') == 0)
    {
        ::rtl::OUString sTmp = sValue.copy(1);
        sValue = sTmp;
    }
    nTo = sValue.indexOf( sSegmentation, nFrom );
    while ( nTo != -1 )
    {
        ::rtl::OUString sTmpValue = sValue.copy( nFrom, nTo - nFrom );
        sTmpValue = sTmpValue.replaceAt(0, OUString(sSheetName + ::rtl::OUString::createFromAscii("!")).getLength(), sSheetName + ::rtl::OUString::createFromAscii("."));
        if (sTmpValue.copy(0, sSheetName.getLength()).equals(sSheetName))
        {
            sTmpValue = ::rtl::OUString::createFromAscii("$") + sTmpValue;
        }
        sTmpValue += sNewSegmentation;
        sResult += sTmpValue;
        nFrom = nTo + 1;
        nTo = sValue.indexOf( sSegmentation, nFrom );
    }
    ::rtl::OUString sTmpValue = sValue.copy( nFrom );
    sTmpValue = sTmpValue.replaceAt(0, OUString(sSheetName + ::rtl::OUString::createFromAscii("!")).getLength(), sSheetName + ::rtl::OUString::createFromAscii("."));
    if (sTmpValue.copy(0, sSheetName.getLength()).equals(sSheetName))
    {
        sTmpValue = ::rtl::OUString::createFromAscii("$") + sTmpValue;
    }
    sResult += sTmpValue;
    mxNamedRange->setContent(sResult);
}

::rtl::OUString
ScVbaName::getRefersTo() throw (css::uno::RuntimeException)
{
    return getValue();
}

void
ScVbaName::setRefersTo( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
    setValue( rRefersTo );
}

::rtl::OUString
ScVbaName::getRefersToLocal() throw (css::uno::RuntimeException)
{
    return getRefersTo();
}

void
ScVbaName::setRefersToLocal( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
    setRefersTo( rRefersTo );
}

::rtl::OUString
ScVbaName::getRefersToR1C1() throw (css::uno::RuntimeException)
{
    return getRefersTo();
}

void
ScVbaName::setRefersToR1C1( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
    setRefersTo( rRefersTo );
}

::rtl::OUString
ScVbaName::getRefersToR1C1Local() throw (css::uno::RuntimeException)
{
    return getRefersTo();
}

void
ScVbaName::setRefersToR1C1Local( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
    setRefersTo( rRefersTo );
}

css::uno::Reference< ov::excel::XRange >
ScVbaName::getRefersToRange() throw (css::uno::RuntimeException)
{
    uno::Reference< ov::excel::XRange > xRange = ScVbaRange::getRangeObjectForName(
        mxContext, mxNamedRange->getName(), excel::getDocShell( mxModel ), formula::FormulaGrammar::CONV_XL_R1C1 );
    return xRange;
}

void
ScVbaName::setRefersToRange( const css::uno::Reference< ov::excel::XRange > /*rRange*/ ) throw (css::uno::RuntimeException)
{
}

void
ScVbaName::Delete() throw (css::uno::RuntimeException)
{
    mxNames->removeByName( mxNamedRange->getName() );
}

rtl::OUString
ScVbaName::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaName"));
}

uno::Sequence< rtl::OUString >
ScVbaName::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Name" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
