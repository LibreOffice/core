/*************************************************************************
 *
 *  $RCSfile: unomap.cxx,v $
 *
 *  $Revision: 1.51 $
 *
 *  last change: $Author: os $ $Date: 2001-02-19 10:37:13 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <swtypes.hxx>
#include <hintids.hxx>
#include <cmdid.h>


#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
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
#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACINGMODE_HPP_
#include <com/sun/star/style/LineSpacingMode.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHADJUST_HPP_
#include <com/sun/star/style/ParagraphAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_DROPCAPFORMAT_HPP_
#include <com/sun/star/style/DropCapFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_TABSTOP_HPP_
#include <com/sun/star/style/TabStop.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_TABALIGN_HPP_
#include <com/sun/star/style/TabAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_PAGESTYLELAYOUT_HPP_
#include <com/sun/star/style/PageStyleLayout.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_BREAKTYPE_HPP_
#include <com/sun/star/style/BreakType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XMODULE_HPP_
#include <com/sun/star/text/XModule.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGEMOVER_HPP_
#include <com/sun/star/text/XTextRangeMover.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTESSETTINGSSUPPLIER_HPP_
#include <com/sun/star/text/XFootnotesSettingsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTE_HPP_
#include <com/sun/star/text/XFootnote.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTESSUPPLIER_HPP_
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XENDNOTESSUPPLIER_HPP_
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XENDNOTESSETTINGSSUPPLIER_HPP_
#include <com/sun/star/text/XEndnotesSettingsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_FOOTNOTENUMBERING_HPP_
#include <com/sun/star/text/FootnoteNumbering.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTIONSSUPPLIER_HPP_
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTION_HPP_
#include <com/sun/star/text/XTextSection.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SECTIONFILELINK_HPP_
#include <com/sun/star/text/SectionFileLink.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_RELORIENTATION_HPP_
#include <com/sun/star/text/RelOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATION_HPP_
#include <com/sun/star/text/VertOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIZONTALADJUST_HPP_
#include <com/sun/star/text/HorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_DOCUMENTSTATISTIC_HPP_
#include <com/sun/star/text/DocumentStatistic.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/HoriOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_NOTEPRINTMODE_HPP_
#include <com/sun/star/text/NotePrintMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SIZETYPE_HPP_
#include <com/sun/star/text/SizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_VERTORIENTATIONFORMAT_HPP_
#include <com/sun/star/text/VertOrientationFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SETVARIABLETYPE_HPP_
#include <com/sun/star/text/SetVariableType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_USERDATAPART_HPP_
#include <com/sun/star/text/UserDataPart.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_CHAPTERFORMAT_HPP_
#include <com/sun/star/text/ChapterFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_PLACEHOLDERTYPE_HPP_
#include <com/sun/star/text/PlaceholderType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEMPLATEDISPLAYFORMAT_HPP_
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_USERFIELDFORMAT_HPP_
#include <com/sun/star/text/UserFieldFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_PAGENUMBERTYPE_HPP_
#include <com/sun/star/text/PageNumberType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_REFERENCEFIELDPART_HPP_
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_FILENAMEDISPLAYFORMAT_HPP_
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDEPENDENTTEXTFIELD_HPP_
#include <com/sun/star/text/XDependentTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTGRAPHICOBJECTSSUPPLIER_HPP_
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLECURSOR_HPP_
#include <com/sun/star/text/XTextTableCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLESSUPPLIER_HPP_
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TABLECOLUMNSEPARATOR_HPP_
#include <com/sun/star/text/TableColumnSeparator.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTTABLE_HPP_
#include <com/sun/star/text/XTextTable.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEXMARK_HPP_
#include <com/sun/star/text/XDocumentIndexMark.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEXESSUPPLIER_HPP_
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCOLUMNSEQUENCE_HPP_
#include <com/sun/star/text/TextColumnSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAMESSUPPLIER_HPP_
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFRAME_HPP_
#include <com/sun/star/text/XTextFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XVETOABLECHANGELISTENER_HPP_
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATECHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PropertyAttribute_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATECHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyStateChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWFORMAT_HPP_
#include <com/sun/star/table/ShadowFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_TABLEBORDER_HPP_
#include <com/sun/star/table/TableBorder.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XFORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _SVX_PBINITEM_HXX //autogen
#include <svx/pbinitem.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::i18n;
using namespace ::rtl;

SwUnoPropertyMapProvider aSwMapProvider;

#ifndef MID_TXT_LMARGIN
#define MID_TXT_LMARGIN 11
#endif
/******************************************************************************
    UI-Maps
******************************************************************************/
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
    SfxItemPropertySet* pRet = 0;
    switch(nPropSetId)
    {
        case PROPERTY_SET_CHAR_STYLE:
            if(!pCharStyleMap)
                pCharStyleMap = new SwItemPropertySet(
                    GetPropertyMap(PROPERTY_MAP_CHAR_STYLE ));
            pRet = pCharStyleMap;
        break;
        case PROPERTY_SET_PARA_STYLE:
            if(!pParaStyleMap)
                pParaStyleMap = new SwItemPropertySet(
                    GetPropertyMap(PROPERTY_MAP_PARA_STYLE ));
            pRet = pParaStyleMap;
        break;
        case PROPERTY_SET_FRAME_STYLE:
            if(!pFrameStyleMap)
                pFrameStyleMap = new SwItemPropertySet(
                GetPropertyMap(PROPERTY_MAP_FRAME_STYLE ));
            pRet = pFrameStyleMap;
        break;
        case PROPERTY_SET_PAGE_STYLE:
            if(!pPageStyleMap)
            pPageStyleMap = new SwItemPropertySet(
                GetPropertyMap(PROPERTY_MAP_PAGE_STYLE ));
            pRet = pPageStyleMap;
        break;
        case PROPERTY_SET_NUM_STYLE:
            if(!pNumStyleMap)
                pNumStyleMap = new SwItemPropertySet(
                    GetPropertyMap(PROPERTY_MAP_NUM_STYLE ));
            pRet = pNumStyleMap;
        break;
    }
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
    int nDiff = strcmp( ((const SfxItemPropertyMap*)pSmaller)->pName, ((const SfxItemPropertyMap*)pBigger)->pName );
    return nDiff;

}
/* -----------------24.06.98 18:12-------------------
 *
 * --------------------------------------------------*/
void SwUnoPropertyMapProvider::Sort(sal_uInt16 nId)
{
    SfxItemPropertyMap* pTemp = aMapArr[nId];
    sal_uInt16 i = 0;
    while( pTemp[i].pName )
    {
        i++;
    }
    qsort(aMapArr[nId], i, sizeof(SfxItemPropertyMap), lcl_CompareMap);
}

