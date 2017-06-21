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

#ifndef INCLUDED_SC_INC_ACCESSIBLEFILTERMENUITEM_HXX
#define INCLUDED_SC_INC_ACCESSIBLEFILTERMENUITEM_HXX

#include "AccessibleContextBase.hxx"
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <vcl/vclptr.hxx>

class ScMenuFloatingWindow;

typedef ::cppu::ImplHelper1<
    css::accessibility::XAccessibleAction > ScAccessibleFilterMenuItem_BASE;

class ScAccessibleFilterMenuItem :
    public ScAccessibleContextBase,
    public ScAccessibleFilterMenuItem_BASE
{
public:
    explicit ScAccessibleFilterMenuItem(
        const css::uno::Reference< css::accessibility::XAccessible>& rxParent,
        ScMenuFloatingWindow* pWin, const OUString& rName, size_t nMenuPos);

    virtual ~ScAccessibleFilterMenuItem() override;

    /// XAccessibleContext

    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleChild(sal_Int32 nIndex) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet() override;

    virtual OUString SAL_CALL getImplementationName() override;

    /// XAccessibleAction

    virtual ::sal_Int32 SAL_CALL getAccessibleActionCount() override;

    virtual sal_Bool SAL_CALL doAccessibleAction(sal_Int32 nIndex) override;

    virtual OUString SAL_CALL getAccessibleActionDescription(sal_Int32 nIndex) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL
        getAccessibleActionKeyBinding(sal_Int32 nIndex) override;

    /// XInterface

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType ) override;

    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    /// Non-UNO Methods

protected:

    virtual tools::Rectangle GetBoundingBoxOnScreen() const override;

    virtual tools::Rectangle GetBoundingBox() const override;

private:
    bool isSelected() const;
    void updateStateSet();

private:
    css::uno::Reference< css::accessibility::XAccessibleStateSet > mxStateSet;

    VclPtr<ScMenuFloatingWindow> mpWindow;
    size_t mnMenuPos;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
