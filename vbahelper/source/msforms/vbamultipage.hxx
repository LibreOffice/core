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
#ifndef SC_VBA_MULTIPAGE_HXX
#define SC_VBA_MULTIPAGE_HXX
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/msforms/XMultiPage.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper1< ScVbaControl, ov::msforms::XMultiPage > MultiPageImpl_BASE;

class ScVbaMultiPage : public MultiPageImpl_BASE
{
    css::uno::Reference< css::container::XIndexAccess > getPages( sal_Int32 nPages );
public:
    ScVbaMultiPage(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::uno::XInterface >& xControl,
        const css::uno::Reference< css::frame::XModel >& xModel,
        ov::AbstractGeometryAttributes* pGeomHelper);

   // Attributes
    virtual sal_Int32 SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( sal_Int32 _value ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Pages( const css::uno::Any& index ) throw (css::uno::RuntimeException);

    //XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
    // XDefaultProperty
    OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException) { return OUString("Value"); }
};
#endif //SC_VBA_LABEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
