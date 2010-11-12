/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    //sName += UniString( getWorkSheet()->getName()); //liuchen 2009-9-9 resolve the defect that the name get by macro code are not the same with that in UI (for example, if a name of "AA" is found in the UI "Define Names" dialog box, the result of get that name through macro code will be "Sheet1!AA")
    //sName += String::CreateFromAscii("!");
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
    return getValue( formula::FormulaGrammar::GRAM_NATIVE_XL_A1 );
}

::rtl::OUString
ScVbaName::getValue(const formula::FormulaGrammar::Grammar eGrammar) throw (css::uno::RuntimeException)
{
    rtl::OUString sValue = mxNamedRange->getContent();
    ScDocShell* pDocShell = excel::getDocShell( mxModel );
    ScDocument* pDoc = pDocShell ? pDocShell->GetDocument() : NULL;
    String aContent;
    excel::CompileODFFormulaToExcel( pDoc, sValue, aContent, eGrammar );
    if ( aContent.Len() > 0 )
    {
        sValue = aContent;
    }
    if ( sValue.indexOf('=') != 0 )
    {
        sValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("=")) + sValue;
    }
    return sValue;
}

void
ScVbaName::setValue( const ::rtl::OUString & rValue ) throw (css::uno::RuntimeException)
{
    ::rtl::OUString sValue = rValue;
    ScDocShell* pDocShell = excel::getDocShell( mxModel );
    ScDocument* pDoc = pDocShell ? pDocShell->GetDocument() : NULL;
    String aContent;
    excel::CompileExcelFormulaToODF( pDoc, sValue, aContent );
    if ( aContent.Len() > 0 )
    {
        sValue = aContent;
    }
    mxNamedRange->setContent( sValue );
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
    return getValue( formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1 );
}

void
ScVbaName::setRefersToR1C1( const ::rtl::OUString & rRefersTo ) throw (css::uno::RuntimeException)
{
    setRefersTo( rRefersTo );
}

::rtl::OUString
ScVbaName::getRefersToR1C1Local() throw (css::uno::RuntimeException)
{
    return getValue( formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1 );
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

rtl::OUString&
ScVbaName::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaName") );
    return sImplName;
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
