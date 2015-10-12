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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAWINDOWS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAWINDOWS_HXX

#include <ooo/vba/excel/XWindows.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>

#include "excelvbahelper.hxx"
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XWindows > ScVbaWindows_BASE;

class ScVbaWindows : public ScVbaWindows_BASE
{
public:
    ScVbaWindows( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~ScVbaWindows() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // XWindows
    virtual void SAL_CALL Arrange( ::sal_Int32 ArrangeStyle, const css::uno::Any& ActiveWorkbook, const css::uno::Any& SyncHorizontal, const css::uno::Any& SyncVertical ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAWINDOWS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
