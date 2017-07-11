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

#include <i18nutil/searchopt.hxx>
#include <o3tl/any.hxx>
#include <vcl/svapp.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <memory>

using namespace ::com::sun::star;

class SwSearchProperties_Impl
{
    std::unique_ptr<std::unique_ptr<beans::PropertyValue>[]> pValueArr;
    const PropertyEntryVector_t                              aPropertyEntries;

    SwSearchProperties_Impl(const SwSearchProperties_Impl&) = delete;
    SwSearchProperties_Impl& operator=(const SwSearchProperties_Impl&) = delete;

public:
    SwSearchProperties_Impl();

    /// @throws beans::UnknownPropertyException
    /// @throws lang::IllegalArgumentException
    /// @throws uno::RuntimeException
    void    SetProperties(const uno::Sequence< beans::PropertyValue >& aSearchAttribs);
    const uno::Sequence< beans::PropertyValue > GetProperties() const;

    void    FillItemSet(SfxItemSet& rSet, bool bIsValueSearch) const;
    bool    HasAttributes() const;
};

SwSearchProperties_Impl::SwSearchProperties_Impl() :
    aPropertyEntries( aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR)->getPropertyMap().getPropertyEntries() )
{
    size_t nArrLen = aPropertyEntries.size();
    pValueArr.reset( new std::unique_ptr<beans::PropertyValue>[nArrLen] );
}

void SwSearchProperties_Impl::SetProperties(const uno::Sequence< beans::PropertyValue >& aSearchAttribs)
{
    const beans::PropertyValue* pProps = aSearchAttribs.getConstArray();

    //delete all existing values
    for(size_t i = 0; i < aPropertyEntries.size(); ++i)
    {
        pValueArr[i].reset();
    }

    const sal_uInt32 nLen = aSearchAttribs.getLength();
    for(sal_uInt32 i = 0; i < nLen; ++i)
    {
        sal_uInt32 nIndex = 0;
        PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
        while(pProps[i].Name != aIt->sName)
        {
            ++aIt;
            nIndex++;
            if( aIt == aPropertyEntries.end() )
                throw beans::UnknownPropertyException();
        }
        pValueArr[nIndex].reset( new beans::PropertyValue(pProps[i]) );
    }
}

const uno::Sequence< beans::PropertyValue > SwSearchProperties_Impl::GetProperties() const
{
    sal_uInt32 nPropCount = 0;
    for( size_t i = 0; i < aPropertyEntries.size(); i++)
        if(pValueArr[i])
            nPropCount++;

    uno::Sequence< beans::PropertyValue > aRet(nPropCount);
    beans::PropertyValue* pProps = aRet.getArray();
    nPropCount = 0;
    for(size_t i = 0; i < aPropertyEntries.size(); i++)
    {
        if(pValueArr[i])
        {
            pProps[nPropCount++] = *(pValueArr[i]);
        }
    }
    return aRet;
}

