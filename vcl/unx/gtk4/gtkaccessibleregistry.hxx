/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <gtk/gtk.h>
#include <com/sun/star/accessibility/XAccessible.hpp>

struct LoAccessible;

class GtkAccessibleRegistry
{
private:
    static std::map<css::accessibility::XAccessible*, LoAccessible*> m_aMapping;
    GtkAccessibleRegistry() = delete;

public:
    /** Returns the related LoAccessible* for the XAccessible. Creates a new one if none exists yet. */
    static LoAccessible* getLOAccessible(css::uno::Reference<css::accessibility::XAccessible> xAcc,
                                         GdkDisplay* pDisplay, GtkAccessible* pParent = nullptr);
    /** Removes the entry for the given XAccessible. */
    static void remove(css::uno::Reference<css::accessibility::XAccessible> xAcc);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
