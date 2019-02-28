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

#include <assert.h>
#include <stdlib.h>

#include <sal/log.hxx>

#include <tools/stream.hxx>

#include <sfx2/module.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/objsh.hxx>
#include <rtl/strbuf.hxx>

extern "C" {

static int
SfxCompareSlots_qsort( const void* pSmaller, const void* pBigger )
{
    return static_cast<int>(static_cast<SfxSlot const *>(pSmaller)->GetSlotId()) -
           static_cast<int>(static_cast<SfxSlot const *>(pBigger)->GetSlotId());
}

static int
SfxCompareSlots_bsearch( const void* pSmaller, const void* pBigger )
{
    return static_cast<int>(*static_cast<sal_uInt16 const *>(pSmaller)) -
           static_cast<int>(static_cast<SfxSlot const *>(pBigger)->GetSlotId());
}

}

struct SfxObjectUI_Impl
{
    sal_uInt16 const         nPos;
    SfxVisibilityFlags const nFlags;
    sal_uInt32 const         nObjId;
    bool                     bContext;
    SfxShellFeature const    nFeature;

    SfxObjectUI_Impl(sal_uInt16 n, SfxVisibilityFlags f, sal_uInt32 nId, SfxShellFeature nFeat) :
        nPos(n),
        nFlags(f),
        nObjId(nId),
        bContext(false),
        nFeature(nFeat)
    {
    }
};

struct SfxInterface_Impl
{
    std::vector<std::unique_ptr<SfxObjectUI_Impl>>
                            aObjectBars;    // registered ObjectBars
    std::vector<std::unique_ptr<SfxObjectUI_Impl>>
                            aChildWindows;  // registered ChildWindows
    OUString                aPopupName;     // registered PopupMenu
    StatusBarId             eStatBarResId;  // registered StatusBar
    SfxModule*              pModule;
    bool                    bRegistered;

    SfxInterface_Impl()
        : eStatBarResId(StatusBarId::None)
        , pModule(nullptr)
        , bRegistered(false)
    {
    }
};

static SfxObjectUI_Impl* CreateObjectBarUI_Impl(sal_uInt16 nPos, SfxVisibilityFlags nFlags, ToolbarId eId, SfxShellFeature nFeature);

// constructor, registers a new unit
SfxInterface::SfxInterface( const char *pClassName,
                            bool bUsableSuperClass,
                            SfxInterfaceId nId,
                            const SfxInterface* pParent,
                            SfxSlot &rSlotMap, sal_uInt16 nSlotCount ):
    pName(pClassName),
    pGenoType(pParent),
    nClassId(nId),
    bSuperClass(bUsableSuperClass),
    pImplData(new SfxInterface_Impl)
{
    SetSlotMap( rSlotMap, nSlotCount );
}

void SfxInterface::Register( SfxModule* pMod )
{
    pImplData->bRegistered = true;
    pImplData->pModule = pMod;
    if ( pMod )
        pMod->GetSlotPool()->RegisterInterface(*this);
    else
        SfxGetpApp()->GetAppSlotPool_Impl().RegisterInterface(*this);
}

void SfxInterface::SetSlotMap( SfxSlot& rSlotMap, sal_uInt16 nSlotCount )
{
    pSlots = &rSlotMap;
    nCount = nSlotCount;
    SfxSlot* pIter = pSlots;
    if ( 1 == nCount && !pIter->pNextSlot )
        pIter->pNextSlot = pIter;

    if ( !pIter->pNextSlot )
    {
        // sort the SfxSlots by id
        qsort( pSlots, nCount, sizeof(SfxSlot), SfxCompareSlots_qsort );

        // link masters and slaves
        sal_uInt16 nIter = 1;
        for ( pIter = pSlots; nIter <= nCount; ++pIter, ++nIter )
        {

            assert( nIter == nCount ||
                    pIter->GetSlotId() != (pIter+1)->GetSlotId() );

            if ( nullptr == pIter->GetNextSlot() )
            {
                // Slots referring in circle to the next with the same
                // Status method.
                SfxSlot *pLastSlot = pIter;
                for ( sal_uInt16 n = nIter; n < Count(); ++n )
                {
                    SfxSlot *pCurSlot = pSlots+n;
                    if ( pCurSlot->GetStateFnc() == pIter->GetStateFnc() )
                    {
                        pLastSlot->pNextSlot = pCurSlot;
                        pLastSlot = pCurSlot;
                    }
                }
                pLastSlot->pNextSlot = pIter;
            }
        }
    }
#ifdef DBG_UTIL
    else
    {
        sal_uInt16 nIter = 1;
        for ( SfxSlot *pNext = pIter+1; nIter < nCount; ++pNext, ++nIter )
        {

            if ( pNext->GetSlotId() <= pIter->GetSlotId() )
                SAL_WARN( "sfx.control", "Wrong order" );

            const SfxSlot *pCurSlot = pIter;
            do
            {
                pCurSlot = pCurSlot->pNextSlot;
                if ( pCurSlot->GetStateFnc() != pIter->GetStateFnc() )
                {
                    SAL_WARN("sfx.control", "Linked Slots with different State Methods : "
                                << pCurSlot->GetSlotId()
                                << " , " << pIter->GetSlotId() );
                }
            }
            while ( pCurSlot != pIter );

            pIter = pNext;
        }
    }
#endif
}


