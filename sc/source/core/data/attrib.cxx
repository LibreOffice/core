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

#include <com/sun/star/util/CellProtection.hpp>

#include <scitems.hxx>

#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/borderline.hxx>
#include <editeng/itemtype.hxx>
#include <svl/itempool.hxx>

#include <libxml/xmlwriter.h>

#include <attrib.hxx>
#include <global.hxx>
#include <editutil.hxx>
#include <mid.h>
#include <globstr.hrc>
#include <scresid.hxx>
#include <textuno.hxx>

using namespace com::sun::star;


SfxPoolItem* ScProtectionAttr::CreateDefault() { return new ScProtectionAttr; }

/**
 * General Help Function
 */
bool ScHasPriority( const ::editeng::SvxBorderLine* pThis, const ::editeng::SvxBorderLine* pOther )
{

    if (!pThis)
        return false;
    if (!pOther)
        return true;

    sal_uInt16 nThisSize = pThis->GetScaledWidth();
    sal_uInt16 nOtherSize = pOther->GetScaledWidth();

    if (nThisSize > nOtherSize)
        return true;
    else if (nThisSize < nOtherSize)
        return false;
    else
    {
        if ( pOther->GetInWidth() && !pThis->GetInWidth() )
            return true;
        else if ( pThis->GetInWidth() && !pOther->GetInWidth() )
            return false;
        else
        {
            return true; // FIXME: What is this?
        }
    }
}

/** Item - Implementations */

/**
 * Merge
 */
ScMergeAttr::ScMergeAttr():
    SfxPoolItem(ATTR_MERGE),
    nColMerge(0),
    nRowMerge(0)
{}

ScMergeAttr::ScMergeAttr( SCCOL nCol, SCROW nRow):
    SfxPoolItem(ATTR_MERGE),
    nColMerge(nCol),
    nRowMerge(nRow)
{}

ScMergeAttr::ScMergeAttr(const ScMergeAttr& rItem):
    SfxPoolItem(ATTR_MERGE)
{
    nColMerge = rItem.nColMerge;
    nRowMerge = rItem.nRowMerge;
}

ScMergeAttr::~ScMergeAttr()
{
}

bool ScMergeAttr::operator==( const SfxPoolItem& rItem ) const
{
    return SfxPoolItem::operator==(rItem)
             && (nColMerge == static_cast<const ScMergeAttr&>(rItem).nColMerge)
             && (nRowMerge == static_cast<const ScMergeAttr&>(rItem).nRowMerge);
}

ScMergeAttr* ScMergeAttr::Clone( SfxItemPool * ) const
{
    return new ScMergeAttr(*this);
}

void ScMergeAttr::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ScMergeAttr"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("col-merge"), BAD_CAST(OString::number(GetColMerge()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("row-merge"), BAD_CAST(OString::number(GetRowMerge()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("merged"), BAD_CAST(OString::boolean(IsMerged()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/**
 * MergeFlag
 */
ScMergeFlagAttr::ScMergeFlagAttr():
    SfxInt16Item(ATTR_MERGE_FLAG, 0)
{
}

ScMergeFlagAttr::ScMergeFlagAttr(ScMF nFlags):
    SfxInt16Item(ATTR_MERGE_FLAG, static_cast<sal_Int16>(nFlags))
{
}

ScMergeFlagAttr::~ScMergeFlagAttr()
{
}

ScMergeFlagAttr* ScMergeFlagAttr::Clone(SfxItemPool *) const
{
    return new ScMergeFlagAttr(*this);
}

bool ScMergeFlagAttr::HasPivotButton() const
{
    return bool(GetValue() & ScMF::Button);
}

bool ScMergeFlagAttr::HasPivotPopupButton() const
{
    return bool(GetValue() & ScMF::ButtonPopup);
}

bool ScMergeFlagAttr::HasPivotToggle() const
{
    auto nFlags = GetValue();
    return (nFlags & ScMF::DpCollapse) || (nFlags & ScMF::DpExpand);
}

bool ScMergeFlagAttr::HasPivotMultiFieldPopupButton() const
{
    return bool(GetValue() & ScMF::ButtonPopup2);
}

