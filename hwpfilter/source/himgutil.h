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

#ifndef _HIMGUTIL_H_
#define _HIMGUTIL_H_

/**
 * Graphics file format identifiers.
 */
#define RFT_ERROR    -1                           /* couldn't open file, or whatever... */
#define RFT_UNKNOWN   0
#define RFT_GIF       1
#define RFT_PM        2
#define RFT_PBM       3
#define RFT_XBM       4
#define RFT_SUNRAS    5
#define RFT_BMP       6
#define RFT_UTAHRLE   7
#define RFT_IRIS      8
#define RFT_PCX       9
#define RFT_JFIF     10
#define RFT_TIFF     11
#define RFT_PDSVICAR 12
#define RFT_COMPRESS 13
#define RFT_PS       14
#define RFT_IFF      15
#define RFT_TARGA    16
#define RFT_XPM      17
#define RFT_XWD      18
#define RFT_FITS     19
#define RFT_WMF      20

// function declaration
struct EmPicture;

/**
 * Extract the name from given object
 * @param empic Object having name to extract
 * @returns Name of embeded picture
 */
const char *GetEmbImgname(const EmPicture *empic);
#endif                                            /* _HIMGUTIL_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
