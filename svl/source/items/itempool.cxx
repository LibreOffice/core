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

#include <svl/itempool.hxx>
#include <svl/setitem.hxx>

#include <string.h>
#include <libxml/xmlwriter.h>

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/hint.hxx>
#include <svl/itemset.hxx>
#include <tools/debug.hxx>

#include <cassert>
#include <vector>

#ifdef DBG_UTIL
static size_t nAllDirectlyPooledSfxPoolItemCount(0);
static size_t nRemainingDirectlyPooledSfxPoolItemCount(0);
size_t getAllDirectlyPooledSfxPoolItemCount() { return nAllDirectlyPooledSfxPoolItemCount; }
size_t getRemainingDirectlyPooledSfxPoolItemCount() { return nRemainingDirectlyPooledSfxPoolItemCount; }
#endif

// WhichIDs that need to set SFX_ITEMINFOFLAG_SUPPORT_SURROGATE in SfxItemInfo
// to true to allow a register of all items of that type/with that WhichID
// to be accessible using SfxItemPool::GetItemSurrogates. Created by
// grepping for 'GetItemSurrogates' usages & interpreting. Some
// are double, more may be necessary. There is a SAL_INFO("svl.items", ...)
// in SfxItemPool::GetItemSurrogates that will give hints on missing flags.
//
// due to SwTable::UpdateFields
// due to SwCursorShell::GotoNxtPrvTableFormula
// due to DocumentFieldsManager::UpdateTableFields
// due to SwTable::GatherFormulas
//  RES_BOXATR_FORMULA ok
// due to SwContentTree::EditEntry
// due to SwDoc::FindINetAttr
// due to SwUndoResetAttr::RedoImpl
// due to SwContentTree::EditEntry
// due to SwContentTree::BringEntryToAttention
//  RES_TXTATR_REFMARK ok
// due to ImpEditEngine::WriteRTF
// due to ScDocument::UpdateFontCharSet()
// due to ScXMLFontAutoStylePool_Impl
// due to SdXImpressDocument::getPropertyValue
// due to Writer::AddFontItems_
//  EE_CHAR_FONTINFO ok
// due to ImpEditEngine::WriteRTF
// due to ScXMLFontAutoStylePool_Impl
// due to SdXImpressDocument::getPropertyValue
// due to Writer::AddFontItems_
//  EE_CHAR_FONTINFO_CJK ok
// due to ImpEditEngine::WriteRTF
// due to ScXMLFontAutoStylePool_Impl
// due to SdXImpressDocument::getPropertyValue
// due to Writer::AddFontItems_
//  EE_CHAR_FONTINFO_CTL ok
// due to ImpEditEngine::WriteRTF
//  EE_CHAR_COLOR ok
// due to ScDocumentPool::StyleDeleted
// due to ScDocument::UpdateFontCharSet()
// due to ScXMLFontAutoStylePool_Impl
//  ATTR_FONT ok
// due to OptimizeHasAttrib
//  ATTR_ROTATE_VALUE ok
// due to ScDocument::GetDocColors()
//  ATTR_BACKGROUND ok
//  ATTR_FONT_COLOR ok
// due to ScXMLExport::CollectUserDefinedNamespaces
//  ATTR_USERDEF ok
// due to ScXMLExport::CollectUserDefinedNamespaces
// due to SwXMLExport::exportDoc
//  EE_PARA_XMLATTRIBS ok
// due to ScXMLExport::CollectUserDefinedNamespaces
// due to SwXMLExport::exportDoc
//  EE_CHAR_XMLATTRIBS ok
// due to ScXMLExport::CollectUserDefinedNamespaces
// due to SwXMLExport::exportDoc
//  SDRATTR_XMLATTRIBUTES ok
// due to ScXMLFontAutoStylePool_Impl
//  ATTR_CJK_FONT ok
//  ATTR_CTL_FONT ok
//  ATTR_PAGE_HEADERLEFT ok
//  ATTR_PAGE_FOOTERLEFT ok
//  ATTR_PAGE_HEADERRIGHT ok
//  ATTR_PAGE_FOOTERRIGHT ok
//  ATTR_PAGE_HEADERFIRST ok
//  ATTR_PAGE_FOOTERFIRST ok
// due to ScCellShell::ExecuteEdit
// due to ScTabViewShell::CreateRefDialogController
//  SCITEM_CONDFORMATDLGDATA ok
// due to SdDrawDocument::UpdatePageRelativeURLs
//  EE_FEATURE_FIELD ok
// due to SvxUnoMarkerTable::replaceByName
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
// due to XLineStartItem::checkForUniqueItem
//  XATTR_LINESTART ok
// due to SvxUnoMarkerTable::replaceByName
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
// due to XLineStartItem::checkForUniqueItem
//  XATTR_LINEEND ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
// due to NameOrIndex::CheckNamedItem all derived from NameOrIndex
//  XATTR_FILLBITMAP ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
//  XATTR_LINEDASH ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
// due to NameOrIndex::CheckNamedItem all derived from NameOrIndex
//  XATTR_FILLGRADIENT ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
//  XATTR_FILLHATCH ok
// due to SvxUnoNameItemTable
// due to SvxShape::setPropertyValueImpl/SvxShape::SetFillAttribute
//  XATTR_FILLFLOATTRANSPARENCE ok
// due to NamespaceIteratorImpl
//      -> needs to be evaluated
// due to SwCursorShell::GotoNxtPrvTOXMark
// due to SwDoc::GetTOIKeys
//  RES_TXTATR_TOXMARK ok
// due to SwDoc::GetRefMark
// due to SwDoc::CallEvent
// due to SwURLStateChanged::Notify
// due to SwHTMLWriter::CollectLinkTargets
// due to MSWordExportBase::CollectOutlineBookmarks
//  RES_TXTATR_INETFMT ok
// due to SwDoc::GetAllUsedDB
// due to lcl_FindInputField
// due to SwViewShell::IsAnyFieldInDoc
//  RES_TXTATR_FIELD ok
// due to SwDoc::GetAllUsedDB
// due to lcl_FindInputField
// due to SwViewShell::IsAnyFieldInDoc
//  RES_TXTATR_INPUTFIELD ok
// due to SwDoc::SetDefault
//  RES_PARATR_TABSTOP ok
// due to SwDoc::GetDocColors()
// due to RtfExport::OutColorTable
//  RES_CHRATR_COLOR ok
// due to SwDoc::GetDocColors()
//  RES_CHRATR_HIGHLIGHT ok
// due to SwDoc::GetDocColors()
//  RES_BACKGROUND ok
// due to SwNode::FindPageDesc
// due to SwPageNumberFieldType::ChangeExpansion
// due to SwFrame::GetVirtPageNum
//  RES_PAGEDESC ok
// due to SwAutoStylesEnumImpl::
//  RES_TXTATR_CJK_RUBY ok
// due to SwHTMLWriter::CollectLinkTargets
// due to MSWordExportBase::CollectOutlineBookmarks
//  RES_URL
// due to RtfExport::OutColorTable
//  RES_CHRATR_UNDERLINE ok
//  RES_CHRATR_OVERLINE ok
//  RES_CHRATR_BACKGROUND ok
//  RES_SHADOW ok
//  RES_BOX ok
//  RES_CHRATR_BOX ok
//  XATTR_FILLCOLOR ok
// due to wwFontHelper::InitFontTable
// due to SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl
//  RES_CHRATR_FONT ok
// due to wwFontHelper::InitFontTable
// due to SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl
//  RES_CHRATR_CJK_FONT ok
// due to wwFontHelper::InitFontTable
// due to SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl
//  RES_CHRATR_CTL_FONT
// due to SwXMLExport::exportDoc
//  RES_UNKNOWNATR_CONTAINER ok
//  RES_TXTATR_UNKNOWN_CONTAINER ok