void ScMergeFlagAttr::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ScMergeFlagAttr"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("overlapped"), BAD_CAST(OString::boolean(IsOverlapped()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("hor_overlapped"), BAD_CAST(OString::boolean(IsHorOverlapped()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("ver_overlapped"), BAD_CAST(OString::boolean(IsVerOverlapped()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("autofilter"), BAD_CAST(OString::boolean(HasAutoFilter()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("scenario"), BAD_CAST(OString::boolean(IsScenario()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pivot-button"), BAD_CAST(OString::boolean(HasPivotButton()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pivot-popup-button"), BAD_CAST(OString::boolean(HasPivotPopupButton()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/**
 * Protection
 */
ScProtectionAttr::ScProtectionAttr():
    SfxPoolItem(ATTR_PROTECTION),
    bProtection(true),
    bHideFormula(false),
    bHideCell(false),
    bHidePrint(false)
{
}

ScProtectionAttr::ScProtectionAttr( bool bProtect, bool bHFormula,
                                    bool bHCell, bool bHPrint):
    SfxPoolItem(ATTR_PROTECTION),
    bProtection(bProtect),
    bHideFormula(bHFormula),
    bHideCell(bHCell),
    bHidePrint(bHPrint)
{
}

ScProtectionAttr::ScProtectionAttr(const ScProtectionAttr& rItem):
    SfxPoolItem(ATTR_PROTECTION)
{
    bProtection  = rItem.bProtection;
    bHideFormula = rItem.bHideFormula;
    bHideCell    = rItem.bHideCell;
    bHidePrint   = rItem.bHidePrint;
}

ScProtectionAttr::~ScProtectionAttr()
{
}

bool ScProtectionAttr::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId  )
    {
        case 0 :
        {
            util::CellProtection aProtection;
            aProtection.IsLocked        = bProtection;
            aProtection.IsFormulaHidden = bHideFormula;
            aProtection.IsHidden        = bHideCell;
            aProtection.IsPrintHidden   = bHidePrint;
            rVal <<= aProtection;
            break;
        }
        case MID_1 :
            rVal <<= bProtection; break;
        case MID_2 :
            rVal <<= bHideFormula; break;
        case MID_3 :
            rVal <<= bHideCell; break;
        case MID_4 :
            rVal <<= bHidePrint; break;
        default:
            OSL_FAIL("Wrong MemberID!");
            return false;
    }

    return true;
}

bool ScProtectionAttr::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bRet = false;
    bool bVal = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            util::CellProtection aProtection;
            if ( rVal >>= aProtection )
            {
                bProtection  = aProtection.IsLocked;
                bHideFormula = aProtection.IsFormulaHidden;
                bHideCell    = aProtection.IsHidden;
                bHidePrint   = aProtection.IsPrintHidden;
                bRet = true;
            }
            else
            {
                OSL_FAIL("exception - wrong argument");
            }
            break;
        }
        case MID_1 :
            bRet = (rVal >>= bVal); if (bRet) bProtection=bVal; break;
        case MID_2 :
            bRet = (rVal >>= bVal); if (bRet) bHideFormula=bVal; break;
        case MID_3 :
            bRet = (rVal >>= bVal); if (bRet) bHideCell=bVal; break;
        case MID_4 :
            bRet = (rVal >>= bVal); if (bRet) bHidePrint=bVal; break;
        default:
            OSL_FAIL("Wrong MemberID!");
    }

    return bRet;
}

OUString ScProtectionAttr::GetValueText() const
{
    const OUString aStrYes ( ScResId(STR_YES) );
    const OUString aStrNo  ( ScResId(STR_NO) );

    const OUString aValue  = "("
        + (bProtection ? aStrYes : aStrNo)
        + ","
        + (bHideFormula ? aStrYes : aStrNo)
        + ","
        + (bHideCell ? aStrYes : aStrNo)
        + ","
        + (bHidePrint ? aStrYes : aStrNo)
        + ")";

    return aValue;
}

bool ScProtectionAttr::GetPresentation
    (
        SfxItemPresentation ePres,
        MapUnit /* eCoreMetric */,
        MapUnit /* ePresMetric */,
        OUString& rText,
        const IntlWrapper& /* rIntl */
    ) const
{
    const OUString aStrYes ( ScResId(STR_YES) );
    const OUString aStrNo  ( ScResId(STR_NO) );

    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = GetValueText();
            break;

        case SfxItemPresentation::Complete:
            rText  = ScResId(STR_PROTECTION)
                + ": "
                + (bProtection ? aStrYes : aStrNo)
                + ", "
                + ScResId(STR_FORMULAS)
                + ": "
                + (!bHideFormula ? aStrYes : aStrNo)
                + ", "
                + ScResId(STR_HIDE)
                + ": "
                + (bHideCell ? aStrYes : aStrNo)
                + ", "
                + ScResId(STR_PRINT)
                + ": "
                + (!bHidePrint ? aStrYes : aStrNo);
            break;

        default: break;
    }

    return true;
}

