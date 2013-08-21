/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "unosrch.hxx"
#include <doc.hxx>
#include <hints.hxx>
#include <unomap.hxx>
#include <unobaseclass.hxx>
#include <unomid.h>

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include "editeng/unolingu.hxx"
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

class SwSearchProperties_Impl
{
    beans::PropertyValue**          pValueArr; //
    sal_uInt32                      nArrLen;
    const PropertyEntryVector_t     aPropertyEntries;
public:
    SwSearchProperties_Impl();
    ~SwSearchProperties_Impl();

    void    SetProperties(const uno::Sequence< beans::PropertyValue >& aSearchAttribs)
        throw( beans::UnknownPropertyException, lang::IllegalArgumentException, uno::RuntimeException );
    const uno::Sequence< beans::PropertyValue > GetProperties() const;

    void    FillItemSet(SfxItemSet& rSet, sal_Bool bIsValueSearch) const;
    sal_Bool    HasAttributes() const;
};

SwSearchProperties_Impl::SwSearchProperties_Impl() :
    nArrLen(0),
    aPropertyEntries( aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR)->getPropertyMap().getPropertyEntries())
{
    nArrLen = aPropertyEntries.size();
    pValueArr = new beans::PropertyValue*[nArrLen];
    for(sal_uInt32 i = 0; i < nArrLen; i++)
        pValueArr[i] = 0;
}

SwSearchProperties_Impl::~SwSearchProperties_Impl()
{
    for(sal_uInt32 i = 0; i < nArrLen; i++)
        delete pValueArr[i];
    delete[] pValueArr;
}

void    SwSearchProperties_Impl::SetProperties(const uno::Sequence< beans::PropertyValue >& aSearchAttribs)
                throw( beans::UnknownPropertyException, lang::IllegalArgumentException, uno::RuntimeException )
{
    const beans::PropertyValue* pProps = aSearchAttribs.getConstArray();
    sal_uInt32 i;

    //delete all existing values
    for( i = 0; i < nArrLen; i++)
    {
        delete pValueArr[i];
        pValueArr[i] = 0;
    }

    sal_uInt32 nLen = aSearchAttribs.getLength();
    for(i = 0; i < nLen; i++)
    {
        sal_uInt16 nIndex = 0;
        PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
        while(pProps[i].Name != aIt->sName)
        {
            ++aIt;
            nIndex++;
            if( aIt == aPropertyEntries.end() )
                throw beans::UnknownPropertyException();
        }
        pValueArr[nIndex] = new beans::PropertyValue(pProps[i]);
    }
}

const uno::Sequence< beans::PropertyValue > SwSearchProperties_Impl::GetProperties() const
{
    sal_uInt32 nPropCount = 0;
    sal_uInt32 i;
    for( i = 0; i < nArrLen; i++)
        if(pValueArr[i])
            nPropCount++;

    uno::Sequence< beans::PropertyValue > aRet(nPropCount);
    beans::PropertyValue* pProps = aRet.getArray();
    nPropCount = 0;
    for(i = 0; i < nArrLen; i++)
    {
        if(pValueArr[i])
        {
            pProps[nPropCount++] = *(pValueArr[i]);
        }
    }
    return aRet;
}