#if OSL_DEBUG_LEVEL > 0
#include <map>

static void
lcl_CheckSlots2(std::map<sal_uInt16, sal_uInt16> & rSlotMap,
        SfxItemPool const& rPool, SfxItemInfo const* pInfo)
{
    if (!pInfo)
        return; // may not be initialized yet
    if (rPool.GetName() == "EditEngineItemPool")
        return; // HACK: this one has loads of duplicates already, ignore it :(
    sal_uInt16 const nFirst(rPool.GetFirstWhich());
    sal_uInt16 const nCount(rPool.GetLastWhich() - rPool.GetFirstWhich() + 1);
    for (sal_uInt16 n = 0; n < nCount; ++n)
    {
        sal_uInt16 const nSlotId(pInfo[n]._nItemInfoSlotID);
        if (nSlotId != 0
            && nSlotId != 10883  // preexisting duplicate SID_ATTR_GRAF_CROP
            && nSlotId != 10023  // preexisting duplicate SID_ATTR_BORDER_INNER
            && nSlotId != 10024  // preexisting duplicate SID_ATTR_BORDER_OUTER
            && nSlotId != 11013  // preexisting duplicate SID_ATTR_BORDER_DIAG_TLBR
            && nSlotId != 11014) // preexisting duplicate SID_ATTR_BORDER_DIAG_BLTR
        {   // check for duplicate slot-id mapping
            std::map<sal_uInt16, sal_uInt16>::const_iterator const iter(
                rSlotMap.find(nSlotId));
            sal_uInt16 const nWhich(nFirst + n);
            if (iter != rSlotMap.end())
            {
                SAL_WARN("svl", "SfxItemPool: duplicate SlotId " << nSlotId
                        << " mapped to " << iter->second << " and " << nWhich);
                assert(false);
            }
            rSlotMap.insert(std::make_pair(nSlotId, nWhich));
        }
    }
}