bool ScProtectionAttr::operator==( const SfxPoolItem& rItem ) const
{
    return SfxPoolItem::operator==(rItem)
             && (bProtection == static_cast<const ScProtectionAttr&>(rItem).bProtection)
             && (bHideFormula == static_cast<const ScProtectionAttr&>(rItem).bHideFormula)
             && (bHideCell == static_cast<const ScProtectionAttr&>(rItem).bHideCell)
             && (bHidePrint == static_cast<const ScProtectionAttr&>(rItem).bHidePrint);
}

ScProtectionAttr* ScProtectionAttr::Clone( SfxItemPool * ) const
{
    return new ScProtectionAttr(*this);
}

void ScProtectionAttr::SetProtection( bool bProtect)
{
    bProtection =  bProtect;
}

void ScProtectionAttr::SetHideFormula( bool bHFormula)
{
    bHideFormula = bHFormula;
}

void ScProtectionAttr::SetHideCell( bool bHCell)
{
    bHideCell = bHCell;
}

void ScProtectionAttr::SetHidePrint( bool bHPrint)
{
    bHidePrint = bHPrint;
}

void ScProtectionAttr::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ScProtectionAttr"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("protection"), BAD_CAST(OString::boolean(GetProtection()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("hide-formula"), BAD_CAST(OString::boolean(GetHideFormula()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("hide-cell"), BAD_CAST(OString::boolean(GetHideCell()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("hide-print"), BAD_CAST(OString::boolean(GetHidePrint()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/**
 * ScPageHFItem - Dates from the Head and Foot lines
 */
ScPageHFItem::ScPageHFItem( sal_uInt16 nWhichP )
    :   SfxPoolItem ( nWhichP )
{
}

ScPageHFItem::ScPageHFItem( const ScPageHFItem& rItem )
    :   SfxPoolItem ( rItem )
{
    if ( rItem.pLeftArea )
        pLeftArea = rItem.pLeftArea->Clone();
    if ( rItem.pCenterArea )
        pCenterArea = rItem.pCenterArea->Clone();
    if ( rItem.pRightArea )
        pRightArea = rItem.pRightArea->Clone();
}

ScPageHFItem::~ScPageHFItem()
{
}

bool ScPageHFItem::QueryValue( uno::Any& rVal, sal_uInt8 /* nMemberId */ ) const
{
    rtl::Reference<ScHeaderFooterContentObj> xContent =
        new ScHeaderFooterContentObj();
    xContent->Init(pLeftArea.get(), pCenterArea.get(), pRightArea.get());

    uno::Reference<sheet::XHeaderFooterContent> xCont(xContent);

    rVal <<= xCont;
    return true;
}

bool ScPageHFItem::PutValue( const uno::Any& rVal, sal_uInt8 /* nMemberId */ )
{
    bool bRet = false;
    uno::Reference<sheet::XHeaderFooterContent> xContent;
    if ( rVal >>= xContent )
    {
        if ( xContent.is() )
        {
            rtl::Reference<ScHeaderFooterContentObj> pImp =
                    ScHeaderFooterContentObj::getImplementation( xContent );
            if (pImp.is())
            {
                const EditTextObject* pImpLeft = pImp->GetLeftEditObject();
                pLeftArea.reset();
                if (pImpLeft)
                    pLeftArea = pImpLeft->Clone();

                const EditTextObject* pImpCenter = pImp->GetCenterEditObject();
                pCenterArea.reset();
                if (pImpCenter)
                    pCenterArea = pImpCenter->Clone();

                const EditTextObject* pImpRight = pImp->GetRightEditObject();
                pRightArea.reset();
                if (pImpRight)
                    pRightArea = pImpRight->Clone();

                if ( !pLeftArea || !pCenterArea || !pRightArea )
                {
                    // no Text with Null are left
                    ScEditEngineDefaulter aEngine( EditEngine::CreatePool().get(), true );
                    if (!pLeftArea)
                        pLeftArea = aEngine.CreateTextObject();
                    if (!pCenterArea)
                        pCenterArea = aEngine.CreateTextObject();
                    if (!pRightArea)
                        pRightArea = aEngine.CreateTextObject();
                }

                bRet = true;
            }
        }
    }

    if (!bRet)
    {
        OSL_FAIL("exception - wrong argument");
    }

    return true;
}

