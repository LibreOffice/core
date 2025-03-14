/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <rtl/ustring.hxx>

namespace jsdialog
{
template <class T> class WidgetRegister
{
    // Map to remember the LOKWindowId <-> widget binding.
    std::map<OUString, T> m_aWidgetMap;
    std::map<OUString, T>& Map() { return m_aWidgetMap; }
    const std::map<OUString, T>& Map() const { return m_aWidgetMap; }

public:
    void Remember(const OUString& rId, const T& pWidget);
    void Forget(const OUString& rId);
    T Find(const OUString& rId) const;
};

template <class T> void WidgetRegister<T>::Remember(const OUString& nWindowId, const T& pMenu)
{
    Map()[nWindowId] = pMenu;
}

template <class T> void WidgetRegister<T>::Forget(const OUString& nWindowId)
{
    auto it = Map().find(nWindowId);
    if (it != Map().end())
        Map().erase(it);
}

template <class T> T WidgetRegister<T>::Find(const OUString& nWindowId) const
{
    const auto it = Map().find(nWindowId);

    if (it != Map().end())
        return it->second;

    return nullptr;
}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
