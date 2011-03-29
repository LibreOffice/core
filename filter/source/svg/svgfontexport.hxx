 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svgfontexport.hxx,v $
 * $Revision: 1.2.110.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SVGFONTEXPORT_HXX
#define SVGFONTEXPORT_HXX

#include <comphelper/stl_types.hxx>
#include "svgfilter.hxx"
#include "svgwriter.hxx"

// -----------------
// - SVGFontExport -
// -----------------

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
