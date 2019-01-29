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
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/basemutex.hxx>
#include <vcl/vclptr.hxx>

namespace tools { class Rectangle; }
class Ruler;


typedef ::cppu::WeakAggComponentImplHelper5<
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleEventBroadcaster,
            css::lang::XServiceInfo >
            SvtRulerAccessible_Base;

class SvtRulerAccessible final : public ::cppu::BaseMutex, public SvtRulerAccessible_Base
{
public:
    //=====  internal  ========================================================
    SvtRulerAccessible(
        const css::uno::Reference< css::accessibility::XAccessible>& rxParent, Ruler& rRepresentation, const OUString& rName );

    /// @throws css::uno::RuntimeException
    bool
        isVisible();

    //=====  XAccessible  =====================================================

    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() override;

    //=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL
        containsPoint( const css::awt::Point& rPoint ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    virtual css::awt::Rectangle SAL_CALL
        getBounds() override;

    virtual css::awt::Point SAL_CALL
        getLocation() override;

    virtual css::awt::Point SAL_CALL
        getLocationOnScreen() override;

    virtual css::awt::Size SAL_CALL
        getSize() override;

    virtual void SAL_CALL
        grabFocus() override;

    virtual sal_Int32 SAL_CALL
        getForeground(  ) override;
    virtual sal_Int32 SAL_CALL
        getBackground(  ) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild( sal_Int32 nIndex ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent() override;

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() override;

    virtual OUString SAL_CALL
        getAccessibleDescription() override;

    virtual OUString SAL_CALL
        getAccessibleName() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet() override;

    virtual css::lang::Locale SAL_CALL
        getLocale() override;
    //=====  XAccessibleEventBroadcaster  =====================================

    virtual void SAL_CALL
        addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

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

private:

    virtual ~SvtRulerAccessible() override;

    virtual void SAL_CALL disposing() override;

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;

    /// @throws DisposedException if it's not alive
    void ThrowExceptionIfNotAlive();

    /// @Return the object's current bounding box relative to the desktop.
    ///
    /// @throws css::uno::RuntimeException
    tools::Rectangle GetBoundingBoxOnScreen();

    /// @Return the object's current bounding box relative to the parent object.
    ///
    /// @throws css::uno::RuntimeException
    tools::Rectangle GetBoundingBox();

    /// Name of this object.
    OUString const               msName;

    /// Reference to the parent object.
    css::uno::Reference< css::accessibility::XAccessible >
                                        mxParent;

    /// pointer to internal representation
    VclPtr<Ruler>                       mpRepr;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;
};

inline bool SvtRulerAccessible::IsAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
