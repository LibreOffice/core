/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtCore/QObject>

#include <com/sun/star/accessibility/XAccessible.hpp>

// Wrapper class to hold a css::accessibility::XAccessible object
// while being able to pass it as a QObject
class QtXAccessible : public QObject
{
    Q_OBJECT

public:
    QtXAccessible(css::uno::Reference<css::accessibility::XAccessible> xAccessible);

    /** Reference to the XAccessible.
      * This is cleared once it has been passed to the QtAccessibleWidget,
      * which then keeps an own reference and takes care of all required
      * access to the XAccessible for the Qt a11y bridge. */
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
