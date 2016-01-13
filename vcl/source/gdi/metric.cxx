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
    mnBulletOffset( 0 ),
    mnMiscFlags( 0 ),
    mnRefCount( 1 ),
    mbScalableFont( false ),
    mbFullstopCentered( false )
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
    if( mbScalableFont != r.mbScalableFont
        || mbFullstopCentered != r.mbFullstopCentered )
        return false;
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

FontMetric::FontMetric()
:   mpImplMetric( new ImplFontMetric )
{}

FontMetric::FontMetric( const FontMetric& rFontMetric )
:  Font( rFontMetric )
{
    mpImplMetric = rFontMetric.mpImplMetric;
    mpImplMetric->AddReference();
}

FontMetric::~FontMetric()
{
    mpImplMetric->DeReference();
}

FontMetric& FontMetric::operator=( const FontMetric& rFontMetric )
{
    Font::operator=( rFontMetric );

    if( mpImplMetric != rFontMetric.mpImplMetric )
    {
        mpImplMetric->DeReference();
        mpImplMetric = rFontMetric.mpImplMetric;
        mpImplMetric->AddReference();
    }

    return *this;
}

bool FontMetric::operator==( const FontMetric& rFontMetric ) const
{
    if( !Font::operator==( rFontMetric ) )
        return false;
    if( mpImplMetric == rFontMetric.mpImplMetric )
        return true;
    if( *mpImplMetric == *rFontMetric.mpImplMetric  )
        return true;
    return false;
}

FontType FontMetric::GetType() const
{
    return (mpImplMetric->IsScalable() ? TYPE_SCALABLE : TYPE_RASTER);
}

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

long FontMetric::GetBulletOffset() const
{
    return mpImplMetric->GetBulletOffset();
}

bool FontMetric::IsScalable() const
{
    return mpImplMetric->IsScalable();
}

void FontMetric::SetScalableFlag(bool bScalable)
{
    mpImplMetric->SetScalableFlag( bScalable );
}

bool FontMetric::IsFullstopCentered() const
{
    return mpImplMetric->IsFullstopCentered();
}

void FontMetric::SetFullstopCenteredFlag(bool bScalable)
{
    mpImplMetric->SetFullstopCenteredFlag( bScalable );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
