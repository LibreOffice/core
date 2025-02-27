/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

class AccessibleObject
    : public cppu::ImplInheritanceHelper<comphelper::OAccessibleComponentHelper,
                                         css::accessibility::XAccessible, css::awt::XWindowListener>
{
public:
    AccessibleObject(const sal_Int16 nRole, OUString sName);
    void LateInitialization();

    virtual void SetWindow (
        const css::uno::Reference<css::awt::XWindow>& rxContentWindow,
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);
    void SetAccessibleParent (const css::uno::Reference<css::accessibility::XAccessible>& rxAccessibleParent);

    virtual void SAL_CALL disposing() override;

    void AddChild (const ::rtl::Reference<AccessibleObject>& rpChild);
    void RemoveChild (const ::rtl::Reference<AccessibleObject>& rpChild);

    void SetIsFocused (const bool bIsFocused);
    void SetAccessibleName (const OUString& rsName);

    void FireAccessibleEvent (
        const sal_Int16 nEventId,
        const css::uno::Any& rOldValue,
        const css::uno::Any& rNewValue);

    void UpdateStateSet();

    //----- XAccessible -------------------------------------------------------

    virtual css::uno::Reference<css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() override;

    //-----  XAccessibleContext  ----------------------------------------------

    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int64 nIndex) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleParent() override;

    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent() override;

    virtual sal_Int16 SAL_CALL getAccessibleRole() override;

    virtual OUString SAL_CALL getAccessibleDescription() override;

    virtual OUString SAL_CALL getAccessibleName() override;

    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet() override;

    virtual sal_Int64 SAL_CALL getAccessibleStateSet() override;

    virtual css::lang::Locale SAL_CALL getLocale() override;

    //-----  XAccessibleComponent  --------------------------------------------

    virtual css::uno::Reference<css::accessibility::XAccessible> SAL_CALL
        getAccessibleAtPoint (
            const css::awt::Point& aPoint) override;

    virtual void SAL_CALL grabFocus() override;

    virtual sal_Int32 SAL_CALL getForeground() override;

    virtual sal_Int32 SAL_CALL getBackground() override;

    //----- XWindowListener ---------------------------------------------------

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent) override;

    //----- XEventListener ----------------------------------------------------

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

protected:
    OUString msName;
    css::uno::Reference<css::awt::XWindow2> mxContentWindow;
    css::uno::Reference<css::awt::XWindow2> mxBorderWindow;
    const sal_Int16 mnRole;
    sal_Int64 mnStateSet;
    bool mbIsFocused;
    css::uno::Reference<css::accessibility::XAccessible> mxParentAccessible;
    ::std::vector<rtl::Reference<AccessibleObject> > maChildren;

    // OCommonAccessibleComponent
    virtual css::awt::Rectangle implGetBounds() override;

    virtual awt::Point GetRelativeLocation();
    virtual awt::Size GetSize();

    virtual bool GetWindowState (const sal_Int64 nType) const;

    void UpdateState (const sal_Int64 aState, const bool bValue);

    /// @throws css::lang::DisposedException
    void ThrowIfDisposed() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