#define _STANDARD_FONT_PROPERTIES \
    { SW_PROP_NAME(UNO_NAME_CHAR_HEIGHT),           RES_CHRATR_FONTSIZE  ,  &::getCppuType((Float*)0),          PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},      \
    { SW_PROP_NAME(UNO_NAME_CHAR_WEIGHT),           RES_CHRATR_WEIGHT    ,  &::getCppuType((Float*)0),              PropertyAttribute::MAYBEVOID, MID_WEIGHT},                    \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_NAME),        RES_CHRATR_FONT,        &::getCppuType((OUString*)0),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },                  \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_STYLE_NAME),  RES_CHRATR_FONT,        &::getCppuType((OUString*)0), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                    \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_FAMILY),      RES_CHRATR_FONT,        &::getCppuType((sal_Int16*)0),                  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_CHAR_SET),    RES_CHRATR_FONT,        &::getCppuType((sal_Int16*)0),  PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                    \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_PITCH),       RES_CHRATR_FONT,        &::getCppuType((sal_Int16*)0),                  PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NAME(UNO_NAME_CHAR_POSTURE),          RES_CHRATR_POSTURE   ,  &::getCppuType((FontSlant*)0),          PropertyAttribute::MAYBEVOID, MID_POSTURE},                   \
    { SW_PROP_NAME(UNO_NAME_CHAR_LOCALE),           RES_CHRATR_LANGUAGE ,   &::getCppuType((Locale*)0)  ,       PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _CJK_FONT_PROPERTIES \
    { SW_PROP_NAME(UNO_NAME_CHAR_HEIGHT_ASIAN),         RES_CHRATR_CJK_FONTSIZE  ,  &::getCppuType((Float*)0),          PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},   \
    { SW_PROP_NAME(UNO_NAME_CHAR_WEIGHT_ASIAN),         RES_CHRATR_CJK_WEIGHT    ,  &::getCppuType((Float*)0),              PropertyAttribute::MAYBEVOID, MID_WEIGHT},                 \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_NAME_ASIAN),      RES_CHRATR_CJK_FONT,        &::getCppuType((OUString*)0),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },               \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN),RES_CHRATR_CJK_FONT,        &::getCppuType((OUString*)0), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },                 \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_FAMILY_ASIAN),    RES_CHRATR_CJK_FONT,        &::getCppuType((sal_Int16*)0),                  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN),  RES_CHRATR_CJK_FONT,        &::getCppuType((sal_Int16*)0),  PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },                 \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_PITCH_ASIAN),     RES_CHRATR_CJK_FONT,        &::getCppuType((sal_Int16*)0),                  PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NAME(UNO_NAME_CHAR_POSTURE_ASIAN),        RES_CHRATR_CJK_POSTURE   ,  &::getCppuType((FontSlant*)0),          PropertyAttribute::MAYBEVOID, MID_POSTURE},                \
    { SW_PROP_NAME(UNO_NAME_CHAR_LOCALE_ASIAN),         RES_CHRATR_CJK_LANGUAGE ,   &::getCppuType((Locale*)0)  ,       PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#define _CTL_FONT_PROPERTIES \
    { SW_PROP_NAME(UNO_NAME_CHAR_HEIGHT_COMPLEX),           RES_CHRATR_CTL_FONTSIZE  ,  &::getCppuType((Float*)0),          PropertyAttribute::MAYBEVOID, MID_FONTHEIGHT|CONVERT_TWIPS},\
    { SW_PROP_NAME(UNO_NAME_CHAR_WEIGHT_COMPLEX),           RES_CHRATR_CTL_WEIGHT    ,  &::getCppuType((Float*)0),              PropertyAttribute::MAYBEVOID, MID_WEIGHT},              \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_NAME_COMPLEX),        RES_CHRATR_CTL_FONT,        &::getCppuType((OUString*)0),  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY_NAME },            \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX),  RES_CHRATR_CTL_FONT,        &::getCppuType((OUString*)0), PropertyAttribute::MAYBEVOID, MID_FONT_STYLE_NAME },              \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_FAMILY_COMPLEX),      RES_CHRATR_CTL_FONT,        &::getCppuType((sal_Int16*)0),                  PropertyAttribute::MAYBEVOID, MID_FONT_FAMILY   },    \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX),    RES_CHRATR_CTL_FONT,        &::getCppuType((sal_Int16*)0),  PropertyAttribute::MAYBEVOID, MID_FONT_CHAR_SET },              \
    { SW_PROP_NAME(UNO_NAME_CHAR_FONT_PITCH_COMPLEX),       RES_CHRATR_CTL_FONT,        &::getCppuType((sal_Int16*)0),                  PropertyAttribute::MAYBEVOID, MID_FONT_PITCH   },     \
    { SW_PROP_NAME(UNO_NAME_CHAR_POSTURE_COMPLEX),          RES_CHRATR_CTL_POSTURE   ,  &::getCppuType((FontSlant*)0),          PropertyAttribute::MAYBEVOID, MID_POSTURE},             \
    { SW_PROP_NAME(UNO_NAME_CHAR_LOCALE_COMPLEX),           RES_CHRATR_CTL_LANGUAGE ,   &::getCppuType((Locale*)0)  ,       PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
#define _REDLINE_NODE_PROPERTIES \
    { SW_PROP_NAME(UNO_NAME_START_REDLINE),         FN_UNO_REDLINE_NODE_START,  new uno::Type(::getCppuType((Sequence<PropertyValue>*)0))  ,    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff },\
    { SW_PROP_NAME(UNO_NAME_END_REDLINE),           FN_UNO_REDLINE_NODE_END,    new uno::Type(::getCppuType((Sequence<PropertyValue>*)0))  ,        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff },
#else
#define _REDLINE_NODE_PROPERTIES \
    { SW_PROP_NAME(UNO_NAME_START_REDLINE),         FN_UNO_REDLINE_NODE_START , &::getCppuType((Sequence<PropertyValue>*)0)  ,      PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff }, \
    { SW_PROP_NAME(UNO_NAME_END_REDLINE),           FN_UNO_REDLINE_NODE_END ,   &::getCppuType((Sequence<PropertyValue>*)0)  ,          PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,  0xff },
#endif

#define _REDLINE_PROPERTIES \
    {SW_PROP_NAME(UNO_NAME_REDLINE_AUTHOR),     0, &::getCppuType((OUString*)0),                        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NAME(UNO_NAME_REDLINE_DATE_TIME),  0, &::getCppuType((util::DateTime*)0),                  PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NAME(UNO_NAME_REDLINE_COMMENT),    0, &::getCppuType((OUString*)0),                        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NAME(UNO_NAME_REDLINE_TYPE),       0, &::getCppuType((OUString*)0),                        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NAME(UNO_NAME_REDLINE_SUCCESSOR_DATA),  0, &::getCppuType((Sequence<PropertyValue>*)0),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},\
    {SW_PROP_NAME(UNO_NAME_REDLINE_IDENTIFIER), 0, &::getCppuType((OUString*)0),                        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},\
    {SW_PROP_NAME(UNO_NAME_REDLINE_TEXT  ), 0, &::getCppuType((Reference<XText>*)0),                    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},

/* -----------------24.06.98 18:12-------------------
 *
 * --------------------------------------------------*/
#define COMMON_CRSR_PARA_PROPERTIES \
        { SW_PROP_NAME(UNO_NAME_PARA_IS_HYPHENATION                ),   RES_PARATR_HYPHENZONE,      &::getBooleanCppuType(),    PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },                                        \
        { SW_PROP_NAME(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS ),   RES_PARATR_HYPHENZONE,      &::getCppuType((const sal_Int16*)0),    PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },                              \
        { SW_PROP_NAME(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS),   RES_PARATR_HYPHENZONE,      &::getCppuType((const sal_Int16*)0),    PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },                              \
        { SW_PROP_NAME(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS       ),   RES_PARATR_HYPHENZONE,      &::getCppuType((const sal_Int16*)0),    PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},                              \
        { SW_PROP_NAME(UNO_NAME_BREAK_TYPE),                RES_BREAK,              &::getCppuType((const style::BreakType*)0),             PropertyAttribute::MAYBEVOID, 0},                                                          \
        { SW_PROP_NAME(UNO_NAME_CHAR_AUTO_KERNING),         RES_CHRATR_AUTOKERN  ,  &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_CHAR_BACK_COLOR),               RES_CHRATR_BACKGROUND,  &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR         },                                           \
        { SW_PROP_NAME(UNO_NAME_PARA_BACK_COLOR),               RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR         },                                           \
        { SW_PROP_NAME(UNO_NAME_CHAR_CASE_MAP),             RES_CHRATR_CASEMAP,     &::getCppuType((const sal_Int16*)0),            PropertyAttribute::MAYBEVOID, 0},                                                            \
        { SW_PROP_NAME(UNO_NAME_CHAR_COLOR),                    RES_CHRATR_COLOR,       &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID, 0},                                                           \
        { SW_PROP_NAME(UNO_NAME_CHAR_STRIKEOUT),            RES_CHRATR_CROSSEDOUT,  &::getCppuType((const sal_Int16*)0),                    PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},                                              \
        { SW_PROP_NAME(UNO_NAME_CHAR_CROSSED_OUT),          RES_CHRATR_CROSSEDOUT,  &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID, MID_CROSSED_OUT},                                                       \
        { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT),           RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int16*)0),            PropertyAttribute::MAYBEVOID, MID_ESC           },                                                   \
        { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT),        RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int8*)0)  ,       PropertyAttribute::MAYBEVOID, MID_ESC_HEIGHT},                                               \
        { SW_PROP_NAME(UNO_NAME_CHAR_AUTO_ESCAPEMENT),          RES_CHRATR_ESCAPEMENT,  &::getBooleanCppuType()  ,              PropertyAttribute::MAYBEVOID, MID_AUTO_ESC  },                                                \
        { SW_PROP_NAME(UNO_NAME_CHAR_FLASH              ),  RES_CHRATR_BLINK    ,   &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE),            RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int16*)0),            PropertyAttribute::MAYBEVOID, MID_UNDERLINE},                                                \
        { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_COLOR),              RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID, MID_UL_COLOR},                                                \
        { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR),              RES_CHRATR_UNDERLINE ,  &::getBooleanCppuType(),            PropertyAttribute::MAYBEVOID, MID_UL_HASCOLOR},                                                \
        { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_URL    },                                          \
        { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_FILTER    },                                       \
        { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_LOCATION),         RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},                                \
        { SW_PROP_NAME(UNO_NAME_PARA_LEFT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID, MID_TXT_LMARGIN|CONVERT_TWIPS},                                   \
        { SW_PROP_NAME(UNO_NAME_PARA_RIGHT_MARGIN),             RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},                                  \
        { SW_PROP_NAME(UNO_NAME_PARA_FIRST_LINE_INDENT),        RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},                         \
        _STANDARD_FONT_PROPERTIES \
        _CJK_FONT_PROPERTIES \
        _CTL_FONT_PROPERTIES \
        { SW_PROP_NAME(UNO_NAME_CHAR_KERNING            ),  RES_CHRATR_KERNING    , &::getCppuType((const sal_Int16*)0)  ,          PropertyAttribute::MAYBEVOID,   0},                                                           \
        { SW_PROP_NAME(UNO_NAME_CHAR_NO_HYPHENATION     ),  RES_CHRATR_NOHYPHEN ,   &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_CHAR_SHADOWED),             RES_CHRATR_SHADOWED  ,  &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID, 0},                                                                     \
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),           RES_TXTATR_CHARFMT,     &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID,     0},                                                         \
        { SW_PROP_NAME(UNO_NAME_CHAR_CONTOURED),                RES_CHRATR_CONTOUR,     &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID, 0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_DROP_CAP_FORMAT),           RES_PARATR_DROP,        &::getCppuType((const style::DropCapFormat*)0)  , PropertyAttribute::MAYBEVOID, MID_DROPCAP_FORMAT|CONVERT_TWIPS},                        \
        { SW_PROP_NAME(UNO_NAME_DROP_CAP_WHOLE_WORD),       RES_PARATR_DROP,        &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID, MID_DROPCAP_WHOLE_WORD },                                               \
        { SW_PROP_NAME(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME),  RES_PARATR_DROP,        &::getCppuType((const OUString*)0)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },                                  \
        { SW_PROP_NAME(UNO_NAME_PARA_KEEP_TOGETHER  ),      RES_KEEP,               &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_PARA_SPLIT      ),          RES_PARATR_SPLIT,       &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_PARA_WIDOWS),               RES_PARATR_WIDOWS,      &::getCppuType((const sal_Int8*)0),PropertyAttribute::MAYBEVOID,     0},                                                                  \
        { SW_PROP_NAME(UNO_NAME_PARA_ORPHANS),              RES_PARATR_ORPHANS,      &::getCppuType((const sal_Int8*)0),PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_PAGE_DESC_NAME),            RES_PAGEDESC,           &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },                                    \
        { SW_PROP_NAME(UNO_NAME_PAGE_NUMBER_OFFSET),            RES_PAGEDESC,           &::getCppuType((const sal_Int16*)0),        PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},                                   \
        { SW_PROP_NAME(UNO_NAME_PARA_ADJUST),                   RES_PARATR_ADJUST,      &::getCppuType((const sal_Int16*)0),        PropertyAttribute::MAYBEVOID, MID_PARA_ADJUST},                                              \
        { SW_PROP_NAME(UNO_NAME_PARA_EXPAND_SINGLE_WORD),   RES_PARATR_ADJUST,      &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID, MID_EXPAND_SINGLE   },                                                  \
        { SW_PROP_NAME(UNO_NAME_PARA_LAST_LINE_ADJUST),       RES_PARATR_ADJUST, &::getCppuType((const sal_Int16*)0),               PropertyAttribute::MAYBEVOID, MID_LAST_LINE_ADJUST},                                         \
        { SW_PROP_NAME(UNO_NAME_PARA_LINE_NUMBER_COUNT  ),  RES_LINENUMBER,     &::getBooleanCppuType(),                PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_COUNT      },                                                \
        { SW_PROP_NAME(UNO_NAME_PARA_LINE_NUMBER_START_VALUE),RES_LINENUMBER, &::getCppuType((const sal_Int32*)0),                  PropertyAttribute::MAYBEVOID ,MID_LINENUMBER_STARTVALUE},                                    \
        { SW_PROP_NAME(UNO_NAME_PARA_LINE_SPACING),             RES_PARATR_LINESPACING, &::getCppuType((const style::LineSpacing*)0),       PropertyAttribute::MAYBEVOID,     CONVERT_TWIPS},                                      \
        { SW_PROP_NAME(UNO_NAME_PARA_REGISTER_MODE_ACTIVE),     RES_PARATR_REGISTER,&::getBooleanCppuType()  ,              PropertyAttribute::MAYBEVOID, 0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_PARA_TOP_MARGIN),           RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID, MID_UP_MARGIN|CONVERT_TWIPS},                                  \
        { SW_PROP_NAME(UNO_NAME_PARA_BOTTOM_MARGIN),            RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID, MID_LO_MARGIN|CONVERT_TWIPS},                                 \
        { SW_PROP_NAME(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND, &::getBooleanCppuType(),             PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT        },                                                \
        { SW_PROP_NAME(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND, &::getBooleanCppuType(),                    PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT        },                                                \
        { SW_PROP_NAME(UNO_NAME_PARA_STYLE_NAME),           FN_UNO_PARA_STYLE,      &::getCppuType((const OUString*)0),                 PropertyAttribute::MAYBEVOID,     0},                                                       \
        { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_NAME),                   FN_UNO_PAGE_STYLE,      &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},                       \
        { SW_PROP_NAME(UNO_NAME_NUMBERING_STYLE_NAME),      RES_PARATR_NUMRULE,     &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID,   0},                                                           \
        { SW_PROP_NAME(UNO_NAME_WORD_MODE           ),          RES_CHRATR_WORDLINEMODE,&::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID,     0},                                                             \
        { SW_PROP_NAME(UNO_NAME_NUMBERING_IS_NUMBER),       FN_UNO_IS_NUMBER,       &::getBooleanCppuType()  ,          PropertyAttribute::MAYBEVOID,     0},                                                                 \
        { SW_PROP_NAME(UNO_NAME_NUMBERING_LEVEL),           FN_UNO_NUM_LEVEL,       &::getCppuType((const sal_Int16*)0),            PropertyAttribute::MAYBEVOID, 0},                                                            \
        { SW_PROP_NAME(UNO_NAME_NUMBERING_RULES),           FN_UNO_NUM_RULES,       &::getCppuType((const uno::Reference<container::XIndexReplace>*)0),     PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                        \
        { SW_PROP_NAME(UNO_NAME_NUMBERING_START_VALUE),     FN_UNO_NUM_START_VALUE, &::getCppuType((const sal_Int16*)0),            PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},                                                \
        { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX),            FN_UNO_DOCUMENT_INDEX,  &::getCppuType((const uno::Reference<text::XDocumentIndex>*)0), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },            \
        { SW_PROP_NAME(UNO_NAME_TEXT_TABLE),                FN_UNO_TEXT_TABLE,      &::getCppuType((const uno::Reference<text::XTextTable>*)0),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },               \
        { SW_PROP_NAME(UNO_NAME_CELL),                      FN_UNO_CELL,            &::getCppuType((uno::Reference<table::XCell>*)0),           PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { SW_PROP_NAME(UNO_NAME_TEXT_FRAME),                FN_UNO_TEXT_FRAME,      &::getCppuType((uno::Reference<text::XTextFrame>*)0),       PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                     \
        { SW_PROP_NAME(UNO_NAME_TEXT_SECTION),              FN_UNO_TEXT_SECTION,    &::getCppuType((uno::Reference<text::XTextSection>*)0), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },                    \
        { SW_PROP_NAME(UNO_NAME_LEFT_BORDER),                   RES_BOX,                &::getCppuType((const table::BorderLine*)0),        PropertyAttribute::MAYBEVOID, LEFT_BORDER  |CONVERT_TWIPS },                            \
        { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER),              RES_BOX,                &::getCppuType((const table::BorderLine*)0),        PropertyAttribute::MAYBEVOID, RIGHT_BORDER |CONVERT_TWIPS },                                \
        { SW_PROP_NAME(UNO_NAME_TOP_BORDER),                    RES_BOX,                &::getCppuType((const table::BorderLine*)0),        PropertyAttribute::MAYBEVOID, TOP_BORDER   |CONVERT_TWIPS },                            \
        { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER),             RES_BOX,                &::getCppuType((const table::BorderLine*)0),        PropertyAttribute::MAYBEVOID, BOTTOM_BORDER|CONVERT_TWIPS },                                \
        { SW_PROP_NAME(UNO_NAME_BORDER_DISTANCE),           RES_BOX,                &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID, BORDER_DISTANCE|CONVERT_TWIPS },                               \
        { SW_PROP_NAME(UNO_NAME_LEFT_BORDER_DISTANCE),  RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },                                                            \
        { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },                                                            \
        { SW_PROP_NAME(UNO_NAME_TOP_BORDER_DISTANCE),       RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },                                                        \
        { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER_DISTANCE),    RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },                                                        \
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_U_R_L   ),           RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_URL},                                                                 \
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_TARGET  ),           RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_TARGET},                                                              \
        { SW_PROP_NAME(UNO_NAME_HYPER_LINK_NAME ),          RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },                                                      \
        { SW_PROP_NAME(UNO_NAME_UNVISITED_CHAR_STYLE_NAME),   RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),       PROPERTY_NONE ,MID_URL_VISITED_FMT       },                                                       \
        { SW_PROP_NAME(UNO_NAME_VISITED_CHAR_STYLE_NAME),     RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),           PROPERTY_NONE ,MID_URL_UNVISITED_FMT     },                                                     \
        { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),       RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },                                   \
        { SW_PROP_NAME(UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES),  RES_TXTATR_UNKNOWN_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },                               \
        { SW_PROP_NAME(UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL), FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL,&::getCppuType((const sal_Int8*)0),   PropertyAttribute::READONLY, 0},                                                     \
        { SW_PROP_NAME(UNO_NAME_PARA_CONDITIONAL_STYLE_NAME),  FN_UNO_PARA_CONDITIONAL_STYLE_NAME, &::getCppuType((const OUString*)0),      PropertyAttribute::READONLY, 0},                                                     \
        { SW_PROP_NAME(UNO_NAME_PARA_IS_NUMBERING_RESTART),     FN_NUMBER_NEWSTART,     &::getBooleanCppuType(),    PropertyAttribute::MAYBEVOID, 0 },                                                                        \
        { SW_PROP_NAME(UNO_NAME_PARA_SHADOW_FORMAT),            RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS}, \
        { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_IS_ON),            RES_CHRATR_TWO_LINES,           &::getBooleanCppuType(),    PropertyAttribute::MAYBEVOID, MID_TWOLINES}, \
        { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_PREFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PropertyAttribute::MAYBEVOID, MID_START_BRACKET}, \
        { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_SUFFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PropertyAttribute::MAYBEVOID, MID_END_BRACKET}, \
        { SW_PROP_NAME(UNO_NAME_CHAR_EMPHASIS),                 RES_CHRATR_EMPHASIS_MARK,           &::getCppuType((const sal_Int16*)0),    PropertyAttribute::MAYBEVOID, MID_EMPHASIS},\
        { SW_PROP_NAME(UNO_NAME_PARA_IS_HANGING_PUNCTUATION),   RES_PARATR_HANGINGPUNCTUATION,  &::getBooleanCppuType(),    PROPERTY_NONE ,0     },    \
        { SW_PROP_NAME(UNO_NAME_PARA_IS_CHARACTER_DISTANCE),    RES_PARATR_SCRIPTSPACE,         &::getBooleanCppuType(),    PROPERTY_NONE ,0     },    \
        { SW_PROP_NAME(UNO_NAME_PARA_IS_FORBIDDEN_RULES),       RES_PARATR_FORBIDDEN_RULES,     &::getBooleanCppuType(),    PROPERTY_NONE ,0     }, \
        { SW_PROP_NAME(UNO_NAME_CHAR_ROTATION               ),  RES_CHRATR_ROTATE,      &::getCppuType((sal_Int16*)0),  PropertyAttribute::MAYBEVOID,     MID_ROTATE      },  \
        { SW_PROP_NAME(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE),  RES_CHRATR_ROTATE,      &::getBooleanCppuType(),    PropertyAttribute::MAYBEVOID,         MID_FITTOLINE  },  \
        { SW_PROP_NAME(UNO_NAME_CHAR_SCALE_WIDTH            ),  RES_CHRATR_SCALEW,      &::getCppuType((const sal_Int16*)0),    PropertyAttribute::MAYBEVOID,          0 },  \
        { SW_PROP_NAME(UNO_NAME_RUBY_TEXT           ),          RES_TXTATR_CJK_RUBY,    &::getCppuType((OUString*)0),   PropertyAttribute::MAYBEVOID,          MID_RUBY_TEXT },  \
        { SW_PROP_NAME(UNO_NAME_RUBY_ADJUST         ),          RES_TXTATR_CJK_RUBY,    &::getCppuType((sal_Int16*)0),  PropertyAttribute::MAYBEVOID,          MID_RUBY_ADJUST },  \
        { SW_PROP_NAME(UNO_NAME_RUBY_CHAR_STYLE_NAME),          RES_TXTATR_CJK_RUBY,    &::getCppuType((OUString*)0),   PropertyAttribute::MAYBEVOID,          MID_RUBY_CHARSTYLE },  \


#define _BASE_INDEX_PROPERTIES_\
        { SW_PROP_NAME(UNO_NAME_TITLE), WID_IDX_TITLE,  &::getCppuType((const OUString*)0)  , PROPERTY_NONE,     0},\
        { SW_PROP_NAME(UNO_NAME_CONTENT_SECTION), WID_IDX_CONTENT_SECTION,  &::getCppuType((uno::Reference<text::XTextSection>*)0)  , PropertyAttribute::READONLY,     0},\
        { SW_PROP_NAME(UNO_NAME_HEADER_SECTION), WID_IDX_HEADER_SECTION,  &::getCppuType((uno::Reference<text::XTextSection>*)0)  , PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,     0},\

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
#define ANCHOR_TYPES_PROPERTY   { SW_PROP_NAME(UNO_NAME_ANCHOR_TYPES   ),   FN_UNO_ANCHOR_TYPES, new uno::Type(::getCppuType((uno::Sequence<text::TextContentAnchorType>*)0)),PropertyAttribute::READONLY, 0xff},
#else
#define ANCHOR_TYPES_PROPERTY    { SW_PROP_NAME(UNO_NAME_ANCHOR_TYPES   ),  FN_UNO_ANCHOR_TYPES,    &::getCppuType((const uno::Sequence<text::TextContentAnchorType>*)0),PropertyAttribute::READONLY, 0xff},
#endif

