/*************************************************************************
 *
 *  $RCSfile: unomap.cxx,v $
 *
 *  $Revision: 1.153 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:57:27 $
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

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif
#ifndef _COMPHELPER_TYPEGENERATION_HXX
#include <comphelper/TypeGeneration.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PropertyAttribute_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_PAGENUMBERTYPE_HPP_
#include <com/sun/star/text/PageNumberType.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef SW_UNOMID_HXX
#include <unomid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::rtl;
using namespace ::comphelper;

#define SW_PROP_NMID(id)    ((const char*)id), 0

#ifndef MID_TXT_LMARGIN
#define MID_TXT_LMARGIN 11
#endif

/******************************************************************************
    UI-Maps
******************************************************************************/

SwUnoPropertyMapProvider aSwMapProvider;

/* -----------------24.06.98 18:12-------------------
 *
 * --------------------------------------------------*/
SwUnoPropertyMapProvider::SwUnoPropertyMapProvider()
{
    for( sal_uInt16 i = 0; i < PROPERTY_MAP_END; i++ )
    {
        aMapArr[i] = 0;
    }
}
/* -----------------19.02.99 08:31-------------------
 *
 * --------------------------------------------------*/
SwUnoPropertyMapProvider::~SwUnoPropertyMapProvider()
{
    delete pCharStyleMap;
    delete pParaStyleMap;
    delete pFrameStyleMap;
    delete pPageStyleMap;
    delete pNumStyleMap;
}
/* -----------------19.02.99 08:31-------------------
 *
 * --------------------------------------------------*/
SfxItemPropertySet& SwUnoPropertyMapProvider::GetPropertySet(sal_Int8 nPropSetId)
{
    SfxItemPropertySet** ppMap = 0;
    sal_uInt16 nPropertyId;
    switch(nPropSetId)
    {
    case PROPERTY_SET_CHAR_STYLE:
        ppMap = &pCharStyleMap;
        nPropertyId = PROPERTY_MAP_CHAR_STYLE;
        break;
    case PROPERTY_SET_PARA_STYLE:
        ppMap = &pParaStyleMap;
        nPropertyId = PROPERTY_MAP_PARA_STYLE;
        break;
    case PROPERTY_SET_FRAME_STYLE:
        ppMap = &pFrameStyleMap;
        nPropertyId = PROPERTY_MAP_FRAME_STYLE;
        break;
    case PROPERTY_SET_PAGE_STYLE:
        ppMap = &pPageStyleMap;
        nPropertyId = PROPERTY_MAP_PAGE_STYLE;
        break;
    case PROPERTY_SET_NUM_STYLE:
        ppMap = &pNumStyleMap;
        nPropertyId = PROPERTY_MAP_NUM_STYLE;
        break;
    }

    SfxItemPropertySet* pRet;
    if( ppMap )
    {
        if( !*ppMap )
            *ppMap = new SwItemPropertySet( GetPropertyMap( nPropertyId ));
        pRet = *ppMap;
    }
    else
        pRet = 0;
    return *pRet;
}
/* -----------------25.06.98 07:19-------------------
 *
 * --------------------------------------------------*/
EXTERN_C
#if defined( PM2 ) && (!defined( CSET ) && !defined ( MTW ) && !defined( WTC ))
int _stdcall
#else
#ifdef WNT
int _cdecl
#else
int
#endif
#endif
lcl_CompareMap(const void* pSmaller, const void* pBigger )
{
    int nDiff = strcmp( ((const SfxItemPropertyMap*)pSmaller)->pName,
                        ((const SfxItemPropertyMap*)pBigger)->pName );
    return nDiff;

}
/* -----------------24.06.98 18:12-------------------
 *
 * --------------------------------------------------*/
void SwUnoPropertyMapProvider::Sort( sal_uInt16 nId )
{
    SfxItemPropertyMap* p = aMapArr[nId];
    sal_uInt16 i = 0;
    for( ; p->pName; ++p, ++i )
    {
        // set the name
        const SwPropNameLen& rPropNm = GetPropName( (USHORT)(long)p->pName );
        p->pName = rPropNm.pName;
        p->nNameLen = rPropNm.nNameLen;
        // get the cppu type from the comphelper
        sal_uInt16 nTyp = (sal_uInt16) (long) p->pType;
        GenerateCppuType( nTyp, p->pType );
        DBG_ASSERT( nTyp != (sal_uInt16) (long) p->pType, "unknown type" );
    }
    qsort( aMapArr[nId], i, sizeof(SfxItemPropertyMap), lcl_CompareMap );
}


#define _STANDARD_FONT_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_CHAR_HEIGHT), RES_CHRATR_FONTSIZE  ,  CPPU_E2T(CPPUTYPE_FLOAT),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},      \
    { SW_PROP_NMID(UNO_NAME_CHAR_WEIGHT), RES_CHRATR_WEIGHT    ,  CPPU_E2T(CPPUTYPE_FLOAT),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},                    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME), RES_CHRATR_FONT,       CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },                  \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME), RES_CHRATR_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY), RES_CHRATR_FONT,     CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET), RES_CHRATR_FONT,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH), RES_CHRATR_FONT,      CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NMID(UNO_NAME_CHAR_POSTURE), RES_CHRATR_POSTURE   ,  CPPU_E2T(CPPUTYPE_FONTSLANT),        PropertyAttribute::MAYBEVOID, MID_POSTURE},                   \
    { SW_PROP_NMID(UNO_NAME_CHAR_LOCALE), RES_CHRATR_LANGUAGE ,   CPPU_E2T(CPPUTYPE_LOCALE)  ,          PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _CJK_FONT_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_CHAR_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE  ,  CPPU_E2T(CPPUTYPE_FLOAT),           PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},   \
    { SW_PROP_NMID(UNO_NAME_CHAR_WEIGHT_ASIAN), RES_CHRATR_CJK_WEIGHT    ,  CPPU_E2T(CPPUTYPE_FLOAT),           PropertyAttribute::MAYBEVOID, MID_WEIGHT},                 \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME_ASIAN), RES_CHRATR_CJK_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },               \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN), RES_CHRATR_CJK_FONT,       CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                 \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY_ASIAN), RES_CHRATR_CJK_FONT,       CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN), RES_CHRATR_CJK_FONT,     CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                 \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH_ASIAN), RES_CHRATR_CJK_FONT,        CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NMID(UNO_NAME_CHAR_POSTURE_ASIAN), RES_CHRATR_CJK_POSTURE   ,  CPPU_E2T(CPPUTYPE_FONTSLANT),          PropertyAttribute::MAYBEVOID, MID_POSTURE},                \
    { SW_PROP_NMID(UNO_NAME_CHAR_LOCALE_ASIAN), RES_CHRATR_CJK_LANGUAGE ,   CPPU_E2T(CPPUTYPE_LOCALE)  ,        PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _CTL_FONT_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_CHAR_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE  ,  CPPU_E2T(CPPUTYPE_FLOAT),         PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},\
    { SW_PROP_NMID(UNO_NAME_CHAR_WEIGHT_COMPLEX), RES_CHRATR_CTL_WEIGHT    ,  CPPU_E2T(CPPUTYPE_FLOAT),             PropertyAttribute::MAYBEVOID, MID_WEIGHT},              \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME_COMPLEX), RES_CHRATR_CTL_FONT,       CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },            \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX), RES_CHRATR_CTL_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },              \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY_COMPLEX), RES_CHRATR_CTL_FONT,     CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX), RES_CHRATR_CTL_FONT,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },              \
    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH_COMPLEX), RES_CHRATR_CTL_FONT,      CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NMID(UNO_NAME_CHAR_POSTURE_COMPLEX), RES_CHRATR_CTL_POSTURE   ,  CPPU_E2T(CPPUTYPE_FONTSLANT),        PropertyAttribute::MAYBEVOID, MID_POSTURE},             \
    { SW_PROP_NMID(UNO_NAME_CHAR_LOCALE_COMPLEX), RES_CHRATR_CTL_LANGUAGE ,   CPPU_E2T(CPPUTYPE_LOCALE)  ,          PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _REDLINE_NODE_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_START_REDLINE), FN_UNO_REDLINE_NODE_START , CPPU_E2T(CPPUTYPE_PROPERTYVALUE),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff }, \
    { SW_PROP_NMID(UNO_NAME_END_REDLINE), FN_UNO_REDLINE_NODE_END ,     CPPU_E2T(CPPUTYPE_PROPERTYVALUE),       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff },

