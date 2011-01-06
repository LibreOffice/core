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
#include "precompiled_xmloff.hxx"

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
// --> OD 2008-01-16 #newlistlevelattrs#
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/LabelFollow.hpp>
// <--
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/ustrbuf.hxx>

#include <tools/debug.hxx>

#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "fonthdl.hxx"
#include "xmloff/XMLTextListAutoStylePool.hxx"
#include <xmloff/xmlnume.hxx>
#include <xmloff/xmlexp.hxx>
#include <tools/fontenum.hxx>


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE[] = "SymbolTextDistance";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_PARENT_NUMBERING[] = "ParentNumbering";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_CHAR_STYLE_NAME[] = "CharStyleName";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_CHAR[] = "BulletChar";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_RELSIZE[] = "BulletRelSize";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_COLOR[] = "BulletColor";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHIC_BITMAP[] = "GraphicBitmap";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHIC_SIZE[] = "GraphicSize";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_VERT_ORIENT[] = "VertOrient";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_NUMBERINGTYPE[] = "NumberingType";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_HEADING_STYLE_NAME[] = "HeadingStyleName";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_PREFIX[] = "Prefix";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_SUFFIX[] = "Suffix";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_ADJUST[] = "Adjust";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_LEFT_MARGIN[] = "LeftMargin";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET[] = "FirstLineOffset";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_FONT[] = "BulletFont";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHICURL[] = "GraphicURL";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_START_WITH[] = "StartWith";
// --> OD 2008-01-15 #newlistlevelattrs#
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_POSITION_AND_SPACE_MODE[] = "PositionAndSpaceMode";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_LABEL_FOLLOWED_BY[] = "LabelFollowedBy";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_LISTTAB_STOP_POSITION[] = "ListtabStopPosition";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_FIRST_LINE_INDENT[] = "FirstLineIndent";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_INDENT_AT[] = "IndentAt";
// <--

void SvxXMLNumRuleExport::exportLevelStyles( const uno::Reference< ::com::sun::star::container::XIndexReplace > & xNumRule,
                                             sal_Bool bOutline )
{
    sal_Int32 nCount = xNumRule ->getCount();
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        uno::Any aEntry( xNumRule->getByIndex( i ) );
        uno::Sequence<beans::PropertyValue> aSeq;
        if( aEntry >>= aSeq )
        {
            exportLevelStyle( i, aSeq, bOutline );
        }
    }
}