#define CHECK_SLOTS() \
do { \
    std::map<sal_uInt16, sal_uInt16> slotmap; \
    for (SfxItemPool * p = mpMaster; p; p = p->mpSecondary.get()) \
    { \
        lcl_CheckSlots2(slotmap, *p, p->pItemInfos); \
    } \
} while (false)

#else
#define CHECK_SLOTS() do {} while (false)
#endif

void SfxItemPool::registerItemSet(SfxItemSet& rSet)
{
    registeredSfxItemSets& rTarget(GetMasterPool()->maRegisteredSfxItemSets);
#ifdef DBG_UTIL
    const size_t nBefore(rTarget.size());
#endif
    rTarget.insert(&rSet);
#ifdef DBG_UTIL
    const size_t nAfter(rTarget.size());
    if (nBefore + 1 != nAfter)
    {
        SAL_WARN("svl.items", "SfxItemPool::registerItemSet: ItemSet was already registered (!)");
    }
#endif
}

void SfxItemPool::unregisterItemSet(SfxItemSet& rSet)
{
    registeredSfxItemSets& rTarget(GetMasterPool()->maRegisteredSfxItemSets);
#ifdef DBG_UTIL
    const size_t nBefore(rTarget.size());
#endif
    rTarget.erase(&rSet);
#ifdef DBG_UTIL
    const size_t nAfter(rTarget.size());
    if (nBefore != nAfter + 1)
    {
        SAL_WARN("svl.items", "SfxItemPool::unregisterItemSet: ItemSet was not registered (!)");
    }
#endif
}

void SfxItemPool::registerPoolItemHolder(SfxPoolItemHolder& rHolder)
{
    registeredSfxPoolItemHolders& rTarget(GetMasterPool()->maRegisteredSfxPoolItemHolders);
#ifdef DBG_UTIL
    const size_t nBefore(rTarget.size());
#endif
    rTarget.insert(&rHolder);
#ifdef DBG_UTIL
    const size_t nAfter(rTarget.size());
    if (nBefore + 1 != nAfter)
    {
        SAL_WARN("svl.items", "SfxItemPool::registerPoolItemHolder: SfxPoolItemHolder was already registered (!)");
    }
#endif
}

void SfxItemPool::unregisterPoolItemHolder(SfxPoolItemHolder& rHolder)
{
    registeredSfxPoolItemHolders& rTarget(GetMasterPool()->maRegisteredSfxPoolItemHolders);
#ifdef DBG_UTIL
    const size_t nBefore(rTarget.size());
#endif
    rTarget.erase(&rHolder);
#ifdef DBG_UTIL
    const size_t nAfter(rTarget.size());
    if (nBefore != nAfter + 1)
    {
        SAL_WARN("svl.items", "SfxItemPool::unregisterPoolItemHolder: SfxPoolItemHolder was not registered (!)");
    }
#endif
}

sal_uInt16 SfxItemPool::GetFirstWhich() const
{
    return mnStart;
}

sal_uInt16 SfxItemPool::GetLastWhich() const
{
    return mnEnd;
}

bool SfxItemPool::IsInRange( sal_uInt16 nWhich ) const
{
    return nWhich >= mnStart && nWhich <= mnEnd;
}

sal_uInt16 SfxItemPool::GetIndex_Impl(sal_uInt16 nWhich) const
{
    if (nWhich < mnStart || nWhich > mnEnd)
    {
        assert(false && "missing bounds check before use");
        return 0;
    }
    return nWhich - mnStart;
}

