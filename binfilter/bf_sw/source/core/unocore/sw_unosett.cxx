/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <swtypes.hxx>
#include <cmdid.h>
#include <hintids.hxx>
#include "poolfmt.hrc"
#include "poolfmt.hxx"

#include <errhdl.hxx>

#include <fmtcol.hxx>

#include <unomap.hxx>

#include <cppuhelper/implbase4.hxx>

#include <unostyle.hxx>
#include <unosett.hxx>
#include <unoprnms.hxx>
#include <bf_svtools/itemprop.hxx>
#include <ftninfo.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <pagedesc.hxx>
#include <charfmt.hxx>
#include <lineinfo.hxx>
#include <docsh.hxx>
#include <docary.hxx>
#include <docstyle.hxx>
#include <fmtclds.hxx>
#include <bf_svx/brshitem.hxx>
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
#include <unoobj.hxx>
#include <vcl/font.hxx>
#include <bf_svx/flstitem.hxx>
#include <vcl/metric.hxx>
#include <bf_svtools/ctrltool.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <bf_svx/unofdesc.hxx>
#include <bf_svx/unomid.hxx>
#include <vcl/graph.hxx>
#include <rtl/string.hxx>
#include <fmtornt.hxx>
#include <unomid.h>
#include <SwStyleNameMapper.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;

using rtl::OUString;

struct PropValData
{
    uno::Any 		aVal;
    OUString			sPropName;
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


#define WID_PREFIX						0
#define WID_SUFFIX  					1
#define WID_NUMBERING_TYPE              2
#define WID_START_AT                    3
#define WID_FOOTNOTE_COUNTING 			4
#define WID_PARAGRAPH_STYLE             5
#define WID_PAGE_STYLE                  6
#define WID_CHARACTER_STYLE             7
#define WID_POSITION_END_OF_DOC         8
#define WID_END_NOTICE                  9
#define WID_BEGIN_NOTICE                10
#define WID_ANCHOR_CHARACTER_STYLE		11

const SfxItemPropertyMap* GetFootnoteMap()
{
    static SfxItemPropertyMap aFootnoteMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_ANCHOR_CHAR_STYLE_NAME),WID_ANCHOR_CHARACTER_STYLE, &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_BEGIN_NOTICE),  		WID_BEGIN_NOTICE, 		&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),  		WID_CHARACTER_STYLE, 	&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_END_NOTICE),  			WID_END_NOTICE ,  		&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_FOOTNOTE_COUNTING), 	WID_FOOTNOTE_COUNTING, 	&::getCppuType((const sal_Int16*)0), PROPERTY_NONE,  	0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),		WID_NUMBERING_TYPE, 	&::getCppuType((const sal_Int16*)0), PROPERTY_NONE,     	0},
        { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_NAME),  		WID_PAGE_STYLE, 		&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_PARA_STYLE_NAME),  		WID_PARAGRAPH_STYLE, 	&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_POSITION_END_OF_DOC), 	WID_POSITION_END_OF_DOC,&::getBooleanCppuType(), PROPERTY_NONE,     	0},
        { SW_PROP_NAME(UNO_NAME_PREFIX),				WID_PREFIX, 			&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_START_AT),  			WID_START_AT , 			&::getCppuType((const sal_Int16*)0), PROPERTY_NONE,     	0},
        { SW_PROP_NAME(UNO_NAME_SUFFIX),  				WID_SUFFIX, 			&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        {0,0,0,0}
    };
    return aFootnoteMap_Impl;
}
const SfxItemPropertyMap* GetEndnoteMap()
{
    static SfxItemPropertyMap aEndnoteMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_ANCHOR_CHAR_STYLE_NAME),WID_ANCHOR_CHARACTER_STYLE, &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),  		WID_CHARACTER_STYLE, 	&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),		WID_NUMBERING_TYPE, 	&::getCppuType((const sal_Int16*)0), PROPERTY_NONE,     	0},
        { SW_PROP_NAME(UNO_NAME_PAGE_STYLE_NAME),  		WID_PAGE_STYLE, 		&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_PARA_STYLE_NAME),  		WID_PARAGRAPH_STYLE, 	&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_PREFIX),				WID_PREFIX, 	&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_START_AT),  			WID_START_AT , 			&::getCppuType((const sal_Int16*)0), PROPERTY_NONE,     	0},
        { SW_PROP_NAME(UNO_NAME_SUFFIX),  				WID_SUFFIX, 	&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        {0,0,0,0}
    };
    return aEndnoteMap_Impl;
}
const SfxItemPropertyMap* GetNumberingRulesMap()
{
    static SfxItemPropertyMap aNumberingRulesMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_IS_ABSOLUTE_MARGINS),		WID_IS_ABS_MARGINS,	&::getBooleanCppuType(), 			PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC),  			WID_IS_AUTOMATIC,	&::getBooleanCppuType(), 			PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_IS_CONTINUOUS_NUMBERING),  	WID_CONTINUOUS, 	&::getBooleanCppuType(), 			PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NAME),  					WID_RULE_NAME	, 	&::getCppuType((const OUString*)0), PropertyAttribute::READONLY,     0},
        {0,0,0,0}
    };
    return aNumberingRulesMap_Impl;
}
#define WID_NUM_ON						0
#define WID_SEPARATOR_INTERVAL          1
#define WID_NUMBERING_TYPE              2
#define WID_NUMBER_POSITION             3
#define WID_DISTANCE                    4
#define WID_INTERVAL                    5
#define WID_SEPARATOR_TEXT              6
//#define WID_CHARACTER_STYLE             7
#define WID_COUNT_EMPTY_LINES           8
#define WID_COUNT_LINES_IN_FRAMES       9
#define WID_RESTART_AT_EACH_PAGE		10

