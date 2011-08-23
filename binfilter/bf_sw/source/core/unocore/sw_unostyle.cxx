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

#include <hintids.hxx>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <bf_svtools/smplhint.hxx>
#include <bf_svtools/ctrltool.hxx>
#include <bf_svtools/style.hxx>
#include <bf_so3/svstor.hxx>

#ifndef _SVX_PAGEITEM_HXX //autogen
#define ITEMID_SETITEM
#include <bf_svx/pageitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#define ITEMID_SIZE SID_ATTR_PAGE_SIZE
#include <bf_svx/sizeitem.hxx>
#endif
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/flstitem.hxx>
#include <bf_svx/svxids.hrc>
#include <bf_svx/paperinf.hxx>
#include <pagedesc.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>
#include <charfmt.hxx>
#include <cmdid.h>

#include <cppuhelper/implbase4.hxx>

#include <unostyle.hxx>
#include <unomap.hxx>
#include <unosett.hxx>
#include <docsh.hxx>
#include <swstyle.h>
#include <paratr.hxx>
#include <unomid.h>
#include <unoprnms.hxx>
#include <shellio.hxx>
#include <docstyle.hxx>

#include <errhdl.hxx>

#include <unoobj.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <tools/urlobj.hxx>
#include <poolfmt.hrc>
#include <poolfmt.hxx>
#include "unoevent.hxx"
#include <fmtruby.hxx>
#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <SwStyleNameMapper.hxx>
#include <bf_sfx2/printer.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <GetMetricVal.hxx>
#include <fmtfsize.hxx>
#define STYLE_FAMILY_COUNT 5            // we have 5 style families
#define TYPE_BOOL       0
#define TYPE_SIZE       1
#define TYPE_BRUSH      2
#define TYPE_ULSPACE    3
#define TYPE_SHADOW     4
#define TYPE_LRSPACE    5
#define TYPE_BOX        6
namespace binfilter {

const unsigned short aStyleByIndex[] =
{
    SFX_STYLE_FAMILY_CHAR,
    SFX_STYLE_FAMILY_PARA,
    SFX_STYLE_FAMILY_PAGE     ,
    SFX_STYLE_FAMILY_FRAME    ,
    SFX_STYLE_FAMILY_PSEUDO
};

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::container;
using ::com::sun::star::frame::XModel;

using rtl::OUString;
/******************************************************************************
 *
 ******************************************************************************/
//convert FN_... to RES_ in header and footer itemset
sal_uInt16 lcl_ConvertFNToRES(sal_uInt16 nFNId)
{
    sal_uInt16 nRes = USHRT_MAX;
    switch(nFNId)
    {
        case FN_UNO_FOOTER_ON:
        case FN_UNO_HEADER_ON:
        break;
        case FN_UNO_FOOTER_BACKGROUND:
        case FN_UNO_HEADER_BACKGROUND:      nRes = RES_BACKGROUND;
        break;
        case FN_UNO_FOOTER_BOX:
        case FN_UNO_HEADER_BOX:             nRes = RES_BOX;
        break;
        case FN_UNO_FOOTER_LR_SPACE:
        case FN_UNO_HEADER_LR_SPACE:        nRes = RES_LR_SPACE;
        break;
        case FN_UNO_FOOTER_SHADOW:
        case FN_UNO_HEADER_SHADOW:          nRes = RES_SHADOW;
        break;
        case FN_UNO_FOOTER_BODY_DISTANCE:
        case FN_UNO_HEADER_BODY_DISTANCE:   nRes = RES_UL_SPACE;
        break;
        case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE:
        case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE: nRes = SID_ATTR_PAGE_DYNAMIC;
        break;
        case FN_UNO_FOOTER_SHARE_CONTENT:
        case FN_UNO_HEADER_SHARE_CONTENT:   nRes = SID_ATTR_PAGE_SHARED;
        break;
        case FN_UNO_FOOTER_HEIGHT:
        case FN_UNO_HEADER_HEIGHT:          nRes = SID_ATTR_PAGE_SIZE;
        break;
        case FN_UNO_FOOTER_EAT_SPACING:
        case FN_UNO_HEADER_EAT_SPACING:   nRes = RES_HEADER_FOOTER_EAT_SPACING;
        break;
    }
    return nRes;

}

SwGetPoolIdFromName lcl_GetSwEnumFromSfxEnum ( SfxStyleFamily eFamily )
{
    switch ( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
            return GET_POOLID_CHRFMT;
        case SFX_STYLE_FAMILY_PARA:
            return GET_POOLID_TXTCOLL;
        case SFX_STYLE_FAMILY_FRAME:
            return GET_POOLID_FRMFMT;
        case SFX_STYLE_FAMILY_PAGE:
            return GET_POOLID_PAGEDESC;
        case SFX_STYLE_FAMILY_PSEUDO:
            return GET_POOLID_NUMRULE;
        default:
            DBG_ASSERT(sal_False, "someone asking for all styles in unostyle.cxx!" );
            return GET_POOLID_CHRFMT;
    }
}

/******************************************************************
 * SwXStyleFamilies
 ******************************************************************/
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXStyleFamilies::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXStyleFamilies");
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXStyleFamilies::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.style.StyleFamilies") == rServiceName;
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXStyleFamilies::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.style.StyleFamilies");
    return aRet;
}
/*-- 16.12.98 15:13:26---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyleFamilies::SwXStyleFamilies(SwDocShell& rDocShell) :
    SwUnoCollection(rDocShell.GetDoc()),
    pxCharStyles(0),
    pxParaStyles(0),
    pxFrameStyles(0),
    pxPageStyles(0),
    pxNumberingStyles(0),
    pDocShell(&rDocShell)
{

}
/*-- 16.12.98 15:13:26---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyleFamilies::~SwXStyleFamilies()
{
    delete pxCharStyles;
    delete pxParaStyles;
    delete pxFrameStyles;
    delete pxPageStyles;
    delete pxNumberingStyles;
}
/*-- 21.12.98 12:05:22---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SAL_CALL SwXStyleFamilies::getByName(const OUString& Name)
    throw(
        container::NoSuchElementException,
        lang::WrappedTargetException,
        RuntimeException )
{
    SolarMutexGuard aGuard;
// der Index kommt aus const unsigned short aStyleByIndex[] =
    Any aRet;
    if(!IsValid())
        throw RuntimeException();
    if(Name.compareToAscii("CharacterStyles") == 0 )
        aRet = getByIndex(0);
    else if(Name.compareToAscii("ParagraphStyles") == 0)
        aRet = getByIndex(1);
    else if(Name.compareToAscii("FrameStyles") == 0 )
        aRet = getByIndex(3);
    else if(Name.compareToAscii("PageStyles") == 0 )
        aRet = getByIndex(2);
    else if(Name.compareToAscii("NumberingStyles") == 0 )
        aRet = getByIndex(4);
    else
        throw container::NoSuchElementException();
    return aRet;
}
/*-- 21.12.98 12:05:22---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXStyleFamilies::getElementNames(void) throw( RuntimeException )
{
    Sequence< OUString > aNames(STYLE_FAMILY_COUNT);
    OUString* pNames = aNames.getArray();
    pNames[0] = C2U("CharacterStyles");
    pNames[1] = C2U("ParagraphStyles");
    pNames[2] = C2U("FrameStyles");
    pNames[3] = C2U("PageStyles");
    pNames[4] = C2U("NumberingStyles");
    return aNames;
}
/*-- 21.12.98 12:05:22---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyleFamilies::hasByName(const OUString& Name) throw( RuntimeException )
{
    if( Name.compareToAscii("CharacterStyles") == 0 ||
        Name.compareToAscii("ParagraphStyles") == 0 ||
        Name.compareToAscii("FrameStyles") == 0 ||
        Name.compareToAscii("PageStyles") == 0 ||
        Name.compareToAscii("NumberingStyles") == 0 )
        return sal_True;
    else
        return sal_False;
}
/*-- 16.12.98 15:13:27---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXStyleFamilies::getCount(void) throw( RuntimeException )
{
    return STYLE_FAMILY_COUNT;
}
/*-- 16.12.98 15:13:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyleFamilies::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Any aRet;
    if(nIndex >= STYLE_FAMILY_COUNT)
        throw lang::IndexOutOfBoundsException();
    if(IsValid())
    {
        Reference< container::XNameContainer >  aRef;
        sal_uInt16 nType = aStyleByIndex[nIndex];
        switch( nType )
        {
            case SFX_STYLE_FAMILY_CHAR:
            {
                if(!pxCharStyles)
                {
                    ((SwXStyleFamilies*)this)->pxCharStyles = new Reference< container::XNameContainer > ();
                    *pxCharStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxCharStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PARA:
            {
                if(!pxParaStyles)
                {
                    ((SwXStyleFamilies*)this)->pxParaStyles = new Reference< container::XNameContainer > ();
                    *pxParaStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxParaStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PAGE     :
            {
                if(!pxPageStyles)
                {
                    ((SwXStyleFamilies*)this)->pxPageStyles = new Reference< container::XNameContainer > ();
                    *pxPageStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxPageStyles;
            }
            break;
            case SFX_STYLE_FAMILY_FRAME    :
            {
                if(!pxFrameStyles)
                {
                    ((SwXStyleFamilies*)this)->pxFrameStyles = new Reference< container::XNameContainer > ();
                    *pxFrameStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxFrameStyles;
            }
            break;
            case SFX_STYLE_FAMILY_PSEUDO:
            {
                if(!pxNumberingStyles)
                {
                    ((SwXStyleFamilies*)this)->pxNumberingStyles = new Reference< container::XNameContainer > ();
                    *pxNumberingStyles = new SwXStyleFamily(pDocShell, nType);
                }
                aRef = *pxNumberingStyles;
            }
            break;
        }
        aRet.setValue(&aRef, ::getCppuType((const Reference<container::XNameContainer>*)0));
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 16.12.98 15:13:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Type SwXStyleFamilies::getElementType(void)
    throw( RuntimeException )
{
    return ::getCppuType((const Reference<container::XNameContainer>*)0);

}
/*-- 16.12.98 15:13:28---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyleFamilies::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}
/*-- 16.12.98 15:13:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamilies::loadStylesFromURL(const OUString& rURL,
    const Sequence< PropertyValue >& aOptions)
    throw( io::IOException, RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool    bLoadStyleText = sal_True;
    sal_Bool    LoadStyleFrame = sal_True;
    sal_Bool    bLoadStylePage = sal_True;
    sal_Bool    bLoadStyleOverwrite = sal_True;
    sal_Bool    bLoadStyleNumbering = sal_True;
    sal_Bool    bLoadStyleFrame = sal_True;
    if(IsValid() && rURL.getLength())
    {
        const Any* pVal;
        int nCount = aOptions.getLength();
        const PropertyValue* pArray = aOptions.getConstArray();
        for(int i = 0; i < nCount; i++)
            if( ( pVal = &pArray[i].Value)->getValueType() ==
                    ::getBooleanCppuType() )
            {
                String sName = pArray[i].Name;
                sal_Bool bVal = *(sal_Bool*)pVal->getValue();
                if( sName.EqualsAscii(SW_PROP_NAME_STR(UNO_NAME_OVERWRITE_STYLES     )))
                    bLoadStyleOverwrite = bVal;
                else if( sName.EqualsAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_NUMBERING_STYLES )))
                    bLoadStyleNumbering = bVal;
                else if( sName.EqualsAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_PAGE_STYLES   )))
                    bLoadStylePage = bVal;
                else if( sName.EqualsAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_FRAME_STYLES     )))
                    bLoadStyleFrame = bVal;
                else if( sName.EqualsAscii(SW_PROP_NAME_STR(UNO_NAME_LOAD_TEXT_STYLES      )))
                    bLoadStyleText = bVal;
            }

        SwgReaderOption aOpt;
        aOpt.SetFrmFmts( bLoadStyleFrame );
        aOpt.SetTxtFmts( bLoadStyleText );
        aOpt.SetPageDescs( bLoadStylePage );
        aOpt.SetNumRules( bLoadStyleNumbering );
        aOpt.SetMerge( !bLoadStyleOverwrite );

        ULONG nErr = pDocShell->LoadStylesFromFile( rURL, aOpt, TRUE );
        if( nErr )
            throw io::IOException();
    }
    else
        throw RuntimeException();
}
/*-- 16.12.98 15:13:28---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< PropertyValue > SwXStyleFamilies::getStyleLoaderOptions(void)
        throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Sequence< PropertyValue > aSeq(5);
    PropertyValue* pArray = aSeq.getArray();
    Any aVal;
    sal_Bool bTemp = sal_True;
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[0] = PropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_LOAD_TEXT_STYLES)), -1, aVal, PropertyState_DIRECT_VALUE);
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[1] = PropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_LOAD_FRAME_STYLES)), -1, aVal, PropertyState_DIRECT_VALUE);
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[2] = PropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_LOAD_PAGE_STYLES)), -1, aVal, PropertyState_DIRECT_VALUE);
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[3] = PropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_LOAD_NUMBERING_STYLES)), -1, aVal, PropertyState_DIRECT_VALUE);
    aVal.setValue(&bTemp, ::getCppuBooleanType());
    pArray[4] = PropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_OVERWRITE_STYLES)), -1, aVal, PropertyState_DIRECT_VALUE);
    return aSeq;
}

/******************************************************************
 * SwXStyleFamily
 ******************************************************************/
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXStyleFamily::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXStyleFamily");
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXStyleFamily::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.style.StyleFamily") == rServiceName;
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXStyleFamily::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.style.StyleFamily");
    return aRet;
}
/*-- 16.12.98 16:03:56---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyleFamily::SwXStyleFamily(SwDocShell* pDocSh, sal_uInt16 nFamily) :
        pBasePool(pDocSh->GetStyleSheetPool()),
        pDocShell(pDocSh),
        eFamily((SfxStyleFamily)nFamily)
{
/*  switch( nFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_CHAR_STYLE);
        break;
        case SFX_STYLE_FAMILY_PARA:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARA_STYLE);
        break;
        case SFX_STYLE_FAMILY_PAGE:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PAGE_STYLE);
        break;
        case SFX_STYLE_FAMILY_FRAME:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_FRAME_STYLE);
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
            _pPropMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_NUM_STYLE);
        break;
    }*/
    StartListening(*pBasePool);
}
/*-- 16.12.98 16:03:56---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyleFamily::~SwXStyleFamily()
{

}
sal_Int32 lcl_GetCountOrName ( const SwDoc &rDoc, SfxStyleFamily eFamily, String *pString, sal_uInt16 nIndex = USHRT_MAX )
{
    sal_Int32 nCount = 0;
    switch( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            sal_uInt16 nBaseCount =  RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN  +
                                     RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN;
            nIndex -= nBaseCount;
            const sal_uInt16 nArrLen = rDoc.GetCharFmts()->Count();
            for( sal_uInt16 i = 0; i < nArrLen; i++ )
            {
                SwCharFmt* pFmt = (*rDoc.GetCharFmts())[ i ];
                if( pFmt->IsDefault() && pFmt != rDoc.GetDfltCharFmt() )
                    continue;
                if ( IsPoolUserFmt ( pFmt->GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        // the default character format needs to be set to "Default!"
                        if(rDoc.GetDfltCharFmt() == pFmt)
                            SwStyleNameMapper::FillUIName( 
                                RES_POOLCOLL_STANDARD, *pString );
                        else
                            *pString = pFmt->GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
        case SFX_STYLE_FAMILY_PARA:
        {
            sal_uInt16 nBaseCount = RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN +
                                    RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
                                    RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                    RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                    RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                    RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN;
            nIndex -= nBaseCount;
            const sal_uInt16 nArrLen = rDoc.GetTxtFmtColls()->Count();
            for ( sal_uInt16 i = 0; i < nArrLen; i++ )
            {
                SwTxtFmtColl * pColl = (*rDoc.GetTxtFmtColls())[i];
                if ( pColl->IsDefault() )
                    continue;
                if ( IsPoolUserFmt ( pColl->GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = pColl->GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
        case SFX_STYLE_FAMILY_FRAME:
        {
            sal_uInt16 nBaseCount = RES_POOLFRM_END - RES_POOLFRM_BEGIN;
            nIndex -= nBaseCount;
            const sal_uInt16 nArrLen = rDoc.GetFrmFmts()->Count();
            for( sal_uInt16 i = 0; i < nArrLen; i++ )
            {
                SwFrmFmt* pFmt = (*rDoc.GetFrmFmts())[ i ];
                if(pFmt->IsDefault() || pFmt->IsAuto())
                    continue;
                if ( IsPoolUserFmt ( pFmt->GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = pFmt->GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
        case SFX_STYLE_FAMILY_PAGE:
        {
            sal_uInt16 nBaseCount = RES_POOLPAGE_END - RES_POOLPAGE_BEGIN;
            nIndex -= nBaseCount;
            const sal_uInt16 nArrLen = rDoc.GetPageDescCnt();
            for(sal_uInt16 i = 0; i < nArrLen; ++i)
            {
                const SwPageDesc& rDesc = rDoc.GetPageDesc(i);

                /*if(rDesc.IsDefault() || rDesc.IsAuto())
                    continue;*/
                if ( IsPoolUserFmt ( rDesc.GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = rDesc.GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            sal_uInt16 nBaseCount = RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN;
            nIndex -= nBaseCount;
            const SwNumRuleTbl& rNumTbl = rDoc.GetNumRuleTbl();
            for(USHORT i = 0; i < rNumTbl.Count(); ++i)
            {
                const SwNumRule& rRule = *rNumTbl[ i ];
                if( rRule.IsAutoRule() )
                    continue;
                if ( IsPoolUserFmt ( rRule.GetPoolFmtId() ) )
                {
                    if ( nIndex == nCount )
                    {
                        *pString = rRule.GetName();
                        break;
                    }
                    nCount++;
                }
            }
            nCount += nBaseCount;
        }
        break;
    }
    return nCount;
}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXStyleFamily::getCount(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return lcl_GetCountOrName ( *pDocShell->GetDoc(), eFamily, NULL );
}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyleFamily::getByIndex(sal_Int32 nTempIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Any aRet;
    if ( nTempIndex >= 0 && nTempIndex < USHRT_MAX )
    {
        sal_uInt16 nIndex = static_cast < sal_uInt16 > ( nTempIndex );
        if(pBasePool)
        {
            String sStyleName;
            switch( eFamily )
            {
                case SFX_STYLE_FAMILY_CHAR:
                {
                    if ( nIndex < ( RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCHR_NORMAL_BEGIN + nIndex, sStyleName );
                    else if ( nIndex < ( RES_POOLCHR_HTML_END - RES_POOLCHR_HTML_BEGIN  +
                                         RES_POOLCHR_NORMAL_END - RES_POOLCHR_NORMAL_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCHR_HTML_BEGIN
                                                        - RES_POOLCHR_NORMAL_END + RES_POOLCHR_NORMAL_BEGIN
                                                        + nIndex, sStyleName );
                }
                break;
                case SFX_STYLE_FAMILY_PARA:
                {
                    if ( nIndex < ( RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCOLL_TEXT_BEGIN + nIndex, sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCOLL_LISTS_BEGIN
                                                        - RES_POOLCOLL_TEXT_END + RES_POOLCOLL_TEXT_BEGIN
                                                        + nIndex, sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                         RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCOLL_EXTRA_BEGIN
                                                         - RES_POOLCOLL_LISTS_END + RES_POOLCOLL_LISTS_BEGIN
                                                         - RES_POOLCOLL_TEXT_END  + RES_POOLCOLL_TEXT_BEGIN
                                                         + nIndex, sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                         RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                         RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCOLL_REGISTER_BEGIN
                                                         - RES_POOLCOLL_EXTRA_END + RES_POOLCOLL_EXTRA_BEGIN
                                                         - RES_POOLCOLL_LISTS_END + RES_POOLCOLL_LISTS_BEGIN
                                                         - RES_POOLCOLL_TEXT_END  + RES_POOLCOLL_TEXT_BEGIN
                                                         + nIndex, sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
                                         RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                         RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                         RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCOLL_DOC_BEGIN
                                                         - RES_POOLCOLL_REGISTER_END + RES_POOLCOLL_REGISTER_BEGIN
                                                         - RES_POOLCOLL_EXTRA_END + RES_POOLCOLL_EXTRA_BEGIN
                                                         - RES_POOLCOLL_LISTS_END + RES_POOLCOLL_LISTS_BEGIN
                                                         - RES_POOLCOLL_TEXT_END  + RES_POOLCOLL_TEXT_BEGIN
                                                         + nIndex, sStyleName );
                    else if ( nIndex < ( RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN +
                                         RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN +
                                         RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN +
                                         RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN +
                                         RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN +
                                         RES_POOLCOLL_TEXT_END  - RES_POOLCOLL_TEXT_BEGIN ) )
                        SwStyleNameMapper::FillUIName ( RES_POOLCOLL_HTML_BEGIN
                                                         - RES_POOLCOLL_DOC_END + RES_POOLCOLL_DOC_BEGIN
                                                         - RES_POOLCOLL_REGISTER_END + RES_POOLCOLL_REGISTER_BEGIN
                                                         - RES_POOLCOLL_EXTRA_END + RES_POOLCOLL_EXTRA_BEGIN
                                                         - RES_POOLCOLL_LISTS_END + RES_POOLCOLL_LISTS_BEGIN
                                                         - RES_POOLCOLL_TEXT_END  + RES_POOLCOLL_TEXT_BEGIN
                                                         + nIndex, sStyleName );
                }
                break;
                case SFX_STYLE_FAMILY_FRAME:
                {
                    if ( nIndex < ( RES_POOLFRM_END - RES_POOLFRM_BEGIN ) )
                    {
                        SwStyleNameMapper::FillUIName ( RES_POOLFRM_BEGIN + nIndex, sStyleName );
                    }
                }
                break;
                case SFX_STYLE_FAMILY_PAGE:
                {
                    if ( nIndex < ( RES_POOLPAGE_END - RES_POOLPAGE_BEGIN ) )
                    {
                        SwStyleNameMapper::FillUIName ( RES_POOLPAGE_BEGIN + nIndex, sStyleName );
                    }
                }
                break;
                case SFX_STYLE_FAMILY_PSEUDO:
                {
                    if ( nIndex < ( RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN ) )
                    {
                        SwStyleNameMapper::FillUIName ( RES_POOLNUMRULE_BEGIN + nIndex, sStyleName );
                    }
                }
                break;
            }
            if ( !sStyleName.Len() )
                lcl_GetCountOrName ( *pDocShell->GetDoc(), eFamily, &sStyleName, nIndex );

            if ( sStyleName.Len() )
            {
                SfxStyleSheetBase* pBase = pBasePool->Find( sStyleName, eFamily );
                if(pBase)
                {
                    Reference< style::XStyle >  xStyle = _FindStyle(sStyleName);
                    if(!xStyle.is())
                    {
                        xStyle = eFamily == SFX_STYLE_FAMILY_PAGE ?
                            new SwXPageStyle(*pBasePool, pDocShell, eFamily, sStyleName) :
                                eFamily == SFX_STYLE_FAMILY_FRAME ?
                                new SwXFrameStyle(*pBasePool, pDocShell->GetDoc(), pBase->GetName()):
                                    new SwXStyle(*pBasePool, eFamily, pDocShell->GetDoc(), sStyleName);
                    }
                    aRet.setValue(&xStyle, ::getCppuType((Reference<style::XStyle>*)0));
                }
                else
                    throw container::NoSuchElementException();
            }
            else
                throw lang::IndexOutOfBoundsException();
        }
        else
            throw RuntimeException();
    }
    else
        throw lang::IndexOutOfBoundsException();

    return aRet;
}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyleFamily::getByName(const OUString& rName)
    throw( container::NoSuchElementException, lang::WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Any aRet;
    String sStyleName;
    SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True );
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
        {
            Reference< style::XStyle >  xStyle = _FindStyle(sStyleName);
            if(!xStyle.is())
            {
                xStyle = eFamily == SFX_STYLE_FAMILY_PAGE ?
                    new SwXPageStyle(*pBasePool, pDocShell, eFamily, sStyleName) :
                        eFamily == SFX_STYLE_FAMILY_FRAME ?
                        new SwXFrameStyle(*pBasePool, pDocShell->GetDoc(), pBase->GetName()):
                            new SwXStyle(*pBasePool, eFamily, pDocShell->GetDoc(), sStyleName);
            }
            aRet.setValue(&xStyle, ::getCppuType((Reference<style::XStyle>*)0));
        }
        else
            throw container::NoSuchElementException();
    }
    else
        throw RuntimeException();
    return aRet;

}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< OUString > SwXStyleFamily::getElementNames(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    Sequence< OUString > aRet;
    if(pBasePool)
    {
        SfxStyleSheetIterator* pIterator = pBasePool->CreateIterator(eFamily, 0xffff);
        sal_uInt16 nCount = pIterator->Count();
        aRet.realloc(nCount);
        OUString* pArray = aRet.getArray();
        String aString;
        for(sal_uInt16 i = 0; i < nCount; i++)
        {
            SwStyleNameMapper::FillProgName((*pIterator)[i]->GetName(), aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True );
            pArray[i] = OUString ( aString );
        }
        delete pIterator;
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 16.12.98 16:03:57---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyleFamily::hasByName(const OUString& rName) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        String sStyleName;
        SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True );
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        bRet = 0 != pBase;
    }
    else
        throw RuntimeException();
    return bRet;

}
/*-- 16.12.98 16:03:58---------------------------------------------------

  -----------------------------------------------------------------------*/