sal_uInt16 SfxItemPool::GetSize_Impl() const
{
    return mnEnd - mnStart + 1;
}

const SfxPoolItem* SfxItemPool::GetPoolDefaultItem( sal_uInt16 nWhich ) const
{
    const SfxPoolItem* pRet;
    if( IsInRange( nWhich ) )
        pRet = maPoolDefaults[GetIndex_Impl(nWhich)];
    else if( mpSecondary )
        pRet = mpSecondary->GetPoolDefaultItem( nWhich );
    else
    {
        assert(false && "unknown WhichId - cannot get pool default");
        pRet = nullptr;
    }
    return pRet;
}


bool SfxItemPool::CheckItemInfoFlag(sal_uInt16 nWhich, sal_uInt16 nMask) const
{
    if (!IsInRange(nWhich))
    {
        // get to correct pool
        if (mpSecondary)
            return mpSecondary->CheckItemInfoFlag(nWhich, nMask);
        return false;
    }

    return CheckItemInfoFlag_Impl(nWhich - mnStart, nMask);
}

SfxBroadcaster& SfxItemPool::BC()
{
    return aBC;
}


/**
 * This is the regular ctor to be used for this class.
 * An SfxItemPool instance is initialized, which can manage Items in the
 * range from 'nStartWhich' to 'nEndWhich'.
 *
 * For every one of these WhichIds a static Default must be present in the
 * 'pDefaults' array. They start with an SfxPoolItem (with the WhichId
 * 'nStartWhich'), are sorted by WhichId and consecutively stored.
 *
 * 'pItemInfos' is a USHORT array arranged in the same way, which holds
 *  SlotIds and Flags. These SlotIds can be 0, if the affected Items are
 *  exclusively used in the Core.
 *  The flags allow for e.g. enabling value sharing (poolable).
 *
 *  If the Pool is supposed to hold SfxSetItems, the ctor cannot yet contain
 *  static Defaults. This needs to be done afterwards, using
 *  @see SfxItemPool::SetDefaults(std::vector<SfxPoolItem*>*).
 *
 *  @see SfxItemPool::SetDefaults(std::vector<SfxPoolItem*>*)
 *  @see SfxItemPool::ReleaseDefaults(std::vector<SfxPoolItem*>*,bool)
 *  @see SfxItemPool::ReleaseDefaults(bool)
 */
SfxItemPool::SfxItemPool
(
    const OUString&     rName,          /* Pool name to identify in the file format */
    sal_uInt16          nStartWhich,    /* First WhichId of the Pool (must be > 0) */
    sal_uInt16          nEndWhich,      /* Last WhichId of the Pool */
    const SfxItemInfo*  pInfo,          /* SID Map and Item flags */
    std::vector<SfxPoolItem*>*
                        pDefaults       /* Pointer to static Defaults;
                                           is directly referenced by the Pool,
                                           but no transfer of ownership */
)
: salhelper::SimpleReferenceObject()
,pItemInfos(pInfo)
, aName(rName)
, maPoolDefaults(nEndWhich - nStartWhich + 1)
, mpStaticDefaults(nullptr)
, mpMaster(this)
, mnStart(nStartWhich)
, mnEnd(nEndWhich)
, eDefMetric(MapUnit::MapCM)
, maRegisteredSfxItemSets()
, maRegisteredSfxPoolItemHolders()
, maDirectPutItems()
, mbPreDeleteDone(false)
{
    eDefMetric = MapUnit::MapTwip;

    if ( pDefaults )
        SetDefaults(pDefaults);

#ifdef DBG_UTIL
    if (pItemInfos)
    {
        auto p = pItemInfos;
        auto nWhich = nStartWhich;
        while (nWhich <= nEndWhich)
        {
            if (p->_nItemInfoSlotID == nWhich)
            {
                SAL_WARN("svl.items", "No point mapping a SID to itself, just put a 0 here in the SfxItemInfo array, at index " << (p - pItemInfos));
                assert(false);
            }
            ++p;
            ++nWhich;
        }
    }
#endif
}


