/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textparagraphpropertiescontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:30:33 $
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

#include "oox/drawingml/textparagraphpropertiescontext.hxx"

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/textcharacterpropertiescontext.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "textspacingcontext.hxx"
#include "textfontcontext.hxx"
#include "texttabstoplistcontext.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using ::com::sun::star::awt::FontDescriptor;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;

namespace oox { namespace drawingml {

// CT_TextParagraphProperties
TextParagraphPropertiesContext::TextParagraphPropertiesContext( ContextHandler& rParent,
                                                                const Reference< XFastAttributeList >& xAttribs,
                                                                TextParagraphProperties& rTextParagraphProperties )
: ContextHandler( rParent )
, mrTextParagraphProperties( rTextParagraphProperties )
, mrSpaceBefore( rTextParagraphProperties.getParaTopMargin() )
, mrSpaceAfter( rTextParagraphProperties.getParaBottomMargin() )
, mrBulletList( rTextParagraphProperties.getBulletList() )
{
    OUString sValue;
    AttributeList attribs( xAttribs );

    PropertyMap& rPropertyMap( mrTextParagraphProperties.getTextParagraphPropertyMap() );

    // ST_TextAlignType
    if ( xAttribs->hasAttribute( XML_algn ) )
    {
        sal_Int32 nAlign = xAttribs->getOptionalValueToken( XML_algn, XML_l );
        const OUString sParaAdjust( CREATE_OUSTRING( "ParaAdjust" ) );
        rPropertyMap[ sParaAdjust ] <<= GetParaAdjust( nAlign );
    }
//  OSL_TRACE( "OOX: para adjust %d", GetParaAdjust( nAlign ));
    // TODO see to do the same with RubyAdjust

    // ST_Coordinate32
//  sValue = xAttribs->getOptionalValue( XML_defTabSz );    SJ: we need to be able to set the default tab size for each text object,
//                                                          this is possible at the moment only for the whole document.
//  sal_Int32 nDefTabSize = ( sValue.getLength() == 0 ? 0 : GetCoordinate(  sValue ) );
    // TODO

//  bool bEaLineBrk = attribs.getBool( XML_eaLnBrk, true );
    if ( xAttribs->hasAttribute( XML_latinLnBrk ) )
    {
        bool bLatinLineBrk = attribs.getBool( XML_latinLnBrk, true );
        const OUString sParaIsHyphenation( CREATE_OUSTRING( "ParaIsHyphenation" ) );
        rPropertyMap[ sParaIsHyphenation ] <<= bLatinLineBrk;
    }
    // TODO see what to do with Asian hyphenation

    // ST_TextFontAlignType
    // TODO
//  sal_Int32 nFontAlign = xAttribs->getOptionalValueToken( XML_fontAlgn, XML_base );

    if ( xAttribs->hasAttribute( XML_hangingPunct ) )
    {
        bool bHangingPunct = attribs.getBool( XML_hangingPunct, false );
        const OUString sParaIsHangingPunctuation( CREATE_OUSTRING( "ParaIsHangingPunctuation" ) );
        rPropertyMap[ sParaIsHangingPunctuation ] <<= bHangingPunct;
    }

  // ST_Coordinate
    if ( xAttribs->hasAttribute( XML_indent ) )
    {
        sValue = xAttribs->getOptionalValue( XML_indent );
        const OUString sParaFirstLineIndent( CREATE_OUSTRING( "ParaFirstLineIndent" ) );
        rPropertyMap[ sParaFirstLineIndent ] <<= ( sValue.getLength() == 0 ? 0 : GetCoordinate(  sValue ) );
    }

  // ST_TextIndentLevelType
    // -1 is an invalid value and denote the lack of level
    sal_Int32 nLevel = attribs.getInteger( XML_lvl, 0 );
    if( nLevel > 8 || nLevel < 0 )
    {
        nLevel = 0;
    }

    mrTextParagraphProperties.setLevel( static_cast< sal_Int16 >( nLevel ) );

    char name[] = "Outline X";
    name[8] = static_cast<char>( '1' + nLevel );
    const OUString sStyleNameValue( rtl::OUString::createFromAscii( name ) );
    mrBulletList.setStyleName( sStyleNameValue );

    // ST_TextMargin
    // ParaLeftMargin
    if ( xAttribs->hasAttribute( XML_marL ) )
    {
        sValue = xAttribs->getOptionalValue( XML_marL );
        sal_Int32 nMarL = ( sValue.getLength() == 0 ? 0 : GetCoordinate(  sValue ) );
        const OUString sParaLeftMargin( CREATE_OUSTRING( "ParaLeftMargin" ) );
        rPropertyMap[ sParaLeftMargin ] <<= nMarL;
    }

    // ParaRightMargin
    if ( xAttribs->hasAttribute( XML_marR ) )
    {
        sValue = xAttribs->getOptionalValue( XML_marR );
        sal_Int32 nMarR  = ( sValue.getLength() == 0 ? 0 : GetCoordinate( sValue ) );
        const OUString sParaRightMargin( CREATE_OUSTRING( "ParaRightMargin" ) );
        rPropertyMap[ sParaRightMargin ] <<= nMarR;
    }

    if ( xAttribs->hasAttribute( XML_rtl ) )
    {
        bool bRtl = attribs.getBool( XML_rtl, false );
        const OUString sTextWritingMode( CREATE_OUSTRING( "TextWritingMode" ) );
        rPropertyMap[ sTextWritingMode ] <<= ( bRtl ? WritingMode_RL_TB : WritingMode_LR_TB );
    }
}



TextParagraphPropertiesContext::~TextParagraphPropertiesContext()
{
    PropertyMap& rPropertyMap( mrTextParagraphProperties.getTextParagraphPropertyMap() );
    if ( maLineSpacing.bHasValue )
    {
        const OUString sParaLineSpacing( CREATE_OUSTRING( "ParaLineSpacing" ) );
        //OSL_TRACE( "OOX: ParaLineSpacing unit = %d, value = %d", maLineSpacing.nUnit, maLineSpacing.nValue );
        rPropertyMap[ sParaLineSpacing ] <<= maLineSpacing.toLineSpacing();
    }
    ::std::list< TabStop >::size_type nTabCount = maTabList.size();
    if( nTabCount != 0 )
    {
        Sequence< TabStop > aSeq( nTabCount );
        TabStop * aArray = aSeq.getArray();
        OSL_ENSURE( aArray != NULL, "sequence array is NULL" );
        ::std::copy( maTabList.begin(), maTabList.end(), aArray );
        const OUString sParaTabStops( CREATE_OUSTRING( "ParaTabStops" ) );
        rPropertyMap[ sParaTabStops ] <<= aSeq;
    }
    if( mrBulletList.is() )
    {
        const rtl::OUString sIsNumbering( CREATE_OUSTRING( "IsNumbering" ) );
        rPropertyMap[ sIsNumbering ] <<= sal_True;
    }
    sal_Int16 nLevel = mrTextParagraphProperties.getLevel();
//  OSL_TRACE("OOX: numbering level = %d", nLevel );
    const OUString sNumberingLevel( CREATE_OUSTRING( "NumberingLevel" ) );
    rPropertyMap[ sNumberingLevel ] <<= (sal_Int16)nLevel;
    sal_Bool bTmp = sal_True;
    const OUString sNumberingIsNumber( CREATE_OUSTRING( "NumberingIsNumber" ) );
    rPropertyMap[ sNumberingIsNumber ] <<= bTmp;
}

// --------------------------------------------------------------------

void TextParagraphPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}



// --------------------------------------------------------------------

Reference< XFastContextHandler > TextParagraphPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& rXAttributes ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case NMSP_DRAWINGML|XML_lnSpc:          // CT_TextSpacing
            xRet.set( new TextSpacingContext( *this, maLineSpacing ) );
            break;
        case NMSP_DRAWINGML|XML_spcBef:         // CT_TextSpacing
            xRet.set( new TextSpacingContext( *this, mrSpaceBefore ) );
            break;
        case NMSP_DRAWINGML|XML_spcAft:         // CT_TextSpacing
            xRet.set( new TextSpacingContext( *this, mrSpaceAfter ) );
            break;

