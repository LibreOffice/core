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
#include "precompiled_sw.hxx"

#include <svx/svxids.hrc>
#include <editeng/memberids.hrc>
#include <swtypes.hxx>
#include <cmdid.h>
#include <hintids.hxx>
#include "poolfmt.hrc"
#include "poolfmt.hxx"
#include <fmtcol.hxx>
#include <unomap.hxx>
#include <unostyle.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <ftninfo.hxx>
#include <doc.hxx>
#include <pagedesc.hxx>
#include <charfmt.hxx>
#include <lineinfo.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <docstyle.hxx>
#include <fmtclds.hxx>
#include <editeng/brshitem.hxx>
#include <com/sun/star/text/XFootnotesSettingsSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSettingsSupplier.hpp>
#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <vcl/font.hxx>
#include <editeng/flstitem.hxx>
#include <vcl/metric.hxx>
#include <svtools/ctrltool.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <editeng/unofdesc.hxx>
#include <fmtornt.hxx>
#include <SwStyleNameMapper.hxx>
// --> OD 2008-01-15 #newlistlevelattrs#
#include <com/sun/star/text/PositionAndSpaceMode.hpp>
#include <com/sun/star/text/LabelFollow.hpp>
// <--
#include <numrule.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;

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
SV_DECL_PTRARR(PropValDataArr, PropValDataPtr, 5, 5 )
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

const SfxItemPropertySet* GetFootnoteSet()
{
    static SfxItemPropertyMapEntry aFootnoteMap_Impl[] =
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
        {0,0,0,0,0,0}
    };
    static SfxItemPropertySet aFootnoteSet_Impl(aFootnoteMap_Impl);
    return &aFootnoteSet_Impl;
}
const SfxItemPropertySet* GetEndnoteSet()
{
    static SfxItemPropertyMapEntry aEndnoteMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_ANCHOR_CHAR_STYLE_NAME),WID_ANCHOR_CHARACTER_STYLE, &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),       WID_CHARACTER_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),        WID_NUMBERING_TYPE,     &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,         0},
        { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_NAME),       WID_PAGE_STYLE,         &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_PARA_STYLE_NAME),       WID_PARAGRAPH_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_PREFIX),                WID_PREFIX,     &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_START_AT),              WID_START_AT ,          &::getCppuType((const sal_Int16*)0), PROPERTY_NONE,         0},
        { SW_PROP_NAME(UNO_NAME_SUFFIX),                WID_SUFFIX,     &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        {0,0,0,0,0,0}
    };
    static SfxItemPropertySet aEndnoteSet_Impl(aEndnoteMap_Impl);
    return &aEndnoteSet_Impl;
}
const SfxItemPropertySet* GetNumberingRulesSet()
{
    static SfxItemPropertyMapEntry aNumberingRulesMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_IS_ABSOLUTE_MARGINS),       WID_IS_ABS_MARGINS, &::getBooleanCppuType(),            PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC),              WID_IS_AUTOMATIC,   &::getBooleanCppuType(),            PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_IS_CONTINUOUS_NUMBERING),   WID_CONTINUOUS,     &::getBooleanCppuType(),            PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NAME),                      WID_RULE_NAME   ,   &::getCppuType((const OUString*)0), PropertyAttribute::READONLY,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_IS_OUTLINE),      WID_IS_OUTLINE, &::getBooleanCppuType(),            PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_DEFAULT_LIST_ID),           WID_DEFAULT_LIST_ID, &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0},
        {0,0,0,0,0,0}
    };
    static SfxItemPropertySet  aNumberingRulesSet_Impl( aNumberingRulesMap_Impl );
    return &aNumberingRulesSet_Impl;
}
#define WID_NUM_ON                      0
#define WID_SEPARATOR_INTERVAL          1
#define WID_NUMBERING_TYPE              2
#define WID_NUMBER_POSITION             3
#define WID_DISTANCE                    4
#define WID_INTERVAL                    5
#define WID_SEPARATOR_TEXT              6
//#define WID_CHARACTER_STYLE             7
#define WID_COUNT_EMPTY_LINES           8
#define WID_COUNT_LINES_IN_FRAMES       9
#define WID_RESTART_AT_EACH_PAGE        10

