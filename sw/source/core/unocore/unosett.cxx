/*************************************************************************
 *
 *  $RCSfile: unosett.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-09-27 13:57:06 $
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
#include <cmdid.h>
#include <hintids.hxx>
#include "poolfmt.hrc"
#include "poolfmt.hxx"
#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif

#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _UNOSETT_HXX
#include <unosett.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef SW_LINEINFO_HXX //autogen
#include <lineinfo.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
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
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HPP_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINENUMBERPOSITION_HPP_
#include <com/sun/star/style/LineNumberPosition.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PropertyAttribute_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef SVX_UNOFDESC_HXX
#include <svx/unofdesc.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef SW_UNOMID_HXX
#include <unomid.h>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;

struct PropValData
{
    uno::Any        aVal;
    OUString            sPropName;
    PropValData(void* pVal, const char* cPropName, uno::Type aType ) :
        aVal(pVal, aType),
        sPropName(OUString::createFromAscii(cPropName))
        {}
    PropValData(const uno::Any& rVal, const OUString& rPropName) :
        aVal(rVal),
        sPropName(rPropName)
        {}
};

typedef PropValData* PropValDataPtr;
SV_DECL_PTRARR(PropValDataArr, PropValDataPtr, 5, 5 );
SV_IMPL_PTRARR(PropValDataArr, PropValDataPtr)


#define WID_PREFIX                      0
#define WID_SUFFIX                      1
#define WID_NUMBERING_TYPE              2
#define WID_START_AT                    3
#define WID_FOOTNOTE_COUNTING           4
#define WID_PARAGRAPH_STYLE             5
#define WID_PAGE_STYLE                  6
#define WID_CHARACTER_STYLE             7
#define WID_POSITION_END_OF_DOC         8
#define WID_END_NOTICE                  9
#define WID_BEGIN_NOTICE                10
#define WID_ANCHOR_CHARACTER_STYLE      11

const SfxItemPropertyMap* GetFootnoteMap()
{
    static SfxItemPropertyMap aFootnoteMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_ANCHOR_CHAR_STYLE_NAME),WID_ANCHOR_CHARACTER_STYLE, &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_BEGIN_NOTICE),          WID_BEGIN_NOTICE,       &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),       WID_CHARACTER_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_END_NOTICE),            WID_END_NOTICE ,        &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_FOOTNOTE_COUNTING),     WID_FOOTNOTE_COUNTING,  &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),        WID_NUMBERING_TYPE,     &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,         0},
        { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_NAME),       WID_PAGE_STYLE,         &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_PARA_STYLE_NAME),       WID_PARAGRAPH_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_POSITION_END_OF_DOC),   WID_POSITION_END_OF_DOC,&::getBooleanCppuType(), PROPERTY_NONE,         0},
        { SW_PROP_NAME(UNO_NAME_PREFIX),                WID_PREFIX,             &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_START_AT),              WID_START_AT ,          &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,         0},
        { SW_PROP_NAME(UNO_NAME_SUFFIX),                WID_SUFFIX,             &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        {0,0,0,0}
    };
    return aFootnoteMap_Impl;
}
const SfxItemPropertyMap* GetEndnoteMap()
{
    static SfxItemPropertyMap aEndnoteMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_ANCHOR_CHAR_STYLE_NAME),WID_ANCHOR_CHARACTER_STYLE, &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),       WID_CHARACTER_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),        WID_NUMBERING_TYPE,     &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,         0},
        { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_NAME),       WID_PAGE_STYLE,         &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_PARA_STYLE_NAME),       WID_PARAGRAPH_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_PREFIX),                WID_PREFIX,     &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_START_AT),              WID_START_AT ,          &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,         0},
        { SW_PROP_NAME(UNO_NAME_SUFFIX),                WID_SUFFIX,     &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        {0,0,0,0}
    };
    return aEndnoteMap_Impl;
}
const SfxItemPropertyMap* GetNumberingRulesMap()
{
    static SfxItemPropertyMap aNumberingRulesMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_IS_ABSOLUTE_MARGINS),       WID_IS_ABS_MARGINS, &::getBooleanCppuType(),            PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC),              WID_IS_AUTOMATIC,   &::getBooleanCppuType(),            PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_IS_CONTINUOUS_NUMBERING),   WID_CONTINUOUS,     &::getBooleanCppuType(),            PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NAME),                      WID_RULE_NAME   ,   &::getCppuType((const OUString*)0), PropertyAttribute::READONLY,     0},
        {0,0,0,0}
    };
    return aNumberingRulesMap_Impl;
}
#define WID_NUM_ON                      0
#define WID_SEPARATOR_LINE_DISTANCE     1
#define WID_NUMBERING_TYPE              2
#define WID_NUMBER_POSITION             3
#define WID_DISTANCE                    4
#define WID_LINE_INTERVAL               5
#define WID_LINE_SEPARATOR              6
//#define WID_CHARACTER_STYLE             7
#define WID_COUNT_EMPTY_LINES           8
#define WID_COUNT_LINES_IN_FRAMES       9

const SfxItemPropertyMap* GetLineNumberingMap()
{
    static SfxItemPropertyMap aLineNumberingMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),         WID_CHARACTER_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_COUNT_EMPTY_LINES),       WID_COUNT_EMPTY_LINES , &::getBooleanCppuType(),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_COUNT_LINES_IN_FRAMES),   WID_COUNT_LINES_IN_FRAMES, &::getBooleanCppuType(),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_DISTANCE       ),         WID_DISTANCE       ,    &::getCppuType((const sal_Int32*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_LINE_INTERVAL  ),         WID_LINE_INTERVAL  ,    &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_LINE_SEPARATOR ),         WID_LINE_SEPARATOR,     &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBER_POSITION),         WID_NUMBER_POSITION,    &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),          WID_NUMBERING_TYPE ,    &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_ON),                        WID_NUM_ON,             &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_SEPARATOR_LINE_DISTANCE), WID_SEPARATOR_LINE_DISTANCE, &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        {0,0,0,0}
    };
    return aLineNumberingMap_Impl;
}

/* -----------------05.05.98 08:30-------------------
 *
 * --------------------------------------------------*/
SwCharFmt* lcl_getCharFmt(SwDoc* pDoc, const uno::Any& aValue)
{
    SwCharFmt* pRet = 0;
    String sStandard(SW_RES(STR_POOLCOLL_STANDARD));
    OUString uTmp;
    aValue >>= uTmp;
    String sCharFmt(SwXStyleFamilies::GetUIName(uTmp, SFX_STYLE_FAMILY_CHAR));
    if(sStandard != sCharFmt)
    {
        pRet = pDoc->FindCharFmtByName( sCharFmt );
    }
    if(!pRet)
    {
        sal_uInt16 nId = pDoc->GetPoolId(sCharFmt, GET_POOLID_CHRFMT);
        if(USHRT_MAX != nId)
            pRet = pDoc->GetCharFmtFromPool( nId );
    }
    return pRet;
}
/* -----------------05.05.98 08:30-------------------
 *
 * --------------------------------------------------*/
SwTxtFmtColl* lcl_GetParaStyle(SwDoc* pDoc, const uno::Any& aValue)
{
    OUString uTmp;
    aValue >>= uTmp;
    String sParaStyle(SwXStyleFamilies::GetUIName(uTmp, SFX_STYLE_FAMILY_PARA));
    SwTxtFmtColl* pRet = pDoc->FindTxtFmtCollByName( sParaStyle );
    if( !pRet  )
    {
        sal_uInt16 nId = pDoc->GetPoolId( sParaStyle, GET_POOLID_TXTCOLL );
        if( USHRT_MAX != nId  )
            pRet = pDoc->GetTxtCollFromPool( nId );
    }
    return pRet;
}
/* -----------------05.05.98 08:30-------------------
 *
 * --------------------------------------------------*/
SwPageDesc* lcl_GetPageDesc(SwDoc* pDoc, const uno::Any& aValue)
{
    SwPageDesc* pRet = 0;
    sal_uInt16 nCount = pDoc->GetPageDescCnt();
    OUString uTmp;
    aValue >>= uTmp;
    String sPageDesc(SwXStyleFamilies::GetUIName(uTmp, SFX_STYLE_FAMILY_PAGE));
    for( sal_uInt16 i = 0; i < nCount; i++)
    {
        const SwPageDesc& rDesc = pDoc->GetPageDesc( i );
        if(rDesc.GetName() == sPageDesc)
        {
            pRet = (SwPageDesc*)&rDesc;
            break;
        }
    }
    if(!pRet)
    {
        sal_uInt16 nId = pDoc->GetPoolId(sPageDesc, GET_POOLID_PAGEDESC);
        if(USHRT_MAX != nId)
            pRet = pDoc->GetPageDescFromPool( nId );
    }
    return pRet;
}
/******************************************************************************
 *
 ******************************************************************************/
