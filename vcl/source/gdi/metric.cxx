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

#include <impfont.hxx>
#include <vcl/metric.hxx>

#include <vector>
#include <set>

#include <cstdio>

ImplFontMetric::ImplFontMetric()
:   mnAscent( 0 ),
    mnDescent( 0 ),
    mnIntLeading( 0 ),
    mnExtLeading( 0 ),
    mnLineHeight( 0 ),
    mnSlant( 0 ),
    mnMiscFlags( 0 ),
    mnRefCount( 1 )
{}

inline void ImplFontMetric::AddReference()
{
    // TODO: disable refcounting on the default maps?
    ++mnRefCount;
}

inline void ImplFontMetric::DeReference()
{
    // TODO: disable refcounting on the default maps?
    if( --mnRefCount <= 0 )
        delete this;
}

bool ImplFontMetric::operator==( const ImplFontMetric& r ) const
{
    if( mnMiscFlags  != r.mnMiscFlags )
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

namespace vcl {

FontInfo::FontInfo()
:   mpImplMetric( new ImplFontMetric )
{}

FontInfo::FontInfo( const FontInfo& rInfo )
:  Font( rInfo )
{
    mpImplMetric = rInfo.mpImplMetric;
    mpImplMetric->AddReference();
}

FontInfo::~FontInfo()
{
    mpImplMetric->DeReference();
}

FontInfo& FontInfo::operator=( const FontInfo& rInfo )
{
    Font::operator=( rInfo );

    if( mpImplMetric != rInfo.mpImplMetric )
    {
        mpImplMetric->DeReference();
        mpImplMetric = rInfo.mpImplMetric;
        mpImplMetric->AddReference();
    }

    return *this;
}

bool FontInfo::operator==( const FontInfo& rInfo ) const
{
    if( !Font::operator==( rInfo ) )
        return false;
    if( mpImplMetric == rInfo.mpImplMetric )
        return true;
    if( *mpImplMetric == *rInfo.mpImplMetric  )
        return true;
    return false;
}

FontType FontInfo::GetType() const
{
    return (mpImplMetric->IsScalable() ? TYPE_SCALABLE : TYPE_RASTER);
}

}

FontMetric::FontMetric( const FontMetric& rMetric ):
    vcl::FontInfo( rMetric )
{}

long FontMetric::GetAscent() const
{
    return mpImplMetric->GetAscent();
}

long FontMetric::GetDescent() const
{
    return mpImplMetric->GetDescent();
}

long FontMetric::GetIntLeading() const
{
    return mpImplMetric->GetIntLeading();
}

long FontMetric::GetExtLeading() const
{
    return mpImplMetric->GetExtLeading();
}

long FontMetric::GetLineHeight() const
{
    return mpImplMetric->GetLineHeight();
}

long FontMetric::GetSlant() const
{
    return mpImplMetric->GetSlant();
}

bool FontMetric::IsFullstopCentered() const
{
    return mpImplMetric->IsFullstopCentered();
}

FontMetric& FontMetric::operator =( const FontMetric& rMetric )
{
    vcl::FontInfo::operator=( rMetric );
    return *this;
}

bool FontMetric::operator==( const FontMetric& rMetric ) const
{
    return vcl::FontInfo::operator==( rMetric );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
