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

#ifndef INCLUDED_VCL_METRIC_HXX
#define INCLUDED_VCL_METRIC_HXX

#include <vcl/dllapi.h>
#include <vcl/font.hxx>
#include <tools/ref.hxx>
#include <tools/gen.hxx>

class FontCharMap;

typedef tools::SvRef<FontCharMap> FontCharMapRef;

class VCL_DLLPUBLIC FontMetric : public vcl::Font
{
public:
    explicit            FontMetric();
                        FontMetric( const FontMetric& );  // TODO make this explicit
                        ~FontMetric() override;

    tools::Long                GetAscent() const                           { return mnAscent; }
    tools::Long                GetDescent() const                          { return mnDescent; }
    tools::Long                GetInternalLeading() const                  { return mnIntLeading; }
    tools::Long                GetExternalLeading() const                  { return mnExtLeading; }
    tools::Long                GetLineHeight() const                       { return mnLineHeight; } // TODO this is ascent + descnt
    tools::Long                GetSlant() const                            { return mnSlant; }
    tools::Long                GetBulletOffset() const                     { return mnBulletOffset; }

    void                SetAscent( tools::Long nAscent )                   { mnAscent = nAscent; }
    void                SetDescent( tools::Long nDescent )                 { mnDescent = nDescent; }
    void                SetExternalLeading( tools::Long nExtLeading )      { mnExtLeading = nExtLeading; }
    void                SetInternalLeading( tools::Long nIntLeading )      { mnIntLeading = nIntLeading; }
    void                SetLineHeight( tools::Long nHeight )               { mnLineHeight = nHeight; } // TODO this is ascent + descent
    void                SetSlant( tools::Long nSlant )                     { mnSlant = nSlant; }
    void                SetBulletOffset( tools::Long nOffset )             { mnBulletOffset = nOffset; }

    bool                IsFullstopCentered() const                  { return mbFullstopCentered; }

    void                SetFullstopCenteredFlag( bool bCentered )   { mbFullstopCentered = bCentered; }

    using Font::operator=;
    FontMetric&         operator=( const FontMetric& rMetric );
    FontMetric&         operator=( FontMetric&& rMetric );
    bool                operator==( const FontMetric& rMetric ) const;
    bool                operator!=( const FontMetric& rMetric ) const
                            { return !operator==( rMetric ); }
private:
    tools::Long                mnAscent;                      // Ascent
    tools::Long                mnDescent;                     // Descent
    tools::Long                mnIntLeading;                  // Internal Leading
    tools::Long                mnExtLeading;                  // External Leading
    tools::Long                mnLineHeight;                  // Ascent+Descent+EmphasisMark
    tools::Long                mnSlant;                       // Slant
    tools::Long                mnBulletOffset;                // Offset for non-printing character

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

#endif // INCLUDED_VCL_METRIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
