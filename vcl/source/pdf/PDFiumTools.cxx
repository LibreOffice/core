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
        return OUString();

    OUString prefix = rInput.copy(0, 2);
    if (prefix != "D:")
        return OUString();

    OUString sYear = rInput.copy(2, 4);

    OUString sMonth("01");
    if (rInput.getLength() >= 8)
        sMonth = rInput.copy(6, 2);

    OUString sDay("01");
    if (rInput.getLength() >= 10)
        sDay = rInput.copy(8, 2);

    OUString sHours("00");
    if (rInput.getLength() >= 12)
        sHours = rInput.copy(10, 2);

    OUString sMinutes("00");
    if (rInput.getLength() >= 14)
        sMinutes = rInput.copy(12, 2);

    OUString sSeconds("00");
    if (rInput.getLength() >= 16)
        sSeconds = rInput.copy(14, 2);

    OUString sTimeZoneMark("Z");
    if (rInput.getLength() >= 17)
        sTimeZoneMark = rInput.copy(16, 1);

    OUString sTimeZoneHours("00");
    OUString sTimeZoneMinutes("00");
    if ((sTimeZoneMark == "+" || sTimeZoneMark == "-") && rInput.getLength() >= 22)
    {
        OUString sTimeZoneSeparator = rInput.copy(19, 1);
        if (sTimeZoneSeparator == "'")
        {
            sTimeZoneHours = rInput.copy(17, 2);
            sTimeZoneMinutes = rInput.copy(20, 2);
        }
    }

    OUString sTimeZoneString;
    if (sTimeZoneMark == "+" || sTimeZoneString == "-")
        sTimeZoneString = sTimeZoneMark + sTimeZoneHours + ":" + sTimeZoneMinutes;
    else if (sTimeZoneMark == "Z")
        sTimeZoneString = sTimeZoneMark;

    return sYear + "-" + sMonth + "-" + sDay + "T" + sHours + ":" + sMinutes + ":" + sSeconds
           + sTimeZoneString;
}
} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