bool ScPageHFItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScPageHFItem& r = static_cast<const ScPageHFItem&>(rItem);

    return    ScGlobal::EETextObjEqual(pLeftArea.get(),   r.pLeftArea.get())
           && ScGlobal::EETextObjEqual(pCenterArea.get(), r.pCenterArea.get())
           && ScGlobal::EETextObjEqual(pRightArea.get(),  r.pRightArea.get());
}

ScPageHFItem* ScPageHFItem::Clone( SfxItemPool* ) const
{
    return new ScPageHFItem( *this );
}

void ScPageHFItem::SetLeftArea( const EditTextObject& rNew )
{
    pLeftArea = rNew.Clone();
}

void ScPageHFItem::SetCenterArea( const EditTextObject& rNew )
{
    pCenterArea = rNew.Clone();
}

void ScPageHFItem::SetRightArea( const EditTextObject& rNew )
{
    pRightArea = rNew.Clone();
}
void ScPageHFItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ScPageHFItem"));
    GetLeftArea()->dumpAsXml(pWriter);
    GetCenterArea()->dumpAsXml(pWriter);
    GetRightArea()->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

/**
 * ScViewObjectModeItem - Display Mode of View Objects
 */
ScViewObjectModeItem::ScViewObjectModeItem( sal_uInt16 nWhichP )
    : SfxEnumItem( nWhichP, VOBJ_MODE_SHOW )
{
}

ScViewObjectModeItem::ScViewObjectModeItem( sal_uInt16 nWhichP, ScVObjMode eMode )
    : SfxEnumItem( nWhichP, eMode )
{
}

ScViewObjectModeItem::~ScViewObjectModeItem()
{
}

bool ScViewObjectModeItem::GetPresentation
(
    SfxItemPresentation ePres,
    MapUnit             /* eCoreUnit */,
    MapUnit             /* ePresUnit */,
    OUString&           rText,
    const IntlWrapper& /* rIntl */
)   const
{
    OUString aDel(u": "_ustr);
    rText.clear();

    switch ( ePres )
    {
        case SfxItemPresentation::Complete:
            switch( Which() )
            {
                case SID_SCATTR_PAGE_CHARTS:
                rText = ScResId(STR_VOBJ_CHART) + aDel;
                break;

                case SID_SCATTR_PAGE_OBJECTS:
                rText = ScResId(STR_VOBJ_OBJECT) + aDel;
                break;

                case SID_SCATTR_PAGE_DRAWINGS:
                rText = ScResId(STR_VOBJ_DRAWINGS) + aDel;
                break;

                default: break;
            }
            [[fallthrough]];
        case SfxItemPresentation::Nameless:
            if (GetValue() == VOBJ_MODE_SHOW)
                rText += ScResId(STR_VOBJ_MODE_SHOW);
            else
                rText += ScResId(STR_VOBJ_MODE_HIDE);
            return true;

        default: break;
            // added to avoid warnings
    }

    return false;
}

sal_uInt16 ScViewObjectModeItem::GetValueCount() const
{
    return 2;
}

ScViewObjectModeItem* ScViewObjectModeItem::Clone( SfxItemPool* ) const
{
    return new ScViewObjectModeItem( *this );
}

ScPageScaleToItem::ScPageScaleToItem() :
    SfxPoolItem( ATTR_PAGE_SCALETO ),
    mnWidth( 0 ),
    mnHeight( 0 )
{
}

ScPageScaleToItem::ScPageScaleToItem( sal_uInt16 nWidth, sal_uInt16 nHeight ) :
    SfxPoolItem( ATTR_PAGE_SCALETO ),
    mnWidth( nWidth ),
    mnHeight( nHeight )
{
}

ScPageScaleToItem::~ScPageScaleToItem()
{
}

ScPageScaleToItem* ScPageScaleToItem::Clone( SfxItemPool* ) const
{
    return new ScPageScaleToItem( *this );
}

bool ScPageScaleToItem::operator==( const SfxPoolItem& rCmp ) const
{
    assert(SfxPoolItem::operator==(rCmp));
    const ScPageScaleToItem& rPageCmp = static_cast< const ScPageScaleToItem& >( rCmp );
    return (mnWidth == rPageCmp.mnWidth) && (mnHeight == rPageCmp.mnHeight);
}

