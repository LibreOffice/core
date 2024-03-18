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

#include <memory>
#include <utility>
#include <scitems.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/postitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/intitem.hxx>
#include <svl/numformat.hxx>
#include <svl/whiter.hxx>
#include <svl/zforlist.hxx>
#include <vcl/outdev.hxx>
#include <tools/fract.hxx>
#include <tools/UnitConversion.hxx>
#include <osl/diagnose.h>

#include <attrib.hxx>
#include <patattr.hxx>
#include <stlsheet.hxx>
#include <stlpool.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <validat.hxx>
#include <scmod.hxx>
#include <fillinfo.hxx>
#include <comphelper/lok.hxx>
#include <tabvwsh.hxx>

CellAttributeHelper::CellAttributeHelper(SfxItemPool& rSfxItemPool)
: mrSfxItemPool(rSfxItemPool)
, mpDefaultCellAttribute(nullptr)
, maRegisteredCellAttributes()
, mpLastHit(nullptr)
, mnCurrentMaxKey(0)
{
}

CellAttributeHelper::~CellAttributeHelper()
{
    delete mpDefaultCellAttribute;
}

const ScPatternAttr* CellAttributeHelper::registerAndCheck(const ScPatternAttr& rCandidate, bool bPassingOwnership) const
{
    if (&rCandidate == &getDefaultCellAttribute())
        return &rCandidate;

    assert(rCandidate.pCellAttributeHelper == this && "WRONG CellAttributeHelper in ScPatternAttr (!)");

    if (rCandidate.isRegistered())
    {
        assert(!bPassingOwnership && "Trying to register an already registered CellAttribute with ownership change (!)");
        rCandidate.mnRefCount++;
        return &rCandidate;
    }

    if (ScPatternAttr::areSame(mpLastHit, &rCandidate))
    {
        // hit for single-entry cache, make use of it
        mpLastHit->mnRefCount++;
        if (bPassingOwnership)
            delete &rCandidate;
        return mpLastHit;
    }

    for (const ScPatternAttr* pCheck : maRegisteredCellAttributes)
    {
        if (mpLastHit == pCheck)
            // ptr compare: already checked above, skip this one
            continue;

        if (ScPatternAttr::areSame(pCheck, &rCandidate))
        {
            pCheck->mnRefCount++;
            if (bPassingOwnership)
                delete &rCandidate;
            mpLastHit = pCheck;
            return pCheck;
        }
    }

    const ScPatternAttr* pCandidate(bPassingOwnership ? &rCandidate : new ScPatternAttr(rCandidate));
    pCandidate->mnRefCount++;
    const_cast<ScPatternAttr*>(pCandidate)->SetPAKey(mnCurrentMaxKey++);
    maRegisteredCellAttributes.insert(pCandidate);
    mpLastHit = pCandidate;
    return pCandidate;
}

void CellAttributeHelper::doUnregister(const ScPatternAttr& rCandidate)
{
    if (&rCandidate == &getDefaultCellAttribute())
        return;

    assert(rCandidate.isRegistered());
    rCandidate.mnRefCount--;

    if (0 != rCandidate.mnRefCount)
        return;

    if (mpLastHit == &rCandidate)
        mpLastHit = nullptr;

    maRegisteredCellAttributes.erase(&rCandidate);
    delete &rCandidate;
}

const ScPatternAttr& CellAttributeHelper::getDefaultCellAttribute() const
{
    // *have* to create on-demand due to mrScDocument.GetPool() *can* be nullptr
    // since mxPoolHelper is *only* created for SCDOCMODE_DOCUMENT and
    // SCDOCMODE_FUNCTIONACCESS (!)
    if (!mpDefaultCellAttribute)
    {
        // GetRscString only works after ScGlobal::Init (indicated by the EmptyBrushItem)
        // TODO: Write additional method ScGlobal::IsInit() or somesuch
        //       or detect whether this is the Secondary Pool for a MessagePool
        if (ScGlobal::GetEmptyBrushItem())
        {
            const OUString aInitialStyle(ScResId(STR_STYLENAME_STANDARD));
            mpDefaultCellAttribute = new ScPatternAttr(
                *const_cast<CellAttributeHelper*>(this),
                nullptr, // no SfxItemSet
                &aInitialStyle);
        }
        else
        {
            mpDefaultCellAttribute = new ScPatternAttr(*const_cast<CellAttributeHelper*>(this));
        }
    }
    return *mpDefaultCellAttribute;
}

void CellAttributeHelper::CellStyleDeleted(const ScStyleSheet& rStyle)
{
    for (const ScPatternAttr* pCheck : maRegisteredCellAttributes)
    {
        if (&rStyle == pCheck->GetStyleSheet())
            const_cast<ScPatternAttr*>(pCheck)->StyleToName();
    }
}

void CellAttributeHelper::CellStyleCreated(ScDocument& rDoc, std::u16string_view rName)
{
    // If a style was created, don't keep any pattern with its name string in the pool,
    // because it would compare equal to a pattern with a pointer to the new style.
    // Calling StyleSheetChanged isn't enough because the pool may still contain items
    // for undo or clipboard content.
    for (const ScPatternAttr* pCheck : maRegisteredCellAttributes)
    {
        if (nullptr == pCheck->GetStyleSheet() && pCheck->GetStyleName() && rName == *pCheck->GetStyleName())
            const_cast<ScPatternAttr*>(pCheck)->UpdateStyleSheet(rDoc); // find and store style pointer
    }
}

void CellAttributeHelper::UpdateAllStyleSheets(ScDocument& rDoc)
{
    for (const ScPatternAttr* pCheck : maRegisteredCellAttributes)
        const_cast<ScPatternAttr*>(pCheck)->UpdateStyleSheet(rDoc);

    // force existence, then access
    getDefaultCellAttribute();
    mpDefaultCellAttribute->UpdateStyleSheet(rDoc);
}

void CellAttributeHelper::AllStylesToNames()
{
    for (const ScPatternAttr* pCheck : maRegisteredCellAttributes)
        const_cast<ScPatternAttr*>(pCheck)->StyleToName();

    // force existence, then access
    getDefaultCellAttribute();
    mpDefaultCellAttribute->StyleToName();
}

CellAttributeHolder::CellAttributeHolder(const ScPatternAttr* pNew, bool bPassingOwnership)
: mpScPatternAttr(nullptr)
{
    if (nullptr != pNew)
        suppress_fun_call_w_exception(mpScPatternAttr = pNew->getCellAttributeHelper().registerAndCheck(*pNew, bPassingOwnership));
}

CellAttributeHolder::CellAttributeHolder(const CellAttributeHolder& rHolder)
: mpScPatternAttr(nullptr)
{
    if (rHolder.getScPatternAttr())
        suppress_fun_call_w_exception(mpScPatternAttr = rHolder.getScPatternAttr()->getCellAttributeHelper().registerAndCheck(*rHolder.getScPatternAttr(), false));
}

CellAttributeHolder::CellAttributeHolder(CellAttributeHolder&& rHolder) noexcept
: mpScPatternAttr(rHolder.mpScPatternAttr)
{
    rHolder.mpScPatternAttr = nullptr;
}

CellAttributeHolder::~CellAttributeHolder()
{
    if (nullptr != mpScPatternAttr)
        suppress_fun_call_w_exception(mpScPatternAttr->getCellAttributeHelper().doUnregister(*mpScPatternAttr));
}

CellAttributeHolder& CellAttributeHolder::operator=(const CellAttributeHolder& rHolder)
{
    if (nullptr != mpScPatternAttr)
    {
        mpScPatternAttr->getCellAttributeHelper().doUnregister(*mpScPatternAttr);
        mpScPatternAttr = nullptr;
    }

    if (rHolder.getScPatternAttr())
        mpScPatternAttr = rHolder.getScPatternAttr()->getCellAttributeHelper().registerAndCheck(*rHolder.getScPatternAttr(), false);

    return *this;
}

CellAttributeHolder& CellAttributeHolder::operator=(CellAttributeHolder&& rHolder)
{
    if (nullptr != mpScPatternAttr)
    {
        mpScPatternAttr->getCellAttributeHelper().doUnregister(*mpScPatternAttr);
        mpScPatternAttr = nullptr;
    }

    std::swap(mpScPatternAttr, rHolder.mpScPatternAttr);
    assert(!rHolder.mpScPatternAttr);

    return *this;
}

bool CellAttributeHolder::operator==(const CellAttributeHolder& rHolder) const
{
    // here we have registered entries, so no need to test for equality
    return mpScPatternAttr == rHolder.mpScPatternAttr;
}

void CellAttributeHolder::setScPatternAttr(const ScPatternAttr* pNew, bool bPassingOwnership)
{
    if (nullptr != mpScPatternAttr)
        mpScPatternAttr->getCellAttributeHelper().doUnregister(*mpScPatternAttr);

    mpScPatternAttr = nullptr;

    if (nullptr != pNew)
        mpScPatternAttr = pNew->getCellAttributeHelper().registerAndCheck(*pNew, bPassingOwnership);
}

bool CellAttributeHolder::areSame(const CellAttributeHolder* p1, const CellAttributeHolder* p2)
{
    if (p1 == p2)
        // pointer compare, this handles already
        // nullptr and if indeed handed over twice
        return true;

    if (nullptr == p1 || nullptr == p2)
        // one ptr is nullptr, not both, that would
        // have triggered above
        return false;

    // return content compare using operator== at last
    return *p1 == *p2;
}

#ifdef DBG_UTIL
static size_t nUsedScPatternAttr(0);
#endif