#define COMMON_FRAME_PROPERTIES \
    { SW_PROP_NAME(UNO_NAME_ANCHOR_PAGE_NO),            RES_ANCHOR,             &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ANCHOR_PAGENUM       },              \
    { SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE   ),            RES_ANCHOR,             &::getCppuType((text::TextContentAnchorType*)0),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},             \
    ANCHOR_TYPES_PROPERTY\
    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },                      \
    { SW_PROP_NAME(UNO_NAME_CONTENT_PROTECTED ),        RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_CONTENT   },                          \
    { SW_PROP_NAME(UNO_NAME_FRAME_STYLE_NAME),          0,                      &::getCppuType((const OUString*)0),         PROPERTY_NONE, 0},                                   \
    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),    RES_BACKGROUND,         &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_GRAPHIC_URL    },                 \
    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),     RES_BACKGROUND,         &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_GRAPHIC_FILTER    },              \
    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ), RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},    \
    { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN),           RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},            \
    { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},            \
    { SW_PROP_NAME(UNO_NAME_WIDTH),                     RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS},\
    { SW_PROP_NAME(UNO_NAME_HEIGHT),                    RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS},\
    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT  ),             RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },             \
    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_POSITION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    }, \
    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_RELATION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_RELATION  },               \
    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_U_R_L   ),       RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_URL},                         \
    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_TARGET  ),       RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_TARGET},                      \
    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_NAME ),      RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },                 \
    { SW_PROP_NAME(UNO_NAME_OPAQUE),                    RES_OPAQUE,             &::getBooleanCppuType(),            PROPERTY_NONE, 0},                                        \
    { SW_PROP_NAME(UNO_NAME_PAGE_TOGGLE),               RES_HORI_ORIENT,        &::getBooleanCppuType(),            PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },               \
    { SW_PROP_NAME(UNO_NAME_POSITION_PROTECTED),    RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_POSITION},                      \
    { SW_PROP_NAME(UNO_NAME_PRINT),                     RES_PRINT,              &::getBooleanCppuType(),            PROPERTY_NONE, 0},                                        \
    { SW_PROP_NAME(UNO_NAME_RELATIVE_HEIGHT),       RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },            \
    { SW_PROP_NAME(UNO_NAME_RELATIVE_WIDTH),            RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },         \
    { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT),             RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},             \
    { SW_PROP_NAME(UNO_NAME_SERVER_MAP      ),      RES_URL,                &::getBooleanCppuType(),            PROPERTY_NONE ,MID_URL_SERVERMAP         },                      \
    { SW_PROP_NAME(UNO_NAME_SIZE),                  RES_FRM_SIZE,           &::getCppuType((const awt::Size*)0),            PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},        \
    { SW_PROP_NAME(UNO_NAME_SIZE_PROTECTED    ),    RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_SIZE    },                      \
    { SW_PROP_NAME(UNO_NAME_SIZE_RELATIVE),             RES_FRM_SIZE,           &::getBooleanCppuType()  ,          PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_REL_SIZE   },          \
    { SW_PROP_NAME(UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT),   RES_FRM_SIZE,           &::getBooleanCppuType()  ,          PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },  \
    { SW_PROP_NAME(UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH),   RES_FRM_SIZE,           &::getBooleanCppuType()  ,          PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },     \
    { SW_PROP_NAME(UNO_NAME_TEXT_WRAP),                 RES_SURROUND,           &::getCppuType((text::WrapTextMode*)0),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },        \
    { SW_PROP_NAME(UNO_NAME_SURROUND                  ), RES_SURROUND,          &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },        \
    { SW_PROP_NAME(UNO_NAME_SURROUND_ANCHORONLY),   RES_SURROUND,           &::getBooleanCppuType(),            PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },                 \
    { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),                RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},          \
    { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),             RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},          \
    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },                    \
    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT  ),             RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },             \
    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_POSITION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    }, \
    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_RELATION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_RELATION  },               \
    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER),               RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },             \
    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER),          RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },                 \
    { SW_PROP_NAME(UNO_NAME_TOP_BORDER),                RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },             \
    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER),         RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },                 \
    { SW_PROP_NAME(UNO_NAME_BORDER_DISTANCE),         RES_BOX,              &::getCppuType((const sal_Int32*)0),    0, BORDER_DISTANCE|CONVERT_TWIPS },                       \
    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER_DISTANCE),  RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },                \
    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },                \
    { SW_PROP_NAME(UNO_NAME_TOP_BORDER_DISTANCE),       RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },            \
    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER_DISTANCE),    RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },            \
    { SW_PROP_NAME(UNO_LINK_DISPLAY_NAME),          FN_PARAM_LINK_DISPLAY_NAME,     &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0xff},                \
    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),       RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },\
    { SW_PROP_NAME(UNO_NAME_Z_ORDER),               FN_UNO_Z_ORDER,         &::getCppuType((const sal_Int32*)0),        PROPERTY_NONE, 0},



#define COMMON_TEXT_CONTENT_PROPERTIES \
                    { SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE   ),    FN_UNO_ANCHOR_TYPE, &::getCppuType((text::TextContentAnchorType*)0),                PropertyAttribute::READONLY, MID_ANCHOR_ANCHORTYPE},\
                    ANCHOR_TYPES_PROPERTY\
                    { SW_PROP_NAME(UNO_NAME_TEXT_WRAP),         FN_UNO_TEXT_WRAP,   &::getCppuType((text::WrapTextMode*)0),                 PropertyAttribute::READONLY, MID_SURROUND_SURROUNDTYPE  },


#define     _PROP_DIFF_FONTHEIGHT \
                    { SW_PROP_NAME(UNO_NAME_CHAR_PROP_HEIGHT),              RES_CHRATR_FONTSIZE ,           &::getCppuType((Float*)0),                                          PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NAME(UNO_NAME_CHAR_DIFF_HEIGHT),              RES_CHRATR_FONTSIZE ,           &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF},\
                    { SW_PROP_NAME(UNO_NAME_CHAR_PROP_HEIGHT_ASIAN),        RES_CHRATR_CJK_FONTSIZE ,           &::getCppuType((Float*)0),                                          PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NAME(UNO_NAME_CHAR_DIFF_HEIGHT_ASIAN),        RES_CHRATR_CJK_FONTSIZE ,           &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF},\
                    { SW_PROP_NAME(UNO_NAME_CHAR_PROP_HEIGHT_COMPLEX),      RES_CHRATR_CTL_FONTSIZE ,           &::getCppuType((Float*)0),                                          PROPERTY_NONE , MID_FONTHEIGHT_PROP},\
                    { SW_PROP_NAME(UNO_NAME_CHAR_DIFF_HEIGHT_COMPLEX),      RES_CHRATR_CTL_FONTSIZE ,           &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE , MID_FONTHEIGHT_DIFF},

