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

#include <poolio.hxx>

#include <cassert>
#include <vector>

#ifdef DBG_UTIL
static size_t nAllDirectlyPooledSfxPoolItemCount(0);
static size_t nRemainingDirectlyPooledSfxPoolItemCount(0);
size_t getAllDirectlyPooledSfxPoolItemCount() { return nAllDirectlyPooledSfxPoolItemCount; }
size_t getRemainingDirectlyPooledSfxPoolItemCount() { return nRemainingDirectlyPooledSfxPoolItemCount; }
#endif

// WhichIDs that need to set _bNeedsPoolRegistration in SfxItemInfo
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
        sal_uInt16 const nSlotId(pInfo[n]._nSID);
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
    for (SfxItemPool * p = pImpl->mpMaster; p; p = p->pImpl->mpSecondary.get()) \
    { \
        lcl_CheckSlots2(slotmap, *p, p->pItemInfos); \
    } \
} while (false)

#else
#define CHECK_SLOTS() do {} while (false)
#endif

sal_uInt16 SfxItemPool::GetFirstWhich() const
{
    return pImpl->mnStart;
}

sal_uInt16 SfxItemPool::GetLastWhich() const
{
    return pImpl->mnEnd;
}

bool SfxItemPool::IsInRange( sal_uInt16 nWhich ) const
{
    return nWhich >= pImpl->mnStart && nWhich <= pImpl->mnEnd;
}

sal_uInt16 SfxItemPool::GetIndex_Impl(sal_uInt16 nWhich) const
{
    if (nWhich < pImpl->mnStart || nWhich > pImpl->mnEnd)
    {
        assert(false && "missing bounds check before use");
        return 0;
    }
    return nWhich - pImpl->mnStart;
}

sal_uInt16 SfxItemPool::GetSize_Impl() const
{
    return pImpl->mnEnd - pImpl->mnStart + 1;
}

const SfxPoolItem* SfxItemPool::GetPoolDefaultItem( sal_uInt16 nWhich ) const
{
    const SfxPoolItem* pRet;
    if( IsInRange( nWhich ) )
        pRet = pImpl->maPoolDefaults[GetIndex_Impl(nWhich)];
    else if( pImpl->mpSecondary )
        pRet = pImpl->mpSecondary->GetPoolDefaultItem( nWhich );
    else
    {
        assert(false && "unknown WhichId - cannot get pool default");
        pRet = nullptr;
    }
    return pRet;
}


bool SfxItemPool::NeedsPoolRegistration(sal_uInt16 nWhich) const
{
    if (!IsInRange(nWhich))
    {
        // get to correct pool
        if (pImpl->mpSecondary)
            return pImpl->mpSecondary->NeedsPoolRegistration(nWhich);
        return false;
    }

    return NeedsPoolRegistration_Impl(nWhich - pImpl->mnStart);
}

bool SfxItemPool::Shareable(sal_uInt16 nWhich) const
{
    if (!IsInRange(nWhich))
    {
        // get to correct pool
        if (pImpl->mpSecondary)
            return pImpl->mpSecondary->Shareable(nWhich);
        return false;
    }

    return Shareable_Impl(nWhich - pImpl->mnStart);
}