const SfxItemPropertyMap* GetLineNumberingMap()
{
    static SfxItemPropertyMap aLineNumberingMap_Impl[] =
    {
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME),         WID_CHARACTER_STYLE,    &::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_COUNT_EMPTY_LINES),		  WID_COUNT_EMPTY_LINES , &::getBooleanCppuType(),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_COUNT_LINES_IN_FRAMES),   WID_COUNT_LINES_IN_FRAMES, &::getBooleanCppuType(),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_DISTANCE       ),         WID_DISTANCE       ,    &::getCppuType((const sal_Int32*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_IS_ON), 					WID_NUM_ON, 			&::getBooleanCppuType()  ,  		PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_INTERVAL  ),              WID_INTERVAL  ,    	&::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_SEPARATOR_TEXT ),         WID_SEPARATOR_TEXT, 	&::getCppuType((const OUString*)0), PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBER_POSITION),         WID_NUMBER_POSITION,    &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE),          WID_NUMBERING_TYPE ,    &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_RESTART_AT_EACH_PAGE), 	  WID_RESTART_AT_EACH_PAGE, &::getBooleanCppuType()  ,  		PROPERTY_NONE,     0},
        { SW_PROP_NAME(UNO_NAME_SEPARATOR_INTERVAL), 	  WID_SEPARATOR_INTERVAL, &::getCppuType((const sal_Int16*)0),PROPERTY_NONE,     0},
        {0,0,0,0}
    };
    return aLineNumberingMap_Impl;
}

SwCharFmt* lcl_getCharFmt(SwDoc* pDoc, const uno::Any& aValue)
{
    SwCharFmt* pRet = 0;
    String sStandard(SW_RES(STR_POOLCOLL_STANDARD));
    OUString uTmp;
    aValue >>= uTmp;
    String sCharFmt;
    SwStyleNameMapper::FillUIName(uTmp, sCharFmt, GET_POOLID_CHRFMT, sal_True);
    if(sStandard != sCharFmt)
    {
        pRet = pDoc->FindCharFmtByName( sCharFmt );
    }
    if(!pRet)
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(sCharFmt, GET_POOLID_CHRFMT);
        if(USHRT_MAX != nId)
            pRet = pDoc->GetCharFmtFromPool( nId );
    }
    return pRet;
}

SwTxtFmtColl* lcl_GetParaStyle(SwDoc* pDoc, const uno::Any& aValue)
{
    OUString uTmp;
    aValue >>= uTmp;
    String sParaStyle;
    SwStyleNameMapper::FillUIName(uTmp, sParaStyle, GET_POOLID_TXTCOLL, sal_True );
    SwTxtFmtColl* pRet = pDoc->FindTxtFmtCollByName( sParaStyle );
    if( !pRet  )
    {
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sParaStyle, GET_POOLID_TXTCOLL );
        if( USHRT_MAX != nId  )
            pRet = pDoc->GetTxtCollFromPool( nId );
    }
    return pRet;
}