void SwSearchProperties_Impl::FillItemSet(SfxItemSet& rSet, bool bIsValueSearch) const
{

    SfxPoolItem* pBoxItem = nullptr,
    *pCharBoxItem = nullptr,
    *pBreakItem = nullptr,
    *pAutoKernItem  = nullptr,
    *pWLineItem   = nullptr,
    *pTabItem  = nullptr,
    *pSplitItem  = nullptr,
    *pRegItem  = nullptr,
    *pLineSpaceItem  = nullptr,
    *pLineNumItem  = nullptr,
    *pKeepItem  = nullptr,
    *pLRItem  = nullptr,
    *pULItem  = nullptr,
    *pBackItem  = nullptr,
    *pAdjItem  = nullptr,
    *pDescItem  = nullptr,
    *pInetItem  = nullptr,
    *pDropItem  = nullptr,
    *pWeightItem  = nullptr,
    *pULineItem  = nullptr,
    *pOLineItem  = nullptr,
    *pCharFormatItem  = nullptr,
    *pShadItem  = nullptr,
    *pPostItem  = nullptr,
    *pNHyphItem  = nullptr,
    *pLangItem  = nullptr,
    *pKernItem  = nullptr,
    *pFontSizeItem  = nullptr,
    *pFontItem  = nullptr,
    *pBlinkItem  = nullptr,
    *pEscItem  = nullptr,
    *pCrossedOutItem  = nullptr,
    *pContourItem  = nullptr,
    *pCharColorItem  = nullptr,
    *pCasemapItem  = nullptr,
    *pBrushItem  = nullptr,
    *pFontCJKItem = nullptr,
    *pFontSizeCJKItem = nullptr,
    *pCJKLangItem = nullptr,
    *pCJKPostureItem = nullptr,
    *pCJKWeightItem = nullptr,
    *pFontCTLItem = nullptr,
    *pFontSizeCTLItem = nullptr,
    *pCTLLangItem = nullptr,
    *pCTLPostureItem = nullptr,
    *pCTLWeightItem = nullptr,
    *pShadowItem  = nullptr;

    PropertyEntryVector_t::const_iterator aIt = aPropertyEntries.begin();
    for(size_t i = 0; i < aPropertyEntries.size(); i++, ++aIt)
    {
        if(pValueArr[i])
        {
            SfxPoolItem* pTempItem = nullptr;
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
                    if(!pCharFormatItem)
                        pCharFormatItem = rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone();
                    pTempItem = pCharFormatItem;
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
    delete pCharFormatItem  ;
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

bool SwSearchProperties_Impl::HasAttributes() const
{
    for(size_t i = 0; i < aPropertyEntries.size(); i++)
        if(pValueArr[i])
            return true;
    return false;
}

SwXTextSearch::SwXTextSearch() :
    m_pSearchProperties( new SwSearchProperties_Impl),
    m_pReplaceProperties( new SwSearchProperties_Impl),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_SEARCH)),
    m_bAll(false),
    m_bWord(false),
    m_bBack(false),
    m_bExpr(false),
    m_bCase(false),
    m_bStyles(false),
    m_bSimilarity(false),
    m_bLevRelax(false),
    m_nLevExchange(2),
    m_nLevAdd(2),
    m_nLevRemove(2),
    m_bIsValueSearch(true)
{
}

SwXTextSearch::~SwXTextSearch()
{
    delete m_pSearchProperties;
    delete m_pReplaceProperties;
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
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

OUString SwXTextSearch::getSearchString()
{
    SolarMutexGuard aGuard;
    return m_sSearchText;
}

void SwXTextSearch::setSearchString(const OUString& rString)
{
    SolarMutexGuard aGuard;
    m_sSearchText = rString;
}

OUString SwXTextSearch::getReplaceString()
{
    SolarMutexGuard aGuard;
    return m_sReplaceText;
}

void SwXTextSearch::setReplaceString(const OUString& rReplaceString)
{
    SolarMutexGuard aGuard;
    m_sReplaceText = rReplaceString;
}

uno::Reference< beans::XPropertySetInfo >  SwXTextSearch::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo >  aRef = m_pPropSet->getPropertySetInfo();
    return aRef;
}

void SwXTextSearch::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
{
    SolarMutexGuard aGuard;
    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
    if(pEntry)
    {
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException ("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        bool bVal = false;
        if(auto b = o3tl::tryAccess<bool>(aValue))
            bVal = *b;
        switch(pEntry->nWID)
        {
            case WID_SEARCH_ALL :           m_bAll        = bVal; break;
            case WID_WORDS:                 m_bWord       = bVal; break;
            case WID_BACKWARDS :            m_bBack       = bVal; break;
            case WID_REGULAR_EXPRESSION :   m_bExpr       = bVal; break;
            case WID_CASE_SENSITIVE  :      m_bCase       = bVal; break;
            //case WID_IN_SELECTION  :      bInSel      = bVal; break;
            case WID_STYLES          :      m_bStyles     = bVal; break;
            case WID_SIMILARITY      :      m_bSimilarity = bVal; break;
            case WID_SIMILARITY_RELAX:      m_bLevRelax   = bVal; break;
            case WID_SIMILARITY_EXCHANGE:   aValue >>= m_nLevExchange; break;
            case WID_SIMILARITY_ADD:        aValue >>= m_nLevAdd; break;
            case WID_SIMILARITY_REMOVE :    aValue >>= m_nLevRemove;break;
        };
    }
    else
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
}

uno::Any SwXTextSearch::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
    bool bSet = false;
    if(pEntry)
    {
        sal_Int16 nSet = 0;
        switch(pEntry->nWID)
        {
            case WID_SEARCH_ALL :           bSet = m_bAll; goto SET_BOOL;
            case WID_WORDS:                 bSet = m_bWord; goto SET_BOOL;
            case WID_BACKWARDS :            bSet = m_bBack; goto SET_BOOL;
            case WID_REGULAR_EXPRESSION :   bSet = m_bExpr; goto SET_BOOL;
            case WID_CASE_SENSITIVE  :      bSet = m_bCase; goto SET_BOOL;
            //case WID_IN_SELECTION  :      bSet = bInSel; goto SET_BOOL;
            case WID_STYLES          :      bSet = m_bStyles; goto SET_BOOL;
            case WID_SIMILARITY      :      bSet = m_bSimilarity; goto SET_BOOL;
            case WID_SIMILARITY_RELAX:      bSet = m_bLevRelax;
SET_BOOL:
            aRet <<= bSet;
            break;
            case WID_SIMILARITY_EXCHANGE:   nSet = m_nLevExchange; goto SET_UINT16;
            case WID_SIMILARITY_ADD:        nSet = m_nLevAdd; goto SET_UINT16;
            case WID_SIMILARITY_REMOVE :    nSet = m_nLevRemove;
SET_UINT16:
            aRet <<= nSet;
            break;
        };
    }
    else
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    return aRet;
}

void SwXTextSearch::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextSearch::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextSearch::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXTextSearch::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/)
{
    OSL_FAIL("not implemented");
}