SfxBroadcaster& SfxItemPool::BC()
{
    return pImpl->aBC;
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
) :
    pItemInfos(pInfo),
    pImpl( new SfxItemPool_Impl( this, rName, nStartWhich, nEndWhich ) ),
    ppRegisteredSfxPoolItems(nullptr)
{
    pImpl->eDefMetric = MapUnit::MapTwip;

    if ( pDefaults )
        SetDefaults(pDefaults);

#ifdef DBG_UTIL
    if (pItemInfos)
    {
        auto p = pItemInfos;
        auto nWhich = nStartWhich;
        while (nWhich <= nEndWhich)
        {
            if (p->_nSID == nWhich)
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
) :
    salhelper::SimpleReferenceObject(),
    pItemInfos(rPool.pItemInfos),
    pImpl( new SfxItemPool_Impl( this, rPool.pImpl->aName, rPool.pImpl->mnStart, rPool.pImpl->mnEnd ) ),
    ppRegisteredSfxPoolItems(nullptr)
{
    pImpl->eDefMetric = rPool.pImpl->eDefMetric;

    // Take over static Defaults
    if ( bCloneStaticDefaults )
    {
        std::vector<SfxPoolItem *>* ppDefaults = new std::vector<SfxPoolItem*>(pImpl->mnEnd-pImpl->mnStart+1);
        for ( sal_uInt16 n = 0; n <= pImpl->mnEnd - pImpl->mnStart; ++n )
        {
            (*ppDefaults)[n] = (*rPool.pImpl->mpStaticDefaults)[n]->Clone(this);
            (*ppDefaults)[n]->setStaticDefault();
        }

        SetDefaults( ppDefaults );
    }
    else
        SetDefaults( rPool.pImpl->mpStaticDefaults );

    // Copy Pool Defaults
    for (size_t n = 0; n < pImpl->maPoolDefaults.size(); ++n )
        if (rPool.pImpl->maPoolDefaults[n])
        {
            pImpl->maPoolDefaults[n] = rPool.pImpl->maPoolDefaults[n]->Clone(this); //resets kind
            pImpl->maPoolDefaults[n]->setPoolDefault();
        }

    // Repair linkage
    if ( rPool.pImpl->mpSecondary )
        SetSecondaryPool( rPool.pImpl->mpSecondary->Clone().get() );
}

void SfxItemPool::SetDefaults( std::vector<SfxPoolItem*>* pDefaults )
{
    DBG_ASSERT( pDefaults, "first we ask for it, and then we don't give back..." );
    DBG_ASSERT( !pImpl->mpStaticDefaults, "already have Defaults" );

    pImpl->mpStaticDefaults = pDefaults;
    //! if ((*mpStaticDefaults)->GetKind() != SfxItemKind::StaticDefault)
    //! FIXME: Probably doesn't work with SetItems at the end
    {
        DBG_ASSERT( (*pImpl->mpStaticDefaults)[0]->GetRefCount() == 0 ||
                    IsDefaultItem( (*pImpl->mpStaticDefaults)[0] ),
                    "these are not static" );
        for ( sal_uInt16 n = 0; n <= pImpl->mnEnd - pImpl->mnStart; ++n )
        {
            assert(  ((*pImpl->mpStaticDefaults)[n]->Which() == n + pImpl->mnStart)
                        && "items ids in pool-ranges and in static-defaults do not match" );
            (*pImpl->mpStaticDefaults)[n]->setStaticDefault();
            DBG_ASSERT(nullptr == ppRegisteredSfxPoolItems || nullptr == ppRegisteredSfxPoolItems[n]
                || ppRegisteredSfxPoolItems[n]->empty(), "defaults with setitems with items?!" );
        }
    }
}

void SfxItemPool::ClearDefaults()
{
    pImpl->mpStaticDefaults = nullptr;
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
    DBG_ASSERT( pImpl->mpStaticDefaults, "requirements not met" );
    ReleaseDefaults( pImpl->mpStaticDefaults, bDelete );

    // mpStaticDefaults points to deleted memory if bDelete == true.
    if ( bDelete )
        pImpl->mpStaticDefaults = nullptr;
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
    // Caution: ppRegisteredSfxPoolItems is on-demand created and can be nullptr
    if ( nullptr != ppRegisteredSfxPoolItems || !pImpl->maPoolDefaults.empty() )
        Delete();

    if (pImpl->mpMaster != nullptr && pImpl->mpMaster != this)
    {
        // This condition indicates an error.
        // A pImpl->mpMaster->SetSecondaryPool(...) call should have been made
        // earlier to prevent this. At this point we can only try to
        // prevent a crash later on.
        DBG_ASSERT( pImpl->mpMaster == this, "destroying active Secondary-Pool" );
        if (pImpl->mpMaster->pImpl->mpSecondary == this)
            pImpl->mpMaster->pImpl->mpSecondary = nullptr;
    }
}

void SfxItemPool::SetSecondaryPool( SfxItemPool *pPool )
{
    // Reset Master in attached Pools
    if ( pImpl->mpSecondary )
    {
#ifdef DBG_UTIL
        if (nullptr != pImpl->mpStaticDefaults
            && nullptr != ppRegisteredSfxPoolItems
            && nullptr != pImpl->mpSecondary->ppRegisteredSfxPoolItems)
            // Delete() did not yet run?
        {
            // Does the Master have SetItems?
            bool bHasSetItems(false);

            for (sal_uInt16 i(0); !bHasSetItems && i < pImpl->mnEnd - pImpl->mnStart; ++i)
            {
                const SfxPoolItem* pStaticDefaultItem((*pImpl->mpStaticDefaults)[i]);
                bHasSetItems = pStaticDefaultItem->isSetItem();
            }

            if (bHasSetItems)
            {
                // Detached Pools must be empty
                registeredSfxPoolItems** ppSet(pImpl->mpSecondary->ppRegisteredSfxPoolItems);

                for (sal_uInt16 a(0); a < pImpl->mpSecondary->GetSize_Impl(); a++, ppSet++)
                {
                    if (nullptr != *ppSet && !(*ppSet)->empty())
                    {
                        SAL_WARN("svl.items", "old secondary pool: " << pImpl->mpSecondary->pImpl->aName
                                        << " of pool: " << pImpl->aName << " must be empty.");
                        break;
                    }
                }
            }
        }
#endif

        pImpl->mpSecondary->pImpl->mpMaster = pImpl->mpSecondary.get();
        for ( SfxItemPool *p = pImpl->mpSecondary->pImpl->mpSecondary.get(); p; p = p->pImpl->mpSecondary.get() )
            p->pImpl->mpMaster = pImpl->mpSecondary.get();
    }

    // Set Master of new Secondary Pools
    DBG_ASSERT( !pPool || pPool->pImpl->mpMaster == pPool, "Secondary is present in two Pools" );
    SfxItemPool *pNewMaster = GetMasterPool() ? pImpl->mpMaster : this;
    for ( SfxItemPool *p = pPool; p; p = p->pImpl->mpSecondary.get() )
        p->pImpl->mpMaster = pNewMaster;

    // Remember new Secondary Pool
    pImpl->mpSecondary = pPool;

    CHECK_SLOTS();
}

void SfxItemPool::SetItemInfos(SfxItemInfo const*const pInfo)
{
    pItemInfos = pInfo;
    CHECK_SLOTS();
}


MapUnit SfxItemPool::GetMetric( sal_uInt16 ) const
{
    return pImpl->eDefMetric;
}


void SfxItemPool::SetDefaultMetric( MapUnit eNewMetric )
{
//    assert((pImpl->eDefMetric == eNewMetric || !pImpl->mpPoolRanges) && "pool already frozen, cannot change metric");
    pImpl->eDefMetric = eNewMetric;
}

MapUnit SfxItemPool::GetDefaultMetric() const
{
    return pImpl->eDefMetric;
}

const OUString& SfxItemPool::GetName() const
{
    return pImpl->aName;
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
    // Caution: ppRegisteredSfxPoolItems is on-demand created and can be nullptr
    if (nullptr == ppRegisteredSfxPoolItems && pImpl->maPoolDefaults.empty())
        return;

    // Inform e.g. running Requests
    pImpl->aBC.Broadcast( SfxHint( SfxHintId::Dying ) );

    // Iterate through twice: first for the SetItems.
    if (nullptr != pImpl->mpStaticDefaults && nullptr != ppRegisteredSfxPoolItems)
    {
        for (size_t n = 0; n < GetSize_Impl(); ++n)
        {
            // *mpStaticDefaultItem could've already been deleted in a class derived
            // from SfxItemPool
            // This causes chaos in Itempool!
            const SfxPoolItem* pStaticDefaultItem((*pImpl->mpStaticDefaults)[n]);
            if (pStaticDefaultItem->isSetItem() && nullptr != ppRegisteredSfxPoolItems[n])
            {
                // SfxSetItem found, remove PoolItems (and defaults) with same ID
                auto& rArray(*(ppRegisteredSfxPoolItems[n]));
                for (auto& rItemPtr : rArray)
                {
                    ReleaseRef(*rItemPtr, rItemPtr->GetRefCount()); // for RefCount check in dtor
                    delete rItemPtr;
                }
                rArray.clear();
                // let pImpl->DeleteItems() delete item arrays in maPoolItems
                auto& rItemPtr = pImpl->maPoolDefaults[n];
                if (rItemPtr)
                {
#ifdef DBG_UTIL
                    ClearRefCount(*rItemPtr);
#endif
                    delete rItemPtr;
                    rItemPtr = nullptr;
                }
            }
        }
    }

    if (nullptr != ppRegisteredSfxPoolItems)
    {
        registeredSfxPoolItems** ppSet(ppRegisteredSfxPoolItems);

        for (sal_uInt16 a(0); a < GetSize_Impl(); a++, ppSet++)
        {
            if (nullptr != *ppSet)
            {
                for (auto& rCandidate : **ppSet)
                {
                    if (nullptr != rCandidate && !IsDefaultItem(rCandidate))
                    {
                        ReleaseRef(*rCandidate, rCandidate->GetRefCount()); // for RefCount check in dtor
                        delete rCandidate;
                    }
                }

                delete *ppSet;
                *ppSet = nullptr;
            }
        }

        delete[] ppRegisteredSfxPoolItems;
        ppRegisteredSfxPoolItems = nullptr;
    }

    // default items
    for (auto rItemPtr : pImpl->maPoolDefaults)
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

    pImpl->DeleteItems();
}


void SfxItemPool::SetPoolDefaultItem(const SfxPoolItem &rItem)
{
    if ( IsInRange(rItem.Which()) )
    {
        auto& rOldDefault =
            pImpl->maPoolDefaults[GetIndex_Impl(rItem.Which())];
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
    else if ( pImpl->mpSecondary )
        pImpl->mpSecondary->SetPoolDefaultItem(rItem);
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
            pImpl->maPoolDefaults[GetIndex_Impl(nWhichId)];
        if (rOldDefault)
        {
            rOldDefault->SetRefCount(0);
            delete rOldDefault;
            rOldDefault = nullptr;
        }
    }
    else if ( pImpl->mpSecondary )
        pImpl->mpSecondary->ResetPoolDefaultItem(nWhichId);
    else
    {
        assert(false && "unknown WhichId - cannot reset pool default");
    }
}

const SfxPoolItem& SfxItemPool::DirectPutItemInPoolImpl(const SfxPoolItem& rItem, sal_uInt16 nWhich, bool bPassingOwnership)
{
#ifdef DBG_UTIL
    nAllDirectlyPooledSfxPoolItemCount++;
    nRemainingDirectlyPooledSfxPoolItemCount++;
#endif

    // make sure to use 'master'-pool, that's the one used by SfxItemSets
    const SfxPoolItem* pRetval(implCreateItemEntry(*GetMasterPool(), &rItem, nWhich, bPassingOwnership));

    // For the moment, as long as DirectPutItemInPoolImpl is used, make sure that
    // the changes in implCreateItemEntry do not change anything, that would
    // risc memory leaks by not (ab)using the garbage collector aspect of the pool.
    registerSfxPoolItem(*pRetval);

    return *pRetval;
}

void SfxItemPool::DirectRemoveItemFromPool(const SfxPoolItem& rItem)
{
#ifdef DBG_UTIL
    nRemainingDirectlyPooledSfxPoolItemCount--;
#endif

    // make sure to use 'master'-pool, that's the one used by SfxItemSets
    implCleanupItemEntry(*GetMasterPool(), &rItem);
}

const SfxPoolItem& SfxItemPool::GetDefaultItem( sal_uInt16 nWhich ) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetDefaultItem( nWhich );
        assert(!"unknown which - don't ask me for defaults");
    }

    DBG_ASSERT( pImpl->mpStaticDefaults, "no defaults known - don't ask me for defaults" );
    sal_uInt16 nPos = GetIndex_Impl(nWhich);
    SfxPoolItem* pDefault = pImpl->maPoolDefaults[nPos];
    if ( pDefault )
        return *pDefault;
    return *(*pImpl->mpStaticDefaults)[nPos];
}