const SfxItemPropertyMap*   SwUnoPropertyMapProvider::GetPropertyMap(sal_uInt16 nPropertyId)
{
    DBG_ASSERT(nPropertyId < PROPERTY_MAP_END, "Id ?" )
    if(!aMapArr[nPropertyId])
    {
        switch(nPropertyId)
        {
            case PROPERTY_MAP_TEXT_CURSOR:
            {
                static SfxItemPropertyMap aCharAndParaMap_Impl[] =
                {
                    COMMON_CRSR_PARA_PROPERTIES
                    { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX_MARK),           FN_UNO_DOCUMENT_INDEX_MARK, &::getCppuType((const uno::Reference<text::XDocumentIndexMark>*)0), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { SW_PROP_NAME(UNO_NAME_TEXT_FIELD),                FN_UNO_TEXT_FIELD,      &::getCppuType((const uno::Reference<text::XTextField>*)0),     PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { SW_PROP_NAME(UNO_NAME_REFERENCE_MARK),            FN_UNO_REFERENCE_MARK,  &::getCppuType((uno::Reference<text::XTextContent>*)0), PropertyAttribute::MAYBEVOID ,0 },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE),                  FN_UNO_FOOTNOTE,        &::getCppuType((uno::Reference<text::XFootnote>*)0),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    { SW_PROP_NAME(UNO_NAME_ENDNOTE),                       FN_UNO_ENDNOTE,         &::getCppuType((uno::Reference<text::XFootnote>*)0),        PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),  RES_PARATR_TABSTOP,  new uno::Type(::getCppuType((uno::Sequence<style::TabStop>*)0)),       PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#else
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),                  RES_PARATR_TABSTOP,     &::getCppuType((const uno::Sequence<style::TabStop>*)0),        PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_EVENTS ),        RES_TXTATR_INETFMT,     &::getCppuType((uno::Reference<container::XNameReplace>*)0),    PropertyAttribute::MAYBEVOID, MID_URL_HYPERLINKEVENTS},

#endif
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aCharAndParaMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH:
            {
                static SfxItemPropertyMap aParagraphMap_Impl[] =
                {
                    COMMON_CRSR_PARA_PROPERTIES
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),  RES_PARATR_TABSTOP,  new uno::Type(::getCppuType((uno::Sequence<style::TabStop>*)0)),       PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#else
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),                  RES_PARATR_TABSTOP,     &::getCppuType((const uno::Sequence<style::TabStop>*)0),        PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#endif
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aParagraphMap_Impl;
            }
            break;
            case PROPERTY_MAP_CHAR_STYLE :
            {
                static SfxItemPropertyMap aCharStyleMap [] =
                {
                    { SW_PROP_NAME(UNO_NAME_CHAR_AUTO_KERNING    ),         RES_CHRATR_AUTOKERN  ,  &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_NAME_CHAR_BACK_COLOR),               RES_CHRATR_BACKGROUND,  &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_CHAR_CASE_MAP),             RES_CHRATR_CASEMAP,     &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COLOR),                    RES_CHRATR_COLOR,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_STRIKEOUT),            RES_CHRATR_CROSSEDOUT,  &::getCppuType((const sal_Int16*)0),                    PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CROSSED_OUT),          RES_CHRATR_CROSSEDOUT,  &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT),           RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ESC          },
                    { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT),        RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE, MID_ESC_HEIGHT},
                    { SW_PROP_NAME(UNO_NAME_CHAR_FLASH           ),     RES_CHRATR_BLINK    ,   &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE),            RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, MID_UNDERLINE},
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_COLOR),              RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, MID_UL_COLOR},
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR),              RES_CHRATR_UNDERLINE ,  &::getBooleanCppuType(),            PROPERTY_NONE, MID_UL_HASCOLOR},
                    { SW_PROP_NAME(UNO_NAME_CHAR_KERNING       ),           RES_CHRATR_KERNING    , &::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE,  0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_NO_HYPHENATION     ),      RES_CHRATR_NOHYPHEN ,   &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_SHADOWED),             RES_CHRATR_SHADOWED  ,  &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CONTOURED),                RES_CHRATR_CONTOUR,     &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_WORD_MODE           ),          RES_CHRATR_WORDLINEMODE,&::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_U_R_L   ),           RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_URL},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_TARGET  ),           RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_TARGET},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_NAME ),          RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
                    { SW_PROP_NAME(UNO_NAME_UNVISITED_CHAR_STYLE_NAME),   RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),       PROPERTY_NONE ,MID_URL_VISITED_FMT       },
                    { SW_PROP_NAME(UNO_NAME_VISITED_CHAR_STYLE_NAME),     RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),           PROPERTY_NONE ,MID_URL_UNVISITED_FMT     },
                    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),   RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_IS_PHYSICAL),                  FN_UNO_IS_PHYSICAL,    &::getBooleanCppuType(), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_DISPLAY_NAME),              FN_UNO_DISPLAY_NAME, &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_IS_ON),            RES_CHRATR_TWO_LINES,           &::getBooleanCppuType(),    PROPERTY_NONE, MID_TWOLINES},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_PREFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PROPERTY_NONE, MID_START_BRACKET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_SUFFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PROPERTY_NONE, MID_END_BRACKET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_EMPHASIS),             RES_CHRATR_EMPHASIS_MARK,           &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, MID_EMPHASIS},
                    _PROP_DIFF_FONTHEIGHT
                    { SW_PROP_NAME(UNO_NAME_CHAR_ROTATION               ),  RES_CHRATR_ROTATE,      &::getCppuType((sal_Int16*)0),  PROPERTY_NONE,    MID_ROTATE      },
                    { SW_PROP_NAME(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE),  RES_CHRATR_ROTATE,      &::getBooleanCppuType(),    PROPERTY_NONE,        MID_FITTOLINE  },
                    { SW_PROP_NAME(UNO_NAME_CHAR_SCALE_WIDTH            ),  RES_CHRATR_SCALEW,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE,         0 },
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aCharStyleMap;
            }
            break;
            case PROPERTY_MAP_PARA_STYLE :
            {
                static SfxItemPropertyMap aParaStyleMap [] =
                {
                    { SW_PROP_NAME(UNO_NAME_BREAK_TYPE),                    RES_BREAK,              &::getCppuType((const style::BreakType*)0),         PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_PAGE_DESC_NAME),            RES_PAGEDESC,           &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGEDESCNAME },
                    { SW_PROP_NAME(UNO_NAME_PAGE_NUMBER_OFFSET),            RES_PAGEDESC,           &::getCppuType((const sal_Int16*)0),        PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_AUTO_KERNING    ),     RES_CHRATR_AUTOKERN  ,  &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND, &::getBooleanCppuType(),         PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_NAME_CHAR_BACK_COLOR),               RES_CHRATR_BACKGROUND,  &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_PARA_BACK_COLOR      ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_LOCATION),     RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CASE_MAP),             RES_CHRATR_CASEMAP,     &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COLOR),                    RES_CHRATR_COLOR,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_STRIKEOUT),            RES_CHRATR_CROSSEDOUT,  &::getCppuType((const sal_Int16*)0),                    PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CROSSED_OUT),          RES_CHRATR_CROSSEDOUT,  &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT),           RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ESC          },
                    { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT),        RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE, MID_ESC_HEIGHT},
                    { SW_PROP_NAME(UNO_NAME_CHAR_FLASH              ),  RES_CHRATR_BLINK    ,   &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE),            RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, MID_UNDERLINE},
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_COLOR),              RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, MID_UL_COLOR},
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR),              RES_CHRATR_UNDERLINE ,  &::getBooleanCppuType(),            PROPERTY_NONE, MID_UL_HASCOLOR},
                    { SW_PROP_NAME(UNO_NAME_PARA_LEFT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_TXT_LMARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_RIGHT_MARGIN),             RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_LEFT_MARGIN_RELATIVE),  RES_LR_SPACE,         &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,     MID_L_REL_MARGIN},
                    { SW_PROP_NAME(UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE), RES_LR_SPACE,         &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,     MID_R_REL_MARGIN},
                    { SW_PROP_NAME(UNO_NAME_PARA_FIRST_LINE_INDENT),        RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE),   RES_LR_SPACE,       &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_FIRST_LINE_REL_INDENT|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_CHAR_KERNING            ),  RES_CHRATR_KERNING    , &::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE,  0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_NO_HYPHENATION     ),  RES_CHRATR_NOHYPHEN ,   &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_SHADOWED),             RES_CHRATR_SHADOWED  ,  &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CONTOURED),                RES_CHRATR_CONTOUR,     &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_FORMAT),           RES_PARATR_DROP,        &::getCppuType((const style::DropCapFormat*)0)  , PROPERTY_NONE, MID_DROPCAP_FORMAT|CONVERT_TWIPS     },
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_WHOLE_WORD),       RES_PARATR_DROP,        &::getBooleanCppuType()  ,          PROPERTY_NONE, MID_DROPCAP_WHOLE_WORD },
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME),  RES_PARATR_DROP,        &::getCppuType((const OUString*)0)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },
                    { SW_PROP_NAME(UNO_NAME_PARA_KEEP_TOGETHER  ),      RES_KEEP,               &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_SPLIT      ),          RES_PARATR_SPLIT,       &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_WIDOWS),               RES_PARATR_WIDOWS,      &::getCppuType((const sal_Int8*)0),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_ORPHANS),              RES_PARATR_ORPHANS,      &::getCppuType((const sal_Int8*)0),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_EXPAND_SINGLE_WORD),     RES_PARATR_ADJUST,      &::getBooleanCppuType()  ,        PROPERTY_NONE, MID_EXPAND_SINGLE   },
                    { SW_PROP_NAME(UNO_NAME_PARA_LAST_LINE_ADJUST),       RES_PARATR_ADJUST,      &::getCppuType((const sal_Int16*)0),          PROPERTY_NONE, MID_LAST_LINE_ADJUST},
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_NUMBER_COUNT  ),  RES_LINENUMBER,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_LINENUMBER_COUNT     },
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_NUMBER_START_VALUE),RES_LINENUMBER,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_LINENUMBER_STARTVALUE},
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_SPACING),             RES_PARATR_LINESPACING, &::getCppuType((const style::LineSpacing*)0),PROPERTY_NONE,     CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_ADJUST),                   RES_PARATR_ADJUST,      &::getCppuType((const sal_Int16*)0),        PROPERTY_NONE, MID_PARA_ADJUST},
                    { SW_PROP_NAME(UNO_NAME_PARA_REGISTER_MODE_ACTIVE),     RES_PARATR_REGISTER,    &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_PARA_TOP_MARGIN),           RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_BOTTOM_MARGIN),            RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_TOP_MARGIN_RELATIVE),    RES_UL_SPACE,         &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, MID_UP_REL_MARGIN},
                    { SW_PROP_NAME(UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE), RES_UL_SPACE,         &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, MID_LO_REL_MARGIN},
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),  RES_PARATR_TABSTOP,  new uno::Type(::getCppuType((uno::Sequence<style::TabStop>*)0)),       PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#else
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),                  RES_PARATR_TABSTOP,     &::getCppuType((const uno::Sequence<style::TabStop>*)0),        PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#endif
                    { SW_PROP_NAME(UNO_NAME_WORD_MODE           ),          RES_CHRATR_WORDLINEMODE,&::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER),               RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER),          RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_TOP_BORDER),                RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER),         RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BORDER_DISTANCE),         RES_BOX,              &::getCppuType((const sal_Int32*)0),    0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER_DISTANCE),  RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_TOP_BORDER_DISTANCE),       RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER_DISTANCE),    RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOLLOW_STYLE),          FN_UNO_FOLLOW_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_U_R_L   ),           RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_URL},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_TARGET  ),           RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_TARGET},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_NAME ),          RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
                    { SW_PROP_NAME(UNO_NAME_UNVISITED_CHAR_STYLE_NAME),   RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),       PROPERTY_NONE ,MID_URL_VISITED_FMT       },
                    { SW_PROP_NAME(UNO_NAME_VISITED_CHAR_STYLE_NAME),     RES_TXTATR_INETFMT,     &::getCppuType((const OUString*)0),           PROPERTY_NONE ,MID_URL_UNVISITED_FMT     },
                    { SW_PROP_NAME(UNO_NAME_PARA_IS_HYPHENATION                ),   RES_PARATR_HYPHENZONE,      &::getBooleanCppuType(),    PropertyAttribute::MAYBEVOID, MID_IS_HYPHEN         },
                    { SW_PROP_NAME(UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS ),   RES_PARATR_HYPHENZONE,      &::getCppuType((const sal_Int16*)0),    PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_LEAD   },
                    { SW_PROP_NAME(UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS),   RES_PARATR_HYPHENZONE,      &::getCppuType((const sal_Int16*)0),    PropertyAttribute::MAYBEVOID, MID_HYPHEN_MIN_TRAIL  },
                    { SW_PROP_NAME(UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS       ),   RES_PARATR_HYPHENZONE,      &::getCppuType((const sal_Int16*)0),    PropertyAttribute::MAYBEVOID, MID_HYPHEN_MAX_HYPHENS},
                    { SW_PROP_NAME(UNO_NAME_NUMBERING_STYLE_NAME),          RES_PARATR_NUMRULE,     &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID,   0},
                    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),       RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_IS_PHYSICAL),                  FN_UNO_IS_PHYSICAL,    &::getBooleanCppuType(), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_IS_AUTO_UPDATE),            FN_UNO_IS_AUTO_UPDATE, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_DISPLAY_NAME),              FN_UNO_DISPLAY_NAME, &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_CATEGORY),                  FN_UNO_CATEGORY,    &::getCppuType((sal_Int16*)0),          PROPERTY_NONE , 0 },
                    { SW_PROP_NAME(UNO_NAME_PARA_SHADOW_FORMAT),            RES_SHADOW,     &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_IS_ON),            RES_CHRATR_TWO_LINES,           &::getBooleanCppuType(),    PROPERTY_NONE, MID_TWOLINES},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_PREFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PROPERTY_NONE, MID_START_BRACKET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_SUFFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PROPERTY_NONE, MID_END_BRACKET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_EMPHASIS),             RES_CHRATR_EMPHASIS_MARK,           &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, MID_EMPHASIS},
                    { SW_PROP_NAME(UNO_NAME_PARA_IS_HANGING_PUNCTUATION),   RES_PARATR_HANGINGPUNCTUATION,  &::getBooleanCppuType(),    PROPERTY_NONE ,0     },
                    { SW_PROP_NAME(UNO_NAME_PARA_IS_CHARACTER_DISTANCE),    RES_PARATR_SCRIPTSPACE,         &::getBooleanCppuType(),    PROPERTY_NONE ,0     },
                    { SW_PROP_NAME(UNO_NAME_PARA_IS_FORBIDDEN_RULES),       RES_PARATR_FORBIDDEN_RULES,     &::getBooleanCppuType(),    PROPERTY_NONE ,0     },
                    _PROP_DIFF_FONTHEIGHT
                    { SW_PROP_NAME(UNO_NAME_CHAR_ROTATION               ),  RES_CHRATR_ROTATE,      &::getCppuType((sal_Int16*)0),  PROPERTY_NONE,    MID_ROTATE      },
                    { SW_PROP_NAME(UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE),  RES_CHRATR_ROTATE,      &::getBooleanCppuType(),    PROPERTY_NONE,        MID_FITTOLINE  },
                    { SW_PROP_NAME(UNO_NAME_CHAR_SCALE_WIDTH            ),  RES_CHRATR_SCALEW,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE,         0 },
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aParaStyleMap;
            }
            break;
            case PROPERTY_MAP_FRAME_STYLE:
            {
                static SfxItemPropertyMap aFrameStyleMap   [] =
                {
                    { SW_PROP_NAME(UNO_NAME_ANCHOR_PAGE_NO),            RES_ANCHOR,             &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ANCHOR_PAGENUM       },
                    { SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE   ),            RES_ANCHOR,             &::getCppuType((text::TextContentAnchorType*)0),            PROPERTY_NONE, MID_ANCHOR_ANCHORTYPE},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NAME(UNO_NAME_CHAIN_NEXT_NAME),           RES_CHAIN,              &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_CHAIN_NEXTNAME},
                //  { SW_PROP_NAME(UNO_NAME_CHAIN_PREV_NAME),           RES_CHAIN,              &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_CHAIN_PREVNAME},
                /*not impl*/    { SW_PROP_NAME(UNO_NAME_CLIENT_MAP      ),      RES_URL,                &::getBooleanCppuType(),            PROPERTY_NONE ,MID_URL_CLIENTMAP         },
                    { SW_PROP_NAME(UNO_NAME_CONTENT_PROTECTED ),        RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_CONTENT   },
                    { SW_PROP_NAME(UNO_NAME_EDIT_IN_READONLY),      RES_EDIT_IN_READONLY,   &::getBooleanCppuType(),            PROPERTY_NONE, 0},
                //  { SW_PROP_NAME(UNO_NAME_GRAPHIC           ),        RES_BACKGROUND,         &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),         RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN),           RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT  ),             RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
                    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_POSITION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_HORIORIENT_POSITION|CONVERT_TWIPS    },
                    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_RELATION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_RELATION  },
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_U_R_L   ),       RES_URL,                &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_URL_URL},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_TARGET  ),       RES_URL,                &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_URL_TARGET},
                    { SW_PROP_NAME(UNO_NAME_HYPER_LINK_NAME ),      RES_URL,                &::getCppuType((const OUString*)0),         PROPERTY_NONE ,MID_URL_HYPERLINKNAME     },
                    { SW_PROP_NAME(UNO_NAME_OPAQUE),                    RES_OPAQUE,             &::getBooleanCppuType(),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_PAGE_TOGGLE),               RES_HORI_ORIENT,        &::getBooleanCppuType(),            PROPERTY_NONE ,MID_HORIORIENT_PAGETOGGLE },
                    { SW_PROP_NAME(UNO_NAME_POSITION_PROTECTED),    RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_POSITION},
                    { SW_PROP_NAME(UNO_NAME_PRINT),                     RES_PRINT,              &::getBooleanCppuType(),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_WIDTH),                     RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
                    { SW_PROP_NAME(UNO_NAME_HEIGHT),                    RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NAME(UNO_NAME_RELATIVE_HEIGHT),       RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE,   MID_FRMSIZE_REL_HEIGHT },
                    { SW_PROP_NAME(UNO_NAME_RELATIVE_WIDTH),            RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE,   MID_FRMSIZE_REL_WIDTH  },
                    { SW_PROP_NAME(UNO_NAME_SIZE_TYPE),                 RES_FRM_SIZE,           &::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    { SW_PROP_NAME(UNO_NAME_SIZE),                  RES_FRM_SIZE,           &::getCppuType((const awt::Size*)0),            PROPERTY_NONE, MID_FRMSIZE_SIZE|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_SIZE_RELATIVE),             RES_FRM_SIZE,           &::getBooleanCppuType()  ,          PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_REL_SIZE   },
                    { SW_PROP_NAME(UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT),           RES_FRM_SIZE,           &::getBooleanCppuType()  ,          PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT    },
                    { SW_PROP_NAME(UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH),           RES_FRM_SIZE,           &::getBooleanCppuType()  ,          PROPERTY_NONE,   MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH },
                //  { SW_PROP_NAME(UNO_NAME_WIDTH),                     RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_WIDTH            },
                    { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT),             RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_SERVER_MAP      ),      RES_URL,                &::getBooleanCppuType(),            PROPERTY_NONE ,MID_URL_SERVERMAP         },
                    { SW_PROP_NAME(UNO_NAME_SIZE_PROTECTED    ),    RES_PROTECT,            &::getBooleanCppuType(),            PROPERTY_NONE, MID_PROTECT_SIZE    },
                    //Surround bleibt, weil es mit der 5.1 ausgeliefert wurde, obwohl es mit text::WrapTextMode identisch ist
                    { SW_PROP_NAME(UNO_NAME_SURROUND                  ), RES_SURROUND,          &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { SW_PROP_NAME(UNO_NAME_TEXT_WRAP),             RES_SURROUND,           &::getCppuType((text::WrapTextMode*)0),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { SW_PROP_NAME(UNO_NAME_SURROUND_ANCHORONLY),   RES_SURROUND,           &::getBooleanCppuType(),            PROPERTY_NONE, MID_SURROUND_ANCHORONLY      },
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),                RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),             RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT  ),             RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_POSITION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_VERTORIENT_POSITION|CONVERT_TWIPS    },
                    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_RELATION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_RELATION  },
                    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER),               RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER),          RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_TOP_BORDER),                RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER),         RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BORDER_DISTANCE),         RES_BOX,    &::getCppuType((const sal_Int32*)0),  0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER_DISTANCE),  RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_TOP_BORDER_DISTANCE),       RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER_DISTANCE),    RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),       RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_IS_PHYSICAL),                  FN_UNO_IS_PHYSICAL,    &::getBooleanCppuType(), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_IS_AUTO_UPDATE),            FN_UNO_IS_AUTO_UPDATE, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_DISPLAY_NAME),              FN_UNO_DISPLAY_NAME, &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aFrameStyleMap;
            }
            break;
            case PROPERTY_MAP_PAGE_STYLE :
            {
                static SfxItemPropertyMap aPageStyleMap [] =
                {
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NAME(UNO_NAME_GRAPHIC           ),        RES_BACKGROUND,         &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),    RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),     RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ), RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN),           RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER),               RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER),          RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_TOP_BORDER),                RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER),         RES_BOX,                &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BORDER_DISTANCE),         RES_BOX,    &::getCppuType((const sal_Int32*)0),  0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER_DISTANCE),  RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_TOP_BORDER_DISTANCE),       RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER_DISTANCE),    RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT),             RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},

                    { SW_PROP_NAME(UNO_NAME_HEADER_BACK_COLOR        ),     FN_UNO_HEADER_BACKGROUND,   &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NAME(UNO_NAME_HEADER_GRAPHIC           ),         FN_UNO_HEADER_BACKGROUND,   &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NAME(UNO_NAME_HEADER_GRAPHIC_URL      ),      RES_BACKGROUND,             &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_HEADER_GRAPHIC_FILTER  ),       RES_BACKGROUND,             &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_HEADER_GRAPHIC_LOCATION  ),         FN_UNO_HEADER_BACKGROUND,   &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_HEADER_LEFT_MARGIN),                FN_UNO_HEADER_LR_SPACE,     &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_HEADER_RIGHT_MARGIN),           FN_UNO_HEADER_LR_SPACE,     &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_HEADER_BACK_TRANSPARENT),   FN_UNO_HEADER_BACKGROUND,   &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_NAME_HEADER_LEFT_BORDER),                FN_UNO_HEADER_BOX,              &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_RIGHT_BORDER),               FN_UNO_HEADER_BOX,              &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_TOP_BORDER),             FN_UNO_HEADER_BOX,              &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_BOTTOM_BORDER),          FN_UNO_HEADER_BOX,              &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_BORDER_DISTANCE),            FN_UNO_HEADER_BOX,    &::getCppuType((const sal_Int32*)0),  0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_LEFT_BORDER_DISTANCE),   FN_UNO_HEADER_BOX,              &::getCppuType((const sal_Int32*)0),    0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_RIGHT_BORDER_DISTANCE),  FN_UNO_HEADER_BOX,              &::getCppuType((const sal_Int32*)0),    0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_TOP_BORDER_DISTANCE),        FN_UNO_HEADER_BOX,              &::getCppuType((const sal_Int32*)0),    0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_BOTTOM_BORDER_DISTANCE), FN_UNO_HEADER_BOX,              &::getCppuType((const sal_Int32*)0),    0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HEADER_SHADOW_FORMAT),          FN_UNO_HEADER_SHADOW,       &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_HEADER_BODY_DISTANCE),          FN_UNO_HEADER_BODY_DISTANCE,&::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_LO_MARGIN|CONVERT_TWIPS       },
                    { SW_PROP_NAME(UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT),      FN_UNO_HEADER_IS_DYNAMIC_DISTANCE,&::getBooleanCppuType(),          PROPERTY_NONE ,0         },
                    { SW_PROP_NAME(UNO_NAME_HEADER_IS_SHARED),          FN_UNO_HEADER_SHARE_CONTENT,&::getBooleanCppuType(),            PROPERTY_NONE ,0         },
                    { SW_PROP_NAME(UNO_NAME_HEADER_HEIGHT),                 FN_UNO_HEADER_HEIGHT,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NAME(UNO_NAME_HEADER_IS_ON),                      FN_UNO_HEADER_ON,           &::getBooleanCppuType(),            PROPERTY_NONE ,0         },

                    { SW_PROP_NAME(UNO_NAME_FOOTER_BACK_COLOR        ),     FN_UNO_FOOTER_BACKGROUND,   &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                //  { SW_PROP_NAME(UNO_NAME_FOOTER_GRAPHIC           ),         FN_UNO_FOOTER_BACKGROUND,   &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NAME(UNO_NAME_FOOTER_GRAPHIC_URL      ),      RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_GRAPHIC_FILTER  ),       RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_GRAPHIC_LOCATION  ),         FN_UNO_FOOTER_BACKGROUND,   &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_FOOTER_LEFT_MARGIN),                FN_UNO_FOOTER_LR_SPACE,     &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_FOOTER_RIGHT_MARGIN),           FN_UNO_FOOTER_LR_SPACE,     &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_FOOTER_BACK_TRANSPARENT ),  FN_UNO_FOOTER_BACKGROUND,   &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_LEFT_BORDER),            FN_UNO_FOOTER_BOX,              &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_RIGHT_BORDER),           FN_UNO_FOOTER_BOX,              &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_TOP_BORDER),             FN_UNO_FOOTER_BOX,              &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_BOTTOM_BORDER),          FN_UNO_FOOTER_BOX,              &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_BORDER_DISTANCE),        FN_UNO_FOOTER_BOX,    &::getCppuType((const sal_Int32*)0),  0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_LEFT_BORDER_DISTANCE),   FN_UNO_FOOTER_BOX,              &::getCppuType((const sal_Int32*)0),    0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_RIGHT_BORDER_DISTANCE),  FN_UNO_FOOTER_BOX,              &::getCppuType((const sal_Int32*)0),    0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_TOP_BORDER_DISTANCE),    FN_UNO_FOOTER_BOX,              &::getCppuType((const sal_Int32*)0),    0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_BOTTOM_BORDER_DISTANCE), FN_UNO_FOOTER_BOX,              &::getCppuType((const sal_Int32*)0),    0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_SHADOW_FORMAT),          FN_UNO_FOOTER_SHADOW,       &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_FOOTER_BODY_DISTANCE),          FN_UNO_FOOTER_BODY_DISTANCE,&::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_UP_MARGIN|CONVERT_TWIPS       },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_IS_DYNAMIC_HEIGHT),  FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE,&::getBooleanCppuType(),          PROPERTY_NONE ,0         },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_IS_SHARED),          FN_UNO_FOOTER_SHARE_CONTENT,&::getBooleanCppuType(),            PROPERTY_NONE ,0         },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_HEIGHT),                 FN_UNO_FOOTER_HEIGHT,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_SIZE_HEIGHT|CONVERT_TWIPS         },
                    { SW_PROP_NAME(UNO_NAME_FOOTER_IS_ON),                      FN_UNO_FOOTER_ON,           &::getBooleanCppuType(),            PROPERTY_NONE ,0         },


                    { SW_PROP_NAME(UNO_NAME_IS_LANDSCAPE),              SID_ATTR_PAGE,          &::getBooleanCppuType(),            PROPERTY_NONE ,MID_PAGE_ORIENTATION   },
                    { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),            SID_ATTR_PAGE,          &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE , MID_PAGE_NUMTYPE       },
                    { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_LAYOUT),         SID_ATTR_PAGE,          &::getCppuType((const style::PageStyleLayout*)0),   PROPERTY_NONE ,MID_PAGE_LAYOUT     },
                    { SW_PROP_NAME(UNO_NAME_PRINTER_PAPER_TRAY),        RES_PAPER_BIN,          &::getCppuType((const sal_Int8*)0),             PROPERTY_NONE , 0 },
