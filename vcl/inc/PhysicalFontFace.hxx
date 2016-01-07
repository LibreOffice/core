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

#ifndef INCLUDED_VCL_INC_PHYSICALFONTFACE_HXX
#define INCLUDED_VCL_INC_PHYSICALFONTFACE_HXX

#include <vcl/dllapi.h>

#include "fontentry.hxx"
#include "fontattributes.hxx"
#include "fontselect.hxx"

class ImplFontEntry;
struct FontMatchStatus;
class FontSelectPattern;
class PhysicalFontFamily;

struct FontMatchStatus
{
public:
    int                 mnFaceMatch;
    int                 mnHeightMatch;
    int                 mnWidthMatch;
    const OUString*     mpTargetStyleName;
};

// - PhysicalFontFace -

// TODO: no more direct access to members
// TODO: add reference counting
// TODO: get rid of height/width for scalable fonts
// TODO: make cloning cheaper

// abstract base class for physical font faces

// Note about physical and logical fonts:
//
// A physical font is a concept we have taken from the Java world
//
// From https://docs.oracle.com/javase/tutorial/2d/text/fonts.html
//
//      There are two types of fonts: physical fonts and logical fonts.
//      Physical fonts are the actual font libraries consisting of, for
//      example, TrueType or PostScript Type 1 fonts. The physical fonts
//      may be Time, Helvetica, Courier, or any number of other fonts,
//      including international fonts. Logical fonts are the following
//      five font families: Serif, SansSerif, Monospaced, Dialog, and
//      DialogInput. These logical fonts are not actual font libraries.
//      Instead, the logical font names are mapped to physical fonts by
//      the Java runtime environment.

class VCL_PLUGIN_PUBLIC PhysicalFontFace : public ImplFontAttributes
{
public:
    virtual                ~PhysicalFontFace() {}

    // by using an PhysicalFontFace object as a factory for its corresponding
    // ImplFontEntry an ImplFontEntry can be extended to cache device and
    // font instance specific data
    virtual ImplFontEntry*  CreateFontInstance( FontSelectPattern& ) const = 0;
    virtual PhysicalFontFace* Clone() const = 0;

    int                     GetHeight() const           { return mnHeight; }
    int                     GetWidth() const            { return mnWidth; }
    virtual sal_IntPtr      GetFontId() const = 0;
    bool                    IsScalable() const          { return (mnHeight == 0); }

    bool                    IsBetterMatch( const FontSelectPattern&, FontMatchStatus& ) const;
    sal_Int32               CompareWithSize( const PhysicalFontFace& ) const;
    sal_Int32               CompareIgnoreSize( const PhysicalFontFace& ) const;

protected:
    explicit                PhysicalFontFace( const ImplFontAttributes& );
    void                    SetBitmapSize( int nW, int nH ) { mnWidth=nW; mnHeight=nH; }

    long                    mnWidth;    // Width (in pixels)
    long                    mnHeight;   // Height (in pixels)
};

#endif // INCLUDED_VCL_INC_PHYSICALFONTFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

