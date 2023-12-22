/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <sal/log.hxx>

#include "gtkaccessibleeventlistener.hxx"
#include "gtkaccessibleregistry.hxx"

#if GTK_CHECK_VERSION(4, 9, 0)

GtkAccessibleEventListener::GtkAccessibleEventListener(LoAccessible* pLoAccessible)
    : m_pLoAccessible(pLoAccessible)
{
    assert(m_pLoAccessible);
    g_object_ref(m_pLoAccessible);
}

GtkAccessibleEventListener::~GtkAccessibleEventListener()
{
    assert(m_pLoAccessible);
    g_object_unref(m_pLoAccessible);
}

void GtkAccessibleEventListener::disposing(const com::sun::star::lang::EventObject&)
{
    assert(m_pLoAccessible);
    GtkAccessibleRegistry::remove(m_pLoAccessible->uno_accessible);
}

void GtkAccessibleEventListener::notifyEvent(
    const com::sun::star::accessibility::AccessibleEventObject& rEvent)
{
    switch (rEvent.EventId)
    {
        case css::accessibility::AccessibleEventId::STATE_CHANGED:
        {
            sal_Int64 nState;
            bool bNewValueSet = false;
            if (rEvent.NewValue >>= nState)
            {
                bNewValueSet = true;
            }
            else if (!(rEvent.OldValue >>= nState))
            {
                assert(false && "neither old nor new value set");
            }

            if (nState == css::accessibility::AccessibleStateType::CHECKED)
            {
                GtkAccessibleTristate eState
                    = bNewValueSet ? GTK_ACCESSIBLE_TRISTATE_TRUE : GTK_ACCESSIBLE_TRISTATE_FALSE;
                gtk_accessible_update_state(GTK_ACCESSIBLE(m_pLoAccessible),
                                            GTK_ACCESSIBLE_STATE_CHECKED, eState, -1);
            }
            break;
        }
        default:
            break;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