const WhichRangesContainer aScPatternAttrSchema(svl::Items<ATTR_PATTERN_START, ATTR_PATTERN_END>);

ScPatternAttr::ScPatternAttr(CellAttributeHelper& rHelper, const SfxItemSet* pItemSet, const OUString* pStyleName)
: maLocalSfxItemSet(rHelper.GetPool(), aScPatternAttrSchema)
, pName()
, mxVisible()
, pStyle(nullptr)
, pCellAttributeHelper(&rHelper)
, mnPAKey(0)
, mnRefCount(0)
#ifdef DBG_UTIL
, m_nSerialNumber(nUsedScPatternAttr++)
, m_bDeleted(false)
#endif
{
    if (nullptr != pStyleName)
        pName = *pStyleName;

    // We need to ensure that ScPatternAttr is using the correct WhichRange,
    // see comments in commit message. This does transfers the items with
    // minimized overhead, too
    if (nullptr != pItemSet)
    {
        // CAUTION: Use bInvalidAsDefault == false for the ::Put,
        // we *need* to take over also Items/Slots in state
        // SfxItemState::INVALID aka IsInvalidItem, this is a precious
        // value/information e.g. in ScDocument::CreateSelectionPattern
        maLocalSfxItemSet.Put(*pItemSet, false);
    }
}

ScPatternAttr::ScPatternAttr(const ScPatternAttr& rPatternAttr)
: maLocalSfxItemSet(rPatternAttr.maLocalSfxItemSet)
, pName(rPatternAttr.pName)
, mxVisible()
, pStyle(rPatternAttr.pStyle)
, pCellAttributeHelper(rPatternAttr.pCellAttributeHelper)
, mnPAKey(rPatternAttr.mnPAKey)
, mnRefCount(0)
#ifdef DBG_UTIL
, m_nSerialNumber(nUsedScPatternAttr++)
, m_bDeleted(false)
#endif
{
}

ScPatternAttr::~ScPatternAttr()
{
#ifdef DBG_UTIL
    m_bDeleted = true;
#endif
    // should no longer be referenced, complain if not so
    assert(!isRegistered());
}

static bool StrCmp( const OUString* pStr1, const OUString* pStr2 )
{
    if (pStr1 == pStr2)
        return true;
    if (pStr1 && !pStr2)
        return false;
    if (!pStr1 && pStr2)
        return false;
    return *pStr1 == *pStr2;
}

constexpr size_t compareSize = ATTR_PATTERN_END - ATTR_PATTERN_START + 1;

bool ScPatternAttr::operator==(const ScPatternAttr& rCmp) const
{
    // check if same incarnation
    if (this == &rCmp)
        return true;

    // check everything except the SfxItemSet from base class SfxSetItem
    if (!StrCmp(GetStyleName(), rCmp.GetStyleName()))
        return false;

    // here we need to compare the SfxItemSet. We *know* that these are
    // all simple (one range, same range)
    const SfxItemSet& rSet1(maLocalSfxItemSet);
    const SfxItemSet& rSet2(rCmp.maLocalSfxItemSet);

    // the former method 'FastEqualPatternSets' mentioned:
    //   "Actually test_tdf133629 from UITest_calc_tests9 somehow manages to have
    //   a different range (and I don't understand enough why), so better be safe and compare fully."
    // in that case the hash code above would already fail, too
    assert(rSet1.TotalCount() == compareSize && rSet2.TotalCount() == compareSize);

    // check pools, do not accept different pools
    if (rSet1.GetPool() != rSet2.GetPool())
        return false;

    // check count of set items, has to be equal
    if (rSet1.Count() != rSet2.Count())
        return false;

    // both have no items, done
    if (0 == rSet1.Count())
        return true;

    // compare each item separately
    const SfxPoolItem **ppItem1(rSet1.GetItems_Impl());
    const SfxPoolItem **ppItem2(rSet2.GetItems_Impl());

    // are all pointers the same?
    if (0 == memcmp(ppItem1, ppItem2, compareSize * sizeof(ppItem1[0])))
        return true;

    for (sal_uInt16 nPos(0); nPos < compareSize; nPos++)
    {
        if (!SfxPoolItem::areSame(*ppItem1, *ppItem2))
            return false;
        ++ppItem1;
        ++ppItem2;
    }

    return true;
}

bool ScPatternAttr::areSame(const ScPatternAttr* pItem1, const ScPatternAttr* pItem2)
{
    if (pItem1 == pItem2)
        // pointer compare, this handles already
        // nullptr and if indeed handed over twice
        return true;

    if (nullptr == pItem1 || nullptr == pItem2)
        // one ptr is nullptr, not both, that would
        // have triggered above
        return false;

    // return content compare using operator== at last
    return *pItem1 == *pItem2;
}

SvxCellOrientation ScPatternAttr::GetCellOrientation( const SfxItemSet& rItemSet, const SfxItemSet* pCondSet )
{
    SvxCellOrientation eOrient = SvxCellOrientation::Standard;

    if( GetItem( ATTR_STACKED, rItemSet, pCondSet ).GetValue() )
    {
        eOrient = SvxCellOrientation::Stacked;
    }
    else
    {
        Degree100 nAngle = GetItem( ATTR_ROTATE_VALUE, rItemSet, pCondSet ).GetValue();
        if( nAngle == 9000_deg100 )
            eOrient = SvxCellOrientation::BottomUp;
        else if( nAngle == 27000_deg100 )
            eOrient = SvxCellOrientation::TopBottom;
    }

    return eOrient;
}

SvxCellOrientation ScPatternAttr::GetCellOrientation( const SfxItemSet* pCondSet ) const
{
    return GetCellOrientation( GetItemSet(), pCondSet );
}

namespace {

void getFontIDsByScriptType(SvtScriptType nScript,
        TypedWhichId<SvxFontItem>& nFontId,
        TypedWhichId<SvxFontHeightItem>& nHeightId,
        TypedWhichId<SvxWeightItem>& nWeightId,
        TypedWhichId<SvxPostureItem>& nPostureId,
        TypedWhichId<SvxLanguageItem>& nLangId)
{
    if ( nScript == SvtScriptType::ASIAN )
    {
        nFontId    = ATTR_CJK_FONT;
        nHeightId  = ATTR_CJK_FONT_HEIGHT;
        nWeightId  = ATTR_CJK_FONT_WEIGHT;
        nPostureId = ATTR_CJK_FONT_POSTURE;
        nLangId    = ATTR_CJK_FONT_LANGUAGE;
    }
    else if ( nScript == SvtScriptType::COMPLEX )
    {
        nFontId    = ATTR_CTL_FONT;
        nHeightId  = ATTR_CTL_FONT_HEIGHT;
        nWeightId  = ATTR_CTL_FONT_WEIGHT;
        nPostureId = ATTR_CTL_FONT_POSTURE;
        nLangId    = ATTR_CTL_FONT_LANGUAGE;
    }
    else
    {
        nFontId    = ATTR_FONT;
        nHeightId  = ATTR_FONT_HEIGHT;
        nWeightId  = ATTR_FONT_WEIGHT;
        nPostureId = ATTR_FONT_POSTURE;
        nLangId    = ATTR_FONT_LANGUAGE;
    }
}

}

void ScPatternAttr::fillFont(
        vcl::Font& rFont, const SfxItemSet& rItemSet, ScAutoFontColorMode eAutoMode,
        const OutputDevice* pOutDev, const Fraction* pScale,
        const SfxItemSet* pCondSet, SvtScriptType nScript,
        const Color* pBackConfigColor, const Color* pTextConfigColor)
{
    model::ComplexColor aComplexColor;

    //  determine effective font color
    ScPatternAttr::fillFontOnly(rFont, rItemSet, pOutDev, pScale, pCondSet, nScript);
    ScPatternAttr::fillColor(aComplexColor, rItemSet, eAutoMode, pCondSet, pBackConfigColor, pTextConfigColor);

    //  set font effects
    rFont.SetColor(aComplexColor.getFinalColor());
}