void SwSearchProperties_Impl::FillItemSet(SfxItemSet& rSet, sal_Bool bIsValueSearch) const
{

    SfxPoolItem* pBoxItem = 0,
    *pCharBoxItem = 0,
    *pBreakItem = 0,
    *pAutoKernItem  = 0,
    *pWLineItem   = 0,
    *pTabItem  = 0,
    *pSplitItem  = 0,
    *pRegItem  = 0,
    *pLineSpaceItem  = 0,
    *pLineNumItem  = 0,
    *pKeepItem  = 0,
    *pLRItem  = 0,
    *pULItem  = 0,
    *pBackItem  = 0,
    *pAdjItem  = 0,
    *pDescItem  = 0,
    *pInetItem  = 0,
    *pDropItem  = 0,
    *pWeightItem  = 0,
    *pULineItem  = 0,
    *pOLineItem  = 0,
    *pCharFmtItem  = 0,
    *pShadItem  = 0,
    *pPostItem  = 0,
    *pNHyphItem  = 0,
    *pLangItem  = 0,
    *pKernItem  = 0,
    *pFontSizeItem  = 0,
    *pFontItem  = 0,
    *pBlinkItem  = 0,
    *pEscItem  = 0,
    *pCrossedOutItem  = 0,
    *pContourItem  = 0,
    *pCharColorItem  = 0,
    *pCasemapItem  = 0,
    *pBrushItem  = 0,
    *pFontCJKItem = 0,
    *pFontSizeCJKItem = 0,
    *pCJKLangItem = 0,
    *pCJKPostureItem = 0,
    *pCJKWeightItem = 0,
    *pFontCTLItem = 0,
    *pFontSizeCTLItem = 0,
    *pCTLLangItem = 0,
    *pCTLPostureItem = 0,
    *pCTLWeightItem = 0,
    *pShadowItem  = 0;

    PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
    for(sal_uInt32 i = 0; i < nArrLen; i++, ++aIt)
    {
        if(pValueArr[i])
        {
            SfxPoolItem* pTempItem = 0;
            switch(aIt->nWID)
            {
                case  RES_BOX:
                    if(!pBoxItem)
                        pBoxItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pBoxItem;
                break;
                case  RES_CHRATR_BOX:
                    if(!pCharBoxItem)
                        pCharBoxItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCharBoxItem;
                break;
                case  RES_BREAK:
                    if(!pBreakItem)
                        pBreakItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pBreakItem;
                break;
                case  RES_CHRATR_AUTOKERN:
                    if(!pAutoKernItem)
                        pAutoKernItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pAutoKernItem;
                    break;
                case  RES_CHRATR_BACKGROUND:
                    if(!pBrushItem)
                        pBrushItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pBrushItem;
                break;
                case  RES_CHRATR_CASEMAP:
                    if(!pCasemapItem)
                        pCasemapItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCasemapItem;
                break;
                case  RES_CHRATR_COLOR:
                    if(!pCharColorItem)
                        pCharColorItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCharColorItem;
                break;
                case  RES_CHRATR_CONTOUR:
                    if(!pContourItem)
                        pContourItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pContourItem;
                break;
                case  RES_CHRATR_CROSSEDOUT:
                    if(!pCrossedOutItem)
                        pCrossedOutItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCrossedOutItem;
                break;
                case  RES_CHRATR_ESCAPEMENT:
                    if(!pEscItem)
                        pEscItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pEscItem;
                break;
                case  RES_CHRATR_BLINK:
                    if(!pBlinkItem)
                        pBlinkItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pBlinkItem;
                break;
                case  RES_CHRATR_FONT:
                    if(!pFontItem)
                        pFontItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pFontItem;
                break;
                case  RES_CHRATR_FONTSIZE:
                    if(!pFontSizeItem)
                        pFontSizeItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pFontSizeItem;
                break;
                case  RES_CHRATR_KERNING:
                    if(!pKernItem)
                        pKernItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pKernItem;
                break;
                case  RES_CHRATR_LANGUAGE:
                    if(!pLangItem)
                        pLangItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pLangItem;
                break;
                case  RES_CHRATR_NOHYPHEN:
                    if(!pNHyphItem)
                        pNHyphItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pNHyphItem;
                break;
                case  RES_CHRATR_POSTURE:
                    if(!pPostItem)
                        pPostItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pPostItem;
                break;
                case  RES_CHRATR_SHADOWED:
                    if(!pShadItem)
                        pShadItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pShadItem;
                break;
                case  RES_TXTATR_CHARFMT:
                    if(!pCharFmtItem)
                        pCharFmtItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCharFmtItem;
                break;
                case  RES_CHRATR_UNDERLINE:
                    if(!pULineItem)
                        pULineItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pULineItem;
                break;
                case  RES_CHRATR_OVERLINE:
                    if(!pOLineItem)
                        pOLineItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pOLineItem;
                break;
                case  RES_CHRATR_WEIGHT:
                    if(!pWeightItem)
                        pWeightItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pWeightItem;
                break;
                case  RES_PARATR_DROP:
                    if(!pDropItem)
                        pDropItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pDropItem;
                break;
                case  RES_TXTATR_INETFMT:
                    if(!pInetItem)
                        pInetItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pInetItem;
                break;
                case  RES_PAGEDESC:
                    if(!pDescItem)
                        pDescItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pDescItem;
                break;
                case  RES_PARATR_ADJUST:
                    if(!pAdjItem)
                        pAdjItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pAdjItem;
                break;
                case  RES_BACKGROUND:
                    if(!pBackItem)
                        pBackItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pBackItem;
                break;
                case  RES_UL_SPACE:
                    if(!pULItem)
                        pULItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pULItem;
                break;
                case  RES_LR_SPACE:
                    if(!pLRItem)
                        pLRItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pLRItem;
                break;
                case  RES_KEEP:
                    if(!pKeepItem)
                        pKeepItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pKeepItem;
                break;
                case  RES_LINENUMBER:
                    if(!pLineNumItem)
                        pLineNumItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pLineNumItem;
                break;
                case  RES_PARATR_LINESPACING:
                    if(!pLineSpaceItem)
                        pLineSpaceItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pLineSpaceItem;
                break;
                case  RES_PARATR_REGISTER:
                    if(!pRegItem)
                        pRegItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pRegItem;
                break;
                case  RES_PARATR_SPLIT:
                    if(!pSplitItem)
                        pSplitItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pSplitItem;
                break;
                case  RES_PARATR_TABSTOP:
                    if(!pTabItem)
                        pTabItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pTabItem;
                break;
                case  RES_CHRATR_WORDLINEMODE:
                    if(!pWLineItem)
                        pWLineItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pWLineItem;
                break;
                case RES_CHRATR_CJK_FONT:
                    if(!pFontCJKItem )
                        pFontCJKItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pFontCJKItem;
                break;
                case RES_CHRATR_CJK_FONTSIZE:
                    if(!pFontSizeCJKItem )
                        pFontSizeCJKItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pFontSizeCJKItem;
                break;
                case RES_CHRATR_CJK_LANGUAGE:
                    if(!pCJKLangItem )
                        pCJKLangItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCJKLangItem;
                break;
                case RES_CHRATR_CJK_POSTURE:
                    if(!pCJKPostureItem )
                        pCJKPostureItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCJKPostureItem;
                break;
                case RES_CHRATR_CJK_WEIGHT:
                    if(!pCJKWeightItem )
                        pCJKWeightItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCJKWeightItem;
                break;
                case RES_CHRATR_CTL_FONT:
                    if(!pFontCTLItem )
                        pFontCTLItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pFontCTLItem;
                break;
                case RES_CHRATR_CTL_FONTSIZE:
                    if(!pFontSizeCTLItem )
                        pFontSizeCTLItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pFontSizeCTLItem;
                break;
                case RES_CHRATR_CTL_LANGUAGE:
                    if(!pCTLLangItem )
                        pCTLLangItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCTLLangItem;
                break;
                case RES_CHRATR_CTL_POSTURE:
                    if(!pCTLPostureItem )
                        pCTLPostureItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCTLPostureItem;
                break;
                case RES_CHRATR_CTL_WEIGHT:
                    if(!pCTLWeightItem )
                        pCTLWeightItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCTLWeightItem;
                break;
                case RES_CHRATR_SHADOW:
                    if(!pShadowItem )
                        pShadowItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pShadowItem;
                break;
            }
            if(pTempItem)
            {
                if(bIsValueSearch)
                {
                    pTempItem->PutValue(pValueArr[i]->Value, aIt->nMemberId);
                    rSet.Put(*pTempItem);
                }
                else
                    rSet.InvalidateItem( pTempItem->Which() );
            }
        }
    }
    delete pBoxItem;
    delete pCharBoxItem;
    delete pBreakItem;
    delete pAutoKernItem ;
    delete pWLineItem;
    delete pTabItem;
    delete pSplitItem;
    delete pRegItem;
    delete pLineSpaceItem ;
    delete pLineNumItem  ;
    delete pKeepItem;
    delete pLRItem  ;
    delete pULItem  ;
    delete pBackItem;
    delete pAdjItem;
    delete pDescItem;
    delete pInetItem;
    delete pDropItem;
    delete pWeightItem;
    delete pULineItem;
    delete pOLineItem;
    delete pCharFmtItem  ;
    delete pShadItem;
    delete pPostItem;
    delete pNHyphItem;
    delete pLangItem;
    delete pKernItem;
    delete pFontSizeItem ;
    delete pFontItem;
    delete pBlinkItem;
    delete pEscItem;
    delete pCrossedOutItem;
    delete pContourItem  ;
    delete pCharColorItem;
    delete pCasemapItem  ;
    delete pBrushItem  ;
    delete pShadowItem;
}

