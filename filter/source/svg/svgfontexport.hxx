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

#ifndef SVGFONTEXPORT_HXX
#define SVGFONTEXPORT_HXX

#include <comphelper/stl_types.hxx>
#include <tools/fontenum.hxx>


// -----------------
// - SVGFontExport -
// -----------------


class ObjectRepresentation;
class SVGExport;
class Font;
class OutputDevice;


class SVGFontExport
{
    typedef ::std::vector< ObjectRepresentation >                               ObjectVector;
    typedef ::std::set< ::rtl::OUString, ::std::greater< ::rtl::OUString > >    GlyphSet;
    typedef ::std::map< FontItalic, GlyphSet >                                  FontItalicMap;
    typedef ::std::map< FontWeight, FontItalicMap >                             FontWeightMap;
    typedef ::std::map< ::rtl::OUString, FontWeightMap >                        FontNameMap;
    typedef FontNameMap                                                         GlyphTree;

private:

    SVGExport&          mrExport;
    GlyphTree           maGlyphTree;
    ObjectVector        maObjects;
    sal_Int32           mnCurFontId;

    GlyphSet&           implGetGlyphSet( const Font& rFont );
    void                implCollectGlyphs();
    void                implEmbedFont( const Font& rFont );
    void                implEmbedGlyph( OutputDevice& rOut, const ::rtl::OUString& rCellStr );

public:

                        SVGFontExport( SVGExport& rExport, const ::std::vector< ObjectRepresentation >& rObjects );
                        ~SVGFontExport();

    void                EmbedFonts();
    ::rtl::OUString     GetMappedFontName( const ::rtl::OUString& rFontName ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
