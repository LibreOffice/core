/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtAccessibleRegistry.hxx>
#include <QtXAccessible.hxx>

#include <cassert>

std::map<XAccessible*, QObject*> QtAccessibleRegistry::m_aMapping = {};

QObject* QtAccessibleRegistry::getQObject(css::uno::Reference<XAccessible> xAcc)
{
    if (!xAcc.is())
        return nullptr;

    // look for existing entry in the map
    auto entry = m_aMapping.find(xAcc.get());
    if (entry != m_aMapping.end())
        return entry->second;

    // create a new object and remember it in the map
    QtXAccessible* pQtAcc = new QtXAccessible(xAcc);
    m_aMapping.emplace(xAcc.get(), pQtAcc);
    return pQtAcc;
}

void QtAccessibleRegistry::insert(css::uno::Reference<XAccessible> xAcc, QObject* pQObject)
{
    assert(pQObject);
    m_aMapping.emplace(xAcc.get(), pQObject);
}

void QtAccessibleRegistry::remove(css::uno::Reference<XAccessible> xAcc)
{
    assert(xAcc.is());
    m_aMapping.erase(xAcc.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
