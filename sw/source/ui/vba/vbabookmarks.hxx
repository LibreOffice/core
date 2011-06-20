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
#ifndef SW_VBA_BOOKMARKS_HXX
#define SW_VBA_BOOKMARKS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XBookmarks.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XText.hpp>

typedef CollTestImplHelper< ooo::vba::word::XBookmarks > SwVbaBookmarks_BASE;

class SwVbaBookmarks : public SwVbaBookmarks_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XBookmarksSupplier > mxBookmarksSupplier;
    css::uno::Reference< css::text::XText > mxText;

private:
    void removeBookmarkByName( const rtl::OUString& rName ) throw (css::uno::RuntimeException);

public:
    SwVbaBookmarks( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xBookmarks, const css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~SwVbaBookmarks() {}

    static void addBookmarkByName( const css::uno::Reference< css::frame::XModel >& xModel, const rtl::OUString& rName, const css::uno::Reference< css::text::XTextRange >& rTextRange ) throw (css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // SwVbaBookmarks_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

    // XBookmarks
    virtual sal_Int32 SAL_CALL getDefaultSorting() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDefaultSorting( sal_Int32 _type ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getShowHidden() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setShowHidden( sal_Bool _hidden ) throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL Add( const rtl::OUString& rName, const css::uno::Any& rRange ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL Exists( const rtl::OUString& rName ) throw (css::uno::RuntimeException);
};

#endif /* SW_VBA_BOOKMARKS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