SwPageDesc* lcl_GetPageDesc(SwDoc* pDoc, const uno::Any& aValue)
{
    SwPageDesc* pRet = 0;
    sal_uInt16 nCount = pDoc->GetPageDescCnt();
    OUString uTmp;
    aValue >>= uTmp;
    String sPageDesc;
    SwStyleNameMapper::FillUIName(uTmp, sPageDesc, GET_POOLID_PAGEDESC, sal_True );
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
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(sPageDesc, GET_POOLID_PAGEDESC);
        if(USHRT_MAX != nId)
            pRet = pDoc->GetPageDescFromPool( nId );
    }
    return pRet;
}

// Numerierung
const unsigned short aSvxToUnoAdjust[] =
{
    HoriOrientation::LEFT,	//3
    HoriOrientation::RIGHT,  //1
    USHRT_MAX,
    HoriOrientation::CENTER, //2
    USHRT_MAX,
    USHRT_MAX
};

const unsigned short aUnoToSvxAdjust[] =
{
    USHRT_MAX,
    SVX_ADJUST_RIGHT,       // 1
    SVX_ADJUST_CENTER,      // 3
    SVX_ADJUST_LEFT,		// 0
    USHRT_MAX,
    USHRT_MAX
};

/******************************************************************
 * SwXFootnoteProperties
 ******************************************************************/
OUString SwXFootnoteProperties::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFootnoteProperties");
}

BOOL SwXFootnoteProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.FootnoteSettings") == rServiceName;
}

Sequence< OUString > SwXFootnoteProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.FootnoteSettings");
    return aRet;
}

SwXFootnoteProperties::SwXFootnoteProperties(SwDoc* pDc) :
    pDoc(pDc),
    _pMap(GetFootnoteMap())
{
}

SwXFootnoteProperties::~SwXFootnoteProperties()
{
}

uno::Reference< beans::XPropertySetInfo >  SwXFootnoteProperties::getPropertySetInfo(void)
                                                                throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}

void SwXFootnoteProperties::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pDoc)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXFootnoteProperties::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(pDoc)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
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
                    SwStyleNameMapper::FillProgName(aString, aString, GET_POOLID_TXTCOLL, sal_True);
                    aRet <<= OUString ( aString );
                }
                break;
                case  WID_PAGE_STYLE :
                {
                    String aString;
                    if( rFtnInfo.GetPageDescDep()->GetRegisteredIn() )
                    {
                        SwStyleNameMapper::FillProgName(
                                rFtnInfo.GetPageDesc( *pDoc )->GetName(),
                                aString,
                                GET_POOLID_PAGEDESC,
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
                    if( pMap->nWID == WID_ANCHOR_CHARACTER_STYLE )
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
                                GET_POOLID_CHRFMT,
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

void SwXFootnoteProperties::addPropertyChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXFootnoteProperties::removePropertyChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXFootnoteProperties::addVetoableChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXFootnoteProperties::removeVetoableChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

/******************************************************************
 * SwXEndnoteProperties
 ******************************************************************/
OUString SwXEndnoteProperties::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXEndnoteProperties");
}

BOOL SwXEndnoteProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.FootnoteSettings") == rServiceName;
}

Sequence< OUString > SwXEndnoteProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.FootnoteSettings");
    return aRet;
}

SwXEndnoteProperties::SwXEndnoteProperties(SwDoc* pDc) :
    pDoc(pDc),
    _pMap(GetEndnoteMap())
{
}

SwXEndnoteProperties::~SwXEndnoteProperties()
{
}

uno::Reference< beans::XPropertySetInfo >  SwXEndnoteProperties::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}