sal_Bool    SwSearchProperties_Impl::HasAttributes() const
{
    for(sal_uInt32 i = 0; i < nArrLen; i++)
        if(pValueArr[i])
            return sal_True;
    return sal_False;
}

SwXTextSearch::SwXTextSearch() :
    pSearchProperties( new SwSearchProperties_Impl),
    pReplaceProperties( new SwSearchProperties_Impl),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_SEARCH)),
    bAll(sal_False),
    bWord(sal_False),
    bBack(sal_False),
    bExpr(sal_False),
    bCase(sal_False),
    bStyles(sal_False),
    bSimilarity(sal_False),
    bLevRelax(sal_False),
    nLevExchange(2),
    nLevAdd(2),
    nLevRemove(2),
    bIsValueSearch(sal_True)
{
}

SwXTextSearch::~SwXTextSearch()
{
    delete pSearchProperties;
    delete pReplaceProperties;
}

namespace
{
    class theSwXTextSearchUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextSearchUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextSearch::getUnoTunnelId()
{
    return theSwXTextSearchUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXTextSearch::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

OUString SwXTextSearch::getSearchString(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return sSearchText;
}

void SwXTextSearch::setSearchString(const OUString& rString)
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sSearchText = String(rString);
}

OUString SwXTextSearch::getReplaceString(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return sReplaceText;
}

void SwXTextSearch::setReplaceString(const OUString& rReplaceString) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sReplaceText = String(rReplaceString);
}