namespace {
void lclAppendScalePageCount( OUString& rText, sal_uInt16 nPages )
{
    rText += ": ";
    if( nPages )
    {
        OUString aPages(ScResId(STR_SCATTR_PAGE_SCALE_PAGES, nPages));
        rText += aPages.replaceFirst( "%1", OUString::number( nPages ) );
    }
    else
        rText += ScResId( STR_SCATTR_PAGE_SCALE_AUTO );
}
} // namespace

bool ScPageScaleToItem::GetPresentation(
        SfxItemPresentation ePres, MapUnit, MapUnit, OUString& rText, const IntlWrapper& ) const
{
    rText.clear();
    if( !IsValid())
        return false;

    OUString aName( ScResId( STR_SCATTR_PAGE_SCALETO ) );
    OUString aValue( ScResId( STR_SCATTR_PAGE_SCALE_WIDTH ) );
    lclAppendScalePageCount( aValue, mnWidth );
    aValue += ", " + ScResId( STR_SCATTR_PAGE_SCALE_HEIGHT );
    lclAppendScalePageCount( aValue, mnHeight );

    switch( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = aValue;
            return true;

        case SfxItemPresentation::Complete:
            rText = aName + " (" + aValue + ")";
            return true;

        default:
            OSL_FAIL( "ScPageScaleToItem::GetPresentation - unknown presentation mode" );
    }
    return false;
}

bool ScPageScaleToItem::QueryValue( uno::Any& rAny, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    switch( nMemberId )
    {
        case SC_MID_PAGE_SCALETO_WIDTH:     rAny <<= mnWidth;   break;
        case SC_MID_PAGE_SCALETO_HEIGHT:    rAny <<= mnHeight;  break;
        default:
            OSL_FAIL( "ScPageScaleToItem::QueryValue - unknown member ID" );
            bRet = false;
    }
    return bRet;
}