void SwXEndnoteProperties::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pDoc)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
                    aEndInfo.aFmt.SetNumberingType(nTmp);
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
                case WID_ANCHOR_CHARACTER_STYLE:
                case  WID_CHARACTER_STYLE    :
                {
                    SwCharFmt* pFmt = lcl_getCharFmt(pDoc, aValue);
                    if(pFmt)
                    {
                        if(pMap->nWID == WID_ANCHOR_CHARACTER_STYLE)
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

uno::Any SwXEndnoteProperties::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(pDoc)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
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
                            GET_POOLID_TXTCOLL,
                            sal_True);
                    aRet <<= OUString ( aString );

                }
                break;
                case  WID_PAGE_STYLE :
                {
                    String aString;
                    if( rEndInfo.GetPageDescDep()->GetRegisteredIn() )
                    {
                        SwStyleNameMapper::FillProgName(
                            rEndInfo.GetPageDesc( *pDoc )->GetName(),
                            aString,
                            GET_POOLID_PAGEDESC,
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
                    if( pMap->nWID == WID_ANCHOR_CHARACTER_STYLE )
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
                                GET_POOLID_CHRFMT,
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

void SwXEndnoteProperties::addPropertyChangeListener(
    const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXEndnoteProperties::removePropertyChangeListener(const OUString& PropertyName,
        const uno:: Reference< beans::XPropertyChangeListener > & aListener)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXEndnoteProperties::addVetoableChangeListener(const OUString& PropertyName,
    const uno:: Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXEndnoteProperties::removeVetoableChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

/******************************************************************
 * SwXLineNumberingProperties
 ******************************************************************/
OUString SwXLineNumberingProperties::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXLineNumberingProperties");
}

BOOL SwXLineNumberingProperties::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.LineNumberingProperties") == rServiceName;
}

Sequence< OUString > SwXLineNumberingProperties::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.LineNumberingProperties");
    return aRet;
}

SwXLineNumberingProperties::SwXLineNumberingProperties(SwDoc* pDc) :
    pDoc(pDc),
    _pMap(GetLineNumberingMap())
{
}

SwXLineNumberingProperties::~SwXLineNumberingProperties()
{
}

uno::Reference< beans::XPropertySetInfo >  SwXLineNumberingProperties::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}

void SwXLineNumberingProperties::setPropertyValue(
    const OUString& rPropertyName, const Any& aValue)
        throw( UnknownPropertyException, PropertyVetoException,
                IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pDoc)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
                    SvxNumberType aNumType(aInfo.GetNumType());
                    INT16 nTmp;
                    aValue >>= nTmp;
                    aNumType.SetNumberingType(nTmp);
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
                    aInfo.SetPosFromLeft(Min(static_cast<sal_Int32>(MM100_TO_TWIP(nVal)), sal_Int32(0xffff)));
                }
                break;
                case WID_INTERVAL   :
                {
                    INT16 nTmp;
                    aValue >>= nTmp;
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

Any SwXLineNumberingProperties::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Any aRet;
    if(pDoc)
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
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
                {
                    String aString;
                    // return empty string if no char format is set
                    // otherwise it would be created here
                    if(rInfo.GetRegisteredIn())
                    {
                        SwStyleNameMapper::FillProgName(
                                    rInfo.GetCharFmt(*pDoc)->GetName(),
                                    aString,
                                    GET_POOLID_CHRFMT,
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
                    aRet <<= static_cast < sal_Int32 >(TWIP_TO_MM100(nPos));
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

void SwXLineNumberingProperties::addPropertyChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented");
}

void SwXLineNumberingProperties::removePropertyChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented");
}

void SwXLineNumberingProperties::addVetoableChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented");
}

void SwXLineNumberingProperties::removeVetoableChangeListener(const OUString& PropertyName, const uno:: Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
DBG_WARNING("not implemented");
}

/******************************************************************
 * SwXNumberingRules
 ******************************************************************/
String	SwXNumberingRules::sInvalidStyle(String::CreateFromAscii("__XXX___invalid"));

const String&	SwXNumberingRules::GetInvalidStyle()
{
    return sInvalidStyle;
}

const uno::Sequence< sal_Int8 > & SwXNumberingRules::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

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

OUString SwXNumberingRules::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXNumberingRules");
}

BOOL SwXNumberingRules::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.NumberingRules") == rServiceName;
}

Sequence< OUString > SwXNumberingRules::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.NumberingRules");
    return aRet;
}