//                  { SW_PROP_NAME(UNO_NAME_REGISTER_MODE_ACTIVE),  SID_SWREGISTER_MODE,    &::getBooleanCppuType(),            PROPERTY_NONE , 0 },
                    { SW_PROP_NAME(UNO_NAME_REGISTER_PARAGRAPH_STYLE),SID_SWREGISTER_COLLECTION,    &::getCppuType((const OUString*)0),         PROPERTY_NONE , 0 },
                    { SW_PROP_NAME(UNO_NAME_SIZE),                  SID_ATTR_PAGE_SIZE,     &::getCppuType((const awt::Size*)0),            PROPERTY_NONE,   MID_SIZE_SIZE|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_WIDTH),                 SID_ATTR_PAGE_SIZE,     &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_SIZE_WIDTH|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_HEIGHT),                SID_ATTR_PAGE_SIZE,     &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_SIZE_HEIGHT|CONVERT_TWIPS            },
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),            RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),             RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_HEADER_TEXT),           FN_UNO_HEADER,          &::getCppuType((uno::Reference<text::XText>*)0),            PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_HEADER_TEXT_LEFT),      FN_UNO_HEADER_LEFT,     &::getCppuType((uno::Reference<text::XText>*)0),            PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_HEADER_TEXT_RIGHT),     FN_UNO_HEADER_RIGHT,    &::getCppuType((uno::Reference<text::XText>*)0),            PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_FOOTER_TEXT),           FN_UNO_FOOTER,          &::getCppuType((uno::Reference<text::XText>*)0),            PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_FOOTER_TEXT_LEFT),      FN_UNO_FOOTER_LEFT,     &::getCppuType((uno::Reference<text::XText>*)0),            PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_FOOTER_TEXT_RIGHT),     FN_UNO_FOOTER_RIGHT,    &::getCppuType((uno::Reference<text::XText>*)0),            PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_FOLLOW_STYLE),          FN_UNO_FOLLOW_STYLE,    &::getCppuType((const OUString*)0),         PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),       RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_IS_PHYSICAL),                  FN_UNO_IS_PHYSICAL,    &::getBooleanCppuType(), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_DISPLAY_NAME),              FN_UNO_DISPLAY_NAME, &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_HEIGHT),               FN_PARAM_FTN_INFO,      &::getCppuType((const sal_Int32*)0), PROPERTY_NONE ,        MID_FTN_HEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_LINE_WEIGHT),          FN_PARAM_FTN_INFO,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE ,     MID_LINE_WEIGHT|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_LINE_COLOR),           FN_PARAM_FTN_INFO,      &::getCppuType((const sal_Int32*)0),    PROPERTY_NONE ,     MID_LINE_COLOR},
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_LINE_RELATIVE_WIDTH),  FN_PARAM_FTN_INFO,      &::getCppuType((const sal_Int8*)0),     PROPERTY_NONE ,     MID_LINE_RELWIDTH    },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_LINE_ADJUST),          FN_PARAM_FTN_INFO,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE ,     MID_LINE_ADJUST     },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_LINE_TEXT_DISTANCE),   FN_PARAM_FTN_INFO,      &::getCppuType((const sal_Int32*)0),    PROPERTY_NONE ,     MID_LINE_TEXT_DIST   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_LINE_DISTANCE),        FN_PARAM_FTN_INFO,      &::getCppuType((const sal_Int32*)0),    PROPERTY_NONE ,     MID_LINE_FOOTNOTE_DIST|CONVERT_TWIPS},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aPageStyleMap;
            }
            break;
            case PROPERTY_MAP_NUM_STYLE  :
            {
                static SfxItemPropertyMap aNumStyleMap      [] =
                {
                    { SW_PROP_NAME(UNO_NAME_NUMBERING_RULES),       FN_UNO_NUM_RULES,   &::getCppuType((uno::Reference<container::XIndexReplace>*)0), PROPERTY_NONE, CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_IS_PHYSICAL),           FN_UNO_IS_PHYSICAL,       &::getBooleanCppuType(), PropertyAttribute::READONLY, 0},
                    { SW_PROP_NAME(UNO_NAME_DISPLAY_NAME),          FN_UNO_DISPLAY_NAME, &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aNumStyleMap;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE :
            {
                static SfxItemPropertyMap aTablePropertyMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ), RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE,MID_BACK_COLOR         },
                    { SW_PROP_NAME(UNO_NAME_BREAK_TYPE),            RES_BREAK,              &::getCppuType((const style::BreakType*)0),         PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),    RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),      RES_BACKGROUND,        &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ), RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN),           RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT  ),         RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_HORIORIENT_ORIENT    },
                    { SW_PROP_NAME(UNO_NAME_KEEP_TOGETHER),         RES_KEEP,               &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_SPLIT    ),             RES_LAYOUT_SPLIT,       &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_PAGE_NUMBER_OFFSET),    RES_PAGEDESC,           &::getCppuType((const sal_Int16*)0),        PropertyAttribute::MAYBEVOID, MID_PAGEDESC_PAGENUMOFFSET},
                    { SW_PROP_NAME(UNO_NAME_PAGE_DESC_NAME),        RES_PAGEDESC,           &::getCppuType((const OUString*)0),         PROPERTY_NONE, 0xff},
                    { SW_PROP_NAME(UNO_NAME_RELATIVE_WIDTH),        FN_TABLE_RELATIVE_WIDTH,&::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE, 0xff },
                    { SW_PROP_NAME(UNO_NAME_REPEAT_HEADLINE ),      FN_TABLE_HEADLINE_REPEAT,&::getBooleanCppuType(),       PROPERTY_NONE, 0xff},
                    { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT),         RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_SIZE_RELATIVE),         FN_TABLE_IS_RELATIVE_WIDTH,  &::getBooleanCppuType()  ,     PROPERTY_NONE, 0xff },
                    { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),            RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),         RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT),  RES_BACKGROUND,     &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_NAME_WIDTH),                 FN_TABLE_WIDTH,         &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, 0xff},
                    { SW_PROP_NAME(UNO_NAME_CHART_ROW_AS_LABEL),        FN_UNO_RANGE_ROW_LABEL,         &::getBooleanCppuType(),            PROPERTY_NONE,  0},
                    { SW_PROP_NAME(UNO_NAME_CHART_COLUMN_AS_LABEL),     FN_UNO_RANGE_COL_LABEL,         &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_TABLE_BORDER),          FN_UNO_TABLE_BORDER,            &::getCppuType((const table::TableBorder*)0),   PropertyAttribute::MAYBEVOID, CONVERT_TWIPS },
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_TABLE_COLUMN_SEPARATORS),   FN_UNO_TABLE_COLUMN_SEPARATORS, new uno::Type(::getCppuType((const uno::Sequence<text::TableColumnSeparator>*)0)),      PropertyAttribute::MAYBEVOID, 0 },
#else
                    { SW_PROP_NAME(UNO_NAME_TABLE_COLUMN_SEPARATORS),   FN_UNO_TABLE_COLUMN_SEPARATORS, &::getCppuType((const uno::Sequence<text::TableColumnSeparator>*)0),    PropertyAttribute::MAYBEVOID, 0 },
