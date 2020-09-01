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
#ifndef INCLUDED_XMLOFF_CONTEXTID_HXX
#define INCLUDED_XMLOFF_CONTEXTID_HXX

/** These defines determine the unique ids for XML style-context-id's
    used in the SvXMLAutoStylePoolP.
 */

#define XML_SC_CTF_START    0x00001000
#define XML_SD_CTF_START    0x00002000
#define XML_TEXT_CTF_START  0x00003000
#define XML_SCH_CTF_START   0x00004000
#define XML_PM_CTF_START    0x00005000      // page master
#define XML_FORM_CTF_START  0x00006000
#define XML_DB_CTF_START    0x00007000


#define CTF_SD_CONTROL_SHAPE_DATA_STYLE     ( XML_SD_CTF_START +  1 )
#define CTF_SD_NUMBERINGRULES_NAME          ( XML_SD_CTF_START +  2 )
#define CTF_SD_SHAPE_PARA_ADJUST            ( XML_SD_CTF_START +  3 )

#define CTF_FORMS_DATA_STYLE                ( XML_FORM_CTF_START +  0 )

#endif // INCLUDED_XMLOFF_CONTEXTID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
