/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <TableDesignHelpBar.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <helpids.h>

using namespace dbaui;

#define DETAILS_MIN_HELP_WIDTH          200

OTableDesignHelpBar::OTableDesignHelpBar(std::unique_ptr<weld::TextView> xTextWin)
    : m_xTextWin(std::move(xTextWin))
{
    m_xTextWin->set_size_request(DETAILS_MIN_HELP_WIDTH, -1);
    m_xTextWin->set_help_id(HID_TAB_DESIGN_HELP_TEXT_FRAME);
}

void OTableDesignHelpBar::SetHelpText( const OUString& rText )
{
    if (!m_xTextWin)
        return;
    m_xTextWin->set_text(rText);
}

bool OTableDesignHelpBar::isCopyAllowed()
{
    int mStartPos, nEndPos;
    return m_xTextWin && m_xTextWin->get_selection_bounds(mStartPos, nEndPos);
}

bool OTableDesignHelpBar::isCutAllowed()
{
    return false;
}

bool OTableDesignHelpBar::isPasteAllowed()
{
    return false;
}

void OTableDesignHelpBar::cut()
{
}

void OTableDesignHelpBar::copy()
{
    if (!m_xTextWin)
        return;
    m_xTextWin->copy_clipboard();
}

void OTableDesignHelpBar::paste()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