Type SwXStyleFamily::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((const Reference<style::XStyle>*)0);

}
/*-- 16.12.98 16:03:58---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyleFamily::hasElements(void) throw( RuntimeException )
{
    if(!pBasePool)
        throw RuntimeException();
    return sal_True;
}
/*-- 16.12.98 16:03:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamily::insertByName(const OUString& rName, const Any& rElement)
        throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        String sStyleName;
        SwStyleNameMapper::FillUIName(rName, sStyleName, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True);
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        SfxStyleSheetBase* pUINameBase = pBasePool->Find( sStyleName );
        if(pBase || pUINameBase)
            throw container::ElementExistException();
        else
        {
            if(rElement.getValueType().getTypeClass() ==
                                            TypeClass_INTERFACE)
            {
                Reference< XInterface > * pxRef =
                    (Reference< XInterface > *)rElement.getValue();

                Reference<lang::XUnoTunnel> xStyleTunnel( *pxRef, UNO_QUERY);

                SwXStyle* pNewStyle = 0;
                if(xStyleTunnel.is())
                {
                    pNewStyle = (SwXStyle*)xStyleTunnel->getSomething(
                                            SwXStyle::getUnoTunnelId());
                }
                if(!pNewStyle || !pNewStyle->IsDescriptor() ||
                    pNewStyle->GetFamily() != eFamily)
                        throw lang::IllegalArgumentException();
                if(pNewStyle)
                {
                    sal_uInt16 nMask = 0xffff;
                    if(eFamily == SFX_STYLE_FAMILY_PARA && !pNewStyle->IsConditional())
                        nMask &= ~SWSTYLEBIT_CONDCOLL;
                    SfxStyleSheetBase& rNewBase = pBasePool->Make(sStyleName, eFamily, nMask);
                    pNewStyle->SetDoc(pDocShell->GetDoc(), pBasePool);
                    pNewStyle->SetStyleName(sStyleName);
                    String sParentStyleName(pNewStyle->GetParentStyleName());
                    if(sParentStyleName.Len())
                    {
                        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
                        SfxStyleSheetBase* pParentBase = pBasePool->Find(sParentStyleName);
                        if(pParentBase && pParentBase->GetFamily() == eFamily &&
                            &pParentBase->GetPool() == pBasePool)
                            pBasePool->SetParent( eFamily, sStyleName,  sParentStyleName );

                    }
                    //so, jetzt sollten noch die Properties des Descriptors angewandt werden
                    pNewStyle->ApplyDescriptorProperties();
                }
                else
                    throw lang::IllegalArgumentException();
            }
            else
                throw lang::IllegalArgumentException();
        }
    }
    else
        throw RuntimeException();
}
/*-- 16.12.98 16:03:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamily::replaceByName(const OUString& rName, const Any& rElement)
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(rName);
        //Ersetzung geht nur fuer benutzerdefinierte Styles
        if(!pBase)
            throw container::NoSuchElementException();
        if(!pBase->IsUserDefined())
            throw lang::IllegalArgumentException();
        //if theres an object available to this style then it must be invalidated
        Reference< style::XStyle >  xStyle = _FindStyle(pBase->GetName());
        if(xStyle.is())
        {
            Reference<XUnoTunnel> xTunnel( xStyle, UNO_QUERY);
            if(xTunnel.is())
            {
                SwXStyle* pStyle = (SwXStyle*)xTunnel->getSomething(
                                                    SwXStyle::getUnoTunnelId());
                pStyle->Invalidate();
            }
        }

        pBasePool->Erase(pBase);
        insertByName(rName, rElement);
    }
    else
        throw RuntimeException();
}
/*-- 16.12.98 16:03:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamily::removeByName(const OUString& rName) throw( container::NoSuchElementException, lang::WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        String aString;
        SwStyleNameMapper::FillUIName(rName, aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True );

        SfxStyleSheetBase* pBase = pBasePool->Find( aString );
        if(pBase)
            pBasePool->Erase(pBase);
        else
            throw container::NoSuchElementException();
    }
    else
        throw RuntimeException();
}
/*-- 16.12.98 16:03:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyleFamily::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint *pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint && ( pHint->GetId() & SFX_HINT_DYING ) )
    {
        pBasePool = 0;
        pDocShell = 0;
        EndListening(rBC);
    }
}
/*-- 16.12.98 16:03:59---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyle*   SwXStyleFamily::_FindStyle(const String& rStyleName)const
{
    sal_uInt16  nLCount = pBasePool->GetListenerCount();
    SfxListener* pListener = 0;
    for( sal_uInt16 i = 0; i < nLCount; i++)
    {
        pListener = pBasePool->GetListener( i );
        if(PTR_CAST(SwXStyle, pListener) && ((SwXStyle*)pListener)->GetFamily() == eFamily && ((SwXStyle*)pListener)->GetStyleName() == rStyleName)
        {
            return (SwXStyle*)pListener;
        }
    }
    return 0;
}
/******************************************************************
 *
 ******************************************************************/
