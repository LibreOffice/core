/*************************************************************************
 *
 *  $RCSfile: xmlnume.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:05 $
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

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_FONTTYPES_HXX
#include "fonthdl.hxx"
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XCHAPTERNUMBERINGSUPPLIER_HPP_
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#include "XMLTextListAutoStylePool.hxx"
#endif
#ifndef _XMLOFF_XMLNUME_HXX
#include "xmlnume.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE[] = "SymbolTextDistance";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_PARENT_NUMBERING[] = "ParentNumbering";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_CHAR_STYLE_NAME[] = "CharStyleName";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_CHAR[] = "BulletChar";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHIC_BITMAP[] = "GraphicBitmap";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHIC_SIZE[] = "GraphicSize";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_VERT_ORIENT[] = "VertOrient";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_NUMBERINGTYPE[] = "NumberingType";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_PREFIX[] = "Prefix";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_SUFFIX[] = "Suffix";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_ADJUST[] = "Adjust";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_LEFT_MARGIN[] = "LeftMargin";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET[] =
    "FirstLineOffset";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_FONT[] = "BulletFont";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHICURL[] = "GraphicURL";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_START_WITH[] = "StartWith";

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

void SvxXMLNumRuleExport::exportLevelStyle( INT32 nLevel,
                                    const uno::Sequence<beans::PropertyValue>& rProps,
                                    BOOL bOutline )
{
    sal_Int16 eType = NumberingType::CHAR_SPECIAL;

    sal_Int16 eAdjust = HoriOrientation::LEFT;
    OUString sPrefix, sSuffix;
    OUString sTextStyleName;
    sal_Int32 nSpaceBefore = 0, nMinLabelWidth = 0, nMinLabelDist = 0;

    sal_Int16 nStartValue = 1, nDisplayLevels = 1;

    sal_Unicode cBullet = 0xf095;
    OUString sBulletFontName, sBulletFontStyleName ;
    sal_Int16 eBulletFontFamily = FAMILY_DONTKNOW;
    sal_Int16 eBulletFontPitch = PITCH_DONTKNOW;
    rtl_TextEncoding eBulletFontEncoding = RTL_TEXTENCODING_DONTKNOW;

    OUString sImageURL;
    uno::Reference< ::com::sun::star::awt::XBitmap >  xBitmap;
    sal_Int32 nImageWidth = 0, nImageHeight = 0;
    sal_Int16 eImageVertOrient = VertOrientation::LINE_CENTER;

    sal_Int32 nCount = rProps.getLength();
    const beans::PropertyValue* pPropArray = rProps.getConstArray();
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        const beans::PropertyValue& rProp = pPropArray[i];

        if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_NUMBERINGTYPE, sizeof(XML_UNO_NAME_NRULE_NUMBERINGTYPE) ) )
        {
            sal_Int16 nType;
            rProp.Value >>= nType;
            if( nType >= NumberingType::CHARS_UPPER_LETTER &&
                nType <= NumberingType::CHARS_LOWER_LETTER_N )
            {
                eType = nType;
            }
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_PREFIX, sizeof(XML_UNO_NAME_NRULE_PREFIX) ) )
        {
            rProp.Value >>= sPrefix;
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_SUFFIX, sizeof(XML_UNO_NAME_NRULE_SUFFIX) ) )
        {
            rProp.Value >>= sSuffix;
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_BULLET_CHAR, sizeof(XML_UNO_NAME_NRULE_BULLET_CHAR) ) )
        {
            OUString sValue;
            rProp.Value >>= sValue;
            if( sValue.getLength() > 0 )
                cBullet = (sal_Unicode)sValue[0];
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_ADJUST, sizeof(XML_UNO_NAME_NRULE_ADJUST) ) )
        {
            sal_Int16 nValue;
            rProp.Value >>= nValue;
            eAdjust = nValue;
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_BULLET_FONT, sizeof(XML_UNO_NAME_NRULE_BULLET_FONT) ) )
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
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_GRAPHICURL, sizeof(XML_UNO_NAME_NRULE_GRAPHICURL) ) )
        {
            rProp.Value >>= sImageURL;
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_GRAPHIC_BITMAP, sizeof(XML_UNO_NAME_NRULE_GRAPHIC_BITMAP) ) )
        {
            rProp.Value >>= xBitmap;
        }
        else  if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_START_WITH, sizeof(XML_UNO_NAME_NRULE_START_WITH) ) )
        {
            rProp.Value >>= nStartValue;
        }
        else  if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_LEFT_MARGIN, sizeof(XML_UNO_NAME_NRULE_LEFT_MARGIN) ) )
        {
            rProp.Value >>= nSpaceBefore;
        }
        else  if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET, sizeof(XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET) ) )
        {
            rProp.Value >>= nMinLabelWidth;
        }
        else  if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE, sizeof(XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE) ) )
        {
            rProp.Value >>= nMinLabelDist;
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_PARENT_NUMBERING, sizeof(XML_UNO_NAME_NRULE_PARENT_NUMBERING) ) )
        {
            rProp.Value >>= nDisplayLevels;
            if( nDisplayLevels > nLevel+1 )
                nDisplayLevels = nLevel+1;
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_CHAR_STYLE_NAME, sizeof(XML_UNO_NAME_NRULE_CHAR_STYLE_NAME) ) )
        {
            rProp.Value >>= sTextStyleName;
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_GRAPHIC_SIZE, sizeof(XML_UNO_NAME_NRULE_GRAPHIC_SIZE) ) )
        {
            awt::Size aSize;
            if( rProp.Value >>= aSize )
            {
                nImageWidth = aSize.Width;
                nImageHeight = aSize.Height;
            }
        }
        else if( 0 == rProp.Name.compareToAscii( XML_UNO_NAME_NRULE_VERT_ORIENT, sizeof(XML_UNO_NAME_NRULE_VERT_ORIENT) ) )
        {
            sal_Int16 nValue;
            rProp.Value >>= nValue;
            eImageVertOrient = nValue;
        }
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
    sTmp.append( nLevel+1L );
    GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_level, sTmp.makeStringAndClear() );
    if( sTextStyleName.getLength() > 0 )
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_style_name, sTextStyleName );
    if( sPrefix.getLength() > 0 )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_num_prefix, sPrefix );
    if( sSuffix.getLength() > 0 )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_num_suffix, sSuffix );

    const sal_Char *pElem = sXML_list_level_style_number;
    if( NumberingType::CHAR_SPECIAL == eType )
    {
        // <text:list-level-style-bullet>
        pElem = sXML_list_level_style_bullet;

        // text:bullet-char="..."
        sTmp.append( cBullet );
        GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_bullet_char,
                      sTmp.makeStringAndClear() );
    }
    else if( NumberingType::BITMAP == eType )
    {
        // <text:list-level-style-image>

        pElem = sXML_list_level_style_image;


        if( sImageURL.getLength() )
        {
            String sURL( sImageURL );
            INetURLObject::AbsToRel( sURL );
            sImageURL = sURL;
            GetExport().AddAttribute( XML_NAMESPACE_XLINK, sXML_href, sImageURL );

            GetExport().AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type, sXML_simple );
            GetExport().AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_show, sXML_embed );
            GetExport().AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_actuate, sXML_onLoad );
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
            pElem = sXML_outline_level_style;
        else
            pElem = sXML_list_level_style_number;

        const char *pNumFormat = GetNumFormatValue( eType );
        if( pNumFormat )
            GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_num_format,
                                 pNumFormat );
        const char *pNumLetterSync = GetNumLetterSync( eType );
        if( pNumLetterSync )
            GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_num_letter_sync,
                                 pNumLetterSync );

        if( nStartValue > 1 )
        {
            sTmp.append( (sal_Int32)nStartValue );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_start_value,
                          sTmp.makeStringAndClear() );
        }
        if( nDisplayLevels > 1 && NumberingType::NUMBER_NONE != eType )
        {
            sTmp.append( (sal_Int32)nDisplayLevels );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_display_levels,
                          sTmp.makeStringAndClear() );
        }
    }

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, pElem,
                                  sal_True, sal_True );

        nSpaceBefore += nMinLabelWidth;
        nMinLabelWidth = -nMinLabelWidth;
        OUStringBuffer sBuffer;
        if( nSpaceBefore > 0 )
        {
            GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nSpaceBefore );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_space_before,
                          sBuffer.makeStringAndClear() );
        }
        if( nMinLabelWidth != 0 )
        {
            GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nMinLabelWidth );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_min_label_width,
                          sBuffer.makeStringAndClear() );
        }
        if( nMinLabelDist > 0 )
        {
            GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nMinLabelDist );
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_min_label_distance,
                          sBuffer.makeStringAndClear() );
        }
        if( HoriOrientation::LEFT != eAdjust )
        {
            const char *pValue = 0;
            switch( eAdjust )
            {
            case HoriOrientation::RIGHT:    pValue = sXML_end;  break;
            case HoriOrientation::CENTER:   pValue = sXML_center;   break;
            }
            if( pValue )
                GetExport().AddAttributeASCII( XML_NAMESPACE_FO, sXML_text_align, pValue );
        }

        if( NumberingType::CHAR_SPECIAL == eType )
        {
            if( sBulletFontName.getLength() )
            {
                Any aAny;
                OUString sTmp;

                const SvXMLUnitConverter& rUnitConv =
                    GetExport().GetMM100UnitConverter();
                XMLFontFamilyNamePropHdl aFamilyNameHdl;
                aAny <<= sBulletFontName;
                if( aFamilyNameHdl.exportXML( sTmp, aAny, rUnitConv ) )
                    GetExport().AddAttribute( XML_NAMESPACE_FO,
                                              sXML_font_family, sTmp );

                if( sBulletFontStyleName.getLength() )
                    GetExport().AddAttribute( XML_NAMESPACE_FO,
                                              sXML_font_style_name,
                                              sBulletFontStyleName );

                XMLFontFamilyPropHdl aFamilyHdl;
                aAny <<= (sal_Int16)eBulletFontFamily;
                if( aFamilyHdl.exportXML( sTmp, aAny, rUnitConv  ) )
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              sXML_font_family_generic, sTmp );

                XMLFontPitchPropHdl aPitchHdl;
                aAny <<= (sal_Int16)eBulletFontPitch;
                if( aPitchHdl.exportXML( sTmp, aAny, rUnitConv  ) )
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              sXML_font_pitch, sTmp );

                XMLFontEncodingPropHdl aEncHdl;
                aAny <<= (sal_Int16)eBulletFontEncoding;
                if( aEncHdl.exportXML( sTmp, aAny, rUnitConv  ) )
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              sXML_font_charset, sTmp );
            }
        }
        else if( NumberingType::BITMAP == eType )
        {
            const char *pValue = 0;
            switch( eImageVertOrient )
            {
            case VertOrientation::BOTTOM:   // yes, its OK: BOTTOM means that the baseline
                                    // hits the frame at its topmost position
            case VertOrientation::LINE_TOP:
            case VertOrientation::CHAR_TOP:
                pValue = sXML_top;
                break;
            case VertOrientation::CENTER:
            case VertOrientation::LINE_CENTER:
            case VertOrientation::CHAR_CENTER:
                pValue = sXML_middle;
                break;
            case VertOrientation::TOP:      // yes, its OK: TOP means that the baseline
                                    // hits the frame at its bottommost position
            case VertOrientation::LINE_BOTTOM:
            case VertOrientation::CHAR_BOTTOM:
                pValue = sXML_bottom;
                break;
            }
            if( pValue )
                GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_vertical_pos, pValue );

            pValue = 0;
            switch( eImageVertOrient )
            {
            case VertOrientation::TOP:
            case VertOrientation::CENTER:
            case VertOrientation::BOTTOM:
                pValue = sXML_baseline;
                break;
            case VertOrientation::LINE_TOP:
            case VertOrientation::LINE_CENTER:
            case VertOrientation::LINE_BOTTOM:
                pValue = sXML_line;
                break;
            case VertOrientation::CHAR_TOP:
            case VertOrientation::CHAR_CENTER:
            case VertOrientation::CHAR_BOTTOM:
                pValue = sXML_char;
                break;
            }
            if( pValue )
                GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE, sXML_vertical_rel, pValue );

            if( nImageWidth > 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nImageWidth );
                GetExport().AddAttribute( XML_NAMESPACE_FO, sXML_width,
                              sBuffer.makeStringAndClear() );
            }

            if( nImageHeight > 0 )
            {
                GetExport().GetMM100UnitConverter().convertMeasure( sBuffer, nImageHeight );
                GetExport().AddAttribute( XML_NAMESPACE_FO, sXML_height,
                              sBuffer.makeStringAndClear() );
            }
        }

        if( GetExport().GetAttrList().getLength() > 0 )
        {
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                      sXML_properties, sal_True, sal_True );
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
    sIsContinuousNumbering( RTL_CONSTASCII_USTRINGPARAM( "IsContinuousNumbering" ) )
{
}

SvxXMLNumRuleExport::~SvxXMLNumRuleExport()
{
}

void SvxXMLNumRuleExport::Export( const OUString& rName,
                                  sal_Bool bContNumbering )
{
    GetExport().CheckAttrList();

    // style:name="..."
    GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_name, rName );

    // text:consecutive-numbering="..."
    if( bContNumbering )
        GetExport().AddAttributeASCII( XML_NAMESPACE_TEXT, sXML_consecutive_numbering,
                             sXML_true );

    // other application specific attributes
    AddListStyleAttributes();

    OUString sElem = GetExport().GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TEXT,
                                   OUString::createFromAscii(sXML_list_style) );
    GetExport().GetDocHandler()->ignorableWhitespace( sWS );
    GetExport().GetDocHandler()->startElement( sElem, GetExport().GetXAttrList() );
    GetExport().ClearAttrList();

    uno::Reference< ::com::sun::star::container::XIndexReplace >  xNumRule = GetUNONumRule();
    if( xNumRule.is() )
        exportLevelStyles( xNumRule );

    GetExport().GetDocHandler()->ignorableWhitespace( sWS );
    GetExport().GetDocHandler()->endElement( sElem );
}

void SvxXMLNumRuleExport::ExportOutline()
{
    GetExport().CheckAttrList();

    OUString sElem = GetExport().GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_TEXT,
                                   OUString::createFromAscii(sXML_outline_style) );
    GetExport().GetDocHandler()->ignorableWhitespace( sWS );
    GetExport().GetDocHandler()->startElement( sElem, GetExport().GetXAttrList() );

    uno::Reference< ::com::sun::star::container::XIndexReplace >  xNumRule = GetUNONumRule();
    if( xNumRule.is() )
        exportLevelStyles( xNumRule, sal_True );

    GetExport().GetDocHandler()->ignorableWhitespace( sWS );
    GetExport().GetDocHandler()->endElement( sElem );
}

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
    GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_name, rName );

    // text:consecutive-numbering="..."
    sal_Bool bContNumbering = sal_False;
    if( xPropSetInfo.is() &&
        xPropSetInfo->hasPropertyByName( sIsContinuousNumbering ) )
    {
        Any aAny( xPropSet->getPropertyValue( sIsContinuousNumbering ) );
        bContNumbering = *(sal_Bool *)aAny.getValue();
    }
    if( bContNumbering )
        GetExport().AddAttributeASCII( XML_NAMESPACE_TEXT,
                                       sXML_consecutive_numbering, sXML_true );

    // other application specific attributes
    AddListStyleAttributes();

    {
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT, sXML_list_style ,
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
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                      sXML_outline_style, sal_True, sal_True );
            exportLevelStyles( xNumRule, sal_True );
        }
    }
}

void SvxXMLNumRuleExport::exportStyles( sal_Bool bUsed,
                                         XMLTextListAutoStylePool *pPool )
{
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

const sal_Char *SvxXMLNumRuleExport::GetNumFormatValue( sal_Int16 eNumType )
{
    const sal_Char *pFormat = 0;
    switch( eNumType )
    {
    case NumberingType::CHARS_UPPER_LETTER: pFormat = sXML_A; break;
    case NumberingType::CHARS_LOWER_LETTER: pFormat = sXML_a; break;
    case NumberingType::ROMAN_UPPER:            pFormat = sXML_I; break;
    case NumberingType::ROMAN_LOWER:            pFormat = sXML_i; break;
    case NumberingType::ARABIC:             pFormat = sXML_1; break;
    case NumberingType::CHARS_UPPER_LETTER_N:   pFormat = sXML_A; break;
    case NumberingType::CHARS_LOWER_LETTER_N:   pFormat = sXML_a; break;
    case NumberingType::NUMBER_NONE:            pFormat = sXML__empty; break;

    case NumberingType::CHAR_SPECIAL:
    case NumberingType::PAGE_DESCRIPTOR:
    case NumberingType::BITMAP:
        DBG_ASSERT( pFormat, "invalid number format" );
        break;
    }

    return pFormat;
}

const sal_Char *SvxXMLNumRuleExport::GetNumLetterSync( sal_Int16 eNumType )
{
    const sal_Char *pSync = 0;
    switch( eNumType )
    {
    case NumberingType::CHARS_UPPER_LETTER:
    case NumberingType::CHARS_LOWER_LETTER:
    case NumberingType::ROMAN_UPPER:
    case NumberingType::ROMAN_LOWER:
    case NumberingType::ARABIC:
    case NumberingType::NUMBER_NONE:
        // default
        // pSync = sXML_false;
        break;

    case NumberingType::CHARS_UPPER_LETTER_N:
    case NumberingType::CHARS_LOWER_LETTER_N:
        pSync = sXML_true;
        break;

    case NumberingType::CHAR_SPECIAL:
    case NumberingType::PAGE_DESCRIPTOR:
    case NumberingType::BITMAP:
        DBG_ASSERT( pSync, "invalid number format" );
        break;
    }

    return pSync;
}

