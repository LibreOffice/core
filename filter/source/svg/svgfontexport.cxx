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
#include <vcl/settings.hxx>

static const sal_Int32 nFontEM = 2048;


// - SVGFontExport -


SVGFontExport::SVGFontExport( SVGExport& rExport, const ::std::vector< ObjectRepresentation >& rObjects ) :
    mrExport( rExport ),
    maObjects( rObjects ),
    mnCurFontId( 0 )
{
}



SVGFontExport::~SVGFontExport()
{
}



SVGFontExport::GlyphSet& SVGFontExport::implGetGlyphSet( const vcl::Font& rFont )
{
    FontWeight      eWeight( WEIGHT_NORMAL );
    FontItalic      eItalic( ITALIC_NONE );
    OUString aFontName( rFont.GetName() );
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



void SVGFontExport::implCollectGlyphs()
{
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    ObjectVector::const_iterator    aIter( maObjects.begin() );

    pVDev->EnableOutput( false );

    while( aIter != maObjects.end() )
    {
        if( (*aIter).HasRepresentation() )
        {
            const GDIMetaFile& rMtf = (*aIter).GetRepresentation();

            pVDev->Push();

            for( size_t i = 0, nCount = rMtf.GetActionSize(); i < nCount; ++i )
            {
                OUString             aText;
                MetaAction*          pAction = rMtf.GetAction( i );
                const MetaActionType nType = pAction->GetType();

                switch( nType )
                {
                    case( MetaActionType::TEXT ):
                    {
                        const MetaTextAction* pA = static_cast<const MetaTextAction*>(pAction);
                        sal_Int32             aLength=std::min( pA->GetText().getLength(), pA->GetLen() );
                        aText = pA->GetText().copy( pA->GetIndex(), aLength );
                    }
                    break;

                    case( MetaActionType::TEXTRECT ):
                    {
                        const MetaTextRectAction* pA = static_cast<const MetaTextRectAction*>(pAction);
                        aText = pA->GetText();
                    }
                    break;

                    case( MetaActionType::TEXTARRAY ):
                    {
                        const MetaTextArrayAction*  pA = static_cast<const MetaTextArrayAction*>(pAction);
                        sal_Int32                   aLength=std::min( pA->GetText().getLength(), pA->GetLen() );
                        aText = pA->GetText().copy( pA->GetIndex(), aLength );
                    }
                    break;

                    case( MetaActionType::STRETCHTEXT ):
                    {
                        const MetaStretchTextAction* pA = static_cast<const MetaStretchTextAction*>(pAction);
                        sal_Int32                    aLength=std::min( pA->GetText().getLength(), pA->GetLen() );
                        aText = pA->GetText().copy( pA->GetIndex(), aLength );
                    }
                    break;

                    default:
                        pAction->Execute( pVDev );
                    break;
                }

                if( !aText.isEmpty() )
                {
                    GlyphSet& rGlyphSet = implGetGlyphSet( pVDev->GetFont() );
                    css::uno::Reference< css::i18n::XBreakIterator > xBI(
                        vcl::unohelper::CreateBreakIterator() );

                    if( xBI.is() )
                    {
                        const css::lang::Locale&   rLocale = Application::GetSettings().GetLanguageTag().getLocale();
                        sal_Int32                               nCurPos = 0, nLastPos = -1;

                        while( ( nCurPos < aText.getLength() ) && ( nCurPos > nLastPos ) )
                        {
                            sal_Int32 nCount2 = 1;

                            nLastPos = nCurPos;
                            nCurPos = xBI->nextCharacters( aText, nCurPos, rLocale,
                                                           css::i18n::CharacterIteratorMode::SKIPCELL,
                                                           nCount2, nCount2 );

                            rGlyphSet.insert( aText.copy( nLastPos, nCurPos - nLastPos ) );
                        }
                    }
                    else
                    {
                        const sal_Unicode* pStr = aText.getStr();

                        for( sal_uInt32 k = 0, nLen = aText.getLength(); k < nLen; ++k )
                            rGlyphSet.insert( OUString( pStr[ k ] ) );
                    }
                }
            }

            pVDev->Pop();
        }

        ++aIter;
    }
}



void SVGFontExport::implEmbedFont( const vcl::Font& rFont )
{
    if( mrExport.IsEmbedFonts() )
    {
        GlyphSet& rGlyphSet = implGetGlyphSet( rFont );

        if( !rGlyphSet.empty() )
        {
            GlyphSet::const_iterator    aIter( rGlyphSet.begin() );
            const OUString              aEmbeddedFontStr( "EmbeddedFont_" );

            {
                SvXMLElementExport  aExp( mrExport, XML_NAMESPACE_NONE, "defs", true, true );
                OUString     aCurIdStr( aEmbeddedFontStr );
                OUString     aUnitsPerEM( OUString::number( nFontEM ) );
                ScopedVclPtrInstance< VirtualDevice > pVDev;
                vcl::Font           aFont( rFont );

                aFont.SetSize( Size( 0, nFontEM ) );
                aFont.SetAlign( ALIGN_BASELINE );

                pVDev->SetMapMode( MAP_100TH_MM );
                pVDev->SetFont( aFont );

                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", aCurIdStr += OUString::number( ++mnCurFontId ) );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", aUnitsPerEM );

                {
                    SvXMLElementExport  aExp2( mrExport, XML_NAMESPACE_NONE, "font", true, true );
                    OUString     aFontWeight;
                    OUString     aFontStyle;
                    const Size         aSize( nFontEM, nFontEM );

                    // Font Weight
                    if( aFont.GetWeight() != WEIGHT_NORMAL )
                        aFontWeight = "bold";
                    else
                        aFontWeight = "normal";

                    // Font Italic
                    if( aFont.GetItalic() != ITALIC_NONE )
                        aFontStyle = "italic";
                    else
                        aFontStyle = "normal";

                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-family", GetMappedFontName( rFont.GetName() ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "units-per-em", aUnitsPerEM );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-weight", aFontWeight );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-style", aFontStyle );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "ascent", OUString::number( pVDev->GetFontMetric().GetAscent() ) );
                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "descent", OUString::number( pVDev->GetFontMetric().GetDescent() ) );

                    {
                        SvXMLElementExport aExp3( mrExport, XML_NAMESPACE_NONE, "font-face", true, true );
                    }

                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", OUString::number( aSize.Width() ) );

                    {
                        const Point         aPos;
                        const tools::PolyPolygon   aMissingGlyphPolyPoly( Rectangle( aPos, aSize ) );

                        mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", SVGActionWriter::GetPathString( aMissingGlyphPolyPoly, false ) );

                        {
                            SvXMLElementExport  aExp4( mrExport, XML_NAMESPACE_NONE, "missing-glyph", true, true );
                        }
                    }

                    while( aIter != rGlyphSet.end() )
                    {
                        implEmbedGlyph( *pVDev.get(), *aIter );
                        ++aIter;
                    }
                }
            }
        }
    }
}