SfxItemPool* SfxItemPool::GetSecondaryPool() const
{
    return pImpl->mpSecondary.get();
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
    return pImpl->mpMaster;
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
    assert(pImpl->mpPoolRanges.empty() && "pool already frozen, cannot freeze twice");
    FillItemIdRanges_Impl( pImpl->mpPoolRanges );
}


void SfxItemPool::FillItemIdRanges_Impl( WhichRangesContainer& pWhichRanges ) const
{
    DBG_ASSERT( pImpl->mpPoolRanges.empty(), "GetFrozenRanges() would be faster!" );

    pWhichRanges.reset();

    // Merge all ranges, keeping them sorted
    for (const SfxItemPool* pPool = this; pPool; pPool = pPool->pImpl->mpSecondary.get())
        pWhichRanges = pWhichRanges.MergeRange(pPool->pImpl->mnStart, pPool->pImpl->mnEnd);
}

const WhichRangesContainer& SfxItemPool::GetFrozenIdRanges() const
{
    return pImpl->mpPoolRanges;
}

const SfxPoolItem *SfxItemPool::GetItem2Default(sal_uInt16 nWhich) const
{
    if ( !IsInRange(nWhich) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetItem2Default( nWhich );
        assert(false && "unknown WhichId - cannot resolve surrogate");
        return nullptr;
    }
    return (*pImpl->mpStaticDefaults)[ GetIndex_Impl(nWhich) ];
}