void ScPatternAttr::fillFontOnly(
        vcl::Font& rFont, const SfxItemSet& rItemSet,
        const OutputDevice* pOutDev, const Fraction* pScale,
        const SfxItemSet* pCondSet, SvtScriptType nScript)
{
    // Read items

    const SvxFontItem* pFontAttr;
    sal_uInt32 nFontHeight;
    FontWeight eWeight;
    FontItalic eItalic;
    FontLineStyle eUnder;
    FontLineStyle eOver;
    bool bWordLine;
    FontStrikeout eStrike;
    bool bOutline;
    bool bShadow;
    FontEmphasisMark eEmphasis;
    FontRelief eRelief;
    LanguageType eLang;

    TypedWhichId<SvxFontItem> nFontId(0);
    TypedWhichId<SvxFontHeightItem> nHeightId(0);
    TypedWhichId<SvxWeightItem> nWeightId(0);
    TypedWhichId<SvxPostureItem> nPostureId(0);
    TypedWhichId<SvxLanguageItem> nLangId(0);
    getFontIDsByScriptType(nScript, nFontId, nHeightId, nWeightId, nPostureId, nLangId);

    if (pCondSet)
    {
        pFontAttr = pCondSet->GetItemIfSet( nFontId );
        if ( !pFontAttr )
            pFontAttr = &rItemSet.Get( nFontId );

        const SvxFontHeightItem* pFontHeightItem = pCondSet->GetItemIfSet( nHeightId );
        if ( !pFontHeightItem )
            pFontHeightItem = &rItemSet.Get( nHeightId );
        nFontHeight = pFontHeightItem->GetHeight();

        const SvxWeightItem* pFontHWeightItem = pCondSet->GetItemIfSet( nWeightId );
        if ( !pFontHWeightItem )
            pFontHWeightItem = &rItemSet.Get( nWeightId );
        eWeight = pFontHWeightItem->GetValue();

        const SvxPostureItem* pPostureItem = pCondSet->GetItemIfSet( nPostureId );
        if ( !pPostureItem )
            pPostureItem = &rItemSet.Get( nPostureId );
        eItalic = pPostureItem->GetValue();

        const SvxUnderlineItem* pUnderlineItem = pCondSet->GetItemIfSet( ATTR_FONT_UNDERLINE );
        if ( !pUnderlineItem )
            pUnderlineItem = &rItemSet.Get( ATTR_FONT_UNDERLINE );
        eUnder = pUnderlineItem->GetValue();

        const SvxOverlineItem* pOverlineItem = pCondSet->GetItemIfSet( ATTR_FONT_OVERLINE );
        if ( !pOverlineItem )
            pOverlineItem = &rItemSet.Get( ATTR_FONT_OVERLINE );
        eOver = pOverlineItem->GetValue();

        const SvxWordLineModeItem* pWordlineItem = pCondSet->GetItemIfSet( ATTR_FONT_WORDLINE );
        if ( !pWordlineItem )
            pWordlineItem = &rItemSet.Get( ATTR_FONT_WORDLINE );
        bWordLine = pWordlineItem->GetValue();

        const SvxCrossedOutItem* pCrossedOutItem = pCondSet->GetItemIfSet( ATTR_FONT_CROSSEDOUT );
        if ( !pCrossedOutItem )
            pCrossedOutItem = &rItemSet.Get( ATTR_FONT_CROSSEDOUT );
        eStrike = pCrossedOutItem->GetValue();

        const SvxContourItem* pContourItem = pCondSet->GetItemIfSet( ATTR_FONT_CONTOUR );
        if ( !pContourItem )
            pContourItem = &rItemSet.Get( ATTR_FONT_CONTOUR );
        bOutline = pContourItem->GetValue();

        const SvxShadowedItem* pShadowedItem = pCondSet->GetItemIfSet( ATTR_FONT_SHADOWED );
        if ( !pShadowedItem )
            pShadowedItem = &rItemSet.Get( ATTR_FONT_SHADOWED );
        bShadow = pShadowedItem->GetValue();

        const SvxEmphasisMarkItem* pEmphasisMarkItem = pCondSet->GetItemIfSet( ATTR_FONT_EMPHASISMARK );
        if ( !pEmphasisMarkItem )
            pEmphasisMarkItem = &rItemSet.Get( ATTR_FONT_EMPHASISMARK );
        eEmphasis = pEmphasisMarkItem->GetEmphasisMark();

        const SvxCharReliefItem* pCharReliefItem = pCondSet->GetItemIfSet( ATTR_FONT_RELIEF );
        if ( !pCharReliefItem )
            pCharReliefItem = &rItemSet.Get( ATTR_FONT_RELIEF );
        eRelief = pCharReliefItem->GetValue();

        const SvxLanguageItem* pLanguageItem = pCondSet->GetItemIfSet( nLangId );
        if ( !pLanguageItem )
            pLanguageItem = &rItemSet.Get( nLangId );
        eLang = pLanguageItem->GetLanguage();
    }
    else    // Everything from rItemSet
    {
        pFontAttr = &rItemSet.Get( nFontId );
        nFontHeight = rItemSet.Get( nHeightId ).GetHeight();
        eWeight = rItemSet.Get( nWeightId ).GetValue();
        eItalic = rItemSet.Get( nPostureId ).GetValue();
        eUnder = rItemSet.Get( ATTR_FONT_UNDERLINE ).GetValue();
        eOver = rItemSet.Get( ATTR_FONT_OVERLINE ).GetValue();
        bWordLine = rItemSet.Get( ATTR_FONT_WORDLINE ).GetValue();
        eStrike = rItemSet.Get( ATTR_FONT_CROSSEDOUT ).GetValue();
        bOutline = rItemSet.Get( ATTR_FONT_CONTOUR ).GetValue();
        bShadow = rItemSet.Get( ATTR_FONT_SHADOWED ).GetValue();
        eEmphasis = rItemSet.Get( ATTR_FONT_EMPHASISMARK ).GetEmphasisMark();
        eRelief = rItemSet.Get( ATTR_FONT_RELIEF ).GetValue();
        // for graphite language features
        eLang = rItemSet.Get( nLangId ).GetLanguage();
    }
    OSL_ENSURE(pFontAttr,"Oops?");

    //  Evaluate

    //  FontItem:

    if (rFont.GetFamilyName() != pFontAttr->GetFamilyName())
        rFont.SetFamilyName( pFontAttr->GetFamilyName() );
    if (rFont.GetStyleName() != pFontAttr->GetStyleName())
        rFont.SetStyleName( pFontAttr->GetStyleName() );

    rFont.SetFamily( pFontAttr->GetFamily() );
    rFont.SetCharSet( pFontAttr->GetCharSet() );
    rFont.SetPitch( pFontAttr->GetPitch() );

    rFont.SetLanguage(eLang);

    //  Size

    if ( pOutDev != nullptr )
    {
        Size aEffSize;
        Fraction aFraction( 1,1 );
        if (pScale)
            aFraction = *pScale;
        Size aSize( 0, static_cast<tools::Long>(nFontHeight) );
        MapMode aDestMode = pOutDev->GetMapMode();
        MapMode aSrcMode( MapUnit::MapTwip, Point(), aFraction, aFraction );
        if (aDestMode.GetMapUnit() == MapUnit::MapPixel && pOutDev->GetDPIX() > 0)
            aEffSize = pOutDev->LogicToPixel( aSize, aSrcMode );
        else
        {
            Fraction aFractOne(1,1);
            aDestMode.SetScaleX( aFractOne );
            aDestMode.SetScaleY( aFractOne );
            aEffSize = OutputDevice::LogicToLogic( aSize, aSrcMode, aDestMode );
        }
        rFont.SetFontSize( aEffSize );
    }
    else /* if pOutDev != NULL */
    {
        rFont.SetFontSize( Size( 0, static_cast<tools::Long>(nFontHeight) ) );
    }

    //  set font effects
    rFont.SetWeight( eWeight );
    rFont.SetItalic( eItalic );
    rFont.SetUnderline( eUnder );
    rFont.SetOverline( eOver );
    rFont.SetWordLineMode( bWordLine );
    rFont.SetStrikeout( eStrike );
    rFont.SetOutline( bOutline );
    rFont.SetShadow( bShadow );
    rFont.SetEmphasisMark( eEmphasis );
    rFont.SetRelief( eRelief );
    rFont.SetTransparent( true );
}

void ScPatternAttr::fillColor(model::ComplexColor& rComplexColor, const SfxItemSet& rItemSet, ScAutoFontColorMode eAutoMode, const SfxItemSet* pCondSet, const Color* pBackConfigColor, const Color* pTextConfigColor)
{
    model::ComplexColor aComplexColor;

    Color aColor;

    SvxColorItem const* pColorItem = nullptr;

    if (pCondSet)
        pColorItem = pCondSet->GetItemIfSet(ATTR_FONT_COLOR);

    if (!pColorItem)
        pColorItem = &rItemSet.Get(ATTR_FONT_COLOR);

    if (pColorItem)
    {
        aComplexColor = pColorItem->getComplexColor();
        aColor = pColorItem->GetValue();
    }

    if (aComplexColor.getType() == model::ColorType::Unused)
    {
        aComplexColor.setColor(aColor);
    }

    if ((aColor == COL_AUTO && eAutoMode != ScAutoFontColorMode::Raw)
        || eAutoMode == ScAutoFontColorMode::IgnoreFont
        || eAutoMode == ScAutoFontColorMode::IgnoreAll)
    {
        //  get background color from conditional or own set
        Color aBackColor;
        if ( pCondSet )
        {
            const SvxBrushItem* pItem = pCondSet->GetItemIfSet(ATTR_BACKGROUND);
            if (!pItem)
                pItem = &rItemSet.Get(ATTR_BACKGROUND);
            aBackColor = pItem->GetColor();
        }
        else
        {
            aBackColor = rItemSet.Get(ATTR_BACKGROUND).GetColor();
        }

        //  if background color attribute is transparent, use window color for brightness comparisons
        if (aBackColor == COL_TRANSPARENT
            || eAutoMode == ScAutoFontColorMode::IgnoreBack
            || eAutoMode == ScAutoFontColorMode::IgnoreAll)
        {
            if (!comphelper::LibreOfficeKit::isActive())
            {
                if ( eAutoMode == ScAutoFontColorMode::Print )
                    aBackColor = COL_WHITE;
                else if ( pBackConfigColor )
                {
                    // pBackConfigColor can be used to avoid repeated lookup of the configured color
                    aBackColor = *pBackConfigColor;
                }
                else
                    aBackColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
            }
            else
            {
                // Get document color from current view instead
                SfxViewShell* pSfxViewShell = SfxViewShell::Current();
                ScTabViewShell* pViewShell = dynamic_cast<ScTabViewShell*>(pSfxViewShell);
                if (pViewShell)
                {
                    const ScViewRenderingOptions& rViewRenderingOptions = pViewShell->GetViewRenderingData();
                    aBackColor = rViewRenderingOptions.GetDocColor();
                }
            }
        }

        //  get system text color for comparison
        Color aSysTextColor;
        if (eAutoMode == ScAutoFontColorMode::Print)
        {
            aSysTextColor = COL_BLACK;
        }
        else if (pTextConfigColor)
        {
            // pTextConfigColor can be used to avoid repeated lookup of the configured color
            aSysTextColor = *pTextConfigColor;
        }
        else
        {
            aSysTextColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor;
        }

        //  select the resulting color
        if ( aBackColor.IsDark() && aSysTextColor.IsDark() )
        {
            //  use white instead of dark on dark
            aColor = COL_WHITE;
        }
        else if ( aBackColor.IsBright() && aSysTextColor.IsBright() )
        {
            //  use black instead of bright on bright
            aColor = COL_BLACK;
        }
        else
        {
            //  use aSysTextColor (black for ScAutoFontColorMode::Print, from style settings otherwise)
            aColor = aSysTextColor;
        }
    }
    aComplexColor.setFinalColor(aColor);
    rComplexColor = aComplexColor;
}