#define _REDLINE_PROPERTIES \
    {SW_PROP_NMID(UNO_NAME_REDLINE_AUTHOR), 0, CPPU_E2T(CPPUTYPE_OUSTRING),                     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_DATE_TIME), 0, CPPU_E2T(CPPUTYPE_DATETIME),                  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_COMMENT), 0, CPPU_E2T(CPPUTYPE_OUSTRING),                        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_TYPE), 0, CPPU_E2T(CPPUTYPE_OUSTRING),                       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_SUCCESSOR_DATA), 0, CPPU_E2T(CPPUTYPE_PROPERTYVALUE),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_IDENTIFIER), 0, CPPU_E2T(CPPUTYPE_OUSTRING),                         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    {SW_PROP_NMID(UNO_NAME_IS_IN_HEADER_FOOTER), 0, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    {SW_PROP_NMID(UNO_NAME_REDLINE_TEXT), 0, CPPU_E2T(CPPUTYPE_REFTEXT),                    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NMID(UNO_NAME_MERGE_LAST_PARA), 0, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},

/* -----------------24.06.98 18:12-------------------
 *
 * --------------------------------------------------*/
#define COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        { SW_PROP_NMID(UNO_NAME_PARA_STYLE_NAME), FN_UNO_PARA_STYLE,        CPPU_E2T(CPPUTYPE_OUSTRING),                PropertyAttribute::MAYBEVOID,     0},                                                       \
        { SW_PROP_NMID(UNO_NAME_PAGE_STYLE_NAME), FN_UNO_PAGE_STYLE,        CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},                       \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_IS_NUMBER), FN_UNO_IS_NUMBER,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_LEVEL), FN_UNO_NUM_LEVEL,     CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, 0},                                                            \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_RULES), FN_UNO_NUM_RULES,     CPPU_E2T(CPPUTYPE_REFIDXREPL),  PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                        \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_START_VALUE), FN_UNO_NUM_START_VALUE, CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                                                \
        { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX), FN_UNO_DOCUMENT_INDEX, CPPU_E2T(CPPUTYPE_REFDOCINDEX), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },            \
        { SW_PROP_NMID(UNO_NAME_TEXT_TABLE), FN_UNO_TEXT_TABLE,     CPPU_E2T(CPPUTYPE_REFTXTTABLE),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },               \
        { SW_PROP_NMID(UNO_NAME_CELL), FN_UNO_CELL,         CPPU_E2T(CPPUTYPE_REFCELL),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { SW_PROP_NMID(UNO_NAME_TEXT_FRAME), FN_UNO_TEXT_FRAME,     CPPU_E2T(CPPUTYPE_REFTEXTFRAME),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { SW_PROP_NMID(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, CPPU_E2T(CPPUTYPE_REFTEXTSECTION),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                    \
        { SW_PROP_NMID(UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL), FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL,CPPU_E2T(CPPUTYPE_INT8),   PropertyAttribute::READONLY, 0},                                                     \
        { SW_PROP_NMID(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME), FN_UNO_PARA_CONDITIONAL_STYLE_NAME, CPPU_E2T(CPPUTYPE_OUSTRING),      PropertyAttribute::READONLY, 0},                                                     \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_NUMBERING_RESTART), FN_NUMBER_NEWSTART,     CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, 0 },

#define COMMON_HYPERLINK_PROPERTIES \
        { SW_PROP_NMID(UNO_NAME_HYPER_LINK_U_R_L), RES_TXTATR_INETFMT,          CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_URL},                \
        { SW_PROP_NMID(UNO_NAME_HYPER_LINK_TARGET), RES_TXTATR_INETFMT,         CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_TARGET},             \
        { SW_PROP_NMID(UNO_NAME_HYPER_LINK_NAME), RES_TXTATR_INETFMT,           CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_HYPERLINKNAME  },    \
        { SW_PROP_NMID(UNO_NAME_UNVISITED_CHAR_STYLE_NAME), RES_TXTATR_INETFMT, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_UNVISITED_FMT   },   \
        { SW_PROP_NMID(UNO_NAME_VISITED_CHAR_STYLE_NAME), RES_TXTATR_INETFMT,   CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_URL_VISITED_FMT  },

#define COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_HYPHENATION), RES_PARATR_HYPHENZONE,        CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },                                        \
        { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS), RES_PARATR_HYPHENZONE,         CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },                              \
        { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS), RES_PARATR_HYPHENZONE,        CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },                              \
        { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS), RES_PARATR_HYPHENZONE,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},                              \
        { SW_PROP_NMID(UNO_NAME_BREAK_TYPE), RES_BREAK,                 CPPU_E2T(CPPUTYPE_BREAK),           PropertyAttribute::MAYBEVOID, 0},                                                          \
        { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR         },                                           \
        { SW_PROP_NMID(UNO_NAME_PARA_BACK_COLOR), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR         },                                           \
        { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, 0},                                                            \
        { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, 0},                                                           \
        { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},                                              \
        { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PropertyAttribute::MAYBEVOID, MID_CROSSED_OUT},                                                       \
        { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PropertyAttribute::MAYBEVOID, MID_ESC           },                                                   \
        { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PropertyAttribute::MAYBEVOID, MID_ESC_HEIGHT},                                               \
        { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,            PropertyAttribute::MAYBEVOID, MID_AUTO_ESC  },                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),              PropertyAttribute::MAYBEVOID, MID_UNDERLINE},                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PropertyAttribute::MAYBEVOID, MID_UL_COLOR},                                                \
        { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PropertyAttribute::MAYBEVOID, MID_UL_HASCOLOR},                                                \
        { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING),        PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },                                          \
        { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING),        PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },                                       \
        { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},                                \
        { SW_PROP_NMID(UNO_NAME_PARA_LEFT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN|CONVERT_TWIPS},                                   \
        { SW_PROP_NMID(UNO_NAME_PARA_RIGHT_MARGIN), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_BOOLEAN),      PropertyAttribute::MAYBEVOID, MID_FIRST_AUTO},                                      \
        { SW_PROP_NMID(UNO_NAME_PARA_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32),      PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},                         \
        _STANDARD_FONT_PROPERTIES \
        _CJK_FONT_PROPERTIES \
        _CTL_FONT_PROPERTIES \
        { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PropertyAttribute::MAYBEVOID,   CONVERT_TWIPS},                                                           \
        { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},                                                                     \
        { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_DROP_CAP_FORMAT), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_DROPCAPFMT)  , PropertyAttribute::MAYBEVOID, MID_DROPCAP_FORMAT|CONVERT_TWIPS},                        \
        { SW_PROP_NMID(UNO_NAME_DROP_CAP_WHOLE_WORD), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PropertyAttribute::MAYBEVOID, MID_DROPCAP_WHOLE_WORD },                                               \
        { SW_PROP_NMID(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_KEEP_TOGETHER), RES_KEEP,              CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_PARA_SPLIT), RES_PARATR_SPLIT,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_PARA_WIDOWS), RES_PARATR_WIDOWS,        CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},                                                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_ORPHANS), RES_PARATR_ORPHANS,      CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,          CPPU_E2T(CPPUTYPE_OUSTRING),            PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },                                    \
        { SW_PROP_NMID(UNO_NAME_PAGE_NUMBER_OFFSET), RES_PAGEDESC,              CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},                                   \
        { SW_PROP_NMID(UNO_NAME_PARA_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),         PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST},                                              \
        { SW_PROP_NMID(UNO_NAME_PARA_EXPAND_SINGLE_WORD), RES_PARATR_ADJUST,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, MID_EXPAND_SINGLE   },                                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_LAST_LINE_ADJUST), RES_PARATR_ADJUST, CPPU_E2T(CPPUTYPE_INT16),                PropertyAttribute::MAYBEVOID, MID_LAST_LINE_ADJUST},                                         \
        { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_COUNT), RES_LINENUMBER,    CPPU_E2T(CPPUTYPE_BOOLEAN),             PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_COUNT      },                                                \
        { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_START_VALUE), RES_LINENUMBER, CPPU_E2T(CPPUTYPE_INT32),                    PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_STARTVALUE},                                    \
        { SW_PROP_NMID(UNO_NAME_PARA_LINE_SPACING), RES_PARATR_LINESPACING, CPPU_E2T(CPPUTYPE_LINESPACE),       PropertyAttribute::MAYBEVOID,     CONVERT_TWIPS},                                      \
        { SW_PROP_NMID(UNO_NAME_PARA_REGISTER_MODE_ACTIVE), RES_PARATR_REGISTER,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,           PropertyAttribute::MAYBEVOID, 0},                                                                 \
        { SW_PROP_NMID(UNO_NAME_PARA_TOP_MARGIN), RES_UL_SPACE,             CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_UP_MARGIN|CONVERT_TWIPS},                                  \
        { SW_PROP_NMID(UNO_NAME_PARA_BOTTOM_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_LO_MARGIN|CONVERT_TWIPS},                                 \
        { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND, CPPU_E2T(CPPUTYPE_BOOLEAN),              PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT        },                                                \
        { SW_PROP_NMID(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND, CPPU_E2T(CPPUTYPE_BOOLEAN),                 PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT        },                                                \
        { SW_PROP_NMID(UNO_NAME_NUMBERING_STYLE_NAME), RES_PARATR_NUMRULE,  CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,   0},                                                           \
        { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,          PropertyAttribute::MAYBEVOID,     0},                                                             \
        { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),      PropertyAttribute::MAYBEVOID, LEFT_BORDER  |CONVERT_TWIPS },                            \
        { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),      PropertyAttribute::MAYBEVOID, RIGHT_BORDER |CONVERT_TWIPS },                                \
        { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),      PropertyAttribute::MAYBEVOID, TOP_BORDER   |CONVERT_TWIPS },                            \
        { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),      PropertyAttribute::MAYBEVOID, BOTTOM_BORDER|CONVERT_TWIPS },                                \
        { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },                               \
        { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),        PropertyAttribute::MAYBEVOID, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },                                                            \
        { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),        PropertyAttribute::MAYBEVOID, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },                                                            \
        { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),    PropertyAttribute::MAYBEVOID, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },                                                        \
        { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),    PropertyAttribute::MAYBEVOID, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },                                                        \
        { SW_PROP_NMID(UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },                                   \
        { SW_PROP_NMID(UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES), RES_TXTATR_UNKNOWN_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },                               \
        { SW_PROP_NMID(UNO_NAME_PARA_SHADOW_FORMAT), RES_SHADOW,            CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS}, \
        { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, MID_TWOLINES}, \
        { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_START_BRACKET}, \
        { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_END_BRACKET}, \
        { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_EMPHASIS},\
        { SW_PROP_NMID(UNO_NAME_PARA_IS_HANGING_PUNCTUATION), RES_PARATR_HANGINGPUNCTUATION,  CPPU_E2T(CPPUTYPE_BOOLEAN),    PropertyAttribute::MAYBEVOID ,0  },    \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_CHARACTER_DISTANCE), RES_PARATR_SCRIPTSPACE,         CPPU_E2T(CPPUTYPE_BOOLEAN),    PropertyAttribute::MAYBEVOID ,0  },    \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_FORBIDDEN_RULES), RES_PARATR_FORBIDDEN_RULES,     CPPU_E2T(CPPUTYPE_BOOLEAN),    PropertyAttribute::MAYBEVOID ,0  }, \
        { SW_PROP_NMID(UNO_NAME_PARA_VERT_ALIGNMENT), RES_PARATR_VERTALIGN,CPPU_E2T(CPPUTYPE_INT16),    PropertyAttribute::MAYBEVOID,     0      },  \
        { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,     MID_ROTATE      },  \
        { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID,         MID_FITTOLINE  },  \
        { SW_PROP_NMID(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,          0 },  \
        { SW_PROP_NMID(UNO_NAME_RUBY_TEXT), RES_TXTATR_CJK_RUBY,    CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID,          MID_RUBY_TEXT },  \
        { SW_PROP_NMID(UNO_NAME_RUBY_ADJUST), RES_TXTATR_CJK_RUBY,  CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,          MID_RUBY_ADJUST },  \
        { SW_PROP_NMID(UNO_NAME_RUBY_CHAR_STYLE_NAME), RES_TXTATR_CJK_RUBY, CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID,          MID_RUBY_CHARSTYLE },  \
        { SW_PROP_NMID(UNO_NAME_RUBY_IS_ABOVE), RES_TXTATR_CJK_RUBY,    CPPU_E2T(CPPUTYPE_BOOLEAN),  PropertyAttribute::MAYBEVOID,         MID_RUBY_ABOVE },  \
        { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PropertyAttribute::MAYBEVOID,      MID_RELIEF }, \
        { SW_PROP_NMID(UNO_NAME_SNAP_TO_GRID), RES_PARATR_SNAPTOGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID, 0 }, \
        { SW_PROP_NMID(UNO_NAME_PARA_IS_CONNECT_BORDER), RES_PARATR_CONNECT_BORDER, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID, 0}, \
        { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },\


#define TABSTOPS_MAP_ENTRY                { SW_PROP_NMID(UNO_NAME_TABSTOPS), RES_PARATR_TABSTOP,   CPPU_E2T(CPPUTYPE_SEQTABSTOP),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},

#define COMMON_CRSR_PARA_PROPERTIES \
        COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN \
        COMMON_HYPERLINK_PROPERTIES \
        { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},\
        { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAMES), FN_UNO_CHARFMT_SEQUENCE,  CPPU_E2T(CPPUTYPE_OUSTRINGS),     PropertyAttribute::MAYBEVOID,     0},


#define COMMON_CRSR_PARA_PROPERTIES_2 \
        COMMON_CRSR_PARA_PROPERTIES_FN_ONLY \
        COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN

#define  COMPLETE_TEXT_CURSOR_MAP\
        COMMON_CRSR_PARA_PROPERTIES\
        { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX_MARK), FN_UNO_DOCUMENT_INDEX_MARK, CPPU_E2T(CPPUTYPE_REFDOCIDXMRK), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { SW_PROP_NMID(UNO_NAME_TEXT_FIELD), FN_UNO_TEXT_FIELD,      CPPU_E2T(CPPUTYPE_REFTXTFIELD),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { SW_PROP_NMID(UNO_NAME_REFERENCE_MARK), FN_UNO_REFERENCE_MARK,  CPPU_E2T(CPPUTYPE_REFTEXTCNTNT), PropertyAttribute::MAYBEVOID ,0 },\
        { SW_PROP_NMID(UNO_NAME_FOOTNOTE), FN_UNO_FOOTNOTE,        CPPU_E2T(CPPUTYPE_REFFOOTNOTE),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { SW_PROP_NMID(UNO_NAME_ENDNOTE), FN_UNO_ENDNOTE,         CPPU_E2T(CPPUTYPE_REFFOOTNOTE),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },\
        { SW_PROP_NMID(UNO_NAME_HYPER_LINK_EVENTS), RES_TXTATR_INETFMT,     CPPU_E2T(CPPUTYPE_REFNMREPLACE), PropertyAttribute::MAYBEVOID, MID_URL_HYPERLINKEVENTS},\
        TABSTOPS_MAP_ENTRY



#define _BASE_INDEX_PROPERTIES_\
        { SW_PROP_NMID(UNO_NAME_TITLE), WID_IDX_TITLE,  CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},\
        { SW_PROP_NMID(UNO_NAME_NAME),  WID_IDX_NAME,   CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},\
        { SW_PROP_NMID(UNO_NAME_CONTENT_SECTION), WID_IDX_CONTENT_SECTION,  CPPU_E2T(CPPUTYPE_REFTEXTSECTION)  , PropertyAttribute::READONLY,     0},\
        { SW_PROP_NMID(UNO_NAME_HEADER_SECTION), WID_IDX_HEADER_SECTION,  CPPU_E2T(CPPUTYPE_REFTEXTSECTION)  , PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,     0},\

#define ANCHOR_TYPES_PROPERTY    { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPES), FN_UNO_ANCHOR_TYPES,    CPPU_E2T(CPPUTYPE_SEQANCHORTYPE),PropertyAttribute::READONLY, 0xff},

#define COMMON_FRAME_PROPERTIES \
    { SW_PROP_NMID(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, MID_ANCHOR_PAGENUM       },              \
    { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           CPPU_E2T(CPPUTYPE_TXTCNTANCHOR),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},             \
    { SW_PROP_NMID(UNO_NAME_ANCHOR_FRAME), RES_ANCHOR,             CPPU_E2T(CPPUTYPE_REFTEXTFRAME),    PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORFRAME},             \
    ANCHOR_TYPES_PROPERTY\
    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },                      \
    { SW_PROP_NMID(UNO_NAME_BACK_COLOR_R_G_B), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE ,MID_BACK_COLOR_R_G_B},    \
    { SW_PROP_NMID(UNO_NAME_BACK_COLOR_TRANSPARENCY), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE ,MID_BACK_COLOR_TRANSPARENCY},    \
    { SW_PROP_NMID(UNO_NAME_CONTENT_PROTECTED), RES_PROTECT,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_CONTENT  },                          \
    { SW_PROP_NMID(UNO_NAME_FRAME_STYLE_NAME), FN_UNO_FRAME_STYLE_NAME,CPPU_E2T(CPPUTYPE_OUSTRING),         PROPERTY_NONE, 0},                                   \
    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE ,MID_GRAPHIC_URL    },                 \
    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE ,MID_GRAPHIC_FILTER    },              \
    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},    \
    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_TRANSPARENCY), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENCY},    \
    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},            \
    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},            \
    { SW_PROP_NMID(UNO_NAME_WIDTH), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS},\
    { SW_PROP_NMID(UNO_NAME_HEIGHT), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS},\
    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },             \
    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    }, \
    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_RELATION  },               \
    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_U_R_L), RES_URL,                 CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE ,MID_URL_URL},                         \
    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_TARGET), RES_URL,                CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE ,MID_URL_TARGET},                      \
    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_NAME), RES_URL,              CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },                 \
    { SW_PROP_NMID(UNO_NAME_OPAQUE), RES_OPAQUE,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},                                        \
    { SW_PROP_NMID(UNO_NAME_PAGE_TOGGLE), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },               \
    { SW_PROP_NMID(UNO_NAME_POSITION_PROTECTED), RES_PROTECT,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_POSITION},                      \
    { SW_PROP_NMID(UNO_NAME_PRINT), RES_PRINT,              CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},                                        \
    { SW_PROP_NMID(UNO_NAME_RELATIVE_HEIGHT), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },            \
    { SW_PROP_NMID(UNO_NAME_RELATIVE_WIDTH), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },         \
    { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},             \
    { SW_PROP_NMID(UNO_NAME_IMAGE_MAP), RES_URL,                    CPPU_E2T(CPPUTYPE_REFIDXCNTNR), PROPERTY_NONE, MID_URL_CLIENTMAP}, \
    { SW_PROP_NMID(UNO_NAME_SERVER_MAP), RES_URL,                CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE ,MID_URL_SERVERMAP   },                      \
    { SW_PROP_NMID(UNO_NAME_SIZE), RES_FRM_SIZE,            CPPU_E2T(CPPUTYPE_AWTSIZE),             PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},        \
    { SW_PROP_NMID(UNO_NAME_SIZE_PROTECTED), RES_PROTECT,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_SIZE    },                      \
    { SW_PROP_NMID(UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },  \
    { SW_PROP_NMID(UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },     \
    { SW_PROP_NMID(UNO_NAME_TEXT_WRAP), RES_SURROUND,           CPPU_E2T(CPPUTYPE_WRAPTXTMODE),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },        \
    { SW_PROP_NMID(UNO_NAME_SURROUND), RES_SURROUND,          CPPU_E2T(CPPUTYPE_WRAPTXTMODE),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE },        \
    { SW_PROP_NMID(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },                 \
    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},          \
    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},          \
    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },                    \
    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },             \
    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    }, \
    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_RELATION  },               \
    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },             \
    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },                 \
    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },             \
    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },                 \
    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BORDER_DISTANCE|CONVERT_TWIPS },                       \
    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },                \
    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },                \
    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },            \
    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },            \
    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},                \
    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },\
    { SW_PROP_NMID(UNO_NAME_Z_ORDER), FN_UNO_Z_ORDER,           CPPU_E2T(CPPUTYPE_INT32),       PROPERTY_NONE, 0},



