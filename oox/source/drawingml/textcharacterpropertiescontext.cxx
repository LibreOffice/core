/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textcharacterpropertiescontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:28:07 $
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

#include "oox/drawingml/textcharacterpropertiescontext.hxx"

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/relations.hxx"
#include "textfontcontext.hxx"
#include "hyperlinkcontext.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::awt;

namespace oox { namespace drawingml {

// --------------------------------------------------------------------

// CT_TextCharacterProperties
TextCharacterPropertiesContext::TextCharacterPropertiesContext(
        ContextHandler& rParent,
        const Reference< XFastAttributeList >& rXAttributes,
        oox::drawingml::TextCharacterProperties& rTextCharacterProperties )
: ContextHandler( rParent )
, mrTextCharacterProperties( rTextCharacterProperties )
{
    AttributeList attribs( rXAttributes );
    PropertyMap& rPropertyMap( mrTextCharacterProperties.getTextCharacterPropertyMap() );

    rtl::OUString aVal( rXAttributes->getOptionalValue( XML_sz ) );
    if ( aVal.getLength() )
    {
        float fTextSize = GetTextSize( aVal );
        const rtl::OUString sCharHeight( CREATE_OUSTRING( "CharHeight" ) );
        const rtl::OUString sCharHeightAsian( CREATE_OUSTRING( "CharHeightAsian" ) );
        const rtl::OUString sCharHeightComplex( CREATE_OUSTRING( "CharHeightComplex" ) );
        rPropertyMap[ sCharHeight ] <<= fTextSize;
        rPropertyMap[ sCharHeightAsian ] <<= fTextSize;
        rPropertyMap[ sCharHeightComplex ] <<= fTextSize;
    }

    bool bIsBold = attribs.getBool( XML_b, false );
    const rtl::OUString sCharWeight( CREATE_OUSTRING( "CharWeight" ) );
    const rtl::OUString sCharWeightAsian( CREATE_OUSTRING( "CharWeightAsian" ) );
    const rtl::OUString sCharWeightComplex( CREATE_OUSTRING( "CharWeightComplex" ) );
    rPropertyMap[ sCharWeight ] <<= ( bIsBold ? FontWeight::BOLD : FontWeight::NORMAL );
    rPropertyMap[ sCharWeightAsian ] <<= ( bIsBold ? FontWeight::BOLD : FontWeight::NORMAL );
    rPropertyMap[ sCharWeightComplex ] <<= ( bIsBold ? FontWeight::BOLD : FontWeight::NORMAL );

    bool bIsItalic = attribs.getBool( XML_i, false );
    const rtl::OUString sCharFontPosture( CREATE_OUSTRING( "CharPosture" ) );
    const rtl::OUString sCharFontPostureAsian( CREATE_OUSTRING( "CharPostureAsian" ) );
    const rtl::OUString sCharFontPostureComplex( CREATE_OUSTRING( "CharPostureComplex" ) );
    rPropertyMap[ sCharFontPosture ] <<= ( bIsItalic ? FontSlant_ITALIC : FontSlant_NONE );
    rPropertyMap[ sCharFontPostureAsian ] <<= ( bIsItalic ? FontSlant_ITALIC : FontSlant_NONE );
    rPropertyMap[ sCharFontPostureComplex ] <<= ( bIsItalic ? FontSlant_ITALIC : FontSlant_NONE );

    sal_Int32 nFontUnderline( rXAttributes->getOptionalValueToken( XML_u, 0 ) );
    if ( nFontUnderline )
    {
        const rtl::OUString sCharUnderline( CREATE_OUSTRING( "CharUnderline" ) );
        rPropertyMap[ sCharUnderline ] <<= GetFontUnderline( nFontUnderline );
        mrTextCharacterProperties.getHasUnderline() <<= sal_True;
    }

    const rtl::OUString sCharStrikeout( CREATE_OUSTRING( "CharStrikeout" ) );
    rPropertyMap[ sCharStrikeout ] <<= GetFontStrikeout( rXAttributes->getOptionalValueToken( XML_strike, XML_noStrike ) );

    // ST_TextCapsType
    const rtl::OUString sCharCaseMap( CREATE_OUSTRING( "CharCaseMap" ) );
    rPropertyMap[ sCharCaseMap ] <<= GetCaseMap( rXAttributes->getOptionalValueToken( XML_cap, XML_none ) );


    OUString sLang = rXAttributes->getOptionalValue( XML_lang );
    if( sLang.getLength( ) )
    {
        const rtl::OUString sCharLocale( CREATE_OUSTRING( "CharLocale" ) );
        const rtl::OUString sCharLocaleAsian( CREATE_OUSTRING( "CharLocaleAsian" ) );
        const rtl::OUString sCharLocaleComplex( CREATE_OUSTRING( "CharLocaleComplex" ) );

        com::sun::star::lang::Locale aLocale;
        OUString aString( sLang );
        sal_Int32  nSepPos = aString.indexOf( (sal_Unicode)'-', 0 );
        if ( nSepPos != -1 )
        {
            aLocale.Language = aString.copy( 0, nSepPos );
            aLocale.Country = aString.copy( nSepPos+1 );
        }
        else
        {
            aLocale.Language = aString;
        }

        rPropertyMap[ sCharLocale ] <<= aLocale;
        rPropertyMap[ sCharLocaleAsian ] <<= aLocale;
        rPropertyMap[ sCharLocaleComplex ] <<= aLocale;
    }


// TODO
/*   todo: we need to be able to iterate over the XFastAttributes

  // ST_TextNonNegativePoint
    const rtl::OUString sCharKerning( CREATE_OUSTRING( "CharKerning" ) );
    //case NMSP_DRAWINGML|XML_kern:

  // ST_TextLanguageID
    OUString sAltLang = rXAttributes->getOptionalValue( XML_altLang );

        case NMSP_DRAWINGML|XML_kumimoji:       // xsd:boolean
            break;
        case NMSP_DRAWINGML|XML_spc:            // ST_TextPoint
        case NMSP_DRAWINGML|XML_normalizeH:     // xsd:boolean
        case NMSP_DRAWINGML|XML_baseline:       // ST_Percentage
        case NMSP_DRAWINGML|XML_noProof:        // xsd:boolean
        case NMSP_DRAWINGML|XML_dirty:          // xsd:boolean
        case NMSP_DRAWINGML|XML_err:            // xsd:boolean
        case NMSP_DRAWINGML|XML_smtClean:       // xsd:boolean
        case NMSP_DRAWINGML|XML_smtId:          // xsd:unsignedInt
            break;
*/

}

TextCharacterPropertiesContext::~TextCharacterPropertiesContext()
{
    PropertyMap& rPropertyMap( mrTextCharacterProperties.getTextCharacterPropertyMap() );

    sal_Int16 nPitch, nFamily;

    if( maLatinFont.is() )
    {
        const rtl::OUString sCharFontName( CREATE_OUSTRING( "CharFontName" ) );
        const rtl::OUString sCharFontPitch( CREATE_OUSTRING( "CharFontPitch" ) );
        const rtl::OUString sCharFontFamily( CREATE_OUSTRING( "CharFontFamily" ) );
        GetFontPitch( maLatinFont.mnPitch, nPitch, nFamily);
        rPropertyMap[ sCharFontName ] <<= maLatinFont.msTypeface;
        rPropertyMap[ sCharFontPitch ] <<= nPitch;
        rPropertyMap[ sCharFontFamily ] <<= nFamily;
    }
    if( maAsianFont.is() )
    {
        const rtl::OUString sCharFontNameAsian( CREATE_OUSTRING( "CharFontNameAsian" ) );
        const rtl::OUString sCharFontPitchAsian( CREATE_OUSTRING( "CharFontPitchAsian" ) );
        const rtl::OUString sCharFontFamilyAsian( CREATE_OUSTRING( "CharFontFamilyAsian" ) );
        GetFontPitch( maAsianFont.mnPitch, nPitch, nFamily);
        rPropertyMap[ sCharFontNameAsian ] <<= maAsianFont.msTypeface;
        rPropertyMap[ sCharFontPitchAsian ] <<= nFamily;
        rPropertyMap[ sCharFontFamilyAsian ] <<= nPitch;
    }
    if( maComplexFont.is() )
    {
        const rtl::OUString sCharFontNameComplex( CREATE_OUSTRING( "CharFontNameComplex" ) );
        const rtl::OUString sCharFontPitchComplex( CREATE_OUSTRING( "CharFontPitchComplex" ) );
        const rtl::OUString sCharFontFamilyComplex( CREATE_OUSTRING( "CharFontFamilyComplex" ) );
        GetFontPitch( maComplexFont.mnPitch, nPitch, nFamily );
        rPropertyMap[ sCharFontNameComplex ] <<= maComplexFont.msTypeface;
        rPropertyMap[ sCharFontPitchComplex ] <<= nPitch;
        rPropertyMap[ sCharFontFamilyComplex ] <<= nFamily;
    }
}

// --------------------------------------------------------------------

void TextCharacterPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > TextCharacterPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttributes ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        // not supported.....
      case NMSP_DRAWINGML|XML_ln:           // CT_LineProperties
// TODO unsupported yet
//          xRet.set( new LinePropertiesContext( getHandler(), xAttributes, maTextOutlineProperties ) );
            break;
        // EG_FillProperties
      case NMSP_DRAWINGML|XML_solidFill:
            xRet.set( new colorChoiceContext( *this, *mrTextCharacterProperties.getCharColor() ) );
            break;
        // EG_EffectProperties
    case NMSP_DRAWINGML|XML_effectDag:
            // CT_EffectContainer 5.1.10.25
      case NMSP_DRAWINGML|XML_effectLst:
            // CT_EffectList 5.1.10.26
            break;

