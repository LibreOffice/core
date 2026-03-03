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

#include <document.hxx>
#include <stlsheet.hxx>
#include <stlpool.hxx>

#include <scitems.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/svddef.hxx>
#include <svx/svdpool.hxx>
#include <svx/xdef.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/numformat.hxx>
#include <svl/hint.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <svl/zformat.hxx>
#include <o3tl/unit_conversion.hxx>
#include <attrib.hxx>

#include <globstr.hrc>
#include <strings.hrc>
#include <scresid.hxx>
#include <sc.hrc>

#include <svx/strarray.hxx>
#include <svx/svxids.hrc>
#include <svl/zforlist.hxx>
#include <svl/intitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/itemiter.hxx>
#include <svl/eitem.hxx>
#include <docsh.hxx>
#include <sfx2/objsh.hxx>

constexpr auto TWO_CM = o3tl::convert(2, o3tl::Length::cm, o3tl::Length::twip); // 1134
constexpr auto HFDIST_CM = o3tl::convert(250, o3tl::Length::mm100, o3tl::Length::twip); // 142

ScStyleSheet::ScStyleSheet( const OUString&     rName,
                            const ScStyleSheetPool& rPoolP,
                            SfxStyleFamily      eFamily,
                            SfxStyleSearchBits  nMaskP,
                            const OUString& rParentStyleSheetName)

    : SfxStyleSheet   ( rName, rPoolP, eFamily, nMaskP, rParentStyleSheetName )
    , eUsage( Usage::UNKNOWN )
{
}

ScStyleSheet::ScStyleSheet( const ScStyleSheet& rStyle )
    : SfxStyleSheet ( rStyle )
    , eUsage( Usage::UNKNOWN )
{
}

ScStyleSheet::~ScStyleSheet()
{
}

bool ScStyleSheet::HasFollowSupport() const
{
    return false;
}

bool ScStyleSheet::HasParentSupport () const
{
    bool bHasParentSupport = false;

    switch ( GetFamily() )
    {
    case SfxStyleFamily::Para: bHasParentSupport = true;   break;
    case SfxStyleFamily::Frame: bHasParentSupport = true;   break;
    case SfxStyleFamily::Page: bHasParentSupport = false;  break;
    default:
        {
            // added to avoid warnings
        }
    }

    return bHasParentSupport;
}

bool ScStyleSheet::SetParent( const OUString& rParentName )
{
    bool bResult = false;
    OUString aEffName = rParentName;
    SfxStyleSheetBase* pStyle = m_pPool->Find( aEffName, nFamily );
    if (!pStyle)
    {
        std::unique_ptr<SfxStyleSheetIterator> pIter = m_pPool->CreateIterator(nFamily);
        pStyle = pIter->First();
        if (pStyle)
            aEffName = pStyle->GetName();
    }

    if ( pStyle && aEffName != GetName() )
    {
        bResult = SfxStyleSheet::SetParent( aEffName );
        if (bResult)
        {
            SfxItemSet& rParentSet = pStyle->GetItemSet();
            GetItemSet().SetParent( &rParentSet );

            // #i113491# Drag&Drop in the stylist's hierarchical view doesn't execute a slot,
            // so the repaint has to come from here (after modifying the ItemSet).
            // RepaintRange checks the document's IsVisible flag and locked repaints.
            ScDocument* pDoc = static_cast<ScStyleSheetPool*>(GetPool())->GetDocument();
            if (pDoc)
                pDoc->RepaintRange( ScRange( 0,0,0, pDoc->MaxCol(),pDoc->MaxRow(),MAXTAB ) );
        }
    }

    return bResult;
}

void ScStyleSheet::ResetParent()
{
    GetItemSet().SetParent(nullptr);
}