#define COMMON_TEXT_CONTENT_PROPERTIES \
        { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPE), FN_UNO_ANCHOR_TYPE, CPPU_E2T(CPPUTYPE_TXTCNTANCHOR),              PropertyAttribute::READONLY, MID_ANCHOR_ANCHORTYPE},\
        ANCHOR_TYPES_PROPERTY\
        { SW_PROP_NMID(UNO_NAME_TEXT_WRAP), FN_UNO_TEXT_WRAP,   CPPU_E2T(CPPUTYPE_WRAPTXTMODE),                 PropertyAttribute::READONLY, MID_SURROUND_SURROUNDTYPE  },


#define     _PROP_DIFF_FONTHEIGHT \
                    { SW_PROP_NMID(UNO_NAME_CHAR_PROP_HEIGHT), RES_CHRATR_FONTSIZE ,            CPPU_E2T(CPPUTYPE_FLOAT),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_DIFF_HEIGHT), RES_CHRATR_FONTSIZE ,            CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_PROP_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE ,          CPPU_E2T(CPPUTYPE_FLOAT),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_DIFF_HEIGHT_ASIAN), RES_CHRATR_CJK_FONTSIZE ,          CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_PROP_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE ,            CPPU_E2T(CPPUTYPE_FLOAT),                                           PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NMID(UNO_NAME_CHAR_DIFF_HEIGHT_COMPLEX), RES_CHRATR_CTL_FONTSIZE ,            CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF|CONVERT_TWIPS},



const SfxItemPropertyMap* SwUnoPropertyMapProvider::GetPropertyMap(sal_uInt16 nPropertyId)
{
    DBG_ASSERT(nPropertyId < PROPERTY_MAP_END, "Id ?" )
    if( !aMapArr[ nPropertyId ] )
    {
        switch(nPropertyId)
        {
            case PROPERTY_MAP_TEXT_CURSOR:
            {
                static SfxItemPropertyMap aCharAndParaMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aCharAndParaMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH:
            {
                static SfxItemPropertyMap aParagraphMap_Impl[] =
                {
                    COMMON_CRSR_PARA_PROPERTIES_2
                    TABSTOPS_MAP_ENTRY
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aParagraphMap_Impl;
            }
            break;
            case PROPERTY_MAP_CHAR_STYLE :
            {
                static SfxItemPropertyMap aCharStyleMap [] =
                {
                    { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE, MID_ESC          },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PROPERTY_NONE, MID_ESC_HEIGHT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PROPERTY_NONE, MID_UNDERLINE},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE, MID_UL_COLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE, MID_UL_HASCOLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,  CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,    PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, MID_TWOLINES},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_START_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_END_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, MID_EMPHASIS},
                    _PROP_DIFF_FONTHEIGHT
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,    MID_ROTATE      },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE,        MID_FITTOLINE  },
                    { SW_PROP_NMID(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,         0 },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE,      MID_RELIEF },
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aCharStyleMap;
            }
            break;
            case PROPERTY_MAP_PARA_STYLE :
            {
                static SfxItemPropertyMap aParaStyleMap [] =
                {
                    { SW_PROP_NMID(UNO_NAME_BREAK_TYPE), RES_BREAK,                 CPPU_E2T(CPPUTYPE_BREAK),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,          CPPU_E2T(CPPUTYPE_OUSTRING),            PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },
                    { SW_PROP_NMID(UNO_NAME_PAGE_NUMBER_OFFSET), RES_PAGEDESC,              CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND, CPPU_E2T(CPPUTYPE_BOOLEAN),          PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_PARA_BACK_COLOR), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE, MID_ESC          },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PROPERTY_NONE, MID_ESC_HEIGHT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PROPERTY_NONE, MID_UNDERLINE},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE, MID_UL_COLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE, MID_UL_HASCOLOR},
                    { SW_PROP_NMID(UNO_NAME_PARA_LEFT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_TXT_LMARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_RIGHT_MARGIN), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_LEFT_MARGIN_RELATIVE), RES_LR_SPACE,          CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,        MID_L_REL_MARGIN},
                    { SW_PROP_NMID(UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE), RES_LR_SPACE,         CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,        MID_R_REL_MARGIN},
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT), RES_LR_SPACE,          CPPU_E2T(CPPUTYPE_BOOLEAN),      PROPERTY_NONE, MID_FIRST_AUTO},
                    { SW_PROP_NMID(UNO_NAME_PARA_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE), RES_LR_SPACE,         CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_FIRST_LINE_REL_INDENT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,  CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_FORMAT), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_DROPCAPFMT)  , PROPERTY_NONE, MID_DROPCAP_FORMAT|CONVERT_TWIPS     },
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_WHOLE_WORD), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PROPERTY_NONE, MID_DROPCAP_WHOLE_WORD },
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_PARA_KEEP_TOGETHER), RES_KEEP,              CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_SPLIT), RES_PARATR_SPLIT,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_WIDOWS), RES_PARATR_WIDOWS,        CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_ORPHANS), RES_PARATR_ORPHANS,      CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_EXPAND_SINGLE_WORD), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PROPERTY_NONE, MID_EXPAND_SINGLE   },
                    { SW_PROP_NMID(UNO_NAME_PARA_LAST_LINE_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),       PROPERTY_NONE, MID_LAST_LINE_ADJUST},
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_COUNT), RES_LINENUMBER,        CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_LINENUMBER_COUNT     },
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_START_VALUE), RES_LINENUMBER,      CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_LINENUMBER_STARTVALUE},
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_SPACING), RES_PARATR_LINESPACING, CPPU_E2T(CPPUTYPE_LINESPACE),PROPERTY_NONE,     CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),         PROPERTY_NONE, MID_PARA_ADJUST},
                    { SW_PROP_NMID(UNO_NAME_PARA_REGISTER_MODE_ACTIVE), RES_PARATR_REGISTER,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_PARA_TOP_MARGIN), RES_UL_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_BOTTOM_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_TOP_MARGIN_RELATIVE), RES_UL_SPACE,        CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, MID_UP_REL_MARGIN},
                    { SW_PROP_NMID(UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE), RES_UL_SPACE,         CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, MID_LO_REL_MARGIN},
                    TABSTOPS_MAP_ENTRY
                    { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,    PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_HYPHENATION), RES_PARATR_HYPHENZONE,      CPPU_E2T(CPPUTYPE_BOOLEAN),  PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS), RES_PARATR_HYPHENZONE,         CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS), RES_PARATR_HYPHENZONE,        CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS), RES_PARATR_HYPHENZONE,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_STYLE_NAME), RES_PARATR_NUMRULE,  CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,   0},
                    { SW_PROP_NMID(UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_PARA_SHADOW_FORMAT), RES_SHADOW,    CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, MID_TWOLINES},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_START_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, MID_END_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, MID_EMPHASIS},
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_HANGING_PUNCTUATION), RES_PARATR_HANGINGPUNCTUATION,  CPPU_E2T(CPPUTYPE_BOOLEAN),   PROPERTY_NONE ,0     },
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_CHARACTER_DISTANCE), RES_PARATR_SCRIPTSPACE,         CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE ,0     },
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_FORBIDDEN_RULES), RES_PARATR_FORBIDDEN_RULES,     CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE ,0    },
                    { SW_PROP_NMID(UNO_NAME_PARA_VERT_ALIGNMENT), RES_PARATR_VERTALIGN,             CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE , 0  },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,    MID_ROTATE      },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE,        MID_FITTOLINE  },
                    { SW_PROP_NMID(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,         0 },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE,      MID_RELIEF },

                    _PROP_DIFF_FONTHEIGHT
                    { SW_PROP_NMID(UNO_NAME_FOLLOW_STYLE), FN_UNO_FOLLOW_STYLE,     CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_IS_AUTO_UPDATE), FN_UNO_IS_AUTO_UPDATE, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_CATEGORY), FN_UNO_CATEGORY, CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE , 0 },
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_CONNECT_BORDER), RES_PARATR_CONNECT_BORDER, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aParaStyleMap;
            }
            break;
            case PROPERTY_MAP_FRAME_STYLE:
            {
                static SfxItemPropertyMap aFrameStyleMap   [] =
                {
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE, MID_ANCHOR_PAGENUM       },
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           CPPU_E2T(CPPUTYPE_TXTCNTANCHOR),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR_R_G_B), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE ,MID_BACK_COLOR_R_G_B},    \
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR_TRANSPARENCY), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE ,MID_BACK_COLOR_TRANSPARENCY},    \
                //  { SW_PROP_NMID(UNO_NAME_CHAIN_NEXT_NAME), RES_CHAIN,                CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE ,MID_CHAIN_NEXTNAME},
                //  { SW_PROP_NMID(UNO_NAME_CHAIN_PREV_NAME), RES_CHAIN,                CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE ,MID_CHAIN_PREVNAME},
                /*not impl*/    { SW_PROP_NMID(UNO_NAME_CLIENT_MAP), RES_URL,               CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_URL_CLIENTMAP         },
                    { SW_PROP_NMID(UNO_NAME_CONTENT_PROTECTED), RES_PROTECT,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_CONTENT   },
                    { SW_PROP_NMID(UNO_NAME_EDIT_IN_READONLY), RES_EDIT_IN_READONLY,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                //  { SW_PROP_NMID(UNO_NAME_GRAPHIC), RES_BACKGROUND,       &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    },
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_RELATION  },
                    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_U_R_L), RES_URL,                 CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_URL_URL},
                    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_TARGET), RES_URL,                CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_URL_TARGET},
                    { SW_PROP_NMID(UNO_NAME_HYPER_LINK_NAME), RES_URL,              CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
                    { SW_PROP_NMID(UNO_NAME_OPAQUE), RES_OPAQUE,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_PAGE_TOGGLE), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
                    { SW_PROP_NMID(UNO_NAME_POSITION_PROTECTED), RES_PROTECT,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_POSITION},
                    { SW_PROP_NMID(UNO_NAME_PRINT), RES_PRINT,              CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_WIDTH), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
                    { SW_PROP_NMID(UNO_NAME_HEIGHT), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NMID(UNO_NAME_RELATIVE_HEIGHT), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },
                    { SW_PROP_NMID(UNO_NAME_RELATIVE_WIDTH), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },
                    { SW_PROP_NMID(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { SW_PROP_NMID(UNO_NAME_SIZE), RES_FRM_SIZE,            CPPU_E2T(CPPUTYPE_AWTSIZE),             PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },
                    { SW_PROP_NMID(UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH), RES_FRM_SIZE,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },
                //  { SW_PROP_NMID(UNO_NAME_WIDTH), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_WIDTH            },
                    { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_SERVER_MAP), RES_URL,               CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_URL_SERVERMAP         },
                    { SW_PROP_NMID(UNO_NAME_SIZE_PROTECTED), RES_PROTECT,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_PROTECT_SIZE    },
                    //Surround bleibt, weil es mit der 5.1 ausgeliefert wurde, obwohl es mit text::WrapTextMode identisch ist
                    { SW_PROP_NMID(UNO_NAME_SURROUND), RES_SURROUND,          CPPU_E2T(CPPUTYPE_WRAPTXTMODE),    PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE },
                    { SW_PROP_NMID(UNO_NAME_TEXT_WRAP), RES_SURROUND,           CPPU_E2T(CPPUTYPE_WRAPTXTMODE),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { SW_PROP_NMID(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },
                    { SW_PROP_NMID(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_RELATION  },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,    CPPU_E2T(CPPUTYPE_INT32), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_IS_AUTO_UPDATE), FN_UNO_IS_AUTO_UPDATE, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aFrameStyleMap;
            }
            break;
            case PROPERTY_MAP_PAGE_STYLE :
            {
                static SfxItemPropertyMap aPageStyleMap [] =
                {
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NMID(UNO_NAME_GRAPHIC), RES_BACKGROUND,       &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,    CPPU_E2T(CPPUTYPE_INT32), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},

                    { SW_PROP_NMID(UNO_NAME_HEADER_BACK_COLOR), FN_UNO_HEADER_BACKGROUND,   CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NMID(UNO_NAME_HEADER_GRAPHIC), FN_UNO_HEADER_BACKGROUND,  &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_HEADER_GRAPHIC_URL), FN_UNO_HEADER_BACKGROUND,          CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_HEADER_GRAPHIC_FILTER), FN_UNO_HEADER_BACKGROUND,           CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_HEADER_GRAPHIC_LOCATION), FN_UNO_HEADER_BACKGROUND,     CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_HEADER_LEFT_MARGIN), FN_UNO_HEADER_LR_SPACE,    CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEADER_RIGHT_MARGIN), FN_UNO_HEADER_LR_SPACE,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEADER_BACK_TRANSPARENT), FN_UNO_HEADER_BACKGROUND,     CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_HEADER_LEFT_BORDER), FN_UNO_HEADER_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_RIGHT_BORDER), FN_UNO_HEADER_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_TOP_BORDER), FN_UNO_HEADER_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_BOTTOM_BORDER), FN_UNO_HEADER_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_BORDER_DISTANCE), FN_UNO_HEADER_BOX,    CPPU_E2T(CPPUTYPE_INT32),    0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_LEFT_BORDER_DISTANCE), FN_UNO_HEADER_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_RIGHT_BORDER_DISTANCE), FN_UNO_HEADER_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_TOP_BORDER_DISTANCE), FN_UNO_HEADER_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_BOTTOM_BORDER_DISTANCE), FN_UNO_HEADER_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HEADER_SHADOW_FORMAT), FN_UNO_HEADER_SHADOW,        CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEADER_BODY_DISTANCE), FN_UNO_HEADER_BODY_DISTANCE,CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE ,MID_LO_MARGIN|CONVERT_TWIPS       },
                    { SW_PROP_NMID(UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT), FN_UNO_HEADER_IS_DYNAMIC_DISTANCE,CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_HEADER_IS_SHARED), FN_UNO_HEADER_SHARE_CONTENT,CPPU_E2T(CPPUTYPE_BOOLEAN),          PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_HEADER_HEIGHT), FN_UNO_HEADER_HEIGHT,       CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NMID(UNO_NAME_HEADER_IS_ON), FN_UNO_HEADER_ON,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_HEADER_DYNAMIC_SPACING), FN_UNO_HEADER_EAT_SPACING,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,0         },

                    { SW_PROP_NMID(UNO_NAME_FOOTER_BACK_COLOR), FN_UNO_FOOTER_BACKGROUND,   CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NMID(UNO_NAME_FOOTER_GRAPHIC), FN_UNO_FOOTER_BACKGROUND,      &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_FOOTER_GRAPHIC_URL), FN_UNO_FOOTER_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_GRAPHIC_FILTER), FN_UNO_FOOTER_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_GRAPHIC_LOCATION), FN_UNO_FOOTER_BACKGROUND,     CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_LEFT_MARGIN), FN_UNO_FOOTER_LR_SPACE,    CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_RIGHT_MARGIN), FN_UNO_FOOTER_LR_SPACE,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BACK_TRANSPARENT), FN_UNO_FOOTER_BACKGROUND,     CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_LEFT_BORDER), FN_UNO_FOOTER_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_RIGHT_BORDER), FN_UNO_FOOTER_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TOP_BORDER), FN_UNO_FOOTER_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BOTTOM_BORDER), FN_UNO_FOOTER_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,    CPPU_E2T(CPPUTYPE_INT32),    0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_LEFT_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_RIGHT_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TOP_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BOTTOM_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_SHADOW_FORMAT), FN_UNO_FOOTER_SHADOW,        CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_BODY_DISTANCE), FN_UNO_FOOTER_BODY_DISTANCE,CPPU_E2T(CPPUTYPE_INT32),            PROPERTY_NONE ,MID_UP_MARGIN|CONVERT_TWIPS       },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_IS_DYNAMIC_HEIGHT), FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE,CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_IS_SHARED), FN_UNO_FOOTER_SHARE_CONTENT,CPPU_E2T(CPPUTYPE_BOOLEAN),          PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_HEIGHT), FN_UNO_FOOTER_HEIGHT,       CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_IS_ON), FN_UNO_FOOTER_ON,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_FOOTER_DYNAMIC_SPACING), FN_UNO_FOOTER_EAT_SPACING,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,0         },


                    { SW_PROP_NMID(UNO_NAME_IS_LANDSCAPE), SID_ATTR_PAGE,           CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE ,MID_PAGE_ORIENTATION   },
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE), SID_ATTR_PAGE,             CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE , MID_PAGE_NUMTYPE       },
                    { SW_PROP_NMID(UNO_NAME_PAGE_STYLE_LAYOUT), SID_ATTR_PAGE,          CPPU_E2T(CPPUTYPE_PAGESTYLELAY),    PROPERTY_NONE ,MID_PAGE_LAYOUT     },
                    { SW_PROP_NMID(UNO_NAME_PRINTER_PAPER_TRAY), RES_PAPER_BIN,             CPPU_E2T(CPPUTYPE_OUSTRING),            PROPERTY_NONE , 0 },