#ifdef DBG_UTIL
static void warnForMissingPoolRegistration(const SfxItemPool& rPool, sal_uInt16 nWhich)
{
    if (!rPool.NeedsPoolRegistration(nWhich))
        SAL_INFO("svl.items", "ITEM: ItemSurrogate requested for WhichID " << nWhich <<
        " class " << typeid(rPool.GetDefaultItem(nWhich)).name() <<
        ": needs _bNeedsPoolRegistration==true in SfxItemInfo for that slot");
}
#endif

const registeredSfxPoolItems& SfxItemPool::GetItemSurrogates(sal_uInt16 nWhich) const
{
    static const registeredSfxPoolItems EMPTY;

    if (!IsInRange(nWhich))
    {
        if (pImpl->mpSecondary)
            return pImpl->mpSecondary->GetItemSurrogates(nWhich);
        return EMPTY;
    }

    if (nullptr == ppRegisteredSfxPoolItems)
    {
#ifdef DBG_UTIL
        warnForMissingPoolRegistration(*this, nWhich);
#endif
        return EMPTY;
    }

    registeredSfxPoolItems* pSet(ppRegisteredSfxPoolItems[nWhich - pImpl->mnStart]);

    if (nullptr == pSet)
    {
#ifdef DBG_UTIL
        warnForMissingPoolRegistration(*this, nWhich);
#endif
        return EMPTY;
    }

    return *pSet;
}

