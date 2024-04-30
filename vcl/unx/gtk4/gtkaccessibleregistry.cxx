/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gtkaccessibleregistry.hxx"
#include "a11y.hxx"

#include <cassert>

#if GTK_CHECK_VERSION(4, 9, 0)

std::map<css::accessibility::XAccessible*, LoAccessible*> GtkAccessibleRegistry::m_aMapping = {};

LoAccessible*
GtkAccessibleRegistry::getLOAccessible(css::uno::Reference<css::accessibility::XAccessible> xAcc,
                                       GdkDisplay* pDisplay, GtkAccessible* pParent)
{
    if (!xAcc.is())
        return nullptr;

    // look for existing entry in the map
    auto entry = m_aMapping.find(xAcc.get());
    if (entry != m_aMapping.end())
        return entry->second;

    assert(pDisplay);
    if (!pParent)
    {
        // try to find parent via XAccessible hierarchy; this could be problematic
        // as it could create a separate hierarchy besides the one including native
        // GTK widgets if no object which already has its native parent set exists
        // in the a11y tree path from the root to this object
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext
            = xAcc->getAccessibleContext();
        assert(xContext);
        css::uno::Reference<css::accessibility::XAccessible> xParent
            = xContext->getAccessibleParent();
        pParent = GTK_ACCESSIBLE(getLOAccessible(xParent, pDisplay, nullptr));
        assert(pParent && "No parent explicitly given and none found via the a11y hierarchy");
    }

    // create a new object and remember it in the map
    LoAccessible* pLoAccessible = lo_accessible_new(pDisplay, pParent, xAcc);
    m_aMapping.emplace(xAcc.get(), pLoAccessible);
    return pLoAccessible;
}

void GtkAccessibleRegistry::remove(css::uno::Reference<css::accessibility::XAccessible> xAcc)
{
    assert(xAcc.is());
    m_aMapping.erase(xAcc.get());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
