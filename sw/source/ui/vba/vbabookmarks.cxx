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
#include "vbabookmarks.hxx"
#include "vbabookmark.hxx"
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <ooo/vba/word/WdBookmarkSortBy.hpp>
#include "vbarange.hxx"
#include "wordvbahelper.hxx"
#include <cppuhelper/implbase2.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class BookmarksEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > mxModel;
public:
    BookmarksEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  const uno::Reference< frame::XModel >& xModel  ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), mxModel( xModel ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        uno::Reference< container::XNamed > xNamed( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        rtl::OUString aName = xNamed->getName();
        return uno::makeAny( uno::Reference< word::XBookmark > ( new SwVbaBookmark( m_xParent, m_xContext, mxModel, aName ) ) );
    }

};

// Bookmarks use case-insensitive name lookup in MS Word.
typedef ::cppu::WeakImplHelper2< container::XNameAccess, container::XIndexAccess > BookmarkCollectionHelper_BASE;
class BookmarkCollectionHelper : public BookmarkCollectionHelper_BASE
{
private:
    uno::Reference< container::XNameAccess > mxNameAccess;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    uno::Any cachePos;
public:
    BookmarkCollectionHelper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) throw (uno::RuntimeException) : mxIndexAccess( xIndexAccess )
    {
        mxNameAccess.set( mxIndexAccess, uno::UNO_QUERY_THROW );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException) { return  mxIndexAccess->getElementType(); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException) { return mxIndexAccess->hasElements(); }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return cachePos;
    }
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        return mxNameAccess->getElementNames();
    }
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        if( mxNameAccess->hasByName( aName ) )
        {
            cachePos = mxNameAccess->getByName( aName );
            return sal_True;
        }
        else
        {
            for( sal_Int32 nIndex = 0; nIndex < mxIndexAccess->getCount(); nIndex++ )
            {
                uno::Reference< container::XNamed > xNamed( mxIndexAccess->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
                rtl::OUString aBookmarkName = xNamed->getName();
                if( aName.equalsIgnoreAsciiCase( aBookmarkName ) )
                {
                    cachePos <<= xNamed;
                    return sal_True;
                }
            }
        }
        return sal_False;
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return mxIndexAccess->getCount();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
    {
        return mxIndexAccess->getByIndex( Index );
    }
};

SwVbaBookmarks::SwVbaBookmarks( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xBookmarks, const uno::Reference< frame::XModel >& xModel ): SwVbaBookmarks_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new BookmarkCollectionHelper( xBookmarks ) ) ), mxModel( xModel )
{
    mxBookmarksSupplier.set( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xDocument( mxModel, uno::UNO_QUERY_THROW );
}
// XEnumerationAccess
uno::Type
SwVbaBookmarks::getElementType() throw (uno::RuntimeException)
{
    return word::XBookmark::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaBookmarks::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new BookmarksEnumeration( getParent(), mxContext,xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
SwVbaBookmarks::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< container::XNamed > xNamed( aSource, uno::UNO_QUERY_THROW );
    rtl::OUString aName = xNamed->getName();
    return uno::makeAny( uno::Reference< word::XBookmark > ( new SwVbaBookmark( getParent(), mxContext, mxModel, aName ) ) );
}

void SwVbaBookmarks::removeBookmarkByName( const rtl::OUString& rName ) throw (uno::RuntimeException)
{
    uno::Reference< text::XTextContent > xBookmark( m_xNameAccess->getByName( rName ), uno::UNO_QUERY_THROW );
    word::getXTextViewCursor( mxModel )->getText()->removeTextContent( xBookmark );
}

void SwVbaBookmarks::addBookmarkByName( const uno::Reference< frame::XModel >& xModel, const rtl::OUString& rName, const uno::Reference< text::XTextRange >& rTextRange ) throw (uno::RuntimeException)
{
    uno::Reference< lang::XMultiServiceFactory > xDocMSF( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextContent > xBookmark( xDocMSF->createInstance(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Bookmark")) ), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNamed > xNamed( xBookmark, uno::UNO_QUERY_THROW );
    xNamed->setName( rName );
    rTextRange->getText()->insertTextContent( rTextRange, xBookmark, sal_False );
}

uno::Any SAL_CALL
SwVbaBookmarks::Add( const rtl::OUString& rName, const uno::Any& rRange ) throw (uno::RuntimeException)
{
    uno::Reference< text::XTextRange > xTextRange;
    uno::Reference< word::XRange > xRange;
    if( rRange >>= xRange )
    {
        SwVbaRange* pRange = dynamic_cast< SwVbaRange* >( xRange.get() );
        if( pRange )
            xTextRange = pRange->getXTextRange();
    }
    else
    {
        // FIXME: insert the bookmark into current view cursor
        xTextRange.set( word::getXTextViewCursor( mxModel ), uno::UNO_QUERY_THROW );
    }

    // remove the exist bookmark
    rtl::OUString aName = rName;
    if( m_xNameAccess->hasByName( aName ) )
        removeBookmarkByName( aName );

    addBookmarkByName( mxModel, aName, xTextRange );

    return uno::makeAny( uno::Reference< word::XBookmark >( new SwVbaBookmark( getParent(), mxContext, mxModel, aName ) ) );
}

sal_Int32 SAL_CALL
SwVbaBookmarks::getDefaultSorting() throw (css::uno::RuntimeException)
{
    return word::WdBookmarkSortBy::wdSortByName;
}

void SAL_CALL
SwVbaBookmarks::setDefaultSorting( sal_Int32/* _type*/ ) throw (css::uno::RuntimeException)
{
    // not support in Writer
}

sal_Bool SAL_CALL
SwVbaBookmarks::getShowHidden() throw (css::uno::RuntimeException)
{
    return sal_True;
}

void SAL_CALL
SwVbaBookmarks::setShowHidden( sal_Bool /*_hidden*/ ) throw (css::uno::RuntimeException)
{
    // not support in Writer
}

sal_Bool SAL_CALL
SwVbaBookmarks::Exists( const rtl::OUString& rName ) throw (css::uno::RuntimeException)
{
    sal_Bool bExist = m_xNameAccess->hasByName( rName );
    return bExist;
}

rtl::OUString&
SwVbaBookmarks::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaBookmarks") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaBookmarks::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Bookmarks") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