std::vector<const SfxPoolItem*> SfxItemPool::FindItemSurrogate(sal_uInt16 nWhich, SfxPoolItem const & rSample) const
{
    static const std::vector<const SfxPoolItem*> EMPTY;

    if (nullptr == ppRegisteredSfxPoolItems)
        return EMPTY;

    if ( !IsInRange(nWhich) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->FindItemSurrogate( nWhich, rSample );
        assert(false && "unknown WhichId - cannot resolve surrogate");
        return EMPTY;
    }

    // get index (must exist due to checks above)
    const sal_uInt16 nIndex(rSample.Which() - pImpl->mnStart);

    if (nullptr == ppRegisteredSfxPoolItems)
    {
#ifdef DBG_UTIL
        warnForMissingPoolRegistration(*this, nWhich);
#endif
        return EMPTY;
    }

    // get registeredSfxPoolItems container
    registeredSfxPoolItems* pSet(ppRegisteredSfxPoolItems[nIndex]);

    if (nullptr == pSet)
    {
#ifdef DBG_UTIL
        warnForMissingPoolRegistration(*this, nWhich);
#endif
        return EMPTY;
    }

    std::vector<const SfxPoolItem*> rv;

    for (const SfxPoolItem* p : *pSet)
        if (rSample == *p)
            rv.push_back(p);

    return rv;
}

