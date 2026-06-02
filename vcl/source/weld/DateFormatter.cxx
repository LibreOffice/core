/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/date.hxx>
#include <unotools/calendarwrapper.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld/DateFormatter.hxx>

namespace weld
{
DateFormatter::DateFormatter(weld::Entry& rEntry)
    : EntryFormatter(rEntry)
    , m_eFormat(ExtDateFieldFormat::SystemShort)
{
    Init();
}

void DateFormatter::Init()
{
    SetFormatValueHdl(LINK(this, DateFormatter, FormatOutputHdl));
    SetParseTextHdl(LINK(this, DateFormatter, ParseInputHdl));

    SetMin(Date(1, 1, 1900));
    SetMax(Date(31, 12, 2200));
}

void DateFormatter::SetExtDateFormat(ExtDateFieldFormat eFormat)
{
    m_eFormat = eFormat;
    ReFormat();
}

DateFormatter::~DateFormatter() = default;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
