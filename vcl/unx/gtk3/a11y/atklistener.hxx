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

#pragma once

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <cppuhelper/implbase.hxx>

#include <vector>

#include "atkwrapper.hxx"

class AtkListener : public ::cppu::WeakImplHelper< css::accessibility::XAccessibleEventListener >
{
public:
    explicit AtkListener(AtkObjectWrapper * pWrapper);

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XAccessibleEventListener
    virtual void SAL_CALL notifyEvent( const css::accessibility::AccessibleEventObject& aEvent ) override;

private:

    AtkObjectWrapper *mpWrapper;
    std::vector< css::uno::Reference< css::accessibility::XAccessible > >
                      m_aChildList;

    virtual ~AtkListener() override;

    // Updates the child list held to provide the old IndexInParent on children_changed::remove
    void updateChildList(
        css::uno::Reference<css::accessibility::XAccessibleContext> const &
            pContext);

    // Process CHILD_EVENT notifications with a new child added
    void handleChildAdded(
        const css::uno::Reference< css::accessibility::XAccessibleContext >& rxParent,
        const css::uno::Reference< css::accessibility::XAccessible>& rxChild);

    // Process CHILD_EVENT notifications with a child removed
    void handleChildRemoved(
        const css::uno::Reference< css::accessibility::XAccessibleContext >& rxParent,
        const css::uno::Reference< css::accessibility::XAccessible>& rxChild);

    // Process INVALIDATE_ALL_CHILDREN notification
    void handleInvalidateChildren(
        const css::uno::Reference< css::accessibility::XAccessibleContext >& rxParent);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