#endif
                    { SW_PROP_NAME(UNO_NAME_TABLE_COLUMN_RELATIVE_SUM), FN_UNO_TABLE_COLUMN_RELATIVE_SUM,       &::getCppuType((const sal_Int16*)0),        PropertyAttribute::READONLY, 0 },
                    COMMON_TEXT_CONTENT_PROPERTIES
                    { SW_PROP_NAME(UNO_LINK_DISPLAY_NAME),          FN_PARAM_LINK_DISPLAY_NAME,     &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),       RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_TEXT_SECTION),              FN_UNO_TEXT_SECTION,    &::getCppuType((uno::Reference<text::XTextSection>*)0), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    _REDLINE_NODE_PROPERTIES
                    {0,0}
                };

                aMapArr[nPropertyId] = aTablePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_CELL :
            {
                static SfxItemPropertyMap aCellMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,     &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE , MID_BACK_COLOR       },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),     RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),             RES_BOXATR_FORMAT,  &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT),  RES_BACKGROUND,     &::getBooleanCppuType(),            PROPERTY_NONE , MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER),               RES_BOX,    &::getCppuType((const table::BorderLine*)0),    0, LEFT_BORDER  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER),          RES_BOX,    &::getCppuType((const table::BorderLine*)0),    0, RIGHT_BORDER |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_TOP_BORDER),                RES_BOX,    &::getCppuType((const table::BorderLine*)0),    0, TOP_BORDER   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER),         RES_BOX,    &::getCppuType((const table::BorderLine*)0),    0, BOTTOM_BORDER|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BORDER_DISTANCE),         RES_BOX,    &::getCppuType((const sal_Int32*)0),  0, BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_LEFT_BORDER_DISTANCE),  RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, LEFT_BORDER_DISTANCE  |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_RIGHT_BORDER_DISTANCE), RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, RIGHT_BORDER_DISTANCE |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_TOP_BORDER_DISTANCE),       RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, TOP_BORDER_DISTANCE   |CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_BORDER_DISTANCE),    RES_BOX,                &::getCppuType((const sal_Int32*)0),    0, BOTTOM_BORDER_DISTANCE|CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),       RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_TEXT_SECTION),              FN_UNO_TEXT_SECTION,    &::getCppuType((uno::Reference<text::XTextSection>*)0), PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aCellMap_Impl;
            }
            break;
            case PROPERTY_MAP_TABLE_RANGE:
            {
                static SfxItemPropertyMap aRangePropertyMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_CHAR_AUTO_KERNING       ),  RES_CHRATR_AUTOKERN  ,  &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CASE_MAP),             RES_CHRATR_CASEMAP,     &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_STRIKEOUT),            RES_CHRATR_CROSSEDOUT,  &::getCppuType((const sal_Int16*)0),                    PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CROSSED_OUT),          RES_CHRATR_CROSSEDOUT,  &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT),           RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ESC          },
                    { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT),        RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE, MID_ESC_HEIGHT},
                    { SW_PROP_NAME(UNO_NAME_CHAR_FLASH              ),  RES_CHRATR_BLINK    ,   &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE),            RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, MID_UNDERLINE},
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_COLOR),              RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, MID_UL_COLOR},
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR),              RES_CHRATR_UNDERLINE ,  &::getBooleanCppuType(),            PROPERTY_NONE, MID_UL_HASCOLOR},
                    { SW_PROP_NAME(UNO_NAME_PARA_LEFT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_TXT_LMARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_RIGHT_MARGIN),             RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_FIRST_LINE_INDENT),        RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_CHAR_KERNING            ),  RES_CHRATR_KERNING    , &::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE,  0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_NO_HYPHENATION     ),  RES_CHRATR_NOHYPHEN ,   &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_SHADOWED),             RES_CHRATR_SHADOWED  ,  &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),           RES_TXTATR_CHARFMT,     &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CONTOURED),            RES_CHRATR_CONTOUR,     &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_FORMAT),           RES_PARATR_DROP,        &::getCppuType((const style::DropCapFormat*)0)  , PROPERTY_NONE, MID_DROPCAP_FORMAT     },
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_WHOLE_WORD),       RES_PARATR_DROP,        &::getBooleanCppuType()  ,          PROPERTY_NONE, MID_DROPCAP_WHOLE_WORD },
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME),  RES_PARATR_DROP,        &::getCppuType((const OUString*)0)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },
                    { SW_PROP_NAME(UNO_NAME_PARA_KEEP_TOGETHER  ),      RES_KEEP,               &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_SPLIT      ),          RES_PARATR_SPLIT,       &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_WIDOWS),               RES_PARATR_WIDOWS,      &::getCppuType((const sal_Int8*)0),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_ORPHANS),              RES_PARATR_ORPHANS,      &::getCppuType((const sal_Int8*)0),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),                 RES_BOXATR_FORMAT,      &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NAME(UNO_NAME_PAGE_NUMBER_OFFSET),            RES_PAGEDESC,           &::getCppuType((const sal_Int16*)0),        PROPERTY_NONE, MID_PAGEDESC_PAGENUMOFFSET},
                    { SW_PROP_NAME(UNO_NAME_PARA_ADJUST),                   RES_PARATR_ADJUST,      &::getCppuType((const sal_Int16*)0),        PROPERTY_NONE, MID_PARA_ADJUST},
                    { SW_PROP_NAME(UNO_NAME_PARA_EXPAND_SINGLE_WORD),     RES_PARATR_ADJUST,      &::getBooleanCppuType()  ,        PROPERTY_NONE, MID_EXPAND_SINGLE   },
                    { SW_PROP_NAME(UNO_NAME_PARA_LAST_LINE_ADJUST),       RES_PARATR_ADJUST,      &::getCppuType((const sal_Int16*)0),          PROPERTY_NONE, MID_LAST_LINE_ADJUST},
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_NUMBER_COUNT  ),  RES_LINENUMBER,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_LINENUMBER_COUNT     },
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_NUMBER_START_VALUE),RES_LINENUMBER,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_LINENUMBER_STARTVALUE},
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_SPACING),             RES_PARATR_LINESPACING, &::getCppuType((const style::LineSpacing*)0),PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_REGISTER_MODE_ACTIVE),      RES_PARATR_REGISTER,    &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),  RES_PARATR_TABSTOP,  new uno::Type(::getCppuType((uno::Sequence<style::TabStop>*)0)),       PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#else
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),                  RES_PARATR_TABSTOP,     &::getCppuType((const uno::Sequence<style::TabStop>*)0),        PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#endif
                    { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),                RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),             RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_WORD_MODE           ),          RES_CHRATR_WORDLINEMODE,&::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COLOR),                    RES_CHRATR_COLOR,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_BACK_COLOR      ),     RES_CHRATR_BACKGROUND,  &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NAME(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  &::getBooleanCppuType(),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NAME(UNO_NAME_PARA_BACK_COLOR      ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NAME(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),            RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),             RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_LOCATION    ),     RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),             FN_UNO_TABLE_CELL_BACKGROUND,   &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),         FN_UNO_TABLE_CELL_BACKGROUND,   &::getCppuType((const sal_Int32*)0),    PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ),     FN_UNO_TABLE_CELL_BACKGROUND,   &::getBooleanCppuType(),    PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NAME(UNO_NAME_CHART_ROW_AS_LABEL),            FN_UNO_RANGE_ROW_LABEL, &::getBooleanCppuType(),            PROPERTY_NONE,  0},
                    { SW_PROP_NAME(UNO_NAME_CHART_COLUMN_AS_LABEL),         FN_UNO_RANGE_COL_LABEL, &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_STYLE_NAME),           FN_UNO_PARA_STYLE,      &::getCppuType((const OUString*)0),         PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_IS_ON),            RES_CHRATR_TWO_LINES,           &::getBooleanCppuType(),    PROPERTY_NONE, MID_TWOLINES},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_PREFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PROPERTY_NONE, MID_START_BRACKET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_SUFFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PROPERTY_NONE, MID_END_BRACKET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_EMPHASIS),             RES_CHRATR_EMPHASIS_MARK,           &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, MID_EMPHASIS},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aRangePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_SECTION:
            {
                static SfxItemPropertyMap aSectionPropertyMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_CONDITION),         WID_SECT_CONDITION, &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_DDE_COMMAND_FILE),  WID_SECT_DDE_TYPE, &::getCppuType((const OUString*)0)  ,        PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_DDE_COMMAND_TYPE),  WID_SECT_DDE_FILE, &::getCppuType((const OUString*)0)  ,        PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_DDE_COMMAND_ELEMENT), WID_SECT_DDE_ELEMENT, &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC_UPDATE), WID_SECT_DDE_AUTOUPDATE, &::getBooleanCppuType(),             PROPERTY_NONE,  0},
                    { SW_PROP_NAME(UNO_NAME_FILE_LINK),         WID_SECT_LINK     , &::getCppuType((const text::SectionFileLink*)0),    PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_VISIBLE),        WID_SECT_VISIBLE   , &::getBooleanCppuType(),           PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_PROTECTED),  WID_SECT_PROTECTED, &::getBooleanCppuType(),            PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LINK_REGION),   WID_SECT_REGION   , &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),    RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),     RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ), RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { SW_PROP_NAME(UNO_LINK_DISPLAY_NAME),          FN_PARAM_LINK_DISPLAY_NAME,     &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),       RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_IS_COLLECT_AT_TEXT_END),RES_FTN_AT_TXTEND,         &::getBooleanCppuType(),                   PROPERTY_NONE ,MID_COLLECT                   },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_IS_RESTART_NUMBERING) ,RES_FTN_AT_TXTEND,      &::getBooleanCppuType(),                   PROPERTY_NONE , MID_RESTART_NUM },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_RESTART_NUMBERING_AT) ,RES_FTN_AT_TXTEND,      &::getCppuType((const sal_Int16*)0),                PROPERTY_NONE , MID_NUM_START_AT},
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_IS_OWN_NUMBERING)     ,RES_FTN_AT_TXTEND,  &::getBooleanCppuType(),                                                    PROPERTY_NONE ,  MID_OWN_NUM     },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_NUMBERING_TYPE)       ,RES_FTN_AT_TXTEND,      &::getCppuType((const sal_Int16*)0),                PROPERTY_NONE , MID_NUM_TYPE    },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_NUMBERING_PREFIX)     ,RES_FTN_AT_TXTEND,  &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,      MID_PREFIX      },
                    { SW_PROP_NAME(UNO_NAME_FOOTNOTE_NUMBERING_SUFFIX)     ,RES_FTN_AT_TXTEND,  &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,      MID_SUFFIX      },
                    { SW_PROP_NAME(UNO_NAME_ENDNOTE_IS_COLLECT_AT_TEXT_END),RES_END_AT_TXTEND,        &::getBooleanCppuType(),                 PROPERTY_NONE , MID_COLLECT                      },
                    { SW_PROP_NAME(UNO_NAME_ENDNOTE_IS_RESTART_NUMBERING)  ,RES_END_AT_TXTEND,        &::getBooleanCppuType(),                 PROPERTY_NONE , MID_RESTART_NUM  },
                    { SW_PROP_NAME(UNO_NAME_ENDNOTE_RESTART_NUMBERING_AT)  ,RES_END_AT_TXTEND,     &::getCppuType((const sal_Int16*)0),             PROPERTY_NONE ,  MID_NUM_START_AT },
                    { SW_PROP_NAME(UNO_NAME_ENDNOTE_IS_OWN_NUMBERING)      ,RES_END_AT_TXTEND,     &::getBooleanCppuType(),                                                 PROPERTY_NONE ,  MID_OWN_NUM      },
                    { SW_PROP_NAME(UNO_NAME_ENDNOTE_NUMBERING_TYPE)        ,RES_END_AT_TXTEND,       &::getCppuType((const sal_Int16*)0),               PROPERTY_NONE ,MID_NUM_TYPE     },
                    { SW_PROP_NAME(UNO_NAME_ENDNOTE_NUMBERING_PREFIX)      ,RES_END_AT_TXTEND,     &::getCppuType((const OUString*)0)  ,        PROPERTY_NONE,   MID_PREFIX       },
                    { SW_PROP_NAME(UNO_NAME_ENDNOTE_NUMBERING_SUFFIX)      ,RES_END_AT_TXTEND,     &::getCppuType((const OUString*)0)  ,        PROPERTY_NONE,   MID_SUFFIX       },
                    { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX), WID_SECT_DOCUMENT_INDEX, &::getCppuType((uno::Reference<text::XDocumentIndex>*)0), PropertyAttribute::READONLY | PropertyAttribute::MAYBEVOID, 0 },
                    COMMON_TEXT_CONTENT_PROPERTIES
                    _REDLINE_NODE_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aSectionPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_SEARCH:
            {
                static SfxItemPropertyMap aSearchPropertyMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_SEARCH_ALL),                    WID_SEARCH_ALL,         &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_BACKWARDS          ),    WID_BACKWARDS,          &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_CASE_SENSITIVE     ),    WID_CASE_SENSITIVE,     &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                //  { SW_PROP_NAME(UNO_NAME_SEARCH_IN_SELECTION       ),    WID_IN_SELECTION,       &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_REGULAR_EXPRESSION ),    WID_REGULAR_EXPRESSION, &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_SIMILARITY         ),    WID_SIMILARITY,         &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_SIMILARITY_ADD     ),    WID_SIMILARITY_ADD,     &::getCppuType((const sal_Int16*)0)  ,  PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_SIMILARITY_EXCHANGE),    WID_SIMILARITY_EXCHANGE,&::getCppuType((const sal_Int16*)0)  ,  PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_SIMILARITY_RELAX   ),    WID_SIMILARITY_RELAX,   &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_SIMILARITY_REMOVE  ),    WID_SIMILARITY_REMOVE,  &::getCppuType((const sal_Int16*)0)  ,  PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_STYLES             ),    WID_STYLES,             &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SEARCH_WORDS                  ),    WID_WORDS,              &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aSearchPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_FRAME:
            {
                static SfxItemPropertyMap aFramePropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    { SW_PROP_NAME(UNO_NAME_CHAIN_NEXT_NAME),           RES_CHAIN,              &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID ,MID_CHAIN_NEXTNAME},
                    { SW_PROP_NAME(UNO_NAME_CHAIN_PREV_NAME),           RES_CHAIN,              &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID ,MID_CHAIN_PREVNAME},
                /*not impl*/    { SW_PROP_NAME(UNO_NAME_CLIENT_MAP      ),      RES_URL,                &::getBooleanCppuType(),            PROPERTY_NONE ,MID_URL_CLIENTMAP         },
                    { SW_PROP_NAME(UNO_NAME_EDIT_IN_READONLY),      RES_EDIT_IN_READONLY,   &::getBooleanCppuType(),            PROPERTY_NONE, 0},
                //  { SW_PROP_NAME(UNO_NAME_GRAPHIC           ),        RES_BACKGROUND,         &,                              PROPERTY_NONE, MID_GRAPHIC
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    //next elements are part of the service description
                    { MAP_CHAR_LEN("FrameHeightAbsolute"),          RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0),        PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                    { MAP_CHAR_LEN("FrameHeightPercent"),               RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0),         PROPERTY_NONE, MID_FRMSIZE_REL_HEIGHT   },
                    { MAP_CHAR_LEN("FrameIsAutomaticHeight"),         RES_FRM_SIZE,         &::getBooleanCppuType(),        PROPERTY_NONE, MID_FRMSIZE_IS_AUTO_HEIGHT   },
                    { MAP_CHAR_LEN("FrameWidthAbsolute"),           RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0),        PROPERTY_NONE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS          },
                    { MAP_CHAR_LEN("FrameWidthPercent"),                RES_FRM_SIZE,           &::getCppuType((const sal_Int8*)0),         PROPERTY_NONE, MID_FRMSIZE_REL_WIDTH    },
                    { SW_PROP_NAME(UNO_NAME_SIZE_TYPE),                 RES_FRM_SIZE,           &::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aFramePropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_GRAPHIC:
            {
                static SfxItemPropertyMap aGraphicPropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    { SW_PROP_NAME(UNO_NAME_CONTOUR_OUTSIDE),       RES_SURROUND,           &::getBooleanCppuType(),            PROPERTY_NONE, MID_SURROUND_CONTOUROUTSIDE  },
                    { SW_PROP_NAME(UNO_NAME_GRAPHIC_CROP),          RES_GRFATR_CROPGRF,     &::getCppuType((const text::GraphicCrop*)0),    PROPERTY_NONE, CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES),           RES_GRFATR_MIRRORGRF,   &::getBooleanCppuType(),            PROPERTY_NONE,      MID_MIRROR_HORZ_EVEN_PAGES            },
                    { SW_PROP_NAME(UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES),            RES_GRFATR_MIRRORGRF,   &::getBooleanCppuType(),            PROPERTY_NONE,      MID_MIRROR_HORZ_ODD_PAGES                 },
                    { SW_PROP_NAME(UNO_NAME_VERT_MIRRORED),         RES_GRFATR_MIRRORGRF,   &::getBooleanCppuType(),            PROPERTY_NONE,     MID_MIRROR_VERT            },
                    { SW_PROP_NAME(UNO_NAME_GRAPHIC_URL),               0,                      &::getCppuType((const OUString*)0), 0, 0 },
                    { SW_PROP_NAME(UNO_NAME_GRAPHIC_FILTER),            0,                      &::getCppuType((const OUString*)0), 0, 0 },
                    { SW_PROP_NAME(UNO_NAME_ACTUAL_SIZE),           0,                      &::getCppuType((const awt::Size*)0),    PropertyAttribute::READONLY, CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_ALTERNATIVE_TEXT),      0,                      &::getCppuType((const OUString*)0),         PROPERTY_NONE , 0   },
                    { SW_PROP_NAME(UNO_NAME_CONTOUR_POLY_POLYGON), FN_PARAM_COUNTOUR_PP, &::getCppuType((PointSequenceSequence*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_GRAPHIC_ROTATION),      RES_GRFATR_ROTATION,     &::getCppuType((sal_Int16*)0), 0,   0},
                    { SW_PROP_NAME(UNO_NAME_ADJUST_LUMINANCE),      RES_GRFATR_LUMINANCE,    &::getCppuType((sal_Int16*)0), 0,   0},
                    { SW_PROP_NAME(UNO_NAME_ADJUST_CONTRAST),       RES_GRFATR_CONTRAST,     &::getCppuType((sal_Int16*)0), 0,   0},
                    { SW_PROP_NAME(UNO_NAME_ADJUST_RED),            RES_GRFATR_CHANNELR,     &::getCppuType((sal_Int16*)0), 0,   0},
                    { SW_PROP_NAME(UNO_NAME_ADJUST_GREEN),          RES_GRFATR_CHANNELG,     &::getCppuType((sal_Int16*)0), 0,   0},
                    { SW_PROP_NAME(UNO_NAME_ADJUST_BLUE),           RES_GRFATR_CHANNELB,     &::getCppuType((sal_Int16*)0), 0,   0},
                    { SW_PROP_NAME(UNO_NAME_GAMMA),         RES_GRFATR_GAMMA,        &::getCppuType((double*)0),        0,   0},
                    { SW_PROP_NAME(UNO_NAME_GRAPHIC_IS_INVERTED),       RES_GRFATR_INVERT,       &::getBooleanCppuType(),   0,   0},
                    { SW_PROP_NAME(UNO_NAME_TRANSPARENCY),  RES_GRFATR_TRANSPARENCY, &::getCppuType((sal_Int16*)0), 0,   0},
                    { SW_PROP_NAME(UNO_NAME_GRAPHIC_COLOR_MODE),    RES_GRFATR_DRAWMODE,     &::getCppuType((drawing::ColorMode*)0),        0,   0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aGraphicPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_EMBEDDED_OBJECT:
            {
                static SfxItemPropertyMap aEmbeddedPropertyMap_Impl[] =
                {
                    COMMON_FRAME_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aEmbeddedPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_SHAPE:
            {
                static SfxItemPropertyMap aShapeMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_ANCHOR_PAGE_NO),            RES_ANCHOR,             &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_PAGENUM      },
                    { SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE   ),            RES_ANCHOR,             &::getCppuType((text::TextContentAnchorType*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_ANCHOR_ANCHORTYPE},
                    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT  ),             RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_ORIENT   },
                    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_POSITION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_POSITION|CONVERT_TWIPS   },
                    { SW_PROP_NAME(UNO_NAME_HORI_ORIENT_RELATION),  RES_HORI_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_HORIORIENT_RELATION },
                    { SW_PROP_NAME(UNO_NAME_LEFT_MARGIN),           RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_L_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_RIGHT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_SURROUND                  ), RES_SURROUND,          &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_SURROUNDTYPE   },
                    { SW_PROP_NAME(UNO_NAME_TEXT_WRAP),                 RES_SURROUND,           &::getCppuType((text::WrapTextMode*)0),             PROPERTY_NONE, MID_SURROUND_SURROUNDTYPE    },
                    { SW_PROP_NAME(UNO_NAME_SURROUND_ANCHORONLY),   RES_SURROUND,           &::getBooleanCppuType(),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID, MID_SURROUND_ANCHORONLY     },
                    { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),                RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),             RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT  ),             RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_ORIENT   },
                    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_POSITION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_POSITION|CONVERT_TWIPS   },
                    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT_RELATION),  RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_VERTORIENT_RELATION },
                    { SW_PROP_NAME(UNO_NAME_TEXT_RANGE),                FN_TEXT_RANGE,          &::getCppuType((uno::Reference<text::XTextRange>*)0),       PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aShapeMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_MARK:
            {
                static SfxItemPropertyMap aIdxMarkMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_ALTERNATIVE_TEXT),WID_ALT_TEXT,         &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PRIMARY_KEY),       WID_PRIMARY_KEY,    &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_SECONDARY_KEY),   WID_SECONDARY_KEY,    &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_MAIN_ENTRY),   WID_MAIN_ENTRY,       &::getBooleanCppuType()  ,      PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aIdxMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_CNTIDX_MARK:
            {
                static SfxItemPropertyMap aCntntMarkMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_ALTERNATIVE_TEXT),WID_ALT_TEXT,         &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL        ),   WID_LEVEL        ,    &::getCppuType((const sal_Int16*)0)  ,      PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aCntntMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_USER_MARK:
            {
                static SfxItemPropertyMap aUserMarkMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_ALTERNATIVE_TEXT),WID_ALT_TEXT,         &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USER_INDEX_NAME), WID_USER_IDX_NAME,    &::getCppuType((const OUString*)0)  ,       PROPERTY_NONE,     0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aUserMarkMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_IDX:
            {
                static SfxItemPropertyMap aTOXIndexMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_CHAPTER               ),   WID_CREATE_FROM_CHAPTER                 ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_PROTECTED                         ),   WID_PROTECTED                           ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_ALPHABETICAL_SEPARATORS       ),   WID_USE_ALPHABETICAL_SEPARATORS         ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_KEY_AS_ENTRY                  ),   WID_USE_KEY_AS_ENTRY                    ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_COMBINED_ENTRIES              ),   WID_USE_COMBINED_ENTRIES                ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_CASE_SENSITIVE                 ),   WID_IS_CASE_SENSITIVE                   ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_P_P                           ),   WID_USE_P_P                             ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_DASH                          ),   WID_USE_DASH                            ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_UPPER_CASE                    ),   WID_USE_UPPER_CASE                      ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_FORMAT                      ),   WID_LEVEL_FORMAT                        ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,    0},
                //  { SW_PROP_NAME(UNO_NAME_???                               ),   WID_???                                 ,  &_getReflection  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME   ),   WID_MAIN_ENTRY_CHARACTER_STYLE_NAME     ,  &::getCppuType((const OUString*)0)  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),    RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),     RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ), RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { MAP_CHAR_LEN("ParaStyleHeading"),                 WID_PARA_HEAD,          &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleSeparator"),           WID_PARA_SEP,           &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel1"),              WID_PARA_LEV1,          &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel2"),              WID_PARA_LEV2,          &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel3"),              WID_PARA_LEV3,          &::getCppuType((const OUString*)0)  , 0,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_COMMA_SEPARATED),        WID_IS_COMMA_SEPARATED, &::getBooleanCppuType(),            PROPERTY_NONE ,0         },
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX_MARKS),      WID_INDEX_MARKS, new uno::Type(::getCppuType((Sequence< Reference< XDocumentIndexMark > >*)0)),         PropertyAttribute::READONLY ,0       },
#else
                    { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX_MARKS),      WID_INDEX_MARKS, &::getCppuType((Sequence< Reference< XDocumentIndexMark > >*)0),           PropertyAttribute::READONLY ,0       },
