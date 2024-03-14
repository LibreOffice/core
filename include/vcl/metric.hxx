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

#pragma once

#include <sal/config.h>

#include <tools/ref.hxx>
#include <tools/gen.hxx>

#include <vcl/dllapi.h>
#include <vcl/font.hxx>

class FontCharMap;
namespace vcl::font { class PhysicalFontFace; }

typedef tools::SvRef<FontCharMap> FontCharMapRef;

class VCL_DLLPUBLIC FontMetric : public vcl::Font
{
public:
    explicit            FontMetric();
                        FontMetric( const FontMetric& );  // TODO make this explicit
    SAL_DLLPRIVATE      FontMetric(vcl::font::PhysicalFontFace const& rFace);
                        ~FontMetric() override;

    sal_Int32           GetAscent() const                           { return mnAscent; }
    sal_Int32           GetDescent() const                          { return mnDescent; }
    sal_Int32           GetInternalLeading() const                  { return mnIntLeading; }
    sal_Int32           GetExternalLeading() const                  { return mnExtLeading; }
    sal_Int32           GetLineHeight() const                       { return mnLineHeight; } // TODO this is ascent + descnt
    sal_Int32           GetSlant() const                            { return mnSlant; }
    sal_Int32           GetBulletOffset() const                     { return mnBulletOffset; }
    sal_Int32           GetHangingBaseline() const                  { return mnHangingBaseline; }

    void                SetAscent( sal_Int32 nAscent )                   { mnAscent = nAscent; }
    void                SetDescent( sal_Int32 nDescent )                 { mnDescent = nDescent; }
    void                SetExternalLeading( sal_Int32 nExtLeading )      { mnExtLeading = nExtLeading; }
    void                SetInternalLeading( sal_Int32 nIntLeading )      { mnIntLeading = nIntLeading; }
    void                SetLineHeight( sal_Int32 nHeight )               { mnLineHeight = nHeight; } // TODO this is ascent + descent
    void                SetSlant( sal_Int32 nSlant )                     { mnSlant = nSlant; }
    void                SetBulletOffset( sal_Int32 nOffset )             { mnBulletOffset = nOffset; }
    void                SetHangingBaseline( sal_Int32 nBaseline )        { mnHangingBaseline = nBaseline; }

    bool                IsFullstopCentered() const                  { return mbFullstopCentered; }

    void                SetFullstopCenteredFlag( bool bCentered )   { mbFullstopCentered = bCentered; }

    using Font::operator=;
    FontMetric&         operator=( const FontMetric& rMetric );
    FontMetric&         operator=( FontMetric&& rMetric );
    bool                operator==( const FontMetric& rMetric ) const;
    bool                operator!=( const FontMetric& rMetric ) const
                            { return !operator==( rMetric ); }

    SAL_DLLPRIVATE bool EqualIgnoreColor( const FontMetric& ) const;

    // Compute value usable as hash.
    size_t              GetHashValueIgnoreColor() const;

private:
    SAL_DLLPRIVATE bool EqualNoBase( const FontMetric& ) const;
    SAL_DLLPRIVATE size_t GetHashValueNoBase() const;
    sal_Int32           mnAscent;                      // Ascent
    sal_Int32           mnDescent;                     // Descent
    sal_Int32           mnIntLeading;                  // Internal Leading
    sal_Int32           mnExtLeading;                  // External Leading
    sal_Int32           mnLineHeight;                  // Ascent+Descent+EmphasisMark
    sal_Int32           mnSlant;                       // Slant
    sal_Int32           mnBulletOffset;                // Offset for non-printing character
    sal_Int32           mnHangingBaseline;             // Offset from Romn baseline to hanging baseline.

    bool                mbFullstopCentered;
};

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const FontMetric& rMetric )
{
    stream << "{"
           << "name=" << "\"" << rMetric.GetFamilyName() << "\""
           << ",size=(" << rMetric.GetFontSize().Width() << "," << rMetric.GetFontSize().Height() << ")"
           << ",ascent=" << rMetric.GetAscent()
           << ",descent=" << rMetric.GetDescent()
           << ",intLeading=" << rMetric.GetInternalLeading()
           << ",extLeading=" << rMetric.GetExternalLeading()
           << ",lineHeight=" << rMetric.GetLineHeight()
           << ",slant=" << rMetric.GetSlant()
           << "}";
    return stream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