void SvxXMLNumRuleExport::exportLevelStyle( sal_Int32 nLevel,
                                    const uno::Sequence<beans::PropertyValue>& rProps,
                                    sal_Bool bOutline )
{
    sal_Int16 eType = NumberingType::CHAR_SPECIAL;

    sal_Int16 eAdjust = HoriOrientation::LEFT;
    OUString sPrefix, sSuffix;
    OUString sTextStyleName;
    sal_Bool bHasColor = sal_False;
    sal_Int32 nColor = 0;
    sal_Int32 nSpaceBefore = 0, nMinLabelWidth = 0, nMinLabelDist = 0;

    sal_Int16 nStartValue = 1, nDisplayLevels = 1, nBullRelSize = 0;

    sal_Unicode cBullet = 0xf095;
    OUString sBulletFontName, sBulletFontStyleName ;
    sal_Int16 eBulletFontFamily = FAMILY_DONTKNOW;
    sal_Int16 eBulletFontPitch = PITCH_DONTKNOW;
    rtl_TextEncoding eBulletFontEncoding = RTL_TEXTENCODING_DONTKNOW;

    OUString sImageURL;
    uno::Reference< ::com::sun::star::awt::XBitmap >  xBitmap;
    sal_Int32 nImageWidth = 0, nImageHeight = 0;
    sal_Int16 eImageVertOrient = VertOrientation::LINE_CENTER;

    // --> OD 2008-01-15 #newlistlevelattrs#
    sal_Int16 ePosAndSpaceMode = PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION;
    sal_Int16 eLabelFollowedBy = LabelFollow::LISTTAB;
    sal_Int32 nListtabStopPosition( 0 );
    sal_Int32 nFirstLineIndent( 0 );
    sal_Int32 nIndentAt( 0 );
    // <--

    const sal_Int32 nCount = rProps.getLength();
    const beans::PropertyValue* pPropArray = rProps.getConstArray();
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        const beans::PropertyValue& rProp = pPropArray[i];

        if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_NUMBERINGTYPE, sizeof(XML_UNO_NAME_NRULE_NUMBERINGTYPE)-1 ) )
        {
            rProp.Value >>= eType;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_PREFIX, sizeof(XML_UNO_NAME_NRULE_PREFIX)-1 ) )
        {
            rProp.Value >>= sPrefix;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_SUFFIX, sizeof(XML_UNO_NAME_NRULE_SUFFIX)-1 ) )
        {
            rProp.Value >>= sSuffix;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_BULLET_CHAR, sizeof(XML_UNO_NAME_NRULE_BULLET_CHAR)-1 ) )
        {
            OUString sValue;
            rProp.Value >>= sValue;
            if( sValue.getLength() > 0 )
            {
                cBullet = (sal_Unicode)sValue[0];
            }
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_BULLET_RELSIZE, sizeof(XML_UNO_NAME_NRULE_BULLET_RELSIZE)-1 ) )
        {
            rProp.Value >>= nBullRelSize;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_ADJUST, sizeof(XML_UNO_NAME_NRULE_ADJUST)-1 ) )
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            eAdjust = nValue;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_BULLET_FONT, sizeof(XML_UNO_NAME_NRULE_BULLET_FONT)-1 ) )
        {
            awt::FontDescriptor rFDesc;
            if( rProp.Value >>= rFDesc )
            {
                sBulletFontName = rFDesc.Name;
                sBulletFontStyleName = rFDesc.StyleName;
                eBulletFontFamily = (sal_Int16)rFDesc.Family;
                eBulletFontPitch = (sal_Int16)rFDesc.Pitch;
                eBulletFontEncoding = (rtl_TextEncoding)rFDesc.CharSet;
            }
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_GRAPHICURL, sizeof(XML_UNO_NAME_NRULE_GRAPHICURL)-1 ) )
        {
            rProp.Value >>= sImageURL;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_GRAPHIC_BITMAP, sizeof(XML_UNO_NAME_NRULE_GRAPHIC_BITMAP)-1 ) )
        {
            rProp.Value >>= xBitmap;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_BULLET_COLOR, sizeof(XML_UNO_NAME_NRULE_BULLET_COLOR)-1 ) )
        {
            rProp.Value >>= nColor;
            bHasColor = sal_True;
        }
        else  if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_START_WITH, sizeof(XML_UNO_NAME_NRULE_START_WITH)-1 ) )
        {
            rProp.Value >>= nStartValue;
        }
        else  if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_LEFT_MARGIN, sizeof(XML_UNO_NAME_NRULE_LEFT_MARGIN)-1 ) )
        {
            rProp.Value >>= nSpaceBefore;
        }
        else  if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET, sizeof(XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET)-1 ) )
        {
            rProp.Value >>= nMinLabelWidth;
        }
        else  if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE, sizeof(XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE)-1 ) )
        {
            rProp.Value >>= nMinLabelDist;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_PARENT_NUMBERING, sizeof(XML_UNO_NAME_NRULE_PARENT_NUMBERING)-1 ) )
        {
            rProp.Value >>= nDisplayLevels;
            if( nDisplayLevels > nLevel+1 )
                nDisplayLevels = static_cast<sal_Int16>( nLevel )+1;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_CHAR_STYLE_NAME, sizeof(XML_UNO_NAME_NRULE_CHAR_STYLE_NAME)-1 ) )
        {
            rProp.Value >>= sTextStyleName;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_GRAPHIC_SIZE, sizeof(XML_UNO_NAME_NRULE_GRAPHIC_SIZE)-1 ) )
        {
            awt::Size aSize;
            if( rProp.Value >>= aSize )
            {
                nImageWidth = aSize.Width;
                nImageHeight = aSize.Height;
            }
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_VERT_ORIENT, sizeof(XML_UNO_NAME_NRULE_VERT_ORIENT)-1 ) )
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            eImageVertOrient = nValue;
        }
        // --> OD 2008-01-16 #newlistlevelattrs#
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_POSITION_AND_SPACE_MODE,
                                          sizeof(XML_UNO_NAME_NRULE_POSITION_AND_SPACE_MODE)-1 ) )
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            ePosAndSpaceMode = nValue;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_LABEL_FOLLOWED_BY,
                                          sizeof(XML_UNO_NAME_NRULE_LABEL_FOLLOWED_BY)-1 ) )
        {
            sal_Int16 nValue = 0;
            rProp.Value >>= nValue;
            eLabelFollowedBy = nValue;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_LISTTAB_STOP_POSITION,
                                          sizeof(XML_UNO_NAME_NRULE_LISTTAB_STOP_POSITION)-1 ) )
        {
            rProp.Value >>= nListtabStopPosition;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_FIRST_LINE_INDENT,
                                          sizeof(XML_UNO_NAME_NRULE_FIRST_LINE_INDENT)-1 ) )
        {
            rProp.Value >>= nFirstLineIndent;
        }
        else if( rProp.Name.equalsAsciiL( XML_UNO_NAME_NRULE_INDENT_AT,
                                          sizeof(XML_UNO_NAME_NRULE_INDENT_AT)-1 ) )
        {
            rProp.Value >>= nIndentAt;
        }
        // <--
    }

    if( bOutline && (NumberingType::CHAR_SPECIAL == eType ||
                     NumberingType::BITMAP == eType) )
    {
        DBG_ASSERT( !bOutline,
           "SvxXMLNumRuleExport::exportLevelStyle: invalid style for outline" );
        return;
    }

    GetExport().CheckAttrList();

    // text:level
    OUStringBuffer sTmp;
    sTmp.append( nLevel + 1 );
    GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_LEVEL, sTmp.makeStringAndClear() );
    // #i110694#: no style-name on list-level-style-image
    if ((sTextStyleName.getLength() > 0) && (NumberingType::BITMAP != eType))
    {
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_STYLE_NAME,
                                GetExport().EncodeStyleName( sTextStyleName ) );
    }
    if( sPrefix.getLength() > 0 )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NUM_PREFIX, sPrefix );
    if( sSuffix.getLength() > 0 )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NUM_SUFFIX, sSuffix );

    enum XMLTokenEnum eElem = XML_LIST_LEVEL_STYLE_NUMBER;
    if( NumberingType::CHAR_SPECIAL == eType )
    {
        // <text:list-level-style-bullet>
        eElem = XML_LIST_LEVEL_STYLE_BULLET;

        if( cBullet )
        {
            if( cBullet < ' ' )
            {
                cBullet = 0xF000 + 149;
            }
            // text:bullet-char="..."
            sTmp.append( cBullet );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_BULLET_CHAR,
                          sTmp.makeStringAndClear() );
        }

    }
    else if( NumberingType::BITMAP == eType )
    {
        // <text:list-level-style-image>

        eElem = XML_LIST_LEVEL_STYLE_IMAGE;


        if( sImageURL.getLength() )
        {
            OUString sURL( GetExport().AddEmbeddedGraphicObject( sImageURL ) );
            if( sURL.getLength() )
            {
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, sURL );

                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                GetExport().AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
            }
        }
        else
        {
            DBG_ASSERT( !xBitmap.is(),
                        "embedded images are not supported by now" );
        }
    }
    else
    {
        // <text:list-level-style-number> or <text:outline-level-style>
        if( bOutline )
            eElem = XML_OUTLINE_LEVEL_STYLE;
        else
            eElem = XML_LIST_LEVEL_STYLE_NUMBER;

        GetExport().GetMM100UnitConverter().convertNumFormat( sTmp, eType );
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NUM_FORMAT,
                                       sTmp.makeStringAndClear() );
        GetExport().GetMM100UnitConverter().convertNumLetterSync( sTmp, eType );
        if( sTmp.getLength() )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                      XML_NUM_LETTER_SYNC,
                                           sTmp.makeStringAndClear() );

        if( nStartValue != 1 )
        {
            sTmp.append( (sal_Int32)nStartValue );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_START_VALUE,
                          sTmp.makeStringAndClear() );
        }
        if( nDisplayLevels > 1 && NumberingType::NUMBER_NONE != eType )
        {
            sTmp.append( (sal_Int32)nDisplayLevels );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_DISPLAY_LEVELS,
                          sTmp.makeStringAndClear() );
        }
    }

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, eElem,
                                  sal_True, sal_True );

        // --> OD 2008-01-16 #newlistlevelattrs#
        OUStringBuffer sBuffer;
        if ( ePosAndSpaceMode == PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION )
        {
            // --> OD 2008-06-05 #i89178#
            // optimization of XML stream size:
            // suppress export of property list-level-position-and-space-mode,
            // if its value is "label-width-and-position" - its the default value
//            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
//                                      XML_LIST_LEVEL_POSITION_AND_SPACE_MODE,
//                                      XML_LABEL_WIDTH_AND_POSITION );
            // <--

            nSpaceBefore += nMinLabelWidth;
            nMinLabelWidth = -nMinLabelWidth;
            if( nSpaceBefore != 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nSpaceBefore );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_SPACE_BEFORE,
                              sBuffer.makeStringAndClear() );
            }
            if( nMinLabelWidth != 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nMinLabelWidth );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_MIN_LABEL_WIDTH,
                              sBuffer.makeStringAndClear() );
            }
            if( nMinLabelDist > 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nMinLabelDist );
                GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_MIN_LABEL_DISTANCE,
                              sBuffer.makeStringAndClear() );
            }
        }
        // --> OD 2008-06-06 #i89178#
        // check, if properties for position-and-space-mode LABEL_ALIGNMENT
        // are allowed to be exported.
        else if ( ePosAndSpaceMode == PositionAndSpaceMode::LABEL_ALIGNMENT &&
                  mbExportPositionAndSpaceModeLabelAlignment )
        // <--
        {
            GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                      XML_LIST_LEVEL_POSITION_AND_SPACE_MODE,
                                      XML_LABEL_ALIGNMENT );
        }
        // <--
        if( HoriOrientation::LEFT != eAdjust )
        {
            enum XMLTokenEnum eValue = XML_TOKEN_INVALID;
            switch( eAdjust )
            {
            case HoriOrientation::RIGHT:    eValue = XML_END;   break;
            case HoriOrientation::CENTER:   eValue = XML_CENTER;    break;
            }
            if( eValue != XML_TOKEN_INVALID )
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_TEXT_ALIGN, eValue );
        }

        if( NumberingType::BITMAP == eType )
        {
            enum XMLTokenEnum eValue = XML_TOKEN_INVALID;
            switch( eImageVertOrient )
            {
            case VertOrientation::BOTTOM:   // yes, its OK: BOTTOM means that the baseline
                                    // hits the frame at its topmost position
            case VertOrientation::LINE_TOP:
            case VertOrientation::CHAR_TOP:
                eValue = XML_TOP;
                break;
            case VertOrientation::CENTER:
            case VertOrientation::LINE_CENTER:
            case VertOrientation::CHAR_CENTER:
                eValue = XML_MIDDLE;
                break;
            case VertOrientation::TOP:      // yes, its OK: TOP means that the baseline
                                    // hits the frame at its bottommost position
            case VertOrientation::LINE_BOTTOM:
            case VertOrientation::CHAR_BOTTOM:
                eValue = XML_BOTTOM;
                break;
            }
            if( eValue != XML_TOKEN_INVALID )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_VERTICAL_POS, eValue );

            eValue = XML_TOKEN_INVALID;
            switch( eImageVertOrient )
            {
            case VertOrientation::TOP:
            case VertOrientation::CENTER:
            case VertOrientation::BOTTOM:
                eValue = XML_BASELINE;
                break;
            case VertOrientation::LINE_TOP:
            case VertOrientation::LINE_CENTER:
            case VertOrientation::LINE_BOTTOM:
                eValue = XML_LINE;
                break;
            case VertOrientation::CHAR_TOP:
            case VertOrientation::CHAR_CENTER:
            case VertOrientation::CHAR_BOTTOM:
                eValue = XML_CHAR;
                break;
            }
            if( eValue != XML_TOKEN_INVALID )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_VERTICAL_REL, eValue );

            if( nImageWidth > 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nImageWidth );
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_WIDTH,
                              sBuffer.makeStringAndClear() );
            }

            if( nImageHeight > 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nImageHeight );
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_HEIGHT,
                              sBuffer.makeStringAndClear() );
            }
        }

        // --> OD 2008-01-16 #newlistlevelattrs#
