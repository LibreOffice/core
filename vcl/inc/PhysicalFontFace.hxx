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

#include <vcl/dllapi.h>

#include "outfont.hxx"

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
class VCL_PLUGIN_PUBLIC PhysicalFontFace : public ImplDevFontAttributes
{
public:
    // by using an PhysicalFontFace object as a factory for its corresponding
    // ImplFontEntry an ImplFontEntry can be extended to cache device and
    // font instance specific data
    virtual ImplFontEntry*  CreateFontInstance( FontSelectPattern& ) const = 0;

    virtual int             GetHeight() const           { return mnHeight; }
    virtual int             GetWidth() const            { return mnWidth; }
    virtual sal_IntPtr      GetFontId() const = 0;
    int                     GetFontMagic() const        { return mnMagic; }
    bool                    IsScalable() const          { return (mnHeight == 0); }
    bool                    CheckMagic( int n ) const   { return (n == mnMagic); }
    PhysicalFontFace*       GetNextFace() const         { return mpNext; }
    PhysicalFontFace*       CreateAlias() const         { return Clone(); }

    bool                    IsBetterMatch( const FontSelectPattern&, FontMatchStatus& ) const;
    sal_Int32               CompareWithSize( const PhysicalFontFace& ) const;
    sal_Int32               CompareIgnoreSize( const PhysicalFontFace& ) const;
    virtual                ~PhysicalFontFace() {}
    virtual PhysicalFontFace* Clone() const = 0;

protected:
    explicit                PhysicalFontFace( const ImplDevFontAttributes&, int nMagic );
    void                    SetBitmapSize( int nW, int nH ) { mnWidth=nW; mnHeight=nH; }

    long                    mnWidth;    // Width (in pixels)
    long                    mnHeight;   // Height (in pixels)

private:
friend class PhysicalFontFamily;
    const int               mnMagic;    // poor man's RTTI
    PhysicalFontFace*       mpNext;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