// Numerierung
const unsigned short aSvxToUnoAdjust[] =
{
    text::HoriOrientation::LEFT,    //3
    text::HoriOrientation::RIGHT,  //1
    USHRT_MAX,
    text::HoriOrientation::CENTER, //2
    USHRT_MAX,
    USHRT_MAX
};

const unsigned short aUnoToSvxAdjust[] =
{
    USHRT_MAX,
    SVX_ADJUST_RIGHT,       // 1
    SVX_ADJUST_CENTER,      // 3
    SVX_ADJUST_LEFT,        // 0
    USHRT_MAX,
    USHRT_MAX
};

/******************************************************************
 * SwXFootnoteProperties
 ******************************************************************/
/* -----------------------------06.04.00 11:43--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFootnoteProperties::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFootnoteProperties");
}
/* -----------------------------06.04.00 11:43--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXFootnoteProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.FootnoteProperties") == rServiceName;
}
/* -----------------------------06.04.00 11:43--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXFootnoteProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.FootnoteProperties");
    return aRet;
}
/*-- 14.12.98 14:03:20---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFootnoteProperties::SwXFootnoteProperties(SwDoc* pDc) :
    pDoc(pDc),
    _pMap(GetFootnoteMap())
{
}
/*-- 14.12.98 14:03:20---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFootnoteProperties::~SwXFootnoteProperties()
{

}
/*-- 14.12.98 14:03:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXFootnoteProperties::getPropertySetInfo(void)
                                                                throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}
/*-- 14.12.98 14:03:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(pDoc)
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            SwFtnInfo aFtnInfo(pDoc->GetFtnInfo());
            switch(pMap->nWID)
            {
                case WID_PREFIX:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFtnInfo.SetPrefix(uTmp);
                }
                break;
                case WID_SUFFIX:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFtnInfo.SetSuffix(uTmp);
                }
                break;
                case  WID_NUMBERING_TYPE :
                {
                    INT16 nTmp;
                    aValue >>= nTmp;
                    aFtnInfo.aFmt.eType = (SvxExtNumType)nTmp;
                }
                break;
                case  WID_START_AT:
                {
                    INT16 nTmp;
                    aValue >>= nTmp;
                    aFtnInfo.nFtnOffset = nTmp;
                }
                break;
                case  WID_FOOTNOTE_COUNTING  :
                {
                    sal_uInt16 nRet = 0;
                    INT16 nTmp;
                    aValue >>= nTmp;
                    switch(nTmp)
                    {
                        case  text::FootnoteNumbering::PER_PAGE:
                            aFtnInfo.eNum = FTNNUM_PAGE;
                        break;
                        case  text::FootnoteNumbering::PER_CHAPTER:
                            aFtnInfo.eNum = FTNNUM_CHAPTER;
                        break;
                        case  text::FootnoteNumbering::PER_DOCUMENT:
                            aFtnInfo.eNum = FTNNUM_DOC;
                        break;
                    }
                }
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTxtFmtColl* pColl = lcl_GetParaStyle(pDoc, aValue);
                    if(pColl)
                        aFtnInfo.SetFtnTxtColl(*pColl);
                }
                break;
                case  WID_PAGE_STYLE :
                {
                    SwPageDesc* pDesc = lcl_GetPageDesc(pDoc, aValue);
                    if(pDesc)
                        aFtnInfo.ChgPageDesc( pDesc );
                }
                break;
                case WID_ANCHOR_CHARACTER_STYLE:
                case  WID_CHARACTER_STYLE    :
                {
                    SwCharFmt* pFmt = lcl_getCharFmt(pDoc, aValue);
                    if(pFmt)
                    {
                        if(pMap->nWID == WID_ANCHOR_CHARACTER_STYLE)
                            aFtnInfo.SetAnchorCharFmt(pFmt);
                        else
                            aFtnInfo.SetCharFmt(pFmt);
                    }
                }
                break;
                case  WID_POSITION_END_OF_DOC:
                {
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                    aFtnInfo.ePos = bVal ? FTNPOS_CHAPTER : FTNPOS_PAGE;
                }
                break;
                case  WID_END_NOTICE         :
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFtnInfo.aQuoVadis = String(uTmp);
                }
                break;
                case  WID_BEGIN_NOTICE       :
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aFtnInfo.aErgoSum = String(uTmp);
                }
                break;
            }
            pDoc->SetFtnInfo(aFtnInfo);
        }
        else
            throw lang::IllegalArgumentException();
    }
    else
        throw uno::RuntimeException();
}
/*-- 14.12.98 14:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXFootnoteProperties::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    if(pDoc)
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();
            switch(pMap->nWID)
            {
                case WID_PREFIX:
                {
                    aRet <<= OUString(rFtnInfo.GetPrefix());
                }
                break;
                case WID_SUFFIX:
                {
                    aRet <<= OUString(rFtnInfo.GetSuffix());
                }
                break;
                case  WID_NUMBERING_TYPE :
                {
                    aRet <<= (sal_Int16)rFtnInfo.aFmt.eType;
                }
                break;
                case  WID_START_AT:
                    aRet <<= (sal_Int16)rFtnInfo.nFtnOffset;
                break;
                case  WID_FOOTNOTE_COUNTING  :
                {
                    sal_Int16 nRet = 0;
                    switch(rFtnInfo.eNum)
                    {
                        case  FTNNUM_PAGE:
                            nRet = text::FootnoteNumbering::PER_PAGE;
                        break;
                        case  FTNNUM_CHAPTER:
                            nRet = text::FootnoteNumbering::PER_CHAPTER;
                        break;
                        case  FTNNUM_DOC:
                            nRet = text::FootnoteNumbering::PER_DOCUMENT;
                        break;
                    }
                    aRet <<= nRet;
                }
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTxtFmtColl* pColl = rFtnInfo.GetFtnTxtColl();
                    OUString sRet;
                    if(pColl)
                        sRet = pColl->GetName();
                    sRet = SwXStyleFamilies::GetProgrammaticName(sRet, SFX_STYLE_FAMILY_PARA);
                    aRet <<= sRet;
                }
                break;
                case  WID_PAGE_STYLE :
                {
                    aRet <<= OUString(
                        SwXStyleFamilies::GetProgrammaticName(
                            rFtnInfo.GetPageDesc( *pDoc )->GetName(),
                            SFX_STYLE_FAMILY_PAGE));
                }
                break;
                case WID_ANCHOR_CHARACTER_STYLE:
                case WID_CHARACTER_STYLE:
                {
                    const SwCharFmt* pCharFmt = pMap->nWID == WID_ANCHOR_CHARACTER_STYLE ?
                            rFtnInfo.GetAnchorCharFmt(*pDoc) : rFtnInfo.GetCharFmt(*pDoc);
                    aRet <<= OUString(
                        SwXStyleFamilies::GetProgrammaticName(pCharFmt->GetName(), SFX_STYLE_FAMILY_CHAR));
                }
                break;
                case  WID_POSITION_END_OF_DOC:
                {
                    sal_Bool bTemp = FTNPOS_CHAPTER == rFtnInfo.ePos;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case  WID_END_NOTICE         :
                    aRet <<= OUString(rFtnInfo.aQuoVadis);
                break;
                case  WID_BEGIN_NOTICE       :
                    aRet <<= OUString(rFtnInfo.aErgoSum);
                break;
            }
        }
        else
            throw lang::IllegalArgumentException();

    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 14.12.98 14:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::addPropertyChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::removePropertyChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::addVetoableChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:03:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::removeVetoableChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

/******************************************************************
 * SwXEndnoteProperties
 ******************************************************************/
