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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBABORDERS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBABORDERS_HXX

#include <ooo/vba/word/XBorders.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::word::XBorders > SwVbaBorders_BASE;
class VbaPalette;
class SwVbaBorders : public SwVbaBorders_BASE
{
    // XEnumerationAccess
    virtual css::uno::Any getItemByIntIndex( const sal_Int32 nIndex ) throw (css::uno::RuntimeException) override;
    css::uno::Reference< css::beans::XPropertySet > m_xProps;
public:
    SwVbaBorders( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::table::XCellRange >& xRange, VbaPalette& rPalette );
    virtual ~SwVbaBorders() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // SwVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;

    virtual sal_Bool SAL_CALL getShadow() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setShadow( sal_Bool _shadow ) throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBABORDERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
