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
#include "vbatablesofcontents.hxx"
#include "vbatableofcontents.hxx"
#include "vbarange.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <cppuhelper/implbase.hxx>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class TablesOfContentsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 m_nIndex;

public:
    explicit TablesOfContentsEnumWrapper( uno::Reference< container::XIndexAccess > xIndexAccess ) : mxIndexAccess(std::move( xIndexAccess )), m_nIndex( 0 )
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( m_nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if( m_nIndex < mxIndexAccess->getCount() )
        {
            return mxIndexAccess->getByIndex( m_nIndex++ );
        }
        throw container::NoSuchElementException();
    }
};

class TableOfContentsCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                                       container::XEnumerationAccess >
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< text::XTextDocument > mxTextDocument;
    std::vector< uno::Reference< text::XDocumentIndex > > maToc;

public:
    /// @throws uno::RuntimeException
    TableOfContentsCollectionHelper( uno::Reference< ov::XHelperInterface > xParent, uno::Reference< uno::XComponentContext > xContext, uno::Reference< text::XTextDocument >  xDoc ): mxParent(std::move( xParent )), mxContext(std::move( xContext )), mxTextDocument(std::move( xDoc ))
    {
        uno::Reference< text::XDocumentIndexesSupplier > xDocIndexSupp( mxTextDocument, uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xDocIndexes = xDocIndexSupp->getDocumentIndexes();
        sal_Int32 nCount = xDocIndexes->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            uno::Reference< text::XDocumentIndex > xToc( xDocIndexes->getByIndex(i), uno::UNO_QUERY_THROW );
            if( xToc->getServiceName() == "com.sun.star.text.ContentIndex" )
            {
                maToc.push_back( xToc );
            }
        }
    }

    virtual sal_Int32 SAL_CALL getCount(  ) override
    {
        return maToc.size();
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();

        uno::Reference< text::XDocumentIndex > xToc( maToc[Index], uno::UNO_SET_THROW );
        return uno::Any( uno::Reference< word::XTableOfContents >( new SwVbaTableOfContents( mxParent, mxContext, mxTextDocument, xToc ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<word::XTableOfContents>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return true;
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new TablesOfContentsEnumWrapper( this );
    }
};

}

SwVbaTablesOfContents::SwVbaTablesOfContents( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextDocument >& xDoc ) : SwVbaTablesOfContents_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new TableOfContentsCollectionHelper( xParent, xContext, xDoc ) ) ),  mxTextDocument( xDoc )
{
}

uno::Reference< word::XTableOfContents > SAL_CALL
SwVbaTablesOfContents::Add( const uno::Reference< word::XRange >& Range, const uno::Any& /*UseHeadingStyles*/, const uno::Any& /*UpperHeadingLevel*/, const uno::Any& LowerHeadingLevel, const uno::Any& UseFields, const uno::Any& /*TableID*/, const uno::Any& /*RightAlignPageNumbers*/, const uno::Any& /*IncludePageNumbers*/, const uno::Any& /*AddedStyles*/, const uno::Any& /*UseHyperlinks*/, const uno::Any& /*HidePageNumbersInWeb*/, const uno::Any& /*UseOutlineLevels*/ )
{
    uno::Reference< lang::XMultiServiceFactory > xDocMSF( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< text::XDocumentIndex > xDocumentIndex( xDocMSF->createInstance(u"com.sun.star.text.ContentIndex"_ustr), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xTocProps( xDocumentIndex, uno::UNO_QUERY_THROW );
    xTocProps->setPropertyValue(u"IsProtected"_ustr, uno::Any( false ) );

    uno::Reference< word::XTableOfContents > xToc( new SwVbaTableOfContents( this, mxContext, mxTextDocument, xDocumentIndex ) );

    sal_Int32 nLowerHeadingLevel = 9;
    if( LowerHeadingLevel.hasValue() )
        LowerHeadingLevel >>= nLowerHeadingLevel;
    xToc->setLowerHeadingLevel( nLowerHeadingLevel );

    bool bUseFields = false;
    if( UseFields.hasValue() )
        UseFields >>= bUseFields;
    xToc->setUseFields( bUseFields );

    xToc->setUseOutlineLevels( true );

    SwVbaRange* pVbaRange = dynamic_cast<SwVbaRange*>( Range.get() );
    if( !pVbaRange )
        throw uno::RuntimeException();

    uno::Reference< text::XTextRange > xTextRange = pVbaRange->getXTextRange();
    uno::Reference< text::XText > xText = pVbaRange->getXText();
    uno::Reference< text::XTextContent > xTextContent( xDocumentIndex, uno::UNO_QUERY_THROW );
    xText->insertTextContent( xTextRange, xTextContent, false );
    xToc->Update();

    return xToc;
}

// XEnumerationAccess
uno::Type
SwVbaTablesOfContents::getElementType()
{
    return cppu::UnoType<word::XTableOfContents>::get();
}
uno::Reference< container::XEnumeration >
SwVbaTablesOfContents::createEnumeration()
{
    return new TablesOfContentsEnumWrapper( m_xIndexAccess );
}

uno::Any
SwVbaTablesOfContents::createCollectionObject( const uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaTablesOfContents::getServiceImplName()
{
    return u"SwVbaTablesOfContents"_ustr;
}

uno::Sequence<OUString>
SwVbaTablesOfContents::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.TablesOfContents"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