SwXNumberingRules::SwXNumberingRules(const SwNumRule& rRule) :
    pNumRule(new SwNumRule(rRule)),
    bOwnNumRuleCreated(TRUE),
    pDoc(0),
    pDocShell(0),
    _pMap(GetNumberingRulesMap())
{
    //erstmal das Doc organisieren; es haengt an den gesetzten Zeichenvorlagen - wenn
    // keine gesetzt sind, muss es auch ohne gehen
    sal_uInt16 i=0;
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

SwXNumberingRules::SwXNumberingRules(SwDocShell& rDocSh) :
    pDoc(0),
    pNumRule(0),
    pDocShell(&rDocSh),
    bOwnNumRuleCreated(FALSE),
    _pMap(GetNumberingRulesMap())
{
    pDocShell->GetDoc()->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXNumberingRules::SwXNumberingRules(SwDoc& rDoc) :
    pDoc(&rDoc),
    pNumRule(0),
    pDocShell(0),
    bOwnNumRuleCreated(FALSE),
    _pMap(GetNumberingRulesMap())
{
    rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    sCreatedNumRuleName = rDoc.GetUniqueNumRuleName();
    sal_uInt16 nIndex = rDoc.MakeNumRule( sCreatedNumRuleName, 0 );
}

SwXNumberingRules::~SwXNumberingRules()
{
    if(pDoc && sCreatedNumRuleName.Len())
        pDoc->DelNumRule( sCreatedNumRuleName );
    if( pNumRule && bOwnNumRuleCreated )
        delete pNumRule;
}

void SwXNumberingRules::replaceByIndex(sal_Int32 nIndex, const uno::Any& rElement)
    throw( lang::IllegalArgumentException, lang::IndexOutOfBoundsException,
                  lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(rElement.getValueType().getTypeClass() != uno::TypeClass_SEQUENCE)
        throw lang::IllegalArgumentException();
    const uno::Sequence<beans::PropertyValue>& rProperties =
                    *(const uno::Sequence<beans::PropertyValue>*)rElement.getValue();
    SwNumRule* pRule = 0;
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
        SwXNumberingRules::setNumberingRuleByIndex( *pRule,
                            rProperties, nIndex);
        sal_uInt16 nPos = pDoc->FindNumRule( sCreatedNumRuleName );
        pDoc->UpdateNumRule( sCreatedNumRuleName, nPos );
    }
    else
        throw uno::RuntimeException();

}

sal_Int32 SwXNumberingRules::getCount(void) throw( uno::RuntimeException )
{
    return MAXLEVEL;
}

uno::Any SwXNumberingRules::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
            uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aVal;
    if(MAXLEVEL <= nIndex)
        throw lang::IndexOutOfBoundsException();
    const SwNumRule* pRule = pNumRule;
    if(!pRule && pDoc && sCreatedNumRuleName.Len())
        pRule = pDoc->FindNumRulePtr( sCreatedNumRuleName );
    if(pRule)
    {
        uno::Sequence<beans::PropertyValue> aRet = getNumberingRuleByIndex(
                                        *pRule, nIndex);
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

uno::Type SwXNumberingRules::getElementType(void)
    throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Sequence<beans::PropertyValue>*)0);
}

sal_Bool SwXNumberingRules::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