/**
 * Copy ctor
 *
 * @see SfxItemPool::Clone() const
*/
SfxItemPool::SfxItemPool
(
    const SfxItemPool&  rPool,                  //  Copy from this instance
    bool                bCloneStaticDefaults    /*  true
                                                    Copy static Defaults

                                                    false
                                                    Take over static Defaults */
)
: salhelper::SimpleReferenceObject()
, pItemInfos(rPool.pItemInfos)
, aName(rPool.aName)
, maPoolDefaults(rPool.mnEnd - rPool.mnStart + 1)
, mpStaticDefaults(nullptr)
, mpMaster(this)
, mnStart(rPool.mnStart)
, mnEnd(rPool.mnEnd)
, eDefMetric(MapUnit::MapCM)
, maRegisteredSfxItemSets()
, maRegisteredSfxPoolItemHolders()
, maDirectPutItems()
, mbPreDeleteDone(false)
{
    eDefMetric = rPool.eDefMetric;

    // Take over static Defaults
    if ( bCloneStaticDefaults )
    {
        std::vector<SfxPoolItem *>* ppDefaults = new std::vector<SfxPoolItem*>(mnEnd-mnStart+1);
        for ( sal_uInt16 n = 0; n <= mnEnd - mnStart; ++n )
        {
            (*ppDefaults)[n] = (*rPool.mpStaticDefaults)[n]->Clone(this);
            (*ppDefaults)[n]->setStaticDefault();
        }

        SetDefaults( ppDefaults );
    }
    else
        SetDefaults( rPool.mpStaticDefaults );

    // Copy Pool Defaults
    for (size_t n = 0; n < maPoolDefaults.size(); ++n )
        if (rPool.maPoolDefaults[n])
        {
            maPoolDefaults[n] = rPool.maPoolDefaults[n]->Clone(this); //resets kind
            maPoolDefaults[n]->setPoolDefault();
        }

    // Repair linkage
    if ( rPool.mpSecondary )
        SetSecondaryPool( rPool.mpSecondary->Clone().get() );
}

void SfxItemPool::SetDefaults( std::vector<SfxPoolItem*>* pDefaults )
{
    DBG_ASSERT( pDefaults, "first we ask for it, and then we don't give back..." );
    DBG_ASSERT( !mpStaticDefaults, "already have Defaults" );

    mpStaticDefaults = pDefaults;
    //! if ((*mpStaticDefaults)->GetKind() != SfxItemKind::StaticDefault)
    //! FIXME: Probably doesn't work with SetItems at the end
    {
        DBG_ASSERT( (*mpStaticDefaults)[0]->GetRefCount() == 0 ||
                    IsDefaultItem( (*mpStaticDefaults)[0] ),
                    "these are not static" );
        for ( sal_uInt16 n = 0; n <= mnEnd - mnStart; ++n )
        {
            assert(  ((*mpStaticDefaults)[n]->Which() == n + mnStart)
                        && "items ids in pool-ranges and in static-defaults do not match" );
            (*mpStaticDefaults)[n]->setStaticDefault();
        }
    }
}

void SfxItemPool::ClearDefaults()
{
    mpStaticDefaults = nullptr;
}

/**
 * Frees the static Defaults of the corresponding SfxItemPool instance
 * and deletes them if specified.
 *
 * The SfxItemPool instance MUST NOT BE USED after this function has
 * been called; only the dtor must be called.
 */
void SfxItemPool::ReleaseDefaults
(
    bool    bDelete     /*  true
                            Deletes the array as well as the single static Defaults

                            false
                            Neither deletes the array not the single static Defaults */
)


{
    DBG_ASSERT( mpStaticDefaults, "requirements not met" );
    ReleaseDefaults( mpStaticDefaults, bDelete );

    // mpStaticDefaults points to deleted memory if bDelete == true.
    if ( bDelete )
        mpStaticDefaults = nullptr;
}


/**
 * Frees the specified static Defaults and also deletes them, if so
 * specified.
 *
 * This method MUST be called AFTER all SfxItemPool instances (which
 * use the specified static Defaults 'pDefault') have been destroyed.
 */
void SfxItemPool::ReleaseDefaults
(
    std::vector<SfxPoolItem*>*
                    pDefaults,  /*  Static Defaults that are to be freed */

    bool            bDelete     /*  true
                                    Deletes the array as well as the specified
                                    static Defaults

                                    false
                                    Neither deletes the array nor the single
                                    static Defaults */
)
{
    DBG_ASSERT( pDefaults, "we first ask for it and the return nothing ..." );

    for ( auto & rpItem : *pDefaults )
    {
        assert(IsStaticDefaultItem(rpItem));
        rpItem->SetRefCount(0);
        if ( bDelete )
        {
            delete rpItem;
            rpItem = nullptr;
        }
    }

    if ( bDelete )
    {
        delete pDefaults;
        pDefaults = nullptr;
    }
}


