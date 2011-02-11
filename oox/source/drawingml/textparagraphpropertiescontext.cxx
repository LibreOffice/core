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

#include "oox/drawingml/textparagraphpropertiescontext.hxx"

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/drawingml/textcharacterpropertiescontext.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/helper/attributelist.hxx"
#include "textspacingcontext.hxx"
#include "texttabstoplistcontext.hxx"

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
        rPropertyMap[ PROP_ParaAdjust ] <<= GetParaAdjust( nAlign );
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
        rPropertyMap[ PROP_ParaIsHyphenation ] <<= bLatinLineBrk;
    }
    // TODO see what to do with Asian hyphenation

    // ST_TextFontAlignType
    // TODO
//  sal_Int32 nFontAlign = xAttribs->getOptionalValueToken( XML_fontAlgn, XML_base );

    if ( xAttribs->hasAttribute( XML_hangingPunct ) )
    {
        bool bHangingPunct = attribs.getBool( XML_hangingPunct, false );
        rPropertyMap[ PROP_ParaIsHangingPunctuation ] <<= bHangingPunct;
    }

  // ST_Coordinate
    if ( xAttribs->hasAttribute( XML_indent ) )
    {
        sValue = xAttribs->getOptionalValue( XML_indent );
        mrTextParagraphProperties.getFirstLineIndentation() = boost::optional< sal_Int32 >( sValue.getLength() == 0 ? 0 : GetCoordinate( sValue ) );
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
        mrTextParagraphProperties.getParaLeftMargin() = boost::optional< sal_Int32 >( sValue.getLength() == 0 ? 0 : GetCoordinate( sValue ) );
    }

    // ParaRightMargin
    if ( xAttribs->hasAttribute( XML_marR ) )
    {
        sValue = xAttribs->getOptionalValue( XML_marR );
        sal_Int32 nMarR  = ( sValue.getLength() == 0 ? 0 : GetCoordinate( sValue ) );
        rPropertyMap[ PROP_ParaRightMargin ] <<= nMarR;
    }

    if ( xAttribs->hasAttribute( XML_rtl ) )
    {
        bool bRtl = attribs.getBool( XML_rtl, false );
        rPropertyMap[ PROP_TextWritingMode ] <<= ( bRtl ? WritingMode_RL_TB : WritingMode_LR_TB );
    }
}



TextParagraphPropertiesContext::~TextParagraphPropertiesContext()
{
    PropertyMap& rPropertyMap( mrTextParagraphProperties.getTextParagraphPropertyMap() );
    if ( maLineSpacing.bHasValue )
        rPropertyMap[ PROP_ParaLineSpacing ] <<= maLineSpacing.toLineSpacing();

    ::std::list< TabStop >::size_type nTabCount = maTabList.size();
    if( nTabCount != 0 )
    {
        Sequence< TabStop > aSeq( nTabCount );
        TabStop * aArray = aSeq.getArray();
        OSL_ENSURE( aArray != NULL, "sequence array is NULL" );
        ::std::copy( maTabList.begin(), maTabList.end(), aArray );
        rPropertyMap[ PROP_ParaTabStops ] <<= aSeq;
    }

    if ( mxBlipProps.get() && mxBlipProps->mxGraphic.is() )
        mrBulletList.setGraphic( mxBlipProps->mxGraphic );

    if( mrBulletList.is() )
        rPropertyMap[ PROP_IsNumbering ] <<= sal_True;
    sal_Int16 nLevel = mrTextParagraphProperties.getLevel();
    rPropertyMap[ PROP_NumberingLevel ] <<= nLevel;
    rPropertyMap[ PROP_NumberingIsNumber ] <<= sal_True;
}

// --------------------------------------------------------------------

void TextParagraphPropertiesContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}



// --------------------------------------------------------------------

Reference< XFastContextHandler > TextParagraphPropertiesContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& rXAttributes ) throw (SAXException, RuntimeException)
{
    AttributeList aAttribs( rXAttributes );
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case A_TOKEN( lnSpc ):          // CT_TextSpacing
            xRet.set( new TextSpacingContext( *this, maLineSpacing ) );
            break;
        case A_TOKEN( spcBef ):         // CT_TextSpacing
            xRet.set( new TextSpacingContext( *this, mrSpaceBefore ) );
            break;
        case A_TOKEN( spcAft ):         // CT_TextSpacing
            xRet.set( new TextSpacingContext( *this, mrSpaceAfter ) );
            break;

        // EG_TextBulletColor
        case A_TOKEN( buClrTx ):        // CT_TextBulletColorFollowText ???
            mrBulletList.mbBulletColorFollowText <<= sal_True;
            break;
        case A_TOKEN( buClr ):          // CT_Color
            xRet.set( new ColorContext( *this, *mrBulletList.maBulletColorPtr ) );
            break;

        // EG_TextBulletSize
        case A_TOKEN( buSzTx ):         // CT_TextBulletSizeFollowText
            mrBulletList.setBulletSize(100);
            break;
        case A_TOKEN( buSzPct ):        // CT_TextBulletSizePercent
            mrBulletList.setBulletSize( static_cast<sal_Int16>( GetPercent( rXAttributes->getOptionalValue( XML_val ) ) / 1000 ) );
            break;
        case A_TOKEN( buSzPts ):        // CT_TextBulletSizePoint
            mrBulletList.setBulletSize(0);
            mrBulletList.setFontSize( static_cast<sal_Int16>(GetTextSize( rXAttributes->getOptionalValue( XML_val ) ) ) );
            break;

        // EG_TextBulletTypeface
        case A_TOKEN( buFontTx ):       // CT_TextBulletTypefaceFollowText
            mrBulletList.mbBulletFontFollowText <<= sal_True;
            break;
        case A_TOKEN( buFont ):         // CT_TextFont
            mrBulletList.maBulletFont.setAttributes( aAttribs );
            break;

        // EG_TextBullet
        case A_TOKEN( buNone ):         // CT_TextNoBullet
            mrBulletList.setNone();
            break;
        case A_TOKEN( buAutoNum ):      // CT_TextAutonumberBullet
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
        case A_TOKEN( buChar ):         // CT_TextCharBullet
            try {
                mrBulletList.setBulletChar( rXAttributes->getValue( XML_char ) );
            }
            catch(SAXException& /* e */)
            {
                OSL_TRACE("OOX: SAXException in XML_buChar");
            }
            break;
        case A_TOKEN( buBlip ):         // CT_TextBlipBullet
            {
                mxBlipProps.reset( new BlipFillProperties );
                xRet.set( new BlipFillContext( *this, rXAttributes, *mxBlipProps ) );
            }
            break;

        case A_TOKEN( tabLst ):         // CT_TextTabStopList
            xRet.set( new TextTabStopListContext( *this, maTabList ) );
            break;
        case A_TOKEN( defRPr ):         // CT_TextCharacterProperties
            xRet.set( new TextCharacterPropertiesContext( *this, rXAttributes, mrTextParagraphProperties.getTextCharacterProperties() ) );
            break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// --------------------------------------------------------------------

} }