uno::Reference< beans::XPropertySetInfo >  SwXTextSearch::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropSet->getPropertySetInfo();
    return aRef;
}

void SwXTextSearch::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
    if(pEntry)
    {
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException ("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        sal_Bool bVal = sal_False;
        if(aValue.getValueType() == ::getBooleanCppuType())
            bVal = *(sal_Bool*)aValue.getValue();
        switch(pEntry->nWID)
        {
            case WID_SEARCH_ALL :           bAll        = bVal; break;
            case WID_WORDS:                 bWord       = bVal; break;
            case WID_BACKWARDS :            bBack       = bVal; break;
            case WID_REGULAR_EXPRESSION :   bExpr       = bVal; break;
            case WID_CASE_SENSITIVE  :      bCase       = bVal; break;
            //case WID_IN_SELECTION  :      bInSel      = bVal; break;
            case WID_STYLES          :      bStyles     = bVal; break;
            case WID_SIMILARITY      :      bSimilarity = bVal; break;
            case WID_SIMILARITY_RELAX:      bLevRelax   = bVal; break;
            case WID_SIMILARITY_EXCHANGE:   aValue >>= nLevExchange; break;
            case WID_SIMILARITY_ADD:        aValue >>= nLevAdd; break;
            case WID_SIMILARITY_REMOVE :    aValue >>= nLevRemove;break;
        };
    }
    else
        throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
}

uno::Any SwXTextSearch::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
    sal_Bool bSet = sal_False;
    if(pEntry)
    {
        sal_Int16 nSet = 0;
        switch(pEntry->nWID)
        {
            case WID_SEARCH_ALL :           bSet = bAll; goto SET_BOOL;
            case WID_WORDS:                 bSet = bWord; goto SET_BOOL;
            case WID_BACKWARDS :            bSet = bBack; goto SET_BOOL;
            case WID_REGULAR_EXPRESSION :   bSet = bExpr; goto SET_BOOL;
            case WID_CASE_SENSITIVE  :      bSet = bCase; goto SET_BOOL;
            //case WID_IN_SELECTION  :      bSet = bInSel; goto SET_BOOL;
            case WID_STYLES          :      bSet = bStyles; goto SET_BOOL;
            case WID_SIMILARITY      :      bSet = bSimilarity; goto SET_BOOL;
            case WID_SIMILARITY_RELAX:      bSet = bLevRelax;
SET_BOOL:
            aRet.setValue(&bSet, ::getBooleanCppuType());
            break;
            case WID_SIMILARITY_EXCHANGE:   nSet = nLevExchange; goto SET_UINT16;
            case WID_SIMILARITY_ADD:        nSet = nLevAdd; goto SET_UINT16;
            case WID_SIMILARITY_REMOVE :    nSet = nLevRemove;
SET_UINT16:
            aRet <<= nSet;
            break;
        };
    }
    else
        throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    return aRet;
}