SfxItemPool::~SfxItemPool()
{
    // Need to be deleted?
    if (!mbPreDeleteDone)//maPoolDefaults.empty())
        Delete();

    if (mpMaster != nullptr && mpMaster != this)
    {
        // This condition indicates an error.
        // A mpMaster->SetSecondaryPool(...) call should have been made
        // earlier to prevent this. At this point we can only try to
        // prevent a crash later on.
        DBG_ASSERT( mpMaster == this, "destroying active Secondary-Pool" );
        if (mpMaster->mpSecondary == this)
            mpMaster->mpSecondary = nullptr;
    }
}

void SfxItemPool::SetSecondaryPool( SfxItemPool *pPool )
{
    // Reset Master in attached Pools
    if ( mpSecondary )
    {
        mpSecondary->mpMaster = mpSecondary.get();
        for ( SfxItemPool *p = mpSecondary->mpSecondary.get(); p; p = p->mpSecondary.get() )
            p->mpMaster = mpSecondary.get();
    }

    // Set Master of new Secondary Pools
    DBG_ASSERT( !pPool || pPool->mpMaster == pPool, "Secondary is present in two Pools" );
    SfxItemPool *pNewMaster = GetMasterPool() ? mpMaster : this;
    for ( SfxItemPool *p = pPool; p; p = p->mpSecondary.get() )
        p->mpMaster = pNewMaster;

    // Remember new Secondary Pool
    mpSecondary = pPool;

    CHECK_SLOTS();
}

void SfxItemPool::SetItemInfos(SfxItemInfo const*const pInfo)
{
    pItemInfos = pInfo;
    CHECK_SLOTS();
}


MapUnit SfxItemPool::GetMetric( sal_uInt16 ) const
{
    return eDefMetric;
}


void SfxItemPool::SetDefaultMetric( MapUnit eNewMetric )
{
//    assert((pImpl->eDefMetric == eNewMetric || !pImpl->mpPoolRanges) && "pool already frozen, cannot change metric");
    eDefMetric = eNewMetric;
}

MapUnit SfxItemPool::GetDefaultMetric() const
{
    return eDefMetric;
}

const OUString& SfxItemPool::GetName() const
{
    return aName;
}


bool SfxItemPool::GetPresentation
(
    const SfxPoolItem&  rItem,
    MapUnit             eMetric,
    OUString&           rText,
    const IntlWrapper&  rIntlWrapper
)   const
{
    return rItem.GetPresentation(
        SfxItemPresentation::Complete, GetMetric(rItem.Which()), eMetric, rText, rIntlWrapper );
}


rtl::Reference<SfxItemPool> SfxItemPool::Clone() const
{
    return new SfxItemPool( *this );
}


void SfxItemPool::Delete()
{
    // Already deleted?
    if (mbPreDeleteDone)//maPoolDefaults.empty())
        return;
    mbPreDeleteDone = true;

    // Inform e.g. running Requests
    aBC.Broadcast( SfxHint( SfxHintId::Dying ) );

    // delete direct put items, may assert here when not empty
    for (const auto& rCand : maDirectPutItems)
        delete rCand;
    maDirectPutItems.clear();

    // default items
    for (auto rItemPtr : maPoolDefaults)
    {
        if (rItemPtr)
        {
#ifdef DBG_UTIL
            ClearRefCount(*rItemPtr);
#endif
            delete rItemPtr;
            rItemPtr = nullptr;
        }
    }

    maPoolDefaults.clear();
    mpPoolRanges.reset();
}


void SfxItemPool::SetPoolDefaultItem(const SfxPoolItem &rItem)
{
    if ( IsInRange(rItem.Which()) )
    {
        auto& rOldDefault =
            maPoolDefaults[GetIndex_Impl(rItem.Which())];
        SfxPoolItem *pNewDefault = rItem.Clone(this);
        pNewDefault->setPoolDefault();
        if (rOldDefault)
        {
            rOldDefault->SetRefCount(0);
            delete rOldDefault;
            rOldDefault = nullptr;
        }
        rOldDefault = pNewDefault;
    }
    else if ( mpSecondary )
        mpSecondary->SetPoolDefaultItem(rItem);
    else
    {
        assert(false && "unknown WhichId - cannot set pool default");
    }
}

