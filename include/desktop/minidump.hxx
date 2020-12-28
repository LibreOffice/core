/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_MINIDUMP_MINIDUMP_HXX
#define INCLUDED_DESKTOP_MINIDUMP_MINIDUMP_HXX

#include <string>

#include <desktop/dllapi.h>

namespace crashreport
{
// when response = nullptr only make test
/** Read+Send, Test and send info from the Dump.ini .

        @param [in] iniPath Path-file to the read/test ini-file (UTF-8 on Windows)
        @param [in] response=nullptr in this case made the Test only
        @param [in] response!=nullptr in this case made the Read+Send

        @retval true       Read+Send, Test was okay
        @retval false      Read+Send, Test is a error
*/

CRASHREPORT_DLLPUBLIC bool readConfig(const std::string& iniPath, std::string* response);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
