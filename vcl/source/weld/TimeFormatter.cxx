/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/vclenum.hxx>
#include <vcl/weld/TimeFormatter.hxx>

namespace weld
{
TimeFormatter::TimeFormatter(weld::Entry& rEntry)
    : EntryFormatter(rEntry)
    , m_eFormat(TimeFieldFormat::F_NONE)
    , m_eTimeFormat(TimeFormat::Hour24)
    , m_bDuration(false)
{
    Init();
}

TimeFormatter::TimeFormatter(weld::FormattedSpinButton& rSpinButton)
    : EntryFormatter(rSpinButton)
    , m_eFormat(TimeFieldFormat::F_NONE)
    , m_eTimeFormat(TimeFormat::Hour24)
    , m_bDuration(false)
{
    Init();
}

void TimeFormatter::Init()
{
    DisableRemainderFactor(); //so with hh::mm::ss, incrementing mm will not reset ss

    SetFormatValueHdl(LINK(this, TimeFormatter, FormatOutputHdl));
    SetParseTextHdl(LINK(this, TimeFormatter, ParseInputHdl));

    SetMin(tools::Time(0, 0));
    SetMax(tools::Time(23, 59, 59, 999999999));

    // so the spin size can depend on which zone the cursor is in
    get_widget().connect_cursor_position(LINK(this, TimeFormatter, CursorChangedHdl));
    // and set the initial spin size
    CursorChangedHdl(get_widget());
}

void TimeFormatter::SetExtFormat(ExtTimeFieldFormat eFormat)
{
    switch (eFormat)
    {
        case ExtTimeFieldFormat::Short24H:
        {
            m_eTimeFormat = TimeFormat::Hour24;
            m_bDuration = false;
            m_eFormat = TimeFieldFormat::F_NONE;
        }
        break;
        case ExtTimeFieldFormat::Long24H:
        {
            m_eTimeFormat = TimeFormat::Hour24;
            m_bDuration = false;
            m_eFormat = TimeFieldFormat::F_SEC;
        }
        break;
        case ExtTimeFieldFormat::Short12H:
        {
            m_eTimeFormat = TimeFormat::Hour12;
            m_bDuration = false;
            m_eFormat = TimeFieldFormat::F_NONE;
        }
        break;
        case ExtTimeFieldFormat::Long12H:
        {
            m_eTimeFormat = TimeFormat::Hour12;
            m_bDuration = false;
            m_eFormat = TimeFieldFormat::F_SEC;
        }
        break;
        case ExtTimeFieldFormat::ShortDuration:
        {
            m_bDuration = true;
            m_eFormat = TimeFieldFormat::F_NONE;
        }
        break;
        case ExtTimeFieldFormat::LongDuration:
        {
            m_bDuration = true;
            m_eFormat = TimeFieldFormat::F_SEC;
        }
        break;
    }

    ReFormat();
}

void TimeFormatter::SetDuration(bool bDuration)
{
    m_bDuration = bDuration;
    ReFormat();
}

void TimeFormatter::SetTimeFormat(TimeFieldFormat eTimeFormat)
{
    m_eFormat = eTimeFormat;
    ReFormat();
}

TimeFormatter::~TimeFormatter() = default;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
