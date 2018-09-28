/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Qt5XAccessible.hxx>
#include <Qt5XAccessible.moc>

#include <Qt5Frame.hxx>
#include <Qt5Tools.hxx>
#include <Qt5Widget.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>

#include <sal/log.hxx>

using namespace css::accessibility;
using namespace css::uno;

Qt5XAccessible::Qt5XAccessible(Reference<XAccessible> xAccessible)
    : m_xAccessible(xAccessible)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