/**
 * Resets the default of the given WhichId back to the static Default.
 * If a pool default exists, it is removed.
 */
void SfxItemPool::ResetPoolDefaultItem( sal_uInt16 nWhichId )
{
    if ( IsInRange(nWhichId) )
    {
        auto& rOldDefault =
            maPoolDefaults[GetIndex_Impl(nWhichId)];
        if (rOldDefault)
        {
            rOldDefault->SetRefCount(0);
            delete rOldDefault;
            rOldDefault = nullptr;
        }
    }
    else if ( mpSecondary )
        mpSecondary->ResetPoolDefaultItem(nWhichId);
    else
    {
        assert(false && "unknown WhichId - cannot reset pool default");
    }
}

const SfxPoolItem& SfxItemPool::DirectPutItemInPool(const SfxPoolItem& rItem)
{
#ifdef DBG_UTIL
    nAllDirectlyPooledSfxPoolItemCount++;
    nRemainingDirectlyPooledSfxPoolItemCount++;
#endif
    // use SfxPoolItemHolder now to secure lifetime
    SfxPoolItemHolder* pHolder(new SfxPoolItemHolder(*GetMasterPool(), &rItem));
    GetMasterPool()->maDirectPutItems.insert(pHolder);
    return *pHolder->getItem();
}

void SfxItemPool::DirectRemoveItemFromPool(const SfxPoolItem& rItem)
{
#ifdef DBG_UTIL
    nRemainingDirectlyPooledSfxPoolItemCount--;
#endif
    directPutSfxPoolItemHolders& rDirects(GetMasterPool()->maDirectPutItems);
    for (directPutSfxPoolItemHolders::iterator aIter(rDirects.begin()); aIter != rDirects.end(); aIter++)
        if ((*aIter)->getItem() == &rItem)
        {
            delete *aIter;
            rDirects.erase(aIter);
            break;
        }
}

const SfxPoolItem& SfxItemPool::GetDefaultItem( sal_uInt16 nWhich ) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( mpSecondary )
            return mpSecondary->GetDefaultItem( nWhich );
        assert(!"unknown which - don't ask me for defaults");
    }

    DBG_ASSERT( mpStaticDefaults, "no defaults known - don't ask me for defaults" );
    sal_uInt16 nPos = GetIndex_Impl(nWhich);
    SfxPoolItem* pDefault = maPoolDefaults[nPos];
    if ( pDefault )
        return *pDefault;
    return *(*mpStaticDefaults)[nPos];
}

SfxItemPool* SfxItemPool::GetSecondaryPool() const
{
    return mpSecondary.get();
}

/* get the last pool by following the GetSecondaryPool chain */
SfxItemPool* SfxItemPool::GetLastPoolInChain()
{
    SfxItemPool* pLast = this;
    while(pLast->GetSecondaryPool())
        pLast = pLast->GetSecondaryPool();
    return pLast;
}

SfxItemPool* SfxItemPool::GetMasterPool() const
{
    return mpMaster;
}

/**
 * This method should be called at the master pool, when all secondary
 * pools are appended to it.
 *
 * It calculates the ranges of 'which-ids' for fast construction of
 * item-sets, which contains all 'which-ids'.
 */
void SfxItemPool::FreezeIdRanges()
{
    assert(mpPoolRanges.empty() && "pool already frozen, cannot freeze twice");
    FillItemIdRanges_Impl( mpPoolRanges );
}


void SfxItemPool::FillItemIdRanges_Impl( WhichRangesContainer& pWhichRanges ) const
{
    DBG_ASSERT( mpPoolRanges.empty(), "GetFrozenRanges() would be faster!" );

    pWhichRanges.reset();

    // Merge all ranges, keeping them sorted
    for (const SfxItemPool* pPool = this; pPool; pPool = pPool->mpSecondary.get())
        pWhichRanges = pWhichRanges.MergeRange(pPool->mnStart, pPool->mnEnd);
}

const WhichRangesContainer& SfxItemPool::GetFrozenIdRanges() const
{
    return mpPoolRanges;
}

const SfxPoolItem *SfxItemPool::GetItem2Default(sal_uInt16 nWhich) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( mpSecondary )
            return mpSecondary->GetItem2Default( nWhich );
        assert(false && "unknown WhichId - cannot resolve surrogate");
        return nullptr;
    }
    return (*mpStaticDefaults)[ GetIndex_Impl(nWhich) ];
}

