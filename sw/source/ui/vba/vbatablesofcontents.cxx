/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
            if( xToc->getServiceName().equalsAscii("com.sun.star.text.ContentIndex") )
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
    uno::Reference< text::XDocumentIndex > xDocumentIndex( xDocMSF->createInstance(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.ContentIndex")) ), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xTocProps( xDocumentIndex, uno::UNO_QUERY_THROW );
    sal_Bool isProtected = sal_False;
    xTocProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IsProtected") ), uno::makeAny( isProtected ) );

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

rtl::OUString&
SwVbaTablesOfContents::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaTablesOfContents") );
    return sImplName;
}

uno::Sequence<rtl::OUString>
SwVbaTablesOfContents::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.TablesOfContents") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
