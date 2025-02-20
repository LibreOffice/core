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

#include "vbatables.hxx"
#include "vbatable.hxx"
#include "vbarange.hxx"
#include "wordvbahelper.hxx"
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <cppuhelper/implbase.hxx>
#include <unotxdoc.hxx>
#include <unocoll.hxx>
#include <unotbl.hxx>
#include <utility>

using namespace ::ooo::vba;
using namespace css;

static uno::Any lcl_createTable( const uno::Reference< XHelperInterface >& xParent,
                                 const uno::Reference< uno::XComponentContext >& xContext,
                                 const rtl::Reference< SwXTextDocument >& xDocument,
                                 const uno::Any& aSource )
{
    uno::Reference< text::XTextTable > xTextTable( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< word::XTable > xTable( new SwVbaTable( xParent, xContext, xDocument, xTextTable ) );
    return uno::Any( xTable );
}

static bool lcl_isInHeaderFooter( const uno::Reference< text::XTextTable >& xTable )
{
    uno::Reference< text::XTextContent > xTextContent( xTable, uno::UNO_QUERY_THROW );
    uno::Reference< text::XText > xText = xTextContent->getAnchor()->getText();
    uno::Reference< lang::XServiceInfo > xServiceInfo( xText, uno::UNO_QUERY );
    if ( !xServiceInfo )
        return false;
    OUString aImplName = xServiceInfo->getImplementationName();
    return aImplName == "SwXHeadFootText";
}

namespace {

class TableCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                             container::XNameAccess >
{
    std::vector<rtl::Reference<SwXTextTable>> mxTables;
    std::vector<rtl::Reference<SwXTextTable>>::iterator m_cachePos;

public:
    explicit TableCollectionHelper( const rtl::Reference< SwXTextDocument >& xDocument )
    {
        // only count the tables in the body text, not in the header/footer
        rtl::Reference< SwXTextTables > xTables = xDocument->getSwTextTables();
        sal_Int32 nCount = xTables->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            rtl::Reference< SwXTextTable > xTable = xTables->getTextTableByIndex( i );
            if( !lcl_isInHeaderFooter( xTable ) )
                mxTables.push_back( xTable );
        }
        m_cachePos = mxTables.begin();
    }
    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override
    {
        return mxTables.size();
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();
        return uno::Any( uno::Reference< text::XTextTable >( mxTables[ Index ] ) );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override { return  cppu::UnoType<text::XTextTable>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override { return getCount() > 0 ; }
    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        uno::Reference< text::XTextTable > xTable( *m_cachePos, uno::UNO_SET_THROW );
        return uno::Any( xTable );
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        uno::Sequence< OUString > sNames( mxTables.size() );
        OUString* pString = sNames.getArray();
        for ( const auto& rxTable : mxTables )
        {
            *pString = rxTable->getName();
            ++pString;
        }
        return sNames;
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        m_cachePos = mxTables.begin();
        auto it_end = mxTables.end();
        for ( ; m_cachePos != it_end; ++m_cachePos )
        {
            if ( aName.equalsIgnoreAsciiCase( (*m_cachePos)->getName() ) )
                break;
        }
        return ( m_cachePos != it_end );
    }
};

class TableEnumerationImpl : public ::cppu::WeakImplHelper< css::container::XEnumeration >
{
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    rtl::Reference< SwXTextDocument > mxDocument;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 mnCurIndex;
public:
    TableEnumerationImpl( uno::Reference< XHelperInterface > xParent,
                          uno::Reference< uno::XComponentContext > xContext,
                          rtl::Reference< SwXTextDocument > xDocument,
                          uno::Reference< container::XIndexAccess >  xIndexAccess )
    : mxParent(std::move( xParent )), mxContext(std::move( xContext )),
      mxDocument(std::move( xDocument )), mxIndexAccess(std::move( xIndexAccess )), mnCurIndex(0)
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( mnCurIndex < mxIndexAccess->getCount() );
    }
    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        return lcl_createTable( mxParent, mxContext, mxDocument, mxIndexAccess->getByIndex( mnCurIndex++ ) );
    }

};

}

SwVbaTables::SwVbaTables( const uno::Reference< XHelperInterface >& xParent,
                          const uno::Reference< uno::XComponentContext > & xContext,
                          const rtl::Reference< SwXTextDocument >& xDocument )
: SwVbaTables_BASE( xParent, xContext , uno::Reference< container::XIndexAccess >( new TableCollectionHelper( xDocument ) ) ),
  mxDocument( xDocument )
{
}

uno::Reference< word::XTable > SAL_CALL
SwVbaTables::Add( const uno::Reference< word::XRange >& Range, const uno::Any& NumRows, const uno::Any& NumColumns, const uno::Any& /*DefaultTableBehavior*/, const uno::Any& /*AutoFitBehavior*/ )
{
    sal_Int32 nCols = 0;
    sal_Int32 nRows = 0;
    SwVbaRange* pVbaRange = dynamic_cast< SwVbaRange* >( Range.get() );
    // Preconditions
    if ( !( pVbaRange && ( NumRows >>= nRows ) && ( NumColumns >>= nCols ) ) )
        throw uno::RuntimeException(); // #FIXME better exception??
    if ( nCols <= 0 || nRows <= 0 )
        throw uno::RuntimeException(); // #FIXME better exception??

    rtl::Reference< SwXTextDocument > xModel( pVbaRange->getDocument() );
    uno::Reference< text::XTextRange > xTextRange = pVbaRange->getXTextRange();

    uno::Reference< text::XTextTable > xTable;
    xTable.set( xModel->createInstance(u"com.sun.star.text.TextTable"_ustr), uno::UNO_QUERY_THROW );

    xTable->initialize( nRows, nCols );
    uno::Reference< text::XText > xText = xTextRange->getText();
    uno::Reference< text::XTextContent > xContext( xTable, uno::UNO_QUERY_THROW );

    xText->insertTextContent( xTextRange, xContext, true );

    // move the current cursor to the first table cell
    uno::Reference< table::XCellRange > xCellRange( xTable, uno::UNO_QUERY_THROW );
    uno::Reference< text::XText> xFirstCellText( xCellRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
    word::getXTextViewCursor( mxDocument )->gotoRange( xFirstCellText->getStart(), false );

    uno::Reference< word::XTable > xVBATable( new SwVbaTable( mxParent, mxContext,  pVbaRange->getDocument(), xTable ) );
    return xVBATable;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaTables::createEnumeration()
{
    return new TableEnumerationImpl( mxParent, mxContext, mxDocument, m_xIndexAccess );
}

// ScVbaCollectionBaseImpl
uno::Any
SwVbaTables::createCollectionObject( const uno::Any& aSource )
{
    return lcl_createTable( mxParent, mxContext, mxDocument, aSource );
}

// XHelperInterface
OUString
SwVbaTables::getServiceImplName()
{
    return u"SwVbaTables"_ustr;
}

// XEnumerationAccess
uno::Type SAL_CALL
SwVbaTables::getElementType()
{
    return  cppu::UnoType<word::XTable>::get();
}

uno::Sequence<OUString>
SwVbaTables::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Tables"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