sal_Bool SwXTextSearch::getValueSearch()
{
    SolarMutexGuard aGuard;
    return m_bIsValueSearch;
}

void SwXTextSearch::setValueSearch(sal_Bool ValueSearch_)
{
    SolarMutexGuard aGuard;
    m_bIsValueSearch = ValueSearch_;
}

uno::Sequence< beans::PropertyValue > SwXTextSearch::getSearchAttributes()
{
    return  m_pSearchProperties->GetProperties();
}

void SwXTextSearch::setSearchAttributes(const uno::Sequence< beans::PropertyValue >& rSearchAttribs)
{
    m_pSearchProperties->SetProperties(rSearchAttribs);
}

uno::Sequence< beans::PropertyValue > SwXTextSearch::getReplaceAttributes()
{
    return m_pReplaceProperties->GetProperties();
}

void SwXTextSearch::setReplaceAttributes(const uno::Sequence< beans::PropertyValue >& rReplaceAttribs)
{
    m_pReplaceProperties->SetProperties(rReplaceAttribs);
}

void    SwXTextSearch::FillSearchItemSet(SfxItemSet& rSet) const
{
    m_pSearchProperties->FillItemSet(rSet, m_bIsValueSearch);
}

void    SwXTextSearch::FillReplaceItemSet(SfxItemSet& rSet) const
{
    m_pReplaceProperties->FillItemSet(rSet, m_bIsValueSearch);
}

bool    SwXTextSearch::HasSearchAttributes() const
{
    return m_pSearchProperties->HasAttributes();
}

bool    SwXTextSearch::HasReplaceAttributes() const
{
    return m_pReplaceProperties->HasAttributes();
}

OUString SwXTextSearch::getImplementationName()
{
    return OUString("SwXTextSearch");
}

sal_Bool SwXTextSearch::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextSearch::getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.util.SearchDescriptor";
    pArray[1] = "com.sun.star.util.ReplaceDescriptor";
    return aRet;
}

void SwXTextSearch::FillSearchOptions( i18nutil::SearchOptions2& rSearchOpt ) const
{
    if( m_bSimilarity )
    {
        rSearchOpt.algorithmType = util::SearchAlgorithms_APPROXIMATE;
        rSearchOpt.AlgorithmType2 = util::SearchAlgorithms2::APPROXIMATE;
        rSearchOpt.changedChars = m_nLevExchange;
        rSearchOpt.deletedChars = m_nLevRemove;
        rSearchOpt.insertedChars = m_nLevAdd;
        if( m_bLevRelax )
            rSearchOpt.searchFlag |= util::SearchFlags::LEV_RELAXED;
    }
    else if( m_bExpr )
    {
        rSearchOpt.algorithmType = util::SearchAlgorithms_REGEXP;
        rSearchOpt.AlgorithmType2 = util::SearchAlgorithms2::REGEXP;
    }
    else
    {
        rSearchOpt.algorithmType = util::SearchAlgorithms_ABSOLUTE;
        rSearchOpt.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
    }

    rSearchOpt.Locale = GetAppLanguageTag().getLocale();
    rSearchOpt.searchString = m_sSearchText;
    rSearchOpt.replaceString = m_sReplaceText;

    if( !m_bCase )
        rSearchOpt.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    if( m_bWord )
        rSearchOpt.searchFlag |= util::SearchFlags::NORM_WORD_ONLY;

//  bInSel: 1;  // How is that possible?
//  TODO: pSearch->bStyles!
//      inSelection??
//      aSrchParam.SetSrchInSelection(TypeConversion::toBOOL(aVal));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