//        if( GetExport().GetAttrList().getLength() > 0 )
        {
            SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_STYLE,
                                      XML_LIST_LEVEL_PROPERTIES, sal_True, sal_True );

            // --> OD 2008-06-06 #i89178#
            // check, if properties for position-and-space-mode LABEL_ALIGNMENT
            // are allowed to be exported.
            if ( ePosAndSpaceMode == PositionAndSpaceMode::LABEL_ALIGNMENT &&
                 mbExportPositionAndSpaceModeLabelAlignment )
            // <--
            {
                enum XMLTokenEnum eValue = XML_LISTTAB;
                if ( eLabelFollowedBy == LabelFollow::SPACE )
                {
                    eValue = XML_SPACE;
                }
                else if ( eLabelFollowedBy == LabelFollow::NOTHING )
                {
                    eValue = XML_NOTHING;
                }
                GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                          XML_LABEL_FOLLOWED_BY, eValue );

                if ( eLabelFollowedBy == LabelFollow::LISTTAB &&
                     nListtabStopPosition > 0 )
                {
                    GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nListtabStopPosition );
                    GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                              XML_LIST_TAB_STOP_POSITION,
                                              sBuffer.makeStringAndClear() );
                }

                if ( nFirstLineIndent != 0 )
                {
                    GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nFirstLineIndent );
                    GetExport().AddAttribute( XML_NAMESPACE_FO,
                                              XML_TEXT_INDENT,
                                              sBuffer.makeStringAndClear() );
                }

                if ( nIndentAt != 0 )
                {
                    GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nIndentAt );
                    GetExport().AddAttribute( XML_NAMESPACE_FO,
                                              XML_MARGIN_LEFT,
                                              sBuffer.makeStringAndClear() );
                }

                SvXMLElementExport aLabelAlignmentElement( GetExport(), XML_NAMESPACE_STYLE,
                                             XML_LIST_LEVEL_LABEL_ALIGNMENT,
                                             sal_True, sal_True );
            }
        }
        // <--

        if( NumberingType::CHAR_SPECIAL == eType )
        {
            if( sBulletFontName.getLength() )
            {
                OUString sStyleName =
                    GetExport().GetFontAutoStylePool()->Find(
                        sBulletFontName, sBulletFontStyleName,
                        eBulletFontFamily, eBulletFontPitch,
                        eBulletFontEncoding );

                if( sStyleName.getLength() )
                {
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_NAME,
                                                  sStyleName );
                }
                else
                {
                    Any aAny;
                    OUString sTemp;

                    const SvXMLUnitConverter& rUnitConv =
                        GetExport().GetMM100UnitConverter();
                    XMLFontFamilyNamePropHdl aFamilyNameHdl;
                    aAny <<= sBulletFontName;
                    if( aFamilyNameHdl.exportXML( sTemp, aAny, rUnitConv ) )
                        GetExport().AddAttribute( XML_NAMESPACE_FO,
                                                  XML_FONT_FAMILY, sTemp );

                    if( sBulletFontStyleName.getLength() )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_STYLE_NAME,
                                                  sBulletFontStyleName );

                    XMLFontFamilyPropHdl aFamilyHdl;
                    aAny <<= (sal_Int16)eBulletFontFamily;
                    if( aFamilyHdl.exportXML( sTemp, aAny, rUnitConv  ) )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_FAMILY_GENERIC,
                                                  sTemp );

                    XMLFontPitchPropHdl aPitchHdl;
                    aAny <<= (sal_Int16)eBulletFontPitch;
                    if( aPitchHdl.exportXML( sTemp, aAny, rUnitConv  ) )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_PITCH, sTemp );

                    XMLFontEncodingPropHdl aEncHdl;
                    aAny <<= (sal_Int16)eBulletFontEncoding;
                    if( aEncHdl.exportXML( sTemp, aAny, rUnitConv  ) )
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_FONT_CHARSET, sTemp );
                }
            }
        }
        if( NumberingType::BITMAP != eType )
        {
            // fo:color = "#..."
            if( bHasColor )
            {
                const Color aColor( nColor );
                if( aColor.GetColor() == 0xffffffff )
                {
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_USE_WINDOW_FONT_COLOR, XML_TRUE );
                }
                else
                {
                    SvXMLUnitConverter::convertColor( sBuffer, aColor );
                    GetExport().AddAttribute( XML_NAMESPACE_FO, XML_COLOR,
                                  sBuffer.makeStringAndClear() );
                }
            }
            // fo:height="...%"
            if( nBullRelSize )
            {
                GetExport().GetMM100UnitConverter().convertPercent( sTmp, nBullRelSize );
                GetExport().AddAttribute( XML_NAMESPACE_FO, XML_FONT_SIZE,
                              sTmp.makeStringAndClear() );
            }
        }
        if( GetExport().GetAttrList().getLength() > 0 )
        {
            SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_STYLE,
                                      XML_TEXT_PROPERTIES, sal_True, sal_True );
        }
        if( NumberingType::BITMAP == eType && sImageURL.getLength() )
        {
            // optional office:binary-data
            GetExport().AddEmbeddedGraphicObjectAsBase64( sImageURL );
        }
    }
}


