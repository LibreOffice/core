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

#ifndef INCLUDED_SVL_NFVERSI_HXX
#define INCLUDED_SVL_NFVERSI_HXX

//      file IDs
#define SV_NUMBERFORMATTER_VERSION_NEWSTANDARD              0x0006
#define SV_NUMBERFORMATTER_VERSION_ADDITIONAL_I18N_FORMATS  0x000e

#define SV_NUMBERFORMATTER_VERSION                          0x000f

// 1 to 1996-01-18
// 2    1996-01-19 added TT.MM.JJJJ
// 3    1996-02-13 (not emergency) added #.##0,00 CCC
// 4    1997-07-30 364i store what SYSTEM actually was (using existing
//                  Dummy field, no change of file format)
// 5    1997-08-07 non-German is not always English
//                  convert older non-German user defined formats on load
// 6    1997-10-17 new keyword NNN for long weekday without separator
//                  not evaluated in older versions!
//                  New standard date formats, DIN, EN etc.
// 7    1998-05-25 Standard format [HH]:MM:SS,00 (NF_TIME_HH_MMSS00) for
//                  automatic input recognition of 100th seconds with hours
// 8    1998-06-10 Standard format WW (NF_DATE_WW) for calendar week
// 9    1998-12-17 new currency formats [$DM-xxx]
// A    1999-01-25 store/load Year2000
// B    1999-02-12 Year2000 is general TwoDigitYearStart
// C    ????-??-?? date/time format of system variables
// D    2000-11-23 new calendar
// E    2001-01-19 additional formats provided by i18n
// F    2012-04-27 fixed fraction formats ?/4 and ??/100, actually added
//                  already 2010/2011 but without versioning

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
