/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <impfont.hxx>
#include <vcl/metric.hxx>

#include <vector>
#include <set>

#include <cstdio>

// =======================================================================

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

// -----------------------------------------------------------------------

inline void ImplFontMetric::AddReference()
{
    // TODO: disable refcounting on the default maps?
    ++mnRefCount;
}

// -----------------------------------------------------------------------

inline void ImplFontMetric::DeReference()
{
    // TODO: disable refcounting on the default maps?
    if( --mnRefCount <= 0 )
        delete this;
}

// -----------------------------------------------------------------------

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

// =======================================================================

FontInfo::FontInfo()
:   mpImplMetric( new ImplFontMetric )
{}

// -----------------------------------------------------------------------

FontInfo::FontInfo( const FontInfo& rInfo )
:  Font( rInfo )
{
    mpImplMetric = rInfo.mpImplMetric;
    mpImplMetric->AddReference();
}

// -----------------------------------------------------------------------

FontInfo::~FontInfo()
{
    mpImplMetric->DeReference();
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

sal_Bool FontInfo::operator==( const FontInfo& rInfo ) const
{
    if( !Font::operator==( rInfo ) )
        return sal_False;
    if( mpImplMetric == rInfo.mpImplMetric )
        return sal_True;
    if( *mpImplMetric == *rInfo.mpImplMetric  )
        return sal_True;
    return sal_False;
}

// -----------------------------------------------------------------------

FontType FontInfo::GetType() const
{
    return (mpImplMetric->IsScalable() ? TYPE_SCALABLE : TYPE_RASTER);
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::IsDeviceFont() const
{
    return mpImplMetric->IsDeviceFont();
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::SupportsLatin() const
{
    return mpImplMetric->SupportsLatin();
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::SupportsCJK() const
{
    return mpImplMetric->SupportsCJK();
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::SupportsCTL() const
{
    return mpImplMetric->SupportsCTL();
}

// =======================================================================

FontMetric::FontMetric( const FontMetric& rMetric )
:    FontInfo( rMetric )
{}

// -----------------------------------------------------------------------

long FontMetric::GetAscent() const
{
    return mpImplMetric->GetAscent();
}

// -----------------------------------------------------------------------

long FontMetric::GetDescent() const
{
    return mpImplMetric->GetDescent();
}

// -----------------------------------------------------------------------

long FontMetric::GetIntLeading() const
{
    return mpImplMetric->GetIntLeading();
}

// -----------------------------------------------------------------------

long FontMetric::GetExtLeading() const
{
    return mpImplMetric->GetExtLeading();
}

// -----------------------------------------------------------------------

long FontMetric::GetLineHeight() const
{
    return mpImplMetric->GetLineHeight();
}

// -----------------------------------------------------------------------

long FontMetric::GetSlant() const
{
    return mpImplMetric->GetSlant();
}

// -----------------------------------------------------------------------

FontMetric& FontMetric::operator =( const FontMetric& rMetric )
{
    FontInfo::operator=( rMetric );
    return *this;
}

// -----------------------------------------------------------------------

sal_Bool FontMetric::operator==( const FontMetric& rMetric ) const
{
    return FontInfo::operator==( rMetric );
}

// =======================================================================