uno::Sequence<beans::PropertyValue> SwXNumberingRules::getNumberingRuleByIndex(
                const SwNumRule& rNumRule, sal_Int32 nIndex) const
{
    SolarMutexGuard aGuard;
    const SwNumFmt& rFmt = rNumRule.Get( (sal_uInt16)nIndex );

    sal_Bool bChapterNum = pDocShell != 0;

    PropValDataArr	aPropertyValues;
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
    SwStyleNameMapper::FillProgName( CharStyleName, aString, GET_POOLID_CHRFMT, sal_True );
    aUString = aString;
    pData = new PropValData((void*)&aUString, "CharStyleName", ::getCppuType((const OUString*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

    //startvalue
    nINT16 = rFmt.GetStart();
    pData = new PropValData((void*)&nINT16, "StartWith", ::getCppuType((const sal_Int16*)0));
    aPropertyValues.Insert(pData, aPropertyValues.Count());

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
            aSize.Width() = TWIP_TO_MM100( aSize.Width() );
            aSize.Height() = TWIP_TO_MM100( aSize.Height() );
            pData = new PropValData((void*)&aSize, SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_SIZE), ::getCppuType((const awt::Size*)0));
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

            sal_Int8 nOutLevel = rTxtColl.GetOutlineLevel();
            if(nOutLevel == nIndex)
            {
                sValue = rTxtColl.GetName();
            }
        }
        String aString;
        SwStyleNameMapper::FillProgName(sValue, aString, GET_POOLID_TXTCOLL, sal_True);
        aUString = aString;

        pData = new PropValData((void*)&aUString, SW_PROP_NAME_STR(UNO_NAME_HEADING_STYLE_NAME), ::getCppuType((const OUString*)0));
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

PropValData* lcl_FindProperty(const char* cName, PropValDataArr&	rPropertyValues)
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

void SwXNumberingRules::setNumberingRuleByIndex(
            SwNumRule& rNumRule,
            const uno::Sequence<beans::PropertyValue>& rProperties, sal_Int32 nIndex)
    throw( uno::RuntimeException, lang::IllegalArgumentException )
{
    SolarMutexGuard aGuard;
    // the order of the names is important!
    static const char* aNumPropertyNames[] =
    {
        "Adjust",  								//0
        "ParentNumbering",                      //1
        "Prefix",                               //2
        "Suffix",                               //3
        "CharStyleName",                        //4
        "StartWith",                            //5
        SW_PROP_NAME_STR(UNO_NAME_LEFT_MARGIN),                   //6
        SW_PROP_NAME_STR(UNO_NAME_SYMBOL_TEXT_DISTANCE),          //7
        SW_PROP_NAME_STR(UNO_NAME_FIRST_LINE_OFFSET),             //8
        "NumberingType",                        //9
        "BulletId",                             //10
        SW_PROP_NAME_STR(UNO_NAME_BULLET_FONT),                   //11
        "BulletFontName",                       //12
        "BulletChar",                           //13
        SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_URL),                   //14
        SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_BITMAP),                //15
        SW_PROP_NAME_STR(UNO_NAME_GRAPHIC_SIZE),                  //16
        SW_PROP_NAME_STR(UNO_NAME_VERT_ORIENT),					//17
        SW_PROP_NAME_STR(UNO_NAME_HEADING_STYLE_NAME)             //18
    };
    const sal_uInt16 nPropNameCount = 19;
    const sal_uInt16 nNotInChapter = 10;


    SwNumFmt aFmt(rNumRule.Get( (sal_uInt16)nIndex ));
    const beans::PropertyValue* pPropArray = rProperties.getConstArray();
    PropValDataArr	aPropertyValues;
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

            bExcept = sal_False;
        PropValData* pData = new PropValData(rProp.Value, rProp.Name );
        aPropertyValues.Insert(pData, aPropertyValues.Count());
    }
    sal_Bool bWrongArg = sal_False;
    if(!bExcept)
       {
        SvxBrushItem* pSetBrush = 0;
        Size* pSetSize = 0;
        SwFmtVertOrient* pSetVOrient = 0;
        BOOL bCharStyleNameSet = FALSE;

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
                        nValue <= HoriOrientation::LEFT &&
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
                    sal_Int16 nSet;
                    pData->aVal >>= nSet;
                    if(nSet >= 0 && MAXLEVEL >= nSet)
                        aFmt.SetIncludeUpperLevels(nSet);
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
                    bCharStyleNameSet = TRUE;
                    OUString uTmp;
                    pData->aVal >>= uTmp;
                    String sCharFmtName;
                    SwStyleNameMapper::FillUIName( uTmp, sCharFmtName, GET_POOLID_CHRFMT, sal_True );
                    SwCharFmt* pCharFmt = 0;
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
                    aFmt.SetStart(nVal);
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
                        aFmt.SetCharTextDistance((sal_uInt16) MM100_TO_TWIP(nValue));
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
                    if(nSet >= 0)
                        aFmt.SetNumberingType(nSet);
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
                        const SvxBrushItem* pOrigBrush = aFmt.GetBrush();
                        if(pOrigBrush)
                        {
                            pSetBrush = new SvxBrushItem(*pOrigBrush);
                        }
                        else
                            pSetBrush = new SvxBrushItem(aEmptyStr, aEmptyStr, GPOS_AREA);
                    }
                    pSetBrush->PutValue( pData->aVal, MID_GRAPHIC_URL );
                }
                break;
                case 15: //UNO_NAME_GRAPHIC_BITMAP,
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
                        if(aFmt.GetGraphicOrientation())
                            pSetVOrient = (SwFmtVertOrient*)aFmt.GetGraphicOrientation()->Clone();
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
                    String sStyleName;
                    SwStyleNameMapper::FillUIName(uTmp, sStyleName, GET_POOLID_TXTCOLL, sal_True );
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
                case 19: // BulletRelSize - unsupported - only available in Impress
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
                            *pSetSize = ::binfilter::GetGraphicSizeTwip(*pGraphic, 0);
                    }
                }
                SvxFrameVertOrient eOrient = pSetVOrient ?
                    (SvxFrameVertOrient)pSetVOrient->GetVertOrient() : SVX_VERT_NONE;
                aFmt.SetGraphicBrush( pSetBrush, pSetSize, SVX_VERT_NONE == eOrient ? 0 : &eOrient );
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

