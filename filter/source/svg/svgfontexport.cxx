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


#include "svgfontexport.hxx"
#include "svgfilter.hxx"
#include "svgwriter.hxx"


#include <vcl/unohelp.hxx>
#include <vcl/font.hxx>
#include <vcl/outdev.hxx>

static const sal_Int32 nFontEM = 2048;

// -----------------
// - SVGFontExport -
// -----------------

SVGFontExport::SVGFontExport( SVGExport& rExport, const ::std::vector< ObjectRepresentation >& rObjects ) :
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

SVGFontExport::GlyphSet& SVGFontExport::implGetGlyphSet( const Font& rFont )
{
    FontWeight      eWeight( WEIGHT_NORMAL );
    FontItalic      eItalic( ITALIC_NONE );
    ::rtl::OUString aFontName( rFont.GetName() );
    sal_Int32       nNextTokenPos( 0 );

    switch( rFont.GetWeight() )
    {
        case WEIGHT_BOLD:
        case WEIGHT_ULTRABOLD:
        case WEIGHT_BLACK:
            eWeight = WEIGHT_BOLD;
        break;

        default:
        break;
    }

    if( rFont.GetItalic() != ITALIC_NONE )
        eItalic = ITALIC_NORMAL;

    return( maGlyphTree[ aFontName.getToken( 0, ';', nNextTokenPos ) ][ eWeight ][ eItalic ] );
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

            for( size_t i = 0, nCount = rMtf.GetActionSize(); i < nCount; ++i )
            {
                ::rtl::OUString     aText;
                MetaAction*         pAction = rMtf.GetAction( i );
                const sal_uInt16    nType = pAction->GetType();

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

                if( !aText.isEmpty() )
                {
                    GlyphSet& rGlyphSet = implGetGlyphSet( aVDev.GetFont() );
                    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > xBI(
                        ::vcl::unohelper::CreateBreakIterator() );

                    if( xBI.is() )
                    {
                        const ::com::sun::star::lang::Locale&   rLocale = Application::GetSettings().GetLocale();
                        sal_Int32                               nCurPos = 0, nLastPos = -1;

                        while( ( nCurPos < aText.getLength() ) && ( nCurPos > nLastPos ) )
                        {
                            sal_Int32 nCount2 = 1;

                            nLastPos = nCurPos;
                            nCurPos = xBI->nextCharacters( aText, nCurPos, rLocale,
                                                           ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL,
                                                           nCount2, nCount2 );

                            rGlyphSet.insert( aText.copy( nLastPos, nCurPos - nLastPos ) );
                        }
                    }
                    else
                    {
                        const sal_Unicode* pStr = aText.getStr();

                        for( sal_uInt32 k = 0, nLen = aText.getLength(); k < nLen; ++k )
                            rGlyphSet.insert( rtl::OUString( pStr[ k ] ) );
                    }
                }
            }

            aVDev.Pop();
        }

        ++aIter;
    }
}

// -----------------------------------------------------------------------------