ScDxfFont ScPatternAttr::GetDxfFont(const SfxItemSet& rItemSet, SvtScriptType nScript)
{
    TypedWhichId<SvxFontItem> nFontId(0);
    TypedWhichId<SvxFontHeightItem> nHeightId(0);
    TypedWhichId<SvxWeightItem> nWeightId(0);
    TypedWhichId<SvxPostureItem> nPostureId(0);
    TypedWhichId<SvxLanguageItem> nLangId(0);
    getFontIDsByScriptType(nScript, nFontId, nHeightId, nWeightId, nPostureId, nLangId);

    ScDxfFont aReturn;

    if ( const SvxFontItem* pItem = rItemSet.GetItemIfSet( nFontId ) )
    {
        aReturn.pFontAttr = pItem;
    }

    if ( const SvxFontHeightItem* pItem = rItemSet.GetItemIfSet( nHeightId ) )
    {
        aReturn.nFontHeight = pItem->GetHeight();
    }

    if ( const SvxWeightItem* pItem = rItemSet.GetItemIfSet( nWeightId ) )
    {
        aReturn.eWeight = pItem->GetValue();
    }

    if ( const SvxPostureItem* pItem = rItemSet.GetItemIfSet( nPostureId ) )
    {
        aReturn.eItalic = pItem->GetValue();
    }

    if ( const SvxUnderlineItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_UNDERLINE ) )
    {
        pItem = &rItemSet.Get( ATTR_FONT_UNDERLINE );
        aReturn.eUnder = pItem->GetValue();
    }

    if ( const SvxOverlineItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_OVERLINE ) )
    {
        aReturn.eOver = pItem->GetValue();
    }

    if ( const SvxWordLineModeItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_WORDLINE ) )
    {
        aReturn.bWordLine = pItem->GetValue();
    }

    if ( const SvxCrossedOutItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_CROSSEDOUT ) )
    {
        pItem = &rItemSet.Get( ATTR_FONT_CROSSEDOUT );
        aReturn.eStrike = pItem->GetValue();
    }

    if ( const SvxContourItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_CONTOUR ) )
    {
        aReturn.bOutline = pItem->GetValue();
    }

    if ( const SvxShadowedItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_SHADOWED ) )
    {
        pItem = &rItemSet.Get( ATTR_FONT_SHADOWED );
        aReturn.bShadow = pItem->GetValue();
    }

    if ( const SvxEmphasisMarkItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_EMPHASISMARK ) )
    {
        aReturn.eEmphasis = pItem->GetEmphasisMark();
    }

    if ( const SvxCharReliefItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_RELIEF ) )
    {
        aReturn.eRelief = pItem->GetValue();
    }

    if ( const SvxColorItem* pItem = rItemSet.GetItemIfSet( ATTR_FONT_COLOR ) )
    {
        aReturn.aColor = pItem->GetValue();
    }

    if ( const SvxLanguageItem* pItem = rItemSet.GetItemIfSet( nLangId ) )
    {
        aReturn.eLang = pItem->GetLanguage();
    }

    return aReturn;
}

template <class T>
static void lcl_populate( std::unique_ptr<T>& rxItem, TypedWhichId<T> nWhich, const SfxItemSet& rSrcSet, const SfxItemSet* pCondSet )
{
    const T* pItem = pCondSet->GetItemIfSet( nWhich );
    if ( !pItem )
        pItem = &rSrcSet.Get( nWhich );
    rxItem.reset(pItem->Clone());
}

