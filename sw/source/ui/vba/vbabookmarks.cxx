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
#include "vbabookmarks.hxx"
#include "vbabookmark.hxx"
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <ooo/vba/word/WdBookmarkSortBy.hpp>
#include "vbarange.hxx"
#include "wordvbahelper.hxx"
#include <cppuhelper/implbase.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class BookmarksEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > mxModel;
public:
    BookmarksEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  const uno::Reference< frame::XModel >& xModel  ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), mxModel( xModel ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        uno::Reference< container::XNamed > xNamed( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        OUString aName = xNamed->getName();
        return uno::makeAny( uno::Reference< word::XBookmark > ( new SwVbaBookmark( m_xParent, m_xContext, mxModel, aName ) ) );
    }

};

// Bookmarks use case-insensitive name lookup in MS Word.
class BookmarkCollectionHelper : public ::cppu::WeakImplHelper< container::XNameAccess,
                                                                container::XIndexAccess >
{
private:
    uno::Reference< container::XNameAccess > mxNameAccess;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    uno::Any cachePos;
public:
    explicit BookmarkCollectionHelper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) throw (uno::RuntimeException) : mxIndexAccess( xIndexAccess )
    {
        mxNameAccess.set( mxIndexAccess, uno::UNO_QUERY_THROW );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException, std::exception) override { return  mxIndexAccess->getElementType(); }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException, std::exception) override { return mxIndexAccess->hasElements(); }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return cachePos;
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException, std::exception) override
    {
        return mxNameAccess->getElementNames();
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (uno::RuntimeException, std::exception) override
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
                OUString aBookmarkName = xNamed->getName();
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
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException, std::exception) override
    {
        return mxIndexAccess->getCount();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception ) override
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
    return cppu::UnoType<word::XBookmark>::get();
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
    OUString aName = xNamed->getName();
    return uno::makeAny( uno::Reference< word::XBookmark > ( new SwVbaBookmark( getParent(), mxContext, mxModel, aName ) ) );
}

void SwVbaBookmarks::removeBookmarkByName( const OUString& rName ) throw (uno::RuntimeException)
{
    uno::Reference< text::XTextContent > xBookmark( m_xNameAccess->getByName( rName ), uno::UNO_QUERY_THROW );
    word::getXTextViewCursor( mxModel )->getText()->removeTextContent( xBookmark );
}

void SwVbaBookmarks::addBookmarkByName( const uno::Reference< frame::XModel >& xModel, const OUString& rName, const uno::Reference< text::XTextRange >& rTextRange ) throw (uno::RuntimeException)
{
    uno::Reference< lang::XMultiServiceFactory > xDocMSF( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextContent > xBookmark( xDocMSF->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNamed > xNamed( xBookmark, uno::UNO_QUERY_THROW );
    xNamed->setName( rName );
    rTextRange->getText()->insertTextContent( rTextRange, xBookmark, sal_False );
}

uno::Any SAL_CALL
SwVbaBookmarks::Add( const OUString& rName, const uno::Any& rRange ) throw (uno::RuntimeException, std::exception)
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
    OUString aName = rName;
    if( m_xNameAccess->hasByName( aName ) )
        removeBookmarkByName( aName );

    addBookmarkByName( mxModel, aName, xTextRange );

    return uno::makeAny( uno::Reference< word::XBookmark >( new SwVbaBookmark( getParent(), mxContext, mxModel, aName ) ) );
}

sal_Int32 SAL_CALL
SwVbaBookmarks::getDefaultSorting() throw (css::uno::RuntimeException, std::exception)
{
    return word::WdBookmarkSortBy::wdSortByName;
}

void SAL_CALL
SwVbaBookmarks::setDefaultSorting( sal_Int32/* _type*/ ) throw (css::uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL
SwVbaBookmarks::getShowHidden() throw (css::uno::RuntimeException, std::exception)
{
    return sal_True;
}

void SAL_CALL
SwVbaBookmarks::setShowHidden( sal_Bool /*_hidden*/ ) throw (css::uno::RuntimeException, std::exception)
{
    // not support in Writer
}

sal_Bool SAL_CALL
SwVbaBookmarks::Exists( const OUString& rName ) throw (css::uno::RuntimeException, std::exception)
{
    bool bExist = m_xNameAccess->hasByName( rName );
    return bExist;
}

OUString
SwVbaBookmarks::getServiceImplName()
{
    return OUString("SwVbaBookmarks");
}

css::uno::Sequence<OUString>
SwVbaBookmarks::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.word.Bookmarks";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
