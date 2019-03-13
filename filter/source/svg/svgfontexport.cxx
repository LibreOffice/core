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
#include <i18nlangtag/languagetag.hxx>

static const sal_Int32 nFontEM = 2048;


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
    const OUString& aFontName( rFont.GetFamilyName() );

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

    return( maGlyphTree[ aFontName.getToken( 0, ';' ) ][ eWeight ][ eItalic ] );
}


void SVGFontExport::implCollectGlyphs()
{
    ScopedVclPtrInstance< VirtualDevice > pVDev;

    pVDev->EnableOutput( false );

    for (auto const& elem : maObjects)
    {
        if( elem.HasRepresentation() )
        {
            const GDIMetaFile& rMtf = elem.GetRepresentation();

            pVDev->Push();

            for( size_t i = 0, nCount = rMtf.GetActionSize(); i < nCount; ++i )
            {
                OUString             aText;
                MetaAction*          pAction = rMtf.GetAction( i );
                const MetaActionType nType = pAction->GetType();

                switch( nType )
                {
                    case MetaActionType::TEXT:
                    {
                        const MetaTextAction* pA = static_cast<const MetaTextAction*>(pAction);
                        sal_Int32             aLength=std::min( pA->GetText().getLength(), pA->GetLen() );
                        aText = pA->GetText().copy( pA->GetIndex(), aLength );
                    }
                    break;

                    case MetaActionType::TEXTRECT:
                    {
                        const MetaTextRectAction* pA = static_cast<const MetaTextRectAction*>(pAction);
                        aText = pA->GetText();
                    }
                    break;

                    case MetaActionType::TEXTARRAY:
                    {
                        const MetaTextArrayAction*  pA = static_cast<const MetaTextArrayAction*>(pAction);
                        sal_Int32                   aLength=std::min( pA->GetText().getLength(), pA->GetLen() );
                        aText = pA->GetText().copy( pA->GetIndex(), aLength );
                    }
                    break;

                    case MetaActionType::STRETCHTEXT:
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
    }
}


void SVGFontExport::implEmbedFont( const vcl::Font& rFont )
{
    if( mrExport.IsEmbedFonts() )
    {
        GlyphSet& rGlyphSet = implGetGlyphSet( rFont );

        if( !rGlyphSet.empty() )
        {
            const OUString              aEmbeddedFontStr( "EmbeddedFont_" );

            {
                SvXMLElementExport  aExp( mrExport, XML_NAMESPACE_NONE, "defs", true, true );
                OUString     aCurIdStr( aEmbeddedFontStr );
                OUString     aUnitsPerEM( OUString::number( nFontEM ) );
                ScopedVclPtrInstance< VirtualDevice > pVDev;
                vcl::Font           aFont( rFont );

                aFont.SetFontSize( Size( 0, nFontEM ) );
                aFont.SetAlignment( ALIGN_BASELINE );

                pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));
                pVDev->SetFont( aFont );

                aCurIdStr += OUString::number( ++mnCurFontId );
                mrExport.AddAttribute( XML_NAMESPACE_NONE, "id", aCurIdStr );
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

                    mrExport.AddAttribute( XML_NAMESPACE_NONE, "font-family", GetMappedFontName( rFont.GetFamilyName() ) );
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
                        const tools::PolyPolygon   aMissingGlyphPolyPoly( tools::Rectangle( aPos, aSize ) );

                        mrExport.AddAttribute( XML_NAMESPACE_NONE, "d", SVGActionWriter::GetPathString( aMissingGlyphPolyPoly, false ) );

                        {
                            SvXMLElementExport  aExp4( mrExport, XML_NAMESPACE_NONE, "missing-glyph", true, true );
                        }
                    }
                    for (auto const& glyph : rGlyphSet)
                    {
                        implEmbedGlyph( *pVDev, glyph);
                    }
                }
            }
        }
    }
}


void SVGFontExport::implEmbedGlyph( OutputDevice const & rOut, const OUString& rCellStr )
{
    tools::PolyPolygon         aPolyPoly;
    const sal_Unicode   nSpace = ' ';

    if( rOut.GetTextOutline( aPolyPoly, rCellStr ) )
    {
        tools::Rectangle aBoundRect;

        aPolyPoly.Scale( 1.0, -1.0 );

        if( !rOut.GetTextBoundRect( aBoundRect, rCellStr ) )
            aBoundRect = tools::Rectangle( Point( 0, 0 ), Size( rOut.GetTextWidth( rCellStr ), 0 ) );

        mrExport.AddAttribute( XML_NAMESPACE_NONE, "unicode", rCellStr );

        if( rCellStr[ 0 ] == nSpace && rCellStr.getLength() == 1 )
            aBoundRect = tools::Rectangle( Point( 0, 0 ), Size( rOut.GetTextWidth( OUString(' ') ), 0 ) );

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

    for (auto const& glyph : maGlyphTree)
    {
        const FontWeightMap&            rFontWeightMap = glyph.second;
        for (auto const& fontWeight : rFontWeightMap)
        {
            const FontItalicMap&            rFontItalicMap = fontWeight.second;
            for (auto const& fontItalic : rFontItalicMap)
            {
                vcl::Font aFont;

                aFont.SetFamilyName( glyph.first );
                aFont.SetWeight( fontWeight.first );
                aFont.SetItalic( fontItalic.first );

                implEmbedFont( aFont );
            }
        }
    }
}


OUString SVGFontExport::GetMappedFontName( const OUString& rFontName ) const
{
    OUString aRet( rFontName.getToken( 0, ';' ) );

    if( mnCurFontId )
        aRet += " embedded";

    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
