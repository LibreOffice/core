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

#ifndef INCLUDED_SVX_AUTOFORMATHELPER_HXX
#define INCLUDED_SVX_AUTOFORMATHELPER_HXX

#include <svx/svxdllapi.h>

class SvStream;

/// Struct with version numbers of the Items
struct SVX_DLLPUBLIC AutoFormatVersions
{
public:
    // BlockA
    sal_uInt16 nFontVersion;
    sal_uInt16 nFontHeightVersion;
    sal_uInt16 nWeightVersion;
    sal_uInt16 nPostureVersion;
    sal_uInt16 nUnderlineVersion;
    sal_uInt16 nOverlineVersion;
    sal_uInt16 nCrossedOutVersion;
    sal_uInt16 nContourVersion;
    sal_uInt16 nShadowedVersion;
    sal_uInt16 nColorVersion;
    sal_uInt16 nBoxVersion;
    sal_uInt16 nLineVersion;
    sal_uInt16 nBrushVersion;
    sal_uInt16 nAdjustVersion;

    // BlockB
    sal_uInt16 nHorJustifyVersion;
    sal_uInt16 nVerJustifyVersion;
    sal_uInt16 nOrientationVersion;
    sal_uInt16 nMarginVersion;
    sal_uInt16 nBoolVersion;
    sal_uInt16 nInt32Version;
    sal_uInt16 nRotateModeVersion;
    sal_uInt16 nNumFormatVersion;

    AutoFormatVersions();

    void LoadBlockA( SvStream& rStream, sal_uInt16 nVer );
    void LoadBlockB( SvStream& rStream, sal_uInt16 nVer );

    static void WriteBlockA(SvStream& rStream, sal_uInt16 fileVersion);
    static void WriteBlockB(SvStream& rStream, sal_uInt16 fileVersion);
};

#endif // INCLUDED_SVX_AUTOFORMATHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