SfxItemSet& ScStyleSheet::GetItemSet()
{
    if ( !pSet )
    {
        switch ( GetFamily() )
        {
            case SfxStyleFamily::Page:
                {
                    // Page templates should not be derivable,
                    // therefore suitable values are set at this point.
                    // (== Standard page template)

                    SfxItemPool& rItemPool = GetPool()->GetPool();
                    pSet = new SfxItemSet(rItemPool, WhichRangesContainer(svl::Items<
                        ATTR_USERDEF, ATTR_USERDEF,
                        ATTR_WRITINGDIR, ATTR_WRITINGDIR,
                        ATTR_BACKGROUND, ATTR_BACKGROUND,
                        ATTR_BORDER, ATTR_SHADOW,
                        ATTR_LRSPACE, ATTR_PAGE_SCALETO>));
                    //  If being loaded also the set is then filled in from the file,
                    //  so the defaults do not need to be set.
                    //  GetPrinter would then also create a new printer,
                    //  because the stored Printer is not loaded yet!

                    ScDocument* pDoc = static_cast<ScStyleSheetPool*>(GetPool())->GetDocument();
                    if ( pDoc )
                    {
                        // Setting reasonable default values:
                        SvxPageItem     aPageItem( ATTR_PAGE );
                        SvxSizeItem     aPaperSizeItem( ATTR_PAGE_SIZE, SvxPaperInfo::GetDefaultPaperSize() );

                        SvxSetItem      aHFSetItem(
                                            rItemPool.GetUserOrPoolDefaultItem(ATTR_PAGE_HEADERSET) );

                        SfxItemSet&     rHFSet = aHFSetItem.GetItemSet();
                        SvxSizeItem     aHFSizeItem( // 0,5 cm + distance
                                            ATTR_PAGE_SIZE,
                                            Size( 0, o3tl::convert(500, o3tl::Length::mm100, o3tl::Length::twip) + HFDIST_CM ) );

                        SvxULSpaceItem  aHFDistItem ( HFDIST_CM,// nUp
                                                      HFDIST_CM,// nLow
                                                      ATTR_ULSPACE );

                        SvxLRSpaceItem aLRSpaceItem(SvxIndentValue::twips(TWO_CM), // nLeft
                                                    SvxIndentValue::twips(TWO_CM), // nRight
                                                    SvxIndentValue::zero(), // nFirstLineOffset
                                                    ATTR_LRSPACE);
                        SvxULSpaceItem  aULSpaceItem( TWO_CM,   // nUp
                                                      TWO_CM,   // nLow
                                                      ATTR_ULSPACE );
                        SvxBoxInfoItem  aBoxInfoItem( ATTR_BORDER_INNER );

                        aBoxInfoItem.SetTable( false );
                        aBoxInfoItem.SetDist( true );
                        aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );

                        aPageItem.SetLandscape( false );

                        rHFSet.Put( aBoxInfoItem );
                        rHFSet.Put( aHFSizeItem );
                        rHFSet.Put( aHFDistItem );
                        rHFSet.Put(SvxLRSpaceItem(SvxIndentValue::zero(), SvxIndentValue::zero(),
                                                  SvxIndentValue::zero(),
                                                  ATTR_LRSPACE)); // Set border to Null

                        aHFSetItem.SetWhich(ATTR_PAGE_HEADERSET);
                        pSet->Put( aHFSetItem );
                        aHFSetItem.SetWhich(ATTR_PAGE_FOOTERSET);
                        pSet->Put( aHFSetItem );
                        pSet->Put( aBoxInfoItem ); // Do not overwrite PoolDefault
                                                   // due to format templates


                        //  Writing direction: not as pool default because the default for cells
                        //  must remain SvxFrameDirection::Environment, and each page style's setting is
                        //  supposed to be saved in the file format.
                        //  The page default depends on the system language.
                        SvxFrameDirection eDirection = ScGlobal::IsSystemRTL() ?
                                        SvxFrameDirection::Horizontal_RL_TB : SvxFrameDirection::Horizontal_LR_TB;
                        pSet->Put( SvxFrameDirectionItem( eDirection, ATTR_WRITINGDIR ) );

                        rItemPool.SetUserDefaultItem( aPageItem );
                        rItemPool.SetUserDefaultItem( aPaperSizeItem );
                        rItemPool.SetUserDefaultItem( aLRSpaceItem );
                        rItemPool.SetUserDefaultItem( aULSpaceItem );
                        rItemPool.SetUserDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALE, 100 ) );
                        ScPageScaleToItem aScaleToItem;
                        rItemPool.SetUserDefaultItem( aScaleToItem );
                        rItemPool.SetUserDefaultItem( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, 0 ) );
                    }
                }
                break;

            case SfxStyleFamily::Frame:
            {
                SfxItemPool* pItemPool = &GetPool()->GetPool();
                assert(pItemPool);
                if (dynamic_cast<SdrItemPool*>(pItemPool) == nullptr)
                    pItemPool = pItemPool->GetSecondaryPool();
                assert(pItemPool);

                pSet = new SfxItemSet(*pItemPool, WhichRangesContainer(svl::Items<
                            XATTR_LINE_FIRST, XATTR_LINE_LAST,
                            XATTR_FILL_FIRST, XATTR_FILL_LAST,
                            SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,
                            SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_WORDWRAP,
                            SDRATTR_EDGE_FIRST, SDRATTR_MEASURE_LAST,
                            SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
                            EE_PARA_START, EE_CHAR_END>));
            }
            break;

            case SfxStyleFamily::Para:
            default:
                pSet = new SfxItemSet(GetPool()->GetPool(), WhichRangesContainer(svl::Items<
                            ATTR_PATTERN_START, ATTR_PATTERN_END>));
                break;
        }
        bMySet = true;
    }
    if ( nHelpId == HID_SC_SHEET_CELL_ERG1 )
    {
        if ( !pSet->Count() )
        {
            // Hack to work around that when this code is called from
            // ~ScStyleSheetPool -> ~SfxStyleSheetPool, GetPool() is no longer
            // an ScStyleSheetPool:
            ScStyleSheetPool * pool = dynamic_cast<ScStyleSheetPool *>(
                GetPool());
            if (pool != nullptr) {
                ScDocument* pDoc = pool->GetDocument();
                if ( pDoc )
                {
                    sal_uInt32 nNumFmt = pDoc->GetFormatTable()->GetStandardFormat( SvNumFormatType::CURRENCY,ScGlobal::eLnge );
                    pSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNumFmt ) );
                }
            }
        }
    }

    return *pSet;
}