class SwStyleProperties_Impl
{
    const SfxItemPropertyMap*   _pMap;
    Any**                   	pAnyArr;
    sal_uInt16                  nArrLen;

public:
    SwStyleProperties_Impl(const SfxItemPropertyMap* _pMap);
    ~SwStyleProperties_Impl();

    sal_Bool    SetProperty(const String& rName, Any aVal);
    sal_Bool    GetProperty(const String& rName, Any*& rpAny);
    sal_Bool    ClearProperty( const OUString& rPropertyName );
    void    ClearAllProperties( );
    void       	GetProperty(const OUString &rPropertyName, const Reference < XPropertySet > &rxPropertySet, uno::Any& rAny );


    const SfxItemPropertyMap*   GetPropertyMap() const {return _pMap;}
};
//--------------------------------------------------------------------
//--------------------------------------------------------------------
SwStyleProperties_Impl::SwStyleProperties_Impl(const SfxItemPropertyMap* pMap) :
    _pMap(pMap),
    nArrLen(0)
{
    const SfxItemPropertyMap* pTmp = _pMap;
    while(pTmp[nArrLen].nWID)
        nArrLen++;
    pAnyArr = new Any* [nArrLen];
    for ( sal_uInt16 i =0 ; i < nArrLen; i++ )
        pAnyArr[i] = 0;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
SwStyleProperties_Impl::~SwStyleProperties_Impl()
{
    for ( sal_uInt16 i =0 ; i < nArrLen; i++ )
        delete pAnyArr[i];
    delete[] pAnyArr;
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
sal_Bool SwStyleProperties_Impl::SetProperty(const String& rName, Any aVal)
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = _pMap;
    while( pTemp->pName )
    {
        if(rName.EqualsAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
    {
        delete pAnyArr[nPos];
        pAnyArr[nPos] = new Any ( aVal );
    }
    return nPos < nArrLen;
}

sal_Bool SwStyleProperties_Impl::ClearProperty( const OUString& rName )
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = _pMap;
    while( pTemp->pName )
    {
        if( rName.equalsAsciiL ( pTemp->pName, pTemp->nNameLen ) )
            break;
        ++nPos;
        ++pTemp;
    }
    if( nPos < nArrLen )
    {
        delete pAnyArr[nPos];
        pAnyArr[ nPos ] = 0;
    }
    return nPos < nArrLen;
}
void SwStyleProperties_Impl::ClearAllProperties( )
{
    for ( sal_uInt16 i = 0; i < nArrLen; i++ )
    {
        delete pAnyArr[i];
        pAnyArr[ i ] = 0;
    }
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
sal_Bool SwStyleProperties_Impl::GetProperty(const String& rName, Any*& rpAny )
{
    sal_uInt16 nPos = 0;
    const SfxItemPropertyMap* pTemp = _pMap;
    while( pTemp->pName )
    {
        if(rName.EqualsAscii(pTemp->pName))
            break;
        ++nPos;
        ++pTemp;
    }
    if(nPos < nArrLen)
        rpAny = pAnyArr[nPos];
    return nPos < nArrLen;
}

void SwStyleProperties_Impl::GetProperty( const OUString &rPropertyName, const Reference < XPropertySet > &rxPropertySet, uno::Any & rAny )
{
    rAny = rxPropertySet->getPropertyValue( rPropertyName );
}

/******************************************************************
 *
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const Sequence< sal_Int8 > & SwXStyle::getUnoTunnelId()
{
    static Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXStyle::getSomething( const Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

TYPEINIT1(SwXStyle, SfxListener);
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXStyle::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXStyle");
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXStyle::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    BOOL bRet = C2U("com.sun.star.style.Style") == rServiceName;
    if(!bRet && SFX_STYLE_FAMILY_CHAR == eFamily)
        bRet = !rServiceName.compareToAscii("com.sun.star.style.CharacterProperties")||
               !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesAsian")||
               !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesComplex");
    if(!bRet && SFX_STYLE_FAMILY_PARA == eFamily)
        bRet = (C2U("com.sun.star.style.ParagraphStyle") == rServiceName)||
               (C2U("com.sun.star.style.ParagraphProperties") == rServiceName) ||
               (C2U("com.sun.star.style.ParagraphPropertiesAsian") == rServiceName) ||
               (C2U("com.sun.star.style.ParagraphPropertiesComplex") == rServiceName);
    if(!bRet && SFX_STYLE_FAMILY_PAGE == eFamily)
        bRet = (C2U("com.sun.star.style.PageProperties") == rServiceName);

    return  bRet;
}
/* -----------------------------06.04.00 11:24--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXStyle::getSupportedServiceNames(void) throw( RuntimeException )
{
    long nCount = 1;
    if(SFX_STYLE_FAMILY_PARA == eFamily)
    {
        nCount = 5;
        if(bIsConditional)
            nCount++;
    }
    else if(SFX_STYLE_FAMILY_CHAR == eFamily)
        nCount = 4;
    else if(SFX_STYLE_FAMILY_PAGE == eFamily)
        nCount = 2;
    Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.style.Style");
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            pArray[1] = C2U("com.sun.star.style.CharacterProperties");
            pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
            pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
        break;
        case SFX_STYLE_FAMILY_PAGE:
            pArray[1] = C2U("com.sun.star.style.PageProperties");
        break;
        case SFX_STYLE_FAMILY_PARA:
            pArray[1] = C2U("com.sun.star.style.ParagraphStyle");
            pArray[2] = C2U("com.sun.star.style.ParagraphProperties");
            pArray[3] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
            pArray[4] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
        if(bIsConditional)
            pArray[5] = C2U("com.sun.star.style.ConditionalParagraphStyle");
        break;
    }
    return aRet;
}
/*-- 17.12.98 08:26:49---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyle::SwXStyle( SwDoc *pDoc, SfxStyleFamily eFam, BOOL bConditional) :
    m_pDoc( pDoc ),
    bIsDescriptor(sal_True),
    bIsConditional(bConditional),
    eFamily(eFam),
    pBasePool(0)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    // get the property set for the default style data
    // First get the model
    Reference < XModel > xModel = pDoc->GetDocShell()->GetBaseModel();
    // Ask the model for it's family supplier interface
    Reference < XStyleFamiliesSupplier > xFamilySupplier ( xModel, UNO_QUERY );
    // Get the style families
    Reference < XNameAccess > xFamilies = xFamilySupplier->getStyleFamilies();

    Any aAny;
    sal_uInt16 nMapId;
    switch( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            nMapId = PROPERTY_MAP_CHAR_STYLE;
        }
        break;
        case SFX_STYLE_FAMILY_PARA:
        {
            nMapId = PROPERTY_MAP_PARA_STYLE;
            aAny = xFamilies->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ParagraphStyles" ) ) );
            // Get the Frame family (and keep it for later)
            aAny >>= mxStyleFamily;
            aAny = mxStyleFamily->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Standard" ) ) );
            aAny >>= mxStyleData;
        }
        break;
        case SFX_STYLE_FAMILY_PAGE:
        {
            nMapId = PROPERTY_MAP_PAGE_STYLE;
            aAny = xFamilies->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "PageStyles" ) ) );
            // Get the Frame family (and keep it for later)
            aAny >>= mxStyleFamily;
            aAny = mxStyleFamily->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Standard" ) ) );
            aAny >>= mxStyleData;
        }
        break;
        case SFX_STYLE_FAMILY_FRAME :
        {
            nMapId = PROPERTY_MAP_FRAME_STYLE;
        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            nMapId = PROPERTY_MAP_NUM_STYLE;
        }
        break;
    }
    pPropImpl = new SwStyleProperties_Impl(aSwMapProvider.GetPropertyMap(nMapId));
}


SwXStyle::SwXStyle(SfxStyleSheetBasePool& rPool, SfxStyleFamily eFam,
        SwDoc*  pDoc,   const String& rStyleName) :
    sStyleName(rStyleName),
    pBasePool(&rPool),
    eFamily(eFam),
    m_pDoc(pDoc),
    bIsDescriptor(sal_False),
    bIsConditional(sal_False),
    pPropImpl(0)
{
    StartListening(rPool);
    if(eFam == SFX_STYLE_FAMILY_PARA)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "where is the style?" );
        if(pBase)
        {
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(sStyleName, GET_POOLID_TXTCOLL);
            if(nId != USHRT_MAX)
                ::binfilter::IsConditionalByPoolId( nId );
            else
                bIsConditional = RES_CONDTXTFMTCOLL == ((SwDocStyleSheet*)pBase)->GetCollection()->Which();
        }
    }
}
/*-- 17.12.98 08:26:50---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXStyle::~SwXStyle()
{
    if(pBasePool)
        EndListening(*pBasePool);
    delete pPropImpl;
}
/*-- 17.12.98 08:26:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        m_pDoc = 0;
        mxStyleData.clear();
        mxStyleFamily.clear();
    }
}
OUString SwXStyle::getName(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    String aString;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "where is the style?" );
        if(!pBase)
            throw RuntimeException();
        SwStyleNameMapper::FillProgName(pBase->GetName(), aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True);
    }
    else
        aString = sStyleName;
    return OUString (aString);
}
/*-- 17.12.98 08:26:51---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::setName(const OUString& rName) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "where is the style?" );
        sal_Bool bExcept = sal_True;
        if(pBase && pBase->IsUserDefined())
        {
            SwDocStyleSheet aTmp( *(SwDocStyleSheet*)pBase );
            bExcept = !aTmp.SetName(rName);
            if(!bExcept)
                sStyleName = String(rName);
        }
        if(bExcept)
            throw RuntimeException();
    }
    else
        sStyleName = String(rName);
}
/*-- 17.12.98 08:26:51---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyle::isUserDefined(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        //if it is not found it must be non user defined
        if(pBase)
            bRet = pBase->IsUserDefined();
    }
    else
        throw RuntimeException();
    return bRet;
}
/*-- 17.12.98 08:26:51---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXStyle::isInUse(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_USED);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
            bRet = pBase->IsUsed();
    }
    else
        throw RuntimeException();
    return bRet;
}
/*-- 17.12.98 08:26:52---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXStyle::getParentStyle(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    String aString;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
            aString = pBase->GetParent();
    }
    else if(bIsDescriptor)
        aString = sParentStyleName;
    else
        throw RuntimeException();
    SwStyleNameMapper::FillProgName(aString, aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True );
    return OUString ( aString );
}
/*-- 17.12.98 08:26:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::setParentStyle(const OUString& rParentStyle)
            throw( container::NoSuchElementException, RuntimeException )
{
    SolarMutexGuard aGuard;
    String sParentStyle;
    SwStyleNameMapper::FillUIName(rParentStyle, sParentStyle, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True );
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        BOOL bExcept = FALSE;
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        if(pBase)
        {
            SwDocStyleSheet aBase(*(SwDocStyleSheet*)pBase);
            if(aBase.GetParent() != sParentStyle)
            {
                bExcept = !aBase.SetParent(sParentStyle);
            }
        }
        else
            bExcept = TRUE;
        if(bExcept)
            throw RuntimeException();
    }
    else if(bIsDescriptor)
    {
        sParentStyleName = String(sParentStyle);
        try
        {
            Any aAny = mxStyleFamily->getByName ( sParentStyle );
            aAny >>= mxStyleData;
        }
        catch ( container::NoSuchElementException& )
        {
        }
        catch ( WrappedTargetException& )
        {
        }
        catch ( RuntimeException& )
        {
        }
    }
    else
        throw RuntimeException();
}
/*-- 17.12.98 08:26:52---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo >  SwXStyle::getPropertySetInfo(void)
    throw( RuntimeException )
{
    Reference< XPropertySetInfo >  xRet;
    switch( eFamily )
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            static Reference< XPropertySetInfo >  xCharRef;
            if(!xCharRef.is())
            {
                SfxItemPropertySet aPropSet(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_CHAR_STYLE));
                xCharRef = aPropSet.getPropertySetInfo();
            }
            xRet = xCharRef;
        }
        break;
        case SFX_STYLE_FAMILY_PARA:
        {
            static Reference< XPropertySetInfo >  xParaRef;
            if(!xParaRef.is())
            {
                SfxItemPropertySet aPropSet(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARA_STYLE));
                xParaRef = aPropSet.getPropertySetInfo();
            }
            xRet = xParaRef;
        }
        break;
        case SFX_STYLE_FAMILY_PAGE     :
        {
            static Reference< XPropertySetInfo >  xPageRef;
            if(!xPageRef.is())
            {
                SfxItemPropertySet aPropSet(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PAGE_STYLE) );
                xPageRef = aPropSet.getPropertySetInfo();
            }
            xRet = xPageRef;
        }
        break;
        case SFX_STYLE_FAMILY_FRAME    :
        {
            static Reference< XPropertySetInfo >  xFrameRef;
            if(!xFrameRef.is())
            {
                SfxItemPropertySet aPropSet(
                aSwMapProvider.GetPropertyMap(PROPERTY_MAP_FRAME_STYLE));
                xFrameRef = aPropSet.getPropertySetInfo();
            }
            xRet = xFrameRef;
        }
        break;
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            static Reference< XPropertySetInfo >  xNumRef;
            if(!xNumRef.is())
            {
                SfxItemPropertySet aPropSet(
                    aSwMapProvider.GetPropertyMap(PROPERTY_MAP_NUM_STYLE));
                xNumRef = aPropSet.getPropertySetInfo();
            }
            xRet = xNumRef;
        }
        break;
    }
    return xRet;
}
/* -----------------23.04.99 13:28-------------------
 *
 * --------------------------------------------------*/
void    SwXStyle::ApplyDescriptorProperties()
{
    bIsDescriptor = sal_False;
    mxStyleData.clear();
    mxStyleFamily.clear();

    const SfxItemPropertyMap* pTemp = pPropImpl->GetPropertyMap();
    while(pTemp->nWID)
    {
        Any* pAny;
        String sPropName(C2S(pTemp->pName));
        OUString sUPropName(sPropName);
        pPropImpl->GetProperty(sPropName, pAny);
        if(pAny)
            setPropertyValue(sUPropName, *pAny);
        pTemp++;
    }
}

/*-- 18.04.01 13:07:27---------------------------------------------------

  -----------------------------------------------------------------------*/
struct SwStyleBase_Impl
{
    SwDoc&              rDoc;

    const SwPageDesc*   pOldPageDesc;

    SwDocStyleSheet*    pNewBase;
    SfxItemSet*         pItemSet;

    const String&       rStyleName;
    sal_uInt16              nPDescPos;

    SwStyleBase_Impl(SwDoc& rSwDoc, const String& rName) :
        rDoc(rSwDoc),
        rStyleName(rName),
        pNewBase(0),
        pItemSet(0),
        pOldPageDesc(0),
        nPDescPos(0xffff)
        {}
    ~SwStyleBase_Impl(){delete pNewBase; delete pItemSet; }

    sal_Bool HasItemSet() {return 0 != pNewBase;}
    SfxItemSet& GetItemSet()
        {
            DBG_ASSERT(pNewBase, "no SwDocStyleSheet available");
            if(!pItemSet)
                pItemSet = new SfxItemSet(pNewBase->GetItemSet());
            return *pItemSet;
        }

        const SwPageDesc& GetOldPageDesc();
};
/* -----------------------------25.04.01 12:44--------------------------------

 ---------------------------------------------------------------------------*/
const SwPageDesc& SwStyleBase_Impl::GetOldPageDesc()
{
    if(!pOldPageDesc)
    {
        sal_uInt16 i;
        sal_uInt16 nPDescCount = rDoc.GetPageDescCnt();
        for(i = 0; i < nPDescCount; i++)
        {
            const SwPageDesc& rDesc = rDoc.GetPageDesc( i );
            if(rDesc.GetName() == rStyleName)
            {
                pOldPageDesc = & rDesc;
                nPDescPos = i;
                break;
            }
        }
        if(!pOldPageDesc)
        {
            for(i = RC_POOLPAGEDESC_BEGIN; i <= STR_POOLPAGE_ENDNOTE; ++i)
            {
                const String aFmtName(SW_RES(i));
                if(aFmtName == rStyleName)
                {
                    pOldPageDesc = rDoc.GetPageDescFromPool( RES_POOLPAGE_BEGIN + i - RC_POOLPAGEDESC_BEGIN );
                    break;
                }
            }
            for(i = 0; i < nPDescCount + 1; i++)
            {
                const SwPageDesc& rDesc = rDoc.GetPageDesc( i );
                if(rDesc.GetName() == rStyleName)
                {
                    nPDescPos = i;
                    break;
                }
            }
        }
    }
    return *pOldPageDesc;
}

/* -----------------------------19.04.01 09:44--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_SetStyleProperty(const SfxItemPropertyMap* pMap,
                        SfxItemPropertySet& rPropSet,
                        const Any& rValue,
                        SwStyleBase_Impl& rBase,
                        SfxStyleSheetBasePool* pBasePool,
                        SwDoc* pDoc,
                        SfxStyleFamily eFamily)
                            throw(PropertyVetoException, IllegalArgumentException,
                                WrappedTargetException, RuntimeException)

{
    switch(pMap->nWID)
    {
        case RES_PAPER_BIN:
        {
            SfxPrinter *pPrinter = pDoc->GetPrt();
            OUString sTmp;
            sal_uInt16 nBin = USHRT_MAX;
            if ( !( rValue >>= sTmp ) )
                throw IllegalArgumentException();
            if ( sTmp.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "[From printer settings]" ) ) )
                nBin = USHRT_MAX-1;
            else if ( pPrinter )
            {
                for (sal_uInt16 i=0, nEnd = pPrinter->GetPaperBinCount(); i < nEnd; i++ )
                {
                    if (sTmp == OUString ( pPrinter->GetPaperBinName ( i ) ) )
                    {
                        nBin = i;
                        break;
                    }
                }
            }
            if ( nBin == USHRT_MAX )
                throw IllegalArgumentException();
            else
            {
                SfxItemSet& rStyleSet = rBase.GetItemSet();
                SfxItemSet aSet(*rStyleSet.GetPool(), pMap->nWID, pMap->nWID);
                aSet.SetParent(&rStyleSet);
                rPropSet.setPropertyValue(*pMap, makeAny ( static_cast < sal_Int8 > ( nBin == USHRT_MAX-1 ? -1 : nBin ) ), aSet);
                rStyleSet.Put(aSet);
            }
        }
        break;
        case  FN_UNO_NUM_RULES: //Sonderbehandlung fuer das SvxNumRuleItem:
        {
            if(rValue.getValueType() == ::getCppuType((Reference< container::XIndexReplace>*)0) )
            {
                Reference< container::XIndexReplace > * pxRulesRef =
                        (Reference< container::XIndexReplace > *)rValue.getValue();

                Reference<lang::XUnoTunnel> xNumberTunnel( *pxRulesRef, UNO_QUERY);

                SwXNumberingRules* pSwXRules = 0;
                if(xNumberTunnel.is())
                {
                    pSwXRules = (SwXNumberingRules*)
                        xNumberTunnel->getSomething(
                                            SwXNumberingRules::getUnoTunnelId());
                }
                if(pSwXRules)
                {
                    const String* pCharStyleNames = pSwXRules->GetNewCharStyleNames();
                    const String* pBulletFontNames = pSwXRules->GetBulletFontNames();

                    SwNumRule aSetRule(*pSwXRules->GetNumRule());
                    const SwCharFmts* pFmts = pDoc->GetCharFmts();
                    sal_uInt16 nChCount = pFmts->Count();
                    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
                    {

                        const SwNumFmt* pFmt = aSetRule.GetNumFmt( i );
                        if(pFmt)
                        {
                            SwNumFmt aFmt(*pFmt);
                            if(
                                pCharStyleNames[i] != SwXNumberingRules::GetInvalidStyle() &&
                                ((pCharStyleNames[i].Len() && !pFmt->GetCharFmt()) ||
                                pCharStyleNames[i].Len() &&
                                            pFmt->GetCharFmt()->GetName() != pCharStyleNames[i] ))
                            {

                                SwCharFmt* pCharFmt = 0;
                                if(pCharStyleNames[i].Len())
                                {
                                    for(sal_uInt16 j = 0; j< nChCount; j++)
                                    {
                                        SwCharFmt* pTmp = (*pFmts)[j];
                                        if(pTmp->GetName() == pCharStyleNames[i])
                                        {
                                            pCharFmt = pTmp;
                                            break;
                                        }
                                    }
                                    if(!pCharFmt)
                                    {

                                        SfxStyleSheetBase* pBase;
                                        pBase = ((SfxStyleSheetBasePool*)pBasePool)->Find(pCharStyleNames[i], SFX_STYLE_FAMILY_CHAR);
                                        if(!pBase)
                                            pBase = &pBasePool->Make(pCharStyleNames[i], SFX_STYLE_FAMILY_CHAR);
                                        pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();

                                    }

                                    aFmt.SetCharFmt( pCharFmt );
                                }
                            }
                            //jetzt nochmal fuer Fonts
                            if(pBulletFontNames[i] != SwXNumberingRules::GetInvalidStyle() &&
                                ((pBulletFontNames[i].Len() && !pFmt->GetBulletFont()) ||
                                pBulletFontNames[i].Len() &&
                                        pFmt->GetBulletFont()->GetName() != pBulletFontNames[i] ))
                            {
                                const SvxFontListItem* pFontListItem =
                                        (const SvxFontListItem* )pDoc->GetDocShell()
                                                            ->GetItem( SID_ATTR_CHAR_FONTLIST );
                                const FontList*  pList = pFontListItem->GetFontList();
                                FontInfo aInfo = pList->Get(
                                    pBulletFontNames[i],WEIGHT_NORMAL, ITALIC_NONE);
                                Font aFont(aInfo);
                                aFmt.SetBulletFont(&aFont);
                            }
                            aSetRule.Set( i, &aFmt );
                        }
                    }
                    rBase.pNewBase->SetNumRule(aSetRule);
                }
            }
            else
                throw lang::IllegalArgumentException();
        }
        break;
        case FN_UNO_FOLLOW_STYLE:
        {
            OUString sTmp;
            rValue >>= sTmp;
            String aString;
            SwStyleNameMapper::FillUIName(sTmp, aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True ) ;
            rBase.pNewBase->SetFollow( aString );
        }
        break;
        case RES_PAGEDESC :
        if( MID_PAGEDESC_PAGEDESCNAME != pMap->nMemberId)
            goto put_itemset;
        {
            // Sonderbehandlung RES_PAGEDESC
            if(rValue.getValueType() != ::getCppuType((const OUString*)0))
                throw lang::IllegalArgumentException();
            SfxItemSet& rStyleSet = rBase.GetItemSet();

            SwFmtPageDesc* pNewDesc = 0;
            const SfxPoolItem* pItem;
            if(SFX_ITEM_SET == rStyleSet.GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
            {
                pNewDesc = new SwFmtPageDesc(*((SwFmtPageDesc*)pItem));
            }
            if(!pNewDesc)
                pNewDesc = new SwFmtPageDesc();
            OUString uDescName;
            rValue >>= uDescName;
            String sDescName;
            SwStyleNameMapper::FillUIName(uDescName, sDescName, GET_POOLID_PAGEDESC, sal_True );
            if(!pNewDesc->GetPageDesc() || pNewDesc->GetPageDesc()->GetName() != sDescName)
            {
                sal_uInt16 nCount = pDoc->GetPageDescCnt();
                sal_Bool bPut = sal_False;
                if(sDescName.Len())
                {
                    SwPageDesc* pPageDesc = ::binfilter::GetPageDescByName_Impl(*pDoc, sDescName);
                    if(pPageDesc)
                    {
                        pPageDesc->Add( pNewDesc );
                        bPut = sal_True;
                    }
                    else
                    {
                        throw lang::IllegalArgumentException();
                    }
                }
                if(!bPut)
                {
                    rStyleSet.ClearItem(RES_BREAK);
                    rStyleSet.Put(SwFmtPageDesc());
                }
                else
                    rStyleSet.Put(*pNewDesc);
            }
            delete pNewDesc;
        }
        break;
        case FN_UNO_IS_AUTO_UPDATE:
        {
            BOOL bAuto = *(sal_Bool*)rValue.getValue();
            if(SFX_STYLE_FAMILY_PARA == eFamily)
                rBase.pNewBase->GetCollection()->SetAutoUpdateFmt(bAuto);
            else if(SFX_STYLE_FAMILY_FRAME == eFamily)
                rBase.pNewBase->GetFrmFmt()->SetAutoUpdateFmt(bAuto);
        }
        break;
        case FN_UNO_CATEGORY:
        {
            if(!rBase.pNewBase->IsUserDefined())
                throw lang::IllegalArgumentException();
            short nSet;
            rValue >>= nSet;

            sal_uInt16 nId;
            switch( nSet )
            {
                case ParagraphStyleCategory::TEXT:
                    nId = SWSTYLEBIT_TEXT;
                    break;
                case ParagraphStyleCategory::CHAPTER:
                    nId = SWSTYLEBIT_CHAPTER;
                    break;
                case ParagraphStyleCategory::LIST:
                    nId = SWSTYLEBIT_LIST;
                    break;
                case ParagraphStyleCategory::INDEX:
                    nId = SWSTYLEBIT_IDX;
                    break;
                case ParagraphStyleCategory::EXTRA:
                    nId = SWSTYLEBIT_EXTRA;
                    break;
                case ParagraphStyleCategory::HTML:
                    nId = SWSTYLEBIT_HTML;
                    break;
                default: throw lang::IllegalArgumentException();
            }
            rBase.pNewBase->SetMask( nId|SFXSTYLEBIT_USERDEF );
        }
        break;
        case SID_SWREGISTER_COLLECTION:
        {
            OUString sName;
            rValue >>= sName;
            SwRegisterItem aReg( sName.getLength() != 0);
            aReg.SetWhich(SID_SWREGISTER_MODE);
            rBase.GetItemSet().Put(aReg);
            String aString;
            SwStyleNameMapper::FillUIName(sName, aString, GET_POOLID_TXTCOLL, sal_True);

            rBase.GetItemSet().Put(SfxStringItem(SID_SWREGISTER_COLLECTION, aString ) );
        }
        break;
        case RES_TXTATR_CJK_RUBY:
            if(MID_RUBY_CHARSTYLE == pMap->nMemberId )
            {
                OUString sTmp;
                if(rValue >>= sTmp)
                {
                    SfxItemSet& rStyleSet = rBase.GetItemSet();
                    SwFmtRuby* pRuby = 0;
                    const SfxPoolItem* pItem;
                    if(SFX_ITEM_SET == rStyleSet.GetItemState( RES_TXTATR_CJK_RUBY, sal_True, &pItem ) )
                        pRuby = new SwFmtRuby(*((SwFmtRuby*)pItem));
                    if(!pRuby)
                        pRuby = new SwFmtRuby(aEmptyStr);
                    String sStyle;
                    SwStyleNameMapper::FillUIName(sTmp, sStyle, GET_POOLID_CHRFMT, sal_True );
                    pRuby->SetCharFmtName( sTmp );
                    pRuby->SetCharFmtId( 0 );
                    if(sTmp.getLength())
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName( sTmp, GET_POOLID_CHRFMT );
                        pRuby->SetCharFmtId(nId);
                    }
                    rStyleSet.Put(*pRuby);
                    delete pRuby;
                }
                else
                    throw lang::IllegalArgumentException();
            }
        goto put_itemset;
        break;
        case RES_PARATR_DROP:
        {
            if( MID_DROPCAP_CHAR_STYLE_NAME == pMap->nMemberId)
            {
                if(rValue.getValueType() == ::getCppuType((const OUString*)0))
                {
                    SfxItemSet& rStyleSet = rBase.GetItemSet();

                    SwFmtDrop* pDrop = 0;
                    const SfxPoolItem* pItem;
                    if(SFX_ITEM_SET == rStyleSet.GetItemState( RES_PARATR_DROP, sal_True, &pItem ) )
                        pDrop = new SwFmtDrop(*((SwFmtDrop*)pItem));
                    if(!pDrop)
                        pDrop = new SwFmtDrop();
                    OUString uStyle;
                    rValue >>= uStyle;
                    String sStyle;
                    SwStyleNameMapper::FillUIName(uStyle, sStyle, GET_POOLID_CHRFMT, sal_True );
                    SwDocStyleSheet* pStyle =
                        (SwDocStyleSheet*)pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle, SFX_STYLE_FAMILY_CHAR);
                    if(pStyle)
                        pDrop->SetCharFmt(pStyle->GetCharFmt());
                    else
                        throw lang::IllegalArgumentException();
                    rStyleSet.Put(*pDrop);
                    delete pDrop;
                }
                else
                    throw lang::IllegalArgumentException();
                break;
            }
        }
        //no break!
        default:
put_itemset:
        {
            SfxItemSet& rStyleSet = rBase.GetItemSet();
            SfxItemSet aSet(*rStyleSet.GetPool(), pMap->nWID, pMap->nWID);
            aSet.SetParent(&rStyleSet);
            rPropSet.setPropertyValue(*pMap, rValue, aSet);
            rStyleSet.Put(aSet);
        }
    }
}
/* -----------------------------18.04.01 13:29--------------------------------

 ---------------------------------------------------------------------------*/
void SwXStyle::setPropertyValues(
    const Sequence< OUString >& rPropertyNames,
    const Sequence< Any >& rValues )
        throw(PropertyVetoException, IllegalArgumentException,
                WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( !m_pDoc )
        throw RuntimeException();
    sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
        case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
    }
    SfxItemPropertySet& aPropSet = aSwMapProvider.GetPropertySet(nPropSetId);

    if(rPropertyNames.getLength() != rValues.getLength())
        throw IllegalArgumentException();

    const OUString* pNames = rPropertyNames.getConstArray();
    const Any* pValues = rValues.getConstArray();
    const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();

    SwStyleBase_Impl aBaseImpl(*m_pDoc, sStyleName);
    if(pBasePool)
    {
        sal_uInt16 nSaveMask = pBasePool->GetSearchMask();
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        pBasePool->SetSearchMask(eFamily, nSaveMask );
        DBG_ASSERT(pBase, "where is the style?" );
        if(pBase)
            aBaseImpl.pNewBase = new SwDocStyleSheet(*(SwDocStyleSheet*)pBase);
        else
            throw RuntimeException();
    }

    for(sal_Int16 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        pMap = SfxItemPropertyMap::GetByName( pMap, pNames[nProp]);

        if(!pMap)
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if ( pMap->nFlags & PropertyAttribute::READONLY)
            throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if(aBaseImpl.pNewBase)
        {
            lcl_SetStyleProperty(pMap, aPropSet, pValues[nProp], aBaseImpl,
                                 pBasePool, m_pDoc, eFamily);
        }
        else if(bIsDescriptor)
        {
            if(!pPropImpl->SetProperty(pNames[nProp], pValues[nProp]))
                throw lang::IllegalArgumentException();
        }
        else
            throw RuntimeException();
    }
    if(aBaseImpl.HasItemSet())
        aBaseImpl.pNewBase->SetItemSet(aBaseImpl.GetItemSet());
}

Any lcl_GetStyleProperty(const SfxItemPropertyMap* pMap,
                        SfxItemPropertySet& rPropSet,
                        SwStyleBase_Impl& rBase,
                        SfxStyleSheetBase* pBase,
                        SfxStyleFamily eFamily,
                        SwDoc *pDoc) throw(RuntimeException)
{
    Any aRet;
    if(FN_UNO_IS_PHYSICAL == pMap->nWID)
    {
        BOOL bPhys = pBase != 0;
        if(pBase)
        {
            bPhys = ((SwDocStyleSheet*)pBase)->IsPhysical();
            // The standard character format is not existing physically
            if( bPhys && SFX_STYLE_FAMILY_CHAR == eFamily &&
                ((SwDocStyleSheet*)pBase)->GetCharFmt() &&
                ((SwDocStyleSheet*)pBase)->GetCharFmt()->IsDefault() )
                bPhys = FALSE;
        }
        aRet.setValue(&bPhys, ::getBooleanCppuType());
    }
    else if(pBase)
    {
        if(!rBase.pNewBase)
            rBase.pNewBase = new SwDocStyleSheet( *(SwDocStyleSheet*)pBase );
        switch(pMap->nWID)
        {
            case RES_PAPER_BIN:
            {
                SfxItemSet& rSet = rBase.GetItemSet();
                aRet = rPropSet.getPropertyValue(*pMap, rSet);
                sal_Int8 nBin;
                aRet >>= nBin;
                if ( nBin == -1 )
                    aRet <<= OUString ( RTL_CONSTASCII_USTRINGPARAM ( "[From printer settings]" ) );
                else
                {
                    SfxPrinter *pPrinter = pDoc->GetPrt();
                    OUString sTmp;
                    if (pPrinter )
                        sTmp = pPrinter->GetPaperBinName ( nBin );
                    aRet <<= sTmp;
                }
            }
            break;
            case  FN_UNO_NUM_RULES: //Sonderbehandlung fuer das SvxNumRuleItem:
            {
                const SwNumRule* pRule = rBase.pNewBase->GetNumRule();
                DBG_ASSERT(pRule, "Wo ist die NumRule?");
                Reference< container::XIndexReplace >  xRules = new SwXNumberingRules(*pRule);
                aRet.setValue(&xRules, ::getCppuType((Reference<container::XIndexReplace>*)0));
            }
            break;
            case FN_UNO_FOLLOW_STYLE:
            {
                String aString;
                SwStyleNameMapper::FillProgName(rBase.pNewBase->GetFollow(), aString, lcl_GetSwEnumFromSfxEnum ( eFamily ), sal_True);
                aRet <<= OUString( aString );
            }
            break;
            case RES_PAGEDESC :
            if( MID_PAGEDESC_PAGEDESCNAME != pMap->nMemberId)
                goto query_itemset;
            {
                // Sonderbehandlung RES_PAGEDESC
                const SfxPoolItem* pItem;
                if(SFX_ITEM_SET == rBase.GetItemSet().GetItemState( RES_PAGEDESC, sal_True, &pItem ) )
                {
                    const SwPageDesc* pDesc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();
                    if(pDesc)
                    {
                        String aString;
                        SwStyleNameMapper::FillProgName(pDesc->GetName(), aString,  GET_POOLID_PAGEDESC, sal_True );
                        aRet <<= OUString( aString );
                    }
                }
            }
            break;
            case FN_UNO_IS_AUTO_UPDATE:
            {
                BOOL bAuto = FALSE;
                if(SFX_STYLE_FAMILY_PARA == eFamily)
                    bAuto = rBase.pNewBase->GetCollection()->IsAutoUpdateFmt();
                else if(SFX_STYLE_FAMILY_FRAME == eFamily)
                    bAuto = rBase.pNewBase->GetFrmFmt()->IsAutoUpdateFmt();
                aRet.setValue(&bAuto, ::getBooleanCppuType());
            }
            break;
            case FN_UNO_DISPLAY_NAME:
            {
                OUString sName(rBase.pNewBase->GetName());
                aRet <<= sName;
            }
            break;
            case FN_UNO_CATEGORY:
            {
                sal_uInt16 nPoolId = rBase.pNewBase->GetCollection()->GetPoolFmtId();
                short nRet = -1;
                switch ( COLL_GET_RANGE_BITS & nPoolId )
                {
                    case COLL_TEXT_BITS:
                        nRet = ParagraphStyleCategory::TEXT;
                        break;
                    case COLL_DOC_BITS:
                        nRet = ParagraphStyleCategory::CHAPTER;
                        break;
                    case COLL_LISTS_BITS:
                        nRet = ParagraphStyleCategory::LIST;
                        break;
                    case COLL_REGISTER_BITS:
                        nRet = ParagraphStyleCategory::INDEX;
                        break;
                    case COLL_EXTRA_BITS:
                        nRet = ParagraphStyleCategory::EXTRA;
                        break;
                    case COLL_HTML_BITS:
                        nRet = ParagraphStyleCategory::HTML;
                        break;
                }
                aRet <<= nRet;
            }
            break;
            case SID_SWREGISTER_COLLECTION:
            {
                const SwPageDesc *pPageDesc = rBase.pNewBase->GetPageDesc();
                const SwTxtFmtColl* pCol = 0;
                String aString;
                if( pPageDesc )
                    pCol = pPageDesc->GetRegisterFmtColl();
                if( pCol )
                    SwStyleNameMapper::FillProgName(
                                pCol->GetName(), aString, GET_POOLID_TXTCOLL, sal_True );
                aRet <<= OUString ( aString );
            }
            break;
            default:
query_itemset:
            {
                SfxItemSet& rSet = rBase.GetItemSet();
                aRet = rPropSet.getPropertyValue(*pMap, rSet);
            }
        }
    }
    else
        throw RuntimeException();
    return aRet;
}
/* -----------------------------19.04.01 09:26--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Any > SwXStyle::getPropertyValues(
    const Sequence< OUString >& rPropertyNames ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( !m_pDoc )
        throw RuntimeException();
    sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
        case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
    }
    SfxItemPropertySet& aPropSet = aSwMapProvider.GetPropertySet(nPropSetId);
    const OUString* pNames = rPropertyNames.getConstArray();
    Sequence< Any > aRet(rPropertyNames.getLength());
    Any* pRet = aRet.getArray();
    const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
    SwStyleBase_Impl aBase(*m_pDoc, sStyleName);
    SfxStyleSheetBase* pBase = 0;
    for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        pMap = SfxItemPropertyMap::GetByName( pMap, pNames[nProp]);
        if(!pMap)
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if(pBasePool)
        {
            if(!pBase)
            {
                sal_uInt16 nSaveMask = pBasePool->GetSearchMask();
                pBasePool->SetSearchMask(eFamily, SFXSTYLEBIT_ALL );
                pBase = pBasePool->Find(sStyleName);
                pBasePool->SetSearchMask(eFamily, nSaveMask );
            }
            pRet[nProp] = lcl_GetStyleProperty(pMap, aPropSet, aBase, pBase, eFamily, GetDoc() );
        }
        else if(bIsDescriptor)
        {
            Any *pAny = 0;
            pPropImpl->GetProperty ( pNames[nProp], pAny );
            if( !pAny )
            {
                sal_Bool bExcept = sal_False;
                switch( eFamily )
                {
                    case SFX_STYLE_FAMILY_PSEUDO:
                        bExcept = sal_True;
                    break;
                    case SFX_STYLE_FAMILY_PARA:
                    case SFX_STYLE_FAMILY_PAGE:
                        pPropImpl->GetProperty ( pNames[nProp], mxStyleData, pRet[ nProp ] );
                    break;
                    case SFX_STYLE_FAMILY_CHAR:
                    case SFX_STYLE_FAMILY_FRAME :
                    {
                        if (pMap->nWID >= POOLATTR_BEGIN && pMap->nWID < RES_UNKNOWNATR_END )
                        {
                            SwFmt * pFmt;
                            if ( eFamily == SFX_STYLE_FAMILY_CHAR )
                                pFmt = m_pDoc->GetDfltCharFmt();
                            else
                                pFmt = m_pDoc->GetDfltFrmFmt();
                            const SwAttrPool * pPool = pFmt->GetAttrSet().GetPool();
                            const SfxPoolItem & rItem = pPool->GetDefaultItem ( pMap->nWID );
                            rItem.QueryValue ( pRet[nProp], pMap->nMemberId );
                        }
                        else
                            bExcept = sal_True;
                    }
                    break;
                }
                if (bExcept )
                {
                    RuntimeException aExcept;
                    aExcept.Message = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "No default value for: " ) ) + pNames[nProp];
                    throw aExcept;
                }
            }
            else
                pRet [ nProp ] = *pAny;
        }
        else
            throw RuntimeException();
    }
    return aRet;
}
/*-- 18.04.01 13:07:29---------------------------------------------------
  -----------------------------------------------------------------------*/
void SwXStyle::addPropertiesChangeListener(
    const Sequence< OUString >& aPropertyNames,
    const Reference< XPropertiesChangeListener >& xListener )
        throw(RuntimeException)
{
}
/*-- 18.04.01 13:07:30---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::removePropertiesChangeListener(
    const Reference< XPropertiesChangeListener >& xListener )
        throw(RuntimeException)
{
}
/*-- 18.04.01 13:07:30---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::firePropertiesChangeEvent(
    const Sequence< OUString >& aPropertyNames,
    const Reference< XPropertiesChangeListener >& xListener )
        throw(RuntimeException)
{
}
/*-- 17.12.98 08:26:53---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::setPropertyValue(const OUString& rPropertyName, const Any& rValue)
    throw( UnknownPropertyException,
        PropertyVetoException,
        IllegalArgumentException,
        WrappedTargetException,
        RuntimeException)
{
    const Sequence<OUString> aProperties(&rPropertyName, 1);
    const Sequence<Any> aValues(&rValue, 1);
    setPropertyValues(aProperties, aValues);
}
/*-- 17.12.98 08:26:53---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyle::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    const Sequence<OUString> aProperties(&rPropertyName, 1);
    return getPropertyValues(aProperties).getConstArray()[0];

}
/*-- 17.12.98 08:26:53---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::addPropertyChangeListener(const OUString& PropertyName,
    const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 17.12.98 08:26:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::removePropertyChangeListener(const OUString& PropertyName,
    const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 17.12.98 08:26:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::addVetoableChangeListener(const OUString& PropertyName,
    const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 17.12.98 08:26:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::removeVetoableChangeListener(const OUString& PropertyName,
    const Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

/*-- 08.03.99 10:50:26---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyState SwXStyle::getPropertyState(const OUString& rPropertyName)
        throw( UnknownPropertyException, RuntimeException )
{
    SolarMutexGuard aGuard;

    Sequence< OUString > aNames(1);
    OUString* pNames = aNames.getArray();
    pNames[0] = rPropertyName;
    Sequence< PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}
/*-- 08.03.99 10:50:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< PropertyState > SwXStyle::getPropertyStates(
    const Sequence< OUString >& rPropertyNames)
        throw( UnknownPropertyException, RuntimeException )
{
    SolarMutexGuard aGuard;
    Sequence< PropertyState > aRet(rPropertyNames.getLength());
    PropertyState* pStates = aRet.getArray();
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily );
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "where is the style?" );

        if(pBase)
        {
            const OUString* pNames = rPropertyNames.getConstArray();
            SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
            sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
            switch(eFamily)
            {
                case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
                case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
                case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE;   break;
                case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
            }

            SfxItemSet aSet = aStyle.GetItemSet();
            SfxItemPropertySet& rStylePropSet = aSwMapProvider.GetPropertySet(nPropSetId);
            for(sal_Int32 i = 0; i < rPropertyNames.getLength(); i++)
            {
                const String& rPropName = pNames[i];
                const SfxItemPropertyMap*   pMap =
                    SfxItemPropertyMap::GetByName(
                            rStylePropSet.getPropertyMap(), rPropName);
                if(!pMap)
                    throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropName, static_cast < cppu::OWeakObject * > ( this ) );
                if( FN_UNO_NUM_RULES ==  pMap->nWID ||
                    FN_UNO_FOLLOW_STYLE == pMap->nWID )
                {
                    pStates[i] = PropertyState_DIRECT_VALUE;
                }
                else if(SFX_STYLE_FAMILY_PAGE == eFamily &&
                        (rPropName.EqualsAscii("Header", 0, 6)
                            || rPropName.EqualsAscii("Footer", 0, 6)))
                {
                    sal_uInt16 nResId = lcl_ConvertFNToRES(pMap->nWID);
                    BOOL bFooter = rPropName.EqualsAscii("Footer", 0, 6);
                    const SvxSetItem* pSetItem;
                    if(SFX_ITEM_SET == aSet.GetItemState(
                            bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                            sal_False, (const SfxPoolItem**)&pSetItem))
                    {
                        const SfxItemSet& rSet = pSetItem->GetItemSet();
                        SfxItemState eState = rSet.GetItemState(nResId, sal_False);
                        if(SFX_ITEM_SET == eState)
                            pStates[i] = PropertyState_DIRECT_VALUE;
                        else
                            pStates[i] = PropertyState_DEFAULT_VALUE;
                    }
                    else
                        pStates[i] = PropertyState_AMBIGUOUS_VALUE;
                }
                else
                {
                    pStates[i] = rStylePropSet.getPropertyState(*pMap, aSet);
                    if( SFX_STYLE_FAMILY_PAGE == eFamily &&
                        SID_ATTR_PAGE_SIZE == pMap->nWID &&
                        PropertyState_DIRECT_VALUE == pStates[i] )
                    {
                        const SvxSizeItem& rSize =
                            static_cast < const SvxSizeItem& >(
                                    aSet.Get(SID_ATTR_PAGE_SIZE) );
                        sal_uInt8 nMemberId = pMap->nMemberId & 0x7f;
                        if( ( LONG_MAX == rSize.GetSize().Width() &&
                              (MID_SIZE_WIDTH == nMemberId ||
                               MID_SIZE_SIZE == nMemberId ) ) ||
                            ( LONG_MAX == rSize.GetSize().Height() &&
                              MID_SIZE_HEIGHT == nMemberId ) )
                        {
                            pStates[i] = PropertyState_DEFAULT_VALUE;
                        }
                    }
                }
            }
        }
        else
            throw RuntimeException();
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 08.03.99 10:50:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXStyle::setPropertyToDefault(const OUString& rPropertyName)
        throw( UnknownPropertyException, RuntimeException )
{
    const Sequence < OUString > aSequence ( &rPropertyName, 1 );
    setPropertiesToDefault ( aSequence );
}

void SAL_CALL SwXStyle::setPropertiesToDefault( const Sequence< OUString >& aPropertyNames )
    throw (UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFmt *pTargetFmt = 0;

    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "Where is the style?");

        if(pBase)
        {
            SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
            switch(eFamily)
            {
                case SFX_STYLE_FAMILY_CHAR: pTargetFmt = aStyle.GetCharFmt(); break;
                case SFX_STYLE_FAMILY_PARA: pTargetFmt = aStyle.GetCollection(); break;
                case SFX_STYLE_FAMILY_FRAME: pTargetFmt = aStyle.GetFrmFmt(); break;
                case SFX_STYLE_FAMILY_PAGE:
                    {
                        sal_uInt16 nPgDscPos = USHRT_MAX;
                        SwPageDesc *pDesc = m_pDoc->FindPageDescByName( aStyle.GetPageDesc()->GetName(), &nPgDscPos );
                        if( pDesc )
                            pTargetFmt = &pDesc->GetMaster();
                    }
                    break;
                case SFX_STYLE_FAMILY_PSEUDO:
                    break;
            }
        }
    }
    sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
        case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
        case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
        case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
    }

    const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(nPropSetId);
    const OUString* pNames = aPropertyNames.getConstArray();

    if ( pTargetFmt )
    {
        for( sal_Int32 nProp = 0, nEnd = aPropertyNames.getLength(); nProp < nEnd; nProp++ )
        {
            pMap = SfxItemPropertyMap::GetByName( pMap, pNames[nProp] );
            if( !pMap )
                throw UnknownPropertyException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is unknown: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pMap->nWID == FN_UNO_FOLLOW_STYLE || pMap->nWID == FN_UNO_NUM_RULES )
                throw RuntimeException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Cannot reset: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pMap->nFlags & PropertyAttribute::READONLY )
                throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

            pTargetFmt->ResetAttr ( pMap->nWID );
        }
    }
    else if ( bIsDescriptor )
    {
        for( sal_Int32 nProp = 0, nEnd = aPropertyNames.getLength(); nProp < nEnd; nProp++ )
            pPropImpl->ClearProperty ( pNames[ nProp ] );
    }
}

void SAL_CALL SwXStyle::setAllPropertiesToDefault(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if(pBasePool)
    {
        pBasePool->SetSearchMask(eFamily);
        SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
        DBG_ASSERT(pBase, "where is the style, you fiend!?");

        if(pBase)
        {
            SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );

            SwFmt *pTargetFmt = 0;
            SwPageDesc *pTargetDesc = 0;
            sal_uInt16 nPgDscPos = USHRT_MAX;
            switch( eFamily )
            {
            case SFX_STYLE_FAMILY_CHAR :
                pTargetFmt = aStyle.GetCharFmt();
                break;
            case SFX_STYLE_FAMILY_PARA :
                pTargetFmt = aStyle.GetCollection();
                break;
            case SFX_STYLE_FAMILY_FRAME:
                pTargetFmt = aStyle.GetFrmFmt();
                break;
            case SFX_STYLE_FAMILY_PAGE:
                {
                    SwPageDesc *pDesc = m_pDoc->FindPageDescByName( aStyle.GetPageDesc()->GetName(), &nPgDscPos );
                    if( pDesc )
                    {
                        pTargetFmt = &pDesc->GetMaster();
                        pDesc->SetUseOn ( PD_ALL );
                    }
                }
                break;
            case SFX_STYLE_FAMILY_PSEUDO:
                break;
            }
            if( pTargetFmt )
            {
                if( USHRT_MAX != nPgDscPos )
                {
                    SwPageDesc& rPageDesc = m_pDoc->_GetPageDesc(nPgDscPos);
                    rPageDesc.ResetAllMasterAttr();

                    SvxLRSpaceItem aLR;
                    sal_Int32 nSize = GetMetricVal ( CM_1) * 2;
                    aLR.SetLeft ( nSize );
                    aLR.SetLeft ( nSize );
                    SvxULSpaceItem aUL;
                    aUL.SetUpper ( static_cast < sal_uInt16 > ( nSize ) );
                    aUL.SetLower ( static_cast < sal_uInt16 > ( nSize ) );
                    pTargetFmt->SetAttr ( aLR );
                    pTargetFmt->SetAttr ( aUL );

                    SwPageDesc* pStdPgDsc = m_pDoc->GetPageDescFromPool( RES_POOLPAGE_STANDARD );
                    SwFmtFrmSize aFrmSz( ATT_FIX_SIZE );
                    if( RES_POOLPAGE_STANDARD == rPageDesc.GetPoolFmtId() )
                    {
                        if( m_pDoc->GetPrt() )
                        {
                            const Size aPhysSize( SvxPaperInfo::GetPaperSize(
                                        static_cast<Printer*>( m_pDoc->GetPrt() )) );
                            aFrmSz.SetSize( aPhysSize );
                        }
                        else
                        {
                            aFrmSz.SetWidth( LONG_MAX );
                            aFrmSz.SetHeight( LONG_MAX );
                        }

                    }
                    else
                    {
                        aFrmSz = pStdPgDsc->GetMaster().GetFrmSize();
                    }
                    if( pStdPgDsc->GetLandscape() )
                    {
                        SwTwips nTmp = aFrmSz.GetHeight();
                        aFrmSz.SetHeight( aFrmSz.GetWidth() );
                        aFrmSz.SetWidth( nTmp );
                    }
                    pTargetFmt->SetAttr( aFrmSz );
                }
                else
                    pTargetFmt->ResetAllAttr();

                if( USHRT_MAX != nPgDscPos )
                    m_pDoc->ChgPageDesc( nPgDscPos, m_pDoc->GetPageDesc(nPgDscPos) );
            }

        }
        else
            throw RuntimeException();
    }
    else if ( bIsDescriptor )
        pPropImpl->ClearAllProperties();
    else
        throw RuntimeException();
}

Sequence< Any > SAL_CALL SwXStyle::getPropertyDefaults( const Sequence< OUString >& aPropertyNames )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = aPropertyNames.getLength();
    Sequence < Any > aRet ( nCount );
    if ( nCount )
    {
        if( pBasePool)
        {
            pBasePool->SetSearchMask(eFamily);
            SfxStyleSheetBase* pBase = pBasePool->Find(sStyleName);
            DBG_ASSERT(pBase, "Doesn't seem to be a style!");

            if(pBase)
            {
                SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
                sal_Int8 nPropSetId = PROPERTY_SET_CHAR_STYLE;
                switch(eFamily)
                {
                    case SFX_STYLE_FAMILY_PARA: nPropSetId = PROPERTY_SET_PARA_STYLE  ; break;
                    case SFX_STYLE_FAMILY_FRAME: nPropSetId = PROPERTY_SET_FRAME_STYLE ;break;
                    case SFX_STYLE_FAMILY_PAGE: nPropSetId = PROPERTY_SET_PAGE_STYLE  ;break;
                    case SFX_STYLE_FAMILY_PSEUDO: nPropSetId = PROPERTY_SET_NUM_STYLE   ;break;
                }

                const SfxItemSet &rSet = aStyle.GetItemSet(), *pParentSet = rSet.GetParent();
                const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(nPropSetId);
                const OUString *pNames = aPropertyNames.getConstArray();
                Any *pRet = aRet.getArray();
                for ( sal_Int32 i = 0 ; i < nCount; i++)
                {
                    pMap = SfxItemPropertyMap::GetByName( pMap, pNames[i]);

                    if ( !pMap )
                        throw UnknownPropertyException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[i], static_cast < cppu::OWeakObject * > ( this ) );

                    if( pParentSet )
                        pRet[i] = aSwMapProvider.GetPropertySet(nPropSetId).getPropertyValue(pNames[i], *pParentSet);
                    else if( pMap->nWID != rSet.GetPool()->GetSlotId(pMap->nWID) )
                    {
                        const SfxPoolItem& rItem = rSet.GetPool()->GetDefaultItem(pMap->nWID);
                        rItem.QueryValue(pRet[i], pMap->nMemberId);
                    }
                }
            }
            else
                throw RuntimeException();
        }
        else
            throw RuntimeException();
    }
    return aRet;
}
/*-- 08.03.99 10:50:27---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXStyle::getPropertyDefault(const OUString& rPropertyName)
    throw( UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    const Sequence < OUString > aSequence ( &rPropertyName, 1 );
    return getPropertyDefaults ( aSequence ).getConstArray()[0];
}
/* -----------------21.01.99 13:08-------------------
 *
 * --------------------------------------------------*/
void SwXStyle::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint *pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint )
    {
        if(( pHint->GetId() & SFX_HINT_DYING ) || ( pHint->GetId() & SFX_STYLESHEET_ERASED))
        {
            pBasePool = 0;
            EndListening(rBC);
        }
        else if( pHint->GetId() &(SFX_STYLESHEET_CHANGED|SFX_STYLESHEET_ERASED) )
        {
            ((SfxStyleSheetPool&)rBC).SetSearchMask(eFamily);
            SfxStyleSheetBase* pOwnBase = ((SfxStyleSheetPool&)rBC).Find(sStyleName);
            if(!pOwnBase)
            {
                EndListening(rBC);
                Invalidate();
            }
        }
    }
}
/* -----------------------------15.08.00 11:35--------------------------------

 ---------------------------------------------------------------------------*/
