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

#include "sallayout.hxx"
#include "svdata.hxx"

#include "fontattributes.hxx"

FontAttributes::FontAttributes()
:   meWeight( WEIGHT_DONTKNOW ),
    meFamily( FAMILY_DONTKNOW ),
    mePitch( PITCH_DONTKNOW ),
    meWidthType ( WIDTH_DONTKNOW ),
    meItalic ( ITALIC_NONE ),
    meAlign ( ALIGN_TOP ),
    meCharSet( RTL_TEXTENCODING_DONTKNOW ),
    mbSymbolFlag( false ),
    mnQuality( 0 ),
    mbOrientation( false ),
    mbDevice( false ),
    mbSubsettable( false ),
    mbEmbeddable ( false )
{}

FontAttributes::FontAttributes( const FontAttributes& rFontAttributes ) :
    maFamilyName( rFontAttributes.maFamilyName ),
    maStyleName( rFontAttributes.maStyleName ),
    meWeight( rFontAttributes.meWeight ),
    meFamily( rFontAttributes.meFamily ),
    mePitch( rFontAttributes.mePitch ),
    meWidthType( rFontAttributes.meWidthType ),
    meItalic( rFontAttributes.meItalic ),
    meAlign( rFontAttributes.meAlign ),
    meCharSet( rFontAttributes.meCharSet ),
    mbSymbolFlag( rFontAttributes.mbSymbolFlag ),
    maMapNames( rFontAttributes.maMapNames ),
    mnQuality( rFontAttributes.mnQuality ),
    mbOrientation( rFontAttributes.mbOrientation ),
    mbDevice( rFontAttributes.mbDevice ),
    mbSubsettable( rFontAttributes.mbSubsettable ),
    mbEmbeddable( rFontAttributes.mbEmbeddable )
{}

bool FontAttributes::CompareDeviceIndependentFontAttributes(const FontAttributes& rOther) const
{
    if (maFamilyName != rOther.maFamilyName)
        return false;

    if (maStyleName != rOther.maStyleName)
        return false;

    if (meWeight != rOther.meWeight)
        return false;

    if (meItalic != rOther.meItalic)
        return false;

    if (meFamily != rOther.meFamily)
        return false;

    if (mePitch != rOther.mePitch)
        return false;

    if (meWidthType != rOther.meWidthType)
        return false;

    if (mbSymbolFlag != rOther.mbSymbolFlag)
        return false;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
