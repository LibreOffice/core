/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef NF_NFVERSI_HXX
#define NF_NFVERSI_HXX

//      file IDs

#define SV_NUMBERFORMATTER_VERSION_SYSTORE                  0x0004
#define SV_NUMBERFORMATTER_VERSION_KEYWORDS                 0x0005
#define SV_NUMBERFORMATTER_VERSION_NEWSTANDARD              0x0006
#define SV_NUMBERFORMATTER_VERSION_NF_TIME_HH_MMSS00        0x0007
#define SV_NUMBERFORMATTER_VERSION_NF_DATE_WW               0x0008
#define SV_NUMBERFORMATTER_VERSION_NEW_CURR                 0x0009
#define SV_NUMBERFORMATTER_VERSION_YEAR2000                 0x000a
#define SV_NUMBERFORMATTER_VERSION_TWODIGITYEAR             0x000b
#define SV_NUMBERFORMATTER_VERSION_NF_DATETIME_SYS_DDMMYYYY_HHMMSS  0x000c
#define SV_NUMBERFORMATTER_VERSION_CALENDAR                 0x000d
#define SV_NUMBERFORMATTER_VERSION_ADDITIONAL_I18N_FORMATS  0x000e
#define SV_NUMBERFORMATTER_VERSION_FIXED_FRACTION           0x000f

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