void SwXStyle::Invalidate()
{
    sStyleName.Erase();
    pBasePool = 0;
    m_pDoc = 0;
    mxStyleData.clear();
    mxStyleFamily.clear();
}


/******************************************************************
 * SwXPageStyle
 ******************************************************************/
/*-- 17.12.98 08:43:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXPageStyle::SwXPageStyle(SfxStyleSheetBasePool& rPool,
        SwDocShell* pDocSh, SfxStyleFamily eFam,
        const String& rStyleName)://, const SfxItemPropertyMap* _pMap) :
    SwXStyle(rPool, eFam, pDocSh->GetDoc(), rStyleName),//, _pMap),
    pDocShell(pDocSh)
{

}
/* -----------------23.08.99 15:52-------------------

 --------------------------------------------------*/
SwXPageStyle::SwXPageStyle(SwDocShell* pDocSh) :
    SwXStyle(pDocSh->GetDoc(), SFX_STYLE_FAMILY_PAGE),
    pDocShell(pDocSh)
{
}

/*-- 17.12.98 08:43:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXPageStyle::~SwXPageStyle()
{

}
/* -----------------------------18.04.01 13:50--------------------------------

 ---------------------------------------------------------------------------*/
void SwXPageStyle::setPropertyValues(
    const Sequence< OUString >& rPropertyNames,
    const Sequence< Any >& rValues )
        throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw RuntimeException();
    SfxItemPropertySet& aPropSet = aSwMapProvider.GetPropertySet(PROPERTY_SET_PAGE_STYLE);

    if(rPropertyNames.getLength() != rValues.getLength())
        throw IllegalArgumentException();

    const OUString* pNames = rPropertyNames.getConstArray();
    const Any* pValues = rValues.getConstArray();
    const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
    SwStyleBase_Impl aBaseImpl(*GetDoc(), GetStyleName());
    if(GetBasePool())
    {
        sal_uInt16 nSaveMask = GetBasePool()->GetSearchMask();
        GetBasePool()->SetSearchMask(GetFamily());
        SfxStyleSheetBase* pBase = GetBasePool()->Find(GetStyleName());
        GetBasePool()->SetSearchMask(GetFamily(), nSaveMask );
        DBG_ASSERT(pBase, "where is the style?" );
        if(pBase)
            aBaseImpl.pNewBase = new SwDocStyleSheet(*(SwDocStyleSheet*)pBase);
        else
            throw RuntimeException();
    }

    for(sal_Int16 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
    {
        pMap = SfxItemPropertyMap::GetByName( pMap, pNames[nProp]);
        if (!pMap)
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        if ( pMap->nFlags & PropertyAttribute::READONLY)
            throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

        if(GetBasePool())
        {
            switch(pMap->nWID)
            {
                case FN_UNO_HEADER_ON:
                case FN_UNO_HEADER_BACKGROUND:
                case FN_UNO_HEADER_BOX:
                case FN_UNO_HEADER_LR_SPACE:
                case FN_UNO_HEADER_SHADOW:
                case FN_UNO_HEADER_BODY_DISTANCE:
                case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE:
                case FN_UNO_HEADER_SHARE_CONTENT:
                case FN_UNO_HEADER_HEIGHT:
                case FN_UNO_HEADER_EAT_SPACING:

                case FN_UNO_FOOTER_ON:
                case FN_UNO_FOOTER_BACKGROUND:
                case FN_UNO_FOOTER_BOX:
                case FN_UNO_FOOTER_LR_SPACE:
                case FN_UNO_FOOTER_SHADOW:
                case FN_UNO_FOOTER_BODY_DISTANCE:
                case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE:
                case FN_UNO_FOOTER_SHARE_CONTENT:
                case FN_UNO_FOOTER_HEIGHT:
                case FN_UNO_FOOTER_EAT_SPACING:
                {
                    sal_Bool bSetItem = sal_False;
                    sal_Bool bFooter = sal_False;
                    sal_uInt16 nItemType = TYPE_BOOL;
                    sal_uInt16 nRes = 0;
                    switch(pMap->nWID)
                    {
                        case FN_UNO_FOOTER_ON:                  bFooter = sal_True;
                        //kein break;
                        case FN_UNO_HEADER_ON:                  nRes = SID_ATTR_PAGE_ON;
                        break;
                        case FN_UNO_FOOTER_BACKGROUND:          bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BACKGROUND:          nRes = RES_BACKGROUND; nItemType = TYPE_BRUSH;
                        break;
                        case FN_UNO_FOOTER_BOX:                 bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BOX:                 nRes = RES_BOX; nItemType = TYPE_BOX;
                        break;
                        case FN_UNO_FOOTER_LR_SPACE:            bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_LR_SPACE:            nRes = RES_LR_SPACE;nItemType = TYPE_LRSPACE;
                        break;
                        case FN_UNO_FOOTER_SHADOW:              bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_SHADOW:              nRes = RES_SHADOW;nItemType = TYPE_SHADOW;
                        break;
                        case FN_UNO_FOOTER_BODY_DISTANCE:       bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_BODY_DISTANCE:       nRes = RES_UL_SPACE;nItemType = TYPE_ULSPACE;
                        break;
                        case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE: bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE: nRes = SID_ATTR_PAGE_DYNAMIC;
                        break;
                        case FN_UNO_FOOTER_SHARE_CONTENT:       bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_SHARE_CONTENT:       nRes = SID_ATTR_PAGE_SHARED;
                        break;
                        case FN_UNO_FOOTER_HEIGHT:              bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_HEIGHT:              nRes = SID_ATTR_PAGE_SIZE;nItemType = TYPE_SIZE;
                        break;
                        case FN_UNO_FOOTER_EAT_SPACING:     bFooter = sal_True;
                        // kein break;
                        case FN_UNO_HEADER_EAT_SPACING:     nRes = RES_HEADER_FOOTER_EAT_SPACING;nItemType = TYPE_SIZE;
                        break;
                    }
                    const SvxSetItem* pSetItem;
                    if(SFX_ITEM_SET == aBaseImpl.GetItemSet().GetItemState(
                            bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                            sal_False, (const SfxPoolItem**)&pSetItem))
                    {
                        SvxSetItem* pNewSetItem = (SvxSetItem*)pSetItem->Clone();
                        SfxItemSet& rSetSet = pNewSetItem->GetItemSet();
                        const SfxPoolItem* pItem = 0;
                        SfxPoolItem* pNewItem = 0;
                        SfxItemState eState = rSetSet.GetItemState(nRes, sal_True, &pItem);
                        if(!pItem && nRes != rSetSet.GetPool()->GetSlotId(nRes))
                            pItem = &rSetSet.GetPool()->GetDefaultItem(nRes);
                        if(pItem)
                        {
                            pNewItem = pItem->Clone();
                        }
                        else
                        {
                            switch(nItemType)
                            {
                                case TYPE_BOOL: pNewItem = new SfxBoolItem(nRes);       break;
                                case TYPE_SIZE: pNewItem = new SvxSizeItem(nRes);       break;
                                case TYPE_BRUSH: pNewItem = new SvxBrushItem(nRes);     break;
                                case TYPE_ULSPACE: pNewItem = new SvxULSpaceItem(nRes); break;
                                case TYPE_SHADOW : pNewItem = new SvxShadowItem(nRes);  break;
                                case TYPE_LRSPACE: pNewItem = new SvxLRSpaceItem(nRes); break;
                                case TYPE_BOX: pNewItem = new SvxBoxItem(nRes);         break;
                            }
                        }
                        bSetItem = pNewItem->PutValue(pValues[nProp], pMap->nMemberId);
                        rSetSet.Put(*pNewItem);
                        aBaseImpl.GetItemSet().Put(*pNewSetItem);
                        delete pNewItem;
                        delete pNewSetItem;
                    }
                    else if(SID_ATTR_PAGE_ON == nRes )
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProp].getValue();
                        if(bVal)
                        {
                            SfxItemSet aTempSet(*aBaseImpl.GetItemSet().GetPool(),
                                RES_BACKGROUND, RES_SHADOW,
                                RES_LR_SPACE, RES_UL_SPACE,
                                nRes, nRes,
                                SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                                SID_ATTR_PAGE_DYNAMIC, SID_ATTR_PAGE_DYNAMIC,
                                SID_ATTR_PAGE_SHARED, SID_ATTR_PAGE_SHARED,
                                0 );
                            aTempSet.Put(SfxBoolItem(nRes, sal_True));
                            aTempSet.Put(SvxSizeItem(SID_ATTR_PAGE_SIZE, Size(MM50, MM50)));
                            aTempSet.Put(SvxLRSpaceItem(RES_LR_SPACE));
                            aTempSet.Put(SvxULSpaceItem(RES_UL_SPACE));
                            aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_SHARED, sal_True));
                            aTempSet.Put(SfxBoolItem(SID_ATTR_PAGE_DYNAMIC, sal_True));

                            SvxSetItem aNewSetItem( bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                                    aTempSet);
                            aBaseImpl.GetItemSet().Put(aNewSetItem);
                        }
                    }
                }
                break;
                case  FN_UNO_HEADER       :
                case  FN_UNO_HEADER_LEFT  :
                case  FN_UNO_HEADER_RIGHT :
                case  FN_UNO_FOOTER       :
                case  FN_UNO_FOOTER_LEFT  :
                case  FN_UNO_FOOTER_RIGHT :
                    throw lang::IllegalArgumentException();
                break;
                case FN_PARAM_FTN_INFO :
                {
                    const SfxPoolItem& rItem = aBaseImpl.GetItemSet().Get(FN_PARAM_FTN_INFO);
                    SfxPoolItem* pNewFtnItem = rItem.Clone();
                    sal_Bool bPut = pNewFtnItem->PutValue(pValues[nProp], pMap->nMemberId);
                    aBaseImpl.GetItemSet().Put(*pNewFtnItem);
                    delete pNewFtnItem;
                    if(!bPut)
                        throw IllegalArgumentException();
                }
                break;
                default:
                    lcl_SetStyleProperty(pMap, aPropSet, pValues[nProp], aBaseImpl,
                                        GetBasePool(), GetDoc(), GetFamily());
            }
        }
        else if(IsDescriptor())
        {
            if(!GetPropImpl()->SetProperty(pNames[nProp], pValues[nProp]))
                throw lang::IllegalArgumentException();
        }
        else
            throw RuntimeException();
    }
    if(aBaseImpl.HasItemSet())
        aBaseImpl.pNewBase->SetItemSet(aBaseImpl.GetItemSet());
}
/* -----------------------------18.04.01 13:50--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Any > SwXPageStyle::getPropertyValues(
    const Sequence< OUString >& rPropertyNames )
        throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    if(!GetDoc())
        throw RuntimeException();

    sal_Int32 nLength = rPropertyNames.getLength();
    SfxItemPropertySet& aPropSet = aSwMapProvider.GetPropertySet(PROPERTY_SET_PAGE_STYLE);
    const OUString* pNames = rPropertyNames.getConstArray();
    Sequence< Any > aRet ( nLength );

    Any* pRet = aRet.getArray();
    const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
    SwStyleBase_Impl aBase(*GetDoc(), GetStyleName());
    SfxStyleSheetBase* pBase = 0;
    for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
    {
        pMap = SfxItemPropertyMap::GetByName( pMap, pNames[nProp]);
        if (!pMap)
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

        if(GetBasePool())
        {
            if(!pBase)
            {
                sal_uInt16 nSaveMask = GetBasePool()->GetSearchMask();
                GetBasePool()->SetSearchMask(GetFamily(), SFXSTYLEBIT_ALL );
                pBase = GetBasePool()->Find(GetStyleName());
                GetBasePool()->SetSearchMask(GetFamily(), nSaveMask );
            }
            sal_uInt16 nRes;
            sal_Bool bHeader = sal_False, bAll = sal_False, bLeft = sal_False, bRight = sal_False;
            switch(pMap->nWID)
            {
                case FN_UNO_HEADER_ON:
                case FN_UNO_HEADER_BACKGROUND:
                case FN_UNO_HEADER_BOX:
                case FN_UNO_HEADER_LR_SPACE:
                case FN_UNO_HEADER_SHADOW:
                case FN_UNO_HEADER_BODY_DISTANCE:
                case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE:
                case FN_UNO_HEADER_SHARE_CONTENT:
                case FN_UNO_HEADER_HEIGHT:
                case FN_UNO_HEADER_EAT_SPACING:

                case FN_UNO_FOOTER_ON:
                case FN_UNO_FOOTER_BACKGROUND:
                case FN_UNO_FOOTER_BOX:
                case FN_UNO_FOOTER_LR_SPACE:
                case FN_UNO_FOOTER_SHADOW:
                case FN_UNO_FOOTER_BODY_DISTANCE:
                case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE:
                case FN_UNO_FOOTER_SHARE_CONTENT:
                case FN_UNO_FOOTER_HEIGHT:
                case FN_UNO_FOOTER_EAT_SPACING:
                {
                    SfxStyleSheetBasePool* pBasePool = ((SwXPageStyle*)this)->GetBasePool();
                    pBasePool->SetSearchMask(GetFamily());
                    SfxStyleSheetBase* pBase = pBasePool->Find(GetStyleName());
                    if(pBase)
                    {
                        SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
                        const SfxItemSet& rSet = aStyle.GetItemSet();
                        sal_Bool bFooter = sal_False;
                        sal_uInt16 nRes = 0;
                        switch(pMap->nWID)
                        {
                            case FN_UNO_FOOTER_ON:
                                bFooter = sal_True;
                            // kein break!
                            case FN_UNO_HEADER_ON:
                            {
                                //falls das SetItem nicht da ist, dann ist der Wert sal_False
                                BOOL bRet = sal_False;
                                pRet[nProp].setValue(&bRet, ::getCppuBooleanType());
                                nRes = SID_ATTR_PAGE_ON;
                            }
                            break;
                            case FN_UNO_FOOTER_BACKGROUND:      bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_BACKGROUND:      nRes = RES_BACKGROUND;
                            break;
                            case FN_UNO_FOOTER_BOX:             bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_BOX:             nRes = RES_BOX;
                            break;
                            case FN_UNO_FOOTER_LR_SPACE:        bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_LR_SPACE:        nRes = RES_LR_SPACE;
                            break;
                            case FN_UNO_FOOTER_SHADOW:          bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_SHADOW:          nRes = RES_SHADOW;
                            break;
                            case FN_UNO_FOOTER_BODY_DISTANCE:   bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_BODY_DISTANCE:   nRes = RES_UL_SPACE;
                            break;
                            case FN_UNO_FOOTER_IS_DYNAMIC_DISTANCE: bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_IS_DYNAMIC_DISTANCE: nRes = SID_ATTR_PAGE_DYNAMIC;
                            break;
                            case FN_UNO_FOOTER_SHARE_CONTENT:   bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_SHARE_CONTENT:   nRes = SID_ATTR_PAGE_SHARED;
                            break;
                            case FN_UNO_FOOTER_HEIGHT:          bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_HEIGHT:          nRes = SID_ATTR_PAGE_SIZE;
                            break;
                            case FN_UNO_FOOTER_EAT_SPACING: bFooter = sal_True;
                            // kein break;
                            case FN_UNO_HEADER_EAT_SPACING: nRes = RES_HEADER_FOOTER_EAT_SPACING;
                            break;
                        }
                        const SvxSetItem* pSetItem;
                        if(SFX_ITEM_SET == rSet.GetItemState(
                                bFooter ? SID_ATTR_PAGE_FOOTERSET : SID_ATTR_PAGE_HEADERSET,
                                sal_False, (const SfxPoolItem**)&pSetItem))
                        {
                            const SfxItemSet& rSet = pSetItem->GetItemSet();
                            const SfxPoolItem* pItem = 0;
                            SfxItemState eState = rSet.GetItemState(nRes, sal_True, &pItem);
                            if(!pItem && nRes != rSet.GetPool()->GetSlotId(nRes))
                                pItem = &rSet.GetPool()->GetDefaultItem(nRes);
                            if(pItem)
                                pItem->QueryValue(pRet[nProp], pMap->nMemberId);
                        }
                    }
                }
                break;
                case  FN_UNO_HEADER       :
                    bAll = sal_True; goto Header;
                case  FN_UNO_HEADER_LEFT  :
                    bLeft = sal_True; goto Header;
                case  FN_UNO_HEADER_RIGHT :
                    bRight = sal_True; goto Header;
Header:
                    bHeader = sal_True;
                    nRes = RES_HEADER; goto MakeObject;
                case  FN_UNO_FOOTER       :
                    bAll = sal_True; goto Footer;
                case  FN_UNO_FOOTER_LEFT  :
                    bLeft = sal_True; goto Footer;
                case  FN_UNO_FOOTER_RIGHT :
                    bRight = sal_True;
Footer:
                    nRes = RES_FOOTER;
MakeObject:
                {
                    const SwPageDesc& rDesc = aBase.GetOldPageDesc();
                    const SwFrmFmt* pFrmFmt = 0;
                    sal_Bool bShare = bHeader && rDesc.IsHeaderShared()||
                                    !bHeader && rDesc.IsFooterShared();
                    // TextLeft returns the left content if there is one,
                    // Text and TextRight return the master content.
                    // TextRight does the same as Text and is for
                    // comptability only.
                    if( bLeft && !bShare )
                        pFrmFmt = &rDesc.GetLeft();
                    else
                        pFrmFmt = &rDesc.GetMaster();
                    if(pFrmFmt)
                    {
                        const SfxItemSet& rSet = pFrmFmt->GetAttrSet();
                        const SfxPoolItem* pItem;
                        SwFrmFmt* pHeadFootFmt;
                        if(SFX_ITEM_SET == rSet.GetItemState(nRes, sal_True, &pItem) &&
                        0 != (pHeadFootFmt = bHeader ?
                                    ((SwFmtHeader*)pItem)->GetHeaderFmt() :
                                        ((SwFmtFooter*)pItem)->GetFooterFmt()))
                        {
                            // gibt es schon ein Objekt dafuer?
                            SwXHeadFootText* pxHdFt = (SwXHeadFootText*)SwClientIter( *pHeadFootFmt ).
                                            First( TYPE( SwXHeadFootText ));
                            Reference< text::XText >  xRet = pxHdFt;
                            if(!pxHdFt)
                                xRet = new SwXHeadFootText(*pHeadFootFmt, bHeader);
                            pRet[nProp].setValue(&xRet, ::getCppuType((Reference<text::XText>*)0));
                        }
                    }
                    break;
                }
                break;
                case FN_PARAM_FTN_INFO :
                {
                    SwDocStyleSheet aStyle( *(SwDocStyleSheet*)pBase );
                    const SfxItemSet& rSet = aStyle.GetItemSet();
                    const SfxPoolItem& rItem = rSet.Get(FN_PARAM_FTN_INFO);
                    rItem.QueryValue(pRet[nProp], pMap->nMemberId);
                }
                break;
                default:
                pRet[nProp] = lcl_GetStyleProperty(pMap, aPropSet, aBase, pBase, GetFamily(), GetDoc() );
            }
        }
        else if(IsDescriptor())
        {
            Any* pAny = 0;
            GetPropImpl()->GetProperty(pNames[nProp], pAny);
            if ( !pAny )
                GetPropImpl()->GetProperty ( pNames[nProp], mxStyleData, pRet[ nProp ] );
            else
                pRet[nProp] = *pAny;
        }
        else
            throw RuntimeException();
    }
    return aRet;
}
/*-- 17.12.98 08:43:36---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXPageStyle::getPropertyValue(const OUString& rPropertyName) throw(
    UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    const Sequence<OUString> aProperties(&rPropertyName, 1);
    return getPropertyValues(aProperties).getConstArray()[0];
}
/*-- 17.12.98 08:43:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXPageStyle::setPropertyValue(const OUString& rPropertyName, const Any& rValue)
    throw( UnknownPropertyException,
        PropertyVetoException,
        lang::IllegalArgumentException,
        lang::WrappedTargetException,
        RuntimeException)
{
    const Sequence<OUString> aProperties(&rPropertyName, 1);
    const Sequence<Any> aValues(&rValue, 1);
    setPropertyValues(aProperties, aValues);
}

SwXFrameStyle::SwXFrameStyle ( SwDoc *pDoc )
: SwXStyle ( pDoc, SFX_STYLE_FAMILY_FRAME, FALSE)
{
}
/* -----------------------------15.12.00 15:45--------------------------------

 ---------------------------------------------------------------------------*/
SwXFrameStyle::~SwXFrameStyle()
{
}
/* -----------------------------15.12.00 14:30--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< uno::Type > SwXFrameStyle::getTypes(  ) throw(RuntimeException)
{
    Sequence< uno::Type > aTypes = SwXStyle::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 1);
    aTypes.getArray()[nLen] = ::getCppuType((Reference<XEventsSupplier>*)0);
    return aTypes;
}
/* -----------------------------15.12.00 14:30--------------------------------

 ---------------------------------------------------------------------------*/
Any SwXFrameStyle::queryInterface( const uno::Type& rType ) throw(RuntimeException)
{
    Any aRet;
    if(rType == ::getCppuType((Reference<XEventsSupplier>*)0))
        aRet <<= Reference<XEventsSupplier>(this);
    else
        aRet = SwXStyle::queryInterface(rType);
    return aRet;
}
/* -----------------------------15.12.00 14:30--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XNameReplace > SwXFrameStyle::getEvents(  ) throw(RuntimeException)
{
    return new SwFrameStyleEventDescriptor( *this );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