void ScPatternAttr::FillToEditItemSet( SfxItemSet& rEditSet, const SfxItemSet& rSrcSet, const SfxItemSet* pCondSet )
{
    //  Read Items

    std::unique_ptr<SvxColorItem> aColorItem(std::make_unique<SvxColorItem>(EE_CHAR_COLOR));              // use item as-is
    std::unique_ptr<SvxFontItem> aFontItem(std::make_unique<SvxFontItem>(EE_CHAR_FONTINFO));            // use item as-is
    std::unique_ptr<SvxFontItem> aCjkFontItem(std::make_unique<SvxFontItem>(EE_CHAR_FONTINFO_CJK));            // use item as-is
    std::unique_ptr<SvxFontItem> aCtlFontItem(std::make_unique<SvxFontItem>(EE_CHAR_FONTINFO_CTL));            // use item as-is
    tools::Long            nTHeight, nCjkTHeight, nCtlTHeight;     // Twips
    FontWeight      eWeight, eCjkWeight, eCtlWeight;
    std::unique_ptr<SvxUnderlineItem> aUnderlineItem(std::make_unique<SvxUnderlineItem>(LINESTYLE_NONE, EE_CHAR_UNDERLINE));
    std::unique_ptr<SvxOverlineItem> aOverlineItem(std::make_unique<SvxOverlineItem>(LINESTYLE_NONE, EE_CHAR_OVERLINE));
    bool            bWordLine;
    FontStrikeout   eStrike;
    FontItalic      eItalic, eCjkItalic, eCtlItalic;
    bool            bOutline;
    bool            bShadow;
    bool            bForbidden;
    FontEmphasisMark eEmphasis;
    FontRelief      eRelief;
    LanguageType    eLang, eCjkLang, eCtlLang;
    bool            bHyphenate;
    SvxFrameDirection eDirection;

    //TODO: additional parameter to control if language is needed?

    if ( pCondSet )
    {
        lcl_populate(aColorItem, ATTR_FONT_COLOR, rSrcSet, pCondSet);
        lcl_populate(aFontItem, ATTR_FONT, rSrcSet, pCondSet);
        lcl_populate(aCjkFontItem, ATTR_CJK_FONT, rSrcSet, pCondSet);
        lcl_populate(aCtlFontItem, ATTR_CTL_FONT, rSrcSet, pCondSet);

        const SvxFontHeightItem* pFontHeightItem = pCondSet->GetItemIfSet( ATTR_FONT_HEIGHT );
        if (!pFontHeightItem)
            pFontHeightItem = &rSrcSet.Get( ATTR_FONT_HEIGHT );
        nTHeight = pFontHeightItem->GetHeight();
        pFontHeightItem = pCondSet->GetItemIfSet( ATTR_CJK_FONT_HEIGHT );
        if ( !pFontHeightItem )
            pFontHeightItem = &rSrcSet.Get( ATTR_CJK_FONT_HEIGHT );
        nCjkTHeight = pFontHeightItem->GetHeight();
        pFontHeightItem = pCondSet->GetItemIfSet( ATTR_CTL_FONT_HEIGHT );
        if ( !pFontHeightItem )
            pFontHeightItem = &rSrcSet.Get( ATTR_CTL_FONT_HEIGHT );
        nCtlTHeight = pFontHeightItem->GetHeight();

        const SvxWeightItem* pWeightItem = pCondSet->GetItemIfSet( ATTR_FONT_WEIGHT );
        if ( !pWeightItem )
            pWeightItem = &rSrcSet.Get( ATTR_FONT_WEIGHT );
        eWeight = pWeightItem->GetValue();
        pWeightItem = pCondSet->GetItemIfSet( ATTR_CJK_FONT_WEIGHT );
        if ( !pWeightItem )
            pWeightItem = &rSrcSet.Get( ATTR_CJK_FONT_WEIGHT );
        eCjkWeight = pWeightItem->GetValue();
        pWeightItem = pCondSet->GetItemIfSet( ATTR_CTL_FONT_WEIGHT );
        if ( !pWeightItem )
            pWeightItem = &rSrcSet.Get( ATTR_CTL_FONT_WEIGHT );
        eCtlWeight = pWeightItem->GetValue();

        const SvxPostureItem* pPostureItem = pCondSet->GetItemIfSet( ATTR_FONT_POSTURE );
        if ( !pPostureItem )
            pPostureItem = &rSrcSet.Get( ATTR_FONT_POSTURE );
        eItalic = pPostureItem->GetValue();
        pPostureItem = pCondSet->GetItemIfSet( ATTR_CJK_FONT_POSTURE );
        if ( !pPostureItem )
            pPostureItem = &rSrcSet.Get( ATTR_CJK_FONT_POSTURE );
        eCjkItalic = pPostureItem->GetValue();
        pPostureItem = pCondSet->GetItemIfSet( ATTR_CTL_FONT_POSTURE );
        if ( !pPostureItem )
            pPostureItem = &rSrcSet.Get( ATTR_CTL_FONT_POSTURE );
        eCtlItalic = pPostureItem->GetValue();

        lcl_populate(aUnderlineItem, ATTR_FONT_UNDERLINE, rSrcSet, pCondSet);
        lcl_populate(aOverlineItem, ATTR_FONT_OVERLINE, rSrcSet, pCondSet);

        const SvxWordLineModeItem* pWordLineModeItem = pCondSet->GetItemIfSet( ATTR_FONT_WORDLINE );
        if ( !pWordLineModeItem )
            pWordLineModeItem = &rSrcSet.Get( ATTR_FONT_WORDLINE );
        bWordLine = pWordLineModeItem->GetValue();

        const SvxCrossedOutItem* pCrossedOutItem = pCondSet->GetItemIfSet( ATTR_FONT_CROSSEDOUT );
        if ( !pCrossedOutItem )
            pCrossedOutItem = &rSrcSet.Get( ATTR_FONT_CROSSEDOUT );
        eStrike = pCrossedOutItem->GetValue();

        const SvxContourItem* pContourItem = pCondSet->GetItemIfSet( ATTR_FONT_CONTOUR );
        if ( !pContourItem )
            pContourItem = &rSrcSet.Get( ATTR_FONT_CONTOUR );
        bOutline = pContourItem->GetValue();

        const SvxShadowedItem* pShadowedItem = pCondSet->GetItemIfSet( ATTR_FONT_SHADOWED );
        if ( !pShadowedItem )
            pShadowedItem = &rSrcSet.Get( ATTR_FONT_SHADOWED );
        bShadow = pShadowedItem->GetValue();

        const SvxForbiddenRuleItem* pForbiddenRuleItem = pCondSet->GetItemIfSet( ATTR_FORBIDDEN_RULES );
        if ( !pForbiddenRuleItem )
            pForbiddenRuleItem = &rSrcSet.Get( ATTR_FORBIDDEN_RULES );
        bForbidden = pForbiddenRuleItem->GetValue();

        const SvxEmphasisMarkItem* pEmphasisMarkItem = pCondSet->GetItemIfSet( ATTR_FONT_EMPHASISMARK );
        if ( !pEmphasisMarkItem )
            pEmphasisMarkItem = &rSrcSet.Get( ATTR_FONT_EMPHASISMARK );
        eEmphasis = pEmphasisMarkItem->GetEmphasisMark();
        const SvxCharReliefItem* pCharReliefItem = pCondSet->GetItemIfSet( ATTR_FONT_RELIEF );
        if ( !pCharReliefItem )
            pCharReliefItem = &rSrcSet.Get( ATTR_FONT_RELIEF );
        eRelief = pCharReliefItem->GetValue();

        const SvxLanguageItem* pLanguageItem = pCondSet->GetItemIfSet( ATTR_FONT_LANGUAGE );
        if ( !pLanguageItem )
            pLanguageItem = &rSrcSet.Get( ATTR_FONT_LANGUAGE );
        eLang = pLanguageItem->GetLanguage();
        pLanguageItem = pCondSet->GetItemIfSet( ATTR_CJK_FONT_LANGUAGE );
        if ( !pLanguageItem )
            pLanguageItem = &rSrcSet.Get( ATTR_CJK_FONT_LANGUAGE );
        eCjkLang = pLanguageItem->GetLanguage();
        pLanguageItem = pCondSet->GetItemIfSet( ATTR_CTL_FONT_LANGUAGE );
        if ( !pLanguageItem )
            pLanguageItem = &rSrcSet.Get( ATTR_CTL_FONT_LANGUAGE );
        eCtlLang = pLanguageItem->GetLanguage();

        const ScHyphenateCell* pHyphenateCell = pCondSet->GetItemIfSet( ATTR_HYPHENATE );
        if ( !pHyphenateCell )
            pHyphenateCell = &rSrcSet.Get( ATTR_HYPHENATE );
        bHyphenate = pHyphenateCell->GetValue();

        const SvxFrameDirectionItem* pFrameDirectionItem = pCondSet->GetItemIfSet( ATTR_WRITINGDIR );
        if ( !pFrameDirectionItem )
            pFrameDirectionItem = &rSrcSet.Get( ATTR_WRITINGDIR );
        eDirection = pFrameDirectionItem->GetValue();
    }
    else        // Everything directly from Pattern
    {
        aColorItem.reset(rSrcSet.Get(ATTR_FONT_COLOR).Clone());
        aFontItem.reset(rSrcSet.Get(ATTR_FONT).Clone());
        aCjkFontItem.reset(rSrcSet.Get(ATTR_CJK_FONT).Clone());
        aCtlFontItem.reset(rSrcSet.Get(ATTR_CTL_FONT).Clone());
        nTHeight = rSrcSet.Get( ATTR_FONT_HEIGHT ).GetHeight();
        nCjkTHeight = rSrcSet.Get( ATTR_CJK_FONT_HEIGHT ).GetHeight();
        nCtlTHeight = rSrcSet.Get( ATTR_CTL_FONT_HEIGHT ).GetHeight();
        eWeight = rSrcSet.Get( ATTR_FONT_WEIGHT ).GetValue();
        eCjkWeight = rSrcSet.Get( ATTR_CJK_FONT_WEIGHT ).GetValue();
        eCtlWeight = rSrcSet.Get( ATTR_CTL_FONT_WEIGHT ).GetValue();
        eItalic = rSrcSet.Get( ATTR_FONT_POSTURE ).GetValue();
        eCjkItalic = rSrcSet.Get( ATTR_CJK_FONT_POSTURE ).GetValue();
        eCtlItalic = rSrcSet.Get( ATTR_CTL_FONT_POSTURE ).GetValue();
        aUnderlineItem.reset(rSrcSet.Get(ATTR_FONT_UNDERLINE).Clone());
        aOverlineItem.reset(rSrcSet.Get(ATTR_FONT_OVERLINE).Clone());
        bWordLine = rSrcSet.Get( ATTR_FONT_WORDLINE ).GetValue();
        eStrike = rSrcSet.Get( ATTR_FONT_CROSSEDOUT ).GetValue();
        bOutline = rSrcSet.Get( ATTR_FONT_CONTOUR ).GetValue();
        bShadow = rSrcSet.Get( ATTR_FONT_SHADOWED ).GetValue();
        bForbidden = rSrcSet.Get( ATTR_FORBIDDEN_RULES ).GetValue();
        eEmphasis = rSrcSet.Get( ATTR_FONT_EMPHASISMARK ).GetEmphasisMark();
        eRelief = rSrcSet.Get( ATTR_FONT_RELIEF ).GetValue();
        eLang = rSrcSet.Get( ATTR_FONT_LANGUAGE ).GetLanguage();
        eCjkLang = rSrcSet.Get( ATTR_CJK_FONT_LANGUAGE ).GetLanguage();
        eCtlLang = rSrcSet.Get( ATTR_CTL_FONT_LANGUAGE ).GetLanguage();
        bHyphenate = rSrcSet.Get( ATTR_HYPHENATE ).GetValue();
        eDirection = rSrcSet.Get( ATTR_WRITINGDIR ).GetValue();
    }

    // Expect to be compatible to LogicToLogic, ie. 2540/1440 = 127/72, and round

    tools::Long nHeight = convertTwipToMm100(nTHeight);
    tools::Long nCjkHeight = convertTwipToMm100(nCjkTHeight);
    tools::Long nCtlHeight = convertTwipToMm100(nCtlTHeight);

    //  put items into EditEngine ItemSet

    if ( aColorItem->GetValue() == COL_AUTO )
    {
        //  When cell attributes are converted to EditEngine paragraph attributes,
        //  don't create a hard item for automatic color, because that would be converted
        //  to black when the item's Store method is used in CreateTransferable/WriteBin.
        //  COL_AUTO is the EditEngine's pool default, so ClearItem will result in automatic
        //  color, too, without having to store the item.
        rEditSet.ClearItem( EE_CHAR_COLOR );
    }
    else
    {
        // tdf#125054 adapt WhichID
        rEditSet.PutAsTargetWhich( std::move(aColorItem), EE_CHAR_COLOR );
    }

    // tdf#125054 adapt WhichID
    rEditSet.PutAsTargetWhich( std::move(aFontItem), EE_CHAR_FONTINFO );
    rEditSet.PutAsTargetWhich( std::move(aCjkFontItem), EE_CHAR_FONTINFO_CJK );
    rEditSet.PutAsTargetWhich( std::move(aCtlFontItem), EE_CHAR_FONTINFO_CTL );

    rEditSet.Put( SvxFontHeightItem( nHeight, 100, EE_CHAR_FONTHEIGHT ) );
    rEditSet.Put( SvxFontHeightItem( nCjkHeight, 100, EE_CHAR_FONTHEIGHT_CJK ) );
    rEditSet.Put( SvxFontHeightItem( nCtlHeight, 100, EE_CHAR_FONTHEIGHT_CTL ) );
    rEditSet.Put( SvxWeightItem ( eWeight,      EE_CHAR_WEIGHT ) );
    rEditSet.Put( SvxWeightItem ( eCjkWeight,   EE_CHAR_WEIGHT_CJK ) );
    rEditSet.Put( SvxWeightItem ( eCtlWeight,   EE_CHAR_WEIGHT_CTL ) );

    // tdf#125054 adapt WhichID
    rEditSet.PutAsTargetWhich( std::move(aUnderlineItem), EE_CHAR_UNDERLINE );
    rEditSet.PutAsTargetWhich( std::move(aOverlineItem), EE_CHAR_OVERLINE );

    rEditSet.Put( SvxWordLineModeItem( bWordLine,   EE_CHAR_WLM ) );
    rEditSet.Put( SvxCrossedOutItem( eStrike,       EE_CHAR_STRIKEOUT ) );
    rEditSet.Put( SvxPostureItem    ( eItalic,      EE_CHAR_ITALIC ) );
    rEditSet.Put( SvxPostureItem    ( eCjkItalic,   EE_CHAR_ITALIC_CJK ) );
    rEditSet.Put( SvxPostureItem    ( eCtlItalic,   EE_CHAR_ITALIC_CTL ) );
    rEditSet.Put( SvxContourItem    ( bOutline,     EE_CHAR_OUTLINE ) );
    rEditSet.Put( SvxShadowedItem   ( bShadow,      EE_CHAR_SHADOW ) );
    rEditSet.Put( SvxForbiddenRuleItem(bForbidden, EE_PARA_FORBIDDENRULES) );
    rEditSet.Put( SvxEmphasisMarkItem( eEmphasis,   EE_CHAR_EMPHASISMARK ) );
    rEditSet.Put( SvxCharReliefItem( eRelief,       EE_CHAR_RELIEF ) );
    rEditSet.Put( SvxLanguageItem   ( eLang,        EE_CHAR_LANGUAGE ) );
    rEditSet.Put( SvxLanguageItem   ( eCjkLang,     EE_CHAR_LANGUAGE_CJK ) );
    rEditSet.Put( SvxLanguageItem   ( eCtlLang,     EE_CHAR_LANGUAGE_CTL ) );
    rEditSet.Put( SfxBoolItem       ( EE_PARA_HYPHENATE, bHyphenate ) );
    rEditSet.Put( SvxFrameDirectionItem( eDirection, EE_PARA_WRITINGDIR ) );

    // Script spacing is always off.
    // The cell attribute isn't used here as long as there is no UI to set it
    // (don't evaluate attributes that can't be changed).
    // If a locale-dependent default is needed, it has to go into the cell
    // style, like the fonts.
    rEditSet.Put( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
}

void ScPatternAttr::FillEditItemSet( SfxItemSet* pEditSet, const SfxItemSet* pCondSet ) const
{
    if( pEditSet )
        FillToEditItemSet( *pEditSet, GetItemSet(), pCondSet );
}

void ScPatternAttr::GetFromEditItemSet( SfxItemSet& rDestSet, const SfxItemSet& rEditSet )
{
    if (const SvxColorItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_COLOR))
        rDestSet.PutAsTargetWhich( *pItem, ATTR_FONT_COLOR );

    if (const SvxFontItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_FONTINFO))
        rDestSet.PutAsTargetWhich( *pItem, ATTR_FONT );
    if (const SvxFontItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_FONTINFO_CJK))
        rDestSet.PutAsTargetWhich( *pItem, ATTR_CJK_FONT );
    if (const SvxFontItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_FONTINFO_CTL))
        rDestSet.PutAsTargetWhich( *pItem, ATTR_CTL_FONT );

    if (const SvxFontHeightItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_FONTHEIGHT))
        rDestSet.Put( SvxFontHeightItem(o3tl::toTwips(pItem->GetHeight(), o3tl::Length::mm100),
                        100, ATTR_FONT_HEIGHT ) );
    if (const SvxFontHeightItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_FONTHEIGHT_CJK))
        rDestSet.Put( SvxFontHeightItem(o3tl::toTwips(pItem->GetHeight(), o3tl::Length::mm100),
                        100, ATTR_CJK_FONT_HEIGHT ) );
    if (const SvxFontHeightItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_FONTHEIGHT_CTL))
        rDestSet.Put( SvxFontHeightItem(o3tl::toTwips(pItem->GetHeight(), o3tl::Length::mm100),
                        100, ATTR_CTL_FONT_HEIGHT ) );

    if (const SvxWeightItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_WEIGHT))
        rDestSet.Put( SvxWeightItem( pItem->GetValue(),
                        ATTR_FONT_WEIGHT) );
    if (const SvxWeightItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_WEIGHT_CJK))
        rDestSet.Put( SvxWeightItem( pItem->GetValue(),
                        ATTR_CJK_FONT_WEIGHT) );
    if (const SvxWeightItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_WEIGHT_CTL))
        rDestSet.Put( SvxWeightItem( pItem->GetValue(),
                        ATTR_CTL_FONT_WEIGHT) );

    // SvxTextLineItem contains enum and color
    if (const SvxUnderlineItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_UNDERLINE))
        rDestSet.PutAsTargetWhich( *pItem, ATTR_FONT_UNDERLINE );
    if (const SvxOverlineItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_OVERLINE))
        rDestSet.PutAsTargetWhich( *pItem, ATTR_FONT_OVERLINE );
    if (const SvxWordLineModeItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_WLM))
        rDestSet.Put( SvxWordLineModeItem( pItem->GetValue(),
                        ATTR_FONT_WORDLINE) );

    if (const SvxCrossedOutItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_STRIKEOUT))
        rDestSet.Put( SvxCrossedOutItem( pItem->GetValue(),
                        ATTR_FONT_CROSSEDOUT) );

    if (const SvxPostureItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_ITALIC))
        rDestSet.Put( SvxPostureItem( pItem->GetValue(),
                        ATTR_FONT_POSTURE) );
    if (const SvxPostureItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_ITALIC_CJK))
        rDestSet.Put( SvxPostureItem( pItem->GetValue(),
                        ATTR_CJK_FONT_POSTURE) );
    if (const SvxPostureItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_ITALIC_CTL))
        rDestSet.Put( SvxPostureItem( pItem->GetValue(),
                        ATTR_CTL_FONT_POSTURE) );

    if (const SvxContourItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_OUTLINE))
        rDestSet.Put( SvxContourItem( pItem->GetValue(),
                        ATTR_FONT_CONTOUR) );
    if (const SvxShadowedItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_SHADOW))
        rDestSet.Put( SvxShadowedItem( pItem->GetValue(),
                        ATTR_FONT_SHADOWED) );
    if (const SvxEmphasisMarkItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_EMPHASISMARK))
        rDestSet.Put( SvxEmphasisMarkItem( pItem->GetEmphasisMark(),
                        ATTR_FONT_EMPHASISMARK) );
    if (const SvxCharReliefItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_RELIEF))
        rDestSet.Put( SvxCharReliefItem( pItem->GetValue(),
                        ATTR_FONT_RELIEF) );

    if (const SvxLanguageItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_LANGUAGE))
        rDestSet.Put( SvxLanguageItem(pItem->GetValue(), ATTR_FONT_LANGUAGE) );
    if (const SvxLanguageItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_LANGUAGE_CJK))
        rDestSet.Put( SvxLanguageItem(pItem->GetValue(), ATTR_CJK_FONT_LANGUAGE) );
    if (const SvxLanguageItem* pItem = rEditSet.GetItemIfSet(EE_CHAR_LANGUAGE_CTL))
        rDestSet.Put( SvxLanguageItem(pItem->GetValue(), ATTR_CTL_FONT_LANGUAGE) );

    const SvxAdjustItem* pAdjustItem = rEditSet.GetItemIfSet(EE_PARA_JUST);

    if (!pAdjustItem)
        return;

    SvxCellHorJustify eVal;
    switch ( pAdjustItem->GetAdjust() )
    {
        case SvxAdjust::Left:
            // EditEngine Default is always set in the GetAttribs() ItemSet !
            // whether left or right, is decided in text / number
            eVal = SvxCellHorJustify::Standard;
            break;
        case SvxAdjust::Right:
            eVal = SvxCellHorJustify::Right;
            break;
        case SvxAdjust::Block:
            eVal = SvxCellHorJustify::Block;
            break;
        case SvxAdjust::Center:
            eVal = SvxCellHorJustify::Center;
            break;
        case SvxAdjust::BlockLine:
            eVal = SvxCellHorJustify::Block;
            break;
        case SvxAdjust::End:
            eVal = SvxCellHorJustify::Right;
            break;
        default:
            eVal = SvxCellHorJustify::Standard;
    }
    if ( eVal != SvxCellHorJustify::Standard )
        rDestSet.Put( SvxHorJustifyItem( eVal, ATTR_HOR_JUSTIFY) );
}

