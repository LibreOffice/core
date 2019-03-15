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

#include <curledit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <osl/diagnose.h>

namespace dbaui
{

OConnectionURLEdit::OConnectionURLEdit(std::unique_ptr<weld::Entry> xEntry, std::unique_ptr<weld::Label> xForcedPrefix)
    : m_pTypeCollection(nullptr)
    , m_bShowPrefix(false)
    , m_xEntry(std::move(xEntry))
    , m_xForcedPrefix(std::move(xForcedPrefix))
{
}

OConnectionURLEdit::~OConnectionURLEdit()
{
}

void OConnectionURLEdit::SetTextNoPrefix(const OUString& _rText)
{
    m_xEntry->set_text(_rText);
}

OUString OConnectionURLEdit::GetTextNoPrefix() const
{
    return m_xEntry->get_text();
}

void OConnectionURLEdit::SetText(const OUString& _rStr)
{
    Selection aNoSelection(0,0);
    SetText(_rStr, aNoSelection);
}

void OConnectionURLEdit::SetText(const OUString& _rStr, const Selection& /*_rNewSelection*/)
{
    m_xForcedPrefix->set_visible(m_bShowPrefix);

    bool bIsEmpty = _rStr.isEmpty();
    // calc the prefix
    OUString sPrefix;
    if (!bIsEmpty)
    {
        // determine the type of the new URL described by the new text
        sPrefix = m_pTypeCollection->getPrefix(_rStr);
    }

    // the fixed text gets the prefix
    m_xForcedPrefix->set_label(sPrefix);

    // do the real SetText
    OUString sNewText( _rStr );
    if ( !bIsEmpty )
        sNewText  = m_pTypeCollection->cutPrefix( _rStr );
    m_xEntry->set_text(sNewText);
}

OUString OConnectionURLEdit::GetText() const
{
    return m_xForcedPrefix->get_label() + m_xEntry->get_text();
}

void OConnectionURLEdit::ShowPrefix(bool _bShowPrefix)
{
    m_bShowPrefix = _bShowPrefix;
    m_xForcedPrefix->set_visible(m_bShowPrefix);
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