uno::Reference< ::com::sun::star::container::XIndexReplace >  SvxXMLNumRuleExport::GetUNONumRule() const
{
    return uno::Reference< ::com::sun::star::container::XIndexReplace > ();
}

void SvxXMLNumRuleExport::AddListStyleAttributes()
{
}


SvxXMLNumRuleExport::SvxXMLNumRuleExport( SvXMLExport& rExp ) :
    rExport( rExp ),
    sNumberingRules( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ),
    sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
    sIsContinuousNumbering( RTL_CONSTASCII_USTRINGPARAM( "IsContinuousNumbering" ) ),
    // --> OD 2008-06-06 #i89178#
    mbExportPositionAndSpaceModeLabelAlignment( true )
    // <--
{
    switch ( GetExport().getDefaultVersion() )
    {
        case SvtSaveOptions::ODFVER_010:
        case SvtSaveOptions::ODFVER_011:
        {
            mbExportPositionAndSpaceModeLabelAlignment = false;
        }
        break;
        default: // ODFVER_UNKNOWN or ODFVER_012
        {
            mbExportPositionAndSpaceModeLabelAlignment = true;
        }
    }
}

SvxXMLNumRuleExport::~SvxXMLNumRuleExport()
{
}

// --> OD 2008-06-17 #i90780#
// refactoring: removing unused methods
//void SvxXMLNumRuleExport::Export( const OUString& rName,
//                                sal_Bool bContNumbering )
//{
//  GetExport().CheckAttrList();