        // EG_TextBulletColor
        case NMSP_DRAWINGML|XML_buClrTx:        // CT_TextBulletColorFollowText ???
            mrBulletList.mbBulletColorFollowText <<= sal_True;
            break;
        case NMSP_DRAWINGML|XML_buClr:          // CT_Color
            xRet.set( new colorChoiceContext( *this, *mrBulletList.maBulletColorPtr ) );
            break;

        // EG_TextBulletSize
        case NMSP_DRAWINGML|XML_buSzTx:         // CT_TextBulletSizeFollowText
            mrBulletList.setBulletSize(100);
            break;
        case NMSP_DRAWINGML|XML_buSzPct:        // CT_TextBulletSizePercent
            mrBulletList.setBulletSize( static_cast<sal_Int16>( GetPercent( rXAttributes->getOptionalValue( XML_val ) ) / 1000 ) );
            break;
        case NMSP_DRAWINGML|XML_buSzPts:        // CT_TextBulletSizePoint
            mrBulletList.setBulletSize(0);
            mrBulletList.setFontSize( static_cast<sal_Int16>(GetTextSize( rXAttributes->getOptionalValue( XML_val ) ) ) );
            break;

        // EG_TextBulletTypeface
        case NMSP_DRAWINGML|XML_buFontTx:       // CT_TextBulletTypefaceFollowText
            mrBulletList.mbBulletFontFollowText <<= sal_True;
            break;
        case NMSP_DRAWINGML|XML_buFont:         // CT_TextFont
            xRet.set( new TextFontContext( *this, aElementToken, rXAttributes, mrBulletList.maBulletFont ) );
            break;

