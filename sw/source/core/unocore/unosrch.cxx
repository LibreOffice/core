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
    std::unordered_map<OUString, beans::PropertyValue> maValues;
    SfxItemPropertyMap                              mrMap;

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
    mrMap( aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_CURSOR)->getPropertyMap() )
{
}

void SwSearchProperties_Impl::SetProperties(const uno::Sequence< beans::PropertyValue >& aSearchAttribs)
{
    //delete all existing values
    maValues.clear();

    for(const beans::PropertyValue& rSearchAttrib : aSearchAttribs)
    {
        const OUString& sName = rSearchAttrib.Name;
        if( !mrMap.hasPropertyByName(sName) )
            throw beans::UnknownPropertyException(sName);
        maValues[sName] = rSearchAttrib;
    }
}

uno::Sequence< beans::PropertyValue > SwSearchProperties_Impl::GetProperties() const
{
    uno::Sequence< beans::PropertyValue > aRet(maValues.size());
    beans::PropertyValue* pProps = aRet.getArray();
    sal_Int32 nPropCount = 0;
    for(auto const & rPair : maValues)
    {
       pProps[nPropCount++] = rPair.second;
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

    auto funcClone = [&rSet](sal_uInt16 nWID, std::unique_ptr<SfxPoolItem> & rpPoolItem)
    {
        if(!rpPoolItem)
            rpPoolItem.reset(rSet.GetPool()->GetDefaultItem(nWID).Clone());
        return rpPoolItem.get();
    };
    for(auto const & rPair : maValues)
    {
        SfxPoolItem* pTempItem = nullptr;
        const SfxItemPropertyMapEntry* pPropEntry = mrMap.getByName(rPair.first);
        assert(pPropEntry && "SetProperties only enters values into maValues if mrMap.hasPropertyByName() wass true");
        const SfxItemPropertyMapEntry & rPropEntry = *pPropEntry;
        sal_uInt16 nWID = rPropEntry.nWID;
        switch(nWID)
        {
            case  RES_BOX:
                pTempItem = funcClone(nWID, pBoxItem);
            break;
            case  RES_CHRATR_BOX:
                pTempItem = funcClone(nWID, pCharBoxItem);
            break;
            case  RES_BREAK:
                pTempItem = funcClone(nWID, pBreakItem);
            break;
            case  RES_CHRATR_AUTOKERN:
                pTempItem = funcClone(nWID, pAutoKernItem);
                break;
            case  RES_CHRATR_BACKGROUND:
                pTempItem = funcClone(nWID, pBrushItem);
            break;
            case  RES_CHRATR_CASEMAP:
                pTempItem = funcClone(nWID, pCasemapItem);
            break;
            case  RES_CHRATR_COLOR:
                pTempItem = funcClone(nWID, pCharColorItem);
            break;
            case  RES_CHRATR_CONTOUR:
                pTempItem = funcClone(nWID, pContourItem);
            break;
            case  RES_CHRATR_CROSSEDOUT:
                pTempItem = funcClone(nWID, pCrossedOutItem);
            break;
            case  RES_CHRATR_ESCAPEMENT:
                pTempItem = funcClone(nWID, pEscItem);
            break;
            case  RES_CHRATR_BLINK:
                pTempItem = funcClone(nWID, pBlinkItem);
            break;
            case  RES_CHRATR_FONT:
                pTempItem = funcClone(nWID, pFontItem);
            break;
            case  RES_CHRATR_FONTSIZE:
                pTempItem = funcClone(nWID, pFontSizeItem);
            break;
            case  RES_CHRATR_KERNING:
                pTempItem = funcClone(nWID, pKernItem);
            break;
            case  RES_CHRATR_LANGUAGE:
                pTempItem = funcClone(nWID, pLangItem);
            break;
            case  RES_CHRATR_NOHYPHEN:
                pTempItem = funcClone(nWID, pNHyphItem);
            break;
            case  RES_CHRATR_POSTURE:
                pTempItem = funcClone(nWID, pPostItem);
            break;
            case  RES_CHRATR_SHADOWED:
                pTempItem = funcClone(nWID, pShadItem);
            break;
            case  RES_TXTATR_CHARFMT:
                pTempItem = funcClone(nWID, pCharFormatItem);
            break;
            case  RES_CHRATR_UNDERLINE:
                pTempItem = funcClone(nWID, pULineItem);
            break;
            case  RES_CHRATR_OVERLINE:
                pTempItem = funcClone(nWID, pOLineItem);
            break;
            case  RES_CHRATR_WEIGHT:
                pTempItem = funcClone(nWID, pWeightItem);
            break;
            case  RES_PARATR_DROP:
                pTempItem = funcClone(nWID, pDropItem);
            break;
            case  RES_TXTATR_INETFMT:
                pTempItem = funcClone(nWID, pInetItem);
            break;
            case  RES_PAGEDESC:
                pTempItem = funcClone(nWID, pDescItem);
            break;
            case  RES_PARATR_ADJUST:
                pTempItem = funcClone(nWID, pAdjItem);
            break;
            case  RES_BACKGROUND:
                pTempItem = funcClone(nWID, pBackItem);
            break;
            case  RES_UL_SPACE:
                pTempItem = funcClone(nWID, pULItem);
            break;
            case  RES_LR_SPACE:
                pTempItem = funcClone(nWID, pLRItem);
            break;
            case  RES_KEEP:
                pTempItem = funcClone(nWID, pKeepItem);
            break;
            case  RES_LINENUMBER:
                pTempItem = funcClone(nWID, pLineNumItem);
            break;
            case  RES_PARATR_LINESPACING:
                pTempItem = funcClone(nWID, pLineSpaceItem);
            break;
            case  RES_PARATR_REGISTER:
                pTempItem = funcClone(nWID, pRegItem);
            break;
            case  RES_PARATR_SPLIT:
                pTempItem = funcClone(nWID, pSplitItem);
            break;
            case  RES_PARATR_TABSTOP:
                pTempItem = funcClone(nWID, pTabItem);
            break;
            case  RES_CHRATR_WORDLINEMODE:
                pTempItem = funcClone(nWID, pWLineItem);
            break;
            case RES_CHRATR_CJK_FONT:
                pTempItem = funcClone(nWID, pFontCJKItem);
            break;
            case RES_CHRATR_CJK_FONTSIZE:
                pTempItem = funcClone(nWID, pFontSizeCJKItem);
            break;
            case RES_CHRATR_CJK_LANGUAGE:
                pTempItem = funcClone(nWID, pCJKLangItem);
            break;
            case RES_CHRATR_CJK_POSTURE:
                pTempItem = funcClone(nWID, pCJKPostureItem);
            break;
            case RES_CHRATR_CJK_WEIGHT:
                pTempItem = funcClone(nWID, pCJKWeightItem);
            break;
            case RES_CHRATR_CTL_FONT:
                pTempItem = funcClone(nWID, pFontCTLItem);
            break;
            case RES_CHRATR_CTL_FONTSIZE:
                pTempItem = funcClone(nWID, pFontSizeCTLItem);
            break;
            case RES_CHRATR_CTL_LANGUAGE:
                pTempItem = funcClone(nWID, pCTLLangItem);
            break;
            case RES_CHRATR_CTL_POSTURE:
                pTempItem = funcClone(nWID, pCTLPostureItem);
            break;
            case RES_CHRATR_CTL_WEIGHT:
                pTempItem = funcClone(nWID, pCTLWeightItem);
            break;
            case RES_CHRATR_SHADOW:
                pTempItem = funcClone(nWID, pShadowItem);
            break;
        }
        if(pTempItem)
        {
            if(bIsValueSearch)
            {
                pTempItem->PutValue(rPair.second.Value, rPropEntry.nMemberId);
                rSet.Put(*pTempItem);
            }
            else
                rSet.InvalidateItem( pTempItem->Which() );
        }
    }
}

bool SwSearchProperties_Impl::HasAttributes() const
{
    return !maValues.empty();
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
    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
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

    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
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