void SfxItemPool::CollectSurrogates(std::unordered_set<const SfxPoolItem*>& rTarget, sal_uInt16 nWhich) const
{
    rTarget.clear();

    if (0 == nWhich)
        return;

    // 1st source for surrogates
    const registeredSfxItemSets& rSets(GetMasterPool()->maRegisteredSfxItemSets);
    const SfxPoolItem* pItem(nullptr);
    for (const auto& rCand : rSets)
        if (SfxItemState::SET == rCand->GetItemState(nWhich, false, &pItem))
            rTarget.insert(pItem);

    // 2nd source for surrogates
    const registeredSfxPoolItemHolders& rHolders(GetMasterPool()->maRegisteredSfxPoolItemHolders);
    for (const auto& rCand : rHolders)
        if (rCand->Which() == nWhich && nullptr != rCand->getItem())
            rTarget.insert(rCand->getItem());

    // the 3rd source for surrogates is the list of direct put items
    // but since these use SfxPoolItemHolder now they are automatically
    // registered at 2nd source - IF NeedsSurrogateSupport is set. So
    // as long as we have this DirectPutItem stuff, iterate here and
    // warn if an Item was added
    const directPutSfxPoolItemHolders& rDirects(GetMasterPool()->maDirectPutItems);
#ifdef DBG_UTIL
    const size_t aBefore(rTarget.size());
#endif
    for (const auto& rCand : rDirects)
        if (rCand->Which() == nWhich && nullptr != rCand->getItem())
            rTarget.insert(rCand->getItem());
#ifdef DBG_UTIL
    const size_t aAfter(rTarget.size());
    if (aBefore != aAfter)
    {
        SAL_WARN("svl.items", "SfxItemPool: Found non-automatically registered Item for Surrogates in DirectPutItems (!)");
    }
#endif
}

void SfxItemPool::GetItemSurrogates(ItemSurrogates& rTarget, sal_uInt16 nWhich) const
{
    std::unordered_set<const SfxPoolItem*> aNewSurrogates;
    CollectSurrogates(aNewSurrogates, nWhich);
    rTarget = ItemSurrogates(aNewSurrogates.begin(), aNewSurrogates.end());
}

void SfxItemPool::FindItemSurrogate(ItemSurrogates& rTarget, sal_uInt16 nWhich, SfxPoolItem const & rSample) const
{
    std::unordered_set<const SfxPoolItem*> aNewSurrogates;
    CollectSurrogates(aNewSurrogates, nWhich);
    rTarget.clear();
    for (const auto& rCand : aNewSurrogates)
        if (rSample == *rCand)
            rTarget.push_back(rCand);
}

sal_uInt16 SfxItemPool::GetWhich( sal_uInt16 nSlotId, bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return nSlotId;

    sal_uInt16 nCount = mnEnd - mnStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nItemInfoSlotID == nSlotId )
            return nOfs + mnStart;
    if ( mpSecondary && bDeep )
        return mpSecondary->GetWhich(nSlotId);
    return nSlotId;
}


sal_uInt16 SfxItemPool::GetSlotId( sal_uInt16 nWhich ) const
{
    if ( !IsWhich(nWhich) )
        return nWhich;

    if ( !IsInRange( nWhich ) )
    {
        if ( mpSecondary )
            return mpSecondary->GetSlotId(nWhich);
        assert(false && "unknown WhichId - cannot get slot-id");
        return 0;
    }

    sal_uInt16 nSID = pItemInfos[nWhich - mnStart]._nItemInfoSlotID;
    return nSID ? nSID : nWhich;
}


sal_uInt16 SfxItemPool::GetTrueWhich( sal_uInt16 nSlotId, bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return 0;

    sal_uInt16 nCount = mnEnd - mnStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nItemInfoSlotID == nSlotId )
            return nOfs + mnStart;
    if ( mpSecondary && bDeep )
        return mpSecondary->GetTrueWhich(nSlotId);
    return 0;
}


sal_uInt16 SfxItemPool::GetTrueSlotId( sal_uInt16 nWhich ) const
{
    if ( !IsWhich(nWhich) )
        return 0;

    if ( !IsInRange( nWhich ) )
    {
        if ( mpSecondary )
            return mpSecondary->GetTrueSlotId(nWhich);
        assert(false && "unknown WhichId - cannot get slot-id");
        return 0;
    }
    return pItemInfos[nWhich - mnStart]._nItemInfoSlotID;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