void ScPatternAttr::GetFromEditItemSet( const SfxItemSet* pEditSet )
{
    if( !pEditSet )
        return;
    GetFromEditItemSet( GetItemSet(), *pEditSet );
    mxVisible.reset();
}

void ScPatternAttr::FillEditParaItems( SfxItemSet* pEditSet ) const
{
    //  already there in GetFromEditItemSet, but not in FillEditItemSet
    //  Default horizontal alignment is always implemented as left

    const SfxItemSet& rMySet = GetItemSet();

    SvxCellHorJustify eHorJust = rMySet.Get(ATTR_HOR_JUSTIFY).GetValue();

    SvxAdjust eSvxAdjust;
    switch (eHorJust)
    {
        case SvxCellHorJustify::Right:  eSvxAdjust = SvxAdjust::Right;  break;
        case SvxCellHorJustify::Center: eSvxAdjust = SvxAdjust::Center; break;
        case SvxCellHorJustify::Block:  eSvxAdjust = SvxAdjust::Block;  break;
        default:                     eSvxAdjust = SvxAdjust::Left;   break;
    }
    pEditSet->Put( SvxAdjustItem( eSvxAdjust, EE_PARA_JUST ) );
}

void ScPatternAttr::DeleteUnchanged( const ScPatternAttr* pOldAttrs )
{
    SfxItemSet& rThisSet = GetItemSet();
    const SfxItemSet& rOldSet = pOldAttrs->GetItemSet();

    const SfxPoolItem* pThisItem;
    const SfxPoolItem* pOldItem;

    for ( sal_uInt16 nSubWhich=ATTR_PATTERN_START; nSubWhich<=ATTR_PATTERN_END; nSubWhich++ )
    {
        //  only items that are set are interesting
        if ( rThisSet.GetItemState( nSubWhich, false, &pThisItem ) == SfxItemState::SET )
        {
            SfxItemState eOldState = rOldSet.GetItemState( nSubWhich, true, &pOldItem );
            if ( eOldState == SfxItemState::SET )
            {
                //  item is set in OldAttrs (or its parent) -> compare pointers
                if (SfxPoolItem::areSame( pThisItem, pOldItem ))
                {
                    rThisSet.ClearItem( nSubWhich );
                    mxVisible.reset();
                }
            }
            else if ( eOldState != SfxItemState::INVALID )
            {
                //  not set in OldAttrs -> compare item value to default item
                if ( *pThisItem == rThisSet.GetPool()->GetUserOrPoolDefaultItem( nSubWhich ) )
                {
                    rThisSet.ClearItem( nSubWhich );
                    mxVisible.reset();
                }
            }
        }
    }
}

