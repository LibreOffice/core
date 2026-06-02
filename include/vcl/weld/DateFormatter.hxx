/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/weld/Entry.hxx>
#include <vcl/weld/EntryFormatter.hxx>

#include <optional>

class CalendarWrapper;
class Date;

enum class ExtDateFieldFormat;

namespace weld
{
class VCL_DLLPUBLIC DateFormatter final : public EntryFormatter
{
public:
    DateFormatter(weld::Entry& rEntry);

    void SetMin(const Date& rNewMin);
    void SetMax(const Date& rNewMax);

    void SetDate(const Date& rNewDate);
    Date GetDate();

    void SetExtDateFormat(ExtDateFieldFormat eFormat);
    void SetShowDateCentury(bool bShowCentury);

    virtual ~DateFormatter() override;

private:
    DECL_DLLPRIVATE_LINK(FormatOutputHdl, double, std::optional<OUString>);
    DECL_DLLPRIVATE_LINK(ParseInputHdl, const OUString&, Formatter::ParseResult);
    DECL_DLLPRIVATE_LINK(CursorChangedHdl, weld::Entry&, void);

    SAL_DLLPRIVATE void Init();
    SAL_DLLPRIVATE CalendarWrapper& GetCalendarWrapper() const;

    SAL_DLLPRIVATE OUString FormatNumber(int nValue) const;

    ExtDateFieldFormat m_eFormat;
    mutable std::unique_ptr<CalendarWrapper> m_xCalendarWrapper;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
