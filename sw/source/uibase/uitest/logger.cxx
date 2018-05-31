/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <edtwin.hxx>
#include <logger.hxx>

void SwUILogger::logAction(VclPtr<vcl::Window> const& xUIElement, SwEventId nEvent)
{
    if (!mbValid)
        return;

    if (xUIElement->get_id().isEmpty())
        return;

    std::unique_ptr<SwEditWinUIObject> pSwEditWinUIObject
        = xUIElement->GetUITestFactory()(xUIElement.get());
    OUString aAction = pSwEditWinUIObject->get_sw_action(nEvent);
    if (!xUIElement->HasFocus() && !child_windows_have_focus(xUIElement))
    {
        return;
    }

    if (!aAction.isEmpty())
        maStream.WriteLine(OUStringToOString(aAction, RTL_TEXTENCODING_UTF8));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
