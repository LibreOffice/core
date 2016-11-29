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

#ifndef INCLUDED_VCL_INC_IMPFONTMETRIC_HXX
#define INCLUDED_VCL_INC_IMPFONTMETRIC_HXX

class ImplFontCharMap;
typedef tools::SvRef<ImplFontCharMap> ImplFontCharMapRef;

class ImplFontMetric : public SvRefBase
{
public:
    explicit            ImplFontMetric();

    long                GetAscent() const                           { return mnAscent; }
    long                GetDescent() const                          { return mnDescent; }
    long                GetInternalLeading() const                  { return mnIntLeading; }
    long                GetExternalLeading() const                  { return mnExtLeading; }
    long                GetLineHeight() const                       { return mnLineHeight; } // TODO this is ascent + descnt
    long                GetSlant() const                            { return mnSlant; }
    long                GetBulletOffset() const                     { return mnBulletOffset; }

    void                SetAscent( long nAscent )                   { mnAscent = nAscent; }
    void                SetDescent( long nDescent )                 { mnDescent = nDescent; }
    void                SetInternalLeading( long nIntLeading )      { mnIntLeading = nIntLeading; }
    void                SetExternalLeading( long nExtLeading )      { mnExtLeading = nExtLeading; }
    void                SetLineHeight( long nHeight )               { mnLineHeight = nHeight; } // TODO this is ascent + descent
    void                SetSlant( long nSlant )                     { mnSlant = nSlant; }
    void                SetBulletOffset( long nOffset )             { mnBulletOffset = nOffset; }

    bool                IsFullstopCentered() const                  { return mbFullstopCentered; }

    void                SetFullstopCenteredFlag( bool bCentered )   { mbFullstopCentered = bCentered; }

    bool                operator==( const ImplFontMetric& ) const;

private:
    friend class FontMetric;

    long                mnAscent;                      // Ascent
    long                mnDescent;                     // Descent
    long                mnIntLeading;                  // Internal Leading
    long                mnExtLeading;                  // External Leading
    long                mnLineHeight;                  // Ascent+Descent+EmphasisMark
    long                mnSlant;                       // Slant
    long                mnBulletOffset;                // Offset for non-printing character

    bool                mbFullstopCentered;

};

#endif // INCLUDED_VCL_INC_IMPFONTMETRIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
