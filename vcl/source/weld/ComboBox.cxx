/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/ComboBox.hxx>

namespace weld
{
void weld::ComboBox::insert(int pos, const OUString& rStr, const OUString* pId,
                            const OUString* pIconName, VirtualDevice* pImageSurface)
{
    disable_notify_events();
    do_insert(pos, rStr, pId, pIconName, pImageSurface);
    enable_notify_events();
}

void weld::ComboBox::set_active(int pos)
{
    disable_notify_events();
    do_set_active(pos);
    enable_notify_events();
}

void weld::ComboBox::set_active_id(const OUString& rStr)
{
    disable_notify_events();
    do_set_active_id(rStr);
    enable_notify_events();
}

bool weld::ComboBox::get_values_changed_from_saved() const
{
    return !m_aSavedValues.empty()
           && std::find(m_aSavedValues.begin(), m_aSavedValues.end(), get_active_text())
                  == m_aSavedValues.end();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