#endif
                    { SW_PROP_NAME(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXIndexMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_CNTNT:
            {
                static SfxItemPropertyMap aTOXContentMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NAME(UNO_NAME_LEVEL                             ), WID_LEVEL                               ,  &::getCppuType((const sal_Int16*)0)  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_MARKS                 ), WID_CREATE_FROM_MARKS                   ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_OUTLINE               ), WID_CREATE_FROM_OUTLINE                 ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                //  { SW_PROP_NAME(UNO_NAME_PARAGRAPH_STYLE_NAMES             ), WID_PARAGRAPH_STYLE_NAMES               ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_CHAPTER               ), WID_CREATE_FROM_CHAPTER                 ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_PROTECTED                         ), WID_PROTECTED                           ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_FORMAT                      ), WID_LEVEL_FORMAT                        ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_PARAGRAPH_STYLES            ), WID_LEVEL_PARAGRAPH_STYLES              ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PropertyAttribute::READONLY,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), WID_CREATE_FROM_PARAGRAPH_STYLES, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                //  { SW_PROP_NAME(UNO_NAME_RECALC_TAB_STOPS                  ), WID_RECALC_TAB_STOPS                    ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                //  { SW_PROP_NAME(UNO_NAME_???                               ), WID_???                                 ,  &_getReflection  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),     RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { MAP_CHAR_LEN("ParaStyleHeading"),     WID_PARA_HEAD,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel1"),  WID_PARA_LEV1,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel2"),  WID_PARA_LEV2,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel3"),  WID_PARA_LEV3,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel4"),  WID_PARA_LEV4,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel5"),  WID_PARA_LEV5,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel6"),  WID_PARA_LEV6,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel7"),  WID_PARA_LEV7,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel8"),  WID_PARA_LEV8,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel9"),  WID_PARA_LEV9,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel10"),     WID_PARA_LEV10,     &::getCppuType((const OUString*)0)  , 0,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, &::getBooleanCppuType(), PROPERTY_NONE, 0},
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX_MARKS),      WID_INDEX_MARKS, new uno::Type(::getCppuType((Sequence< Reference< XDocumentIndexMark > >*)0)),         PropertyAttribute::READONLY ,0       },
#else
                    { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX_MARKS),      WID_INDEX_MARKS, &::getCppuType((Sequence< Reference< XDocumentIndexMark > >*)0),           PropertyAttribute::READONLY ,0       },
#endif
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXContentMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_USER:
            {
                static SfxItemPropertyMap aTOXUserMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_MARKS                 ), WID_CREATE_FROM_MARKS                   ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                //  { SW_PROP_NAME(UNO_NAME_PARAGRAPH_STYLE_NAMES             ), WID_PARAGRAPH_STYLE_NAMES               ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_CHAPTER               ), WID_CREATE_FROM_CHAPTER                 ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_PROTECTED                         ), WID_PROTECTED                           ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_LEVEL_FROM_SOURCE             ), WID_USE_LEVEL_FROM_SOURCE               ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_FORMAT                      ), WID_LEVEL_FORMAT                        ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_PARAGRAPH_STYLES            ), WID_LEVEL_PARAGRAPH_STYLES              ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PropertyAttribute::READONLY,0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), WID_CREATE_FROM_PARAGRAPH_STYLES, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_TABLES                ), WID_CREATE_FROM_TABLES                  ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_TEXT_FRAMES           ), WID_CREATE_FROM_TEXT_FRAMES             ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS       ), WID_CREATE_FROM_GRAPHIC_OBJECTS         ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS      ), WID_CREATE_FROM_EMBEDDED_OBJECTS        ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),         RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { MAP_CHAR_LEN("ParaStyleHeading"),     WID_PARA_HEAD,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel1"),  WID_PARA_LEV1,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel2"),  WID_PARA_LEV2,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel3"),  WID_PARA_LEV3,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel4"),  WID_PARA_LEV4,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel5"),  WID_PARA_LEV5,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel6"),  WID_PARA_LEV6,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel7"),  WID_PARA_LEV7,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel8"),  WID_PARA_LEV8,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel9"),  WID_PARA_LEV9,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel10"),     WID_PARA_LEV10,     &::getCppuType((const OUString*)0)  , 0,     0},
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX_MARKS),  WID_INDEX_MARKS, new uno::Type(::getCppuType((Sequence< Reference< XDocumentIndexMark> >*)0)),          PropertyAttribute::READONLY ,0       },
#else
                    { SW_PROP_NAME(UNO_NAME_DOCUMENT_INDEX_MARKS),  WID_INDEX_MARKS, &::getCppuType((Sequence< Reference< XDocumentIndexMark> >*)0),            PropertyAttribute::READONLY ,0       },
#endif
                    { SW_PROP_NAME(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXUserMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_TABLES:
            {
                static SfxItemPropertyMap aTOXTablesMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_CHAPTER               ), WID_CREATE_FROM_CHAPTER                 ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_LABELS                ), WID_CREATE_FROM_LABELS                  ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_PROTECTED                         ), WID_PROTECTED                           ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LABEL_CATEGORY                    ), WID_LABEL_CATEGORY                      ,  &::getCppuType((const OUString*)0)  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LABEL_DISPLAY_TYPE                ), WID_LABEL_DISPLAY_TYPE                  ,  &::getCppuType((const sal_Int16*)0)  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_FORMAT                      ), WID_LEVEL_FORMAT                        ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,0},
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),         RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { MAP_CHAR_LEN("ParaStyleHeading"),     WID_PARA_HEAD,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel1"),  WID_PARA_LEV1,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXTablesMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_OBJECTS:
            {
                static SfxItemPropertyMap aTOXObjectsMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_CHAPTER               ), WID_CREATE_FROM_CHAPTER                 ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_PROTECTED                         ), WID_PROTECTED                           ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_ALPHABETICAL_SEPARATORS       ), WID_USE_ALPHABETICAL_SEPARATORS         ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_FORMAT                      ), WID_LEVEL_FORMAT                        ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_STAR_MATH             ), WID_CREATE_FROM_STAR_MATH               ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_STAR_CHART            ), WID_CREATE_FROM_STAR_CHART              ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_STAR_CALC             ), WID_CREATE_FROM_STAR_CALC               ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_STAR_DRAW             ), WID_CREATE_FROM_STAR_DRAW               ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS), WID_CREATE_FROM_OTHER_EMBEDDED_OBJECTS  ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),         RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { MAP_CHAR_LEN("ParaStyleHeading"),     WID_PARA_HEAD,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel1"),  WID_PARA_LEV1,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXObjectsMap_Impl;
            }
            break;
            case PROPERTY_MAP_INDEX_ILLUSTRATIONS:
            {
                static SfxItemPropertyMap aTOXIllustrationsMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_CHAPTER               ), WID_CREATE_FROM_CHAPTER                 ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CREATE_FROM_LABELS                ), WID_CREATE_FROM_LABELS                  ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_PROTECTED                         ), WID_PROTECTED                           ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USE_ALPHABETICAL_SEPARATORS       ), WID_USE_ALPHABETICAL_SEPARATORS         ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LABEL_CATEGORY                    ), WID_LABEL_CATEGORY                      ,  &::getCppuType((const OUString*)0)  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LABEL_DISPLAY_TYPE                ), WID_LABEL_DISPLAY_TYPE                  ,  &::getCppuType((const sal_Int16*)0)  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_FORMAT                      ), WID_LEVEL_FORMAT                        ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,0},
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),         RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { MAP_CHAR_LEN("ParaStyleHeading"),     WID_PARA_HEAD,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel1"),  WID_PARA_LEV1,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { SW_PROP_NAME(UNO_NAME_IS_RELATIVE_TABSTOPS), WID_IS_RELATIVE_TABSTOPS, &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTOXIllustrationsMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_ROW:
            {
                static SfxItemPropertyMap aTableRowPropertyMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_VERT_ORIENT  ),             RES_VERT_ORIENT,        &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE ,MID_VERTORIENT_ORIENT    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),         RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_TABLE_COLUMN_SEPARATORS),   FN_UNO_TABLE_COLUMN_SEPARATORS, new uno::Type(::getCppuType((const uno::Sequence<text::TableColumnSeparator>*)0)),      PropertyAttribute::MAYBEVOID, 0 },
#else
                    { SW_PROP_NAME(UNO_NAME_TABLE_COLUMN_SEPARATORS),   FN_UNO_TABLE_COLUMN_SEPARATORS, &::getCppuType((const uno::Sequence<text::TableColumnSeparator>*)0),    PropertyAttribute::MAYBEVOID, 0 },
#endif
                    { SW_PROP_NAME(UNO_NAME_HEIGHT),                FN_UNO_ROW_HEIGHT,      &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,CONVERT_TWIPS },
                    { SW_PROP_NAME(UNO_NAME_IS_AUTO_HEIGHT),            FN_UNO_ROW_AUTO_HEIGHT, &::getBooleanCppuType(),            PROPERTY_NONE , 0 },
                    { SW_PROP_NAME(UNO_NAME_SIZE_TYPE),                 RES_FRM_SIZE,           &::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE,   MID_FRMSIZE_SIZE_TYPE  },
                //  { SW_PROP_NAME(UNO_NAME_HEIGHT),                    RES_FRM_SIZE,           &::getCppuType((const sal_Int32*)0)  ,          PROPERTY_NONE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS         },
                //  { SW_PROP_NAME(UNO_NAME_SHADOW_FORMAT),             RES_SHADOW,             &::getCppuType((const table::ShadowFormat*)0),  PROPERTY_NONE, CONVERT_TWIPS},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTableRowPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_TABLE_CURSOR:
            {
                // das PropertySet entspricht dem Range ohne Chart-Properties
                static SfxItemPropertyMap aTableCursorPropertyMap_Impl [] =
                {
                    { SW_PROP_NAME(UNO_NAME_CHAR_AUTO_KERNING       ),  RES_CHRATR_AUTOKERN  ,  &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CASE_MAP),             RES_CHRATR_CASEMAP,     &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_STRIKEOUT),            RES_CHRATR_CROSSEDOUT,  &::getCppuType((const sal_Int16*)0),                    PropertyAttribute::MAYBEVOID, MID_CROSS_OUT},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CROSSED_OUT),          RES_CHRATR_CROSSEDOUT,  &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT),           RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int16*)0),            PROPERTY_NONE, MID_ESC          },
                    { SW_PROP_NAME(UNO_NAME_CHAR_ESCAPEMENT_HEIGHT),        RES_CHRATR_ESCAPEMENT,  &::getCppuType((const sal_Int8*)0)  ,       PROPERTY_NONE, MID_ESC_HEIGHT},
                    { SW_PROP_NAME(UNO_NAME_CHAR_FLASH              ),  RES_CHRATR_BLINK    ,   &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    _STANDARD_FONT_PROPERTIES
                    _CJK_FONT_PROPERTIES
                    _CTL_FONT_PROPERTIES
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE),            RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, MID_UNDERLINE},
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_COLOR),              RES_CHRATR_UNDERLINE ,  &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, MID_UL_COLOR},
                    { SW_PROP_NAME(UNO_NAME_CHAR_UNDERLINE_HAS_COLOR),              RES_CHRATR_UNDERLINE ,  &::getBooleanCppuType(),            PROPERTY_NONE, MID_UL_HASCOLOR},
                    { SW_PROP_NAME(UNO_NAME_PARA_LEFT_MARGIN),          RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_TXT_LMARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_RIGHT_MARGIN),             RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_R_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_PARA_FIRST_LINE_INDENT),        RES_LR_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_FIRST_LINE_INDENT|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_CHAR_KERNING            ),  RES_CHRATR_KERNING    , &::getCppuType((const sal_Int16*)0)  ,          PROPERTY_NONE,  0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_NO_HYPHENATION     ),  RES_CHRATR_NOHYPHEN ,   &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_SHADOWED),             RES_CHRATR_SHADOWED  ,  &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),           RES_TXTATR_CHARFMT,     &::getCppuType((const OUString*)0),         PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_CONTOURED),                RES_CHRATR_CONTOUR,     &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_FORMAT),           RES_PARATR_DROP,        &::getCppuType((const style::DropCapFormat*)0)  , PROPERTY_NONE, MID_DROPCAP_FORMAT     },
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_WHOLE_WORD),       RES_PARATR_DROP,        &::getBooleanCppuType()  ,          PROPERTY_NONE, MID_DROPCAP_WHOLE_WORD },
                    { SW_PROP_NAME(UNO_NAME_DROP_CAP_CHAR_STYLE_NAME),  RES_PARATR_DROP,        &::getCppuType((const OUString*)0)  ,       PropertyAttribute::MAYBEVOID, MID_DROPCAP_CHAR_STYLE_NAME },
                    { SW_PROP_NAME(UNO_NAME_PARA_KEEP_TOGETHER  ),      RES_KEEP,               &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_SPLIT      ),          RES_PARATR_SPLIT,       &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_WIDOWS),               RES_PARATR_WIDOWS,      &::getCppuType((const sal_Int8*)0),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_ORPHANS),              RES_PARATR_ORPHANS,      &::getCppuType((const sal_Int8*)0),PropertyAttribute::MAYBEVOID,     0},
                    { SW_PROP_NAME(UNO_NAME_NUMBER_FORMAT),                 RES_BOXATR_FORMAT,      &::getCppuType((const sal_Int32*)0),            PropertyAttribute::MAYBEVOID ,0             },
                    { SW_PROP_NAME(UNO_NAME_PAGE_NUMBER_OFFSET),            RES_PAGEDESC,           &::getCppuType((const sal_Int16*)0),        PROPERTY_NONE, MID_PAGEDESC_PAGENUMOFFSET},
                    { SW_PROP_NAME(UNO_NAME_PARA_ADJUST),                   RES_PARATR_ADJUST,      &::getCppuType((const sal_Int16*)0),        PROPERTY_NONE, MID_PARA_ADJUST},
                    { SW_PROP_NAME(UNO_NAME_PARA_EXPAND_SINGLE_WORD),     RES_PARATR_ADJUST,      &::getBooleanCppuType()  ,        PROPERTY_NONE, MID_EXPAND_SINGLE   },
                    { SW_PROP_NAME(UNO_NAME_PARA_LAST_LINE_ADJUST),       RES_PARATR_ADJUST,      &::getCppuType((const sal_Int16*)0),          PROPERTY_NONE, MID_LAST_LINE_ADJUST},
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_NUMBER_COUNT  ),  RES_LINENUMBER,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_LINENUMBER_COUNT     },
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_NUMBER_START_VALUE),RES_LINENUMBER,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_LINENUMBER_STARTVALUE},
                    { SW_PROP_NAME(UNO_NAME_PARA_LINE_SPACING),             RES_PARATR_LINESPACING, &::getCppuType((const style::LineSpacing*)0),PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_REGISTER_MODE_ACTIVE),      RES_PARATR_REGISTER,    &::getBooleanCppuType()  ,          PROPERTY_NONE, 0},
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),  RES_PARATR_TABSTOP,  new uno::Type(::getCppuType((uno::Sequence<style::TabStop>*)0)),       PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#else
                    { SW_PROP_NAME(UNO_NAME_TABSTOPS),                  RES_PARATR_TABSTOP,     &::getCppuType((const uno::Sequence<style::TabStop>*)0),        PropertyAttribute::MAYBEVOID, CONVERT_TWIPS},
