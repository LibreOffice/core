/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