        // EG_TextBullet
        case NMSP_DRAWINGML|XML_buNone:         // CT_TextNoBullet
            mrBulletList.setNone();
            break;
        case NMSP_DRAWINGML|XML_buAutoNum:      // CT_TextAutonumberBullet
        {
            AttributeList attribs( rXAttributes );
            try {
                sal_Int32 nType = rXAttributes->getValueToken( XML_type );
                sal_Int32 nStartAt = attribs.getInteger( XML_startAt, 1 );
                if( nStartAt > 32767 )
                {
                    nStartAt = 32767;
                }
                else if( nStartAt < 1 )
                {
                    nStartAt = 1;
                }
                mrBulletList.setStartAt( nStartAt );
                mrBulletList.setType( nType );
            }
            catch(SAXException& /* e */ )
            {
                OSL_TRACE("OOX: SAXException in XML_buAutoNum");
            }
            break;
        }
        case NMSP_DRAWINGML|XML_buChar:         // CT_TextCharBullet
            try {
                mrBulletList.setBulletChar( rXAttributes->getValue( XML_char ) );
            }
            catch(SAXException& /* e */)
            {
                OSL_TRACE("OOX: SAXException in XML_buChar");
            }
            break;
        case NMSP_DRAWINGML|XML_buBlip:         // CT_TextBlipBullet
            // TODO
            break;

        case NMSP_DRAWINGML|XML_tabLst:         // CT_TextTabStopList
            xRet.set( new TextTabStopListContext( *this, maTabList ) );
            break;
        case NMSP_DRAWINGML|XML_defRPr:         // CT_TextCharacterProperties
            xRet.set( new TextCharacterPropertiesContext( *this, rXAttributes, *mrTextParagraphProperties.getTextCharacterProperties() ) );
            break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// --------------------------------------------------------------------

} }