#endif
                    { SW_PROP_NAME(UNO_NAME_TOP_MARGIN),                RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_UP_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_BOTTOM_MARGIN),             RES_UL_SPACE,           &::getCppuType((const sal_Int32*)0), PROPERTY_NONE, MID_LO_MARGIN|CONVERT_TWIPS},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COLOR),                    RES_CHRATR_COLOR,       &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE, 0},
                    { SW_PROP_NAME(UNO_NAME_CHAR_BACK_COLOR      ),     RES_CHRATR_BACKGROUND,  &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NAME(UNO_NAME_CHAR_BACK_TRANSPARENT), RES_CHRATR_BACKGROUND,  &::getBooleanCppuType(),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NAME(UNO_NAME_PARA_BACK_COLOR      ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NAME(UNO_NAME_PARA_BACK_TRANSPARENT), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),            RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),             RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_PARA_GRAPHIC_LOCATION    ),     RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),             FN_UNO_TABLE_CELL_BACKGROUND,   &::getCppuType((const style::GraphicLocation*)0), PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),         FN_UNO_TABLE_CELL_BACKGROUND,   &::getCppuType((const sal_Int32*)0),    PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_BACK_COLOR       },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ),     FN_UNO_TABLE_CELL_BACKGROUND,   &::getBooleanCppuType(),    PROPERTY_NONE|PropertyAttribute::MAYBEVOID ,MID_GRAPHIC_TRANSPARENT      },
                    { SW_PROP_NAME(UNO_NAME_WORD_MODE           ),          RES_CHRATR_WORDLINEMODE,&::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_PARA_STYLE_NAME),           FN_UNO_PARA_STYLE,      &::getCppuType((const OUString*)0),         PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_USER_DEFINED_ATTRIBUTES),   RES_UNKNOWNATR_CONTAINER, &::getCppuType((uno::Reference<container::XNameContainer>*)0), PropertyAttribute::MAYBEVOID, 0 },
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_IS_ON),            RES_CHRATR_TWO_LINES,           &::getBooleanCppuType(),    PROPERTY_NONE, MID_TWOLINES},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_PREFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PROPERTY_NONE, MID_START_BRACKET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_COMBINE_SUFFIX),           RES_CHRATR_TWO_LINES,           &::getCppuType((const OUString*)0),     PROPERTY_NONE, MID_END_BRACKET},
                    { SW_PROP_NAME(UNO_NAME_CHAR_EMPHASIS),             RES_CHRATR_EMPHASIS_MARK,           &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, MID_EMPHASIS},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTableCursorPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_BOOKMARK:
            {
                static SfxItemPropertyMap aBookmarkPropertyMap_Impl [] =
                {
                    { SW_PROP_NAME(UNO_LINK_DISPLAY_NAME),  FN_PARAM_LINK_DISPLAY_NAME,     &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0xff},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aBookmarkPropertyMap_Impl;
            }
            break;
            case PROPERTY_MAP_PARAGRAPH_EXTENSIONS:
            {
                static SfxItemPropertyMap aParagraphExtensionsMap_Impl[] =
                {
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0}
                };

                aMapArr[nPropertyId] = aParagraphExtensionsMap_Impl;
            }
            break;
            case PROPERTY_MAP_BIBLIOGRAPHY :
            {
                static SfxItemPropertyMap aBibliographyMap_Impl[] =
                {
                    _BASE_INDEX_PROPERTIES_
                    { SW_PROP_NAME(UNO_NAME_IS_PROTECTED          ),    WID_PROTECTED                           ,  &::getBooleanCppuType()  , PROPERTY_NONE,     0},
                    { SW_PROP_NAME(UNO_NAME_TEXT_COLUMNS),          RES_COL,                &::getCppuType((uno::Reference<text::XTextColumns>*)0),    PROPERTY_NONE, MID_COLUMNS},
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_URL      ),        RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_URL    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_FILTER  ),         RES_BACKGROUND,         &::getCppuType((const OUString*)0), PROPERTY_NONE ,MID_GRAPHIC_FILTER    },
                    { SW_PROP_NAME(UNO_NAME_BACK_GRAPHIC_LOCATION    ),         RES_BACKGROUND,         &::getCppuType((const style::GraphicLocation*)0),           PROPERTY_NONE ,MID_GRAPHIC_POSITION},
                    { SW_PROP_NAME(UNO_NAME_BACK_COLOR       ),     RES_BACKGROUND,         &::getCppuType((const sal_Int32*)0),            PROPERTY_NONE ,MID_BACK_COLOR        },
                    { SW_PROP_NAME(UNO_NAME_BACK_TRANSPARENT ), RES_BACKGROUND,         &::getBooleanCppuType(),            PROPERTY_NONE ,MID_GRAPHIC_TRANSPARENT       },
                    { MAP_CHAR_LEN("ParaStyleHeading"),     WID_PARA_HEAD,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { MAP_CHAR_LEN("ParaStyleLevel1"),  WID_PARA_LEV1,  &::getCppuType((const OUString*)0)  , 0,     0},
                    { SW_PROP_NAME(UNO_NAME_LEVEL_FORMAT                      ), WID_LEVEL_FORMAT                        ,  &::getCppuType((uno::Reference<container::XIndexReplace>*)0)  , PROPERTY_NONE,0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aBibliographyMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_DOCUMENT:
            {
                static SfxItemPropertyMap aDocMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_CHAR_LOCALE),                   RES_CHRATR_LANGUAGE ,   &::getCppuType((const lang::Locale*)0)  ,PropertyAttribute::MAYBEVOID,  MID_LANG_LOCALE },
                    { SW_PROP_NAME(UNO_NAME_CHARACTER_COUNT),               WID_DOC_CHAR_COUNT,         &::getCppuType((const sal_Int32*)0),    PropertyAttribute::READONLY,   0},
                    { SW_PROP_NAME(UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L),    WID_DOC_AUTO_MARK_URL, &::getCppuType((const OUString*)0),  PROPERTY_NONE,   0},
                    { SW_PROP_NAME(UNO_NAME_PARAGRAPH_COUNT),               WID_DOC_PARA_COUNT,         &::getCppuType((const sal_Int32*)0),    PropertyAttribute::READONLY,   0},
                    { SW_PROP_NAME(UNO_NAME_RECORD_CHANGES),                WID_DOC_CHANGES_RECORD,     &::getBooleanCppuType(),    PROPERTY_NONE,   0},
                    { SW_PROP_NAME(UNO_NAME_SHOW_CHANGES),                  WID_DOC_CHANGES_SHOW,       &::getBooleanCppuType(),    PROPERTY_NONE,   0},
                    { SW_PROP_NAME(UNO_NAME_WORD_COUNT),                    WID_DOC_WORD_COUNT,         &::getCppuType((const sal_Int32*)0),    PropertyAttribute::READONLY,   0},
                    { SW_PROP_NAME(UNO_NAME_WORD_SEPARATOR),                WID_DOC_WORD_SEPARATOR,     &::getCppuType((const OUString*)0), PROPERTY_NONE,   0},
                    { SW_PROP_NAME(UNO_NAME_HIDE_FIELD_TIPS),               WID_DOC_HIDE_TIPS,          &::getBooleanCppuType(),    PROPERTY_NONE,   0},
                    { SW_PROP_NAME(UNO_NAME_REDLINE_DISPLAY_TYPE),          WID_DOC_REDLINE_DISPLAY,    &::getCppuType((sal_Int16*)0),  PROPERTY_NONE,   0},
                    { SW_PROP_NAME(UNO_NAME_FORBIDDEN_CHARACTERS),          WID_DOC_FORBIDDEN_CHARS,    &::getCppuType((Reference<XForbiddenCharacters>*)0),    PropertyAttribute::READONLY,   0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aDocMap_Impl;
            }
            break;
            case PROPERTY_MAP_LINK_TARGET:
            {
                static SfxItemPropertyMap aLinkTargetMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_LINK_DISPLAY_BITMAP),            0,  &::getCppuType((const uno::Reference<awt::XBitmap>*)0), PropertyAttribute::READONLY, 0xff},
                    { SW_PROP_NAME(UNO_LINK_DISPLAY_NAME),          0,  &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0xff},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aLinkTargetMap_Impl;
            }
            break;
            case PROPERTY_MAP_AUTO_TEXT_GROUP :
            {
                static SfxItemPropertyMap aAutoTextGroupMap_Impl[] =
                {
                    { SW_PROP_NAME(UNO_NAME_FILE_PATH),     WID_GROUP_PATH,     &::getCppuType((const OUString*)0), PROPERTY_NONE,   PropertyAttribute::READONLY},
                    { SW_PROP_NAME(UNO_NAME_TITLE),         WID_GROUP_TITLE, &::getCppuType((const OUString*)0),    PROPERTY_NONE,   0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aAutoTextGroupMap_Impl;
            }
            break;
            case  PROPERTY_MAP_PRINT_SETTINGS:
            {
                static SfxItemPropertyMap aPrintSettingsMap_Impl[] =
                {
                    {SW_PROP_NAME(UNO_NAME_PRINT_ANNOTATION_MODE)  ,    WID_PRTSET_ANNOTATION_MODE , &::getCppuType((const sal_Int16*)0),  PROPERTY_NONE,   0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_BLACK_FONTS)      ,    WID_PRTSET_BLACK_FONTS     , &::getBooleanCppuType(),   PROPERTY_NONE,  0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_CONTROLS)         ,    WID_PRTSET_CONTROLS        , &::getBooleanCppuType(), PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_DRAWINGS)         ,    WID_PRTSET_DRAWINGS        , &::getBooleanCppuType(), PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_GRAPHICS)         ,    WID_PRTSET_GRAPHICS        , &::getBooleanCppuType(), PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_LEFT_PAGES)    ,       WID_PRTSET_LEFT_PAGES       ,   &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_PAGE_BACKGROUND)  ,    WID_PRTSET_PAGE_BACKGROUND , &::getBooleanCppuType(), PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_PROSPECT)         ,    WID_PRTSET_PROSPECT        , &::getBooleanCppuType(), PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_REVERSED)    ,         WID_PRTSET_REVERSED         ,   &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_PRING_RIGHT_PAGES) ,         WID_PRTSET_RIGHT_PAGES      ,   &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_PRINT_TABLES)           ,    WID_PRTSET_TABLES          , &::getBooleanCppuType(), PROPERTY_NONE,    0},
                    {0,0,0,0,0}
                };
                aMapArr[nPropertyId] = aPrintSettingsMap_Impl;
            }
            break;
            case  PROPERTY_MAP_VIEW_SETTINGS:
            {
                static SfxItemPropertyMap aViewSettingsMap_Impl[] =
                {
                    {SW_PROP_NAME(UNO_NAME_SHOW_ANNOTATIONS          ),WID_VIEWSET_ANNOTATIONS          , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_BREAKS               ),WID_VIEWSET_BREAKS               , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_DRAWINGS             ),WID_VIEWSET_DRAWINGS             , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_FIELD_COMMANDS       ),WID_VIEWSET_FIELD_COMMANDS       , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_FOOTNOTE_BACKGROUND  ),WID_VIEWSET_FOOTNOTE_BACKGROUND  , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_GRAPHICS             ),WID_VIEWSET_GRAPHICS             , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_HIDDEN_PARAGRAPHS    ),WID_VIEWSET_HIDDEN_PARAGRAPHS    , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_HIDDEN_TEXT          ),WID_VIEWSET_HIDDEN_TEXT          , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_HORI_RULER          ),WID_VIEWSET_HRULER              , &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_HORI_SCROLL_BAR     ),WID_VIEWSET_HSCROLL             , &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_INDEX_MARK_BACKGROUND),WID_VIEWSET_INDEX_MARK_BACKGROUND, &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_ONLINE_LAYOUT       ),WID_VIEWSET_ONLINE_LAYOUT       , &::getBooleanCppuType(),PropertyAttribute::MAYBEVOID,   0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_PARA_BREAKS          ),WID_VIEWSET_PARA_BREAKS          , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_PROTECTED_SPACES     ),WID_VIEWSET_PROTECTED_SPACES     , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_SOFT_HYPHENS         ),WID_VIEWSET_SOFT_HYPHENS         , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_SPACES               ),WID_VIEWSET_SPACES               , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_TABLE_BOUNDARIES     ),WID_VIEWSET_TABLE_BOUNDARIES     , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_TABLES               ),WID_VIEWSET_TABLES               , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_TABSTOPS             ),WID_VIEWSET_TABSTOPS             , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_TEXT_BOUNDARIES      ),WID_VIEWSET_TEXT_BOUNDARIES      , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_TEXT_FIELD_BACKGROUND),WID_VIEWSET_TEXT_FIELD_BACKGROUND, &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_VERT_RULER          ),WID_VIEWSET_VRULER ,  &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_SHOW_VERT_SCROLL_BAR     ),WID_VIEWSET_VSCROLL,  &::getBooleanCppuType(), PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_SMOOTH_SCROLLING          ),WID_VIEWSET_SMOOTH_SCROLLING     , &::getBooleanCppuType(),PROPERTY_NONE,    0},
                    {SW_PROP_NAME(UNO_NAME_SOLID_MARK_HANDLES       ),WID_VIEWSET_SOLID_MARK_HANDLES      , &::getBooleanCppuType(),PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_ZOOM_TYPE                 ),WID_VIEWSET_ZOOM_TYPE            , &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_ZOOM_VALUE               ),WID_VIEWSET_ZOOM              , &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aViewSettingsMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXTPORTION_EXTENSIONS:
            {
                static SfxItemPropertyMap aTextPortionExtensionMap_Impl[] =
                {
                    {SW_PROP_NAME(UNO_NAME_BOOKMARK),           0, &::getCppuType((const Reference<XTextContent>*)0),   PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY ,0 },
                    {SW_PROP_NAME(UNO_NAME_CONTROL_CHARACTER ), 0, &::getCppuType((const sal_Int16*)0),                 PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, MID_HYPHEN_MIN_LEAD   },
                    {SW_PROP_NAME(UNO_NAME_IS_COLLAPSED),       0, &::getBooleanCppuType(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    {SW_PROP_NAME(UNO_NAME_IS_START),           0, &::getBooleanCppuType(),                             PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY, 0 },
                    _REDLINE_PROPERTIES
                    {SW_PROP_NAME(UNO_NAME_TEXT_PORTION_TYPE),  0, &::getCppuType((OUString*)0),                        PropertyAttribute::READONLY,    0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTextPortionExtensionMap_Impl;
            }
            break;
            case PROPERTY_MAP_FOOTNOTE:
            {
                static SfxItemPropertyMap aFootnoteMap_Impl[] =
                {
                    {SW_PROP_NAME(UNO_NAME_REFERENCE_ID),   0, &::getCppuType((const sal_Int16*)0),PropertyAttribute::READONLY|PropertyAttribute::MAYBEVOID,    0},
                    COMMON_TEXT_CONTENT_PROPERTIES
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aFootnoteMap_Impl;
            }
            break;
            case PROPERTY_MAP_TEXT_COLUMS :
            {
                static SfxItemPropertyMap aTextColumns_Impl[] =
                {
                    {SW_PROP_NAME(UNO_NAME_SEPARATOR_LINE_WIDTH),               WID_TXTCOL_LINE_WIDTH, &::getCppuType((const sal_Int32*)0),PROPERTY_NONE,   0},
                    {SW_PROP_NAME(UNO_NAME_SEPARATOR_LINE_COLOR),               WID_TXTCOL_LINE_COLOR, &::getCppuType((const sal_Int32*)0),PROPERTY_NONE,   0},
                    {SW_PROP_NAME(UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT),     WID_TXTCOL_LINE_REL_HGT, &::getCppuType((const sal_Int32*)0),PROPERTY_NONE, 0},
                    {SW_PROP_NAME(UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT),  WID_TXTCOL_LINE_ALIGN, &::getCppuType((style::VerticalAlignment*)0),PROPERTY_NONE,  0},
                    {SW_PROP_NAME(UNO_NAME_SEPARATOR_LINE_IS_ON),               WID_TXTCOL_LINE_IS_ON, &::getCppuType((const sal_Int32*)0),PROPERTY_NONE,   0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aTextColumns_Impl;
            }
            break;
            case PROPERTY_MAP_REDLINE :
            {
                static SfxItemPropertyMap aRedlineMap_Impl[] =
                {
                    _REDLINE_PROPERTIES
                    {SW_PROP_NAME(UNO_NAME_REDLINE_START), 0, &::getCppuType((Reference<XInterface>*)0),    PropertyAttribute::READONLY,    0},
                    {SW_PROP_NAME(UNO_NAME_REDLINE_END  ), 0, &::getCppuType((Reference<XInterface>*)0),    PropertyAttribute::MAYBEVOID|PropertyAttribute::READONLY,   0},
                    {0,0,0,0}
                };
                aMapArr[nPropertyId] = aRedlineMap_Impl;
            }
            break;
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