/* -----------------------------06.04.00 11:45--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXEndnoteProperties::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXEndnoteProperties");
}
/* -----------------------------06.04.00 11:45--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXEndnoteProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.EndnoteProperties") == rServiceName;
}
/* -----------------------------06.04.00 11:45--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXEndnoteProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.EndnoteProperties");
    return aRet;
}
/*-- 14.12.98 14:27:39---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXEndnoteProperties::SwXEndnoteProperties(SwDoc* pDc) :
    pDoc(pDc),
    _pMap(GetEndnoteMap())
{

}
/*-- 14.12.98 14:27:39---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXEndnoteProperties::~SwXEndnoteProperties()
{

}
/*-- 14.12.98 14:27:40---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXEndnoteProperties::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}
/*-- 14.12.98 14:27:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(pDoc)
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            SwEndNoteInfo aEndInfo(pDoc->GetEndNoteInfo());
            switch(pMap->nWID)
            {
                case WID_PREFIX:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aEndInfo.SetPrefix(uTmp);
                }
                break;
                case WID_SUFFIX:
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aEndInfo.SetSuffix(uTmp);
                }
                break;
                case  WID_NUMBERING_TYPE :
                {
                    INT16 nTmp;
                    aValue >>= nTmp;
                    aEndInfo.aFmt.eType = (SvxExtNumType)nTmp;
                }
                break;
                case  WID_START_AT:
                {
                    INT16 nTmp;
                    aValue >>= nTmp;
                    aEndInfo.nFtnOffset = nTmp;
                }
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTxtFmtColl* pColl = lcl_GetParaStyle(pDoc, aValue);
                    if(pColl)
                        aEndInfo.SetFtnTxtColl(*pColl);
                }
                break;
                case  WID_PAGE_STYLE :
                {
                    SwPageDesc* pDesc = lcl_GetPageDesc(pDoc, aValue);
                    if(pDesc)
                        aEndInfo.ChgPageDesc( pDesc );
                }
                break;
                case  WID_CHARACTER_STYLE    :
                {
                    SwCharFmt* pFmt = lcl_getCharFmt(pDoc, aValue);
                    if(pFmt)
                        aEndInfo.SetCharFmt(pFmt);
                }
                break;
            }
            pDoc->SetEndNoteInfo(aEndInfo);
        }
    }
}
/*-- 14.12.98 14:27:41---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXEndnoteProperties::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    if(pDoc)
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
            switch(pMap->nWID)
            {
                case WID_PREFIX:
                    aRet <<= OUString(rEndInfo.GetPrefix());
                break;
                case WID_SUFFIX:
                    aRet <<= OUString(rEndInfo.GetSuffix());
                break;
                case  WID_NUMBERING_TYPE :
                    aRet <<= (sal_Int16)rEndInfo.aFmt.eType;
                break;
                case  WID_START_AT:
                    aRet <<= (sal_Int16)rEndInfo.nFtnOffset;
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTxtFmtColl* pColl = rEndInfo.GetFtnTxtColl();
                    OUString sRet;
                    if(pColl)
                        sRet = pColl->GetName();
                    aRet <<= OUString(
                        SwXStyleFamilies::GetProgrammaticName(sRet,
                            SFX_STYLE_FAMILY_PARA));

                }
                break;
                case  WID_PAGE_STYLE :
                {
                    aRet <<= OUString(
                        SwXStyleFamilies::GetProgrammaticName(
                            rEndInfo.GetPageDesc( *pDoc )->GetName(),
                            SFX_STYLE_FAMILY_PAGE));
                }
                break;
                case  WID_CHARACTER_STYLE    :
                {
                    const SwCharFmt* pCharFmt = rEndInfo.GetCharFmt(*pDoc);
                    aRet <<= OUString(
                        SwXStyleFamilies::GetProgrammaticName(pCharFmt->GetName(), SFX_STYLE_FAMILY_CHAR));
                }
                break;
            }
        }
    }
    return aRet;
}
/*-- 14.12.98 14:27:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::addPropertyChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:27:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::removePropertyChangeListener(const OUString& PropertyName,
        const uno:: Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:27:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::addVetoableChangeListener(const OUString& PropertyName,
    const uno:: Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:27:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::removeVetoableChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/******************************************************************
 * SwXLineNumberingProperties
 ******************************************************************/
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXLineNumberingProperties::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXLineNumberingProperties");
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXLineNumberingProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.LineNumberingSettings") == rServiceName;
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXLineNumberingProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.LineNumberingSettings");
    return aRet;
}
/*-- 14.12.98 14:33:36---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXLineNumberingProperties::SwXLineNumberingProperties(SwDoc* pDc) :
    pDoc(pDc),
    _pMap(GetLineNumberingMap())
{

}
/*-- 14.12.98 14:33:37---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXLineNumberingProperties::~SwXLineNumberingProperties()
{

}
/*-- 14.12.98 14:33:37---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXLineNumberingProperties::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}
/*-- 14.12.98 14:33:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::setPropertyValue(
    const OUString& rPropertyName, const Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException,
                IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(pDoc)
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            SwLineNumberInfo  aInfo(pDoc->GetLineNumberInfo());
            switch(pMap->nWID)
            {
                case WID_NUM_ON:
                {
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                    aInfo.SetPaintLineNumbers(bVal);
                }
                break;
                case WID_CHARACTER_STYLE :
                {
                    SwCharFmt* pFmt = lcl_getCharFmt(pDoc, aValue);
                    if(pFmt)
                        aInfo.SetCharFmt(pFmt);
                }
                break;
                case WID_NUMBERING_TYPE  :
                {
                    SwNumType aNumType(aInfo.GetNumType());
                    INT16 nTmp;
                    aValue >>= nTmp;
                    aNumType.eType = (SvxExtNumType)nTmp;
                    aInfo.SetNumType(aNumType);
                }
                break;
                case WID_NUMBER_POSITION :
                {
                    INT16 nTmp;
                    aValue >>= nTmp;
                    switch(nTmp)
                    {
                        case  style::LineNumberPosition::LEFT:
                             aInfo.SetPos(LINENUMBER_POS_LEFT); ;
                        break;
                        case style::LineNumberPosition::RIGHT :
                             aInfo.SetPos(LINENUMBER_POS_RIGHT);       ;
                        break;
                        case  style::LineNumberPosition::INSIDE:
                            aInfo.SetPos(LINENUMBER_POS_INSIDE);      ;
                        break;
                        case  style::LineNumberPosition::OUTSIDE:
                            aInfo.SetPos(LINENUMBER_POS_OUTSIDE);
                        break;
                    }
                }
                break;
                case WID_DISTANCE        :
                {
                    INT32 nVal;
                    aValue >>= nVal;
                    aInfo.SetPosFromLeft(Min(nVal, sal_Int32(0xffff)));
                }
                break;
                case WID_LINE_INTERVAL   :
                {
                    INT16 nTmp;
                    aValue >>= nTmp;
                    aInfo.SetCountBy(nTmp);
                }
                break;
                case WID_LINE_SEPARATOR  :
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aInfo.SetDivider(uTmp);
                }
                break;
                case WID_SEPARATOR_LINE_DISTANCE:
                {
                    INT16 nTmp;
                    aValue >>= nTmp;
                    aInfo.SetDividerCountBy(nTmp);
                }
                break;
                case WID_COUNT_EMPTY_LINES :
                {
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                    aInfo.SetCountBlankLines(bVal);
                }
                break;
                case WID_COUNT_LINES_IN_FRAMES :
                {
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                    aInfo.SetCountInFlys(bVal);
                }
                break;
            }
            pDoc->SetLineNumberInfo(aInfo);
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();
}
/*-- 14.12.98 14:33:38---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXLineNumberingProperties::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;
    if(pDoc)
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            const SwLineNumberInfo& rInfo = pDoc->GetLineNumberInfo();
            switch(pMap->nWID)
            {
                case WID_NUM_ON:
                {
                    sal_Bool bTemp = rInfo.IsPaintLineNumbers();
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case WID_CHARACTER_STYLE :
                    aRet <<= OUString(rInfo.GetCharFmt(*pDoc)->GetName());
                break;
                case WID_NUMBERING_TYPE  :
                    aRet <<= (sal_Int16)rInfo.GetNumType().eType;
                break;
                case WID_NUMBER_POSITION :
                {
                    sal_Int16 nRet = 0;
                    switch(rInfo.GetPos())
                    {
                        case  LINENUMBER_POS_LEFT:
                            nRet = style::LineNumberPosition::LEFT;
                        break;
                        case LINENUMBER_POS_RIGHT :
                            nRet = style::LineNumberPosition::RIGHT      ;
                        break;
                        case  LINENUMBER_POS_INSIDE:
                            nRet = style::LineNumberPosition::INSIDE     ;
                        break;
                        case LINENUMBER_POS_OUTSIDE :
                            nRet = style::LineNumberPosition::OUTSIDE    ;
                        break;
                    }
                    aRet <<= nRet;
                }
                break;
                case WID_DISTANCE        :
                {
                    sal_uInt32 nPos = rInfo.GetPosFromLeft();
                    if(USHRT_MAX == nPos)
                        nPos = 0;
                    aRet <<= nPos;
                }
                break;
                case WID_LINE_INTERVAL   :
                    aRet <<= (sal_Int16)rInfo.GetCountBy();
                break;
                case WID_LINE_SEPARATOR  :
                    aRet <<= OUString(rInfo.GetDivider());
                break;
                case WID_SEPARATOR_LINE_DISTANCE:
                    aRet <<= (sal_Int16)rInfo.GetDividerCountBy();
                break;
                case WID_COUNT_EMPTY_LINES :
                {
                    sal_Bool bTemp = rInfo.IsCountBlankLines();
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case WID_COUNT_LINES_IN_FRAMES :
                {
                    sal_Bool bTemp = rInfo.IsCountInFlys();
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
            }
        }
        else
            throw beans::UnknownPropertyException();
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 14.12.98 14:33:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::addPropertyChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:33:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::removePropertyChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:33:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::addVetoableChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented")
}
/*-- 14.12.98 14:33:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::removeVetoableChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented")
}
/******************************************************************
 * SwXNumberingRules
 ******************************************************************/