const SfxItemPropertySet* GetLineNumberingSet()
{
    static SfxItemPropertyMapEntry aLineNumberingMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),         WID_CHARACTER_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_COUNT_EMPTY_LINES),       WID_COUNT_EMPTY_LINES , &::getBooleanCppuType(),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_COUNT_LINES_IN_FRAMES),   WID_COUNT_LINES_IN_FRAMES, &::getBooleanCppuType(),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_DISTANCE       ),         WID_DISTANCE       ,    &::getCppuType((const sal_Int32*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_IS_ON),                     WID_NUM_ON,             &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_INTERVAL  ),              WID_INTERVAL  ,       &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_SEPARATOR_TEXT ),         WID_SEPARATOR_TEXT,   &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBER_POSITION),         WID_NUMBER_POSITION,    &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),          WID_NUMBERING_TYPE ,    &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_RESTART_AT_EACH_PAGE),    WID_RESTART_AT_EACH_PAGE, &::getBooleanCppuType()  ,          PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_SEPARATOR_INTERVAL),      WID_SEPARATOR_INTERVAL, &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        {0,0,0,0,0,0}
    };
    static SfxItemPropertySet aLineNumberingSet_Impl(aLineNumberingMap_Impl);
    return &aLineNumberingSet_Impl;
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
    String sCharFmt;
    SwStyleNameMapper::FillUIName(uTmp, sCharFmt, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True);
    if(sStandard != sCharFmt)
    {
        pRet = pDoc->FindCharFmtByName( sCharFmt );
    }
    if(!pRet)
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(sCharFmt, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
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
    String sParaStyle;
    SwStyleNameMapper::FillUIName(uTmp, sParaStyle, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );
    SwTxtFmtColl* pRet = pDoc->FindTxtFmtCollByName( sParaStyle );
    if( !pRet  )
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sParaStyle, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
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
    String sPageDesc;
    SwStyleNameMapper::FillUIName(uTmp, sPageDesc, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, sal_True );
    for( sal_uInt16 i = 0; i < nCount; i++)
    {
        const SwPageDesc& rDesc = const_cast<const SwDoc *>(pDoc)
            ->GetPageDesc( i );
        if(rDesc.GetName() == sPageDesc)
        {
            pRet = (SwPageDesc*)&rDesc;
            break;
        }
    }
    if(!pRet)
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(sPageDesc, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC);
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
sal_Bool SwXFootnoteProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.FootnoteSettings") == rServiceName;
}
/* -----------------------------06.04.00 11:43--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXFootnoteProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.FootnoteSettings");
    return aRet;
}
/*-- 14.12.98 14:03:20---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFootnoteProperties::SwXFootnoteProperties(SwDoc* pDc) :
    pDoc(pDc),
    m_pPropertySet(GetFootnoteSet())
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
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
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
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap()->getByName( rPropertyName );
        if(pEntry)
        {
            if ( pEntry->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            SwFtnInfo aFtnInfo(pDoc->GetFtnInfo());
            switch(pEntry->nWID)
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
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    if(nTmp >= 0 &&
                        (nTmp <= SVX_NUM_ARABIC ||
                            nTmp > SVX_NUM_BITMAP))
                        aFtnInfo.aFmt.SetNumberingType(nTmp);
                    else
                        throw lang::IllegalArgumentException();
                }
                break;
                case  WID_START_AT:
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    aFtnInfo.nFtnOffset = nTmp;
                }
                break;
                case  WID_FOOTNOTE_COUNTING  :
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    switch(nTmp)
                    {
                        case  FootnoteNumbering::PER_PAGE:
                            aFtnInfo.eNum = FTNNUM_PAGE;
                        break;
                        case  FootnoteNumbering::PER_CHAPTER:
                            aFtnInfo.eNum = FTNNUM_CHAPTER;
                        break;
                        case  FootnoteNumbering::PER_DOCUMENT:
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
                        if(pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE)
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
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap()->getByName( rPropertyName );
        if(pEntry)
        {
            const SwFtnInfo& rFtnInfo = pDoc->GetFtnInfo();
            switch(pEntry->nWID)
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
                    aRet <<= rFtnInfo.aFmt.GetNumberingType();
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
                            nRet = FootnoteNumbering::PER_PAGE;
                        break;
                        case  FTNNUM_CHAPTER:
                            nRet = FootnoteNumbering::PER_CHAPTER;
                        break;
                        case  FTNNUM_DOC:
                            nRet = FootnoteNumbering::PER_DOCUMENT;
                        break;
                    }
                    aRet <<= nRet;
                }
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTxtFmtColl* pColl = rFtnInfo.GetFtnTxtColl();
                    String aString;
                    if(pColl)
                        aString = String ( pColl->GetName() );
                    SwStyleNameMapper::FillProgName(aString, aString, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True);
                    aRet <<= OUString ( aString );
                }
                break;
                case  WID_PAGE_STYLE :
                {
                    String aString;
                    if( rFtnInfo.KnowsPageDesc() )
                    {
                        SwStyleNameMapper::FillProgName(
                                rFtnInfo.GetPageDesc( *pDoc )->GetName(),
                                aString,
                                nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC,
                                sal_True);
                    }
                    aRet <<= OUString ( aString );
                }
                break;
                case WID_ANCHOR_CHARACTER_STYLE:
                case WID_CHARACTER_STYLE:
                {
                    String aString;
                    const SwCharFmt* pCharFmt = 0;
                    if( pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE )
                    {
                        if( rFtnInfo.GetAnchorCharFmtDep()->GetRegisteredIn() )
                            pCharFmt = rFtnInfo.GetAnchorCharFmt(*pDoc);
                    }
                    else
                    {
                        if( rFtnInfo.GetCharFmtDep()->GetRegisteredIn() )
                            pCharFmt = rFtnInfo.GetCharFmt(*pDoc);
                    }
                    if( pCharFmt )
                    {
                        SwStyleNameMapper::FillProgName(
                                pCharFmt->GetName(),
                                aString,
                                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT,
                                sal_True);
                    }
                    aRet <<= OUString ( aString );
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
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 14.12.98 14:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::addPropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::removePropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::addVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:03:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFootnoteProperties::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
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
sal_Bool SwXEndnoteProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.FootnoteSettings") == rServiceName;
}
/* -----------------------------06.04.00 11:45--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXEndnoteProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.FootnoteSettings");
    return aRet;
}
/*-- 14.12.98 14:27:39---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXEndnoteProperties::SwXEndnoteProperties(SwDoc* pDc) :
    pDoc(pDc),
    m_pPropertySet(GetEndnoteSet())
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
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
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
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap()->getByName( rPropertyName );
        if(pEntry)
        {
            if ( pEntry->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            SwEndNoteInfo aEndInfo(pDoc->GetEndNoteInfo());
            switch(pEntry->nWID)
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
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    aEndInfo.aFmt.SetNumberingType(nTmp);
                }
                break;
                case  WID_START_AT:
                {
                    sal_Int16 nTmp = 0;
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
                case WID_ANCHOR_CHARACTER_STYLE:
                case  WID_CHARACTER_STYLE    :
                {
                    SwCharFmt* pFmt = lcl_getCharFmt(pDoc, aValue);
                    if(pFmt)
                    {
                        if(pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE)
                            aEndInfo.SetAnchorCharFmt(pFmt);
                        else
                            aEndInfo.SetCharFmt(pFmt);
                    }
                }
                break;
            }
            pDoc->SetEndNoteInfo(aEndInfo);
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap()->getByName( rPropertyName );
        if(pEntry)
        {
            const SwEndNoteInfo& rEndInfo = pDoc->GetEndNoteInfo();
            switch(pEntry->nWID)
            {
                case WID_PREFIX:
                    aRet <<= OUString(rEndInfo.GetPrefix());
                break;
                case WID_SUFFIX:
                    aRet <<= OUString(rEndInfo.GetSuffix());
                break;
                case  WID_NUMBERING_TYPE :
                    aRet <<= rEndInfo.aFmt.GetNumberingType();
                break;
                case  WID_START_AT:
                    aRet <<= (sal_Int16)rEndInfo.nFtnOffset;
                break;
                case  WID_PARAGRAPH_STYLE    :
                {
                    SwTxtFmtColl* pColl = rEndInfo.GetFtnTxtColl();
                    String aString;
                    if(pColl)
                        aString = pColl->GetName();
                    SwStyleNameMapper::FillProgName(
                            aString,
                            aString,
                            nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL,
                            sal_True);
                    aRet <<= OUString ( aString );

                }
                break;
                case  WID_PAGE_STYLE :
                {
                    String aString;
                    if( rEndInfo.KnowsPageDesc() )
                    {
                        SwStyleNameMapper::FillProgName(
                            rEndInfo.GetPageDesc( *pDoc )->GetName(),
                            aString,
                            nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC,
                            sal_True );
                    }
                    aRet <<= OUString ( aString );
                }
                break;
                case WID_ANCHOR_CHARACTER_STYLE:
                case WID_CHARACTER_STYLE:
                {
                    String aString;
                    const SwCharFmt* pCharFmt = 0;
                    if( pEntry->nWID == WID_ANCHOR_CHARACTER_STYLE )
                    {
                        if( rEndInfo.GetAnchorCharFmtDep()->GetRegisteredIn() )
                            pCharFmt = rEndInfo.GetAnchorCharFmt(*pDoc);
                    }
                    else
                    {
                        if( rEndInfo.GetCharFmtDep()->GetRegisteredIn() )
                            pCharFmt = rEndInfo.GetCharFmt(*pDoc);
                    }
                    if( pCharFmt )
                    {
                        SwStyleNameMapper::FillProgName(
                                pCharFmt->GetName(),
                                aString,
                                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT,
                                sal_True );
                    }
                    aRet <<= OUString ( aString );
                }
                break;
            }
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}
/*-- 14.12.98 14:27:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:27:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::removePropertyChangeListener(const OUString& /*PropertyName*/,
        const uno:: Reference< beans::XPropertyChangeListener > & /*xListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:27:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::addVetoableChangeListener(const OUString& /*PropertyName*/,
    const uno:: Reference< beans::XVetoableChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:27:42---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXEndnoteProperties::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno:: Reference< beans::XVetoableChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
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
sal_Bool SwXLineNumberingProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.LineNumberingProperties") == rServiceName;
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXLineNumberingProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.LineNumberingProperties");
    return aRet;
}
/*-- 14.12.98 14:33:36---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXLineNumberingProperties::SwXLineNumberingProperties(SwDoc* pDc) :
    pDoc(pDc),
    m_pPropertySet(GetLineNumberingSet())
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
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
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
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap()->getByName( rPropertyName );
        if(pEntry)
        {
            if ( pEntry->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            SwLineNumberInfo  aInfo(pDoc->GetLineNumberInfo());
            switch(pEntry->nWID)
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
                    SvxNumberType aNumType(aInfo.GetNumType());
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    aNumType.SetNumberingType(nTmp);
                    aInfo.SetNumType(aNumType);
                }
                break;
                case WID_NUMBER_POSITION :
                {
                    sal_Int16 nTmp = 0;
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
                    sal_Int32 nVal = 0;
                    aValue >>= nVal;
                    sal_Int32 nTmp = MM100_TO_TWIP(nVal);
                    if (nTmp > USHRT_MAX)
                        nTmp = USHRT_MAX;
                    aInfo.SetPosFromLeft( static_cast< sal_uInt16 >(nTmp) );
                }
                break;
                case WID_INTERVAL   :
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    if( nTmp > 0)
                        aInfo.SetCountBy(nTmp);
                }
                break;
                case WID_SEPARATOR_TEXT  :
                {
                    OUString uTmp;
                    aValue >>= uTmp;
                    aInfo.SetDivider(uTmp);
                }
                break;
                case WID_SEPARATOR_INTERVAL:
                {
                    sal_Int16 nTmp = 0;
                    aValue >>= nTmp;
                    if( nTmp >= 0)
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
                case WID_RESTART_AT_EACH_PAGE :
                {
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
                    aInfo.SetRestartEachPage(bVal);
                }
                break;
            }
            pDoc->SetLineNumberInfo(aInfo);
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropertySet->getPropertyMap()->getByName( rPropertyName );
        if(pEntry)
        {
            const SwLineNumberInfo& rInfo = pDoc->GetLineNumberInfo();
            switch(pEntry->nWID)
            {
                case WID_NUM_ON:
                {
                    sal_Bool bTemp = rInfo.IsPaintLineNumbers();
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case WID_CHARACTER_STYLE :
                {
                    String aString;
                    // return empty string if no char format is set
                    // otherwise it would be created here
                    if(rInfo.HasCharFormat())
                    {
                        SwStyleNameMapper::FillProgName(
                                    rInfo.GetCharFmt(*pDoc)->GetName(),
                                    aString,
                                    nsSwGetPoolIdFromName::GET_POOLID_CHRFMT,
                                    sal_True);
                    }
                    aRet <<= OUString ( aString );
                }
                break;
                case WID_NUMBERING_TYPE  :
                    aRet <<= rInfo.GetNumType().GetNumberingType();
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
                    aRet <<= static_cast < sal_Int32 >(TWIP_TO_MM100_UNSIGNED(nPos));
                }
                break;
                case WID_INTERVAL   :
                    aRet <<= (sal_Int16)rInfo.GetCountBy();
                break;
                case WID_SEPARATOR_TEXT  :
                    aRet <<= OUString(rInfo.GetDivider());
                break;
                case WID_SEPARATOR_INTERVAL:
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
                case WID_RESTART_AT_EACH_PAGE :
                {
                    sal_Bool bTemp = rInfo.IsRestartEachPage();
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
            }
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 14.12.98 14:33:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno:: Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:33:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno:: Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:33:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno:: Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented");
}
/*-- 14.12.98 14:33:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXLineNumberingProperties::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno:: Reference< beans::XVetoableChangeListener > & /*xListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented");
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
sal_Int64 SwXNumberingRules::getSomething( const uno::Sequence< sal_Int8 > & rId ) throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
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
sal_Bool SwXNumberingRules::supportsService(const OUString& rServiceName) throw( RuntimeException )
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

/*-- 14.12.98 14:57:57---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXNumberingRules::SwXNumberingRules(const SwNumRule& rRule) :
    pDoc(0),
    pDocShell(0),
    pNumRule(new SwNumRule(rRule)),
    m_pPropertySet(GetNumberingRulesSet()),
    bOwnNumRuleCreated(sal_True)
{
    sal_uInt16 i;

    //erstmal das Doc organisieren; es haengt an den gesetzten Zeichenvorlagen - wenn
    // keine gesetzt sind, muss es auch ohne gehen
    for( i = 0; i < MAXLEVEL; i++)
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
    pDocShell(&rDocSh),
    pNumRule(0),
    m_pPropertySet(GetNumberingRulesSet()),
    bOwnNumRuleCreated(sal_False)
{
    pDocShell->GetDoc()->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}
/* -----------------------------24.08.00 11:36--------------------------------

 ---------------------------------------------------------------------------*/