std::vector<std::pair<sal_uInt16, OUString>> ScStyleSheet::GetItemPresentation(
    MapUnit eMetric, const SfxItemSet* pWorkingSet)
{
    std::vector<std::pair<sal_uInt16, OUString>> aResult;
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());

    const SfxItemSet& rSet = GetItemSet();
    const SfxItemSet* pCheckSet = pWorkingSet ? pWorkingSet : &rSet;

    // Get parent item set for comparison
    const SfxItemSet* pParentSet = nullptr;
    SfxStyleSheetBase* pParentStyle = nullptr;
    if (!GetParent().isEmpty())
    {
        pParentStyle = m_pPool->Find(GetParent(), GetFamily());
        if (pParentStyle)
            pParentSet = &pParentStyle->GetItemSet();
    }

    // Get the number formatter for ATTR_VALUE_FORMAT
    SvNumberFormatter* pFormatter = nullptr;
    if (ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(SfxObjectShell::Current()))
    {
        pFormatter = pDocSh->GetDocument().GetFormatTable();
    }

    SfxItemIter aIter(rSet);

    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        if (IsInvalidItem(pItem))
            continue;

        sal_uInt16 nWhich = pItem->Which();

        // Only show items explicitly SET in this style (not inherited)
        if (pCheckSet->GetItemState(nWhich, false) != SfxItemState::SET)
            continue;

        // Skip items identical to parent
        if (pParentSet)
        {
            const SfxPoolItem* pParentItem = nullptr;
            if (pParentSet->GetItemState(nWhich, true, &pParentItem) == SfxItemState::SET
                && pParentItem && *pParentItem == *pItem)
                continue;
        }

        // Skip items equal to pool default
        if (*pItem == rSet.GetPool()->GetUserOrPoolDefaultItem(nWhich))
            continue;

        // Skip internal/structural items that don't make sense as chips
        switch (nWhich)
        {
            case SID_ATTR_BORDER_INNER:
            case SID_ATTR_PARA_MODEL:
            case SID_ATTR_PAGE_SIZE:
            case SID_ATTR_PAGE_MAXSIZE:
            case SID_ATTR_PAGE_PAPERBIN:
                continue;
            default:
                break;
        }

        OUString aItemPresentation;

        // Special handling for number format: the default GetPresentation
        // just returns the raw format key (e.g. "10122")
        if (nWhich == ATTR_VALUE_FORMAT && pFormatter)
        {
            sal_uInt32 nFormat = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
            const SvNumberformat* pEntry = pFormatter->GetEntry(nFormat);
            if (pEntry)
            {
                OUString sFormatStr = pEntry->GetFormatstring();
                // Truncate very long format strings
                if (sFormatStr.getLength() > 40)
                    sFormatStr = OUString::Concat(sFormatStr.subView(0, 37)) + "...";
                aItemPresentation = SvxAttrNameTable::GetString(
                    SvxAttrNameTable::FindIndex(
                        rSet.GetPool()->GetSlotId(nWhich)));
                if (aItemPresentation.isEmpty())
                    aItemPresentation = ScResId(STR_ATTR_NUM_FORMAT);
                aItemPresentation += ": " + sFormatStr;
            }
            else
            {
                continue;  // Skip unresolvable format keys
            }
        }
        else
        {
            // Standard presentation via pool
            if (!m_pPool->GetPool().GetPresentation(
                    *pItem, eMetric, aItemPresentation, aIntlWrapper))
                continue;

            if (aItemPresentation.isEmpty())
                continue;

            // Add attribute name prefix if not already present
            if (aItemPresentation.indexOf(": ") == -1)
            {
                sal_uInt16 nSlotId = rSet.GetPool()->GetSlotId(nWhich);
                sal_uInt32 nIdx = SvxAttrNameTable::FindIndex(nSlotId);
                OUString aAttrName = SvxAttrNameTable::GetString(nIdx);

                // Fallback for Calc-specific WhichIds not in SvxAttrNameTable
                if (aAttrName.isEmpty())
                {
                    // You may want to add string resources in sc/inc/strings.hrc
                    // for now using English fallbacks that you can later replace
                    // with proper NC_ macros
                    switch (nWhich)
                    {
                        case ATTR_PROTECTION:
                            aAttrName = ScResId(STR_ATTR_PROTECTION);
                            break;
                        case ATTR_ROTATE_VALUE:
                            aAttrName = ScResId(STR_ATTR_ROTATION);
                            break;
                        case ATTR_ROTATE_MODE:
                            aAttrName = ScResId(STR_ATTR_ROTATION_MODE);
                            break;
                        case ATTR_WRITINGDIR:
                            aAttrName = ScResId(STR_ATTR_TEXT_DIR);
                            break;
                        case ATTR_LINEBREAK:
                            aAttrName = ScResId(STR_ATTR_WRAP_TEXT);
                            break;
                        case ATTR_SHRINKTOFIT:
                            aAttrName = ScResId(STR_ATTR_SHRINK_FIT);
                            break;
                        case ATTR_STACKED:
                            aAttrName = ScResId(STR_ATTR_STACKED);
                            break;
                        case ATTR_HOR_JUSTIFY:
                            aAttrName = ScResId(STR_ATTR_H_ALIGN);
                            break;
                        case ATTR_VER_JUSTIFY:
                            aAttrName = ScResId(STR_ATTR_V_ALIGN);
                            break;
                        case ATTR_INDENT:
                            aAttrName = ScResId(STR_ATTR_INDENT);
                            break;
                        case ATTR_MARGIN:
                            aAttrName = ScResId(STR_ATTR_SPACING);
                            break;
                        case ATTR_VERTICAL_ASIAN:
                            aAttrName = ScResId(STR_ATTR_ASIAN_LAYOUT);
                            break;
                        default:
                            break;
                    }
                }

                if (!aAttrName.isEmpty())
                    aItemPresentation = aAttrName + ": " + aItemPresentation;
            }
        }

        if (!aItemPresentation.isEmpty())
            aResult.emplace_back(nWhich, aItemPresentation);
    }

    return aResult;
}

