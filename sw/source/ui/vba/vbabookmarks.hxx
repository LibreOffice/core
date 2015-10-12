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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBABOOKMARKS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBABOOKMARKS_HXX

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
    void removeBookmarkByName( const OUString& rName ) throw (css::uno::RuntimeException);

public:
    SwVbaBookmarks( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::container::XIndexAccess >& xBookmarks, const css::uno::Reference< css::frame::XModel >& xModel );
    virtual ~SwVbaBookmarks() {}

    static void addBookmarkByName( const css::uno::Reference< css::frame::XModel >& xModel, const OUString& rName, const css::uno::Reference< css::text::XTextRange >& rTextRange ) throw (css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // SwVbaBookmarks_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // XBookmarks
    virtual sal_Int32 SAL_CALL getDefaultSorting() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultSorting( sal_Int32 _type ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getShowHidden() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setShowHidden( sal_Bool _hidden ) throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL Add( const OUString& rName, const css::uno::Any& rRange ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL Exists( const OUString& rName ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBABOOKMARKS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