//  // style:name="..."
//  if( rName.getLength() )
//  {
//      sal_Bool bEncoded = sal_False;
//      GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
//                        GetExport().EncodeStyleName( rName, &bEncoded ) );
//      if( bEncoded )
//          GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
//                                   rName);
//  }

//  // text:consecutive-numbering="..."
//  if( bContNumbering )
//      GetExport().AddAttribute( XML_NAMESPACE_TEXT, XML_CONSECUTIVE_NUMBERING,
//                         XML_TRUE );

//  // other application specific attributes
//  AddListStyleAttributes();

//  OUString sElem = GetExport().GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TEXT,
//                                 GetXMLToken(XML_LIST_STYLE) );
//  GetExport().IgnorableWhitespace();
//  GetExport().StartElement( XML_NAMESPACE_TEXT, XML_LIST_STYLE, sal_False );

//  uno::Reference< ::com::sun::star::container::XIndexReplace >  xNumRule = GetUNONumRule();
//  if( xNumRule.is() )
//      exportLevelStyles( xNumRule );

//  GetExport().EndElement( XML_NAMESPACE_TEXT, XML_LIST_STYLE, sal_True );
//}

//void SvxXMLNumRuleExport::ExportOutline()
//{
//  GetExport().IgnorableWhitespace( );
//  GetExport().StartElement( XML_NAMESPACE_TEXT, XML_OUTLINE_STYLE, sal_False );