bool ScPatternAttr::HasItemsSet( const sal_uInt16* pWhich ) const
{
    const SfxItemSet& rSet = GetItemSet();
    for (sal_uInt16 i=0; pWhich[i]; i++)
        if ( rSet.GetItemState( pWhich[i], false ) == SfxItemState::SET )
            return true;
    return false;
}

void ScPatternAttr::ClearItems( const sal_uInt16* pWhich )
{
    SfxItemSet& rSet = GetItemSet();
    for (sal_uInt16 i=0; pWhich[i]; i++)
        rSet.ClearItem(pWhich[i]);
    mxVisible.reset();
}

static SfxStyleSheetBase* lcl_CopyStyleToPool
    (
        SfxStyleSheetBase*      pSrcStyle,
        SfxStyleSheetBasePool*  pSrcPool,
        SfxStyleSheetBasePool*  pDestPool,
        const SvNumberFormatterIndexTable*     pFormatExchangeList
    )
{
    if ( !pSrcStyle || !pDestPool || !pSrcPool )
    {
        OSL_FAIL( "CopyStyleToPool: Invalid Arguments :-/" );
        return nullptr;
    }

    const OUString       aStrSrcStyle = pSrcStyle->GetName();
    const SfxStyleFamily eFamily      = pSrcStyle->GetFamily();
    SfxStyleSheetBase*   pDestStyle   = pDestPool->Find( aStrSrcStyle, eFamily );

    if ( !pDestStyle )
    {
        const OUString   aStrParent = pSrcStyle->GetParent();
        const SfxItemSet& rSrcSet = pSrcStyle->GetItemSet();

        pDestStyle = &pDestPool->Make( aStrSrcStyle, eFamily, SfxStyleSearchBits::UserDefined );
        SfxItemSet& rDestSet = pDestStyle->GetItemSet();
        rDestSet.Put( rSrcSet );

        // number format exchange list has to be handled here, too
        // (only called for cell styles)

        const SfxUInt32Item* pSrcItem;
        if ( pFormatExchangeList &&
             (pSrcItem = rSrcSet.GetItemIfSet( ATTR_VALUE_FORMAT, false )) )
        {
            sal_uInt32 nOldFormat = pSrcItem->GetValue();
            SvNumberFormatterIndexTable::const_iterator it = pFormatExchangeList->find(nOldFormat);
            if (it != pFormatExchangeList->end())
            {
                sal_uInt32 nNewFormat = it->second;
                rDestSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );
            }
        }

        // if necessary create derivative Styles, if not available:

        if ( ScResId(STR_STYLENAME_STANDARD) != aStrParent &&
             aStrSrcStyle != aStrParent &&
             !pDestPool->Find( aStrParent, eFamily ) )
        {
            lcl_CopyStyleToPool( pSrcPool->Find( aStrParent, eFamily ),
                                 pSrcPool, pDestPool, pFormatExchangeList );
        }

        pDestStyle->SetParent( aStrParent );
    }

    return pDestStyle;
}

CellAttributeHolder ScPatternAttr::MigrateToDocument( ScDocument* pDestDoc, ScDocument* pSrcDoc ) const
{
    const SfxItemSet* pSrcSet = &GetItemSet();
    ScPatternAttr* pDestPattern(new ScPatternAttr(pDestDoc->getCellAttributeHelper()));
    SfxItemSet* pDestSet(&pDestPattern->GetItemSet());

    // Copy cell pattern style to other document:
    if ( pDestDoc != pSrcDoc )
    {
        OSL_ENSURE( pStyle, "Missing Pattern-Style! :-/" );

        // if pattern in DestDoc is available, use this, otherwise copy
        // parent style to style or create if necessary and attach DestDoc

        SfxStyleSheetBase* pStyleCpy = lcl_CopyStyleToPool( pStyle,
                                                            pSrcDoc->GetStyleSheetPool(),
                                                            pDestDoc->GetStyleSheetPool(),
                                                            pDestDoc->GetFormatExchangeList() );

        pDestPattern->SetStyleSheet( static_cast<ScStyleSheet*>(pStyleCpy) );
    }

    for ( sal_uInt16 nAttrId = ATTR_PATTERN_START; nAttrId <= ATTR_PATTERN_END; nAttrId++ )
    {
        const SfxPoolItem* pSrcItem;
        SfxItemState eItemState = pSrcSet->GetItemState( nAttrId, false, &pSrcItem );
        if (eItemState==SfxItemState::SET)
        {
            std::unique_ptr<SfxPoolItem> pNewItem;

            if ( nAttrId == ATTR_VALIDDATA )
            {
                // Copy validity to the new document

                sal_uInt32 nNewIndex = 0;
                ScValidationDataList* pSrcList = pSrcDoc->GetValidationList();
                if ( pSrcList )
                {
                    sal_uInt32 nOldIndex = static_cast<const SfxUInt32Item*>(pSrcItem)->GetValue();
                    const ScValidationData* pOldData = pSrcList->GetData( nOldIndex );
                    if ( pOldData )
                        nNewIndex = pDestDoc->AddValidationEntry( *pOldData );
                }
                pNewItem.reset(new SfxUInt32Item( ATTR_VALIDDATA, nNewIndex ));
            }
            else if ( nAttrId == ATTR_VALUE_FORMAT && pDestDoc->GetFormatExchangeList() )
            {
                //  Number format to Exchange List

                sal_uInt32 nOldFormat = static_cast<const SfxUInt32Item*>(pSrcItem)->GetValue();
                SvNumberFormatterIndexTable::const_iterator it = pDestDoc->GetFormatExchangeList()->find(nOldFormat);
                if (it != pDestDoc->GetFormatExchangeList()->end())
                {
                    sal_uInt32 nNewFormat = it->second;
                    pNewItem.reset(new SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ));
                }
            }

            if ( pNewItem )
            {
                pDestSet->Put(std::move(pNewItem));
            }
            else
                pDestSet->Put(*pSrcItem);
        }
    }

    return CellAttributeHolder(pDestPattern, true);
}

bool ScPatternAttr::IsVisible() const
{
    if (!mxVisible.has_value())
        mxVisible = CalcVisible();
    return *mxVisible;
}

bool ScPatternAttr::CalcVisible() const
{
    const SfxItemSet& rSet = GetItemSet();

    if ( const SvxBrushItem* pItem = rSet.GetItemIfSet( ATTR_BACKGROUND ) )
        if ( pItem->GetColor() != COL_TRANSPARENT )
            return true;

    if ( const SvxBoxItem* pBoxItem = rSet.GetItemIfSet( ATTR_BORDER ) )
    {
        if ( pBoxItem->GetTop() || pBoxItem->GetBottom() ||
             pBoxItem->GetLeft() || pBoxItem->GetRight() )
            return true;
    }

    if ( const SvxLineItem* pItem = rSet.GetItemIfSet( ATTR_BORDER_TLBR ) )
        if( pItem->GetLine() )
            return true;

    if ( const SvxLineItem* pItem = rSet.GetItemIfSet( ATTR_BORDER_BLTR ) )
        if( pItem->GetLine() )
            return true;

    if ( const SvxShadowItem* pItem = rSet.GetItemIfSet( ATTR_SHADOW ) )
        if ( pItem->GetLocation() != SvxShadowLocation::NONE )
            return true;

    return false;
}

bool ScPatternAttr::IsVisibleEqual( const ScPatternAttr& rOther ) const
{
    // This method is hot, so we do an optimised comparison here, by
    // walking the two itemsets in parallel, avoiding doing repeated searches.
    auto IsInterestingWhich = [](sal_uInt16 n)
    {
        return n == ATTR_BORDER_TLBR || n == ATTR_BORDER_BLTR || n == ATTR_BACKGROUND
               || n == ATTR_BORDER || n == ATTR_SHADOW;
    };
    SfxWhichIter aIter1(GetItemSet());
    SfxWhichIter aIter2(rOther.GetItemSet());
    sal_uInt16 nWhich1 = aIter1.FirstWhich();
    sal_uInt16 nWhich2 = aIter2.FirstWhich();
    for (;;)
    {
        while (nWhich1 != nWhich2)
        {
            SfxWhichIter* pIterToIncrement;
            sal_uInt16* pSmallerWhich;
            if (nWhich1 == 0 || nWhich1 > nWhich2)
            {
                pSmallerWhich = &nWhich2;
                pIterToIncrement = &aIter2;
            }
            else
            {
                pSmallerWhich = &nWhich1;
                pIterToIncrement = &aIter1;
            }

            if (IsInterestingWhich(*pSmallerWhich))
            {
                // the iter with larger which has already passed this point, and has no interesting
                // item available in the other - so indeed these are unequal
                return false;
            }

            *pSmallerWhich = pIterToIncrement->NextWhich();
        }

        // Here nWhich1 == nWhich2

        if (!nWhich1 /* && !nWhich2*/)
            return true;

        if (IsInterestingWhich(nWhich1))
        {
            const SfxPoolItem* pItem1 = nullptr;
            const SfxPoolItem* pItem2 = nullptr;
            SfxItemState state1 = aIter1.GetItemState(true, &pItem1);
            SfxItemState state2 = aIter2.GetItemState(true, &pItem2);
            if (state1 != state2
                && (state1 < SfxItemState::DEFAULT || state2 < SfxItemState::DEFAULT))
                return false;
            if (!SfxPoolItem::areSame(pItem1, pItem2))
                return false;
        }
        nWhich1 = aIter1.NextWhich();
        nWhich2 = aIter2.NextWhich();
    }
    //TODO: also here only check really visible values !!!
}

