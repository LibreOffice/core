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

#include <hintids.hxx>
#include <unosrch.hxx>
#include <unomap.hxx>
#include <swtypes.hxx>

#include <osl/diagnose.h>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/searchopt.hxx>
#include <o3tl/any.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svl/itemprop.hxx>
#include <svl/itempool.hxx>
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
    uno::Sequence< beans::PropertyValue > GetProperties() const;

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
    //delete all existing values
    for(size_t i = 0; i < aPropertyEntries.size(); ++i)
    {
        pValueArr[i].reset();
    }

    for(const beans::PropertyValue& rSearchAttrib : aSearchAttribs)
    {
        const OUString& sName = rSearchAttrib.Name;
        auto aIt = std::find_if(aPropertyEntries.begin(), aPropertyEntries.end(),
            [&sName](const SfxItemPropertyNamedEntry& rProp) { return rProp.sName == sName; });
        if( aIt == aPropertyEntries.end() )
            throw beans::UnknownPropertyException(sName);
        auto nIndex = static_cast<sal_uInt32>(std::distance(aPropertyEntries.begin(), aIt));
        pValueArr[nIndex].reset( new beans::PropertyValue(rSearchAttrib) );
    }
}

uno::Sequence< beans::PropertyValue > SwSearchProperties_Impl::GetProperties() const
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

    std::unique_ptr<SfxPoolItem> pBoxItem,
    pCharBoxItem,
    pBreakItem,
    pAutoKernItem ,
    pWLineItem  ,
    pTabItem ,
    pSplitItem ,
    pRegItem ,
    pLineSpaceItem ,
    pLineNumItem ,
    pKeepItem ,
    pLRItem ,
    pULItem ,
    pBackItem ,
    pAdjItem ,
    pDescItem ,
    pInetItem ,
    pDropItem ,
    pWeightItem ,
    pULineItem ,
    pOLineItem ,
    pCharFormatItem ,
    pShadItem ,
    pPostItem ,
    pNHyphItem ,
    pLangItem ,
    pKernItem ,
    pFontSizeItem ,
    pFontItem ,
    pBlinkItem ,
    pEscItem ,
    pCrossedOutItem ,
    pContourItem ,
    pCharColorItem ,
    pCasemapItem ,
    pBrushItem ,
    pFontCJKItem,
    pFontSizeCJKItem,
    pCJKLangItem,
    pCJKPostureItem,
    pCJKWeightItem,
    pFontCTLItem,
    pFontSizeCTLItem,
    pCTLLangItem,
    pCTLPostureItem,
    pCTLWeightItem,
    pShadowItem ;

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
                        pBoxItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pBoxItem.get();
                break;
                case  RES_CHRATR_BOX:
                    if(!pCharBoxItem)
                        pCharBoxItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCharBoxItem.get();
                break;
                case  RES_BREAK:
                    if(!pBreakItem)
                        pBreakItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pBreakItem.get();
                break;
                case  RES_CHRATR_AUTOKERN:
                    if(!pAutoKernItem)
                        pAutoKernItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pAutoKernItem.get();
                    break;
                case  RES_CHRATR_BACKGROUND:
                    if(!pBrushItem)
                        pBrushItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pBrushItem.get();
                break;
                case  RES_CHRATR_CASEMAP:
                    if(!pCasemapItem)
                        pCasemapItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCasemapItem.get();
                break;
                case  RES_CHRATR_COLOR:
                    if(!pCharColorItem)
                        pCharColorItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCharColorItem.get();
                break;
                case  RES_CHRATR_CONTOUR:
                    if(!pContourItem)
                        pContourItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pContourItem.get();
                break;
                case  RES_CHRATR_CROSSEDOUT:
                    if(!pCrossedOutItem)
                        pCrossedOutItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCrossedOutItem.get();
                break;
                case  RES_CHRATR_ESCAPEMENT:
                    if(!pEscItem)
                        pEscItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pEscItem.get();
                break;
                case  RES_CHRATR_BLINK:
                    if(!pBlinkItem)
                        pBlinkItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pBlinkItem.get();
                break;
                case  RES_CHRATR_FONT:
                    if(!pFontItem)
                        pFontItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pFontItem.get();
                break;
                case  RES_CHRATR_FONTSIZE:
                    if(!pFontSizeItem)
                        pFontSizeItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pFontSizeItem.get();
                break;
                case  RES_CHRATR_KERNING:
                    if(!pKernItem)
                        pKernItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pKernItem.get();
                break;
                case  RES_CHRATR_LANGUAGE:
                    if(!pLangItem)
                        pLangItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pLangItem.get();
                break;
                case  RES_CHRATR_NOHYPHEN:
                    if(!pNHyphItem)
                        pNHyphItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pNHyphItem.get();
                break;
                case  RES_CHRATR_POSTURE:
                    if(!pPostItem)
                        pPostItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pPostItem.get();
                break;
                case  RES_CHRATR_SHADOWED:
                    if(!pShadItem)
                        pShadItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pShadItem.get();
                break;
                case  RES_TXTATR_CHARFMT:
                    if(!pCharFormatItem)
                        pCharFormatItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCharFormatItem.get();
                break;
                case  RES_CHRATR_UNDERLINE:
                    if(!pULineItem)
                        pULineItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pULineItem.get();
                break;
                case  RES_CHRATR_OVERLINE:
                    if(!pOLineItem)
                        pOLineItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pOLineItem.get();
                break;
                case  RES_CHRATR_WEIGHT:
                    if(!pWeightItem)
                        pWeightItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pWeightItem.get();
                break;
                case  RES_PARATR_DROP:
                    if(!pDropItem)
                        pDropItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pDropItem.get();
                break;
                case  RES_TXTATR_INETFMT:
                    if(!pInetItem)
                        pInetItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pInetItem.get();
                break;
                case  RES_PAGEDESC:
                    if(!pDescItem)
                        pDescItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pDescItem.get();
                break;
                case  RES_PARATR_ADJUST:
                    if(!pAdjItem)
                        pAdjItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pAdjItem.get();
                break;
                case  RES_BACKGROUND:
                    if(!pBackItem)
                        pBackItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pBackItem.get();
                break;
                case  RES_UL_SPACE:
                    if(!pULItem)
                        pULItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pULItem.get();
                break;
                case  RES_LR_SPACE:
                    if(!pLRItem)
                        pLRItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pLRItem.get();
                break;
                case  RES_KEEP:
                    if(!pKeepItem)
                        pKeepItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pKeepItem.get();
                break;
                case  RES_LINENUMBER:
                    if(!pLineNumItem)
                        pLineNumItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pLineNumItem.get();
                break;
                case  RES_PARATR_LINESPACING:
                    if(!pLineSpaceItem)
                        pLineSpaceItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pLineSpaceItem.get();
                break;
                case  RES_PARATR_REGISTER:
                    if(!pRegItem)
                        pRegItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pRegItem.get();
                break;
                case  RES_PARATR_SPLIT:
                    if(!pSplitItem)
                        pSplitItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pSplitItem.get();
                break;
                case  RES_PARATR_TABSTOP:
                    if(!pTabItem)
                        pTabItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pTabItem.get();
                break;
                case  RES_CHRATR_WORDLINEMODE:
                    if(!pWLineItem)
                        pWLineItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pWLineItem.get();
                break;
                case RES_CHRATR_CJK_FONT:
                    if(!pFontCJKItem )
                        pFontCJKItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pFontCJKItem.get();
                break;
                case RES_CHRATR_CJK_FONTSIZE:
                    if(!pFontSizeCJKItem )
                        pFontSizeCJKItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pFontSizeCJKItem.get();
                break;
                case RES_CHRATR_CJK_LANGUAGE:
                    if(!pCJKLangItem )
                        pCJKLangItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCJKLangItem.get();
                break;
                case RES_CHRATR_CJK_POSTURE:
                    if(!pCJKPostureItem )
                        pCJKPostureItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCJKPostureItem.get();
                break;
                case RES_CHRATR_CJK_WEIGHT:
                    if(!pCJKWeightItem )
                        pCJKWeightItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCJKWeightItem.get();
                break;
                case RES_CHRATR_CTL_FONT:
                    if(!pFontCTLItem )
                        pFontCTLItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pFontCTLItem.get();
                break;
                case RES_CHRATR_CTL_FONTSIZE:
                    if(!pFontSizeCTLItem )
                        pFontSizeCTLItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pFontSizeCTLItem.get();
                break;
                case RES_CHRATR_CTL_LANGUAGE:
                    if(!pCTLLangItem )
                        pCTLLangItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCTLLangItem.get();
                break;
                case RES_CHRATR_CTL_POSTURE:
                    if(!pCTLPostureItem )
                        pCTLPostureItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCTLPostureItem.get();
                break;
                case RES_CHRATR_CTL_WEIGHT:
                    if(!pCTLWeightItem )
                        pCTLWeightItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pCTLWeightItem.get();
                break;
                case RES_CHRATR_SHADOW:
                    if(!pShadowItem )
                        pShadowItem.reset(rSet.GetPool()->GetDefaultItem(aIt->nWID).Clone());
                    pTempItem = pShadowItem.get();
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
    m_pSearchProperties.reset();
    m_pReplaceProperties.reset();
}

namespace
{
}

const uno::Sequence< sal_Int8 > & SwXTextSearch::getUnoTunnelId()
{
    static const UnoTunnelIdInit theSwXTextSearchUnoTunnelId;
    return theSwXTextSearchUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL SwXTextSearch::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    if( isUnoTunnelId<SwXTextSearch>(rId) )
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
    if(!pEntry)
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

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

uno::Any SwXTextSearch::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;

    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
    bool bSet = false;
    if(!pEntry)
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

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
    }

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
    return "SwXTextSearch";
}

sal_Bool SwXTextSearch::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextSearch::getSupportedServiceNames()
{
    return { "com.sun.star.util.SearchDescriptor", "com.sun.star.util.ReplaceDescriptor" };
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