void SVGFontExport::implEmbedGlyph( OutputDevice& rOut, const OUString& rCellStr )
{
    tools::PolyPolygon         aPolyPoly;
    const sal_Unicode   nSpace = ' ';

    if( rOut.GetTextOutline( aPolyPoly, rCellStr ) )
    {
        Rectangle aBoundRect;

        aPolyPoly.Scale( 1.0, -1.0 );

        if( !rOut.GetTextBoundRect( aBoundRect, rCellStr ) )
            aBoundRect = Rectangle( Point( 0, 0 ), Size( rOut.GetTextWidth( rCellStr ), 0 ) );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, "unicode", rCellStr );

        if( rCellStr[ 0 ] == nSpace && rCellStr.getLength() == 1 )
            aBoundRect = Rectangle( Point( 0, 0 ), Size( rOut.GetTextWidth( OUString(' ') ), 0 ) );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, "horiz-adv-x", OUString::number( aBoundRect.GetWidth() ) );

        const OUString aPathString( SVGActionWriter::GetPathString( aPolyPoly, false ) );
        if( !aPathString.isEmpty() )
        {
            mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", aPathString );
        }

        {
            SvXMLElementExport aExp( mrExport, XML_NAMESPACE_NONE, "glyph", true, true );
        }
    }
}



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
                vcl::Font aFont;

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



OUString SVGFontExport::GetMappedFontName( const OUString& rFontName ) const
{
    sal_Int32       nNextTokenPos( 0 );
    OUString aRet( rFontName.getToken( 0, ';', nNextTokenPos ) );

    if( mnCurFontId )
        aRet += " embedded";

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