const OUString* ScPatternAttr::GetStyleName() const
{
    return pName ? &*pName : ( pStyle ? &pStyle->GetName() : nullptr );
}

void ScPatternAttr::SetStyleSheet( ScStyleSheet* pNewStyle, bool bClearDirectFormat )
{
    if (pNewStyle)
    {
        SfxItemSet&       rPatternSet = GetItemSet();
        const SfxItemSet& rStyleSet = pNewStyle->GetItemSet();

        if (bClearDirectFormat)
        {
            for (sal_uInt16 i=ATTR_PATTERN_START; i<=ATTR_PATTERN_END; i++)
            {
                if (rStyleSet.GetItemState(i) == SfxItemState::SET)
                    rPatternSet.ClearItem(i);
            }
        }
        rPatternSet.SetParent(&pNewStyle->GetItemSet());
        pStyle = pNewStyle;
        pName.reset();
    }
    else
    {
        OSL_FAIL( "ScPatternAttr::SetStyleSheet( NULL ) :-|" );
        GetItemSet().SetParent(nullptr);
        pStyle = nullptr;
    }
    mxVisible.reset();
}

void ScPatternAttr::UpdateStyleSheet(const ScDocument& rDoc)
{
    if (pName)
    {
        pStyle = static_cast<ScStyleSheet*>(rDoc.GetStyleSheetPool()->Find(*pName, SfxStyleFamily::Para));

        //  use Standard if Style is not found,
        //  to avoid empty display in Toolbox-Controller
        //  Assumes that "Standard" is always the 1st entry!
        if (!pStyle)
        {
            std::unique_ptr<SfxStyleSheetIterator> pIter = rDoc.GetStyleSheetPool()->CreateIterator(SfxStyleFamily::Para);
            pStyle = dynamic_cast< ScStyleSheet* >(pIter->First());
        }

        if (pStyle)
        {
            GetItemSet().SetParent(&pStyle->GetItemSet());
            pName.reset();
        }
    }
    else
        pStyle = nullptr;
    mxVisible.reset();
}

void ScPatternAttr::StyleToName()
{
    // Style was deleted, remember name:

    if ( pStyle )
    {
        pName = pStyle->GetName();
        pStyle = nullptr;
        GetItemSet().SetParent( nullptr );
        mxVisible.reset();
    }
}

bool ScPatternAttr::IsSymbolFont() const
{
    if( const SvxFontItem* pItem = GetItemSet().GetItemIfSet( ATTR_FONT ) )
        return pItem->GetCharSet() == RTL_TEXTENCODING_SYMBOL;
    else
        return false;
}

namespace {

sal_uInt32 getNumberFormatKey(const SfxItemSet& rSet)
{
    return rSet.Get(ATTR_VALUE_FORMAT).GetValue();
}

LanguageType getLanguageType(const SfxItemSet& rSet)
{
    return rSet.Get(ATTR_LANGUAGE_FORMAT).GetLanguage();
}

}

sal_uInt32 ScPatternAttr::GetNumberFormat( SvNumberFormatter* pFormatter ) const
{
    sal_uInt32 nFormat = getNumberFormatKey(GetItemSet());
    LanguageType eLang = getLanguageType(GetItemSet());
    if ( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && eLang == LANGUAGE_SYSTEM )
        ;       // it remains as it is
    else if ( pFormatter )
        nFormat = pFormatter->GetFormatForLanguageIfBuiltIn( nFormat, eLang );
    return nFormat;
}

sal_uInt32 ScPatternAttr::GetNumberFormat( const ScInterpreterContext& rContext ) const
{
    sal_uInt32 nFormat = getNumberFormatKey(GetItemSet());
    LanguageType eLang = getLanguageType(GetItemSet());
    if ( nFormat < SV_COUNTRY_LANGUAGE_OFFSET && eLang == LANGUAGE_SYSTEM )
        ;       // it remains as it is
    else
        nFormat = rContext.NFGetFormatForLanguageIfBuiltIn( nFormat, eLang );
    return nFormat;
}

// the same if conditional formatting is in play:

sal_uInt32 ScPatternAttr::GetNumberFormat( SvNumberFormatter* pFormatter,
                                           const SfxItemSet* pCondSet ) const
{
    assert(pFormatter);
    if (!pCondSet)
        return GetNumberFormat(pFormatter);

    // Conditional format takes precedence over style and even hard format.

    sal_uInt32 nFormat;
    LanguageType eLang;
    if (pCondSet->GetItemState(ATTR_VALUE_FORMAT) == SfxItemState::SET )
    {
        nFormat = getNumberFormatKey(*pCondSet);
        if (pCondSet->GetItemState(ATTR_LANGUAGE_FORMAT) == SfxItemState::SET)
            eLang = getLanguageType(*pCondSet);
        else
            eLang = getLanguageType(GetItemSet());
    }
    else
    {
        nFormat = getNumberFormatKey(GetItemSet());
        eLang = getLanguageType(GetItemSet());
    }

    return pFormatter->GetFormatForLanguageIfBuiltIn(nFormat, eLang);
}

sal_uInt32 ScPatternAttr::GetNumberFormat( const ScInterpreterContext& rContext,
                                           const SfxItemSet* pCondSet ) const
{
    if (!pCondSet)
        return GetNumberFormat(rContext);

    // Conditional format takes precedence over style and even hard format.

    sal_uInt32 nFormat;
    LanguageType eLang;
    if (pCondSet->GetItemState(ATTR_VALUE_FORMAT) == SfxItemState::SET )
    {
        nFormat = getNumberFormatKey(*pCondSet);
        if (pCondSet->GetItemState(ATTR_LANGUAGE_FORMAT) == SfxItemState::SET)
            eLang = getLanguageType(*pCondSet);
        else
            eLang = getLanguageType(GetItemSet());
    }
    else
    {
        nFormat = getNumberFormatKey(GetItemSet());
        eLang = getLanguageType(GetItemSet());
    }

    return rContext.NFGetFormatForLanguageIfBuiltIn(nFormat, eLang);
}

const SfxPoolItem& ScPatternAttr::GetItem( sal_uInt16 nWhich, const SfxItemSet& rItemSet, const SfxItemSet* pCondSet )
{
    const SfxPoolItem* pCondItem;
    if ( pCondSet && pCondSet->GetItemState( nWhich, true, &pCondItem ) == SfxItemState::SET )
        return *pCondItem;
    return rItemSet.Get(nWhich);
}

const SfxPoolItem& ScPatternAttr::GetItem( sal_uInt16 nSubWhich, const SfxItemSet* pCondSet ) const
{
    return GetItem( nSubWhich, GetItemSet(), pCondSet );
}

//  GetRotateVal is tested before ATTR_ORIENTATION

Degree100 ScPatternAttr::GetRotateVal( const SfxItemSet* pCondSet ) const
{
    Degree100 nAttrRotate(0);
    if ( GetCellOrientation() == SvxCellOrientation::Standard )
    {
        bool bRepeat = ( GetItem(ATTR_HOR_JUSTIFY, pCondSet).
                            GetValue() == SvxCellHorJustify::Repeat );
        // ignore orientation/rotation if "repeat" is active
        if ( !bRepeat )
            nAttrRotate = GetItem( ATTR_ROTATE_VALUE, pCondSet ).GetValue();
    }
    return nAttrRotate;
}

ScRotateDir ScPatternAttr::GetRotateDir( const SfxItemSet* pCondSet ) const
{
    ScRotateDir nRet = ScRotateDir::NONE;

    Degree100 nAttrRotate = GetRotateVal( pCondSet );
    if ( nAttrRotate )
    {
        SvxRotateMode eRotMode = GetItem(ATTR_ROTATE_MODE, pCondSet).GetValue();

        if ( eRotMode == SVX_ROTATE_MODE_STANDARD || nAttrRotate == 18000_deg100 )
            nRet = ScRotateDir::Standard;
        else if ( eRotMode == SVX_ROTATE_MODE_CENTER )
            nRet = ScRotateDir::Center;
        else if ( eRotMode == SVX_ROTATE_MODE_TOP || eRotMode == SVX_ROTATE_MODE_BOTTOM )
        {
            Degree100 nRot180 = nAttrRotate % 18000_deg100;     // 1/100 degrees
            if ( nRot180 == 9000_deg100 )
                nRet = ScRotateDir::Center;
            else if ( ( eRotMode == SVX_ROTATE_MODE_TOP && nRot180 < 9000_deg100 ) ||
                      ( eRotMode == SVX_ROTATE_MODE_BOTTOM && nRot180 > 9000_deg100 ) )
                nRet = ScRotateDir::Left;
            else
                nRet = ScRotateDir::Right;
        }
    }

    return nRet;
}

void ScPatternAttr::SetPAKey(sal_uInt64 nKey)
{
    mnPAKey = nKey;
}

sal_uInt64 ScPatternAttr::GetPAKey() const
{
    return mnPAKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