//                  { SW_PROP_NMID(UNO_NAME_REGISTER_MODE_ACTIVE), SID_SWREGISTER_MODE,     CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE , 0 },
                    { SW_PROP_NMID(UNO_NAME_REGISTER_PARAGRAPH_STYLE), SID_SWREGISTER_COLLECTION,   CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE , 0 },
                    { SW_PROP_NMID(UNO_NAME_SIZE), SID_ATTR_PAGE_SIZE,  CPPU_E2T(CPPUTYPE_AWTSIZE),             PROPERTY_NONE,   MID_SIZE_SIZE|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_WIDTH), SID_ATTR_PAGE_SIZE,     CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_SIZE_WIDTH|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEIGHT), SID_ATTR_PAGE_SIZE,    CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_SIZE_HEIGHT|CONVERT_TWIPS            },
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HEADER_TEXT), FN_UNO_HEADER,        CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_HEADER_TEXT_LEFT), FN_UNO_HEADER_LEFT,     CPPU_E2T(CPPUTYPE_REFTEXT),          PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_HEADER_TEXT_RIGHT), FN_UNO_HEADER_RIGHT,    CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TEXT), FN_UNO_FOOTER,        CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TEXT_LEFT), FN_UNO_FOOTER_LEFT,  CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTER_TEXT_RIGHT), FN_UNO_FOOTER_RIGHT,    CPPU_E2T(CPPUTYPE_REFTEXT),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOLLOW_STYLE), FN_UNO_FOLLOW_STYLE,     CPPU_E2T(CPPUTYPE_OUSTRING),        PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_HEIGHT), FN_PARAM_FTN_INFO,        CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE ,       MID_FTN_HEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_WEIGHT), FN_PARAM_FTN_INFO,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE ,     MID_LINE_WEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_COLOR), FN_PARAM_FTN_INFO,        CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE ,     MID_LINE_COLOR},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_RELATIVE_WIDTH), FN_PARAM_FTN_INFO,       CPPU_E2T(CPPUTYPE_INT8),        PROPERTY_NONE ,     MID_LINE_RELWIDTH    },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_ADJUST), FN_PARAM_FTN_INFO,       CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE ,     MID_LINE_ADJUST     },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_TEXT_DISTANCE), FN_PARAM_FTN_INFO,        CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE ,     MID_LINE_TEXT_DIST   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_LINE_DISTANCE), FN_PARAM_FTN_INFO,         CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE ,     MID_LINE_FOOTNOTE_DIST|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                        // writing grid
                    { SW_PROP_NMID(UNO_NAME_GRID_COLOR), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_GRID_COLOR},
                    { SW_PROP_NMID(UNO_NAME_GRID_LINES), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, MID_GRID_LINES},
                    { SW_PROP_NMID(UNO_NAME_GRID_BASE_HEIGHT), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_GRID_BASEHEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_GRID_RUBY_HEIGHT), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_GRID_RUBYHEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_GRID_MODE), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, MID_GRID_TYPE},
                    { SW_PROP_NMID(UNO_NAME_GRID_RUBY_BELOW), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_RUBY_BELOW},
                    { SW_PROP_NMID(UNO_NAME_GRID_PRINT), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_PRINT},
                    { SW_PROP_NMID(UNO_NAME_GRID_DISPLAY), RES_TEXTGRID, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_GRID_DISPLAY},

                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aPageStyleMap;
            }
            break;
            case PROPERTY_MAP_NUM_STYLE  :
            {
                static SfxItemPropertyMap aNumStyleMap      [] =
                {
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_RULES), FN_UNO_NUM_RULES, CPPU_E2T(CPPUTYPE_REFIDXREPL), PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_IS_PHYSICAL), FN_UNO_IS_PHYSICAL,     CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_DISPLAY_NAME), FN_UNO_DISPLAY_NAME, CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aNumStyleMap;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE :
            {
                static SfxItemPropertyMap aTablePropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,        CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE,MID_BACK_COLOR         },
                    { SW_PROP_NMID(UNO_NAME_BREAK_TYPE), RES_BREAK,                 CPPU_E2T(CPPUTYPE_BREAK),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
                    { SW_PROP_NMID(UNO_NAME_KEEP_TOGETHER), RES_KEEP,               CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SPLIT), RES_LAYOUT_SPLIT,       CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_PAGE_NUMBER_OFFSET), RES_PAGEDESC,              CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},
                    { SW_PROP_NMID(UNO_NAME_PAGE_DESC_NAME), RES_PAGEDESC,           CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID, 0xff},
                    { SW_PROP_NMID(UNO_NAME_RELATIVE_WIDTH), FN_TABLE_RELATIVE_WIDTH,CPPU_E2T(CPPUTYPE_INT16)  ,        PROPERTY_NONE, 0xff },
                    { SW_PROP_NMID(UNO_NAME_REPEAT_HEADLINE), FN_TABLE_HEADLINE_REPEAT,CPPU_E2T(CPPUTYPE_BOOLEAN),      PROPERTY_NONE, 0xff},
                    { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_WIDTH), FN_TABLE_WIDTH,         CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, 0xff},
                    { SW_PROP_NMID(UNO_NAME_IS_WIDTH_RELATIVE), FN_TABLE_IS_RELATIVE_WIDTH,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE, 0xff},
                    { SW_PROP_NMID(UNO_NAME_CHART_ROW_AS_LABEL), FN_UNO_RANGE_ROW_LABEL,            CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE,  0},
                    { SW_PROP_NMID(UNO_NAME_CHART_COLUMN_AS_LABEL), FN_UNO_RANGE_COL_LABEL,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TABLE_BORDER), FN_UNO_TABLE_BORDER,         CPPU_E2T(CPPUTYPE_TABLEBORDER), PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TABLE_COLUMN_SEPARATORS), FN_UNO_TABLE_COLUMN_SEPARATORS,   CPPU_E2T(CPPUTYPE_TBLCOLSEP),   PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_TABLE_COLUMN_RELATIVE_SUM), FN_UNO_TABLE_COLUMN_RELATIVE_SUM,       CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::READONLY, 0 },
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, CPPU_E2T(CPPUTYPE_REFTEXTSECTION),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0,0}
                };

                aMapArr[nPropertyId] = aTablePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_CELL :
            {
                static SfxItemPropertyMap aCellMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE , MID_BACK_COLOR       },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,  CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE , MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,    CPPU_E2T(CPPUTYPE_BORDERLINE),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,    CPPU_E2T(CPPUTYPE_BORDERLINE),   0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,    CPPU_E2T(CPPUTYPE_BORDERLINE), 0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,    CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,    CPPU_E2T(CPPUTYPE_INT32), 0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_TEXT_SECTION), FN_UNO_TEXT_SECTION, CPPU_E2T(CPPUTYPE_REFTEXTSECTION),  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), RES_PROTECT,            CPPU_E2T(CPPUTYPE_BOOLEAN), 0, MID_PROTECT_CONTENT},
                    { SW_PROP_NMID(UNO_NAME_CELL_NAME), FN_UNO_CELL_NAME,            CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY,0},
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aCellMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_RANGE:
            {
                static SfxItemPropertyMap aRangePropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLE_NAME), FN_UNO_PARA_STYLE,        CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,      CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHART_ROW_AS_LABEL), FN_UNO_RANGE_ROW_LABEL,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID,  0},
                    { SW_PROP_NMID(UNO_NAME_CHART_COLUMN_AS_LABEL), FN_UNO_RANGE_COL_LABEL, CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), FN_UNO_TABLE_CELL_BACKGROUND,   CPPU_E2T(CPPUTYPE_GRAPHICLOC), PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), FN_UNO_TABLE_CELL_BACKGROUND,  CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), FN_UNO_TABLE_CELL_BACKGROUND,    CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },

                    // most of the properties from paragraph style...
                    { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND, CPPU_E2T(CPPUTYPE_BOOLEAN),          PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_PARA_BACK_COLOR), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PropertyAttribute::MAYBEVOID, MID_ESC          },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PropertyAttribute::MAYBEVOID, MID_ESC_HEIGHT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PropertyAttribute::MAYBEVOID, MID_UNDERLINE},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PropertyAttribute::MAYBEVOID, MID_UL_COLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PropertyAttribute::MAYBEVOID, MID_UL_HASCOLOR},
                    { SW_PROP_NMID(UNO_NAME_PARA_LEFT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_RIGHT_MARGIN), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT), RES_LR_SPACE,          CPPU_E2T(CPPUTYPE_BOOLEAN),      PropertyAttribute::MAYBEVOID, MID_FIRST_AUTO},
                    { SW_PROP_NMID(UNO_NAME_PARA_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PropertyAttribute::MAYBEVOID,  CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_FORMAT), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_DROPCAPFMT)  , PropertyAttribute::MAYBEVOID, MID_DROPCAP_FORMAT|CONVERT_TWIPS     },
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_WHOLE_WORD), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PropertyAttribute::MAYBEVOID, MID_DROPCAP_WHOLE_WORD },
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_PARA_KEEP_TOGETHER), RES_KEEP,              CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_SPLIT), RES_PARATR_SPLIT,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_WIDOWS), RES_PARATR_WIDOWS,        CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_ORPHANS), RES_PARATR_ORPHANS,      CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_EXPAND_SINGLE_WORD), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PropertyAttribute::MAYBEVOID, MID_EXPAND_SINGLE   },
                    { SW_PROP_NMID(UNO_NAME_PARA_LAST_LINE_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::MAYBEVOID, MID_LAST_LINE_ADJUST},
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_COUNT), RES_LINENUMBER,        CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_COUNT     },
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_START_VALUE), RES_LINENUMBER,      CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_STARTVALUE},
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_SPACING), RES_PARATR_LINESPACING, CPPU_E2T(CPPUTYPE_LINESPACE),PropertyAttribute::MAYBEVOID,     CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),         PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST},
                    { SW_PROP_NMID(UNO_NAME_PARA_REGISTER_MODE_ACTIVE), RES_PARATR_REGISTER,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_PARA_TOP_MARGIN), RES_UL_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_BOTTOM_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_LO_MARGIN|CONVERT_TWIPS},
                    TABSTOPS_MAP_ENTRY
                    { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,    PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER), RES_BOX,              CPPU_E2T(CPPUTYPE_BORDERLINE),  0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER), RES_BOX,             CPPU_E2T(CPPUTYPE_BORDERLINE),  0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER), RES_BOX,               CPPU_E2T(CPPUTYPE_BORDERLINE),  0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER), RES_BOX,                CPPU_E2T(CPPUTYPE_BORDERLINE),  0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_LEFT_BORDER_DISTANCE), RES_BOX,             CPPU_E2T(CPPUTYPE_INT32),   0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                CPPU_E2T(CPPUTYPE_INT32),   0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_TOP_BORDER_DISTANCE), RES_BOX,              CPPU_E2T(CPPUTYPE_INT32),   0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_BORDER_DISTANCE), RES_BOX,               CPPU_E2T(CPPUTYPE_INT32),   0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_HYPHENATION), RES_PARATR_HYPHENZONE,      CPPU_E2T(CPPUTYPE_BOOLEAN),  PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS), RES_PARATR_HYPHENZONE,         CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS), RES_PARATR_HYPHENZONE,        CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },
                    { SW_PROP_NMID(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS), RES_PARATR_HYPHENZONE,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_STYLE_NAME), RES_PARATR_NUMRULE,  CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,   0},
                    { SW_PROP_NMID(UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_PARA_SHADOW_FORMAT), RES_SHADOW,    CPPU_E2T(CPPUTYPE_SHADOWFMT),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, MID_TWOLINES},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_START_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_END_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_EMPHASIS},
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_HANGING_PUNCTUATION), RES_PARATR_HANGINGPUNCTUATION,  CPPU_E2T(CPPUTYPE_BOOLEAN),   PropertyAttribute::MAYBEVOID ,0     },
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_CHARACTER_DISTANCE), RES_PARATR_SCRIPTSPACE,         CPPU_E2T(CPPUTYPE_BOOLEAN),    PropertyAttribute::MAYBEVOID ,0     },
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_FORBIDDEN_RULES), RES_PARATR_FORBIDDEN_RULES,     CPPU_E2T(CPPUTYPE_BOOLEAN),    PropertyAttribute::MAYBEVOID ,0    },
                    { SW_PROP_NMID(UNO_NAME_PARA_VERT_ALIGNMENT), RES_PARATR_VERTALIGN,             CPPU_E2T(CPPUTYPE_INT16),    PropertyAttribute::MAYBEVOID , 0  },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION), RES_CHRATR_ROTATE,      CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,    MID_ROTATE      },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE), RES_CHRATR_ROTATE,       CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID,        MID_FITTOLINE  },
                    { SW_PROP_NMID(UNO_NAME_CHAR_SCALE_WIDTH), RES_CHRATR_SCALEW,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID,         0 },
                    { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PropertyAttribute::MAYBEVOID,      MID_RELIEF },

                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_NUMBERING_RULES), FN_UNO_NUM_RULES,     CPPU_E2T(CPPUTYPE_REFIDXREPL),  PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                        \

                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aRangePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_SECTION:
            {
                static SfxItemPropertyMap aSectionPropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_CONDITION), WID_SECT_CONDITION, CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_DDE_COMMAND_FILE), WID_SECT_DDE_TYPE, CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_DDE_COMMAND_TYPE), WID_SECT_DDE_FILE, CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_DDE_COMMAND_ELEMENT), WID_SECT_DDE_ELEMENT, CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC_UPDATE), WID_SECT_DDE_AUTOUPDATE, CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE,  0},
                    { SW_PROP_NMID(UNO_NAME_FILE_LINK), WID_SECT_LINK     , CPPU_E2T(CPPUTYPE_SECTFILELNK),     PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_VISIBLE), WID_SECT_VISIBLE   , CPPU_E2T(CPPUTYPE_BOOLEAN),           PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_SECT_PROTECTED, CPPU_E2T(CPPUTYPE_BOOLEAN),              PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LINK_REGION), WID_SECT_REGION   , CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_IS_COLLECT_AT_TEXT_END), RES_FTN_AT_TXTEND,        CPPU_E2T(CPPUTYPE_BOOLEAN),                PROPERTY_NONE ,MID_COLLECT                   },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_IS_RESTART_NUMBERING), RES_FTN_AT_TXTEND,      CPPU_E2T(CPPUTYPE_BOOLEAN),                PROPERTY_NONE , MID_RESTART_NUM },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_RESTART_NUMBERING_AT), RES_FTN_AT_TXTEND,      CPPU_E2T(CPPUTYPE_INT16),               PROPERTY_NONE , MID_NUM_START_AT},
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_IS_OWN_NUMBERING), RES_FTN_AT_TXTEND,  CPPU_E2T(CPPUTYPE_BOOLEAN),                                                 PROPERTY_NONE ,  MID_OWN_NUM     },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_NUMBERING_TYPE), RES_FTN_AT_TXTEND,        CPPU_E2T(CPPUTYPE_INT16),               PROPERTY_NONE , MID_NUM_TYPE    },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_NUMBERING_PREFIX), RES_FTN_AT_TXTEND,  CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,      MID_PREFIX      },
                    { SW_PROP_NMID(UNO_NAME_FOOTNOTE_NUMBERING_SUFFIX), RES_FTN_AT_TXTEND,  CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,      MID_SUFFIX      },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_IS_COLLECT_AT_TEXT_END), RES_END_AT_TXTEND,        CPPU_E2T(CPPUTYPE_BOOLEAN),                 PROPERTY_NONE , MID_COLLECT                      },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_IS_RESTART_NUMBERING), RES_END_AT_TXTEND,        CPPU_E2T(CPPUTYPE_BOOLEAN),                   PROPERTY_NONE , MID_RESTART_NUM  },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_RESTART_NUMBERING_AT), RES_END_AT_TXTEND,     CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE ,  MID_NUM_START_AT },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_IS_OWN_NUMBERING), RES_END_AT_TXTEND,     CPPU_E2T(CPPUTYPE_BOOLEAN),                                                   PROPERTY_NONE ,  MID_OWN_NUM      },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_NUMBERING_TYPE), RES_END_AT_TXTEND,       CPPU_E2T(CPPUTYPE_INT16),             PROPERTY_NONE ,MID_NUM_TYPE     },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_NUMBERING_PREFIX), RES_END_AT_TXTEND,     CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,   MID_PREFIX       },
                    { SW_PROP_NMID(UNO_NAME_ENDNOTE_NUMBERING_SUFFIX), RES_END_AT_TXTEND,     CPPU_E2T(CPPUTYPE_OUSTRING)  ,        PROPERTY_NONE,   MID_SUFFIX       },
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX), WID_SECT_DOCUMENT_INDEX, CPPU_E2T(CPPUTYPE_REFDOCINDEX), PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_GLOBAL_DOCUMENT_SECTION), WID_SECT_IS_GLOBAL_DOC_SECTION, CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::READONLY, 0 },
                    { SW_PROP_NMID(UNO_NAME_PROTECTION_KEY), WID_SECT_PASSWORD,      CPPU_E2T(CPPUTYPE_SEQINT8), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_DONT_BALANCE_TEXT_COLUMNS), RES_COLUMNBALANCE, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    COMMON_TEXT_CONTENT_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_IS_CURRENTLY_VISIBLE), WID_SECT_CURRENTLY_VISIBLE, CPPU_E2T(CPPUTYPE_BOOLEAN),          PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    /* -> #109700# */
                    { SW_PROP_NMID(UNO_NAME_SECT_LEFT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},                                   \
                    { SW_PROP_NMID(UNO_NAME_SECT_RIGHT_MARGIN), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},                                  \
                    /* <- #1097001# */
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aSectionPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_SEARCH:
            {
                static SfxItemPropertyMap aSearchPropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_SEARCH_ALL), WID_SEARCH_ALL,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_BACKWARDS), WID_BACKWARDS,           CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_CASE_SENSITIVE), WID_CASE_SENSITIVE,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                //  { SW_PROP_NMID(UNO_NAME_SEARCH_IN_SELECTION), WID_IN_SELECTION,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_REGULAR_EXPRESSION), WID_REGULAR_EXPRESSION, CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY), WID_SIMILARITY,         CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY_ADD), WID_SIMILARITY_ADD,     CPPU_E2T(CPPUTYPE_INT16)  ,     PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY_EXCHANGE), WID_SIMILARITY_EXCHANGE,CPPU_E2T(CPPUTYPE_INT16)  ,    PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY_RELAX), WID_SIMILARITY_RELAX,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_SIMILARITY_REMOVE), WID_SIMILARITY_REMOVE,   CPPU_E2T(CPPUTYPE_INT16)  ,     PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_STYLES), WID_STYLES,             CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SEARCH_WORDS), WID_WORDS,               CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aSearchPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_FRAME:
            {
                static SfxItemPropertyMap aFramePropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAIN_NEXT_NAME), RES_CHAIN,                CPPU_E2T(CPPUTYPE_OUSTRING),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_NEXTNAME},
                    { SW_PROP_NMID(UNO_NAME_CHAIN_PREV_NAME), RES_CHAIN,                CPPU_E2T(CPPUTYPE_OUSTRING),            PropertyAttribute::MAYBEVOID ,MID_CHAIN_PREVNAME},
                /*not impl*/    { SW_PROP_NMID(UNO_NAME_CLIENT_MAP), RES_URL,               CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_URL_CLIENTMAP         },
                    { SW_PROP_NMID(UNO_NAME_EDIT_IN_READONLY), RES_EDIT_IN_READONLY,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, 0},
                //  { SW_PROP_NMID(UNO_NAME_GRAPHIC), RES_BACKGROUND,       &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    //next elements are part of the service description
                    { SW_PROP_NMID(UNO_NAME_FRAME_HEIGHT_ABSOLUTE),         RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32),       PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NMID(UNO_NAME_FRAME_HEIGHT_PERCENT),              RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT8),        PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT   },
                    { SW_PROP_NMID(UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT),         RES_FRM_SIZE,        CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE, MID_FRMSIZE_IS_AUTO_HEIGHT   },
                    { SW_PROP_NMID(UNO_NAME_FRAME_WIDTH_ABSOLUTE),          RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT32),       PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
                    { SW_PROP_NMID(UNO_NAME_FRAME_WIDTH_PERCENT),               RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT8),        PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH    },
                    { SW_PROP_NMID(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { SW_PROP_NMID(UNO_NAME_WRITING_MODE), RES_FRAMEDIR, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE, 0 },
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aFramePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_GRAPHIC:
            {
                static SfxItemPropertyMap aGraphicPropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_CROP), RES_GRFATR_CROPGRF,     CPPU_E2T(CPPUTYPE_GRFCROP),  PROPERTY_NONE, CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES), RES_GRFATR_MIRRORGRF, CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE,      MID_MIRROR_HORZ_EVEN_PAGES            },
                    { SW_PROP_NMID(UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES), RES_GRFATR_MIRRORGRF,  CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE,      MID_MIRROR_HORZ_ODD_PAGES                 },
                    { SW_PROP_NMID(UNO_NAME_VERT_MIRRORED), RES_GRFATR_MIRRORGRF,   CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE,     MID_MIRROR_VERT            },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_URL), FN_UNO_GRAPHIC_U_R_L, CPPU_E2T(CPPUTYPE_OUSTRING), 0, 0 },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_FILTER), FN_UNO_GRAPHIC_FILTER,      CPPU_E2T(CPPUTYPE_OUSTRING), 0, 0 },
                    { SW_PROP_NMID(UNO_NAME_ACTUAL_SIZE), FN_UNO_ACTUAL_SIZE,    CPPU_E2T(CPPUTYPE_AWTSIZE),  PropertyAttribute::READONLY, CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_ALTERNATIVE_TEXT), FN_UNO_ALTERNATIVE_TEXT,CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE , 0   },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_POLY_POLYGON), FN_PARAM_COUNTOUR_PP, CPPU_E2T(CPPUTYPE_PNTSEQSEQ), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PIXEL_CONTOUR), FN_UNO_IS_PIXEL_CONTOUR, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC_CONTOUR), FN_UNO_IS_AUTOMATIC_CONTOUR , CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_ROTATION), RES_GRFATR_ROTATION,      CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_LUMINANCE), RES_GRFATR_LUMINANCE,     CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_CONTRAST), RES_GRFATR_CONTRAST,   CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_RED), RES_GRFATR_CHANNELR,    CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_GREEN), RES_GRFATR_CHANNELG,      CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_ADJUST_BLUE), RES_GRFATR_CHANNELB,   CPPU_E2T(CPPUTYPE_INT16),  0,   0},
                    { SW_PROP_NMID(UNO_NAME_GAMMA), RES_GRFATR_GAMMA,        CPPU_E2T(CPPUTYPE_DOUBLE),     0,   0},
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_IS_INVERTED), RES_GRFATR_INVERT,         CPPU_E2T(CPPUTYPE_BOOLEAN),    0,   0},
                    { SW_PROP_NMID(UNO_NAME_TRANSPARENCY), RES_GRFATR_TRANSPARENCY, CPPU_E2T(CPPUTYPE_INT16),   0,   0},
                    { SW_PROP_NMID(UNO_NAME_GRAPHIC_COLOR_MODE), RES_GRFATR_DRAWMODE,    CPPU_E2T(CPPUTYPE_COLORMODE),      0,   0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aGraphicPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_EMBEDDED_OBJECT:
            {
                static SfxItemPropertyMap aEmbeddedPropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_SURROUND_CONTOUR },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE},
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_POLY_POLYGON), FN_PARAM_COUNTOUR_PP, CPPU_E2T(CPPUTYPE_PNTSEQSEQ), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_PIXEL_CONTOUR), FN_UNO_IS_PIXEL_CONTOUR, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC_CONTOUR), FN_UNO_IS_AUTOMATIC_CONTOUR , CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_CLSID),                FN_UNO_CLSID, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_MODEL), FN_UNO_MODEL, CPPU_E2T(CPPUTYPE_REFMODEL), PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_COMPONENT),FN_UNO_COMPONENT, CPPU_E2T(CPPUTYPE_REFCOMPONENT), PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aEmbeddedPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE:
            {
                static SfxItemPropertyMap aShapeMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_PAGE_NO), RES_ANCHOR,            CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_PAGENUM      },
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_TYPE), RES_ANCHOR,           CPPU_E2T(CPPUTYPE_TXTCNTANCHOR),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORTYPE},
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_FRAME), RES_ANCHOR,             CPPU_E2T(CPPUTYPE_REFTEXTFRAME),    PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORFRAME},
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT), RES_HORI_ORIENT,        CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_ORIENT },
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_POSITION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_POSITION|CONVERT_TWIPS   },
                    { SW_PROP_NMID(UNO_NAME_HORI_ORIENT_RELATION), RES_HORI_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_RELATION },
                    { SW_PROP_NMID(UNO_NAME_LEFT_MARGIN), RES_LR_SPACE,             CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_RIGHT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_SURROUND), RES_SURROUND,          CPPU_E2T(CPPUTYPE_WRAPTXTMODE),    PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_SURROUNDTYPE },
                    { SW_PROP_NMID(UNO_NAME_TEXT_WRAP), RES_SURROUND,           CPPU_E2T(CPPUTYPE_WRAPTXTMODE),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { SW_PROP_NMID(UNO_NAME_SURROUND_ANCHORONLY), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_ANCHORONLY     },
                    { SW_PROP_NMID(UNO_NAME_SURROUND_CONTOUR), RES_SURROUND,            CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUR         },
                    { SW_PROP_NMID(UNO_NAME_CONTOUR_OUTSIDE), RES_SURROUND,             CPPU_E2T(CPPUTYPE_BOOLEAN),             PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT), RES_VERT_ORIENT,      CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_ORIENT   },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_POSITION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_POSITION|CONVERT_TWIPS   },
                    { SW_PROP_NMID(UNO_NAME_VERT_ORIENT_RELATION), RES_VERT_ORIENT,     CPPU_E2T(CPPUTYPE_INT16),           PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_RELATION },
                    { SW_PROP_NMID(UNO_NAME_TEXT_RANGE), FN_TEXT_RANGE,         CPPU_E2T(CPPUTYPE_REFTXTRANGE),         PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_OPAQUE), RES_OPAQUE,             CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_ANCHOR_POSITION), FN_ANCHOR_POSITION,    CPPU_E2T(CPPUTYPE_AWTPOINT),    PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aShapeMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_MARK:
            {
                static SfxItemPropertyMap aIdxMarkMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_PRIMARY_KEY), WID_PRIMARY_KEY,  CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SECONDARY_KEY), WID_SECONDARY_KEY,  CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_READING), WID_TEXT_READING, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_PRIMARY_KEY_READING), WID_PRIMARY_KEY_READING, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_SECONDARY_KEY_READING), WID_SECONDARY_KEY_READING, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_MAIN_ENTRY), WID_MAIN_ENTRY,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aIdxMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_CNTIDX_MARK:
            {
                static SfxItemPropertyMap aCntntMarkMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL), WID_LEVEL        ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aCntntMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_USER_MARK:
            {
                static SfxItemPropertyMap aUserMarkMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_ALTERNATIVE_TEXT), WID_ALT_TEXT,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL        ),   WID_LEVEL        ,    CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USER_INDEX_NAME), WID_USER_IDX_NAME,    CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aUserMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_IDX:
            {
                static SfxItemPropertyMap aTOXIndexMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_KEY_AS_ENTRY), WID_USE_KEY_AS_ENTRY                    ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_COMBINED_ENTRIES), WID_USE_COMBINED_ENTRIES                ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_CASE_SENSITIVE), WID_IS_CASE_SENSITIVE                   ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_P_P), WID_USE_P_P                             ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_DASH), WID_USE_DASH                            ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_UPPER_CASE), WID_USE_UPPER_CASE                      ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,   0},
                //  { SW_PROP_NMID(UNO_NAME_???                               ),   WID_???                                 ,  &_getReflection  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME), WID_MAIN_ENTRY_CHARACTER_STYLE_NAME     ,  CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),                 WID_PARA_HEAD,          CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLESEPARATOR),           WID_PARA_SEP,           CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),              WID_PARA_LEV1,          CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL2),              WID_PARA_LEV2,          CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL3),              WID_PARA_LEV3,          CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_COMMA_SEPARATED), WID_IS_COMMA_SEPARATED, CPPU_E2T(CPPUTYPE_BOOLEAN),            PROPERTY_NONE ,0         },
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, CPPU_E2T(CPPUTYPE_DOCIDXMRK),           PropertyAttribute::READONLY ,0       },
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_LOCALE),            WID_IDX_LOCALE,         CPPU_E2T(CPPUTYPE_LOCALE), PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_SORT_ALGORITHM),    WID_IDX_SORT_ALGORITHM,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},\
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXIndexMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_CNTNT:
            {
                static SfxItemPropertyMap aTOXContentMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_LEVEL), WID_LEVEL                               ,  CPPU_E2T(CPPUTYPE_INT16)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_MARKS), WID_CREATE_FROM_MARKS                   ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_OUTLINE), WID_CREATE_FROM_OUTLINE                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                //  { SW_PROP_NMID(UNO_NAME_PARAGRAPH_STYLE_NAMES), WID_PARAGRAPH_STYLE_NAMES               ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_PARAGRAPH_STYLES), WID_LEVEL_PARAGRAPH_STYLES              ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PropertyAttribute::READONLY,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), WID_CREATE_FROM_PARAGRAPH_STYLES, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                //  { SW_PROP_NMID(UNO_NAME_RECALC_TAB_STOPS), WID_RECALC_TAB_STOPS                    ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                //  { SW_PROP_NMID(UNO_NAME_???                               ), WID_???                                 ,  &_getReflection  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL2),  WID_PARA_LEV2,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL3),  WID_PARA_LEV3,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL4),  WID_PARA_LEV4,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL5),  WID_PARA_LEV5,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL6),  WID_PARA_LEV6,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL7),  WID_PARA_LEV7,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL8),  WID_PARA_LEV8,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL9),  WID_PARA_LEV9,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL10),     WID_PARA_LEV10,     CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, CPPU_E2T(CPPUTYPE_DOCIDXMRK),           PropertyAttribute::READONLY ,0       },
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXContentMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_USER:
            {
                static SfxItemPropertyMap aTOXUserMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_MARKS), WID_CREATE_FROM_MARKS                   ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                //  { SW_PROP_NMID(UNO_NAME_PARAGRAPH_STYLE_NAMES), WID_PARAGRAPH_STYLE_NAMES               ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_LEVEL_FROM_SOURCE), WID_USE_LEVEL_FROM_SOURCE               ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_PARAGRAPH_STYLES), WID_LEVEL_PARAGRAPH_STYLES              ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PropertyAttribute::READONLY,0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), WID_CREATE_FROM_PARAGRAPH_STYLES, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_TABLES), WID_CREATE_FROM_TABLES                  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_TEXT_FRAMES), WID_CREATE_FROM_TEXT_FRAMES             ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS), WID_CREATE_FROM_GRAPHIC_OBJECTS         ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS), WID_CREATE_FROM_EMBEDDED_OBJECTS        ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL2),  WID_PARA_LEV2,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL3),  WID_PARA_LEV3,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL4),  WID_PARA_LEV4,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL5),  WID_PARA_LEV5,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL6),  WID_PARA_LEV6,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL7),  WID_PARA_LEV7,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL8),  WID_PARA_LEV8,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL9),  WID_PARA_LEV9,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL10),     WID_PARA_LEV10,     CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_INDEX_MARKS), WID_INDEX_MARKS, CPPU_E2T(CPPUTYPE_DOCIDXMRK),           PropertyAttribute::READONLY ,0       },
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_USER_INDEX_NAME), WID_USER_IDX_NAME,    CPPU_E2T(CPPUTYPE_OUSTRING)  ,      PROPERTY_NONE,     0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXUserMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_TABLES:
            {
                static SfxItemPropertyMap aTOXTablesMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_LABELS), WID_CREATE_FROM_LABELS                  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LABEL_CATEGORY), WID_LABEL_CATEGORY                      ,  CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LABEL_DISPLAY_TYPE), WID_LABEL_DISPLAY_TYPE                  ,  CPPU_E2T(CPPUTYPE_INT16)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXTablesMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_OBJECTS:
            {
                static SfxItemPropertyMap aTOXObjectsMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_STAR_MATH), WID_CREATE_FROM_STAR_MATH               ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_STAR_CHART), WID_CREATE_FROM_STAR_CHART              ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_STAR_CALC), WID_CREATE_FROM_STAR_CALC               ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_STAR_DRAW), WID_CREATE_FROM_STAR_DRAW               ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS), WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXObjectsMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_ILLUSTRATIONS:
            {
                static SfxItemPropertyMap aTOXIllustrationsMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_CHAPTER), WID_CREATE_FROM_CHAPTER                 ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_CREATE_FROM_LABELS), WID_CREATE_FROM_LABELS                  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_USE_ALPHABETICAL_SEPARATORS), WID_USE_ALPHABETICAL_SEPARATORS         ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LABEL_CATEGORY), WID_LABEL_CATEGORY                      ,  CPPU_E2T(CPPUTYPE_OUSTRING)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LABEL_DISPLAY_TYPE), WID_LABEL_DISPLAY_TYPE                  ,  CPPU_E2T(CPPUTYPE_INT16)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXIllustrationsMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_ROW:
            {
                static SfxItemPropertyMap aTableRowPropertyMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE ,MID_BACK_COLOR         },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_TABLE_COLUMN_SEPARATORS), FN_UNO_TABLE_COLUMN_SEPARATORS,   CPPU_E2T(CPPUTYPE_TBLCOLSEP),   PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_HEIGHT), FN_UNO_ROW_HEIGHT,     CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,CONVERT_TWIPS },
                    { SW_PROP_NMID(UNO_NAME_IS_AUTO_HEIGHT), FN_UNO_ROW_AUTO_HEIGHT,    CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE , 0 },
                    { SW_PROP_NMID(UNO_NAME_SIZE_TYPE), RES_FRM_SIZE,           CPPU_E2T(CPPUTYPE_INT16)  ,         PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                //  { SW_PROP_NMID(UNO_NAME_HEIGHT), RES_FRM_SIZE,          CPPU_E2T(CPPUTYPE_INT32)  ,         PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                //  { SW_PROP_NMID(UNO_NAME_SHADOW_FORMAT), RES_SHADOW,             CPPU_E2T(CPPUTYPE_SHADOWFMT),   PROPERTY_NONE, CONVERT_TWIPS},
                    {0,0,0,0,0}
                };

                aMapArr[nPropertyId] = (SfxItemPropertyMap*)aTableRowPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_CURSOR:
            {
                // das PropertySet entspricht dem Range ohne Chart-Properties
                static SfxItemPropertyMap aTableCursorPropertyMap_Impl [] =
                {
                    { SW_PROP_NMID(UNO_NAME_CHAR_AUTO_KERNING), RES_CHRATR_AUTOKERN  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CASE_MAP), RES_CHRATR_CASEMAP,     CPPU_E2T(CPPUTYPE_INT16),           PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_STRIKEOUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_INT16),                  PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CROSSED_OUT), RES_CHRATR_CROSSEDOUT,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,        PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT), RES_CHRATR_ESCAPEMENT,  CPPU_E2T(CPPUTYPE_INT16),             PropertyAttribute::MAYBEVOID, MID_ESC          },
                    { SW_PROP_NMID(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT), RES_CHRATR_ESCAPEMENT,     CPPU_E2T(CPPUTYPE_INT8)  ,          PropertyAttribute::MAYBEVOID, MID_ESC_HEIGHT},
                    { SW_PROP_NMID(UNO_NAME_CHAR_FLASH), RES_CHRATR_BLINK   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT16),      PropertyAttribute::MAYBEVOID, MID_UNDERLINE},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_INT32),            PropertyAttribute::MAYBEVOID, MID_UL_COLOR},
                    { SW_PROP_NMID(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR), RES_CHRATR_UNDERLINE ,  CPPU_E2T(CPPUTYPE_BOOLEAN),              PropertyAttribute::MAYBEVOID, MID_UL_HASCOLOR},
                    { SW_PROP_NMID(UNO_NAME_PARA_LEFT_MARGIN), RES_LR_SPACE,            CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_RIGHT_MARGIN), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_BOOLEAN),      PropertyAttribute::MAYBEVOID, MID_FIRST_AUTO},
                    { SW_PROP_NMID(UNO_NAME_PARA_FIRST_LINE_INDENT), RES_LR_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_KERNING), RES_CHRATR_KERNING    ,  CPPU_E2T(CPPUTYPE_INT16)  ,         PropertyAttribute::MAYBEVOID,  CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_NO_HYPHENATION), RES_CHRATR_NOHYPHEN   ,   CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_SHADOWED), RES_CHRATR_SHADOWED  ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_CONTOURED), RES_CHRATR_CONTOUR,    CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_FORMAT), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_DROPCAPFMT)  , PropertyAttribute::MAYBEVOID, MID_DROPCAP_FORMAT     },
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_WHOLE_WORD), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PropertyAttribute::MAYBEVOID, MID_DROPCAP_WHOLE_WORD },
                    { SW_PROP_NMID(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME), RES_PARATR_DROP,        CPPU_E2T(CPPUTYPE_OUSTRING)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_PARA_KEEP_TOGETHER), RES_KEEP,              CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_SPLIT), RES_PARATR_SPLIT,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_WIDOWS), RES_PARATR_WIDOWS,        CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_ORPHANS), RES_PARATR_ORPHANS,      CPPU_E2T(CPPUTYPE_INT8),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), RES_BOXATR_FORMAT,      CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NMID(UNO_NAME_PARA_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),         PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST},
                    { SW_PROP_NMID(UNO_NAME_PARA_EXPAND_SINGLE_WORD), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_BOOLEAN)  ,         PropertyAttribute::MAYBEVOID, MID_EXPAND_SINGLE   },
                    { SW_PROP_NMID(UNO_NAME_PARA_LAST_LINE_ADJUST), RES_PARATR_ADJUST,      CPPU_E2T(CPPUTYPE_INT16),       PropertyAttribute::MAYBEVOID, MID_LAST_LINE_ADJUST},
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_COUNT), RES_LINENUMBER,        CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_COUNT     },
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_NUMBER_START_VALUE), RES_LINENUMBER,      CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_STARTVALUE},
                    { SW_PROP_NMID(UNO_NAME_PARA_LINE_SPACING), RES_PARATR_LINESPACING, CPPU_E2T(CPPUTYPE_LINESPACE),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_REGISTER_MODE_ACTIVE), RES_PARATR_REGISTER,     CPPU_E2T(CPPUTYPE_BOOLEAN)  ,       PropertyAttribute::MAYBEVOID, 0},
                    TABSTOPS_MAP_ENTRY
                    { SW_PROP_NMID(UNO_NAME_TOP_MARGIN), RES_UL_SPACE,          CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_BOTTOM_MARGIN), RES_UL_SPACE,           CPPU_E2T(CPPUTYPE_INT32), PropertyAttribute::MAYBEVOID, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COLOR), RES_CHRATR_COLOR,      CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_COLOR), RES_CHRATR_BACKGROUND,    CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NMID(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NMID(UNO_NAME_PARA_BACK_COLOR), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_INT32),           PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NMID(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_BOOLEAN),         PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_PARA_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC), PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), FN_UNO_TABLE_CELL_BACKGROUND,   CPPU_E2T(CPPUTYPE_GRAPHICLOC), PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), FN_UNO_TABLE_CELL_BACKGROUND,  CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), FN_UNO_TABLE_CELL_BACKGROUND,    CPPU_E2T(CPPUTYPE_BOOLEAN), PropertyAttribute::MAYBEVOID|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NMID(UNO_NAME_CHAR_WORD_MODE), RES_CHRATR_WORDLINEMODE,CPPU_E2T(CPPUTYPE_BOOLEAN)  ,    PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLE_NAME), FN_UNO_PARA_STYLE,        CPPU_E2T(CPPUTYPE_OUSTRING),         PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NMID(UNO_NAME_USER_DEFINED_ATTRIBUTES), RES_UNKNOWNATR_CONTAINER, CPPU_E2T(CPPUTYPE_REFNAMECNT), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_IS_ON), RES_CHRATR_TWO_LINES,          CPPU_E2T(CPPUTYPE_BOOLEAN),     PropertyAttribute::MAYBEVOID, MID_TWOLINES},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_PREFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_START_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_COMBINE_SUFFIX), RES_CHRATR_TWO_LINES,             CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_END_BRACKET},
                    { SW_PROP_NMID(UNO_NAME_CHAR_EMPHASIS), RES_CHRATR_EMPHASIS_MARK,           CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_EMPHASIS},
                    { SW_PROP_NMID(UNO_NAME_CHAR_RELIEF), RES_CHRATR_RELIEF,      CPPU_E2T(CPPUTYPE_INT16),    PropertyAttribute::MAYBEVOID,      MID_RELIEF },
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTableCursorPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_BOOKMARK:
            {
                static SfxItemPropertyMap aBookmarkPropertyMap_Impl [] =
                {
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), FN_PARAM_LINK_DISPLAY_NAME,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aBookmarkPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH_EXTENSIONS:
            {
                static SfxItemPropertyMap aParagraphExtensionsMap_Impl[] =
                {
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0,0}
                };

                aMapArr[nPropertyId] = aParagraphExtensionsMap_Impl;
            }
            break;
            case PROPERTY_MAP_BIBLIOGRAPHY :
            {
                static SfxItemPropertyMap aBibliographyMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NMID(UNO_NAME_IS_PROTECTED), WID_PROTECTED                           ,  CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,     0},
                    { SW_PROP_NMID(UNO_NAME_TEXT_COLUMNS), RES_COL,                CPPU_E2T(CPPUTYPE_REFTEXTCOL),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_URL), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_FILTER), RES_BACKGROUND,       CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NMID(UNO_NAME_BACK_GRAPHIC_LOCATION), RES_BACKGROUND,         CPPU_E2T(CPPUTYPE_GRAPHICLOC),          PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NMID(UNO_NAME_BACK_COLOR), RES_BACKGROUND,            CPPU_E2T(CPPUTYPE_INT32),           PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NMID(UNO_NAME_BACK_TRANSPARENT), RES_BACKGROUND,      CPPU_E2T(CPPUTYPE_BOOLEAN),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLEHEADING),     WID_PARA_HEAD,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_PARA_STYLELEVEL1),  WID_PARA_LEV1,  CPPU_E2T(CPPUTYPE_OUSTRING)  , 0,     0},
                    { SW_PROP_NMID(UNO_NAME_LEVEL_FORMAT), WID_LEVEL_FORMAT                        ,  CPPU_E2T(CPPUTYPE_REFIDXREPL)  , PROPERTY_NONE,0},
                    { SW_PROP_NMID(UNO_NAME_LOCALE),            WID_IDX_LOCALE,         CPPU_E2T(CPPUTYPE_LOCALE), PROPERTY_NONE,     0},\
                    { SW_PROP_NMID(UNO_NAME_SORT_ALGORITHM),    WID_IDX_SORT_ALGORITHM,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},\
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aBibliographyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_DOCUMENT:
            {
                static SfxItemPropertyMap aDocMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_BASIC_LIBRARIES), WID_DOC_BASIC_LIBRARIES,  CPPU_E2T(CPPUTYPE_REFLIBCONTAINER), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME), RES_CHRATR_FONT,       CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME), RES_CHRATR_FONT,     CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY), RES_CHRATR_FONT,     CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET), RES_CHRATR_FONT,       CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH), RES_CHRATR_FONT,      CPPU_E2T(CPPUTYPE_INT16),                   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN), RES_CHRATR_CJK_FONT,   CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY_ASIAN), RES_CHRATR_CJK_FONT,   CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN), RES_CHRATR_CJK_FONT, CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH_ASIAN), RES_CHRATR_CJK_FONT,    CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_NAME_COMPLEX), RES_CHRATR_CTL_FONT,   CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX), RES_CHRATR_CTL_FONT, CPPU_E2T(CPPUTYPE_OUSTRING),    PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_FAMILY_COMPLEX), RES_CHRATR_CTL_FONT, CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX), RES_CHRATR_CTL_FONT,   CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },
                    { SW_PROP_NMID(UNO_NAME_CHAR_FONT_PITCH_COMPLEX), RES_CHRATR_CTL_FONT,  CPPU_E2T(CPPUTYPE_INT16),   PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },
                    { SW_PROP_NMID(UNO_NAME_CHAR_LOCALE), RES_CHRATR_LANGUAGE ,   CPPU_E2T(CPPUTYPE_LOCALE), PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },
                    { SW_PROP_NMID(UNO_NAME_CHARACTER_COUNT), WID_DOC_CHAR_COUNT,           CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L), WID_DOC_AUTO_MARK_URL, CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_PARAGRAPH_COUNT), WID_DOC_PARA_COUNT,           CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_RECORD_CHANGES), WID_DOC_CHANGES_RECORD,        CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_SHOW_CHANGES), WID_DOC_CHANGES_SHOW,        CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_WORD_COUNT), WID_DOC_WORD_COUNT,            CPPU_E2T(CPPUTYPE_INT32),   PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_WORD_SEPARATOR), WID_DOC_WORD_SEPARATOR,        CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_HIDE_FIELD_TIPS), WID_DOC_HIDE_TIPS,            CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_REDLINE_DISPLAY_TYPE), WID_DOC_REDLINE_DISPLAY,     CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_REDLINE_PROTECTION_KEY), WID_DOC_CHANGES_PASSWORD,      CPPU_E2T(CPPUTYPE_SEQINT8), PROPERTY_NONE, 0 },
                    { SW_PROP_NMID(UNO_NAME_FORBIDDEN_CHARACTERS), WID_DOC_FORBIDDEN_CHARS,    CPPU_E2T(CPPUTYPE_REFFORBCHARS), PropertyAttribute::READONLY,   0},
                    { SW_PROP_NMID(UNO_NAME_TWO_DIGIT_YEAR), WID_DOC_TWO_DIGIT_YEAR,    CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_AUTOMATIC_CONTROL_FOCUS),       WID_DOC_AUTOMATIC_CONTROL_FOCUS,    CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    { SW_PROP_NMID(UNO_NAME_APPLY_FORM_DESIGN_MODE),        WID_DOC_APPLY_FORM_DESIGN_MODE,     CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocMap_Impl;
            }
            break;
            case PROPERTY_MAP_LINK_TARGET:
            {
                static SfxItemPropertyMap aLinkTargetMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_BITMAP), 0,     CPPU_E2T(CPPUTYPE_REFBITMAP), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NMID(UNO_LINK_DISPLAY_NAME), 0,   CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0xff},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aLinkTargetMap_Impl;
            }
            break;
            case PROPERTY_MAP_AUTO_TEXT_GROUP :
            {
                static SfxItemPropertyMap aAutoTextGroupMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_FILE_PATH), WID_GROUP_PATH,     CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE,   PropertyAttribute::READONLY},
                    { SW_PROP_NMID(UNO_NAME_TITLE), WID_GROUP_TITLE, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE,   0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aAutoTextGroupMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXTPORTION_EXTENSIONS:
            {
                static SfxItemPropertyMap aTextPortionExtensionMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    {SW_PROP_NMID(UNO_NAME_BOOKMARK), FN_UNO_BOOKMARK, CPPU_E2T(CPPUTYPE_REFTEXTCNTNT),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    {SW_PROP_NMID(UNO_NAME_CONTROL_CHARACTER), FN_UNO_CONTROL_CHARACTER, CPPU_E2T(CPPUTYPE_INT16),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
                    {SW_PROP_NMID(UNO_NAME_IS_COLLAPSED), FN_UNO_IS_COLLAPSED, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    {SW_PROP_NMID(UNO_NAME_IS_START), FN_UNO_IS_START, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    //_REDLINE_PROPERTIES
                    {SW_PROP_NMID(UNO_NAME_TEXT_PORTION_TYPE), FN_UNO_TEXT_PORTION_TYPE, CPPU_E2T(CPPUTYPE_OUSTRING),                        PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTextPortionExtensionMap_Impl;
            }
            break;
            case PROPERTY_MAP_FOOTNOTE:
            {
                static SfxItemPropertyMap aFootnoteMap_Impl[] =
                {
                    {SW_PROP_NMID(UNO_NAME_REFERENCE_ID), 0, CPPU_E2T(CPPUTYPE_INT16),PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID, 0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aFootnoteMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_COLUMS :
            {
                static SfxItemPropertyMap aTextColumns_Impl[] =
                {
                    {SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC), WID_TXTCOL_IS_AUTOMATIC, CPPU_E2T(CPPUTYPE_BOOLEAN),PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_AUTOMATIC_DISTANCE), WID_TXTCOL_AUTO_DISTANCE, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_WIDTH), WID_TXTCOL_LINE_WIDTH, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_COLOR), WID_TXTCOL_LINE_COLOR, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT), WID_TXTCOL_LINE_REL_HGT, CPPU_E2T(CPPUTYPE_INT32),PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT), WID_TXTCOL_LINE_ALIGN, CPPU_E2T(CPPUTYPE_VERTALIGN),PROPERTY_NONE,   0},
                    {SW_PROP_NMID(UNO_NAME_SEPARATOR_LINE_IS_ON), WID_TXTCOL_LINE_IS_ON, CPPU_E2T(CPPUTYPE_BOOLEAN),PROPERTY_NONE,  0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTextColumns_Impl;
            }
            break;
            case PROPERTY_MAP_REDLINE :
            {
                static SfxItemPropertyMap aRedlineMap_Impl[] =
                {
                    _REDLINE_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    {SW_PROP_NMID(UNO_NAME_REDLINE_START), 0, CPPU_E2T(CPPUTYPE_REFINTERFACE),  PropertyAttribute::READONLY,    0},
                    {SW_PROP_NMID(UNO_NAME_REDLINE_END), 0, CPPU_E2T(CPPUTYPE_REFINTERFACE),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aRedlineMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_DEFAULT :
            {
                static SfxItemPropertyMap aTextDefaultMap_Impl[] =
                {
                    { SW_PROP_NMID(UNO_NAME_TAB_STOP_DISTANCE), RES_PARATR_TABSTOP,     CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, MID_STD_TAB | CONVERT_TWIPS},
                    COMMON_CRSR_PARA_PROPERTIES_WITHOUT_FN
                    COMMON_HYPERLINK_PROPERTIES
                    { SW_PROP_NMID(UNO_NAME_CHAR_STYLE_NAME), RES_TXTATR_CHARFMT,     CPPU_E2T(CPPUTYPE_OUSTRING),  PropertyAttribute::MAYBEVOID,     0},    \
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aTextDefaultMap_Impl;
                for( SfxItemPropertyMap * pMap = aTextDefaultMap_Impl;
                        pMap->pName; ++pMap )
                {
                    // UNO_NAME_PAGE_DESC_NAME should keep its MAYBEVOID flag
                    if (!(RES_PAGEDESC == pMap->nWID && MID_PAGEDESC_PAGEDESCNAME == pMap->nMemberId))
                        pMap->nFlags &= ~PropertyAttribute::MAYBEVOID;
                }
            }
            break;
            case PROPERTY_MAP_REDLINE_PORTION :
            {
                static SfxItemPropertyMap aRedlinePortionMap_Impl[] =
                {
                    COMPLETE_TEXT_CURSOR_MAP
                    {SW_PROP_NMID(UNO_NAME_BOOKMARK), FN_UNO_BOOKMARK, CPPU_E2T(CPPUTYPE_REFTEXTCNTNT),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    {SW_PROP_NMID(UNO_NAME_CONTROL_CHARACTER), FN_UNO_CONTROL_CHARACTER, CPPU_E2T(CPPUTYPE_INT16),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
                    {SW_PROP_NMID(UNO_NAME_IS_COLLAPSED), FN_UNO_IS_COLLAPSED, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    {SW_PROP_NMID(UNO_NAME_IS_START), FN_UNO_IS_START, CPPU_E2T(CPPUTYPE_BOOLEAN),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    _REDLINE_PROPERTIES
                    {SW_PROP_NMID(UNO_NAME_TEXT_PORTION_TYPE), FN_UNO_TEXT_PORTION_TYPE, CPPU_E2T(CPPUTYPE_OUSTRING),                        PropertyAttribute::READONLY, 0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aRedlinePortionMap_Impl;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_DATETIME:
            {
                static SfxItemPropertyMap aDateTimeFieldPropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_ADJUST), FIELD_PROP_SUBTYPE,     CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_DATE_TIME_VALUE), FIELD_PROP_DATE_TIME,  CPPU_E2T(CPPUTYPE_DATETIME), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_IS_DATE),    FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDateTimeFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_USER     :
            {
                static SfxItemPropertyMap aUserFieldPropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL2,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),     FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),  FIELD_PROP_FORMAT,  CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };

                aMapArr[nPropertyId] = aUserFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_SET_EXP  :
            {
                static SfxItemPropertyMap aSetExpFieldPropMap       [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR2,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_HINT),               FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),      FIELD_PROP_FORMAT,  CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT2, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_INPUT),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL3,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE, 0},
                    //TODO: UNO_NAME_VARIABLE_NAME gibt es das wirklich?
                    {SW_PROP_NMID(UNO_NAME_SEQUENCE_VALUE), FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE),  PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_VARIABLE_NAME),  FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aSetExpFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_GET_EXP  :
            {
                static SfxItemPropertyMap aGetExpFieldPropMap       [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL2,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),  FIELD_PROP_FORMAT,  CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_VARIABLE_SUBTYPE),   FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aGetExpFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_FILE_NAME:
            {
                static SfxItemPropertyMap aFileNameFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_FILE_FORMAT), FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL2, CPPU_E2T(CPPUTYPE_BOOLEAN),       PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aFileNameFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_PAGE_NUM :
            {
                static SfxItemPropertyMap aPageNumFieldPropMap      [] =
                {
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_FORMAT,  CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_OFFSET),             FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),  PROPERTY_NONE,   0},
                    {SW_PROP_NMID(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, CPPU_E2T(CPPUTYPE_PAGENUMTYPE), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_USERTEXT),           FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aPageNumFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_AUTHOR   :
            {
                static SfxItemPropertyMap aAuthorFieldPropMap       [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),    FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL2, CPPU_E2T(CPPUTYPE_BOOLEAN),       PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_FULL_NAME),FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aAuthorFieldPropMap;
            }
            break;
            case  PROPERTY_MAP_FLDTYP_CHAPTER  :
            {
                static SfxItemPropertyMap aChapterFieldPropMap      [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CHAPTER_FORMAT),FIELD_PROP_USHORT1,  CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_LEVEL        ),FIELD_PROP_BYTE1,         CPPU_E2T(CPPUTYPE_INT8),    PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aChapterFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_GET_REFERENCE          :
            {
                static SfxItemPropertyMap aGetRefFieldPropMap       [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_REFERENCE_FIELD_PART),FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),  PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_REFERENCE_FIELD_SOURCE),FIELD_PROP_USHORT2, CPPU_E2T(CPPUTYPE_INT16),    PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_SEQUENCE_NUMBER),    FIELD_PROP_SHORT1,  CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SOURCE_NAME),        FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aGetRefFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_CONDITIONED_TEXT      :
            {
                static SfxItemPropertyMap aConditionedTxtFieldPropMap   [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONDITION),      FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_FALSE_CONTENT),  FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_CONDITION_TRUE) ,  FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_TRUE_CONTENT) ,  FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aConditionedTxtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_TEXT :
            {
                static SfxItemPropertyMap aHiddenTxtFieldPropMap    [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONDITION),      FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT) ,       FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_HIDDEN) ,     FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aHiddenTxtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_ANNOTATION            :
            {
                static SfxItemPropertyMap aAnnotationFieldPropMap   [] =
                {
                    {SW_PROP_NMID(UNO_NAME_AUTHOR), FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT),    FIELD_PROP_PAR2,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATE),   FIELD_PROP_DATE,    CPPU_E2T(CPPUTYPE_DATE),    PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aAnnotationFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_INPUT:
            {
                static SfxItemPropertyMap aInputFieldPropMap        [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),    FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_HINT),       FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aInputFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_MACRO                 :
            {
                static SfxItemPropertyMap aMacroFieldPropMap        [] =
                {
                    {SW_PROP_NMID(UNO_NAME_HINT), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_MACRO_NAME),FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_MACRO_LIBRARY),FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aMacroFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DDE                   :
            {
                static SfxItemPropertyMap aDDEFieldPropMap          [] =
                {
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDDEFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DROPDOWN :
            {
                static SfxItemPropertyMap aDropDownMap          [] =
                {
                    {SW_PROP_NMID(UNO_NAME_ITEMS), FIELD_PROP_STRINGS, CPPU_E2T(CPPUTYPE_OUSTRINGS), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SELITEM), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NAME), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDropDownMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_HIDDEN_PARA           :
            {
                static SfxItemPropertyMap aHiddenParaFieldPropMap   [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONDITION),FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_HIDDEN) ,  FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),   PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aHiddenParaFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOC_INFO              :
            {
                static SfxItemPropertyMap aDocInfoFieldPropMap      [] =
                {
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INFO_FORMAT),    FIELD_PROP_USHORT2, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_INFO_TYPE),  FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocInfoFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TEMPLATE_NAME         :
            {
                static SfxItemPropertyMap aTmplNameFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_FILE_FORMAT), FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTmplNameFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_USER_EXT              :
            {
                static SfxItemPropertyMap aUsrExtFieldPropMap       [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),           FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_USER_DATA_TYPE), FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId]= aUsrExtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_SET          :
            {
                static SfxItemPropertyMap aRefPgSetFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_OFFSET),     FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_ON),     FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aRefPgSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_REF_PAGE_GET          :
            {
                static SfxItemPropertyMap aRefPgGetFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aRefPgGetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_JUMP_EDIT             :
            {
                static SfxItemPropertyMap aJumpEdtFieldPropMap      [] =
                {
                    {SW_PROP_NMID(UNO_NAME_HINT),               FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_PLACEHOLDER),        FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_PLACEHOLDER_TYPE), FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),     PROPERTY_NONE,  0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aJumpEdtFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_SCRIPT                :
            {
                static SfxItemPropertyMap aScriptFieldPropMap       [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),        FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SCRIPT_TYPE),    FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_URL_CONTENT),    FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),       PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aScriptFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NEXT_SET     :
            {
                static SfxItemPropertyMap aDBNextSetFieldPropMap    [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONDITION)   ,     FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDBNextSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NUM_SET      :
            {
                static SfxItemPropertyMap aDBNumSetFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONDITION),        FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME   ), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME  ), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SET_NUMBER       ), FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,  0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDBNumSetFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_SET_NUM      :
            {
                static SfxItemPropertyMap aDBSetNumFieldPropMap [] =
                {
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),       FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_SET_NUMBER       ), FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,  0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDBSetNumFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE              :
            {
                static SfxItemPropertyMap aDBFieldPropMap           [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_DATA_BASE_FORMAT),FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),      FIELD_PROP_FORMAT, CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDBFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DATABASE_NAME         :
            {
                static SfxItemPropertyMap aDBNameFieldPropMap       [] =
                {
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME ) , FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME) , FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_VISIBLE),       FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN),    PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDBNameFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCSTAT:
            {
                static SfxItemPropertyMap aDocstatFieldPropMap      [] =
                {
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_TYPE),     FIELD_PROP_USHORT2, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                //  {UNO_NAME_STATISTIC_TYPE_ID,FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16),   PROPERTY_NONE,  0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocstatFieldPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_AUTHOR:
            {
                static SfxItemPropertyMap aDocInfoAuthorPropMap         [] =
                {
                    {SW_PROP_NMID(UNO_NAME_AUTHOR), FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocInfoAuthorPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_DATE_TIME:
            {
                static SfxItemPropertyMap aDocInfoDateTimePropMap           [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATE_TIME_VALUE),        FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_DATE),    FIELD_PROP_BOOL2,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),FIELD_PROP_FORMAT,    CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocInfoDateTimePropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_EDIT_TIME     :
            {
                static SfxItemPropertyMap aDocInfoEditTimePropMap           [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATE_TIME_VALUE),        FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT),FIELD_PROP_FORMAT,    CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),       FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED_LANGUAGE), FIELD_PROP_BOOL4, CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocInfoEditTimePropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_MISC:
            {
                static SfxItemPropertyMap aDocInfoStringContentPropMap          [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT),    FIELD_PROP_PAR1,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocInfoStringContentPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DOCINFO_REVISION          :
            {
                static SfxItemPropertyMap aDocInfoRevisionPropMap   [] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR3, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_REVISION),   FIELD_PROP_USHORT1, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_IS_FIXED),   FIELD_PROP_BOOL1,   CPPU_E2T(CPPUTYPE_BOOLEAN)  , PROPERTY_NONE,0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocInfoRevisionPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_COMBINED_CHARACTERS:
            {
                static SfxItemPropertyMap aCombinedCharactersPropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_CONTENT), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aCombinedCharactersPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_TABLE_FORMULA:
            {
                static SfxItemPropertyMap aTableFormulaPropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_CURRENT_PRESENTATION), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),  PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_SHOW_FORMULA), FIELD_PROP_BOOL1,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBER_FORMAT), FIELD_PROP_FORMAT,   CPPU_E2T(CPPUTYPE_INT32), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTableFormulaPropMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_DUMMY_0:
            {
                static SfxItemPropertyMap aEmptyPropMap         [] =
                {
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aEmptyPropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_USER :
            {
                static SfxItemPropertyMap aUserFieldTypePropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_EXPRESSION),      FIELD_PROP_BOOL1,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,   0},
                    {SW_PROP_NMID(UNO_NAME_NAME),               FIELD_PROP_PAR1,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::MAYBEVOID, 0},
                    {SW_PROP_NMID(UNO_NAME_VALUE),          FIELD_PROP_DOUBLE,  CPPU_E2T(CPPUTYPE_DOUBLE), PROPERTY_NONE,   0},
                    {SW_PROP_NMID(UNO_NAME_CONTENT),            FIELD_PROP_PAR2,    CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aUserFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DDE       :
            {
                static SfxItemPropertyMap aDDEFieldTypePropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_DDE_COMMAND_ELEMENT ), FIELD_PROP_PAR2,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DDE_COMMAND_FILE    ), FIELD_PROP_PAR4,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DDE_COMMAND_TYPE    ), FIELD_PROP_SUBTYPE,   CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_AUTOMATIC_UPDATE), FIELD_PROP_BOOL1,  CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_NAME),               FIELD_PROP_PAR1,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDDEFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_SET_EXP     :
            {
                static SfxItemPropertyMap aSetExpFieldTypePropMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_CHAPTER_NUMBERING_LEVEL),FIELD_PROP_SHORT1,  CPPU_E2T(CPPUTYPE_INT8), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_NAME),               FIELD_PROP_PAR1,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_NUMBERING_SEPARATOR), FIELD_PROP_PAR2,   CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_SUB_TYPE),           FIELD_PROP_SUBTYPE, CPPU_E2T(CPPUTYPE_INT16), PROPERTY_NONE,    0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aSetExpFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DATABASE    :
            {
                static SfxItemPropertyMap aDBFieldTypePropMap           [] =
                {
                    {SW_PROP_NMID(UNO_NAME_DATA_BASE_NAME   ), FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COMMAND_TYPE), FIELD_PROP_SHORT1, CPPU_E2T(CPPUTYPE_INT32),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_TABLE_NAME  ), FIELD_PROP_PAR4, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DATA_COLUMN_NAME ), FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  FIELD_PROP_PROP_SEQ,    CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR3,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDBFieldTypePropMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_DUMMY0      :
            {
                static SfxItemPropertyMap aStandardFieldMasterMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_DEPENDENT_TEXT_FIELDS),  0,  CPPU_E2T(CPPUTYPE_SEQDEPTXTFLD), PropertyAttribute::READONLY, 0},
                    {SW_PROP_NMID(UNO_NAME_NAME),               0,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      0,  CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aStandardFieldMasterMap;
            }
            break;
            case PROPERTY_MAP_FLDTYP_BIBLIOGRAPHY:
            {
                static SfxItemPropertyMap aBibliographyFieldMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_FIELDS  )    , FIELD_PROP_PROP_SEQ, CPPU_E2T(CPPUTYPE_PROPERTYVALUE),PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aBibliographyFieldMap;
            }
            break;
            case PROPERTY_MAP_FLDMSTR_BIBLIOGRAPHY:
            {
                static SfxItemPropertyMap aBibliographyFieldMasterMap[] =
                {
                    {SW_PROP_NMID(UNO_NAME_BRACKET_BEFORE     ) , FIELD_PROP_PAR1, CPPU_E2T(CPPUTYPE_OUSTRING),               PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_BRACKET_AFTER      ) , FIELD_PROP_PAR2, CPPU_E2T(CPPUTYPE_OUSTRING),               PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_NUMBER_ENTRIES  ) , FIELD_PROP_BOOL1, CPPU_E2T(CPPUTYPE_BOOLEAN),                    PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_IS_SORT_BY_POSITION) , FIELD_PROP_BOOL2, CPPU_E2T(CPPUTYPE_BOOLEAN),                    PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_LOCALE),            FIELD_PROP_LOCALE,  CPPU_E2T(CPPUTYPE_LOCALE)  , PROPERTY_NONE,     0},
                    {SW_PROP_NMID(UNO_NAME_SORT_ALGORITHM),    FIELD_PROP_PAR3,  CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_NONE,     0},
                    {SW_PROP_NMID(UNO_NAME_SORT_KEYS          ) , FIELD_PROP_PROP_SEQ, CPPU_E2T(CPPUTYPE_PROPERTYVALUES),   PROPERTY_NONE, 0},
                    {SW_PROP_NMID(UNO_NAME_INSTANCE_NAME),      FIELD_PROP_PAR4,    CPPU_E2T(CPPUTYPE_OUSTRING), PropertyAttribute::READONLY, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aBibliographyFieldMasterMap;
            }
            break;
            case PROPERTY_MAP_TEXT :
            {
                static SfxItemPropertyMap aTextMap[] =
                {
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTextMap;
            }
            break;
            case PROPERTY_MAP_MAILMERGE :
            {
                static SfxItemPropertyMap aMailMergeMap[] =
                {
                    { SW_PROP_NMID(UNO_NAME_SELECTION),             WID_SELECTION,              CPPU_E2T(CPPUTYPE_SEQANY),      PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_RESULT_SET),            WID_RESULT_SET,             CPPU_E2T(CPPUTYPE_REFRESULTSET), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_CONNECTION),            WID_CONNECTION,             CPPU_E2T(CPPUTYPE_REFCONNECTION), PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_MODEL),                 WID_MODEL,                  CPPU_E2T(CPPUTYPE_REFMODEL),    PropertyAttribute::READONLY, 0},
                    { SW_PROP_NMID(UNO_NAME_DATA_SOURCE_NAME),      WID_DATA_SOURCE_NAME,       CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DAD_COMMAND),           WID_DATA_COMMAND,           CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_FILTER),                WID_FILTER,                 CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DOCUMENT_URL),          WID_DOCUMENT_URL,           CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_OUTPUT_URL),            WID_OUTPUT_URL,             CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_DAD_COMMAND_TYPE),      WID_DATA_COMMAND_TYPE,      CPPU_E2T(CPPUTYPE_INT32),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_OUTPUT_TYPE),           WID_OUTPUT_TYPE,            CPPU_E2T(CPPUTYPE_INT16),       PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_ESCAPE_PROCESSING),     WID_ESCAPE_PROCESSING,      CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_SINGLE_PRINT_JOBS),     WID_SINGLE_PRINT_JOBS,      CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_FILE_NAME_FROM_COLUMN), WID_FILE_NAME_FROM_COLUMN,  CPPU_E2T(CPPUTYPE_BOOLEAN),     PROPERTY_NONE, 0},
                    { SW_PROP_NMID(UNO_NAME_FILE_NAME_PREFIX),      WID_FILE_NAME_PREFIX,       CPPU_E2T(CPPUTYPE_OUSTRING),    PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aMailMergeMap;
            }
            break;

            default:
                DBG_ERROR( "unexpected property map ID" );
        }
        Sort(nPropertyId);
    }
    return aMapArr[nPropertyId];
}
/* -----------------04.07.98 11:42-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwItemPropertySet::FillItem(SfxItemSet& rSet, sal_uInt16 nWhich, sal_Bool bGetProperty) const
{
    sal_Bool bRet = sal_False;
/*  if(nWhich == SID_ATTR_PAGE_PAPERBIN)
    {
        rSet.Put(SvxPaperBinItem(SID_ATTR_PAGE_PAPERBIN, 0));
        bRet = sal_True;
    }*/
    return bRet;
}
