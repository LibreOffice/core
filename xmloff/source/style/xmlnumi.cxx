/*************************************************************************
 *
 *  $RCSfile: xmlnumi.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:06 $
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

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "xmltkmap.hxx"
#include "nmspmap.hxx"
#include "xmlnmspe.hxx"
#include "xmlimp.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
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
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#include "xmlkywd.hxx"
#include "i18nmap.hxx"
#include "xmluconv.hxx"
#ifndef _XMLOFF_PROPERTYHANDLER_FONTTYPES_HXX
#include "fonthdl.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX
#include "families.hxx"
#endif


#include "xmlnumi.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE[] =
        "SymbolTextDistance";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_PARENT_NUMBERING[] =
        "ParentNumbering";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_CHAR_STYLE_NAME[] =
        "CharStyleName";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_BULLET_CHAR[] ="BulletChar";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_GRAPHIC_SIZE[] =
        "GraphicSize";
static sal_Char __READONLY_DATA XML_UNO_NAME_NRULE_VERT_ORIENT[] ="VertOrient";

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

// ---------------------------------------------------------------------

class SvxXMLListLevelStyleContext_Impl;

class SvxXMLListLevelStyleAttrContext_Impl : public SvXMLImportContext
{
    SvxXMLListLevelStyleContext_Impl&   rListLevel;

public:

    SvxXMLListLevelStyleAttrContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
             const OUString& rLName,
              const Reference< xml::sax::XAttributeList >& xAttrList,
            SvxXMLListLevelStyleContext_Impl& rLLevel   );
    virtual ~SvxXMLListLevelStyleAttrContext_Impl();
};

// ---------------------------------------------------------------------

enum SvxXMLTextListLevelStyleAttrTokens
{
    XML_TOK_TEXT_LEVEL_ATTR_LEVEL,
    XML_TOK_TEXT_LEVEL_ATTR_STYLE_NAME,
    XML_TOK_TEXT_LEVEL_ATTR_BULLET_CHAR,
    XML_TOK_TEXT_LEVEL_ATTR_HREF,
    XML_TOK_TEXT_LEVEL_ATTR_TYPE,
    XML_TOK_TEXT_LEVEL_ATTR_SHOW,
    XML_TOK_TEXT_LEVEL_ATTR_ACTUATE,
    XML_TOK_TEXT_LEVEL_ATTR_NUM_FORMAT,
    XML_TOK_TEXT_LEVEL_ATTR_NUM_PREFIX,
    XML_TOK_TEXT_LEVEL_ATTR_NUM_SUFFIX,
    XML_TOK_TEXT_LEVEL_ATTR_NUM_LETTER_SYNC,
    XML_TOK_TEXT_LEVEL_ATTR_START_VALUE,
    XML_TOK_TEXT_LEVEL_ATTR_DISPLAY_LEVELS,

    XML_TOK_TEXT_LEVEL_ATTR_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aLevelAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_level, XML_TOK_TEXT_LEVEL_ATTR_LEVEL },
    { XML_NAMESPACE_TEXT, sXML_style_name, XML_TOK_TEXT_LEVEL_ATTR_STYLE_NAME },
    { XML_NAMESPACE_TEXT, sXML_bullet_char, XML_TOK_TEXT_LEVEL_ATTR_BULLET_CHAR },
    { XML_NAMESPACE_XLINK, sXML_href, XML_TOK_TEXT_LEVEL_ATTR_HREF },
    { XML_NAMESPACE_XLINK, sXML_type, XML_TOK_TEXT_LEVEL_ATTR_TYPE },
    { XML_NAMESPACE_XLINK, sXML_show, XML_TOK_TEXT_LEVEL_ATTR_SHOW },
    { XML_NAMESPACE_XLINK, sXML_actuate, XML_TOK_TEXT_LEVEL_ATTR_ACTUATE },

    { XML_NAMESPACE_STYLE, sXML_num_format, XML_TOK_TEXT_LEVEL_ATTR_NUM_FORMAT },
    { XML_NAMESPACE_STYLE, sXML_num_prefix, XML_TOK_TEXT_LEVEL_ATTR_NUM_PREFIX },
    { XML_NAMESPACE_STYLE, sXML_num_suffix, XML_TOK_TEXT_LEVEL_ATTR_NUM_SUFFIX },
    { XML_NAMESPACE_STYLE, sXML_num_letter_sync, XML_TOK_TEXT_LEVEL_ATTR_NUM_LETTER_SYNC },
    { XML_NAMESPACE_TEXT, sXML_start_value, XML_TOK_TEXT_LEVEL_ATTR_START_VALUE },
    { XML_NAMESPACE_TEXT, sXML_display_levels, XML_TOK_TEXT_LEVEL_ATTR_DISPLAY_LEVELS },

    XML_TOKEN_MAP_END
};

class SvxXMLListLevelStyleContext_Impl : public SvXMLImportContext
{
    friend class SvxXMLListLevelStyleAttrContext_Impl;

    OUString            sPrefix;
    OUString            sSuffix;
    OUString            sTextStyleName;
    OUString            sNumFormat;
    OUString            sNumLetterSync;
    OUString            sBulletFontName;
    OUString            sBulletFontStyleName;
    OUString            sImageURL;

    sal_Int32           nLevel;
    sal_Int32           nSpaceBefore;
    sal_Int32           nMinLabelWidth;
    sal_Int32           nMinLabelDist;
    sal_Int32           nImageWidth;
    sal_Int32           nImageHeight;
    sal_Int16           nNumStartValue;
    sal_Int16           nNumDisplayLevels;

    sal_Int16           eAdjust;
    sal_Int16           eBulletFontFamily;
    sal_Int16           eBulletFontPitch;
    rtl_TextEncoding    eBulletFontEncoding;
    sal_Int16           eImageVertOrient;

    sal_Unicode         cBullet;

    sal_Bool            bBullet : 1;
    sal_Bool            bImage : 1;
    sal_Bool            bNum : 1;

    void SetSpaceBefore( sal_Int32 nSet ) { nSpaceBefore = nSet; }
    void SetMinLabelWidth( sal_Int32 nSet ) { nMinLabelWidth = nSet; }
    void SetMinLabelDist( sal_Int32 nSet ) { nMinLabelDist = nSet; }
    void SetAdjust( sal_Int16 eSet ) { eAdjust = eSet; }

    void SetBulletFontName( const OUString& rSet ) { sBulletFontName = rSet; }
    void SetBulletFontStyleName( const OUString& rSet )
         { sBulletFontStyleName = rSet; }
    void SetBulletFontFamily( sal_Int16 eSet ) { eBulletFontFamily = eSet; }
    void SetBulletFontPitch( sal_Int16 eSet ) { eBulletFontPitch = eSet; }
    void SetBulletFontEncoding( rtl_TextEncoding eSet )
         { eBulletFontEncoding = eSet; }

    void SetImageWidth( sal_Int32 nSet ) { nImageWidth = nSet; }
    void SetImageHeight( sal_Int32 nSet ) { nImageHeight = nSet; }
    void SetImageVertOrient( sal_Int16 eSet )
         { eImageVertOrient = eSet; }

public:

    SvxXMLListLevelStyleContext_Impl(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~SvxXMLListLevelStyleContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix, const OUString& rLocalName,
            const Reference< xml::sax::XAttributeList > & xAttrList );

    sal_Int32 GetLevel() const { return nLevel; }
    Sequence<beans::PropertyValue> GetProperties(
            const SvI18NMap *pI18NMap=0 ) const;
};

SvxXMLListLevelStyleContext_Impl::SvxXMLListLevelStyleContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList ) :

    SvXMLImportContext( rImport, nPrfx, rLName ),
    sNumFormat( OUString::createFromAscii( "1" ) ),
    nLevel( -1L ),
    nSpaceBefore( 0L ),
    nMinLabelWidth( 0L ),
    nMinLabelDist( 0L ),
    nImageWidth( 0L ),
    nImageHeight( 0L ),
    nNumStartValue( 1 ),
    nNumDisplayLevels( 1 ),
    eAdjust( HoriOrientation::LEFT ),
    eBulletFontFamily( FAMILY_DONTKNOW ),
    eBulletFontPitch( PITCH_DONTKNOW ),
    eBulletFontEncoding( RTL_TEXTENCODING_DONTKNOW ),
    cBullet( 0 ),
    bNum( sal_False ),
    bBullet( sal_False ),
    bImage( sal_False )
{
    if( rLName.compareToAscii( sXML_list_level_style_number ) == 0 ||
        rLName.compareToAscii( sXML_outline_level_style ) == 0 )
        bNum = sal_True;
    else if( rLName.compareToAscii( sXML_list_level_style_bullet ) == 0 )
        bBullet = sal_True;
    else if( rLName.compareToAscii( sXML_list_level_style_image ) == 0 )
        bImage = sal_True;

    SvXMLTokenMap aTokenMap( aLevelAttrTokenMap );
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_LEVEL_ATTR_LEVEL:
            nLevel = rValue.toInt32();
            if( nLevel >= 1L )
                nLevel--;
            else
                nLevel = 0L;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_STYLE_NAME:
            sTextStyleName = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_BULLET_CHAR:
            if( rValue.getLength() > 0 )
                cBullet = rValue[0];
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_HREF:
            if( bImage )
                sImageURL = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_TYPE:
        case XML_TOK_TEXT_LEVEL_ATTR_SHOW:
        case XML_TOK_TEXT_LEVEL_ATTR_ACTUATE:
            // This properties will be ignored
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_NUM_FORMAT:
            if( bNum )
                sNumFormat = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_NUM_PREFIX:
            sPrefix = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_NUM_SUFFIX:
            sSuffix = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_NUM_LETTER_SYNC:
            if( bNum )
                sNumLetterSync = rValue;
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_START_VALUE:
            if( bNum )
            {
                sal_Int32 nTmp = rValue.toInt32();
                nNumStartValue =
                    (nTmp < 1L) ? 1 : ( (nTmp>SHRT_MAX) ? SHRT_MAX
                                                        : (sal_Int16)nTmp );
            }
            break;
        case XML_TOK_TEXT_LEVEL_ATTR_DISPLAY_LEVELS:
            if( bNum )
            {
                sal_Int32 nTmp = rValue.toInt32();
                nNumDisplayLevels =
                    (nTmp < 1L) ? 1 : ( (nTmp>SHRT_MAX) ? SHRT_MAX
                                                        : (sal_Int16)nTmp );
            }
            break;
        }
    }
}

SvxXMLListLevelStyleContext_Impl::~SvxXMLListLevelStyleContext_Impl()
{
}

SvXMLImportContext *SvxXMLListLevelStyleContext_Impl::CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext;
    if( XML_NAMESPACE_STYLE == nPrefix &&
        rLocalName.compareToAscii( sXML_properties ) == 0 )
    {
        pContext = new SvxXMLListLevelStyleAttrContext_Impl( GetImport(),
                                                             nPrefix,
                                                             rLocalName,
                                                               xAttrList,
                                                             *this );
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

Sequence<beans::PropertyValue> SvxXMLListLevelStyleContext_Impl::GetProperties(
        const SvI18NMap *pI18NMap ) const
{
    sal_Int16 eType;

    sal_Int32 nCount = 0L;
    if( bBullet && 0 != cBullet )
    {
        eType = NumberingType::CHAR_SPECIAL;
        nCount = 10L;
    }
    if( bImage && sImageURL.getLength() > 0L &&
        nImageWidth > 0L && nImageHeight > 0L )
    {
        eType = NumberingType::BITMAP;
        nCount = 11L;
    }
    if( bNum )
    {
        eType = SvxXMLListStyleContext::GetNumType( sNumFormat, sNumLetterSync,
                    NumberingType::ARABIC, sal_True );
        nCount = 10L;
    }

    Sequence<beans::PropertyValue> aPropSeq( nCount );
    if( nCount > 0 )
    {
        beans::PropertyValue *pProps = aPropSeq.getArray();
        sal_Int32 nPos = 0L;
        pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_NUMBERINGTYPE );
        pProps[nPos++].Value <<= (sal_Int16)eType ;

        pProps[nPos].Name = OUString::createFromAscii( XML_UNO_NAME_NRULE_PREFIX );
        pProps[nPos++].Value <<= sPrefix;

        pProps[nPos].Name = OUString::createFromAscii( XML_UNO_NAME_NRULE_SUFFIX );
        pProps[nPos++].Value <<= sSuffix;

        pProps[nPos].Name = OUString::createFromAscii( XML_UNO_NAME_NRULE_ADJUST );
        pProps[nPos++].Value <<= eAdjust;

        sal_Int32 nLeftMargin = nSpaceBefore + nMinLabelWidth;
        if( nLeftMargin < 0 )
            nLeftMargin = 0;
        pProps[nPos].Name =
            OUString::createFromAscii( XML_UNO_NAME_NRULE_LEFT_MARGIN );
        pProps[nPos++].Value <<= (sal_Int32)nLeftMargin;

        sal_Int32 nFirstLineOffset =
                nMinLabelWidth <= nLeftMargin ? -nMinLabelWidth : -nLeftMargin;

        pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_FIRST_LINE_OFFSET );
        pProps[nPos++].Value <<= (sal_Int32)nFirstLineOffset;

        pProps[nPos].Name =
            OUString::createFromAscii(XML_UNO_NAME_NRULE_SYMBOL_TEXT_DISTANCE);
        pProps[nPos++].Value <<= (sal_Int16)nMinLabelDist;

        OUString sStyleName = sTextStyleName;
        if( sStyleName.getLength() && pI18NMap )
            sStyleName = pI18NMap->Get( SFX_STYLE_FAMILY_CHAR, sStyleName );
        pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_CHAR_STYLE_NAME );
        pProps[nPos++].Value <<= sTextStyleName;

        if( bBullet )
        {
            OUStringBuffer sTmp(1);
            sTmp.append( cBullet );
            pProps[nPos].Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_CHAR );
            pProps[nPos++].Value <<= sTmp.makeStringAndClear();

            awt::FontDescriptor aFDesc;
            aFDesc.Name = sBulletFontName;
            if( sBulletFontName.getLength() )
            {
                aFDesc.StyleName = sBulletFontStyleName;
                aFDesc.Family = eBulletFontFamily;
                aFDesc.Pitch = eBulletFontPitch;
                aFDesc.CharSet = eBulletFontEncoding;
                aFDesc.Weight = WEIGHT_DONTKNOW;
                //aFDesc.Transparant = sal_True;
            }

            pProps[nPos].Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_FONT );
            pProps[nPos++].Value <<= aFDesc;
        }

        if( bImage )
        {
            String sURL( sImageURL );
            INetURLObject::RelToAbs( sURL );
            pProps[nPos].Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_GRAPHICURL );
            pProps[nPos++].Value <<= OUString(sURL);

            awt::Size aSize( nImageWidth, nImageHeight );
            pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_GRAPHIC_SIZE );
            pProps[nPos++].Value <<= aSize;

            pProps[nPos].Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_VERT_ORIENT );
            pProps[nPos++].Value <<= (sal_Int16)eImageVertOrient;
        }

        if( bNum )
        {
            pProps[nPos].Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_START_WITH );
            pProps[nPos++].Value <<= (sal_Int16)nNumStartValue;

            pProps[nPos].Name =
                OUString::createFromAscii(XML_UNO_NAME_NRULE_PARENT_NUMBERING);
            pProps[nPos++].Value <<= (sal_Int16)nNumDisplayLevels;
        }
        DBG_ASSERT( nPos == nCount, "array under/overflow" );
    }

    return aPropSeq;
}

// ---------------------------------------------------------------------

enum SvxXMLStyleAttributesAttrTokens
{
    XML_TOK_STYLE_ATTRIBUTES_ATTR_SPACE_BEFORE,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_WIDTH,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_DIST,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_TEXT_ALIGN,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY_GENERIC,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_STYLENAME,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_PITCH,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_CHARSET,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_POS,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_REL,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_WIDTH,
    XML_TOK_STYLE_ATTRIBUTES_ATTR_HEIGHT,

    XML_TOK_STYLE_ATTRIBUTES_ATTR_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aStyleAttributesAttrTokenMap[] =
{
    { XML_NAMESPACE_TEXT, sXML_space_before,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_SPACE_BEFORE },
    { XML_NAMESPACE_TEXT, sXML_min_label_width,
              XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_WIDTH },
    { XML_NAMESPACE_TEXT, sXML_min_label_distance,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_DIST },
    { XML_NAMESPACE_FO, sXML_text_align,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_TEXT_ALIGN },
    { XML_NAMESPACE_FO, sXML_font_family,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY },
    { XML_NAMESPACE_STYLE, sXML_font_family_generic,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY_GENERIC },
    { XML_NAMESPACE_STYLE, sXML_font_style_name,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_STYLENAME },
    { XML_NAMESPACE_STYLE, sXML_font_pitch,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_PITCH },
    { XML_NAMESPACE_STYLE, sXML_font_charset,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_CHARSET },
    { XML_NAMESPACE_STYLE, sXML_vertical_pos,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_POS },
    { XML_NAMESPACE_STYLE, sXML_vertical_rel,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_REL },
    { XML_NAMESPACE_FO, sXML_width,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_WIDTH },
    { XML_NAMESPACE_FO, sXML_height,
            XML_TOK_STYLE_ATTRIBUTES_ATTR_HEIGHT },

    XML_TOKEN_MAP_END
};
SvxXMLListLevelStyleAttrContext_Impl::SvxXMLListLevelStyleAttrContext_Impl(
        SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        SvxXMLListLevelStyleContext_Impl& rLLevel ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rListLevel( rLLevel )
{
    SvXMLTokenMap aTokenMap( aStyleAttributesAttrTokenMap );
    SvXMLUnitConverter aUnitConv( MAP_100TH_MM, MAP_100TH_MM);

    OUString sFontFamily, sFontStyleName, sFontFamilyGeneric,
             sFontPitch, sFontCharset;
    OUString sVerticalPos, sVerticalRel;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nVal;
        switch( aTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_SPACE_BEFORE:
            if( aUnitConv.convertMeasure( nVal, rValue, 0, USHRT_MAX ) )
                rListLevel.SetSpaceBefore( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_WIDTH:
            if( aUnitConv.convertMeasure( nVal, rValue, 0, SHRT_MAX ) )
                rListLevel.SetMinLabelWidth( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_MIN_LABEL_DIST:
            if( aUnitConv.convertMeasure( nVal, rValue, 0, USHRT_MAX ) )
                rListLevel.SetMinLabelDist( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_TEXT_ALIGN:
            if( rValue.getLength() )
            {
                sal_Int16 eAdjust = HoriOrientation::LEFT;
                if( rValue.compareToAscii( sXML_center ) == 0 )
                    eAdjust = HoriOrientation::CENTER;
                else if( rValue.compareToAscii( sXML_end ) == 0 )
                    eAdjust = HoriOrientation::RIGHT;
                rListLevel.SetAdjust( eAdjust );
            }
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY:
            sFontFamily = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_FAMILY_GENERIC:
            sFontFamilyGeneric = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_STYLENAME:
            sFontStyleName = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_PITCH:
            sFontPitch = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_FONT_CHARSET:
            sFontCharset = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_POS:
            sVerticalPos = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_VERTICAL_REL:
            sVerticalRel = rValue;
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_WIDTH:
            if( aUnitConv.convertMeasure( nVal, rValue, 0, LONG_MAX ) )
                rListLevel.SetImageWidth( nVal );
            break;
        case XML_TOK_STYLE_ATTRIBUTES_ATTR_HEIGHT:
            if( aUnitConv.convertMeasure( nVal, rValue, 0, LONG_MAX ) )
                rListLevel.SetImageHeight( nVal );
            break;
        }
    }

    if( sFontFamily.getLength() )
    {
        String sEmpty;
        Any aAny;

        XMLFontFamilyNamePropHdl aFamilyNameHdl;
        if( aFamilyNameHdl.importXML( sFontFamily, aAny, aUnitConv ) )
        {
            OUString sTmp;
            aAny >>= sTmp;
            rListLevel.SetBulletFontName( sTmp);
        }

        XMLFontFamilyPropHdl aFamilyHdl;
        if( sFontFamilyGeneric.getLength() &&
            aFamilyHdl.importXML( sFontFamilyGeneric, aAny, aUnitConv  ) )
        {
            sal_Int16 nTmp;
            aAny >>= nTmp;
            rListLevel.SetBulletFontFamily( nTmp );
        }

        if( sFontStyleName.getLength() )
            rListLevel.SetBulletFontStyleName( sFontStyleName );

        XMLFontPitchPropHdl aPitchHdl;
        if( sFontPitch.getLength() &&
            aPitchHdl.importXML( sFontPitch, aAny, aUnitConv  ) )
        {
            sal_Int16 nTmp;
            aAny >>= nTmp;
            rListLevel.SetBulletFontPitch( nTmp );
        }

        XMLFontEncodingPropHdl aEncHdl;
        if( sFontCharset.getLength() &&
            aEncHdl.importXML( sFontCharset, aAny, aUnitConv  ) )
        {
            sal_Int16 nTmp;
            aAny >>= nTmp;
            rListLevel.SetBulletFontEncoding( nTmp );
        }
    }

    sal_Int16 eVertOrient = VertOrientation::LINE_CENTER;
    if( sVerticalPos.getLength() )
    {
        if( sVerticalPos.compareToAscii( sXML_top ) == 0 )
            eVertOrient = VertOrientation::LINE_TOP;
        else if( sVerticalPos.compareToAscii( sXML_bottom ) == 0 )
            eVertOrient = VertOrientation::LINE_BOTTOM;
    }
    if( sVerticalRel.getLength() )
    {
        if( sVerticalRel.compareToAscii( sXML_baseline ) == 0 )
        {
            // TOP and BOTTOM are exchanged for a baseline relation
            switch( eVertOrient  )
            {
            case VertOrientation::LINE_TOP:
                eVertOrient = VertOrientation::BOTTOM;
                break;
            case VertOrientation::LINE_CENTER:
                eVertOrient = VertOrientation::CENTER;
                break;
            case VertOrientation::LINE_BOTTOM:
                eVertOrient = VertOrientation::TOP;
                break;
            }
        }
        else if( sVerticalRel.compareToAscii( sXML_char ) == 0 )
        {
            switch( eVertOrient  )
            {
            case VertOrientation::LINE_TOP:
                eVertOrient = VertOrientation::CHAR_TOP;
                break;
            case VertOrientation::LINE_CENTER:
                eVertOrient = VertOrientation::CHAR_CENTER;
                break;
            case VertOrientation::LINE_BOTTOM:
                eVertOrient = VertOrientation::CHAR_BOTTOM;
                break;
            }
        }
    }
    rListLevel.SetImageVertOrient( eVertOrient );
}

SvxXMLListLevelStyleAttrContext_Impl::~SvxXMLListLevelStyleAttrContext_Impl()
{
}

// ---------------------------------------------------------------------

typedef SvxXMLListLevelStyleContext_Impl *SvxXMLListLevelStyleContext_ImplPtr;
SV_DECL_PTRARR( SvxXMLListStyle_Impl, SvxXMLListLevelStyleContext_ImplPtr,
                10, 5 )

void SvxXMLListStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                           const OUString& rLocalName,
                                           const OUString& rValue )
{
    if( XML_NAMESPACE_TEXT == nPrefixKey &&
        rLocalName.compareToAscii( sXML_consecutive_numbering ) == 0 )
    {
        bConsecutive = rValue.compareToAscii( sXML_true ) == 0;
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}


SvxXMLListStyleContext::SvxXMLListStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bOutl ) :
    SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList ),
    pLevelStyles( 0 ),
    nLevels( 0 ),
    bConsecutive( sal_False ),
    bOutline( bOutl ),
    sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
    sNumberingRules( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ),
    sName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ),
    sIsContinuousNumbering( RTL_CONSTASCII_USTRINGPARAM( "IsContinuousNumbering" ) )
{
    SetFamily( bOutline ? XML_STYLE_FAMILY_TEXT_OUTLINE :
                          XML_STYLE_FAMILY_TEXT_LIST );
}

SvxXMLListStyleContext::~SvxXMLListStyleContext()
{
    if( pLevelStyles )
    {
        while( pLevelStyles->Count() )
        {
            sal_uInt16 n = pLevelStyles->Count() - 1;
            SvxXMLListLevelStyleContext_Impl *pStyle = (*pLevelStyles)[n];
            pLevelStyles->Remove( n, 1 );
            pStyle->ReleaseRef();
        }
    }

    delete pLevelStyles;
}

TYPEINIT1( SvxXMLListStyleContext, SvXMLStyleContext );

SvXMLImportContext *SvxXMLListStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_TEXT == nPrefix &&
        ( bOutline
              ? rLocalName.compareToAscii( sXML_outline_level_style ) == 0
              : ( rLocalName.compareToAscii(sXML_list_level_style_number) == 0 ||
                rLocalName.compareToAscii(sXML_list_level_style_bullet) == 0 ||
                 rLocalName.compareToAscii(sXML_list_level_style_image) == 0) ) )
    {
        SvxXMLListLevelStyleContext_Impl *pLevelStyle =
            new SvxXMLListLevelStyleContext_Impl( GetImport(), nPrefix,
                                                  rLocalName, xAttrList );
        if( !pLevelStyles )
            pLevelStyles = new SvxXMLListStyle_Impl;
        pLevelStyles->Insert( pLevelStyle, pLevelStyles->Count() );
        pLevelStyle->AddRef();

        pContext = pLevelStyle;
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SvxXMLListStyleContext::FillUnoNumRule(
        const Reference< container::XIndexReplace > & rNumRule,
        const SvI18NMap *pI18NMap ) const
{
    if( pLevelStyles )
    {
        sal_uInt16 nCount = pLevelStyles->Count();
        sal_Int32 nLevels = rNumRule->getCount();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            const SvxXMLListLevelStyleContext_Impl *pLevelStyle =
                (*pLevelStyles)[i];
            sal_Int32 nLevel = pLevelStyle->GetLevel();
            if( nLevel >= 0 && nLevel < nLevels )
            {
                Sequence<beans::PropertyValue> aProps =
                    pLevelStyle->GetProperties( pI18NMap );
                Any aAny;
                aAny <<= aProps;
                rNumRule->replaceByIndex( nLevel, aAny );
            }
        }
    }

    Reference < XPropertySet > xPropSet( rNumRule, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo;
    if (xPropSet.is())
        xPropSetInfo = xPropSet->getPropertySetInfo();
    if( xPropSetInfo.is() &&
        xPropSetInfo->hasPropertyByName( sIsContinuousNumbering ) )
    {
        Any aAny;
        sal_Bool bTmp = bConsecutive;
        aAny.setValue( &bTmp, ::getBooleanCppuType() );
        xPropSet->setPropertyValue( sIsContinuousNumbering, aAny );
    }
}

void SvxXMLListStyleContext::CreateAndInsertLate( sal_Bool bOverwrite )
{
    if( bOutline )
    {
        if( bOverwrite )
        {
            const Reference< XIndexReplace >& rNumRule =
                GetImport().GetTextImport()->GetChapterNumbering();
            if( rNumRule.is() )
                FillUnoNumRule( rNumRule, 0 );
        }
    }
    else
    {
        Reference < XStyle > xStyle;
        const OUString& rName = GetName();
        if( 0 == rName.getLength() )
        {
            SetValid( sal_False );
            return;
        }

        const Reference < XNameContainer >& rNumStyles =
                GetImport().GetTextImport()->GetNumberingStyles();
        if( !rNumStyles.is() )
        {
            SetValid( sal_False );
            return;
        }

        sal_Bool bNew = sal_False;
        if( rNumStyles->hasByName( rName ) )
        {
            Any aAny = rNumStyles->getByName( rName );
            aAny >>= xStyle;
        }
        else
        {
            Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                        UNO_QUERY );
            DBG_ASSERT( xFactory.is(), "no factory" );
            if( !xFactory.is() )
                return;

            Reference < XInterface > xIfc = xFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.NumberingStyle")) );
            if( !xIfc.is() )
                return;
            Reference < XStyle > xTmp( xIfc, UNO_QUERY );
            xStyle = xTmp;
            if( !xStyle.is() )
                return;

            Any aAny;
            aAny <<= xStyle;
            rNumStyles->insertByName( rName, aAny );
            bNew = sal_True;
        }

        Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        if( !bNew && xPropSetInfo->hasPropertyByName( sIsPhysical ) )
        {
            Any aAny = xPropSet->getPropertyValue( sIsPhysical );
            bNew = !*(sal_Bool *)aAny.getValue();
        }

        // for styles the real name is the style name
        sRealName = rName;

        Any aAny = xPropSet->getPropertyValue( sNumberingRules );
        Reference<XIndexReplace> xNumRule;
        aAny >>= xNumRule;
        nLevels = xNumRule->getCount();
        if( bOverwrite || bNew )
        {
            FillUnoNumRule( xNumRule, 0 );
            aAny <<= xNumRule;
            xPropSet->setPropertyValue( sNumberingRules, aAny );
        }
        else
        {
            SetValid( sal_False );
        }

        SetNew( bNew );
    }
}

void SvxXMLListStyleContext::CreateAndInsertAuto() const
{
    DBG_ASSERT( !bOutline, "Outlines cannot be inserted here" );
    DBG_ASSERT( !bUsed, "Numbering Rule is existing already" );

    const OUString& rName = GetName();
    if( bOutline || bUsed || 0 == rName.getLength() )
    {
        ((SvxXMLListStyleContext *)this)->SetValid( sal_False );
        return;
    }

    Reference<XIndexReplace> xNumRule = CreateNumRule(
        ((SvxXMLListStyleContext *)this)->sRealName, GetImport().GetModel() );
    DBG_ASSERT( sRealName.getLength(), "no internal name" );
    ((SvxXMLListStyleContext *)this)->nLevels = xNumRule->getCount();

    FillUnoNumRule( xNumRule, 0 );
}

Reference < XIndexReplace > SvxXMLListStyleContext::CreateNumRule(
                                OUString& rRealName,
                                const Reference < XModel > & rModel )
{
    Reference<XIndexReplace> xNumRule;

    Reference< XMultiServiceFactory > xFactory( rModel, UNO_QUERY );
    DBG_ASSERT( xFactory.is(), "no factory" );
    if( !xFactory.is() )
        return xNumRule;

    Reference < XInterface > xIfc = xFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.NumberingRule")) );
    if( !xIfc.is() )
        return xNumRule;

    xNumRule = Reference<XIndexReplace>( xIfc, UNO_QUERY );
    DBG_ASSERT( xNumRule.is(), "go no numbering rule" );
    if( !xNumRule.is() )
        return xNumRule;

    Reference < XPropertySet > xPropSet( xNumRule, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo;
    if (xPropSet.is())
        xPropSetInfo = xPropSet->getPropertySetInfo();
    const OUString sName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );
    if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( sName ) )
    {
        Any aAny = xPropSet->getPropertyValue( sName );
        aAny >>= rRealName;
    }

    return xNumRule;
}

void SvxXMLListStyleContext::SetDefaultStyle(
        const Reference < XIndexReplace > & rNumRule,
        sal_Int16 nLevel,
        sal_Bool bOrdered )
{
    Sequence<beans::PropertyValue> aPropSeq( bOrdered ? 1 : 4  );
    beans::PropertyValue *pProps = aPropSeq.getArray();

    pProps->Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_NUMBERINGTYPE );
    (pProps++)->Value <<= (sal_Int16)(bOrdered ? NumberingType::ARABIC
                                                 : NumberingType::CHAR_SPECIAL );
    if( !bOrdered )
    {
        // TODO: Bullet-Font
        //aNumFmt.SetBulletFont( &SwNumRule::GetDefBulletFont() );
        awt::FontDescriptor aFDesc;
        aFDesc.Name = OUString(
#if defined UNX
                        RTL_CONSTASCII_USTRINGPARAM( "starbats" )
#else
                        RTL_CONSTASCII_USTRINGPARAM( "StarBats" )
#endif
                                        );
        aFDesc.Family = FAMILY_DONTKNOW ;
        aFDesc.Pitch = PITCH_DONTKNOW ;
        aFDesc.CharSet = RTL_TEXTENCODING_SYMBOL ;
        aFDesc.Weight = WEIGHT_DONTKNOW;
        //aFDesc.Transparant = sal_True;
        pProps->Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_FONT );
        (pProps++)->Value <<= aFDesc;

        OUStringBuffer sTmp(1);
        sTmp.append( (sal_Unicode)(0xF000 + 149) );
        pProps->Name =
                    OUString::createFromAscii( XML_UNO_NAME_NRULE_BULLET_CHAR );
        (pProps++)->Value <<= sTmp.makeStringAndClear();
        pProps->Name =
                OUString::createFromAscii( XML_UNO_NAME_NRULE_CHAR_STYLE_NAME );
        (pProps++)->Value <<=
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Numbering Symbols" ) );
    }

    Any aAny;
    aAny <<= aPropSeq;
    rNumRule->replaceByIndex( nLevel, aAny );
}

sal_Int16 SvxXMLListStyleContext::GetNumType( const OUString& rNumFmt,
                                                   const OUString& rLetterSync,
                                                  sal_Int16 eDflt,
                                                  sal_Bool bNumberNone )
{
    sal_Int16 eValue = eDflt;
    sal_Int32 nLen = rNumFmt.getLength();

    if( 1 == nLen )
    {
        switch( rNumFmt[0] )
        {
        case sal_Unicode('1'):  eValue = NumberingType::ARABIC;         break;
        case sal_Unicode('a'):  eValue = NumberingType::CHARS_LOWER_LETTER; break;
        case sal_Unicode('A'):  eValue = NumberingType::CHARS_UPPER_LETTER; break;
        case sal_Unicode('i'):  eValue = NumberingType::ROMAN_LOWER;    break;
        case sal_Unicode('I'):  eValue = NumberingType::ROMAN_UPPER;    break;
        }
        if( rLetterSync.compareToAscii( sXML_true ) == 0 )
        {
            switch(eValue )
            {
            case NumberingType::CHARS_LOWER_LETTER:
                eValue = NumberingType::CHARS_LOWER_LETTER_N;
                break;
            case NumberingType::CHARS_UPPER_LETTER:
                eValue = NumberingType::CHARS_UPPER_LETTER_N;
                break;
            }
        }
    }
    else if( 0 == nLen )
    {
        if( bNumberNone )
            eValue = NumberingType::NUMBER_NONE;
    }

    return eValue;
}


