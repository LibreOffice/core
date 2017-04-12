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
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <cppuhelper/implbase.hxx>

using namespace ::ooo::vba;
using namespace css;

static uno::Reference< container::XIndexAccess > lcl_getTables( const uno::Reference< frame::XModel >& xDoc )
{
    uno::Reference< container::XIndexAccess > xTables;
    uno::Reference< text::XTextTablesSupplier > xSupp( xDoc, uno::UNO_QUERY );
    if ( xSupp.is() )
        xTables.set( xSupp->getTextTables(), uno::UNO_QUERY_THROW );
    return xTables;
}

static uno::Any lcl_createTable( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xDocument, const uno::Any& aSource )
{
    uno::Reference< text::XTextTable > xTextTable( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xTextDocument( xDocument, uno::UNO_QUERY_THROW );
    uno::Reference< word::XTable > xTable( new SwVbaTable( xParent, xContext, xTextDocument, xTextTable ) );
    return uno::makeAny( xTable );
}

static bool lcl_isInHeaderFooter( const uno::Reference< text::XTextTable >& xTable )
{
    uno::Reference< text::XTextContent > xTextContent( xTable, uno::UNO_QUERY_THROW );
    uno::Reference< text::XText > xText = xTextContent->getAnchor()->getText();
    uno::Reference< lang::XServiceInfo > xServiceInfo( xText, uno::UNO_QUERY_THROW );
    OUString aImplName = xServiceInfo->getImplementationName();
    return aImplName == "SwXHeadFootText";
}

typedef std::vector< uno::Reference< text::XTextTable > > XTextTableVec;

class TableCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                             container::XNameAccess >
{
    XTextTableVec mxTables;
    XTextTableVec::iterator cachePos;

public:
    explicit TableCollectionHelper( const uno::Reference< frame::XModel >& xDocument )
    {
        // only count the tables in the body text, not in the header/footer
        uno::Reference< container::XIndexAccess > xTables = lcl_getTables( xDocument );
        sal_Int32 nCount = xTables->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            uno::Reference< text::XTextTable > xTable( xTables->getByIndex( i ) , uno::UNO_QUERY_THROW );
            if( !lcl_isInHeaderFooter( xTable ) )
                mxTables.push_back( xTable );
        }
        cachePos = mxTables.begin();
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
        uno::Reference< text::XTextTable > xTable( mxTables[ Index ], uno::UNO_QUERY_THROW );
        return uno::makeAny( xTable );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override { return  cppu::UnoType<text::XTextTable>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override { return getCount() > 0 ; }
    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        uno::Reference< text::XTextTable > xTable( *cachePos, uno::UNO_QUERY_THROW );
        return uno::makeAny( xTable );
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        uno::Sequence< OUString > sNames( mxTables.size() );
        OUString* pString = sNames.getArray();
        XTextTableVec::iterator it = mxTables.begin();
        XTextTableVec::iterator it_end = mxTables.end();
        for ( ; it != it_end; ++it, ++pString )
        {
            uno::Reference< container::XNamed > xName( *it, uno::UNO_QUERY_THROW );
            *pString = xName->getName();
        }
        return sNames;
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        cachePos = mxTables.begin();
        XTextTableVec::iterator it_end = mxTables.end();
        for ( ; cachePos != it_end; ++cachePos )
        {
            uno::Reference< container::XNamed > xName( *cachePos, uno::UNO_QUERY_THROW );
            if ( aName.equalsIgnoreAsciiCase( xName->getName() ) )
                break;
        }
        return ( cachePos != it_end );
    }
};

class TableEnumerationImpl : public ::cppu::WeakImplHelper< css::container::XEnumeration >
{
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxDocument;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 mnCurIndex;
public:
    TableEnumerationImpl(  const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xDocument, const uno::Reference< container::XIndexAccess >& xIndexAccess ) : mxParent( xParent ), mxContext( xContext ), mxDocument( xDocument ), mxIndexAccess( xIndexAccess ), mnCurIndex(0)
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

SwVbaTables::SwVbaTables( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xDocument ) : SwVbaTables_BASE( xParent, xContext , uno::Reference< container::XIndexAccess >( new TableCollectionHelper( xDocument ) ) ), mxDocument( xDocument )
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

    uno::Reference< frame::XModel > xModel( pVbaRange->getDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< lang::XMultiServiceFactory > xMsf( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTextRange = pVbaRange->getXTextRange();

    uno::Reference< text::XTextTable > xTable;
    xTable.set( xMsf->createInstance("com.sun.star.text.TextTable"), uno::UNO_QUERY_THROW );

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
    return OUString("SwVbaTables");
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
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.word.Tables";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