SwXNumberingRules::SwXNumberingRules(SwDoc& rDoc) :
    pDoc(&rDoc),
    pDocShell(0),
    pNumRule(0),
    m_pPropertySet(GetNumberingRulesSet()),
    bOwnNumRuleCreated(sal_False)
{
    rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    sCreatedNumRuleName = rDoc.GetUniqueNumRuleName();
#if OSL_DEBUG_LEVEL > 1
    sal_uInt16 nIndex =
#endif
    // --> OD 2008-02-11 #newlistlevelattrs#
    rDoc.MakeNumRule( sCreatedNumRuleName, 0, sal_False,
                      // --> OD 2008-06-06 #i89178#
                      numfunc::GetDefaultPositionAndSpaceMode() );
                      // <--
    // <--
#if OSL_DEBUG_LEVEL > 1
    (void)nIndex;
#endif
}
/*-- 14.12.98 14:57:57---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXNumberingRules::~SwXNumberingRules()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(pDoc && sCreatedNumRuleName.Len())
        pDoc->DelNumRule( sCreatedNumRuleName );
    if( pNumRule && bOwnNumRuleCreated )
        delete pNumRule;
}
/*-- 14.12.98 14:57:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException,
                  lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(nIndex < 0 || MAXLEVEL <= nIndex)
        throw lang::IndexOutOfBoundsException();

    if(rElement.getValueType().getTypeClass() != uno::TypeClass_SEQUENCE)
        throw lang::IllegalArgumentException();
    const uno::Sequence<beans::PropertyValue>& rProperties =
                    *(const uno::Sequence<beans::PropertyValue>*)rElement.getValue();
    SwNumRule* pRule = 0;
    if(pNumRule)
        SwXNumberingRules::SetNumberingRuleByIndex( *pNumRule,
                            rProperties, nIndex);
    else if(pDocShell)
    {
        // --> OD 2008-04-21 #i87650# - correction of cws swwarnings:
        // Do not set member <pNumRule>
//        pNumRule = pDocShell->GetDoc()->GetOutlineNumRule();
//        SwNumRule aNumRule(*pNumRule);
        SwNumRule aNumRule( *(pDocShell->GetDoc()->GetOutlineNumRule()) );
        // <--
        SwXNumberingRules::SetNumberingRuleByIndex( aNumRule,
                            rProperties, nIndex);
        //hier noch die Zeichenformate bei Bedarf setzen
        const SwCharFmts* pFmts = pDocShell->GetDoc()->GetCharFmts();
        sal_uInt16 nChCount = pFmts->Count();
        for(sal_uInt16 i = 0; i < MAXLEVEL;i++)
        {
            SwNumFmt aFmt(aNumRule.Get( i ));
            if(sNewCharStyleNames[i].Len() &&
                !sNewCharStyleNames[i].EqualsAscii(SW_PROP_NAME_STR(UNO_NAME_CHARACTER_FORMAT_NONE)) &&
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
    else if(!pNumRule && pDoc && sCreatedNumRuleName.Len() &&
        0 != (pRule = pDoc->FindNumRulePtr( sCreatedNumRuleName )))
    {
        SwXNumberingRules::SetNumberingRuleByIndex( *pRule,
                            rProperties, nIndex);

        pRule->Validate();
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
    if(nIndex < 0 || MAXLEVEL <= nIndex)
        throw lang::IndexOutOfBoundsException();

    uno::Any aVal;
    const SwNumRule* pRule = pNumRule;
    if(!pRule && pDoc && sCreatedNumRuleName.Len())
        pRule = pDoc->FindNumRulePtr( sCreatedNumRuleName );
    if(pRule)
    {
        uno::Sequence<beans::PropertyValue> aRet = GetNumberingRuleByIndex(
                                        *pRule, nIndex);
        aVal.setValue(&aRet, ::getCppuType((uno::Sequence<beans::PropertyValue>*)0));

    }
    else if(pDocShell)
    {
        uno::Sequence<beans::PropertyValue> aRet = GetNumberingRuleByIndex(
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
uno::Sequence<beans::PropertyValue> SwXNumberingRules::GetNumberingRuleByIndex(
                const SwNumRule& rNumRule, sal_Int32 nIndex) const
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_ASSERT( 0 <= nIndex && nIndex < MAXLEVEL, "index out of range" );

    const SwNumFmt& rFmt = rNumRule.Get( (sal_uInt16)nIndex );

    sal_Bool bChapterNum = pDocShell != 0;

    PropValDataArr  aPropertyValues;
    //fill all properties into the array

    //adjust
    SvxAdjust eAdj = rFmt.GetNumAdjust();
    sal_Int16 nINT16 = aSvxToUnoAdjust[(sal_uInt16)eAdj];
    PropValData* pData = new PropValData((void*)&nINT16, "Adjust", ::getCppuType((const sal_Int16*)0) );
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //parentnumbering
    nINT16 = rFmt.GetIncludeUpperLevels();
    pData = new PropValData((void*)&nINT16, "ParentNumbering", ::getCppuType((const sal_Int16*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //prefix
    OUString aUString = rFmt.GetPrefix();
    pData = new PropValData((void*)&aUString, "Prefix", ::getCppuType((const OUString*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //suffix
    aUString = rFmt.GetSuffix();
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

    String aString;
    SwStyleNameMapper::FillProgName( CharStyleName, aString, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True );
    aUString = aString;
    pData = new PropValData((void*)&aUString, "CharStyleName", ::getCppuType((const OUString*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //startvalue
    nINT16 = rFmt.GetStart();
    pData = new PropValData((void*)&nINT16, "StartWith", ::getCppuType((const sal_Int16*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    // --> OD 2008-01-23 #newlistlevelattrs#
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
    {
        //leftmargin
        sal_Int32 nINT32 = TWIP_TO_MM100(rFmt.GetAbsLSpace());
        pData = new PropValData((void*)&nINT32, SW_PROP_NAME_STR(UNO_NAME_LEFT_MARGIN), ::getCppuType((const sal_Int32*)0));
        aPropertyValues.Insert(pData, aPropertyValues.Count());

        //chartextoffset
        nINT32 = TWIP_TO_MM100(rFmt.GetCharTextDistance());
        pData = new PropValData((void*)&nINT32, SW_PROP_NAME_STR(UNO_NAME_SYMBOL_TEXT_DISTANCE), ::getCppuType((const sal_Int32*)0));
        aPropertyValues.Insert(pData, aPropertyValues.Count());

        //firstlineoffset
        nINT32 = TWIP_TO_MM100(rFmt.GetFirstLineOffset());
        pData = new PropValData((void*)&nINT32, SW_PROP_NAME_STR(UNO_NAME_FIRST_LINE_OFFSET), ::getCppuType((const sal_Int32*)0));
        aPropertyValues.Insert(pData, aPropertyValues.Count());
    }
    // <--

    // --> OD 2008-01-15 #newlistlevelattrs#
    // PositionAndSpaceMode
    nINT16 = PositionAndSpaceMode::LABEL_WIDTH_AND_POSITION;
    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        nINT16 = PositionAndSpaceMode::LABEL_ALIGNMENT;
    }
    pData = new PropValData( (void*)&nINT16,
                             SW_PROP_NAME_STR(UNO_NAME_POSITION_AND_SPACE_MODE),
                             ::getCppuType((const sal_Int16*)0) );
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
    {
        // LabelFollowedBy
        nINT16 = LabelFollow::LISTTAB;
        if ( rFmt.GetLabelFollowedBy() == SvxNumberFormat::SPACE )
        {
            nINT16 = LabelFollow::SPACE;
        }
        else if ( rFmt.GetLabelFollowedBy() == SvxNumberFormat::NOTHING )
        {
            nINT16 = LabelFollow::NOTHING;
        }
        pData = new PropValData( (void*)&nINT16,
                                 SW_PROP_NAME_STR(UNO_NAME_LABEL_FOLLOWED_BY),
                                 ::getCppuType((const sal_Int16*)0) );
        aPropertyValues.Insert(pData, aPropertyValues.Count());

        // ListtabStopPosition
        sal_Int32 nINT32 = TWIP_TO_MM100(rFmt.GetListtabPos());
        pData = new PropValData( (void*)&nINT32,
                                 SW_PROP_NAME_STR(UNO_NAME_LISTTAB_STOP_POSITION),
                                 ::getCppuType((const sal_Int32*)0));
        aPropertyValues.Insert(pData, aPropertyValues.Count());

        // FirstLineIndent
        nINT32 = TWIP_TO_MM100(rFmt.GetFirstLineIndent());
        pData = new PropValData( (void*)&nINT32,
                                 SW_PROP_NAME_STR(UNO_NAME_FIRST_LINE_INDENT),
                                 ::getCppuType((const sal_Int32*)0));
        aPropertyValues.Insert(pData, aPropertyValues.Count());

        // IndentAt
        nINT32 = TWIP_TO_MM100(rFmt.GetIndentAt());
        pData = new PropValData( (void*)&nINT32,
                                 SW_PROP_NAME_STR(UNO_NAME_INDENT_AT),
                                 ::getCppuType((const sal_Int32*)0));
        aPropertyValues.Insert(pData, aPropertyValues.Count());
    }
    // <--

    //numberingtype
    nINT16 = rFmt.GetNumberingType();
    pData = new PropValData((void*)&nINT16, "NumberingType", ::getCppuType((const sal_Int16*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    if(!bChapterNum)
    {
        if(SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType())
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
                pData = new PropValData((void*)&aDesc, SW_PROP_NAME_STR(UNO_NAME_BULLET_FONT), ::getCppuType((const awt::FontDescriptor*)0));
                aPropertyValues.Insert(pData, aPropertyValues.Count());
            }
        }
        if(SVX_NUM_BITMAP == rFmt.GetNumberingType())
        {
            //GraphicURL
            const SvxBrushItem* pBrush = rFmt.GetBrush();
            if(pBrush)
            {
                Any aAny;
                pBrush->QueryValue( aAny, MID_GRAPHIC_URL );
                aAny >>= aUString;
            }
            else
                aUString = aEmptyStr;
            pData = new PropValData((void*)&aUString, SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_URL), ::getCppuType((const OUString*)0));
            aPropertyValues.Insert(pData, aPropertyValues.Count());

            //graphicbitmap
            const Graphic* pGraphic = 0;
            if(pBrush )
                pGraphic = pBrush->GetGraphic();
            if(pGraphic)
            {
                uno::Reference<awt::XBitmap> xBmp = VCLUnoHelper::CreateBitmap( pGraphic->GetBitmapEx() );
                pData = new PropValData((void*)&xBmp, SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_BITMAP),
                                ::getCppuType((const uno::Reference<awt::XBitmap>*)0));
                aPropertyValues.Insert(pData, aPropertyValues.Count());
            }
             Size aSize = rFmt.GetGraphicSize();
            // --> OD 2010-05-04 #i101131# - applying patch from CMC
            // adjust conversion due to type mismatch between <Size> and <awt::Size>
//            aSize.Width() = TWIP_TO_MM100( aSize.Width() );
//            aSize.Height() = TWIP_TO_MM100( aSize.Height() );
//            pData = new PropValData((void*)&aSize, SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_SIZE), ::getCppuType((const awt::Size*)0));
            awt::Size aAwtSize(TWIP_TO_MM100(aSize.Width()), TWIP_TO_MM100(aSize.Height()));
            pData = new PropValData((void*)&aAwtSize, SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_SIZE), ::getCppuType((const awt::Size*)0));
            // <--
            aPropertyValues.Insert(pData, aPropertyValues.Count());

            const SwFmtVertOrient* pOrient = rFmt.GetGraphicOrientation();
            if(pOrient)
            {
                pData = new PropValData((void*)0, SW_PROP_NAME_STR(UNO_NAME_VERT_ORIENT), ::getCppuType((const sal_Int16*)0));
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

            //sal_Int8 nOutLevel = rTxtColl.GetOutlineLevel();      //#outline level,zhaojianwei
            const sal_Int16 nOutLevel = rTxtColl.IsAssignedToListLevelOfOutlineStyle()
                                        ? static_cast<sal_Int16>(rTxtColl.GetAssignedOutlineStyleLevel())
                                        : MAXLEVEL;                 //<-end,zhaojianwei
            if ( nOutLevel == nIndex )
            {
                sValue = rTxtColl.GetName();
                break; // the style for the level in question has been found
            }
            else if( sValue==rTxtColl.GetName() )
            {
                // if the default for the level is existing, but its
                // level is different, then it cannot be the default.
                sValue.Erase();
            }
        }
        String aName;
        SwStyleNameMapper::FillProgName(sValue, aName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True);
        aUString = aName;

        pData = new PropValData((void*)&aUString, SW_PROP_NAME_STR(UNO_NAME_HEADING_STYLE_NAME), ::getCppuType((const OUString*)0));
        aPropertyValues.Insert(pData, aPropertyValues.Count());
    }

    uno::Sequence<beans::PropertyValue> aSeq(aPropertyValues.Count());
    beans::PropertyValue* pArray = aSeq.getArray();

    for(sal_uInt16 i = 0; i < aPropertyValues.Count(); i++)
    {
        pData = aPropertyValues.GetObject(i);
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

void SwXNumberingRules::SetNumberingRuleByIndex(
            SwNumRule& rNumRule,
            const uno::Sequence<beans::PropertyValue>& rProperties, sal_Int32 nIndex)
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    DBG_ASSERT( 0 <= nIndex && nIndex < MAXLEVEL, "index out of range" );

    // the order of the names is important!
    static const char* aNumPropertyNames[] =
    {
        "Adjust",                               //0
        "ParentNumbering",                      //1
        "Prefix",                               //2
        "Suffix",                               //3
        "CharStyleName",                        //4
        "StartWith",                            //5
        SW_PROP_NAME_STR(UNO_NAME_LEFT_MARGIN),                   //6
        SW_PROP_NAME_STR(UNO_NAME_SYMBOL_TEXT_DISTANCE),          //7
        SW_PROP_NAME_STR(UNO_NAME_FIRST_LINE_OFFSET),             //8
        // --> OD 2008-01-15 #newlistlevelattrs#
        SW_PROP_NAME_STR(UNO_NAME_POSITION_AND_SPACE_MODE), //9
        SW_PROP_NAME_STR(UNO_NAME_LABEL_FOLLOWED_BY),       //10
        SW_PROP_NAME_STR(UNO_NAME_LISTTAB_STOP_POSITION),   //11
        SW_PROP_NAME_STR(UNO_NAME_FIRST_LINE_INDENT),       //12
        SW_PROP_NAME_STR(UNO_NAME_INDENT_AT),               //13
        // <--
        "NumberingType",                        //14
        "BulletId",                             //15
        SW_PROP_NAME_STR(UNO_NAME_BULLET_FONT), //16
        "BulletFontName",                       //17
        "BulletChar",                           //18
        SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_URL),    //19
        SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_BITMAP), //20
        SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_SIZE),   //21
        SW_PROP_NAME_STR(UNO_NAME_VERT_ORIENT),    //22
        SW_PROP_NAME_STR(UNO_NAME_HEADING_STYLE_NAME) //23
    };
    // --> OD 2008-01-15 #newlistlevelattrs#
    const sal_uInt16 nPropNameCount = 24;
    const sal_uInt16 nNotInChapter = 15;
    // <--

    const beans::PropertyValue* pPropArray = rProperties.getConstArray();
    PropValDataArr aPropertyValues;
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
        if(bExcept &&
            (rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("BulletRelSize")) ||
             rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("BulletColor")) ) )
        {
            bExcept = sal_False;
        }
        PropValData* pData = new PropValData(rProp.Value, rProp.Name );
        aPropertyValues.Insert(pData, aPropertyValues.Count());
    }

    SwNumFmt aFmt(rNumRule.Get( (sal_uInt16)nIndex ));
    sal_Bool bWrongArg = sal_False;
    if(!bExcept)
       {
        SvxBrushItem* pSetBrush = 0;
        Size* pSetSize = 0;
        SwFmtVertOrient* pSetVOrient = 0;
        sal_Bool bCharStyleNameSet = sal_False;

        for(sal_uInt16 i = 0; i < nPropNameCount && !bExcept && !bWrongArg; i++)
        {
            PropValData* pData = lcl_FindProperty(aNumPropertyNames[i], aPropertyValues);
            if(!pData)
                continue;
            switch(i)
            {
                case 0: //"Adjust"
                {
                    sal_Int16 nValue = 0;
                    pData->aVal >>= nValue;
                    if(nValue > 0 &&
                        nValue <= text::HoriOrientation::LEFT &&
                            USHRT_MAX != aUnoToSvxAdjust[nValue])
                    {
                        aFmt.SetNumAdjust((SvxAdjust)aUnoToSvxAdjust[nValue]);
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 1: //"ParentNumbering",
                {
                    sal_Int16 nSet = 0;
                    pData->aVal >>= nSet;
                    if(nSet >= 0 && MAXLEVEL >= nSet)
                        aFmt.SetIncludeUpperLevels( static_cast< sal_uInt8 >(nSet) );
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
                    aFmt.SetSuffix(uTmp);
                }
                break;
                case 4: //"CharStyleName",
                {
                    bCharStyleNameSet = sal_True;
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    String sCharFmtName;
                    SwStyleNameMapper::FillUIName( uTmp, sCharFmtName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, sal_True );
                    if(sCharFmtName.EqualsAscii(SW_PROP_NAME_STR(UNO_NAME_CHARACTER_FORMAT_NONE)))
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
                        // os 2005-08-22 #i51842#
                        // If the character format has been found it's name should not be in the
                        // char style names array
                        //sNewCharStyleNames[(sal_uInt16)nIndex] = sCharFmtName;
                        sNewCharStyleNames[(sal_uInt16)nIndex].Erase();
                     }
                    else
                        sNewCharStyleNames[(sal_uInt16)nIndex] = sCharFmtName;
                }
                break;
                case 5: //"StartWith",
                {
                    sal_Int16 nVal = 0;
                    pData->aVal >>= nVal;
                    aFmt.SetStart(nVal);
                }
                break;
                case 6: //UNO_NAME_LEFT_MARGIN,
                {
                    sal_Int32 nValue = 0;
                    pData->aVal >>= nValue;
                    // #i23727# nValue can be negative
                    aFmt.SetAbsLSpace((sal_uInt16) MM100_TO_TWIP(nValue));
                }
                break;
                case 7: //UNO_NAME_SYMBOL_TEXT_DISTANCE,
                {
                    sal_Int32 nValue = 0;
                    pData->aVal >>= nValue;
                    if(nValue >= 0)
                        aFmt.SetCharTextDistance((sal_uInt16) MM100_TO_TWIP(nValue));
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 8: //UNO_NAME_FIRST_LINE_OFFSET,
                {
                    sal_Int32 nValue = 0;
                    pData->aVal >>= nValue;
                    // #i23727# nValue can be positive
                    nValue = MM100_TO_TWIP(nValue);
                    aFmt.SetFirstLineOffset((short)nValue);
                }
                break;
                // --> OD 2008-01-15 #newlistlevelattrs#
                case 9: // UNO_NAME_POSITION_AND_SPACE_MODE
                {
                    sal_Int16 nValue = 0;
                    pData->aVal >>= nValue;
                    if ( nValue == 0 )
                    {
                        aFmt.SetPositionAndSpaceMode( SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
                    }
                    else if ( nValue == 1 )
                    {
                        aFmt.SetPositionAndSpaceMode( SvxNumberFormat::LABEL_ALIGNMENT );
                    }
                    else
                    {
                        bWrongArg = sal_True;
                    }
                }
                break;
                case 10: // UNO_NAME_LABEL_FOLLOWED_BY
                {
                    sal_Int16 nValue = 0;
                    pData->aVal >>= nValue;
                    if ( nValue == 0 )
                    {
                        aFmt.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                    }
                    else if ( nValue == 1 )
                    {
                        aFmt.SetLabelFollowedBy( SvxNumberFormat::SPACE );
                    }
                    else if ( nValue == 2 )
                    {
                        aFmt.SetLabelFollowedBy( SvxNumberFormat::NOTHING );
                    }
                    else
                    {
                        bWrongArg = sal_True;
                    }
                }
                break;
                case 11: // UNO_NAME_LISTTAB_STOP_POSITION
                {
                    sal_Int32 nValue = 0;
                    pData->aVal >>= nValue;
                    nValue = MM100_TO_TWIP(nValue);
                    if ( nValue >= 0 )
                    {
                        aFmt.SetListtabPos( nValue );
                    }
                    else
                    {
                        bWrongArg = sal_True;
                    }
                }
                break;
                case 12: // UNO_NAME_FIRST_LINE_INDENT
                {
                    sal_Int32 nValue = 0;
                    pData->aVal >>= nValue;
                    nValue = MM100_TO_TWIP(nValue);
                    aFmt.SetFirstLineIndent( nValue );
                }
                break;
                case 13: // UNO_NAME_INDENT_AT
                {
                    sal_Int32 nValue = 0;
                    pData->aVal >>= nValue;
                    nValue = MM100_TO_TWIP(nValue);
                    aFmt.SetIndentAt( nValue );
                }
                break;
                // <--
                case 14: //"NumberingType"
                {
                    sal_Int16 nSet = 0;
                    pData->aVal >>= nSet;
                    if(nSet >= 0)
                        aFmt.SetNumberingType(nSet);
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 15: //"BulletId",
                {
                    sal_Int16 nSet = 0;
                    if( pData->aVal >>= nSet )
                        aFmt.SetBulletChar(nSet);
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 16: //UNO_NAME_BULLET_FONT,
                {
                     awt::FontDescriptor* pDesc =  (awt::FontDescriptor*)pData->aVal.getValue();
                    if(pDesc)
                    {
                        // --> OD 2008-09-11 #i93725#
                        // do not accept "empty" font
                        if ( pDesc->Name.getLength() > 0 )
                        {
                            Font aFont;
                            SvxUnoFontDescriptor::ConvertToFont( *pDesc, aFont );
                            aFmt.SetBulletFont(&aFont);
                        }
                        // <--
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 17: //"BulletFontName",
                {
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    String sBulletFontName(uTmp);
                    SwDocShell* pLclDocShell = pDocShell ? pDocShell : pDoc ? pDoc->GetDocShell() : 0;
                    if( sBulletFontName.Len() && pLclDocShell )
                    {
                        const SvxFontListItem* pFontListItem =
                                (const SvxFontListItem* )pLclDocShell
                                                    ->GetItem( SID_ATTR_CHAR_FONTLIST );
                        const FontList*  pList = pFontListItem->GetFontList();
                        FontInfo aInfo = pList->Get(
                            sBulletFontName, WEIGHT_NORMAL, ITALIC_NONE);
                        Font aFont(aInfo);
                        aFmt.SetBulletFont(&aFont);
                    }
                    else
                        sNewBulletFontNames[(sal_uInt16)nIndex] = sBulletFontName;
                }
                break;
                case 18: //"BulletChar",
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
                case 19: //UNO_NAME_GRAPHIC_URL,
                {
                    OUString sBrushURL;
                    pData->aVal >>= sBrushURL;
                    if(!pSetBrush)
                    {
                        const SvxBrushItem* pOrigBrush = aFmt.GetBrush();
                        if(pOrigBrush)
                        {
                            pSetBrush = new SvxBrushItem(*pOrigBrush);
                        }
                        else
                            pSetBrush = new SvxBrushItem(aEmptyStr, aEmptyStr, GPOS_AREA, RES_BACKGROUND);
                    }
                    pSetBrush->PutValue( pData->aVal, MID_GRAPHIC_URL );
                }
                break;
                case 20: //UNO_NAME_GRAPHIC_BITMAP,
                {
                    uno::Reference< awt::XBitmap >* pBitmap = (uno::Reference< awt::XBitmap > *)pData->aVal.getValue();
                    if(pBitmap)
                    {
                        if(!pSetBrush)
                        {
                            const SvxBrushItem* pOrigBrush = aFmt.GetBrush();
                            if(pOrigBrush)
                            {
                                pSetBrush = new SvxBrushItem(*pOrigBrush);
                            }
                            else
                                pSetBrush = new SvxBrushItem(aEmptyStr, aEmptyStr, GPOS_AREA, RES_BACKGROUND);
                        }

                        BitmapEx aBmp = VCLUnoHelper::GetBitmap( *pBitmap );
                        Graphic aNewGr(aBmp);
                        pSetBrush->SetGraphic( aNewGr );
                    }
                    else
                        bWrongArg = sal_True;
                }
                break;
                case 21: //UNO_NAME_GRAPHIC_SIZE,
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
                case 22: //VertOrient
                {
                    if(!pSetVOrient)
                    {
                        if(aFmt.GetGraphicOrientation())
                            pSetVOrient = (SwFmtVertOrient*)aFmt.GetGraphicOrientation()->Clone();
                        else
                            pSetVOrient = new SwFmtVertOrient;
                    }
                    ((SfxPoolItem*)pSetVOrient)->PutValue(pData->aVal, MID_VERTORIENT_ORIENT);
                }
                break;
                case 23: //"HeadingStyleName"
                {
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    String sStyleName;
                    SwStyleNameMapper::FillUIName(uTmp, sStyleName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, sal_True );
                    const SwTxtFmtColls* pColls = pDocShell->GetDoc()->GetTxtFmtColls();
                    const sal_uInt16 nCount = pColls->Count();
                    for(sal_uInt16 k = 0; k < nCount; ++k)
                    {
                        SwTxtFmtColl &rTxtColl = *((*pColls)[k]);
                        if(rTxtColl.IsDefault())
                            continue;
                        //if(rTxtColl.GetOutlineLevel() == nIndex &&            //#outline level,removed by zhaojianwei
                        //  rTxtColl.GetName() != sStyleName)
                        //  rTxtColl..SetOutlineLevel(NO_NUMBERING);
                        //else if(rTxtColl.GetName() == sStyleName)
                        //  rTxtColl.SetOutlineLevel(sal_Int8(nIndex));
                        if ( rTxtColl.IsAssignedToListLevelOfOutlineStyle() &&  //add by zhaojianwei
                             rTxtColl.GetAssignedOutlineStyleLevel() == nIndex &&
                             rTxtColl.GetName() != sStyleName )
                        {
                            rTxtColl.DeleteAssignmentToListLevelOfOutlineStyle();
                        }
                        else if ( rTxtColl.GetName() == sStyleName )
                        {
                            rTxtColl.AssignToListLevelOfOutlineStyle( nIndex );
                        }                                                       //<-end,,zhaojianwei,
                    }
                }
                break;
                case 24: // BulletRelSize - unsupported - only available in Impress
                break;
            }
        }
        if(!bExcept && !bWrongArg && (pSetBrush || pSetSize || pSetVOrient))
        {
            if(!pSetBrush && aFmt.GetBrush())
                pSetBrush = new SvxBrushItem(*aFmt.GetBrush());

            if(pSetBrush)
            {
                if(!pSetVOrient && aFmt.GetGraphicOrientation())
                    pSetVOrient = new SwFmtVertOrient(*aFmt.GetGraphicOrientation());

                if(!pSetSize)
                {
                    pSetSize = new Size(aFmt.GetGraphicSize());
                    if(!pSetSize->Width() || !pSetSize->Height())
                    {
                        const Graphic* pGraphic = pSetBrush->GetGraphic();
                        if(pGraphic)
                            *pSetSize = ::GetGraphicSizeTwip(*pGraphic, 0);
                    }
                }
                sal_Int16 eOrient = pSetVOrient ?
                    (sal_Int16)pSetVOrient->GetVertOrient() : text::VertOrientation::NONE;
                aFmt.SetGraphicBrush( pSetBrush, pSetSize, text::VertOrientation::NONE == eOrient ? 0 : &eOrient );
            }
        }
        if((!bCharStyleNameSet || !sNewCharStyleNames[(sal_uInt16)nIndex].Len()) &&
                aFmt.GetNumberingType() == NumberingType::BITMAP && !aFmt.GetCharFmt()
                    && SwXNumberingRules::GetInvalidStyle() != sNewCharStyleNames[(sal_uInt16)nIndex])
        {
            SwStyleNameMapper::FillProgName ( RES_POOLCHR_BUL_LEVEL, sNewCharStyleNames[(sal_uInt16)nIndex] );
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
uno::Reference< XPropertySetInfo > SwXNumberingRules::getPropertySetInfo()
    throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropertySet->getPropertySetInfo();
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
    SwNumRule* pCreatedRule = 0;
    if(!pNumRule)
    {
        if(!pNumRule && pDocShell)
        {
            pDocRule = new SwNumRule(*pDocShell->GetDoc()->GetOutlineNumRule());
        }
        else if(pDoc && sCreatedNumRuleName.Len())
        {
            pCreatedRule = pDoc->FindNumRulePtr( sCreatedNumRuleName);
        }

    }
    if(!pNumRule && !pDocRule && !pCreatedRule)
        throw RuntimeException();


    if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC)))
    {
        sal_Bool bVal = *(sal_Bool*)rValue.getValue();
        if(!pCreatedRule)
            pDocRule ? pDocRule->SetAutoRule(bVal) : pNumRule->SetAutoRule(bVal);
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_CONTINUOUS_NUMBERING)))
    {
        sal_Bool bVal = *(sal_Bool*)rValue.getValue();
        pDocRule ? pDocRule->SetContinusNum(bVal) :
            pCreatedRule ? pCreatedRule->SetContinusNum(bVal) : pNumRule->SetContinusNum(bVal);
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NAME)))
    {
        delete pDocRule;
        throw IllegalArgumentException();
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_ABSOLUTE_MARGINS)))
    {
        sal_Bool bVal = *(sal_Bool*)rValue.getValue();
        pDocRule ? pDocRule->SetAbsSpaces(bVal) :
            pCreatedRule ? pCreatedRule->SetAbsSpaces(bVal) : pNumRule->SetAbsSpaces(bVal);
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NUMBERING_IS_OUTLINE)))
    {
        sal_Bool bVal = *(sal_Bool*)rValue.getValue();
        SwNumRuleType eNumRuleType = bVal ? OUTLINE_RULE : NUM_RULE;
        pDocRule ? pDocRule->SetRuleType(eNumRuleType) :
            pCreatedRule ? pCreatedRule->SetRuleType(eNumRuleType) : pNumRule->SetRuleType(eNumRuleType);
    }
    // --> OD 2008-04-23 #refactorlists#
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DEFAULT_LIST_ID)))
    {
        delete pDocRule;
        throw IllegalArgumentException();
    }
    // <--
    else
        throw UnknownPropertyException();

    if(pDocRule)
    {
        pDocShell->GetDoc()->SetOutlineNumRule(*pDocRule);
        delete pDocRule;
    }
    else if(pCreatedRule)
    {
        pCreatedRule->Validate();
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
    else if(pDoc && sCreatedNumRuleName.Len())
        pRule = pDoc->FindNumRulePtr( sCreatedNumRuleName );
    if(!pRule)
        throw RuntimeException();

    if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC)))
    {
        sal_Bool bVal = pRule->IsAutoRule();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_CONTINUOUS_NUMBERING)))
    {
        sal_Bool bVal = pRule->IsContinusNum();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NAME)))
        aRet <<= OUString(pRule->GetName());
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_ABSOLUTE_MARGINS)))
    {
        sal_Bool bVal = pRule->IsAbsSpaces();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NUMBERING_IS_OUTLINE)))
    {
        sal_Bool bVal = pRule->IsOutlineRule();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    // --> OD 2008-04-23 #refactorlists#
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_DEFAULT_LIST_ID)))
    {
        ASSERT( pRule->GetDefaultListId().Len() != 0,
                "<SwXNumberingRules::getPropertyValue(..)> - no default list id found. Serious defect -> please inform OD." );
        aRet <<= OUString(pRule->GetDefaultListId());
    }
    // <--
    else
        throw UnknownPropertyException();
    return aRet;
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::addPropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::removePropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::addVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 19.07.00 07:49:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/* -----------------------------17.10.00 14:23--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXNumberingRules::getName() throw( RuntimeException )
{
    String aString;
    if(pNumRule)
    {
        SwStyleNameMapper::FillProgName(pNumRule->GetName(), aString, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE, sal_True );
        return OUString ( aString );
    }
    // --> OD 2005-10-25 #126347# - consider chapter numbering <SwXNumberingRules>
    else if ( pDocShell )
    {
        SwStyleNameMapper::FillProgName( pDocShell->GetDoc()->GetOutlineNumRule()->GetName(),
                                         aString, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE, sal_True );
        return OUString ( aString );
    }
    // <--
    else
        return sCreatedNumRuleName;
}
/* -----------------------------17.10.00 14:23--------------------------------

 ---------------------------------------------------------------------------*/
