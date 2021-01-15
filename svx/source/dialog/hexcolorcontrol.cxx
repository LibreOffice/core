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

#include <sax/tools/converter.hxx>
#include <svx/hexcolorcontrol.hxx>
#include <rtl/character.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

namespace weld
{
HexColorControl::HexColorControl(std::unique_ptr<weld::Entry> pEntry)
    : m_xEntry(std::move(pEntry))
    , m_nAsyncModifyEvent(nullptr)
{
    m_xEntry->set_max_length(6);
    m_xEntry->set_width_chars(6);
    m_xEntry->connect_insert_text(LINK(this, HexColorControl, ImplProcessInputHdl));
    m_xEntry->connect_changed(LINK(this, HexColorControl, ImplProcessModifyHdl));
}

HexColorControl::~HexColorControl()
{
    if (m_nAsyncModifyEvent)
        Application::RemoveUserEvent(m_nAsyncModifyEvent);
}

IMPL_LINK_NOARG(HexColorControl, OnAsyncModifyHdl, void*, void)
{
    m_nAsyncModifyEvent = nullptr;
    m_aModifyHdl.Call(*m_xEntry);
}

// tdf#123291 resend it async so it arrives after ImplProcessInputHdl has been
// processed
IMPL_LINK_NOARG(HexColorControl, ImplProcessModifyHdl, weld::Entry&, void)
{
    if (m_nAsyncModifyEvent)
        Application::RemoveUserEvent(m_nAsyncModifyEvent);
    m_nAsyncModifyEvent = Application::PostUserEvent(LINK(this, HexColorControl, OnAsyncModifyHdl));
}

void HexColorControl::SetColor(Color nColor)
{
    OUStringBuffer aBuffer;
    sax::Converter::convertColor(aBuffer, nColor);
    OUString sColor = aBuffer.makeStringAndClear().copy(1);
    if (sColor == m_xEntry->get_text())
        return;
    m_xEntry->set_text(sColor);
}

Color HexColorControl::GetColor() const
{
    sal_Int32 nColor = -1;

    OUString aStr = "#" + m_xEntry->get_text();
    sal_Int32 nLen = aStr.getLength();

    if (nLen < 7)
    {
        static const char* const pNullStr = "000000";
        aStr += OUString::createFromAscii(&pNullStr[nLen - 1]);
    }

    sax::Converter::convertColor(nColor, aStr);

    m_xEntry->set_message_type(nColor != -1 ? weld::EntryMessageType::Normal
                                            : weld::EntryMessageType::Error);

    return Color(ColorTransparency, nColor);
}

IMPL_STATIC_LINK(HexColorControl, ImplProcessInputHdl, OUString&, rTest, bool)
{
    const sal_Unicode* pTest = rTest.getStr();
    sal_Int32 nLen = rTest.getLength();

    OUStringBuffer aFilter(nLen);
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        if (rtl::isAsciiHexDigit(*pTest))
            aFilter.append(*pTest);
        ++pTest;
    }

    rTest = aFilter.makeStringAndClear();
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
