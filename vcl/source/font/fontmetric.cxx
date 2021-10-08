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

#include <sal/config.h>

#include <i18nlangtag/mslangid.hxx>
#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/metric.hxx>
#include <vcl/outdev.hxx>
#include <sal/log.hxx>

#include <font/FontInstanceData.hxx>
#include <font/FontSelectPattern.hxx>
#include <font/LogicalFontInstance.hxx>
#include <font/PhysicalFontFace.hxx>
#include <sft.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::utl;

FontMetric::FontMetric()
:   mnAscent( 0 ),
    mnDescent( 0 ),
    mnIntLeading( 0 ),
    mnExtLeading( 0 ),
    mnLineHeight( 0 ),
    mnSlant( 0 ),
    mnBulletOffset( 0 ),
    mbFullstopCentered( false )
{}

FontMetric::FontMetric( const FontMetric& rFontMetric ) = default;

FontMetric::FontMetric(vcl::font::PhysicalFontFace const& rFace)
    : FontMetric()
{
    SetFamilyName(rFace.GetFamilyName());
    SetStyleName(rFace.GetStyleName());
    SetCharSet(rFace.GetCharSet());
    SetFamily(rFace.GetFamilyType());
    SetPitch(rFace.GetPitch());
    SetWeight(rFace.GetWeight());
    SetItalic(rFace.GetItalic());
    SetAlignment(TextAlign::ALIGN_TOP);
    SetWidthType(rFace.GetWidthType());
    SetQuality(rFace.GetQuality() );
}

FontMetric::~FontMetric()
{
}

FontMetric& FontMetric::operator=(const FontMetric& rFontMetric) = default;

FontMetric& FontMetric::operator=(FontMetric&& rFontMetric) = default;

bool FontMetric::operator==( const FontMetric& r ) const
{
    if( Font::operator!=(r) )
        return false;
    if (mbFullstopCentered != r.mbFullstopCentered)
        return false;
    if( mnAscent     != r.mnAscent )
        return false;
    if( mnDescent    != r.mnDescent )
        return false;
    if( mnIntLeading != r.mnIntLeading )
        return false;
    if( mnExtLeading != r.mnExtLeading )
        return false;
    if( mnSlant      != r.mnSlant )
        return false;

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