bool ScPageScaleToItem::PutValue( const uno::Any& rAny, sal_uInt8 nMemberId )
{
    bool bRet = false;
    switch( nMemberId )
    {
        case SC_MID_PAGE_SCALETO_WIDTH:     bRet = rAny >>= mnWidth;    break;
        case SC_MID_PAGE_SCALETO_HEIGHT:    bRet = rAny >>= mnHeight;   break;
        default:
            OSL_FAIL( "ScPageScaleToItem::PutValue - unknown member ID" );
    }
    return bRet;
}
void ScPageScaleToItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ScPageScaleToItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("width"), BAD_CAST(OString::number(GetWidth()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("height"), BAD_CAST(OString::number(GetHeight()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

ScCondFormatItem::ScCondFormatItem():
    SfxPoolItem( ATTR_CONDITIONAL )
{
}

ScCondFormatItem::ScCondFormatItem( sal_uInt32 nIndex ):
    SfxPoolItem( ATTR_CONDITIONAL )
{
    maIndex.insert(nIndex);
}

ScCondFormatItem::ScCondFormatItem( const ScCondFormatIndexes& rIndex ):
    SfxPoolItem( ATTR_CONDITIONAL ),
    maIndex( rIndex )
{
}

ScCondFormatItem::ScCondFormatItem( ScCondFormatIndexes&& aIndex ) noexcept:
    SfxPoolItem( ATTR_CONDITIONAL ),
    maIndex( std::move(aIndex) )
{
}

ScCondFormatItem::~ScCondFormatItem()
{
}

bool ScCondFormatItem::operator==( const SfxPoolItem& rCmp ) const
{
    if (!SfxPoolItem::operator==(rCmp))
        return false;
    auto const & other = static_cast<const ScCondFormatItem&>(rCmp);
    if (maIndex.empty() && other.maIndex.empty())
        return true;
    // memcmp is faster than operator== on std::vector
    return maIndex.size() == other.maIndex.size()
        && memcmp(&maIndex.front(), &other.maIndex.front(), maIndex.size() * sizeof(sal_uInt32)) == 0;
}

ScCondFormatItem* ScCondFormatItem::Clone(SfxItemPool*) const
{
    return new ScCondFormatItem(maIndex);
}

void ScCondFormatItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("ScCondFormatItem"));
    for (const auto& nItem : maIndex)
    {
        std::string aStrVal = std::to_string(nItem);
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST(aStrVal.c_str()));
        (void)xmlTextWriterEndElement(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);
}

ScRotateValueItem::ScRotateValueItem(Degree100 nAngle)
    : SdrAngleItem(ATTR_ROTATE_VALUE, nAngle)
{
}

ScRotateValueItem* ScRotateValueItem::Clone(SfxItemPool*) const
{
    return new ScRotateValueItem(GetValue());
}

bool ScRotateValueItem::GetPresentation(SfxItemPresentation ePresentation,
                                        MapUnit eCoreMetric, MapUnit ePresMetric,
                                        OUString& rText,
                                        const IntlWrapper& rWrapper) const
{
    bool bRet = SdrAngleItem::GetPresentation(SfxItemPresentation::Nameless, eCoreMetric, ePresMetric, rText, rWrapper);
    if (bRet && ePresentation == SfxItemPresentation::Complete)
        rText = ScResId(STR_TEXTORIENTANGLE) + " " + rText;
    return bRet;
}

ScShrinkToFitCell::ScShrinkToFitCell(bool bShrink)
    : SfxBoolItem(ATTR_SHRINKTOFIT, bShrink)
{
}

ScShrinkToFitCell* ScShrinkToFitCell::Clone(SfxItemPool*) const
{
    return new ScShrinkToFitCell(GetValue());
}

bool ScShrinkToFitCell::GetPresentation(SfxItemPresentation,
                                        MapUnit, MapUnit,
                                        OUString& rText,
                                        const IntlWrapper&) const
{
    TranslateId pId = GetValue() ? STR_SHRINKTOFITCELL_ON : STR_SHRINKTOFITCELL_OFF;
    rText = ScResId(pId);
    return true;
}

ScVerticalStackCell::ScVerticalStackCell(bool bStack)
    : SfxBoolItem(ATTR_STACKED, bStack)
{
}

ScVerticalStackCell* ScVerticalStackCell::Clone(SfxItemPool*) const
{
    return new ScVerticalStackCell(GetValue());
}

bool ScVerticalStackCell::GetPresentation(SfxItemPresentation,
                                          MapUnit, MapUnit,
                                          OUString& rText,
                                          const IntlWrapper&) const
{
    TranslateId pId = GetValue() ? STR_VERTICALSTACKCELL_ON : STR_VERTICALSTACKCELL_OFF;
    rText = ScResId(pId);
    return true;
}

ScLineBreakCell::ScLineBreakCell(bool bStack)
    : SfxBoolItem(ATTR_LINEBREAK, bStack)
{
}

ScLineBreakCell* ScLineBreakCell::Clone(SfxItemPool*) const
{
    return new ScLineBreakCell(GetValue());
}

bool ScLineBreakCell::GetPresentation(SfxItemPresentation,
                                      MapUnit, MapUnit,
                                      OUString& rText,
                                      const IntlWrapper&) const
{
    TranslateId pId = GetValue() ? STR_LINEBREAKCELL_ON : STR_LINEBREAKCELL_OFF;
    rText = ScResId(pId);
    return true;
}

ScHyphenateCell::ScHyphenateCell(bool bHyphenate)
    : SfxBoolItem(ATTR_HYPHENATE, bHyphenate)
{
}

ScHyphenateCell* ScHyphenateCell::Clone(SfxItemPool*) const
{
    return new ScHyphenateCell(GetValue());
}

bool ScHyphenateCell::GetPresentation(SfxItemPresentation,
                                      MapUnit, MapUnit,
                                      OUString& rText,
                                      const IntlWrapper&) const
{
    TranslateId pId = GetValue() ? STR_HYPHENATECELL_ON : STR_HYPHENATECELL_OFF;
    rText = ScResId(pId);
    return true;
}

ScIndentItem::ScIndentItem(sal_uInt16 nIndent)
    : SfxUInt16Item(ATTR_INDENT, nIndent)
{
}

ScIndentItem* ScIndentItem::Clone(SfxItemPool*) const
{
    return new ScIndentItem(GetValue());
}

bool ScIndentItem::GetPresentation(SfxItemPresentation ePres,
                                   MapUnit eCoreUnit, MapUnit,
                                   OUString& rText,
                                   const IntlWrapper& rIntl) const
{
    auto nValue = GetValue();

    switch (ePres)
    {
        case SfxItemPresentation::Complete:
            rText = ScResId(STR_INDENTCELL);
            [[fallthrough]];
        case SfxItemPresentation::Nameless:
            rText += GetMetricText( nValue, eCoreUnit, MapUnit::MapPoint, &rIntl ) +
                  " " + EditResId(GetMetricId(MapUnit::MapPoint));
            return true;
        default: ; //prevent warning
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