String  SwXNumberingRules::sInvalidStyle(String::CreateFromAscii("__XXX___invalid"));

const String&   SwXNumberingRules::GetInvalidStyle()
{
    return sInvalidStyle;
}
/* -----------------------------10.03.00 17:05--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXNumberingRules::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 17:05--------------------------------

 ---------------------------------------------------------------------------*/
// return implementation specific data
sal_Int64 SwXNumberingRules::getSomething( const uno::Sequence< sal_Int8 > & rId ) throw(::com::sun::star::uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
        {
                return (sal_Int64)this;
        }
    return 0;
}

/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXNumberingRules::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXNumberingRules");
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXNumberingRules::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.NumberingRules") == rServiceName;
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXNumberingRules::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.NumberingRules");
    return aRet;
}
/*-- 14.12.98 14:57:56---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXNumberingRules::SwXNumberingRules() :
    pNumRule(0),
    pDoc(0),
    pDocShell(0),
    _pMap(GetNumberingRulesMap())
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        sNewCharStyleNames[i] = SwXNumberingRules::GetInvalidStyle();
        sNewBulletFontNames[i] = SwXNumberingRules::GetInvalidStyle();
    }

}
/*-- 14.12.98 14:57:57---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXNumberingRules::SwXNumberingRules(const SwNumRule& rRule) :
    pNumRule(new SwNumRule(rRule)),
    pDoc(0),
    pDocShell(0),
    _pMap(GetNumberingRulesMap())
{
    //erstmal das Doc organisieren; es haengt an den gesetzten Zeichenvorlagen - wenn
    // keine gesetzt sind, muss es auch ohne gehen
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
    {
        SwNumFmt rFmt(pNumRule->Get(i));
        SwCharFmt* pCharFmt = rFmt.GetCharFmt();
        if(pCharFmt)
        {
            pDoc = pCharFmt->GetDoc();
            break;
        }
    }
    if(pDoc)
        pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    for(i = 0; i < MAXLEVEL; i++)
    {
        sNewCharStyleNames[i] = SwXNumberingRules::GetInvalidStyle();
        sNewBulletFontNames[i] = SwXNumberingRules::GetInvalidStyle();
    }
}
/* -----------------22.02.99 16:35-------------------
 *
 * --------------------------------------------------*/
SwXNumberingRules::SwXNumberingRules(SwDocShell& rDocSh) :
    pDoc(0),
    pNumRule(0),
    pDocShell(&rDocSh),
    _pMap(GetNumberingRulesMap())
{
    pDocShell->GetDoc()->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}
/* -----------------------------24.08.00 11:36--------------------------------

 ---------------------------------------------------------------------------*/
SwXNumberingRules::SwXNumberingRules(SwDoc& rDoc) :
    pDoc(&rDoc),
    pNumRule(0),
    pDocShell(0),
    _pMap(GetNumberingRulesMap())
{
    rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    sCreatedNumRuleName = rDoc.GetUniqueNumRuleName();
    sal_uInt16 nIndex = rDoc.MakeNumRule( sCreatedNumRuleName, 0 );
}

/* -----------------28.10.99 09:40-------------------

 --------------------------------------------------*/
