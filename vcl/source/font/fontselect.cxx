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

#include "i18nlangtag/mslangid.hxx"

#include <unotools/configmgr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>
#include <vcl/outdev.hxx>
#include <vcl/edit.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/fontcharmap.hxx>

#include "sallayout.hxx"
#include "svdata.hxx"

#include "impfont.hxx"
#include "outdata.hxx"
#include "fontinstance.hxx"
#include "fontattributes.hxx"

#include "outdev.h"
#include "window.h"

#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "PhysicalFontFamily.hxx"

#include "svids.hrc"

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include "graphite_features.hxx"
#endif

#include "../gdi/pdfwriter_impl.hxx"

#include <boost/functional/hash.hpp>
#include <cmath>
#include <cstring>
#include <memory>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::utl;

FontSelectPattern::FontSelectPattern( const vcl::Font& rFont,
    const OUString& rSearchName, const Size& rSize, float fExactHeight)
    : FontSelectPatternAttributes(rFont, rSearchName, rSize, fExactHeight)
    , mpFontData( nullptr )
    , mpFontInstance( nullptr )
{
}


FontSelectPatternAttributes::FontSelectPatternAttributes( const vcl::Font& rFont,
    const OUString& rSearchName, const Size& rSize, float fExactHeight )
    : maSearchName( rSearchName )
    , mnWidth( rSize.Width() )
    , mnHeight( rSize.Height() )
    , mfExactHeight( fExactHeight)
    , mnOrientation( rFont.GetOrientation() )
    , meLanguage( rFont.GetLanguage() )
    , mbVertical( rFont.IsVertical() )
    , mbNonAntialiased( false )
    , mbEmbolden( false )
{
    maTargetName = GetFamilyName();

    rFont.GetFontAttributes( *this );

    // normalize orientation between 0 and 3600
    if( 3600 <= (unsigned)mnOrientation )
    {
        if( mnOrientation >= 0 )
            mnOrientation %= 3600;
        else
            mnOrientation = 3600 - (-mnOrientation % 3600);
    }

    // normalize width and height
    if( mnHeight < 0 )
        mnHeight = -mnHeight;
    if( mnWidth < 0 )
        mnWidth = -mnWidth;
}


// NOTE: this ctor is still used on Windows. Do not remove.
#ifdef WNT
FontSelectPatternAttributes::FontSelectPatternAttributes( const PhysicalFontFace& rFontData,
    const Size& rSize, float fExactHeight, int nOrientation, bool bVertical )
    : ImplFontAttributes( rFontData )
    , mnWidth( rSize.Width() )
    , mnHeight( rSize.Height() )
    , mfExactHeight( fExactHeight )
    , mnOrientation( nOrientation )
    , meLanguage( 0 )
    , mbVertical( bVertical )
    , mbNonAntialiased( false )
    , mbEmbolden( false )
{
    maTargetName = maSearchName = GetFamilyName();
    // NOTE: no normalization for width/height/orientation
}

FontSelectPattern::FontSelectPattern( const PhysicalFontFace& rFontData,
    const Size& rSize, float fExactHeight, int nOrientation, bool bVertical )
    : FontSelectPatternAttributes(rFontData, rSize, fExactHeight, nOrientation, bVertical)
    , mpFontData( &rFontData )
    , mpFontInstance( NULL )
{
}
#endif

void FontSelectPattern::copyAttributes(const FontSelectPatternAttributes &rAttributes)
{
    static_cast<FontSelectPatternAttributes&>(*this) = rAttributes;
}

size_t FontSelectPatternAttributes::hashCode() const
{
    // TODO: does it pay off to improve this hash function?
    size_t nHash;
#if ENABLE_GRAPHITE
    // check for features and generate a unique hash if necessary
    if (maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
        != -1)
    {
        nHash = maTargetName.hashCode();
    }
    else
#endif
    {
        nHash = maSearchName.hashCode();
    }
    nHash += 11 * mnHeight;
    nHash += 19 * GetWeight();
    nHash += 29 * GetSlantType();
    nHash += 37 * mnOrientation;
    nHash += 41 * meLanguage;
    if( mbVertical )
        nHash += 53;
    return nHash;
}

bool FontSelectPatternAttributes::operator==(const FontSelectPatternAttributes& rOther) const
{
    if (!CompareDeviceIndependentFontAttributes(rOther))
        return false;

    if (maTargetName != rOther.maTargetName)
        return false;

    if (maSearchName != rOther.maSearchName)
        return false;

    if (mnWidth != rOther.mnWidth)
        return false;

    if (mnHeight != rOther.mnHeight)
        return false;

    if (mfExactHeight != rOther.mfExactHeight)
        return false;

    if (mnOrientation != rOther.mnOrientation)
        return false;

    if (meLanguage != rOther.meLanguage)
        return false;

    if (mbVertical != rOther.mbVertical)
        return false;

    if (mbNonAntialiased != rOther.mbNonAntialiased)
        return false;

    if (mbEmbolden != rOther.mbEmbolden)
        return false;

    if (maItalicMatrix != rOther.maItalicMatrix)
        return false;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
