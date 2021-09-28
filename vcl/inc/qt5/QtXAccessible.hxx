/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vclpluginapi.h>

#include <QtCore/QObject>

#include <com/sun/star/accessibility/XAccessible.hpp>

#include <vcl/window.hxx>

class QtFrame;
class QtWidget;

// Wrapper class to hold a css::accessibility::XAccessible object
// while being able to pass it as a QObject
class QtXAccessible : public QObject
{
    Q_OBJECT

public:
    QtXAccessible(css::uno::Reference<css::accessibility::XAccessible> xAccessible);
    css::uno::Reference<css::accessibility::XAccessible> m_xAccessible;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