SwXNumberingRules::SwXNumberingRules(SwDoc& rDoc, const String& rName) :
    pNumRule(new SwNumRule(rName)),
    pDoc(&rDoc),
    pDocShell(0),
    _pMap(GetNumberingRulesMap())
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}
/*-- 14.12.98 14:57:57---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXNumberingRules::~SwXNumberingRules()
{
    delete pNumRule;
}
/*-- 14.12.98 14:57:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException,
                  lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(rElement.getValueType().getTypeClass() != uno::TypeClass_SEQUENCE)
        throw lang::IllegalArgumentException();
    const uno::Sequence<beans::PropertyValue>& rProperties =
                    *(const uno::Sequence<beans::PropertyValue>*)rElement.getValue();
    if(pNumRule)
        SwXNumberingRules::setNumberingRuleByIndex( *pNumRule,
                            rProperties, nIndex);
    else if(pDocShell)
    {
        const SwNumRule* pNumRule = pDocShell->GetDoc()->GetOutlineNumRule();
        SwNumRule aNumRule(*pNumRule);
        SwXNumberingRules::setNumberingRuleByIndex( aNumRule,
                            rProperties, nIndex);
        //hier noch die Zeichenformate bei Bedarf setzen
        const SwCharFmts* pFmts = pDocShell->GetDoc()->GetCharFmts();
        sal_uInt16 nChCount = pFmts->Count();
        for(sal_uInt16 i = 0; i < MAXLEVEL;i++)
        {
            SwNumFmt aFmt(aNumRule.Get( i ));
            if(sNewCharStyleNames[i].Len() &&
                COMPARE_EQUAL != sNewCharStyleNames[i].CompareToAscii(UNO_NAME_CHARACTER_FORMAT_NONE) &&
                   (!aFmt.GetCharFmt() ||
                    aFmt.GetCharFmt()->GetName()!= sNewCharStyleNames[i] ))
            {
                SwCharFmt* pCharFmt = 0;
                for(sal_uInt16 j = 0; j< nChCount; j++)
                {
                    SwCharFmt* pTmp = (*pFmts)[j];
                    if(pTmp->GetName() == sNewCharStyleNames[i])
                    {
                        pCharFmt = pTmp;
                        break;
                    }
                }
                if(!pCharFmt)
                {
                    SfxStyleSheetBase* pBase;
                    pBase = pDocShell->GetStyleSheetPool()->Find(sNewCharStyleNames[i],
                                                                    SFX_STYLE_FAMILY_CHAR);
                    if(!pBase)
                        pBase = &pDocShell->GetStyleSheetPool()->Make(sNewCharStyleNames[i], SFX_STYLE_FAMILY_CHAR);
                    pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();

                }
                aFmt.SetCharFmt( pCharFmt );
                aNumRule.Set( i, aFmt );
            }
        }
        pDocShell->GetDoc()->SetOutlineNumRule( aNumRule );
    }
    else
        throw uno::RuntimeException();

}
/*-- 14.12.98 14:57:58---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXNumberingRules::getCount(void) throw( uno::RuntimeException )
{
    return MAXLEVEL;
}
/*-- 14.12.98 14:57:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXNumberingRules::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
            uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aVal;
    if(MAXLEVEL <= nIndex)
        throw lang::IndexOutOfBoundsException();
    if(pNumRule)
    {
        uno::Sequence<beans::PropertyValue> aRet = getNumberingRuleByIndex(
                                        *pNumRule, nIndex);
        aVal.setValue(&aRet, ::getCppuType((uno::Sequence<beans::PropertyValue>*)0));

    }
    else if(pDocShell)
    {
        uno::Sequence<beans::PropertyValue> aRet = getNumberingRuleByIndex(
                *pDocShell->GetDoc()->GetOutlineNumRule(), nIndex);
        aVal.setValue(&aRet, ::getCppuType((uno::Sequence<beans::PropertyValue>*)0));
    }
    else
        throw uno::RuntimeException();
    return aVal;
}
/*-- 14.12.98 14:57:59---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXNumberingRules::getElementType(void)
    throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Sequence<beans::PropertyValue>*)0);
}
/*-- 14.12.98 14:57:59---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXNumberingRules::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}
/*-- 14.12.98 14:57:59---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence<beans::PropertyValue> SwXNumberingRules::getNumberingRuleByIndex(
                const SwNumRule& rNumRule, sal_Int32 nIndex) const
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const SwNumFmt& rFmt = rNumRule.Get( (sal_uInt16)nIndex );

    sal_Bool bChapterNum = pDocShell != 0;

    PropValDataArr  aPropertyValues;
    //fill all properties into the array

    //adjust
    SvxAdjust eAdj = rFmt.GetAdjust();
    sal_Int16 nINT16 = aSvxToUnoAdjust[(sal_uInt16)eAdj];
    PropValData* pData = new PropValData((void*)&nINT16, "Adjust", ::getCppuType((const sal_Int16*)0) );
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //parentnumbering
    nINT16 = rFmt.GetUpperLevel();
    pData = new PropValData((void*)&nINT16, "ParentNumbering", ::getCppuType((const sal_Int16*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //prefix
    OUString aUString = rFmt.GetPrefix();
    pData = new PropValData((void*)&aUString, "Prefix", ::getCppuType((const OUString*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //suffix
    aUString = rFmt.GetPostfix();
    pData = new PropValData((void*)&aUString, "Suffix", ::getCppuType((const OUString*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //char style name
    SwCharFmt* pCharFmt = rFmt.GetCharFmt();
    String CharStyleName;
    if(pCharFmt)
        CharStyleName = pCharFmt->GetName();
    //egal ob ein Style vorhanden ist oder nicht ueberschreibt der Array-Eintrag diesen String
    if(sNewCharStyleNames[(sal_uInt16)nIndex].Len() &&
        SwXNumberingRules::sInvalidStyle != sNewCharStyleNames[(sal_uInt16)nIndex])
        CharStyleName = sNewCharStyleNames[(sal_uInt16)nIndex];

    aUString = SwXStyleFamilies::GetProgrammaticName( CharStyleName,
                                                      SFX_STYLE_FAMILY_CHAR );
    pData = new PropValData((void*)&aUString, "CharStyleName", ::getCppuType((const OUString*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //startvalue
    nINT16 = rFmt.GetStartValue();
    pData = new PropValData((void*)&nINT16, "StartWith", ::getCppuType((const sal_Int16*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //leftmargin
    sal_Int32 nINT32 = TWIP_TO_MM100(rFmt.GetAbsLSpace());
    pData = new PropValData((void*)&nINT32, UNO_NAME_LEFT_MARGIN, ::getCppuType((const sal_Int32*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //chartextoffset
    nINT32 = TWIP_TO_MM100(rFmt.GetCharTextOffset());
    pData = new PropValData((void*)&nINT32, UNO_NAME_SYMBOL_TEXT_DISTANCE, ::getCppuType((const sal_Int32*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //firstlineoffset
    nINT32 = TWIP_TO_MM100(rFmt.GetFirstLineOffset());
    pData = new PropValData((void*)&nINT32, UNO_NAME_FIRST_LINE_OFFSET, ::getCppuType((const sal_Int32*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //TODO: Enum fuer NumberingType erweitern
    //
    nINT16 = rFmt.eType;
    pData = new PropValData((void*)&nINT16, "NumberingType", ::getCppuType((const sal_Int16*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    if(!bChapterNum)
    {
        if(SVX_NUM_CHAR_SPECIAL == rFmt.eType)
        {
            //BulletId
            nINT16 = rFmt.GetBulletChar();
            pData = new PropValData((void*)&nINT16, "BulletId", ::getCppuType((const sal_Int16*)0));
            aPropertyValues.Insert(pData, aPropertyValues.Count());

            const Font* pFont = rFmt.GetBulletFont();

            //BulletChar
            aUString = OUString(rFmt.GetBulletChar());
            pData = new PropValData((void*)&aUString, "BulletChar", ::getCppuType((const OUString*)0));
            aPropertyValues.Insert(pData, aPropertyValues.Count());

            //BulletFontName
            String sBulletFontName;
            if(pFont)
                sBulletFontName = pFont->GetStyleName();
            aUString = sBulletFontName;
            pData = new PropValData((void*)&aUString, "BulletFontName", ::getCppuType((const OUString*)0));
            aPropertyValues.Insert(pData, aPropertyValues.Count());

            //BulletFont
            if(pFont)
            {
                 awt::FontDescriptor aDesc;
                SvxUnoFontDescriptor::ConvertFromFont( *pFont, aDesc );
                pData = new PropValData((void*)&aDesc, UNO_NAME_BULLET_FONT, ::getCppuType((const awt::FontDescriptor*)0));
                aPropertyValues.Insert(pData, aPropertyValues.Count());
            }
        }
        if(SVX_NUM_BITMAP == rFmt.eType)
        {
            //GraphicURL
            String sGrURL;
            const SvxBrushItem* pBrush = rFmt.GetGrfBrush();
                if(pBrush && pBrush->GetGraphicLink())
                sGrURL = *pBrush->GetGraphicLink();
            aUString = sGrURL;
            pData = new PropValData((void*)&aUString, UNO_NAME_GRAPHIC_URL, ::getCppuType((const OUString*)0));
            aPropertyValues.Insert(pData, aPropertyValues.Count());

            //graphicbitmap
            const Graphic* pGraphic = 0;
            if(pBrush )
                pGraphic = pBrush->GetGraphic();
            if(pGraphic)
            {
                uno::Reference<awt::XBitmap> xBmp = VCLUnoHelper::CreateBitmap( pGraphic->GetBitmapEx() );
                pData = new PropValData((void*)&xBmp, UNO_NAME_GRAPHIC_BITMAP,
                                ::getCppuType((const uno::Reference<awt::XBitmap>*)0));
                aPropertyValues.Insert(pData, aPropertyValues.Count());
            }
             Size aSize = rFmt.GetGrfSize();
            aSize.Width() = TWIP_TO_MM100( aSize.Width() );
            aSize.Height() = TWIP_TO_MM100( aSize.Height() );
            pData = new PropValData((void*)&aSize, UNO_NAME_GRAPHIC_SIZE, ::getCppuType((const awt::Size*)0));
            aPropertyValues.Insert(pData, aPropertyValues.Count());

            const SwFmtVertOrient* pOrient = rFmt.GetGrfOrient();
            if(pOrient)
            {
                pData = new PropValData((void*)0, UNO_NAME_VERT_ORIENT, ::getCppuType((const sal_Int16*)0));
                ((const SfxPoolItem*)pOrient)->QueryValue(pData->aVal, MID_VERTORIENT_ORIENT);
                aPropertyValues.Insert(pData, aPropertyValues.Count());
            }
        }

    }
    else
    {
        //Vorlagenname
        String sValue(SW_RES(STR_POOLCOLL_HEADLINE1 + nIndex));
        const SwTxtFmtColls* pColls = pDocShell->GetDoc()->GetTxtFmtColls();
        const sal_uInt16 nCount = pColls->Count();
        for(sal_uInt16 i = 0; i < nCount;++i)
        {
            SwTxtFmtColl &rTxtColl = *pColls->operator[](i);
            if(rTxtColl.IsDefault())
                continue;

            sal_Int8 nOutLevel = rTxtColl.GetOutlineLevel();
            if(nOutLevel == nIndex)
            {
                sValue = rTxtColl.GetName();
            }
        }
        aUString = sValue;
        pData = new PropValData((void*)&aUString, UNO_NAME_HEADING_STYLE_NAME, ::getCppuType((const OUString*)0));
        aPropertyValues.Insert(pData, aPropertyValues.Count());
    }

    uno::Sequence<beans::PropertyValue> aSeq(aPropertyValues.Count());
    beans::PropertyValue* pArray = aSeq.getArray();

    for(sal_uInt16 i = 0; i < aPropertyValues.Count(); i++)
    {
        PropValDataPtr pData = aPropertyValues.GetObject(i);
        pArray[i].Value = pData->aVal;
        pArray[i].Name = pData->sPropName;
        pArray[i].Handle = -1;
    }
    aPropertyValues.DeleteAndDestroy(0, aPropertyValues.Count());
    return aSeq;
}
/*-- 14.12.98 14:57:59---------------------------------------------------

  -----------------------------------------------------------------------*/
PropValData* lcl_FindProperty(const char* cName, PropValDataArr&    rPropertyValues)
{
    OUString sCmp = C2U(cName);
    for(sal_uInt16 i = 0; i < rPropertyValues.Count(); i++)
    {
        PropValData* pTemp = rPropertyValues.GetObject(i);
        if(sCmp == pTemp->sPropName)
            return pTemp;
    }
    return 0;
}
//-----------------------------------------------------------------------

