/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <tools/time.hxx>
#include <vcl/dllapi.h>
#include <vcl/weld/Entry.hxx>
#include <vcl/weld/EntryFormatter.hxx>

#include <optional>

enum class ExtTimeFieldFormat;
enum class TimeFieldFormat : sal_Int32;
enum class TimeFormat;

namespace weld
{
class Entry;
class FormattedSpinButton;

class VCL_DLLPUBLIC TimeFormatter final : public EntryFormatter
{
public:
    TimeFormatter(weld::Entry& rEntry);
    TimeFormatter(weld::FormattedSpinButton& rSpinButton);

    void SetExtFormat(ExtTimeFieldFormat eFormat);
    void SetDuration(bool bDuration);
    void SetTimeFormat(TimeFieldFormat eTimeFormat);

    void SetMin(const tools::Time& rNewMin);
    void SetMax(const tools::Time& rNewMax);

    void SetTime(const tools::Time& rNewTime);
    tools::Time GetTime();

    virtual ~TimeFormatter() override;

private:
    DECL_DLLPRIVATE_LINK(FormatOutputHdl, double, std::optional<OUString>);
    DECL_DLLPRIVATE_LINK(ParseInputHdl, const OUString&, Formatter::ParseResult);
    DECL_DLLPRIVATE_LINK(CursorChangedHdl, weld::TextWidget&, void);

    SAL_DLLPRIVATE void Init();

    SAL_DLLPRIVATE static tools::Time ConvertValue(int nValue);
    SAL_DLLPRIVATE static int ConvertValue(const tools::Time& rTime);

    SAL_DLLPRIVATE OUString FormatNumber(int nValue) const;

    TimeFieldFormat m_eFormat;
    TimeFormat m_eTimeFormat;
    bool m_bDuration;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
