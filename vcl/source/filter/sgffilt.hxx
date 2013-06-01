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

#ifndef _SGFFILT_HXX
#define _SGFFILT_HXX
#include <tools/urlobj.hxx>

sal_uInt8 CheckSgfTyp(SvStream& rInp, sal_uInt16& nVersion);
bool SgfBMapFilter(SvStream& rInp, SvStream& rOut);
bool SgfVectFilter(SvStream& rInp, GDIMetaFile& rMtf);
bool SgfSDrwFilter(SvStream& rInp, GDIMetaFile& rMtf, INetURLObject aIniPath );

// Konstanten fÅr CheckSgfTyp()
#define SGF_BITIMAGE   1     /* Bitmap                      */
#define SGF_SIMPVECT   2     /* Einfaches Vectorformat      */
#define SGF_POSTSCRP   3     /* Postscript file             */
#define SGF_STARDRAW   7     /* StarDraw SGV-Datei          */
#define SGF_DONTKNOW 255     /* Unbekannt oder kein SGF/SGV */

#define SGV_VERSION    3     /* SGV mit anderer Version wird abgewiesen */
                             /* 3 entspricht StarDraw 2.00/2.01 MÑrz'93 */
#endif  //_SGFFILT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
