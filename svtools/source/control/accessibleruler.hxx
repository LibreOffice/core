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
#ifndef INCLUDED_SVTOOLS_ACCESSIBLERULER_HXX
#define INCLUDED_SVTOOLS_ACCESSIBLERULER_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <comphelper/compbase.hxx>
#include <vcl/vclptr.hxx>

namespace tools { class Rectangle; }
class Ruler;

class SvtRulerAccessible final
    : public cppu::ImplInheritanceHelper<comphelper::OAccessibleComponentHelper,
                                         css::accessibility::XAccessible, css::lang::XServiceInfo>
{
public:
    //=====  internal  ========================================================
    SvtRulerAccessible(
        css::uno::Reference< css::accessibility::XAccessible> xParent, Ruler& rRepresentation, OUString aName );

    //=====  XAccessible  =====================================================

    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() override;

    //=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    virtual void SAL_CALL
        grabFocus() override;

    virtual sal_Int32 SAL_CALL
        getForeground(  ) override;
    virtual sal_Int32 SAL_CALL
        getBackground(  ) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int64 SAL_CALL
        getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild( sal_Int64 nIndex ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent() override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() override;

    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    virtual OUString SAL_CALL
        getAccessibleName() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() override;

    virtual sal_Int64 SAL_CALL
        getAccessibleStateSet() override;

    virtual css::lang::Locale SAL_CALL
        getLocale() override;

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL
        getImplementationName() override;

    virtual sal_Bool SAL_CALL
        supportsService( const OUString& sServiceName ) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    //=====  XTypeProvider  ===================================================

    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() override;

protected:
    virtual css::awt::Rectangle implGetBounds() override;

private:
    virtual void SAL_CALL disposing() override;

    /// Name of this object.
    OUString                            msName;

    /// Reference to the parent object.
    css::uno::Reference< css::accessibility::XAccessible >
                                        mxParent;

    /// pointer to internal representation
    VclPtr<Ruler>                       mpRepr;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