void SwXNumberingRules::setName(const OUString& /*rName*/) throw( RuntimeException )
{
    RuntimeException aExcept;
    aExcept.Message = C2U("readonly");
    throw aExcept;
}
/*-- 14.12.98 14:58:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXNumberingRules::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        if(bOwnNumRuleCreated)
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
sal_Bool SwXChapterNumbering::supportsService(const OUString& rServiceName) throw( RuntimeException )
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
sal_Bool SwXTextColumns::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextColumns") == rServiceName;
}
/* -----------------------------06.04.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextColumns::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextColumns");
    return aRet;
}
/* -----------------------------24.10.00 16:45--------------------------------

 ---------------------------------------------------------------------------*/
SwXTextColumns::SwXTextColumns(sal_uInt16 nColCount) :
    nReference(0),
    bIsAutomaticWidth(sal_True),
    nAutoDistance(0),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_COLUMS)),
    nSepLineWidth(0),
    nSepLineColor(0), //black
    nSepLineHeightRelative(100),//full height
    nSepLineVertAlign(style::VerticalAlignment_MIDDLE),
    bSepLineIsOn(sal_False)
{
    if(nColCount)
        setColumnCount(nColCount);
}
/*-- 16.12.98 14:06:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextColumns::SwXTextColumns(const SwFmtCol& rFmtCol) :
    nReference(0),
    aTextColumns(rFmtCol.GetNumCols()),
    bIsAutomaticWidth(rFmtCol.IsOrtho()),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_COLUMS))
{
    sal_uInt16 nItemGutterWidth = rFmtCol.GetGutterWidth();
    nAutoDistance = bIsAutomaticWidth ?
                        USHRT_MAX == nItemGutterWidth ? DEF_GUTTER_WIDTH : (sal_Int32)nItemGutterWidth
                        : 0;
    nAutoDistance = TWIP_TO_MM100(nAutoDistance);

    TextColumn* pColumns = aTextColumns.getArray();
    const SwColumns& rCols = rFmtCol.GetColumns();
    for(sal_uInt16 i = 0; i < aTextColumns.getLength(); i++)
    {
        SwColumn* pCol = rCols[i];

        pColumns[i].Width = pCol->GetWishWidth();
        nReference += pColumns[i].Width;
        pColumns[i].LeftMargin =    TWIP_TO_MM100_UNSIGNED(pCol->GetLeft ());
        pColumns[i].RightMargin =   TWIP_TO_MM100_UNSIGNED(pCol->GetRight());
    }
    if(!aTextColumns.getLength())
        nReference = USHRT_MAX;

    nSepLineWidth = rFmtCol.GetLineWidth();
    nSepLineColor = rFmtCol.GetLineColor().GetColor();
    nSepLineHeightRelative = rFmtCol.GetLineHeight();
    bSepLineIsOn = rFmtCol.GetLineAdj() != COLADJ_NONE;
    switch(rFmtCol.GetLineAdj())
    {
        case COLADJ_TOP:    nSepLineVertAlign = style::VerticalAlignment_TOP;   break;
        case COLADJ_BOTTOM: nSepLineVertAlign = style::VerticalAlignment_BOTTOM;    break;
        case COLADJ_CENTER:
        case COLADJ_NONE:   nSepLineVertAlign = style::VerticalAlignment_MIDDLE;
    }
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
    return static_cast< sal_Int16>( aTextColumns.getLength() );
}
/*-- 16.12.98 14:06:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::setColumnCount(sal_Int16 nColumns) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(nColumns <= 0)
        throw uno::RuntimeException();
    bIsAutomaticWidth = sal_True;
    aTextColumns.realloc(nColumns);
     TextColumn* pCols = aTextColumns.getArray();
    nReference = USHRT_MAX;
    sal_Int32 nWidth = nReference / nColumns;
    sal_Int32 nDiff = nReference - nWidth * nColumns;
    sal_Int32 nDist = nAutoDistance / 2;
    for(sal_Int16 i = 0; i < nColumns; i++)
    {
        pCols[i].Width = nWidth;
        pCols[i].LeftMargin = i == 0 ? 0 : nDist;
        pCols[i].RightMargin = i == nColumns - 1 ? 0 : nDist;
    }
    pCols[nColumns - 1].Width += nDiff;
}
/*-- 16.12.98 14:06:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< TextColumn > SwXTextColumns::getColumns(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return aTextColumns;
}
/*-- 16.12.98 14:06:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::setColumns(const uno::Sequence< TextColumn >& rColumns)
            throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nReferenceTemp = 0;
    const TextColumn* prCols = rColumns.getConstArray();
    for(long i = 0; i < rColumns.getLength(); i++)
    {
        nReferenceTemp += prCols[i].Width;
    }
    bIsAutomaticWidth = sal_False;
    nReference = !nReferenceTemp ? USHRT_MAX : nReferenceTemp;
    aTextColumns = rColumns;
}
/*-- 25.10.00 10:15:39---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XPropertySetInfo > SwXTextColumns::getPropertySetInfo(  ) throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropSet->getPropertySetInfo();
    return aRef;
}
/*-- 25.10.00 10:15:39---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
            WrappedTargetException, RuntimeException)
{
    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap()->getByName( rPropertyName );
    if (!pEntry)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pEntry->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    switch(pEntry->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
        {
            sal_Int32 nTmp = 0;
            aValue >>= nTmp;
            if(nTmp < 0)
                throw IllegalArgumentException();
            nSepLineWidth = MM100_TO_TWIP(nTmp);
        }
        break;
        case WID_TXTCOL_LINE_COLOR:
            aValue >>= nSepLineColor;
        break;
        case WID_TXTCOL_LINE_REL_HGT:
        {
            sal_Int8 nTmp = 0;
            aValue >>= nTmp;
            if(nTmp < 0)
                throw IllegalArgumentException();
            nSepLineHeightRelative = nTmp;
        }
        break;
        case WID_TXTCOL_LINE_ALIGN:
        {
            style::VerticalAlignment eAlign;
            if(!(aValue >>= eAlign) )
            {
                sal_Int8 nTmp = 0;
                if (! ( aValue >>= nTmp ) )
                    throw IllegalArgumentException();
                else
                    nSepLineVertAlign = nTmp;
            }
            else
                nSepLineVertAlign = static_cast< sal_Int8 >(eAlign);
        }
        break;
        case WID_TXTCOL_LINE_IS_ON:
            bSepLineIsOn = *(sal_Bool*)aValue.getValue();
        break;
        case WID_TXTCOL_AUTO_DISTANCE:
        {
            sal_Int32 nTmp = 0;
            aValue >>= nTmp;
            if(nTmp < 0 || nTmp >= nReference)
                throw IllegalArgumentException();
            nAutoDistance = nTmp;
            sal_Int32 nColumns = aTextColumns.getLength();
            TextColumn* pCols = aTextColumns.getArray();
            sal_Int32 nDist = nAutoDistance / 2;
            for(sal_Int32 i = 0; i < nColumns; i++)
            {
                pCols[i].LeftMargin = i == 0 ? 0 : nDist;
                pCols[i].RightMargin = i == nColumns - 1 ? 0 : nDist;
            }
        }
        break;
    }
}
/*-- 25.10.00 10:15:40---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXTextColumns::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap()->getByName( rPropertyName );
    if (!pEntry)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    Any aRet;
    switch(pEntry->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
            aRet <<= static_cast < sal_Int32 >(TWIP_TO_MM100(nSepLineWidth));
        break;
        case WID_TXTCOL_LINE_COLOR:
            aRet <<= nSepLineColor;
        break;
        case WID_TXTCOL_LINE_REL_HGT:
            aRet <<= nSepLineHeightRelative;
        break;
        case WID_TXTCOL_LINE_ALIGN:
            aRet <<= (style::VerticalAlignment)nSepLineVertAlign;
        break;
        case WID_TXTCOL_LINE_IS_ON:
            aRet.setValue(&bSepLineIsOn, ::getBooleanCppuType());
        break;
        case WID_TXTCOL_IS_AUTOMATIC :
            aRet.setValue(&bIsAutomaticWidth, ::getBooleanCppuType());
        break;
        case WID_TXTCOL_AUTO_DISTANCE:
            aRet <<= nAutoDistance;
        break;
    }
    return aRet;
}
/*-- 25.10.00 10:15:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::addPropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 25.10.00 10:15:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::removePropertyChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 25.10.00 10:15:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::addVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 25.10.00 10:15:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextColumns::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/, const uno::Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/* -----------------------------25.10.00 11:04--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextColumns::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextColumns::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