sal_uInt16 SfxItemPool::GetWhich( sal_uInt16 nSlotId, bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return nSlotId;

    sal_uInt16 nCount = pImpl->mnEnd - pImpl->mnStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nSID == nSlotId )
            return nOfs + pImpl->mnStart;
    if ( pImpl->mpSecondary && bDeep )
        return pImpl->mpSecondary->GetWhich(nSlotId);
    return nSlotId;
}


sal_uInt16 SfxItemPool::GetSlotId( sal_uInt16 nWhich ) const
{
    if ( !IsWhich(nWhich) )
        return nWhich;

    if ( !IsInRange( nWhich ) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetSlotId(nWhich);
        assert(false && "unknown WhichId - cannot get slot-id");
        return 0;
    }

    sal_uInt16 nSID = pItemInfos[nWhich - pImpl->mnStart]._nSID;
    return nSID ? nSID : nWhich;
}


sal_uInt16 SfxItemPool::GetTrueWhich( sal_uInt16 nSlotId, bool bDeep ) const
{
    if ( !IsSlot(nSlotId) )
        return 0;

    sal_uInt16 nCount = pImpl->mnEnd - pImpl->mnStart + 1;
    for ( sal_uInt16 nOfs = 0; nOfs < nCount; ++nOfs )
        if ( pItemInfos[nOfs]._nSID == nSlotId )
            return nOfs + pImpl->mnStart;
    if ( pImpl->mpSecondary && bDeep )
        return pImpl->mpSecondary->GetTrueWhich(nSlotId);
    return 0;
}


sal_uInt16 SfxItemPool::GetTrueSlotId( sal_uInt16 nWhich ) const
{
    if ( !IsWhich(nWhich) )
        return 0;

    if ( !IsInRange( nWhich ) )
    {
        if ( pImpl->mpSecondary )
            return pImpl->mpSecondary->GetTrueSlotId(nWhich);
        assert(false && "unknown WhichId - cannot get slot-id");
        return 0;
    }
    return pItemInfos[nWhich - pImpl->mnStart]._nSID;
}

void SfxItemPool::registerSfxPoolItem(const SfxPoolItem& rItem)
{
    assert(rItem.Which() != 0);

    if (IsSlot(rItem.Which()))
        // do not register SlotItems
        return;

    if (rItem.isRegisteredAtPool())
        // already registered, done
        return;

    if (!IsInRange(rItem.Which()))
    {
        // get to the right pool
        if (pImpl->mpSecondary)
        {
            pImpl->mpSecondary->registerSfxPoolItem(rItem);
            return;
        }

        return;
    }

    if (nullptr == ppRegisteredSfxPoolItems)
        // on-demand allocate array of registeredSfxPoolItems and init to nullptr
        ppRegisteredSfxPoolItems = new registeredSfxPoolItems*[GetSize_Impl()]{};

    // get correct registeredSfxPoolItems
    const sal_uInt16 nIndex(rItem.Which() - pImpl->mnStart);
    registeredSfxPoolItems* pSet(ppRegisteredSfxPoolItems[nIndex]);

    if (nullptr == pSet)
        // on-demand allocate
        ppRegisteredSfxPoolItems[nIndex] = pSet = new registeredSfxPoolItems;

    // insert to registeredSfxPoolItems and set flag at Item
    pSet->insert(&rItem);
    const_cast<SfxPoolItem&>(rItem).setRegisteredAtPool(true);
}

