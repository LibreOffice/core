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
#include "nameuno.hxx"
#include "compiler.hxx"
#include "tokenarray.hxx"

#include <memory>

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

OUString
ScVbaName::getName() throw (css::uno::RuntimeException, std::exception)
{
    return mxNamedRange->getName();
}

void
ScVbaName::setName( const OUString & rName ) throw (css::uno::RuntimeException, std::exception)
{
    mxNamedRange->setName( rName );
}

OUString
ScVbaName::getNameLocal() throw (css::uno::RuntimeException, std::exception)
{
    return getName();
}

void
ScVbaName::setNameLocal( const OUString & rName ) throw (css::uno::RuntimeException, std::exception)
{
    setName( rName );
}

sal_Bool
ScVbaName::getVisible() throw (css::uno::RuntimeException, std::exception)
{
    return true;
}

void
ScVbaName::setVisible( sal_Bool /*bVisible*/ ) throw (css::uno::RuntimeException, std::exception)
{
}

OUString ScVbaName::getContent( const formula::FormulaGrammar::Grammar eGrammar, bool bPrependEquals )
{
    ScNamedRangeObj* pNamedRange = dynamic_cast< ScNamedRangeObj* >( mxNamedRange.get() );
    OUString aContent;
    if ( pNamedRange )
    {
        ScRangeData* pData = pNamedRange->GetRangeData_Impl();
        if (pData)
            pData->GetSymbol( aContent, eGrammar );
    }
    if ( bPrependEquals )
    {
        if (aContent.indexOf('=') != 0)
            aContent = "=" + aContent;
    }
    return aContent;
}

void  ScVbaName::setContent( const OUString& rContent, const formula::FormulaGrammar::Grammar eGrammar, bool bRemoveEquals )
{
    OUString sContent( rContent );
    if ( bRemoveEquals )
    {
        if (sContent.startsWith("="))
            sContent = sContent.copy(1);
    }
    ScNamedRangeObj* pNamedRange = dynamic_cast< ScNamedRangeObj* >( mxNamedRange.get() );

    // We should be able to do the below by just setting calling SetCode on pNamedRange
    // right?
    if ( pNamedRange && pNamedRange->pDocShell )
    {

        ScDocument& rDoc = pNamedRange->pDocShell->GetDocument();
        ScRangeData* pOldData = pNamedRange->GetRangeData_Impl();
        if (pOldData)
        {
            // Shorter way of doing this ?
            ScCompiler aComp( &rDoc, pOldData->GetPos() );
            aComp.SetGrammar( eGrammar );
            std::unique_ptr<ScTokenArray> pArray(aComp.CompileString(sContent));
            pOldData->SetCode(*pArray);
        }
    }
}

OUString
ScVbaName::getValue() throw (css::uno::RuntimeException, std::exception)
{
    rtl::OUString sResult = getContent( formula::FormulaGrammar::GRAM_NATIVE_XL_A1 );

    return sResult;
}

void
ScVbaName::setValue( const OUString & rValue ) throw (css::uno::RuntimeException, std::exception)
{
    setContent( rValue, formula::FormulaGrammar::GRAM_NATIVE_XL_A1 );
}

OUString
ScVbaName::getRefersTo() throw (css::uno::RuntimeException, std::exception)
{
    return getValue();
}

void
ScVbaName::setRefersTo( const OUString & rRefersTo ) throw (css::uno::RuntimeException, std::exception)
{
    setValue( rRefersTo );
}

OUString
ScVbaName::getRefersToLocal() throw (css::uno::RuntimeException, std::exception)
{
    return getRefersTo();
}

void
ScVbaName::setRefersToLocal( const OUString & rRefersTo ) throw (css::uno::RuntimeException, std::exception)
{
    setRefersTo( rRefersTo );
}

OUString
ScVbaName::getRefersToR1C1() throw (css::uno::RuntimeException, std::exception)
{
    rtl::OUString sResult = getContent( formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1 );
    return  sResult;
}

void
ScVbaName::setRefersToR1C1( const OUString & rRefersTo ) throw (css::uno::RuntimeException, std::exception)
{
    setContent( rRefersTo, formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1 );
}

OUString
ScVbaName::getRefersToR1C1Local() throw (css::uno::RuntimeException, std::exception)
{
    return getRefersToR1C1();
}

void
ScVbaName::setRefersToR1C1Local( const OUString & rRefersTo ) throw (css::uno::RuntimeException, std::exception)
{
    setRefersTo( rRefersTo );
}

css::uno::Reference< ov::excel::XRange >
ScVbaName::getRefersToRange() throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< ov::excel::XRange > xRange = ScVbaRange::getRangeObjectForName(
        mxContext, mxNamedRange->getName(), excel::getDocShell( mxModel ), formula::FormulaGrammar::CONV_XL_R1C1 );
    return xRange;
}

void
ScVbaName::Delete() throw (css::uno::RuntimeException, std::exception)
{
    mxNames->removeByName( mxNamedRange->getName() );
}

OUString
ScVbaName::getServiceImplName()
{
    return OUString( "ScVbaName" );
}

uno::Sequence< OUString >
ScVbaName::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.Name";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