bool ScStyleSheet::IsUsed() const
{
    switch (GetFamily())
    {
        case SfxStyleFamily::Para:
        {
            // Always query the document to let it decide if a rescan is necessary,
            // and store the state.
            ScDocument* pDoc = static_cast<ScStyleSheetPool*>(m_pPool)->GetDocument();
            if ( pDoc && pDoc->IsStyleSheetUsed( *this ) )
                eUsage = Usage::USED;
            else
                eUsage = Usage::NOTUSED;
            return eUsage == Usage::USED;
        }
        case SfxStyleFamily::Page:
        {
            // tdf#108188 - verify that the page style is actually used
            ScDocument* pDoc = static_cast<ScStyleSheetPool*>(m_pPool)->GetDocument();
            if (pDoc && pDoc->IsPageStyleInUse(GetName(), nullptr))
                eUsage = Usage::USED;
            else
                eUsage = Usage::NOTUSED;
            return eUsage == Usage::USED;
        }
        case SfxStyleFamily::Frame:
        {
            ForAllListeners([this] (SfxListener* pListener)
                {
                    auto pUser(dynamic_cast<svl::StyleSheetUser*>(pListener));
                    if (pUser && pUser->isUsedByModel())
                    {
                        eUsage = Usage::USED;
                        return true; // break loop
                    }
                    else
                        eUsage = Usage::NOTUSED;
                    return false;
                });
            return eUsage == Usage::USED;
        }
        default:
            return true;
    }
}

void ScStyleSheet::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
        GetItemSet().SetParent( nullptr );
    if (GetFamily() == SfxStyleFamily::Frame)
        SfxStyleSheet::Notify(rBC, rHint);
}

// Avoid creating a Style "Standard" if this is not the Standard-Name;
// otherwise two styles would have the same name when storing.
// (on loading the style is created directly per Make with the name; making this query
//  not applicable)
//TODO: If at any time during loading SetName is called, a flag has to be set/checked for loading
//TODO: The whole check has to be removed if for a new file version the name transformation is dropped.

bool ScStyleSheet::SetName(const OUString& rNew, bool bReindexNow)
{
    OUString aFileStdName = STRING_STANDARD;
    if ( rNew == aFileStdName && aFileStdName != ScResId(STR_STYLENAME_STANDARD) )
        return false;
    else
        return SfxStyleSheet::SetName(rNew, bReindexNow);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
