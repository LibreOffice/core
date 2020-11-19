/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_ODFFIELDS_HXX
#define INCLUDED_XMLOFF_ODFFIELDS_HXX

#define ODF_FORMTEXT "vnd.oasis.opendocument.field.FORMTEXT"
#define ODF_FORMFIELD_DEFAULT_LENGTH 5

#define ODF_FORMCHECKBOX "vnd.oasis.opendocument.field.FORMCHECKBOX"
#define ODF_FORMCHECKBOX_HELPTEXT "Checkbox_HelpText"
#define ODF_FORMCHECKBOX_NAME "Checkbox_Name"
#define ODF_FORMCHECKBOX_RESULT "Checkbox_Checked"

#define ODF_FORMDROPDOWN "vnd.oasis.opendocument.field.FORMDROPDOWN"
#define ODF_FORMDROPDOWN_LISTENTRY "Dropdown_ListEntry"
#define ODF_FORMDROPDOWN_RESULT "Dropdown_Selected"
#define ODF_FORMDROPDOWN_ENTRY_COUNT_LIMIT 25

#define ODF_FORMDATE "vnd.oasis.opendocument.field.FORMDATE"
#define ODF_FORMDATE_DATEFORMAT "DateField_DateFormat" // e.g. "MM.DD.YY"
#define ODF_FORMDATE_DATEFORMAT_LANGUAGE "DateField_DateFormat_Language" // e.g. "en-US", "hu-HU"
#define ODF_FORMDATE_CURRENTDATE "DateField_CurrentDate" // date string in a specific format
#define ODF_FORMDATE_CURRENTDATE_FORMAT "YYYY-MM-DD" // Coming from MSO
#define ODF_FORMDATE_CURRENTDATE_LANGUAGE LANGUAGE_ENGLISH_US

#define ODF_TOC "vnd.oasis.opendocument.field.TOC"

#define ODF_HYPERLINK "vnd.oasis.opendocument.field.HYPERLINK"

#define ODF_PAGEREF "vnd.oasis.opendocument.field.PAGEREF"

#define ODF_UNHANDLED "vnd.oasis.opendocument.field.UNHANDLED"
#define ODF_OLE_PARAM "vnd.oasis.opendocument.field.ole"
#define ODF_ID_PARAM "vnd.oasis.opendocument.field.id"
#define ODF_CODE_PARAM "vnd.oasis.opendocument.field.code"

#endif // INCLUDED_XMLOFF_ODFFIELDS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