//  uno::Reference< ::com::sun::star::container::XIndexReplace >  xNumRule = GetUNONumRule();
//  if( xNumRule.is() )
//      exportLevelStyles( xNumRule, sal_True );

//  GetExport().EndElement( XML_NAMESPACE_TEXT, XML_OUTLINE_STYLE, sal_True );
//}
// <--

void SvxXMLNumRuleExport::exportNumberingRule(
        const OUString& rName,
        const Reference< XIndexReplace >& rNumRule )
{
    Reference< XPropertySet > xPropSet( rNumRule, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo;
    if( xPropSet.is() )
           xPropSetInfo = xPropSet->getPropertySetInfo();

    GetExport().CheckAttrList();

    // style:name="..."
    if( rName.getLength() )
    {
        sal_Bool bEncoded = sal_False;
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                          GetExport().EncodeStyleName( rName, &bEncoded ) );
        if( bEncoded )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
                                 rName);
    }

    // text:consecutive-numbering="..."
    sal_Bool bContNumbering = sal_False;
    if( xPropSetInfo.is() &&
        xPropSetInfo->hasPropertyByName( sIsContinuousNumbering ) )
    {
        Any aAny( xPropSet->getPropertyValue( sIsContinuousNumbering ) );
        bContNumbering = *(sal_Bool *)aAny.getValue();
    }
    if( bContNumbering )
        GetExport().AddAttribute( XML_NAMESPACE_TEXT,
                                  XML_CONSECUTIVE_NUMBERING, XML_TRUE );

    // other application specific attributes
    AddListStyleAttributes();

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, XML_LIST_STYLE ,
                                  sal_True, sal_True );
        exportLevelStyles( rNumRule );
    }
}

