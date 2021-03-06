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

#include <functional>
#include <map>
#include <set>
#include <vector>

#include <rtl/ustring.hxx>
#include <tools/fontenum.hxx>


class ObjectRepresentation;
class SVGExport;
namespace vcl { class Font; }
class OutputDevice;

#ifdef _MSC_VER
#pragma warning (disable : 4503) // FontNameMap gives decorated name length exceeded
#endif

class SVGFontExport
{
    typedef ::std::vector< ObjectRepresentation >                 ObjectVector;
    typedef ::std::set< OUString, ::std::greater< OUString > >    GlyphSet;
    typedef ::std::map< FontItalic, GlyphSet >                    FontItalicMap;
    typedef ::std::map< FontWeight, FontItalicMap >               FontWeightMap;
    typedef ::std::map< OUString, FontWeightMap >                 FontNameMap;
    typedef FontNameMap                                           GlyphTree;

private:

    SVGExport&          mrExport;
    GlyphTree           maGlyphTree;
    ObjectVector        maObjects;
    sal_Int32           mnCurFontId;

    GlyphSet&           implGetGlyphSet( const vcl::Font& rFont );
    void                implCollectGlyphs();
    void                implEmbedFont( const vcl::Font& rFont );
    void                implEmbedGlyph( OutputDevice const & rOut, const OUString& rCellStr );

public:

                        SVGFontExport( SVGExport& rExport, const ::std::vector< ObjectRepresentation >& rObjects );
                        ~SVGFontExport();

    void                EmbedFonts();
    OUString            GetMappedFontName( const OUString& rFontName ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
