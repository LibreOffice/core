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
#include <android/compatibility.hxx>

#include <editeng/eeitem.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>

#include <libxml/xmlwriter.h>

#include <attrib.hxx>
#include <global.hxx>
#include <editutil.hxx>
#include <sc.hrc>
#include <mid.h>
#include <globstr.hrc>
#include <textuno.hxx>

using namespace com::sun::star;


SfxPoolItem* ScProtectionAttr::CreateDefault() { return new ScProtectionAttr; }
SfxPoolItem* ScDoubleItem::CreateDefault() { SAL_WARN( "sc", "No ScDoubleItem factory available"); return nullptr; }

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
    OSL_ENSURE( Which() != rItem.Which() || typeid(*this) == typeid(rItem), "which ==, type !=" );
    return (Which() == rItem.Which())
             && (nColMerge == static_cast<const ScMergeAttr&>(rItem).nColMerge)
             && (nRowMerge == static_cast<const ScMergeAttr&>(rItem).nRowMerge);
}

SfxPoolItem* ScMergeAttr::Clone( SfxItemPool * ) const
{
    return new ScMergeAttr(*this);
}

void ScMergeAttr::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("ScMergeAttr"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("col-merge"), BAD_CAST(OString::number(GetColMerge()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("row-merge"), BAD_CAST(OString::number(GetRowMerge()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("merged"), BAD_CAST(OString::boolean(IsMerged()).getStr()));
    xmlTextWriterEndElement(pWriter);
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

SfxPoolItem * ScMergeFlagAttr::Clone(SfxItemPool *) const
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

void ScMergeFlagAttr::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("ScMergeFlagAttr"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("overlapped"), BAD_CAST(OString::boolean(IsOverlapped()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("hor_overlapped"), BAD_CAST(OString::boolean(IsHorOverlapped()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("ver_overlapped"), BAD_CAST(OString::boolean(IsVerOverlapped()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("autofilter"), BAD_CAST(OString::boolean(HasAutoFilter()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("scenario"), BAD_CAST(OString::boolean(IsScenario()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pivot-button"), BAD_CAST(OString::boolean(HasPivotButton()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("pivot-popup-button"), BAD_CAST(OString::boolean(HasPivotPopupButton()).getStr()));
    xmlTextWriterEndElement(pWriter);
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
    const OUString aStrYes ( ScGlobal::GetRscString(STR_YES) );
    const OUString aStrNo  ( ScGlobal::GetRscString(STR_NO) );

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
    const OUString aStrYes ( ScGlobal::GetRscString(STR_YES) );
    const OUString aStrNo  ( ScGlobal::GetRscString(STR_NO) );

    switch ( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = GetValueText();
            break;

        case SfxItemPresentation::Complete:
            rText  = ScGlobal::GetRscString(STR_PROTECTION)
                + ": "
                + (bProtection ? aStrYes : aStrNo)
                + ", "
                + ScGlobal::GetRscString(STR_FORMULAS)
                + ": "
                + (!bHideFormula ? aStrYes : aStrNo)
                + ", "
                + ScGlobal::GetRscString(STR_HIDE)
                + ": "
                + (bHideCell ? aStrYes : aStrNo)
                + ", "
                + ScGlobal::GetRscString(STR_PRINT)
                + ": "
                + (!bHidePrint ? aStrYes : aStrNo);
            break;

        default: break;
    }

    return true;
}

bool ScProtectionAttr::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( Which() != rItem.Which() || typeid(*this) == typeid(rItem), "which ==, type !=" );
    return (Which() == rItem.Which())
             && (bProtection == static_cast<const ScProtectionAttr&>(rItem).bProtection)
             && (bHideFormula == static_cast<const ScProtectionAttr&>(rItem).bHideFormula)
             && (bHideCell == static_cast<const ScProtectionAttr&>(rItem).bHideCell)
             && (bHidePrint == static_cast<const ScProtectionAttr&>(rItem).bHidePrint);
}

SfxPoolItem* ScProtectionAttr::Clone( SfxItemPool * ) const
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

/**
 * ScRangeItem - Table range
 */
bool ScRangeItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return aRange == static_cast<const ScRangeItem&>(rAttr).aRange;
}

SfxPoolItem* ScRangeItem::Clone( SfxItemPool* ) const
{
    return new ScRangeItem( *this );
}

bool ScRangeItem::GetPresentation
    (
        SfxItemPresentation ePres,
        MapUnit             /* eCoreUnit */,
        MapUnit             /* ePresUnit */,
        OUString&           rText,
        const IntlWrapper&  /* rIntl */
    ) const
{
    rText.clear();

    switch ( ePres )
    {
        case SfxItemPresentation::Complete:
        rText = ScGlobal::GetRscString(STR_AREA) + ": ";
        SAL_FALLTHROUGH;

        case SfxItemPresentation::Nameless:
        {
            /* Always use OOo:A1 format */
            rText += aRange.Format();
        }
        break;

        default:
        {
            // added to avoid warnings
        }
    }

    return true;
}

/**
 * ScPageHFItem - Dates from the Head and Foot lines
 */
ScPageHFItem::ScPageHFItem( sal_uInt16 nWhichP )
    :   SfxPoolItem ( nWhichP ),
        pLeftArea   ( nullptr ),
        pCenterArea ( nullptr ),
        pRightArea  ( nullptr )
{
}

ScPageHFItem::ScPageHFItem( const ScPageHFItem& rItem )
    :   SfxPoolItem ( rItem ),
        pLeftArea   ( nullptr ),
        pCenterArea ( nullptr ),
        pRightArea  ( nullptr )
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
    delete pLeftArea;
    delete pCenterArea;
    delete pRightArea;
}

bool ScPageHFItem::QueryValue( uno::Any& rVal, sal_uInt8 /* nMemberId */ ) const
{
    rtl::Reference<ScHeaderFooterContentObj> xContent =
        new ScHeaderFooterContentObj();
    xContent->Init(pLeftArea, pCenterArea, pRightArea);

    uno::Reference<sheet::XHeaderFooterContent> xCont(xContent.get());

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
                delete pLeftArea;
                pLeftArea = pImpLeft ? pImpLeft->Clone() : nullptr;

                const EditTextObject* pImpCenter = pImp->GetCenterEditObject();
                delete pCenterArea;
                pCenterArea = pImpCenter ? pImpCenter->Clone() : nullptr;

                const EditTextObject* pImpRight = pImp->GetRightEditObject();
                delete pRightArea;
                pRightArea = pImpRight ? pImpRight->Clone() : nullptr;

                if ( !pLeftArea || !pCenterArea || !pRightArea )
                {
                    // no Text with Null are left
                    ScEditEngineDefaulter aEngine( EditEngine::CreatePool(), true );
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

    return    ScGlobal::EETextObjEqual(pLeftArea,   r.pLeftArea)
           && ScGlobal::EETextObjEqual(pCenterArea, r.pCenterArea)
           && ScGlobal::EETextObjEqual(pRightArea,  r.pRightArea);
}

SfxPoolItem* ScPageHFItem::Clone( SfxItemPool* ) const
{
    return new ScPageHFItem( *this );
}

void ScPageHFItem::SetLeftArea( const EditTextObject& rNew )
{
    delete pLeftArea;
    pLeftArea = rNew.Clone();
}

void ScPageHFItem::SetCenterArea( const EditTextObject& rNew )
{
    delete pCenterArea;
    pCenterArea = rNew.Clone();
}

void ScPageHFItem::SetRightArea( const EditTextObject& rNew )
{
    delete pRightArea;
    pRightArea = rNew.Clone();
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
    OUString aDel(": ");
    rText.clear();

    switch ( ePres )
    {
        case SfxItemPresentation::Complete:
            switch( Which() )
            {
                case SID_SCATTR_PAGE_CHARTS:
                rText = ScGlobal::GetRscString(STR_VOBJ_CHART) + aDel;
                break;

                case SID_SCATTR_PAGE_OBJECTS:
                rText = ScGlobal::GetRscString(STR_VOBJ_OBJECT) + aDel;
                break;

                case SID_SCATTR_PAGE_DRAWINGS:
                rText = ScGlobal::GetRscString(STR_VOBJ_DRAWINGS) + aDel;
                break;

                default: break;
            }
            SAL_FALLTHROUGH;
        case SfxItemPresentation::Nameless:
            if (GetValue() == VOBJ_MODE_SHOW)
                rText += ScGlobal::GetRscString(STR_VOBJ_MODE_SHOW);
            else
                rText += ScGlobal::GetRscString(STR_VOBJ_MODE_HIDE);
            return true;
            break;

        default: break;
            // added to avoid warnings
    }

    return false;
}

sal_uInt16 ScViewObjectModeItem::GetValueCount() const
{
    return 2;
}

SfxPoolItem* ScViewObjectModeItem::Clone( SfxItemPool* ) const
{
    return new ScViewObjectModeItem( *this );
}

sal_uInt16 ScViewObjectModeItem::GetVersion( sal_uInt16 /* nFileVersion */ ) const
{
    return 1;
}

/**
 * Double
 */
ScDoubleItem::ScDoubleItem( sal_uInt16 nWhichP, double nVal )
    :   SfxPoolItem ( nWhichP ),
        nValue  ( nVal )
{
}

ScDoubleItem::ScDoubleItem( const ScDoubleItem& rItem )
    :   SfxPoolItem ( rItem )
{
        nValue = rItem.nValue;
}

bool ScDoubleItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    const ScDoubleItem& _rItem = static_cast<const ScDoubleItem&>(rItem);
    return nValue == _rItem.nValue;
}

SfxPoolItem* ScDoubleItem::Clone( SfxItemPool* ) const
{
    return new ScDoubleItem( *this );
}

ScDoubleItem::~ScDoubleItem()
{
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
        OUString aPages( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_PAGES ) );
        rText += aPages.replaceFirst( "%1", OUString::number( nPages ) );
    }
    else
        rText += ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_AUTO );
}
} // namespace

bool ScPageScaleToItem::GetPresentation(
        SfxItemPresentation ePres, MapUnit, MapUnit, OUString& rText, const IntlWrapper& ) const
{
    rText.clear();
    if( !IsValid())
        return false;

    OUString aName( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALETO ) );
    OUString aValue( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_WIDTH ) );
    lclAppendScalePageCount( aValue, mnWidth );
    aValue = aValue + ", " + ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_HEIGHT );
    lclAppendScalePageCount( aValue, mnHeight );

    switch( ePres )
    {
        case SfxItemPresentation::Nameless:
            rText = aValue;
            return true;
        break;

        case SfxItemPresentation::Complete:
            rText = aName + " (" + aValue + ")";
            return true;
        break;

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

ScCondFormatItem::ScCondFormatItem():
    SfxPoolItem( ATTR_CONDITIONAL )
{
}

ScCondFormatItem::ScCondFormatItem( const std::vector<sal_uInt32>& rIndex ):
    SfxPoolItem( ATTR_CONDITIONAL ),
    maIndex( rIndex )
{
}

ScCondFormatItem::~ScCondFormatItem()
{
}

bool ScCondFormatItem::operator==( const SfxPoolItem& rCmp ) const
{
    return maIndex == static_cast<const ScCondFormatItem&>(rCmp).maIndex;
}

ScCondFormatItem* ScCondFormatItem::Clone(SfxItemPool*) const
{
    return new ScCondFormatItem(maIndex);
}

void ScCondFormatItem::AddCondFormatData( sal_uInt32 nIndex )
{
    maIndex.push_back(nIndex);
}

void ScCondFormatItem::SetCondFormatData( const std::vector<sal_uInt32>& rIndex )
{
    maIndex = rIndex;
}

void ScCondFormatItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("ScCondFormatItem"));
    for (const auto& nItem : maIndex)
    {
        std::string aStrVal = std::to_string(nItem);
        xmlTextWriterStartElement(pWriter, BAD_CAST(aStrVal.c_str()));
        xmlTextWriterEndElement(pWriter);
    }
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