Reference< XPropertySetInfo > SwXNumberingRules::getPropertySetInfo()
    throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}

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
        BOOL bVal = *(sal_Bool*)rValue.getValue();
        if(!pCreatedRule)
            pDocRule ? pDocRule->SetAutoRule(bVal) : pNumRule->SetAutoRule(bVal);
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_CONTINUOUS_NUMBERING)))
    {
        BOOL bVal = *(sal_Bool*)rValue.getValue();
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
        BOOL bVal = *(sal_Bool*)rValue.getValue();
        pDocRule ? pDocRule->SetAbsSpaces(bVal) :
            pCreatedRule ? pCreatedRule->SetAbsSpaces(bVal) : pNumRule->SetAbsSpaces(bVal);
    }
    else
        throw UnknownPropertyException();
    if(pDocRule)
    {
        pDocShell->GetDoc()->SetOutlineNumRule(*pDocRule);
        delete pDocRule;
    }
    else if(pCreatedRule)
    {
        sal_uInt16 nPos = pDoc->FindNumRule( sCreatedNumRuleName );
        pDoc->UpdateNumRule( sCreatedNumRuleName, nPos );
    }
}

Any SwXNumberingRules::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    const SwNumRule* pRule = pNumRule;
    SwNumRule* pCreatedRule = 0;
    if(!pRule && pDocShell)
        pRule = pDocShell->GetDoc()->GetOutlineNumRule();
    else if(pDoc && sCreatedNumRuleName.Len())
        pRule = pDoc->FindNumRulePtr( sCreatedNumRuleName );
    if(!pRule)
        throw RuntimeException();

    if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_AUTOMATIC)))
    {
        BOOL bVal = pRule->IsAutoRule();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_CONTINUOUS_NUMBERING)))
    {
        BOOL bVal = pRule->IsContinusNum();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_NAME)))
        aRet <<= OUString(pRule->GetName());
    else if(rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_IS_ABSOLUTE_MARGINS)))
    {
        BOOL bVal = pRule->IsAbsSpaces();
        aRet.setValue(&bVal, ::getBooleanCppuType());
    }
    else
        throw UnknownPropertyException();
    return aRet;
}

void SwXNumberingRules::addPropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXNumberingRules::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXNumberingRules::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXNumberingRules::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

OUString SwXNumberingRules::getName() throw( RuntimeException )
{
    String aString;
    if(pNumRule)
    {
        SwStyleNameMapper::FillProgName(pNumRule->GetName(), aString, GET_POOLID_NUMRULE, sal_True );
        return OUString ( aString );
    }
    else
        return sCreatedNumRuleName;
}

void SwXNumberingRules::setName(const OUString& Name_) throw( RuntimeException )
{
    RuntimeException aExcept;
    aExcept.Message = C2U("readonly");
    throw aExcept;
}

void SwXNumberingRules::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
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

/******************************************************************
 * SwXChapterNumbering
 ******************************************************************/
OUString SwXChapterNumbering::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXChapterNumbering");
}

BOOL SwXChapterNumbering::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    String sServiceName(rServiceName);
    return sServiceName.EqualsAscii("com.sun.star.text.ChapterNumbering") ||
            sServiceName.EqualsAscii("com.sun.star.text.NumberingRules");
}

Sequence< OUString > SwXChapterNumbering::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ChapterNumbering");
    pArray[1] = C2U("com.sun.star.text.NumberingRules");
    return aRet;
}

SwXChapterNumbering::SwXChapterNumbering(SwDocShell& rDocSh) :
    SwXNumberingRules(rDocSh)
{
}

SwXChapterNumbering::~SwXChapterNumbering()
{
}

/******************************************************************
 * SwXTextColumns
 ******************************************************************/
OUString SwXTextColumns::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextColumns");
}

BOOL SwXTextColumns::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.text.TextColumns") == rServiceName;
}

Sequence< OUString > SwXTextColumns::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextColumns");
    return aRet;
}

SwXTextColumns::SwXTextColumns(sal_uInt16 nColCount) :
    nReference(0),
    nSepLineWidth(0),
    nSepLineColor(0), //black
    nSepLineHeightRelative(100),//full height
    nSepLineVertAlign(style::VerticalAlignment_MIDDLE),
    bSepLineIsOn(sal_False),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_COLUMS)),
    bIsAutomaticWidth(sal_True),
    nAutoDistance(0)
{
    if(nColCount)
        setColumnCount(nColCount);
}

