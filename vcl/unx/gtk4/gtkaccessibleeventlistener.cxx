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
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <sal/log.hxx>

#include "gtkaccessibleeventlistener.hxx"
#include "gtkaccessibleregistry.hxx"

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

void GtkAccessibleEventListener::disposing(const css::lang::EventObject&)
{
    assert(m_pLoAccessible);
    GtkAccessibleRegistry::remove(m_pLoAccessible->uno_accessible);
}

void GtkAccessibleEventListener::notifyEvent(
    const css::accessibility::AccessibleEventObject& rEvent)
{
    switch (rEvent.EventId)
    {
#if GTK_CHECK_VERSION(4, 13, 8)
        case css::accessibility::AccessibleEventId::CARET_CHANGED:
        {
            if (GTK_IS_ACCESSIBLE_TEXT(m_pLoAccessible))
                gtk_accessible_text_update_caret_position(GTK_ACCESSIBLE_TEXT(m_pLoAccessible));
            break;
        }
#endif
        case css::accessibility::AccessibleEventId::STATE_CHANGED:
        {
            sal_Int64 nState = 0;
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
#if GTK_CHECK_VERSION(4, 17, 5)
            else if (nState == css::accessibility::AccessibleStateType::FOCUSED)
            {
                gtk_accessible_update_platform_state(GTK_ACCESSIBLE(m_pLoAccessible),
                                                     GTK_ACCESSIBLE_PLATFORM_STATE_FOCUSED);
            }
#endif
            break;
        }
#if GTK_CHECK_VERSION(4, 13, 8)
        case css::accessibility::AccessibleEventId::TEXT_CHANGED:
        {
            if (!GTK_IS_ACCESSIBLE_TEXT(m_pLoAccessible))
                break;

            GtkAccessibleText* pText = GTK_ACCESSIBLE_TEXT(m_pLoAccessible);

            css::accessibility::TextSegment aDeletedText;
            css::accessibility::TextSegment aInsertedText;
            if (rEvent.OldValue >>= aDeletedText)
            {
                gtk_accessible_text_update_contents(
                    pText, GTK_ACCESSIBLE_TEXT_CONTENT_CHANGE_REMOVE, aDeletedText.SegmentStart,
                    aDeletedText.SegmentEnd);
            }
            if (rEvent.NewValue >>= aInsertedText)
            {
                gtk_accessible_text_update_contents(
                    pText, GTK_ACCESSIBLE_TEXT_CONTENT_CHANGE_INSERT, aInsertedText.SegmentStart,
                    aInsertedText.SegmentEnd);
            }
            return;
        }
        case css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED:
        {
            if (GTK_IS_ACCESSIBLE_TEXT(m_pLoAccessible))
                gtk_accessible_text_update_selection_bound(GTK_ACCESSIBLE_TEXT(m_pLoAccessible));
            break;
        }
#endif
        default:
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