sal_Bool SvxXMLNumRuleExport::exportStyle( const Reference< XStyle >& rStyle )
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    Any aAny;

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        if( !*(sal_Bool *)aAny.getValue() )
            return sal_False;
    }

    aAny = xPropSet->getPropertyValue( sNumberingRules );
    Reference<XIndexReplace> xNumRule;
    aAny >>= xNumRule;

    OUString sName = rStyle->getName();

    exportNumberingRule( sName, xNumRule );

    return sal_True;
}

void SvxXMLNumRuleExport::exportOutline()
{
    Reference< XChapterNumberingSupplier > xCNSupplier( GetExport().GetModel(),
                                                        UNO_QUERY );
    DBG_ASSERT( xCNSupplier.is(), "no chapter numbering supplier" );

    if( xCNSupplier.is() )
    {
        Reference< XIndexReplace > xNumRule( xCNSupplier->getChapterNumberingRules() );
        DBG_ASSERT( xNumRule.is(), "no chapter numbering rules" );

        if( xNumRule.is() )
        {
            // --> OD 2008-06-17 #i90780#
            // Outline style has property style:name since ODF 1.2
            // Thus, export this property and adjust fix for issue #i69627#
            OUString sOutlineStyleName;
            {
                Reference<XPropertySet> xNumRulePropSet(
                    xCNSupplier->getChapterNumberingRules(), UNO_QUERY );
                if (xNumRulePropSet.is())
                {
                    const OUString sName( RTL_CONSTASCII_USTRINGPARAM("Name") );
                    xNumRulePropSet->getPropertyValue( sName ) >>= sOutlineStyleName;
                }
            }
            const SvtSaveOptions::ODFDefaultVersion nODFVersion =
                                                GetExport().getDefaultVersion();
            if ( ( nODFVersion == SvtSaveOptions::ODFVER_010 ||
                   nODFVersion == SvtSaveOptions::ODFVER_011 ) &&
                 GetExport().writeOutlineStyleAsNormalListStyle() )
            {
                exportNumberingRule( sOutlineStyleName, xNumRule );
            }
            else
            {
                if ( nODFVersion != SvtSaveOptions::ODFVER_010 &&
                     nODFVersion != SvtSaveOptions::ODFVER_011 )
                {
                    // style:name="..."
                    GetExport().CheckAttrList();
                    if ( sOutlineStyleName.getLength() > 0 )
                     {
                        sal_Bool bEncoded = sal_False;
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                                        GetExport().EncodeStyleName( sOutlineStyleName,
                                                                     &bEncoded ) );
                        if( bEncoded )
                            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                      XML_DISPLAY_NAME,
                                                      sOutlineStyleName );
                    }
                }
                SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                          XML_OUTLINE_STYLE, sal_True, sal_True );
                exportLevelStyles( xNumRule, sal_True );
            }
        }
    }
}