        case NMSP_DRAWINGML|XML_highlight:      //CT_Color
            xRet.set( new colorChoiceContext( *this, *mrTextCharacterProperties.getHighlightColor() ) );
            break;

        // EG_TextUnderlineLine
        case NMSP_DRAWINGML|XML_uLnTx:          // CT_TextUnderlineLineFollowText
             mrTextCharacterProperties.getUnderlineLineFollowText() <<= sal_True;
            break;
        case NMSP_DRAWINGML|XML_uLn:            // CT_LineProperties
// TODO unsupported yet
//          xRet.set( new LinePropertiesContext( getHandler(), xAttributes, maUnderlineProperties ) );
            break;

        // EG_TextUnderlineFill
        case NMSP_DRAWINGML|XML_uFillTx:        // CT_TextUnderlineFillFollowText
             mrTextCharacterProperties.getUnderlineFillFollowText() <<= sal_True;
            break;
        case NMSP_DRAWINGML|XML_uFill:          // CT_TextUnderlineFillGroupWrapper->EG_FillProperties (not supported)
            xRet.set( new colorChoiceContext( *this, *mrTextCharacterProperties.getUnderlineColor() ) );
            break;

        case NMSP_DRAWINGML|XML_ea:             // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken,  xAttributes, maAsianFont ) );
            break;
        case NMSP_DRAWINGML|XML_cs:             // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken,  xAttributes, maComplexFont ) );
            break;
        case NMSP_DRAWINGML|XML_sym:            // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken,  xAttributes, maSymbolFont ) );
            break;
        case NMSP_DRAWINGML|XML_latin:          // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken,  xAttributes, maLatinFont ) );
            break;
        case NMSP_DRAWINGML|XML_hlinkClick:     // CT_Hyperlink
        case NMSP_DRAWINGML|XML_hlinkMouseOver: // CT_Hyperlink
            xRet.set( new HyperLinkContext( *this, xAttributes,  mrTextCharacterProperties.getHyperlinkPropertyMap() ) );
            break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// --------------------------------------------------------------------

} }