void SwXTextSearch::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextSearch::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw(beans::UnknownPropertyException, lang::WrappedTargetException,uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextSearch::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw(beans::UnknownPropertyException, lang::WrappedTargetException,uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextSearch::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw(beans::UnknownPropertyException, lang::WrappedTargetException,uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

sal_Bool SwXTextSearch::getValueSearch(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return bIsValueSearch;
}

void SwXTextSearch::setValueSearch(sal_Bool ValueSearch_) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    bIsValueSearch = ValueSearch_;
}

uno::Sequence< beans::PropertyValue > SwXTextSearch::getSearchAttributes(void) throw( uno::RuntimeException )
{
    return  pSearchProperties->GetProperties();
}

void SwXTextSearch::setSearchAttributes(const uno::Sequence< beans::PropertyValue >& rSearchAttribs)
    throw( beans::UnknownPropertyException, lang::IllegalArgumentException, uno::RuntimeException )
{
    pSearchProperties->SetProperties(rSearchAttribs);
}

uno::Sequence< beans::PropertyValue > SwXTextSearch::getReplaceAttributes(void)
    throw( uno::RuntimeException )
{
    return pReplaceProperties->GetProperties();
}

void SwXTextSearch::setReplaceAttributes(const uno::Sequence< beans::PropertyValue >& rReplaceAttribs)
    throw( beans::UnknownPropertyException, lang::IllegalArgumentException, uno::RuntimeException )
{
    pReplaceProperties->SetProperties(rReplaceAttribs);
}

void    SwXTextSearch::FillSearchItemSet(SfxItemSet& rSet) const
{
    pSearchProperties->FillItemSet(rSet, bIsValueSearch);
}

void    SwXTextSearch::FillReplaceItemSet(SfxItemSet& rSet) const
{
    pReplaceProperties->FillItemSet(rSet, bIsValueSearch);
}

sal_Bool    SwXTextSearch::HasSearchAttributes() const
{
    return pSearchProperties->HasAttributes();
}

sal_Bool    SwXTextSearch::HasReplaceAttributes() const
{
    return pReplaceProperties->HasAttributes();
}

OUString SwXTextSearch::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTextSearch");
}

sal_Bool SwXTextSearch::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextSearch::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.util.SearchDescriptor";
    pArray[1] = "com.sun.star.util.ReplaceDescriptor";
    return aRet;
}

void SwXTextSearch::FillSearchOptions( util::SearchOptions& rSearchOpt ) const
{
    if( bSimilarity )
    {
        rSearchOpt.algorithmType = util::SearchAlgorithms_APPROXIMATE;
        rSearchOpt.changedChars = nLevExchange;
        rSearchOpt.deletedChars = nLevRemove;
        rSearchOpt.insertedChars = nLevAdd;
        if( bLevRelax )
            rSearchOpt.searchFlag |= util::SearchFlags::LEV_RELAXED;
    }
    else if( bExpr )
        rSearchOpt.algorithmType = util::SearchAlgorithms_REGEXP;
    else
        rSearchOpt.algorithmType = util::SearchAlgorithms_ABSOLUTE;

    rSearchOpt.Locale = GetAppLanguageTag().getLocale();
    rSearchOpt.searchString = sSearchText;
    rSearchOpt.replaceString = sReplaceText;

    if( !bCase )
        rSearchOpt.transliterateFlags |= i18n::TransliterationModules_IGNORE_CASE;
    if( bWord )
        rSearchOpt.searchFlag |= util::SearchFlags::NORM_WORD_ONLY;

//  bInSel: 1;  // wie geht das?
//  TODO: pSearch->bStyles!
//      inSelection??
//      aSrchParam.SetSrchInSelection(TypeConversion::toBOOL(aVal));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
