/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TOOLS_DATETIMEUTILS_HXX
#define INCLUDED_TOOLS_DATETIMEUTILS_HXX

#include <tools/datetime.hxx>

// This function converts a 'DateTime' object to an 'OString' object
TOOLS_DLLPUBLIC OString DateTimeToOString(const DateTime& rDateTime);
// This function converts a 'DateTime' object to an 'OUString' object
TOOLS_DLLPUBLIC OUString DateTimeToOUString(const DateTime& rDateTime);

// This function converts a 'Date' object to an 'OString' object in ISO-8601 representation
TOOLS_DLLPUBLIC OString DateToOString(const Date& rDate);

// This function converts a 'Date' object to an 'OUString' object in DD/MM/YYYY format
TOOLS_DLLPUBLIC OUString DateToDDMMYYYYOUString(const Date& rDate);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
