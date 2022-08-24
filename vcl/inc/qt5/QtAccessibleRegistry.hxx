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

#include <QtCore/QObject>

#include <com/sun/star/accessibility/XAccessible.hpp>

using namespace css::accessibility;

/**
 * Maintains a mapping between XAccessible objects and the
 * associated QObjects. The corresponding QObject can be
 * passed to the QAccessible::queryAccessibleInterface method in
 * order to retrieve the QAccessibleInterface for the
 * XAccessible object.
 */
class QtAccessibleRegistry
{
private:
    static std::map<css::accessibility::XAccessible*, QObject*> m_aMapping;
    QtAccessibleRegistry() = delete;

public:
    /** Returns the related QObject* for the XAccessible. Creates a new one if none exists yet. */
    static QObject* getQObject(css::uno::Reference<XAccessible> xAcc);
    static void insert(css::uno::Reference<XAccessible> xAcc, QObject* pQObject);
    /** Removes the entry for the given XAccessible. */
    static void remove(css::uno::Reference<XAccessible> xAcc);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