void SVGFontExport::implEmbedFont( const Font& rFont )
{
    if( mrExport.IsEmbedFonts() )
    {
        GlyphSet& rGlyphSet = implGetGlyphSet( rFont );

        if( !rGlyphSet.empty() )
        {
            GlyphSet::const_iterator    aIter( rGlyphSet.begin() );
            const ::rtl::OUString       aEmbeddedFontStr( B2UCONST( "EmbeddedFont_" ) );

            {
                SvXMLElementExport  aExp( mrExport, XML_NAMESPACE_NONE, "defs", sal_True, sal_True );
                ::rtl::OUString     aCurIdStr( aEmbeddedFontStr );
                ::rtl::OUString     aUnitsPerEM( ::rtl::OUString::valueOf( nFontEM ) );
                VirtualDevice       aVDev;
                Font                aFont( rFont );

                aFont.SetSize( Size( 0, nFontEM ) );
                aFont.SetAlign( ALIGN_BASELINE );

                aVDev.SetMapMode( MAP_100TH_MM );
                aVDev.SetFont( aFont );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", aCurIdStr += ::rtl::OUString::valueOf( ++mnCurFontId ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", aUnitsPerEM );

                {
                    SvXMLElementExport  aExp2( mrExport, XML_NAMESPACE_NONE, "font", sal_True, sal_True );
                    ::rtl::OUString     aFontWeight;
                    ::rtl::OUString     aFontStyle;
                    const Size         aSize( nFontEM, nFontEM );

                    // Font Weight
                    if( aFont.GetWeight() != WEIGHT_NORMAL )
                        aFontWeight = B2UCONST( "bold" );
                    else
                        aFontWeight = B2UCONST( "normal" );

                    // Font Italic
                    if( aFont.GetItalic() != ITALIC_NONE )
                        aFontStyle = B2UCONST( "italic" );
                    else
                        aFontStyle = B2UCONST( "normal" );

                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-family", GetMappedFontName( rFont.GetName() ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "units-per-em", aUnitsPerEM );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-weight", aFontWeight );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-style", aFontStyle );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "ascent", ::rtl::OUString::valueOf( aVDev.GetFontMetric().GetAscent() ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "descent", ::rtl::OUString::valueOf( aVDev.GetFontMetric().GetDescent() ) );

                    {
                        SvXMLElementExport aExp3( mrExport, XML_NAMESPACE_NONE, "font-face", sal_True, sal_True );
                    }

                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", ::rtl::OUString::valueOf( aSize.Width() ) );

                    {
                        const Point         aPos;
                        const PolyPolygon   aMissingGlyphPolyPoly( Rectangle( aPos, aSize ) );

                        mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", SVGActionWriter::GetPathString( aMissingGlyphPolyPoly, sal_False ) );

                        {
                            SvXMLElementExport  aExp4( mrExport, XML_NAMESPACE_NONE, "missing-glyph", sal_True, sal_True );
                        }
                    }

                    while( aIter != rGlyphSet.end() )
                    {
                        implEmbedGlyph( aVDev, *aIter );
                        ++aIter;
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SVGFontExport::implEmbedGlyph( OutputDevice& rOut, const ::rtl::OUString& rCellStr )
{
    PolyPolygon         aPolyPoly;
    const sal_Unicode   nSpace = ' ';

    if( rOut.GetTextOutline( aPolyPoly, rCellStr ) )
    {
        Rectangle aBoundRect;

        aPolyPoly.Scale( 1.0, -1.0 );

        if( !rOut.GetTextBoundRect( aBoundRect, rCellStr ) )
            aBoundRect = Rectangle( Point( 0, 0 ), Size( rOut.GetTextWidth( rCellStr ), 0 ) );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, "unicode", rCellStr );

        if( rCellStr[ 0 ] == nSpace && rCellStr.getLength() == 1 )
            aBoundRect = Rectangle( Point( 0, 0 ), Size( rOut.GetTextWidth( rtl::OUString(' ') ), 0 ) );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", ::rtl::OUString::valueOf( aBoundRect.GetWidth() ) );

        const ::rtl::OUString aPathString( SVGActionWriter::GetPathString( aPolyPoly, sal_False ) );
        if( !aPathString.isEmpty() )
        {
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", aPathString );
        }

        {
            SvXMLElementExport aExp( mrExport, XML_NAMESPACE_NONE, "glyph", sal_True, sal_True );
        }
    }
}

// -----------------------------------------------------------------------------

void SVGFontExport::EmbedFonts()
{
    implCollectGlyphs();

    GlyphTree::const_iterator aGlyphTreeIter( maGlyphTree.begin() );

    while( aGlyphTreeIter != maGlyphTree.end() )
    {
        const FontWeightMap&            rFontWeightMap = (*aGlyphTreeIter).second;
        FontWeightMap::const_iterator   aFontWeightIter( rFontWeightMap.begin() );

        while( aFontWeightIter != rFontWeightMap.end() )
        {
            const FontItalicMap&            rFontItalicMap = (*aFontWeightIter).second;
            FontItalicMap::const_iterator   aFontItalicIter( rFontItalicMap.begin() );

            while( aFontItalicIter != rFontItalicMap.end() )
            {
                Font aFont;

                aFont.SetName( (*aGlyphTreeIter).first );
                aFont.SetWeight( (*aFontWeightIter).first );
                aFont.SetItalic( (*aFontItalicIter).first );

                implEmbedFont( aFont );

                ++aFontItalicIter;
            }

            ++aFontWeightIter;
        }

        ++aGlyphTreeIter;
    }
}

// -----------------------------------------------------------------------------

::rtl::OUString SVGFontExport::GetMappedFontName( const ::rtl::OUString& rFontName ) const
{
    sal_Int32       nNextTokenPos( 0 );
    ::rtl::OUString aRet( rFontName.getToken( 0, ';', nNextTokenPos ) );

    if( mnCurFontId )
        aRet += B2UCONST( " embedded" );

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
