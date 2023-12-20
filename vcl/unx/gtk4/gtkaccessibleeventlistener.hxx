/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <gtk/gtk.h>

#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <cppuhelper/implbase.hxx>

#include "a11y.hxx"

class GtkAccessibleEventListener final
    : public cppu::WeakImplHelper<css::accessibility::XAccessibleEventListener>
{
public:
    explicit GtkAccessibleEventListener(LoAccessible* pLoAccessible);
    ~GtkAccessibleEventListener();

    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override;
    virtual void SAL_CALL
    notifyEvent(const css::accessibility::AccessibleEventObject& rEvent) override;

private:
    LoAccessible* m_pLoAccessible;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
