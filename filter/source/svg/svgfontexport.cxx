 /*************************************************************************
 *
 *  $RCSfile: svgfontexport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2002-08-16 10:56:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "svgfontexport.hxx"

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
    VirtualDevice                   aVDev;
    ObjectVector::const_iterator    aIter( maObjects.begin() );

    aVDev.EnableOutput( sal_False );

    while( aIter != maObjects.end() )
    {
        if( (*aIter).HasRepresentation() )
        {
            const GDIMetaFile& rMtf = (*aIter).GetRepresentation();

            aVDev.Push();

            for( sal_uInt32 i = 0, nCount = rMtf.GetActionCount(); i < nCount; ++i )
            {
                ::rtl::OUString     aText;
                MetaAction*         pAction = rMtf.GetAction( i );
                const USHORT        nType = pAction->GetType();

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

                    case( META_TEXTARRAY_ACTION ):
                    {
                        const MetaTextArrayAction*  pA = (const MetaTextArrayAction*) pAction;
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
                    const sal_Unicode* pStr = aText.getStr();

                    for( sal_uInt32 i = 0, nLen = aText.getLength(); i < nLen; ++i )
                        maGlyphs[ aVDev.GetFont().GetName() ].insert( pStr[ i ] );
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
    ::std::set< sal_Unicode >::const_iterator   aIter( rGlyphs.begin() );
    const ::rtl::OUString                       aEmbeddedFontStr( B2UCONST( "EmbeddedFont_" ) );

    {
        SvXMLElementExport  aExp( mrExport, XML_NAMESPACE_NONE, "defs", TRUE, TRUE );
        const sal_uInt32    nEM = 2048;
        ::rtl::OUString     aCurIdStr( aEmbeddedFontStr );
        ::rtl::OUString     aUnitsPerEM( SVGActionWriter::GetValueString( nEM ) );
        VirtualDevice       aVDev;
        const Font          aFont( rFontName, Size( 0, nEM ) );

        aVDev.SetMapMode( MAP_100TH_MM );
        aVDev.SetFont( aFont );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", aCurIdStr += SVGActionWriter::GetValueString( ++mnCurFontId ) );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", aUnitsPerEM );

        {
            SvXMLElementExport  aExp( mrExport, XML_NAMESPACE_NONE, "font", TRUE, TRUE );
            Point               aPos;
            Size                aSize( nEM >> 1, nEM );
            PolyPolygon         aMissingGlyphPolyPoly( Rectangle( aPos, aSize ) );

            aMissingGlyphPolyPoly.Move( 0, -aVDev.GetFontMetric().GetAscent() );
            aMissingGlyphPolyPoly.Scale( 1.0, -1.0 );

            mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-family", GetMappedFontName( rFontName ) );
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "units-per-em", aUnitsPerEM );

            {
                SvXMLElementExport aExp( mrExport, XML_NAMESPACE_NONE, "font-face", TRUE, TRUE );
            }

            mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", SVGActionWriter::GetValueString( aSize.Width() ) );

            {
                SvXMLElementExport aExp( mrExport, XML_NAMESPACE_NONE, "missing-glyph", TRUE, TRUE );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, "style", B2UCONST( "fill:none;stroke:black;stroke-width:33" ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", SVGActionWriter::GetPathString( aMissingGlyphPolyPoly, sal_False ) );

                {
                    SvXMLElementExport aExp( mrExport, XML_NAMESPACE_NONE, "path", TRUE, TRUE );
                }
            }

            while( aIter != rGlyphs.end() )
            {
                implEmbedGlyph( aVDev, ::rtl::OUString( *aIter ) );
                ++aIter;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SVGFontExport::implEmbedGlyph( OutputDevice& rOut, const ::rtl::OUString& rGlyphs )
{
    ::std::vector< PolyPolygon > aPolyPolys;

    if( rOut.GetTextOutlines( aPolyPolys, rGlyphs, 0, 0, 1, sal_False ) )
    {
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "unicode", rGlyphs );
        mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", SVGActionWriter::GetValueString( rOut.GetTextWidth( rGlyphs ) ) );

        {
            SvXMLElementExport                      aExp( mrExport, XML_NAMESPACE_NONE, "glyph", TRUE, TRUE );
            ::std::vector< PolyPolygon >::iterator  pIter( aPolyPolys.begin() );

            while( pIter != aPolyPolys.end() )
            {
                PolyPolygon& rPolyPoly = *pIter;

                rPolyPoly.Move( 0, -rOut.GetFontMetric().GetAscent() );
                rPolyPoly.Scale( 1.0, -1.0 );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", SVGActionWriter::GetPathString( rPolyPoly, sal_False ) );

                {
                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_NONE, B2UCONST( "path" ), TRUE, TRUE );
                }

                ++pIter;
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

    if( mnCurFontId )
        aRet += B2UCONST( " embedded" );

    return aRet;
}