SfxInterface::~SfxInterface()
{
    SfxModule *pMod = pImplData->pModule;
    bool bRegistered = pImplData->bRegistered;
    assert( bRegistered );
    if ( bRegistered )
    {
        if ( pMod )
        {
            // can return nullptr if we are called from the SfxSlotPool destructor
            if (pMod->GetSlotPool())
                pMod->GetSlotPool()->ReleaseInterface(*this);
        }
        else
            SfxGetpApp()->GetAppSlotPool_Impl().ReleaseInterface(*this);
    }
}


// searches for the specified func

const SfxSlot* SfxInterface::GetSlot( sal_uInt16 nFuncId ) const
{

    assert( pSlots );
    assert( nCount );

    // find the id using binary search
    void* p = bsearch( &nFuncId, pSlots, nCount, sizeof(SfxSlot),
                       SfxCompareSlots_bsearch );
    if ( !p && pGenoType )
        return pGenoType->GetSlot( nFuncId );

    return static_cast<const SfxSlot*>(p);
}

const SfxSlot* SfxInterface::GetSlot( const OUString& rCommand ) const
{
    static const char UNO_COMMAND[] = ".uno:";

    OUString aCommand( rCommand );
    if ( aCommand.startsWith( UNO_COMMAND ) )
         aCommand = aCommand.copy( sizeof( UNO_COMMAND )-1 );

    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        if ( (pSlots+n)->pUnoName &&
             aCommand.compareToIgnoreAsciiCaseAscii( (pSlots+n)->GetUnoName() ) == 0 )
            return pSlots+n;
    }

    return pGenoType ? pGenoType->GetSlot( aCommand ) : nullptr;
}


const SfxSlot* SfxInterface::GetRealSlot( const SfxSlot *pSlot ) const
{

    assert( pSlots );
    assert( nCount );

    if ( !ContainsSlot_Impl(pSlot) )
    {
        if(pGenoType)
            return pGenoType->GetRealSlot(pSlot);
        SAL_WARN( "sfx.control", "unknown Slot" );
        return nullptr;
    }

    return nullptr;
}


void SfxInterface::RegisterPopupMenu( const OUString& rResourceName )
{
    pImplData->aPopupName = rResourceName;
}

void SfxInterface::RegisterObjectBar(sal_uInt16 nPos, SfxVisibilityFlags nFlags, ToolbarId eId)
{
    RegisterObjectBar(nPos, nFlags, eId, SfxShellFeature::NONE);
}

void SfxInterface::RegisterObjectBar(sal_uInt16 nPos, SfxVisibilityFlags nFlags, ToolbarId eId, SfxShellFeature nFeature)
{
    SfxObjectUI_Impl* pUI = CreateObjectBarUI_Impl(nPos, nFlags, eId, nFeature);
    if ( pUI )
        pImplData->aObjectBars.emplace_back(pUI);
}

SfxObjectUI_Impl* CreateObjectBarUI_Impl(sal_uInt16 nPos, SfxVisibilityFlags nFlags, ToolbarId eId, SfxShellFeature nFeature)
{
    if (nFlags == SfxVisibilityFlags::Invisible)
        nFlags |= SfxVisibilityFlags::Standard;

    return new SfxObjectUI_Impl(nPos, nFlags, static_cast<sal_uInt32>(eId), nFeature);
}

ToolbarId SfxInterface::GetObjectBarId(sal_uInt16 nNo) const
{
    bool bGenoType = (pGenoType != nullptr && pGenoType->UseAsSuperClass());
    if ( bGenoType )
    {
        // Are there toolbars in the super class?
        sal_uInt16 nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetObjectBarId(nNo);
        else
            nNo = nNo - nBaseCount;
    }

    assert( nNo<pImplData->aObjectBars.size() );

    return static_cast<ToolbarId>(pImplData->aObjectBars[nNo]->nObjId);
}