SwXTextColumns::SwXTextColumns(const SwFmtCol& rFmtCol) :
    aTextColumns(rFmtCol.GetNumCols()),
    nReference(0),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_COLUMS)),
    bIsAutomaticWidth(rFmtCol.IsOrtho())
{
    USHORT nItemGutterWidth = rFmtCol.GetGutterWidth();
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
        pColumns[i].LeftMargin = 	TWIP_TO_MM100(pCol->GetLeft ());
        pColumns[i].RightMargin = 	TWIP_TO_MM100(pCol->GetRight());
    }
    if(!aTextColumns.getLength())
        nReference = USHRT_MAX;

    nSepLineWidth = rFmtCol.GetLineWidth();
    nSepLineColor = rFmtCol.GetLineColor().GetColor();
    nSepLineHeightRelative = rFmtCol.GetLineHeight();
    bSepLineIsOn = rFmtCol.GetLineAdj() != COLADJ_NONE;
    switch(rFmtCol.GetLineAdj())
    {
        case COLADJ_TOP: 	nSepLineVertAlign = style::VerticalAlignment_TOP; 	break;
        case COLADJ_BOTTOM: nSepLineVertAlign = style::VerticalAlignment_BOTTOM;	break;
        case COLADJ_CENTER:
        case COLADJ_NONE:	nSepLineVertAlign = style::VerticalAlignment_MIDDLE;
    }
}

SwXTextColumns::~SwXTextColumns()
{
}

sal_Int32 SwXTextColumns::getReferenceValue(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return nReference;
}

sal_Int16 SwXTextColumns::getColumnCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return aTextColumns.getLength();
}

void SwXTextColumns::setColumnCount(sal_Int16 nColumns) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(nColumns <= 0)
        throw uno::RuntimeException();
    bIsAutomaticWidth = sal_True;
    aTextColumns.realloc(nColumns);
     TextColumn* pCols = aTextColumns.getArray();
    nReference = USHRT_MAX;
    sal_uInt16 nWidth = nReference / nColumns;
    sal_uInt16 nDiff = nReference - nWidth * nColumns;
    sal_Int32 nDist = nAutoDistance / 2;
    for(sal_Int16 i = 0; i < nColumns; i++)
    {
        pCols[i].Width = nWidth;
        pCols[i].LeftMargin = i == 0 ? 0 : nDist;
        pCols[i].RightMargin = i == nColumns - 1 ? 0 : nDist;
    }
    pCols[nColumns - 1].Width += nDiff;
}

uno::Sequence< TextColumn > SwXTextColumns::getColumns(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return aTextColumns;
}

void SwXTextColumns::setColumns(const uno::Sequence< TextColumn >& rColumns)
            throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_uInt16 nReferenceTemp = 0;
    const TextColumn* prCols = rColumns.getConstArray();
    for(long i = 0; i < rColumns.getLength(); i++)
    {
        //wenn die Breite Null ist, oder die Raender breiter als die Spalte werden -> exception
        if(!prCols[i].Width ||
            prCols[i].LeftMargin + prCols[i].RightMargin >= prCols[i].Width)
            throw uno::RuntimeException();
        nReferenceTemp += prCols[i].Width;
    }
    bIsAutomaticWidth = sal_False;
    nReference = !nReferenceTemp ? USHRT_MAX : nReferenceTemp;
    aTextColumns = rColumns;
}

Reference< XPropertySetInfo > SwXTextColumns::getPropertySetInfo(  ) throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo( _pMap );
    return aRef;
}

void SwXTextColumns::setPropertyValue( const OUString& rPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException,
            WrappedTargetException, RuntimeException)
{
    const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    if ( pMap->nFlags & PropertyAttribute::READONLY)
        throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    switch(pMap->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
        {
            sal_Int32 nTmp; aValue >>= nTmp;
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
            sal_Int8 nTmp; aValue >>= nTmp;
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
                sal_Int8 nTmp;
                if (! ( aValue >>= nTmp ) )
                    throw IllegalArgumentException();
                else
                    nSepLineVertAlign = static_cast < style::VerticalAlignment > ( nTmp );
            }
            else
                nSepLineVertAlign = eAlign;
        }
        break;
        case WID_TXTCOL_LINE_IS_ON:
            bSepLineIsOn = *(sal_Bool*)aValue.getValue();
        break;
        case WID_TXTCOL_AUTO_DISTANCE:
        {
            sal_Int32 nTmp;
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

Any SwXTextColumns::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                    _pMap, rPropertyName);
    if (!pMap)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    Any aRet;
    switch(pMap->nWID)
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

void SwXTextColumns::addPropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXTextColumns::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXTextColumns::addVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

void SwXTextColumns::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}

const uno::Sequence< sal_Int8 > & SwXTextColumns::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXTextColumns::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
