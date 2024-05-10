/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/filter/PDFiumLibrary.hxx>

namespace vcl::pdf
{
OUString convertPdfDateToISO8601(std::u16string_view rInput)
{
    if (rInput.size() < 6)
        return {};

    std::u16string_view prefix = rInput.substr(0, 2);
    if (prefix != u"D:")
        return {};

    std::u16string_view sYear = rInput.substr(2, 4);

    std::u16string_view sMonth(u"01");
    if (rInput.size() >= 8)
        sMonth = rInput.substr(6, 2);

    std::u16string_view sDay(u"01");
    if (rInput.size() >= 10)
        sDay = rInput.substr(8, 2);

    std::u16string_view sHours(u"00");
    if (rInput.size() >= 12)
        sHours = rInput.substr(10, 2);

    std::u16string_view sMinutes(u"00");
    if (rInput.size() >= 14)
        sMinutes = rInput.substr(12, 2);

    std::u16string_view sSeconds(u"00");
    if (rInput.size() >= 16)
        sSeconds = rInput.substr(14, 2);

    OUString sTimeZoneMark(u"Z"_ustr);
    if (rInput.size() >= 17)
        sTimeZoneMark = rInput.substr(16, 1);

    std::u16string_view sTimeZoneHours(u"00");
    std::u16string_view sTimeZoneMinutes(u"00");
    if ((sTimeZoneMark == "+" || sTimeZoneMark == "-") && rInput.size() >= 22)
    {
        std::u16string_view sTimeZoneSeparator = rInput.substr(19, 1);
        if (sTimeZoneSeparator == u"'")
        {
            sTimeZoneHours = rInput.substr(17, 2);
            sTimeZoneMinutes = rInput.substr(20, 2);
        }
    }

    OUString sTimeZoneString;
    if (sTimeZoneMark == "+" || sTimeZoneString == "-")
        sTimeZoneString = sTimeZoneMark + sTimeZoneHours + ":" + sTimeZoneMinutes;
    else if (sTimeZoneMark == "Z")
        sTimeZoneString = sTimeZoneMark;

    return OUString::Concat(sYear) + "-" + sMonth + "-" + sDay + "T" + sHours + ":" + sMinutes + ":"
           + sSeconds + sTimeZoneString;
}
} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