sal_uInt16 SfxInterface::GetObjectBarPos( sal_uInt16 nNo ) const
{
    bool bGenoType = (pGenoType != nullptr && pGenoType->UseAsSuperClass());
    if ( bGenoType )
    {
        // Are there toolbars in the super class?
        sal_uInt16 nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetObjectBarPos( nNo );
        else
            nNo = nNo - nBaseCount;
    }

    assert( nNo<pImplData->aObjectBars.size() );

    return pImplData->aObjectBars[nNo]->nPos;
}

SfxVisibilityFlags SfxInterface::GetObjectBarFlags( sal_uInt16 nNo ) const
{
    bool bGenoType = (pGenoType != nullptr && pGenoType->UseAsSuperClass());
    if ( bGenoType )
    {
        // Are there toolbars in the super class?
        sal_uInt16 nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetObjectBarFlags( nNo );
        else
            nNo = nNo - nBaseCount;
    }

    assert( nNo<pImplData->aObjectBars.size() );

    return pImplData->aObjectBars[nNo]->nFlags;
}

sal_uInt16 SfxInterface::GetObjectBarCount() const
{
    if (pGenoType && pGenoType->UseAsSuperClass())
        return pImplData->aObjectBars.size() + pGenoType->GetObjectBarCount();
    else
        return pImplData->aObjectBars.size();
}

void SfxInterface::RegisterChildWindow(sal_uInt16 nId, bool bContext)
{
    RegisterChildWindow(nId, bContext, SfxShellFeature::NONE);
}

void SfxInterface::RegisterChildWindow(sal_uInt16 nId, bool bContext, SfxShellFeature nFeature)
{
    SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(0, SfxVisibilityFlags::Invisible, nId, nFeature);
    pUI->bContext = bContext;
    pImplData->aChildWindows.emplace_back(pUI);
}

void SfxInterface::RegisterStatusBar(StatusBarId eId)
{
    pImplData->eStatBarResId = eId;
}

sal_uInt32 SfxInterface::GetChildWindowId (sal_uInt16 nNo) const
{
    if ( pGenoType )
    {
        // Are there ChildWindows in the superclass?
        sal_uInt16 nBaseCount = pGenoType->GetChildWindowCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetChildWindowId( nNo );
        else
            nNo = nNo - nBaseCount;
    }

    assert( nNo<pImplData->aChildWindows.size() );

    sal_uInt32 nRet = pImplData->aChildWindows[nNo]->nObjId;
    if ( pImplData->aChildWindows[nNo]->bContext )
        nRet += sal_uInt16( nClassId ) << 16;
    return nRet;
}

SfxShellFeature SfxInterface::GetChildWindowFeature (sal_uInt16 nNo) const
{
    if ( pGenoType )
    {
        // Are there ChildWindows in the superclass?
        sal_uInt16 nBaseCount = pGenoType->GetChildWindowCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetChildWindowFeature( nNo );
        else
            nNo = nNo - nBaseCount;
    }

    assert( nNo<pImplData->aChildWindows.size() );

    return pImplData->aChildWindows[nNo]->nFeature;
}


sal_uInt16 SfxInterface::GetChildWindowCount() const
{
    if (pGenoType)
        return pImplData->aChildWindows.size() + pGenoType->GetChildWindowCount();
    else
        return pImplData->aChildWindows.size();
}

const OUString& SfxInterface::GetPopupMenuName() const
{
    return pImplData->aPopupName;
}

StatusBarId SfxInterface::GetStatusBarId() const
{
    if (pImplData->eStatBarResId == StatusBarId::None && pGenoType)
        return pGenoType->GetStatusBarId();
    else
        return pImplData->eStatBarResId;
}

SfxShellFeature SfxInterface::GetObjectBarFeature ( sal_uInt16 nNo ) const
{
    bool bGenoType = (pGenoType != nullptr && pGenoType->UseAsSuperClass());
    if ( bGenoType )
    {
        // Are there toolbars in the super class?
        sal_uInt16 nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetObjectBarFeature( nNo );
        else
            nNo = nNo - nBaseCount;
    }

    assert( nNo<pImplData->aObjectBars.size() );

    return pImplData->aObjectBars[nNo]->nFeature;
}

bool SfxInterface::IsObjectBarVisible(sal_uInt16 nNo) const
{
    bool bGenoType = (pGenoType != nullptr && pGenoType->UseAsSuperClass());
    if ( bGenoType )
    {
        // Are there toolbars in the super class?
        sal_uInt16 nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->IsObjectBarVisible( nNo );
        else
            nNo = nNo - nBaseCount;
    }

    assert( nNo<pImplData->aObjectBars.size() );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
