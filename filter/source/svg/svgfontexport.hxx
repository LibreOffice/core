 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svgfontexport.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:56:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    typedef ::std::hash_map< ::rtl::OUString, ::std::set< sal_Unicode >, ::comphelper::UStringHash > GlyphMap;
    typedef ::std::vector< ObjectRepresentation > ObjectVector;

private:

    SvXMLExport&        mrExport;
    GlyphMap            maGlyphs;
    ObjectVector        maObjects;
    sal_uInt32          mnCurFontId;

    void                implCollectGlyphs();
    void                implEmbedFont( const ::rtl::OUString& rFontName, const ::std::set< sal_Unicode >& rGlyphs );
    void                implEmbedGlyph( OutputDevice& rOut, const ::rtl::OUString& rGlyphs );

public:

                        SVGFontExport( SvXMLExport& rExport, const ::std::vector< ObjectRepresentation >& rObjects );
                        ~SVGFontExport();

    void                EmbedFonts();
    ::rtl::OUString     GetMappedFontName( const ::rtl::OUString& rFontName ) const;
};

#endif
