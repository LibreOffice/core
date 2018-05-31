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
#include <loggerdata.hxx>

void SwUILogger::log(const OUString& rString) { UITestLogger::getInstance().log(rString); }

void SwUILogger::logAction(VclPtr<vcl::Window> const& xUIElement, SwEventId nEvent)
{
    if (!UITestLogger::getInstance().getLoggerStatus())
        return;

    if (xUIElement->get_id().isEmpty())
        return;

    std::unique_ptr<SwEditWinUIObject> pSwEditWinUIObject
        = xUIElement->GetUITestFactory()(xUIElement.get());
    OUString aAction = pSwEditWinUIObject->get_sw_action(nEvent);

    log(aAction);
}

SwUILogger& SwUILogger::getInstance()
{
    if (!m_pSwUILogger)
    {
        m_pSwUILogger.reset(new SwUILogger);
    }
    return m_pSwUILogger
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */