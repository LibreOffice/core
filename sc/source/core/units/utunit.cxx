/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include "utunit.hxx"

#include "util.hxx"

using namespace sc::units;

OUString UtUnit::getString() const {
    char aBuf[200];
    int nChars = ut_format(mpUnit.get(), aBuf, 200, UT_UTF8);
    if (nChars == -1) {
        SAL_INFO("sc.units", "couldn't format unit: " << getUTStatus());
        // Placeholder for unformattable strings.
        return "?";
    }

    // If the output doesn't fit in the buffer, ut_format doesn't write
    // a terminating null. However for any output we have the correct length
    // as returned by ut_format, which is the easiest way to ensure the OString
    // constructor will always work correctly. (Alternatively we could retry with
    // a larger buffer, however this method is purely for debugging purposes for now.)

    return OUString(aBuf, nChars, RTL_TEXTENCODING_UTF8);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
