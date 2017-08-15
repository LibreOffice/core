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

    sal_Int32           GetAscent() const                           { return mnAscent; }
    sal_Int32           GetDescent() const                          { return mnDescent; }
    sal_Int32           GetInternalLeading() const                  { return mnIntLeading; }
    sal_Int32           GetExternalLeading() const                  { return mnExtLeading; }
    sal_Int32           GetLineHeight() const                       { return mnLineHeight; } // TODO this is ascent + descnt
    sal_Int32           GetSlant() const                            { return mnSlant; }
    sal_Int32           GetBulletOffset() const                     { return mnBulletOffset; }

    void                SetAscent( sal_Int32 nAscent )                   { mnAscent = nAscent; }
    void                SetDescent( sal_Int32 nDescent )                 { mnDescent = nDescent; }
    void                SetInternalLeading( sal_Int32 nIntLeading )      { mnIntLeading = nIntLeading; }
    void                SetExternalLeading( sal_Int32 nExtLeading )      { mnExtLeading = nExtLeading; }
    void                SetLineHeight( sal_Int32 nHeight )               { mnLineHeight = nHeight; } // TODO this is ascent + descent
    void                SetSlant( sal_Int32 nSlant )                     { mnSlant = nSlant; }
    void                SetBulletOffset( sal_Int32 nOffset )             { mnBulletOffset = nOffset; }

    bool                IsFullstopCentered() const                  { return mbFullstopCentered; }

    void                SetFullstopCenteredFlag( bool bCentered )   { mbFullstopCentered = bCentered; }

    bool                operator==( const ImplFontMetric& ) const;

private:
    friend class FontMetric;

    sal_Int32           mnAscent;                      // Ascent
    sal_Int32           mnDescent;                     // Descent
    sal_Int32           mnIntLeading;                  // Internal Leading
    sal_Int32           mnExtLeading;                  // External Leading
    sal_Int32           mnLineHeight;                  // Ascent+Descent+EmphasisMark
    sal_Int32           mnSlant;                       // Slant
    sal_Int32           mnBulletOffset;                // Offset for non-printing character

    bool                mbFullstopCentered;

};

#endif // INCLUDED_VCL_INC_IMPFONTMETRIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
