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
OUString convertPdfDateToISO8601(OUString const& rInput)
{
    if (rInput.getLength() < 6)
        return {};

    std::u16string_view prefix = rInput.subView(0, 2);
    if (prefix != u"D:")
        return {};

    std::u16string_view sYear = rInput.subView(2, 4);

    std::u16string_view sMonth(u"01");
    if (rInput.getLength() >= 8)
        sMonth = rInput.subView(6, 2);

    std::u16string_view sDay(u"01");
    if (rInput.getLength() >= 10)
        sDay = rInput.subView(8, 2);

    std::u16string_view sHours(u"00");
    if (rInput.getLength() >= 12)
        sHours = rInput.subView(10, 2);

    std::u16string_view sMinutes(u"00");
    if (rInput.getLength() >= 14)
        sMinutes = rInput.subView(12, 2);

    std::u16string_view sSeconds(u"00");
    if (rInput.getLength() >= 16)
        sSeconds = rInput.subView(14, 2);

    OUString sTimeZoneMark("Z");
    if (rInput.getLength() >= 17)
        sTimeZoneMark = rInput.subView(16, 1);

    std::u16string_view sTimeZoneHours(u"00");
    std::u16string_view sTimeZoneMinutes(u"00");
    if ((sTimeZoneMark == "+" || sTimeZoneMark == "-") && rInput.getLength() >= 22)
    {
        std::u16string_view sTimeZoneSeparator = rInput.subView(19, 1);
        if (sTimeZoneSeparator == u"'")
        {
            sTimeZoneHours = rInput.subView(17, 2);
            sTimeZoneMinutes = rInput.subView(20, 2);
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
