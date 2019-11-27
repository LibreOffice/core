/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>

#include "Qt5AccessibleWidget.hxx"

#include <cppuhelper/implbase.hxx>

class Qt5AccessibleEventListener final
    : public cppu::WeakImplHelper<css::accessibility::XAccessibleEventListener>
{
public:
    Qt5AccessibleEventListener(
        const css::uno::Reference<css::accessibility::XAccessible> xAccessible,
        Qt5AccessibleWidget* pAccessibleWidget);

    virtual void SAL_CALL
    notifyEvent(const css::accessibility::AccessibleEventObject& aEvent) override;

    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

private:
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
    Qt5AccessibleWidget* m_pAccessibleWidget;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