void SvxXMLNumRuleExport::exportStyles( sal_Bool bUsed,
                                         XMLTextListAutoStylePool *pPool,
                                         sal_Bool bExportChapterNumbering )
{
    if( bExportChapterNumbering )
        exportOutline();

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(), UNO_QUERY );
    DBG_ASSERT( xFamiliesSupp.is(), "No XStyleFamiliesSupplier from XModel for export!" );
    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        DBG_ASSERT( xFamiliesSupp.is(), "getStyleFamilies() from XModel failed for export!" );

        if( xFamilies.is() )
        {
            const OUString aNumberStyleName( RTL_CONSTASCII_USTRINGPARAM( "NumberingStyles" ));

            Reference< XIndexAccess > xStyles;
            if( xFamilies->hasByName( aNumberStyleName ) )
            {
                xFamilies->getByName( aNumberStyleName ) >>= xStyles;

                DBG_ASSERT( xStyles.is(), "Style not found for export!" );

                if( xStyles.is() )
                {
                    const sal_Int32 nStyles = xStyles->getCount();

                    for( sal_Int32 i=0; i < nStyles; i++ )
                    {
                        Reference< XStyle > xStyle;
                        xStyles->getByIndex( i ) >>= xStyle;

                        if( !bUsed || xStyle->isInUse() )
                        {
                            exportStyle( xStyle );
                            if( pPool )
                                pPool->RegisterName( xStyle->getName() );
                        }
                    }
                }
            }
        }
    }
}

sal_Bool SvxXMLNumRuleExport::GetOutlineStyles( XMLStringVector& rStyleNames,
   const ::com::sun::star::uno::Reference<
                   ::com::sun::star::frame::XModel > & rModel   )
{
    Reference< XChapterNumberingSupplier > xCNSupplier( rModel,
                                                        UNO_QUERY );
    sal_Int32 nLevels = 0;
    Reference< XIndexReplace > xNumRule;
    if( xCNSupplier.is() )
    {
        xNumRule = xCNSupplier->getChapterNumberingRules();
        if( xNumRule.is() )
            nLevels = xNumRule->getCount();
    }

    rStyleNames.resize( nLevels );
    for( sal_Int32 i=0; i<nLevels; i++ )
    {
        uno::Any aEntry( xNumRule->getByIndex( i ) );
        uno::Sequence<beans::PropertyValue> aSeq;
        if( aEntry >>= aSeq )
        {
            const sal_Int32 nCount = aSeq.getLength();
            const beans::PropertyValue* pPropArray = aSeq.getConstArray();
            for( sal_Int32 j=0; j<nCount; j++ )
            {
                const beans::PropertyValue& rProp = pPropArray[j];

                if( rProp.Name.equalsAsciiL(
                            XML_UNO_NAME_NRULE_HEADING_STYLE_NAME,
                            sizeof(XML_UNO_NAME_NRULE_HEADING_STYLE_NAME)-1 ) )
                {
                    rProp.Value >>= rStyleNames[i];
                    break;
                }
            }
        }
    }

    return nLevels != 0;
}