void SwXNumberingRules::setNumberingRuleByIndex(
            SwNumRule& rNumRule,
            const uno::Sequence<beans::PropertyValue>& rProperties, sal_Int32 nIndex)
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // the order of the names is important!
    const char* aNumPropertyNames[] =
    {
        "Adjust",                               //0
        "ParentNumbering",                      //1
        "Prefix",                               //2
        "Suffix",                               //3
        "CharStyleName",                        //4
        "StartWith",                            //5
        UNO_NAME_LEFT_MARGIN,                   //6
        UNO_NAME_SYMBOL_TEXT_DISTANCE,          //7
        UNO_NAME_FIRST_LINE_OFFSET,             //8
        "NumberingType",                        //9
        "BulletId",                             //10
        UNO_NAME_BULLET_FONT,                   //11
        "BulletFontName",                       //12
        "BulletChar",                           //13
        UNO_NAME_GRAPHIC_URL,                   //14
        UNO_NAME_GRAPHIC_BITMAP,                //15
        UNO_NAME_GRAPHIC_SIZE,                  //16
        UNO_NAME_VERT_ORIENT,                   //17
        UNO_NAME_HEADING_STYLE_NAME             //18
    };
    const sal_uInt16 nPropNameCount = 19;
    const sal_uInt16 nNotInChapter = 10;


    SwNumFmt aFmt(rNumRule.Get( (sal_uInt16)nIndex ));
    const beans::PropertyValue* pPropArray = rProperties.getConstArray();
    PropValDataArr  aPropertyValues;
    sal_Bool bExcept = sal_False;
    for(int i = 0; i < rProperties.getLength() && !bExcept; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        bExcept = sal_True;
        for(sal_uInt16 j = 0; j < (pDocShell ? nPropNameCount : nPropNameCount - 1); j++)
        {
            //some values not in chapter numbering
            if(pDocShell && j == nNotInChapter)
                j = nPropNameCount - 1;
            if(COMPARE_EQUAL == rProp.Name.compareToAscii(aNumPropertyNames[j]))
            {
                bExcept = sal_False;
                break;
            }
        }
        PropValData* pData = new PropValData(rProp.Value, rProp.Name );
        aPropertyValues.Insert(pData, aPropertyValues.Count());
    }
    sal_Bool bWrongArg = sal_False;
    if(!bExcept)
       {
        SvxBrushItem* pSetBrush = 0;
        Size* pSetSize = 0;
        SwFmtVertOrient* pSetVOrient = 0;

        for(sal_uInt16 i = 0; i < nPropNameCount && !bExcept && !bWrongArg; i++)
        {
            PropValData* pData = lcl_FindProperty(aNumPropertyNames[i], aPropertyValues);
            if(!pData)
                continue;
            switch(i)
            {
                case 0: //"Adjust"
                {
                    sal_Int16 nValue;
                    pData->aVal >>= nValue;
                    if(nValue > 0 &&
                        nValue <= text::HoriOrientation::LEFT &&
                            USHRT_MAX != aUnoToSvxAdjust[nValue])
                    {
                        aFmt.SetAdjust((SvxAdjust)aUnoToSvxAdjust[nValue]);
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 1: //"ParentNumbering",
                {
                    sal_Int16 nSet;
                    pData->aVal >>= nSet;
                    if(nSet >= 0 && MAXLEVEL >= nSet)
                        aFmt.SetUpperLevel(nSet);
                }
                break;
                case 2: //"Prefix",
                {
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    aFmt.SetPrefix(uTmp);
                }
                break;
                case 3: //"Suffix",
                {
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    aFmt.SetPostfix(uTmp);
                }
                break;
                case 4: //"CharStyleName",
                {
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    String sCharFmtName(
                        SwXStyleFamilies::GetUIName( uTmp,
                                                    SFX_STYLE_FAMILY_CHAR ) );
                    SwCharFmt* pCharFmt = 0;
                    if(sCharFmtName.EqualsAscii(UNO_NAME_CHARACTER_FORMAT_NONE))
                    {
                        sNewCharStyleNames[(sal_uInt16)nIndex] = SwXNumberingRules::GetInvalidStyle();
                        aFmt.SetCharFmt(0);
                    }
                    else if(pDocShell || pDoc)
                    {
                        SwDoc* pLocalDoc = pDoc ? pDoc : pDocShell->GetDoc();
                        const SwCharFmts* pFmts = pLocalDoc->GetCharFmts();
                        sal_uInt16 nChCount = pFmts->Count();

                        SwCharFmt* pCharFmt = 0;
                        if(sCharFmtName.Len())
                        {
                            for(sal_uInt16 j = 0; j< nChCount; j++)
                            {
                                SwCharFmt* pTmp = (*pFmts)[j];
                                if(pTmp->GetName() == sCharFmtName)
                                {
                                    pCharFmt = pTmp;
                                    break;
                                }
                            }
                            if(!pCharFmt)
                            {

                                SfxStyleSheetBase* pBase;
                                SfxStyleSheetBasePool* pPool = pLocalDoc->GetDocShell()->GetStyleSheetPool();
                                pBase = ((SfxStyleSheetBasePool*)pPool)->Find(sCharFmtName, SFX_STYLE_FAMILY_CHAR);
                                if(!pBase)
                                    pBase = &pPool->Make(sCharFmtName, SFX_STYLE_FAMILY_CHAR);
                                pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();
                            }
                        }
                        aFmt.SetCharFmt( pCharFmt );
                        sNewCharStyleNames[(sal_uInt16)nIndex] = sCharFmtName;
                     }
                    else
                        sNewCharStyleNames[(sal_uInt16)nIndex] = sCharFmtName;
                }
                break;
                case 5: //"StartWith",
                {
                    INT16 nVal;
                    pData->aVal >>= nVal;
                    aFmt.SetStartValue(nVal);
                }
                break;
                case 6: //UNO_NAME_LEFT_MARGIN,
                {
                    sal_Int32 nValue;
                    pData->aVal >>= nValue;
                    if(nValue >= 0)
                        aFmt.SetAbsLSpace((sal_uInt16) MM100_TO_TWIP(nValue));
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 7: //UNO_NAME_SYMBOL_TEXT_DISTANCE,
                {
                    sal_Int32 nValue;
                    pData->aVal >>= nValue;
                    if(nValue >= 0)
                        aFmt.SetCharTextOffset((sal_uInt16) MM100_TO_TWIP(nValue));
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 8: //UNO_NAME_FIRST_LINE_OFFSET,
                {
                    sal_Int32 nValue;
                    pData->aVal >>= nValue;
                    if(nValue <= 0)
                    {
                        nValue = MM100_TO_TWIP(nValue);
                        if(-nValue > aFmt.GetAbsLSpace())
                            aFmt.SetAbsLSpace(-nValue);
                        aFmt.SetFirstLineOffset((short)nValue);
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 9: //"NumberingType"
                {
                    sal_Int16 nSet;
                    pData->aVal >>= nSet;
                    if(nSet <= (sal_Int16)SVX_NUM_CHARS_LOWER_LETTER_N)
                        aFmt.eType = (SvxExtNumType)nSet;
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 10: //"BulletId",
                {
                    sal_Int16 nSet;
                    pData->aVal >>= nSet;
                    if(nSet < 0xff)
                        aFmt.SetBulletChar(nSet);
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 11: //UNO_NAME_BULLET_FONT,
                {
                     awt::FontDescriptor* pDesc =  (awt::FontDescriptor*)pData->aVal.getValue();
                    if(pDesc)
                    {
                        Font aFont;
                        SvxUnoFontDescriptor::ConvertToFont( *pDesc, aFont );
                        aFmt.SetBulletFont(&aFont);
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 12: //"BulletFontName",
                {
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    String sBulletFontName(uTmp);
                    sNewBulletFontNames[(sal_uInt16)nIndex] = sBulletFontName;
                }
                break;
                case 13: //"BulletChar",
                {
                    OUString aChar;
                    pData->aVal >>= aChar;
                    if(aChar.getLength() == 1)
                    {
                        aFmt.SetBulletChar(aChar.toChar());
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 14: //UNO_NAME_GRAPHIC_URL,
                {
                    OUString sBrushURL;
                    pData->aVal >>= sBrushURL;
                    if(!pSetBrush)
                    {
                        const SvxBrushItem* pOrigBrush = aFmt.GetGrfBrush();
                        if(pOrigBrush)
                        {
                            pSetBrush = new SvxBrushItem(*pOrigBrush);
                        }
                        else
                            pSetBrush = new SvxBrushItem(sBrushURL, aEmptyStr, GPOS_AREA);
                    }
                    pSetBrush->SetGraphicLink( sBrushURL );
                }
                break;
                case 15: //UNO_NAME_GRAPHIC_BITMAP,
                {
                    uno::Reference< awt::XBitmap >* pBitmap = (uno::Reference< awt::XBitmap > *)pData->aVal.getValue();
                    if(pBitmap)
                    {
                        if(!pSetBrush)
                        {
                            const SvxBrushItem* pOrigBrush = aFmt.GetGrfBrush();
                            if(pOrigBrush)
                            {
                                pSetBrush = new SvxBrushItem(*pOrigBrush);
                            }
                            else
                                pSetBrush = new SvxBrushItem(aEmptyStr, aEmptyStr, GPOS_AREA);
                        }

                        BitmapEx aBmp = VCLUnoHelper::GetBitmap( *pBitmap );
                        Graphic aNewGr(aBmp);
                        pSetBrush->SetGraphic( aNewGr );
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 16: //UNO_NAME_GRAPHIC_SIZE,
                {
                    if(!pSetSize)
                        pSetSize = new Size;
                    if(pData->aVal.getValueType() == ::getCppuType((awt::Size*)0))
                    {
                         awt::Size* pSize =  (awt::Size*)pData->aVal.getValue();
                        pSize->Width = MM100_TO_TWIP(pSize->Width);
                        pSize->Height = MM100_TO_TWIP(pSize->Height);
                        pSetSize->Width() = pSize->Width;
                        pSetSize->Height() = pSize->Height;
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 17: //VertOrient
                {
                    if(!pSetVOrient)
                    {
                        if(aFmt.GetGrfOrient())
                            pSetVOrient = (SwFmtVertOrient*)aFmt.GetGrfOrient()->Clone();
                        else
                            pSetVOrient = new SwFmtVertOrient;
                    }
                    ((SfxPoolItem*)pSetVOrient)->PutValue(pData->aVal, MID_VERTORIENT_ORIENT);
                }
                break;
                case 18: //"HeadingStyleName"
                {
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    String sStyleName = uTmp;
                    const SwTxtFmtColls* pColls = pDocShell->GetDoc()->GetTxtFmtColls();
                    const sal_uInt16 nCount = pColls->Count();
                    for(sal_uInt16 i = 0; i < nCount; ++i)
                    {
                        SwTxtFmtColl &rTxtColl = *pColls->operator[](i);
                        if(rTxtColl.IsDefault())
                            continue;
                        if(rTxtColl.GetOutlineLevel() == nIndex &&
                            rTxtColl.GetName() != sStyleName)
                            rTxtColl.SetOutlineLevel(NO_NUMBERING);
                        else if(rTxtColl.GetName() == sStyleName)
                            rTxtColl.SetOutlineLevel(sal_Int8(nIndex));
                    }
                }
                break;
            }
        }
        if(!bExcept && !bWrongArg && (pSetBrush || pSetSize || pSetVOrient))
        {
            if(!pSetBrush && aFmt.GetGrfBrush())
                pSetBrush = new SvxBrushItem(*aFmt.GetGrfBrush());

            if(pSetBrush)
            {
                if(!pSetVOrient && aFmt.GetGrfOrient())
                    pSetVOrient = new SwFmtVertOrient(*aFmt.GetGrfOrient());

                if(!pSetSize)
                {
                    pSetSize = new Size(aFmt.GetGrfSize());
                    if(!pSetSize->Width() || !pSetSize->Height())
                    {
                        const Graphic* pGraphic = pSetBrush->GetGraphic();
                        *pSetSize = ::GetGraphicSizeTwip(*pGraphic, 0);
                    }
                }
                aFmt.SetGrfBrush( pSetBrush, pSetSize, pSetVOrient );
            }
        }
        delete pSetBrush;
        delete pSetSize;
        delete pSetVOrient;
      }
    aPropertyValues.DeleteAndDestroy(0, aPropertyValues.Count());

    if(bWrongArg)
        throw lang::IllegalArgumentException();
    else if(bExcept)
        throw uno::RuntimeException();
    rNumRule.Set( (sal_uInt16)nIndex, aFmt );

}
/*-- 19.07.00 07:49:17---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo > SwXNumberingRules::getPropertySetInfo()
    throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}
/*-- 19.07.00 07:49:17---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
    throw(UnknownPropertyException, PropertyVetoException,
        IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    SwNumRule* pDocRule = 0;
    if(!pNumRule && pDocShell)
    {
        pDocRule = new SwNumRule(*pDocShell->GetDoc()->GetOutlineNumRule());
    }
    if(!pNumRule && !pDocRule)
        throw RuntimeException();


    if(0 == rPropertyName.compareToAscii(UNO_NAME_IS_AUTOMATIC))
    {
        BOOL bVal = *(sal_Bool*)rValue.getValue();
        pDocRule ? pDocRule->SetAutoRule(bVal) :pNumRule->SetAutoRule(bVal);
    }
    else if(0 == rPropertyName.compareToAscii(UNO_NAME_IS_CONTINUOUS_NUMBERING))
    {
        BOOL bVal = *(sal_Bool*)rValue.getValue();
        pDocRule ? pDocRule->SetContinusNum(bVal) : pNumRule->SetContinusNum(bVal);
    }
    else if(0 == rPropertyName.compareToAscii(UNO_NAME_NAME))
    {
        delete pDocRule;
        throw IllegalArgumentException();
    }
    else if(0 == rPropertyName.compareToAscii(UNO_NAME_IS_ABSOLUTE_MARGINS))
    {
        BOOL bVal = *(sal_Bool*)rValue.getValue();
        pDocRule ? pDocRule->SetAbsSpaces(bVal) : pNumRule->SetAbsSpaces(bVal);
    }
    else
        throw UnknownPropertyException();
    if(pDocRule)
    {
        pDocShell->GetDoc()->SetOutlineNumRule(*pDocRule);
        delete pDocRule;
    }
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXNumberingRules::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    const SwNumRule* pRule = pNumRule;
    if(!pRule && pDocShell)
        pRule = pDocShell->GetDoc()->GetOutlineNumRule();
    if(!pRule)
        throw RuntimeException();

    if(0 == rPropertyName.compareToAscii(UNO_NAME_IS_AUTOMATIC))
    {
        BOOL bVal = pRule->IsAutoRule();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else if(0 == rPropertyName.compareToAscii(UNO_NAME_IS_CONTINUOUS_NUMBERING))
    {
        BOOL bVal = pRule->IsContinusNum();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else if(0 == rPropertyName.compareToAscii(UNO_NAME_NAME))
        aRet <<= OUString(pRule->GetName());
    else if(0 == rPropertyName.compareToAscii(UNO_NAME_IS_ABSOLUTE_MARGINS))
    {
        BOOL bVal = pRule->IsAbsSpaces();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else
        throw UnknownPropertyException();
    return aRet;
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::addPropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 14.12.98 14:58:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        delete pNumRule;
        pNumRule = 0;
        pDoc = 0;
    }
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXChapterNumbering::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXChapterNumbering");
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXChapterNumbering::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.ChapterNumbering") ||
            sServiceName.EqualsAscii("com.sun.star.text.NumberingRules");
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXChapterNumbering::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ChapterNumbering");
    pArray[1] = C2U("com.sun.star.text.NumberingRules");
    return aRet;
}
/* -----------------22.02.99 16:33-------------------
 *
 * --------------------------------------------------*/
SwXChapterNumbering::SwXChapterNumbering(SwDocShell& rDocSh) :
    SwXNumberingRules(rDocSh)
{
}
/* -----------------22.02.99 16:33-------------------
 *
 * --------------------------------------------------*/
SwXChapterNumbering::~SwXChapterNumbering()
{
}

/******************************************************************
 * SwXTextColumns
 ******************************************************************/
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextColumns::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextColumns");
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextColumns::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.XTextColumns") == rServiceName;
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextColumns::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.XTextColumns");
    return aRet;
}
/*-- 16.12.98 14:06:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextColumns::SwXTextColumns(const SwFmtCol& rFmtCol) :
    aTextColumns(rFmtCol.GetNumCols()),
    nReference(0)
{
    TextColumn* pColumns = aTextColumns.getArray();
    const SwColumns& rCols = rFmtCol.GetColumns();
    for(sal_uInt16 i = 0; i < aTextColumns.getLength(); i++)
    {
        SwColumn* pCol = rCols[i];

        pColumns[i].Width = pCol->GetWishWidth();
        nReference += pColumns[i].Width;
        pColumns[i].LeftMargin =    TWIP_TO_MM100(pCol->GetLeft ());
        pColumns[i].RightMargin =   TWIP_TO_MM100(pCol->GetRight());
    }
    if(!aTextColumns.getLength())
        nReference = USHRT_MAX;

}
/*-- 16.12.98 14:06:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextColumns::~SwXTextColumns()
{

}
/*-- 16.12.98 14:06:54---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXTextColumns::getReferenceValue(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return nReference;
}
/*-- 16.12.98 14:06:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 SwXTextColumns::getColumnCount(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return aTextColumns.getLength();
}
/*-- 16.12.98 14:06:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::setColumnCount(sal_Int16 nColumns) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(nColumns <= 0)
        throw uno::RuntimeException();
    aTextColumns.realloc(nColumns);
 text::TextColumn* pCols = aTextColumns.getArray();
    nReference = USHRT_MAX;
    sal_uInt16 nWidth = nReference / nColumns;
    sal_uInt16 nDiff = nReference - nWidth * nColumns;
    for(sal_Int16 i = 0; i < nColumns; i++)
    {
        pCols[i].Width = nWidth;
        pCols[i].LeftMargin = 0;
        pCols[i].RightMargin = 0;
    }
    pCols[nColumns - 1].Width += nDiff;
}
/*-- 16.12.98 14:06:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< text::TextColumn > SwXTextColumns::getColumns(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return aTextColumns;
}
/*-- 16.12.98 14:06:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::setColumns(const uno::Sequence< text::TextColumn >& rColumns)
            throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_uInt16 nReferenceTemp = 0;
    const text::TextColumn* prCols = rColumns.getConstArray();
    for(long i = 0; i < rColumns.getLength(); i++)
    {
        //wenn die Breite Null ist, oder die Raender breiter als die Spalte werden -> exception
        if(!prCols[i].Width ||
            prCols[i].LeftMargin + prCols[i].RightMargin >= prCols[i].Width)
            throw uno::RuntimeException();
        nReferenceTemp += prCols[i].Width;
    }
    nReference = !nReferenceTemp ? USHRT_MAX : nReferenceTemp;
    aTextColumns = rColumns;
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/19 00:08:29  hr
    initial import

    Revision 1.64  2000/09/18 16:04:35  willem.vandorp
    OpenOffice header added.

    Revision 1.63  2000/09/18 12:34:26  os
    property AnchorCharStyleName in Footnotes/Endnotes

    Revision 1.62  2000/09/05 15:17:55  os
    string length available again

    Revision 1.61  2000/08/25 09:03:37  os
    service NumberingRules added

    Revision 1.60  2000/08/24 11:14:34  mib
    bug fixes for XML import

    Revision 1.59  2000/08/10 08:25:05  mib
    #74404#: NumberingCharStyleName now handles programmatic names

    Revision 1.58  2000/08/07 09:16:51  os
    #77157# service name LineNumberingSettings

    Revision 1.57  2000/07/19 11:01:02  os
    #76846# properties added

    Revision 1.56  2000/07/05 09:00:26  os
    #76619# service name added

    Revision 1.55  2000/06/20 10:51:30  os
    #70059# set character style name if pDocShell or pDoc is set

    Revision 1.54  2000/05/25 09:55:12  hr
    exception specification

    Revision 1.53  2000/05/16 09:14:55  os
    project usr removed

    Revision 1.52  2000/04/26 11:35:20  os
    GetName() returns String&

    Revision 1.51  2000/04/19 13:35:31  os
    UNICODE

    Revision 1.50  2000/04/11 08:31:04  os
    UNICODE

    Revision 1.49  2000/03/31 06:06:48  os
    UNO III: toolkit includes

    Revision 1.48  2000/03/27 10:21:10  os
    UNO III

    Revision 1.47  2000/03/21 15:42:25  os
    UNOIII

    Revision 1.46  2000/02/11 14:35:52  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.45  2000/02/02 16:52:53  sub
    #65293#: PropertyValue changes

    Revision 1.44  1999/12/03 11:11:11  os
    #70234# property name defined

    Revision 1.43  1999/11/22 10:37:57  os
    missing headers added

    Revision 1.42  1999/11/19 16:40:19  os
    modules renamed

    Revision 1.41  1999/11/04 07:44:46  mib
    SwXNumberingRule bug fixes

    Revision 1.40  1999/11/01 11:40:21  mib
    SwXNumberingRules bug fixes

    Revision 1.39  1999/10/28 07:44:05  os
    new properties/changes for XML

    Revision 1.38  1999/07/07 05:41:04  OS
    #67461# check index


      Rev 1.37   07 Jul 1999 07:41:04   OS
   #67461# check index

      Rev 1.36   22 Jun 1999 11:00:00   OS
   #66004# bullet and graphic properties not in chapter numbering

      Rev 1.35   10 May 1999 11:16:00   OS
   #66004# Zuordnung ChapterNumbering/NumberingRules berichtigt

      Rev 1.34   22 Apr 1999 16:13:54   OS
   #65194# throw -> throw; #65124# not impl. nur noch warning; EventListener

      Rev 1.33   25 Mar 1999 14:36:44   OS
   #63922# sal_Int32 statt sal_Int16 fuer Numerierungseinzuege

      Rev 1.32   15 Mar 1999 14:37:48   OS
   #62845# Makro fuer ServiceInfo jetzt auch fuer OS/2

      Rev 1.31   12 Mar 1999 09:41:26   OS
   #62845# lang::XServiceInfo impl.

      Rev 1.30   09 Mar 1999 12:41:26   OS
   #62008# Solar-Mutex

      Rev 1.29   08 Mar 1999 07:45:20   MH
   update 515

      Rev 1.28   05 Mar 1999 13:52:08   OS
   HoriOrientation - UPD515

      Rev 1.27   04 Mar 1999 15:03:08   OS
   #62191# UINT nicht mehr verwenden

      Rev 1.26   23 Feb 1999 10:22:12   OS
   #61767# Kapitelnumerierung funktioniert wieder

      Rev 1.25   22 Feb 1999 15:38:32   OS
   #60606# Numerierungsregel incl. Font richtig setzen;#62196 text::TextColumns: richtige Raender

      Rev 1.24   19 Feb 1999 16:54:54   OS
   #60606# Numerierungsregel setzbar

      Rev 1.23   15 Feb 1999 11:25:02   OS
   #52654# NumberingType in der NumberingRules

      Rev 1.22   28 Jan 1999 16:45:02   OS
   #56371# keine Objekte fuer DEBUG anlegen

      Rev 1.21   21 Jan 1999 09:57:04   OS
   #60971# Zeichenvorlagen der Numerierung an Absatzvorlagen setzen

      Rev 1.20   05 Jan 1999 12:11:10   OS
   #60606# #52654# Font fuer Numerierung setzen

      Rev 1.19   16 Dec 1998 14:28:20   OS
   #56371# Zwischenstand

      Rev 1.18   10 Dec 1998 15:53:34   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.17   17 Nov 1998 11:05:06   OS
   #58263# NumType durch SvxExtNumType ersetzt

      Rev 1.16   06 Nov 1998 13:11:24   OS
   #52654##59069# Spalten vollstaendig

      Rev 1.15   17 Sep 1998 16:01:08   OS
   52654# ParagraphStyle->ParaStyle, CharacterStyle->CharStyle

      Rev 1.14   10 Sep 1998 12:51:56   OS
   #52654# PROPERTY_NONE statt PROPERTY_BOUND

      Rev 1.13   10 Jul 1998 18:08:30   OS
   PropertySetInfo und IdlClass static

      Rev 1.12   27 Jun 1998 16:22:06   OS
   SwXNumberingRule ist Client

      Rev 1.11   18 Jun 1998 18:10:48   OS
   Twip-mm-Konvertierung

      Rev 1.10   18 Jun 1998 13:25:08   OS
   queryInterface im ChapterNumbering


      Rev 1.9   18 Jun 1998 08:53:56   MH
   Chg: Syntax OS2

      Rev 1.8   12 Jun 1998 13:49:14   OS
   Package-Umstellung

      Rev 1.7   04 Jun 1998 09:40:02   OS
   getIdlClasses


      Rev 1.6   29 May 1998 13:49:08   OS
   Spalten eingebaut

      Rev 1.5   26 May 1998 12:34:00   OS
   SwXNumberingRules verbessert

      Rev 1.4   25 May 1998 09:57:12   OS
   +SwXNumberingRules, SwXChapterNumbering hierher verschoben

      Rev 1.3   13 May 1998 15:27:02   OS
   Umstellung der text::TextDocument-Interfaces

      Rev 1.2   07 May 1998 14:56:18   MIB
   Header

      Rev 1.1   05 May 1998 10:04:00   OS
   Properties vollstaendig

      Rev 1.0   04 May 1998 13:41:04   OS
   Initial revision.

------------------------------------------------------------------------*/

