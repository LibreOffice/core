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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <ooo/vba/word/WdBookmarkSortBy.hpp>
#include "vbarange.hxx"
#include "wordvbahelper.hxx"
#include <cppuhelper/implbase.hxx>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

namespace {

class BookmarksEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > mxModel;
public:
    /// @throws uno::RuntimeException
    BookmarksEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  uno::Reference< frame::XModel > xModel  ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), mxModel(std::move( xModel )) {}

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        uno::Reference< container::XNamed > xNamed( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return uno::Any(uno::Reference<word::XBookmark>(new SwVbaBookmark(m_xParent, m_xContext, mxModel, xNamed->getName())));
    }

};

// Bookmarks use case-insensitive name lookup in MS Word.
class BookmarkCollectionHelper : public ::cppu::WeakImplHelper< container::XNameAccess,
                                                                container::XIndexAccess >
{
private:
    uno::Reference< container::XNameAccess > mxNameAccess;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    uno::Any m_cachePos;
public:
    /// @throws uno::RuntimeException
    explicit BookmarkCollectionHelper( uno::Reference< container::XIndexAccess >  xIndexAccess ) : mxIndexAccess(std::move( xIndexAccess ))
    {
        mxNameAccess.set( mxIndexAccess, uno::UNO_QUERY_THROW );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override { return  mxIndexAccess->getElementType(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override { return mxIndexAccess->hasElements(); }
    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return m_cachePos;
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        return mxNameAccess->getElementNames();
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        if( mxNameAccess->hasByName( aName ) )
        {
            m_cachePos = mxNameAccess->getByName( aName );
            return true;
        }
        else
        {
            for( sal_Int32 nIndex = 0; nIndex < mxIndexAccess->getCount(); nIndex++ )
            {
                uno::Reference< container::XNamed > xNamed( mxIndexAccess->getByIndex( nIndex ), uno::UNO_QUERY_THROW );
                OUString aBookmarkName = xNamed->getName();
                if( aName.equalsIgnoreAsciiCase( aBookmarkName ) )
                {
                    m_cachePos <<= xNamed;
                    return true;
                }
            }
        }
        return false;
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
    {
        return mxIndexAccess->getCount();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        return mxIndexAccess->getByIndex( Index );
    }
};

}

SwVbaBookmarks::SwVbaBookmarks( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xBookmarks, uno::Reference< frame::XModel > xModel ): SwVbaBookmarks_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new BookmarkCollectionHelper( xBookmarks ) ) ), mxModel(std::move( xModel ))
{
    mxBookmarksSupplier.set( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xDocument( mxModel, uno::UNO_QUERY_THROW );
}
// XEnumerationAccess
uno::Type
SwVbaBookmarks::getElementType()
{
    return cppu::UnoType<word::XBookmark>::get();
}
uno::Reference< container::XEnumeration >
SwVbaBookmarks::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new BookmarksEnumeration( getParent(), mxContext,xEnumAccess->createEnumeration(), mxModel );
}

uno::Any
SwVbaBookmarks::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< container::XNamed > xNamed( aSource, uno::UNO_QUERY_THROW );
    return uno::Any(uno::Reference<word::XBookmark>(new SwVbaBookmark(getParent(), mxContext, mxModel, xNamed->getName())));
}

void SwVbaBookmarks::removeBookmarkByName( const OUString& rName )
{
    uno::Reference< text::XTextContent > xBookmark( m_xNameAccess->getByName( rName ), uno::UNO_QUERY_THROW );
    word::getXTextViewCursor( mxModel )->getText()->removeTextContent( xBookmark );
}

void SwVbaBookmarks::addBookmarkByName( const uno::Reference< frame::XModel >& xModel, const OUString& rName, const uno::Reference< text::XTextRange >& rTextRange )
{
    uno::Reference< lang::XMultiServiceFactory > xDocMSF( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextContent > xBookmark( xDocMSF->createInstance(u"com.sun.star.text.Bookmark"_ustr), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNamed > xNamed( xBookmark, uno::UNO_QUERY_THROW );
    xNamed->setName( rName );
    rTextRange->getText()->insertTextContent( rTextRange, xBookmark, false );
}

uno::Any SAL_CALL
SwVbaBookmarks::Add( const OUString& rName, const uno::Any& rRange )
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
    if( m_xNameAccess->hasByName( rName ) )
        removeBookmarkByName( rName );

    addBookmarkByName( mxModel, rName, xTextRange );

    return uno::Any( uno::Reference< word::XBookmark >( new SwVbaBookmark( getParent(), mxContext, mxModel, rName ) ) );
}

sal_Int32 SAL_CALL
SwVbaBookmarks::getDefaultSorting()
{
    return word::WdBookmarkSortBy::wdSortByName;
}

void SAL_CALL
SwVbaBookmarks::setDefaultSorting( sal_Int32/* _type*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL
SwVbaBookmarks::getShowHidden()
{
    return true;
}

void SAL_CALL
SwVbaBookmarks::setShowHidden( sal_Bool /*_hidden*/ )
{
    // not support in Writer
}

sal_Bool SAL_CALL
SwVbaBookmarks::Exists( const OUString& rName )
{
    bool bExist = m_xNameAccess->hasByName( rName );
    return bExist;
}

OUString
SwVbaBookmarks::getServiceImplName()
{
    return u"SwVbaBookmarks"_ustr;
}

css::uno::Sequence<OUString>
SwVbaBookmarks::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.Bookmarks"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