void SfxItemPool::unregisterSfxPoolItem(const SfxPoolItem& rItem)
{
    if (!rItem.isRegisteredAtPool())
        // Item is not registered, done
        return;

    if (!IsInRange(rItem.Which()))
    {
        // get to the right pool
        if (pImpl->mpSecondary)
        {
            pImpl->mpSecondary->unregisterSfxPoolItem(rItem);
            return;
        }

        assert(false && "unknown WhichId - cannot execute unregisterSfxPoolItem");
        return;
    }

    // we need a valid WhichID and the array of containers has to exist
    assert(rItem.Which() != 0);
    assert(nullptr != ppRegisteredSfxPoolItems);

    // get index (must exist due to checks above)
    const sal_uInt16 nIndex(rItem.Which() - pImpl->mnStart);

    // a valid registeredSfxPoolItems container has to exist
    registeredSfxPoolItems* pSet(ppRegisteredSfxPoolItems[nIndex]);
    assert(nullptr != pSet);

    // remove registered Item and reset flag at Item
    pSet->erase(&rItem);
    const_cast<SfxPoolItem&>(rItem).setRegisteredAtPool(false);
}

bool SfxItemPool::isSfxPoolItemRegisteredAtThisPool(const SfxPoolItem& rItem) const
{
    if (!rItem.isRegisteredAtPool())
        // Item is not registered at all, so also not at this Pool
        return false;

    if (IsSlot(rItem.Which()))
        // do not check being registered for SlotItems
        return false;

    if (!IsInRange(rItem.Which()))
    {
        // get to the right pool
        if (pImpl->mpSecondary)
            return pImpl->mpSecondary->isSfxPoolItemRegisteredAtThisPool(rItem);
        return false;
    }

    // we need a valid WhichID
    assert(rItem.Which() != 0);

    if (nullptr == ppRegisteredSfxPoolItems)
        // when no array of containers exists the Item is not registered
        return false;

    // get index (must exist due to checks above)
    const sal_uInt16 nIndex(rItem.Which() - pImpl->mnStart);

    // get registeredSfxPoolItems container
    registeredSfxPoolItems* pSet(ppRegisteredSfxPoolItems[nIndex]);

    if (nullptr == pSet)
        // when no registeredSfxPoolItems container exists the Item is not registered
        return false;

    // test if Item is registered
    return pSet->find(&rItem) != pSet->end();
}

const SfxPoolItem* SfxItemPool::tryToGetEqualItem(const SfxPoolItem& rItem, sal_uInt16 nWhich) const
{
    if (IsSlot(nWhich))
        // SlotItems are not registered @pool and not in any range
        return nullptr;

    if (!IsInRange(nWhich))
    {
        // get to the right pool
        if (pImpl->mpSecondary)
            return pImpl->mpSecondary->tryToGetEqualItem(rItem, nWhich);
        return nullptr;
    }

    if (nullptr == ppRegisteredSfxPoolItems)
        // no Items at all
        return nullptr;

    // get index (must exist due to checks above)
    const sal_uInt16 nIndex(nWhich - pImpl->mnStart);

    if (!Shareable_Impl(nIndex))
        // not shareable
        return nullptr;

    // get registeredSfxPoolItems container
    registeredSfxPoolItems* pSet(ppRegisteredSfxPoolItems[nIndex]);

    if (nullptr == pSet)
        // no registeredSfxPoolItems for this WhichID
        return nullptr;

    for (const auto& rCandidate : *pSet)
        if (*rCandidate == rItem)
            return rCandidate;

    return nullptr;
}

void SfxItemPool::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SfxItemPool"));

    if (nullptr != ppRegisteredSfxPoolItems)
    {
        registeredSfxPoolItems** ppSet(ppRegisteredSfxPoolItems);

        for (sal_uInt16 a(0); a < GetSize_Impl(); a++, ppSet++)
        {
            if (nullptr != *ppSet)
            {
                for (auto& rCandidate : **ppSet)
                {
                    if (nullptr != rCandidate)
                    {
                        rCandidate->dumpAsXml(pWriter);
                    }
                }
            }
        }
    }

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
