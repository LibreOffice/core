 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"


#include "svgfontexport.hxx"

static const sal_Int32	nFontEM = 2048;

// -----------------
// - SVGFontExport -
// -----------------

SVGFontExport::SVGFontExport( SvXMLExport& rExport, const ::std::vector< ObjectRepresentation >& rObjects ) :
    mrExport( rExport ),
    maObjects( rObjects ),
    mnCurFontId( 0 )
{
}

// -----------------------------------------------------------------------------

SVGFontExport::~SVGFontExport()
{
}

// -----------------------------------------------------------------------------

void SVGFontExport::implCollectGlyphs()
{
    VirtualDevice					aVDev;
    ObjectVector::const_iterator	aIter( maObjects.begin() );

    aVDev.EnableOutput( sal_False );

    while( aIter != maObjects.end() )
    {
        if( (*aIter).HasRepresentation() )
        {
            const GDIMetaFile& rMtf = (*aIter).GetRepresentation();

            aVDev.Push();

            for( sal_uInt32 i = 0, nCount = rMtf.GetActionCount(); i < nCount; ++i )
            {
                ::rtl::OUString		aText;
                MetaAction*			pAction = rMtf.GetAction( i );
                const USHORT		nType = pAction->GetType();

                switch( nType )
                {
                    case( META_TEXT_ACTION ):
                    {
                        const MetaTextAction* pA = (const MetaTextAction*) pAction;
                        aText = String( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                    }			
                    break;

                    case( META_TEXTRECT_ACTION ):
                    {
                        const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;
                        aText = pA->GetText();
                    }
                    break;

                    case( META_TEXTARRAY_ACTION	):
                    {
                        const MetaTextArrayAction*	pA = (const MetaTextArrayAction*) pAction;
                        aText = String( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                    }
                    break;

                    case( META_STRETCHTEXT_ACTION ):
                    {
                        const MetaStretchTextAction* pA = (const MetaStretchTextAction*) pAction;
                        aText = String( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                    }
                    break;

                    default:
                        pAction->Execute( &aVDev );
                    break;
                }

                if( aText.getLength() )
                {
                    const String&		rFontName = aVDev.GetFont().GetName();
                    const sal_Unicode*	pStr = aText.getStr();

                    for( sal_uInt32 j = 0, nLen = aText.getLength(); j < nLen; ++j )
                        maGlyphs[ rFontName ].insert( pStr[ j ] );
                }
            }

            aVDev.Pop();
        }

        ++aIter;
    }
}

// -----------------------------------------------------------------------------

void SVGFontExport::implEmbedFont( const ::rtl::OUString& rFontName, const ::std::set< sal_Unicode >& rGlyphs )
{
#ifdef _SVG_EMBED_FONTS
    ::std::set< sal_Unicode >::const_iterator	aIter( rGlyphs.begin() );
    const ::rtl::OUString						aEmbeddedFontStr( B2UCONST( "EmbeddedFont_" ) );

    {
        SvXMLElementExport	aExp( mrExport, XML_NAMESPACE_NONE, "defs", TRUE, TRUE );
        ::rtl::OUString		aCurIdStr( aEmbeddedFontStr );
        ::rtl::OUString		aUnitsPerEM( SVGActionWriter::GetValueString( nFontEM ) );
        VirtualDevice		aVDev;
        Font			    aFont( rFontName, Size( 0, nFontEM ) );

        aVDev.SetMapMode( MAP_100TH_MM );
        aFont.SetAlign( ALIGN_BASELINE );
        aVDev.SetFont( aFont );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", aCurIdStr += SVGActionWriter::GetValueString( ++mnCurFontId ) );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", aUnitsPerEM );

        {
            SvXMLElementExport  aExp2( mrExport, XML_NAMESPACE_NONE, "font", TRUE, TRUE );
            Point               aPos;
            Size                aSize( nFontEM, nFontEM );
            PolyPolygon         aMissingGlyphPolyPoly( Rectangle( aPos, aSize ) );
    
            aMissingGlyphPolyPoly.Move( 0, -nFontEM );
            aMissingGlyphPolyPoly.Scale( 1.0, -1.0 );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-family", GetMappedFontName( rFontName ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "units-per-em", aUnitsPerEM );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "ascent", SVGActionWriter::GetValueString( aVDev.GetFontMetric().GetAscent() ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "descent", SVGActionWriter::GetValueString( aVDev.GetFontMetric().GetDescent() ) );

            {
                SvXMLElementExport aExp3( mrExport, XML_NAMESPACE_NONE, "font-face", TRUE, TRUE );
            }

            mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", SVGActionWriter::GetValueString( aSize.Width() ) );

            {
                SvXMLElementExport aExp3( mrExport, XML_NAMESPACE_NONE, "missing-glyph", TRUE, TRUE );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, "style", B2UCONST( "fill:none;stroke:black;stroke-width:33" ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", SVGActionWriter::GetPathString( aMissingGlyphPolyPoly, sal_False ) );

                {
                    SvXMLElementExport aExp4( mrExport, XML_NAMESPACE_NONE, "path", TRUE, TRUE );
                }
            }
            
            while( aIter != rGlyphs.end() )
            {
                implEmbedGlyph( aVDev, ::rtl::OUString( *aIter ) );
                ++aIter;
            }
        }
    }
#endif
}

// -----------------------------------------------------------------------------

void SVGFontExport::implEmbedGlyph( OutputDevice& rOut, const ::rtl::OUString& rGlyphs )
{
    PolyPolygon     	aPolyPoly;
    ::rtl::OUString 	aStr( rGlyphs );
    const sal_Unicode	nSpace = ' ';

    if( rOut.GetTextOutline( aPolyPoly, aStr ) )
    {
        Rectangle aBoundRect;

        aPolyPoly.Scale( 1.0, -1.0 );

        if( !rOut.GetTextBoundRect( aBoundRect, aStr ) )
            aBoundRect = Rectangle( Point( 0, 0 ), Size( rOut.GetTextWidth( aStr ), 0 ) );
    
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "unicode", aStr );
        
        if( rGlyphs[ 0 ] == nSpace )
            aBoundRect = Rectangle( Point( 0, 0 ), Size( rOut.GetTextWidth( sal_Unicode( 'x' ) ), 0 ) );
        
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", SVGActionWriter::GetValueString( aBoundRect.GetWidth() ) );

        {
            SvXMLElementExport    aExp( mrExport, XML_NAMESPACE_NONE, "glyph", TRUE, TRUE );
            const ::rtl::OUString aPathString( SVGActionWriter::GetPathString( aPolyPoly, sal_False ) );
                  
            if( aPathString.getLength() )
            {
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", aPathString );
    
                {
                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, B2UCONST( "path" ), TRUE, TRUE );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SVGFontExport::EmbedFonts()
{
    implCollectGlyphs();

    GlyphMap::const_iterator aIter( maGlyphs.begin() );

    while( aIter != maGlyphs.end() )
    {
        implEmbedFont( (*aIter).first, (*aIter).second );
        ++aIter;
    }
}

// -----------------------------------------------------------------------------

::rtl::OUString SVGFontExport::GetMappedFontName( const ::rtl::OUString& rFontName ) const
{
    ::rtl::OUString aRet( String( rFontName ).GetToken( 0, ';' ) );

#ifdef _SVG_EMBED_FONTS
    if( mnCurFontId )
        aRet += B2UCONST( " embedded" );
#endif

    return aRet;
}
