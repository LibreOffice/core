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
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XEnumerationAccess > TableOfContentsCollectionHelper_Base;
typedef std::vector< uno::Reference< text::XDocumentIndex > > XTocVec;

class TablesOfContentsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 nIndex;

public:
    TablesOfContentsEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : mxIndexAccess( xIndexAccess ), nIndex( 0 )
    {
    }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if( nIndex < mxIndexAccess->getCount() )
        {
            return mxIndexAccess->getByIndex( nIndex++ );
        }
        throw container::NoSuchElementException();
    }
};

class TableOfContentsCollectionHelper : public TableOfContentsCollectionHelper_Base
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< text::XTextDocument > mxTextDocument;
    XTocVec maToc;

public:
    TableOfContentsCollectionHelper( const uno::Reference< ov::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextDocument >& xDoc ) throw ( uno::RuntimeException ): mxParent( xParent ), mxContext( xContext ), mxTextDocument( xDoc )
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

    virtual ~TableOfContentsCollectionHelper() {}

    virtual sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return maToc.size();
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();

        uno::Reference< text::XDocumentIndex > xToc( maToc[Index], uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< word::XTableOfContents >( new SwVbaTableOfContents( mxParent, mxContext, mxTextDocument, xToc ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return word::XTableOfContents::static_type(0);
    }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return sal_True;
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new TablesOfContentsEnumWrapper( this );
    }
};

SwVbaTablesOfContents::SwVbaTablesOfContents( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< text::XTextDocument >& xDoc ) throw (uno::RuntimeException) : SwVbaTablesOfContents_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new TableOfContentsCollectionHelper( xParent, xContext, xDoc ) ) ),  mxTextDocument( xDoc )
{
}

uno::Reference< word::XTableOfContents > SAL_CALL
SwVbaTablesOfContents::Add( const uno::Reference< word::XRange >& Range, const uno::Any& /*UseHeadingStyles*/, const uno::Any& /*UpperHeadingLevel*/, const uno::Any& LowerHeadingLevel, const uno::Any& UseFields, const uno::Any& /*TableID*/, const uno::Any& /*RightAlignPageNumbers*/, const uno::Any& /*IncludePageNumbers*/, const uno::Any& /*AddedStyles*/, const uno::Any& /*UseHyperlinks*/, const uno::Any& /*HidePageNumbersInWeb*/, const uno::Any& /*UseOutlineLevels*/ ) throw (uno::RuntimeException)
{
    uno::Reference< lang::XMultiServiceFactory > xDocMSF( mxTextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< text::XDocumentIndex > xDocumentIndex( xDocMSF->createInstance(  OUString("com.sun.star.text.ContentIndex") ), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xTocProps( xDocumentIndex, uno::UNO_QUERY_THROW );
    sal_Bool isProtected = sal_False;
    xTocProps->setPropertyValue( OUString("IsProtected"), uno::makeAny( isProtected ) );

    uno::Reference< word::XTableOfContents > xToc( new SwVbaTableOfContents( this, mxContext, mxTextDocument, xDocumentIndex ) );

    sal_Int32 nLowerHeadingLevel = 9;
    if( LowerHeadingLevel.hasValue() )
        LowerHeadingLevel >>= nLowerHeadingLevel;
    xToc->setLowerHeadingLevel( nLowerHeadingLevel );

    sal_Bool bUseFields = sal_False;
    if( UseFields.hasValue() )
        UseFields >>= bUseFields;
    xToc->setUseFields( bUseFields );

    sal_Bool bUseOutlineLevels = sal_True;
    xToc->setUseOutlineLevels( bUseOutlineLevels );

    SwVbaRange* pVbaRange = dynamic_cast<SwVbaRange*>( Range.get() );
    if( !pVbaRange )
        throw uno::RuntimeException();

    uno::Reference< text::XTextRange > xTextRange = pVbaRange->getXTextRange();
    uno::Reference< text::XText > xText = pVbaRange->getXText();
    uno::Reference< text::XTextContent > xTextContent( xDocumentIndex, uno::UNO_QUERY_THROW );
    xText->insertTextContent( xTextRange, xTextContent, sal_False );
    xToc->Update();

    return xToc;
}

// XEnumerationAccess
uno::Type
SwVbaTablesOfContents::getElementType() throw (uno::RuntimeException)
{
    return word::XTableOfContents::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaTablesOfContents::createEnumeration() throw (uno::RuntimeException)
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
    return OUString("SwVbaTablesOfContents");
}

uno::Sequence<OUString>
SwVbaTablesOfContents::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = OUString("ooo.vba.word.TablesOfContents");
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
