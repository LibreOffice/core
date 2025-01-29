/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#include <vcl/event.hxx>
#include <vcl/unohelp.hxx>

#include <accessibility/AccessibleIconView.hxx>

AccessibleIconView::AccessibleIconView(
    SvTreeListBox& _rListBox, const css::uno::Reference<css::accessibility::XAccessible>& _xParent)
    : AccessibleListBox(_rListBox, _xParent)
{
}

void AccessibleIconView::ProcessWindowEvent(const VclWindowEvent& rVclWindowEvent)
{
    if (!isAlive())
        return;

    switch (rVclWindowEvent.GetId())
    {
        case VclEventId::WindowMouseMove:
            if (MouseEvent* pMouseEvt = static_cast<MouseEvent*>(rVclWindowEvent.GetData()))
            {
                if (auto xChild = getAccessibleAtPoint(
                        vcl::unohelper::ConvertToAWTPoint(pMouseEvt->GetPosPixel())))
                {
                    // Allow announcing the element on mouse hover
                    css::uno::Any aNew(xChild);
                    NotifyAccessibleEvent(
                        css::accessibility::AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, {}, aNew);
                }
            }
            break;
        default:
            AccessibleListBox::ProcessWindowEvent(rVclWindowEvent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
