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


#include <stdlib.h>
#include <tools/rcid.h>
#include <tools/stream.hxx>

#include <sfx2/module.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/msgpool.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/objsh.hxx>
#include <rtl/strbuf.hxx>

DBG_NAME(SfxInterface)

//====================================================================

extern "C"
#ifdef WNT
int _cdecl
#else
int
#endif

SfxCompareSlots_Impl( const void* pSmaller, const void* pBigger )
{
    return ( (int) ((SfxSlot*)pSmaller)->GetSlotId() ) -
           ( (int) ((SfxSlot*)pBigger)->GetSlotId() );
}

//=========================================================================

struct SfxObjectUI_Impl
{
    sal_uInt16  nPos;
    ResId   aResId;
    sal_Bool    bVisible;
    sal_Bool    bContext;
    OUString* pName;
    sal_uInt32  nFeature;

    SfxObjectUI_Impl(sal_uInt16 n, const ResId& rResId, sal_Bool bVis, sal_uInt32 nFeat) :
        nPos(n),
        aResId(rResId.GetId(), *rResId.GetResMgr()),
        bVisible(bVis),
        bContext(sal_False),
        pName(0),
        nFeature(nFeat)
    {
        aResId.SetRT(rResId.GetRT());
    }

    ~SfxObjectUI_Impl()
    {
        delete pName;
    }
};

typedef std::vector<SfxObjectUI_Impl*> SfxObjectUIArr_Impl;

struct SfxInterface_Impl
{
    SfxObjectUIArr_Impl     aObjectBars;    // registered ObjectBars
    SfxObjectUIArr_Impl     aChildWindows;  // registered ChildWindows
    ResId                   aPopupRes;      // registered PopupMenu
    ResId                   aStatBarRes;    // registered StatusBar
    SfxModule*              pModule;
    sal_Bool                bRegistered;

    SfxInterface_Impl() :
        aPopupRes(0,*SfxApplication::GetOrCreate()->GetSfxResManager()),
        aStatBarRes(0,*SfxApplication::GetOrCreate()->GetSfxResManager())
    , pModule(NULL)
    , bRegistered(sal_False)
    {
    }

    ~SfxInterface_Impl()
    {
        for (SfxObjectUIArr_Impl::const_iterator it = aObjectBars.begin(); it != aObjectBars.end(); ++it)
            delete *it;

        for (SfxObjectUIArr_Impl::const_iterator it = aChildWindows.begin(); it != aChildWindows.end(); ++it)
            delete *it;
    }
};

static SfxObjectUI_Impl* CreateObjectBarUI_Impl( sal_uInt16 nPos, const ResId& rResId, sal_uInt32 nFeature, const OUString *pStr );

//====================================================================

//====================================================================
// constuctor, registeres a new unit

SfxInterface::SfxInterface( const char *pClassName,
                            const ResId& rNameResId,
                            SfxInterfaceId nId,
                            const SfxInterface* pParent,
                            SfxSlot &rSlotMap, sal_uInt16 nSlotCount ):
    pName(pClassName),
    pGenoType(pParent),
    nClassId(nId),
    aNameResId(rNameResId.GetId(),*rNameResId.GetResMgr()),
    pImpData(0)
{
    pImpData = new SfxInterface_Impl;
    SetSlotMap( rSlotMap, nSlotCount );
}

void SfxInterface::Register( SfxModule* pMod )
{
    pImpData->bRegistered = sal_True;
    pImpData->pModule = pMod;
    if ( pMod )
        pMod->GetSlotPool()->RegisterInterface(*this);
    else
        SFX_APP()->GetAppSlotPool_Impl().RegisterInterface(*this);
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
        qsort( pSlots, nCount, sizeof(SfxSlot), SfxCompareSlots_Impl );

        // link masters and slaves
        sal_uInt16 nIter = 1;
        for ( pIter = pSlots; nIter <= nCount; ++pIter, ++nIter )
        {

            DBG_ASSERT( nIter == nCount ||
                        pIter->GetSlotId() != (pIter+1)->GetSlotId(),
                        "doppelte SID" );

            // every master refers to his first slave (ENUM),
            // all slaves refer to their master.
            // Slaves refer in a circle to the other slaves with the same master
            if ( pIter->GetKind() == SFX_KIND_ENUM )
            {
                pIter->pLinkedSlot = GetSlot( pIter->nMasterSlotId );
                DBG_ASSERT( pIter->pLinkedSlot, "slave without master" );
                if ( !pIter->pLinkedSlot->pLinkedSlot )
                    ( (SfxSlot*) pIter->pLinkedSlot)->pLinkedSlot = pIter;

                if ( 0 == pIter->GetNextSlot() )
                {
                    SfxSlot *pLastSlot = pIter;
                    for ( sal_uInt16 n = nIter; n < Count(); ++n )
                    {
                        SfxSlot *pCurSlot = (pSlots+n);
                        if ( pCurSlot->nMasterSlotId == pIter->nMasterSlotId )
                        {
                            pLastSlot->pNextSlot = pCurSlot;
                            pLastSlot = pCurSlot;
                        }
                    }
                    pLastSlot->pNextSlot = pIter;
                }
            }
            else if ( 0 == pIter->GetNextSlot() )
            {
                // Slots refering in circle to the next with the same
                // Status method.
                SfxSlot *pLastSlot = pIter;
                for ( sal_uInt16 n = nIter; n < Count(); ++n )
                {
                    SfxSlot *pCurSlot = (pSlots+n);
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
                OSL_FAIL("Wrong order!");

            if ( pIter->GetKind() == SFX_KIND_ENUM )
            {
                const SfxSlot *pMasterSlot = GetSlot(pIter->nMasterSlotId);
                const SfxSlot *pFirstSlave = pMasterSlot->pLinkedSlot;
                const SfxSlot *pSlave = pFirstSlave;
                do
                {
                    if ( pSlave->pLinkedSlot != pMasterSlot )
                    {
                        OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                            "Wrong Master/Slave- link: "));
                        aStr.append(static_cast<sal_Int32>(
                            pMasterSlot->GetSlotId()));
                        aStr.append(RTL_CONSTASCII_STRINGPARAM(" , "));
                        aStr.append(static_cast<sal_Int32>(
                            pSlave->GetSlotId()));
                        OSL_FAIL(aStr.getStr());
                    }

                    if ( pSlave->nMasterSlotId != pMasterSlot->GetSlotId() )
                    {
                        OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                            "Wrong Master/Slave-Ids: "));
                        aStr.append(static_cast<sal_Int32>(
                            pMasterSlot->GetSlotId()));
                        aStr.append(RTL_CONSTASCII_STRINGPARAM(" , "));
                        aStr.append(static_cast<sal_Int32>(
                            pSlave->GetSlotId()));
                        OSL_FAIL(aStr.getStr());
                    }

                    pSlave = pSlave->pNextSlot;
                }
                while ( pSlave != pFirstSlave );
            }
            else
            {
                if ( pIter->pLinkedSlot )
                {
                    if ( pIter->pLinkedSlot->GetKind() != SFX_KIND_ENUM )
                    {
                        OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                            "Slave is no enum: "));
                        aStr.append(static_cast<sal_Int32>(pIter->GetSlotId()));
                        aStr.append(RTL_CONSTASCII_STRINGPARAM(" , "));
                        aStr.append(static_cast<sal_Int32>(
                            pIter->pLinkedSlot->GetSlotId()));
                        OSL_FAIL(aStr.getStr());
                    }
                }

                const SfxSlot *pCurSlot = pIter;
                do
                {
                    pCurSlot = pCurSlot->pNextSlot;
                    if ( pCurSlot->GetStateFnc() != pIter->GetStateFnc() )
                    {
                        OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                            "Linked Slots with different State Methods : "));
                        aStr.append(static_cast<sal_Int32>(
                            pCurSlot->GetSlotId()));
                        aStr.append(RTL_CONSTASCII_STRINGPARAM(" , "));
                        aStr.append(static_cast<sal_Int32>(pIter->GetSlotId()));
                        OSL_FAIL(aStr.getStr());
                    }
                }
                while ( pCurSlot != pIter );
            }

            pIter = pNext;
        }
    }
#endif
}

//--------------------------------------------------------------------

SfxInterface::~SfxInterface()
{
    SfxModule *pMod = pImpData->pModule;
    sal_Bool bRegistered = pImpData->bRegistered;
    delete pImpData;
    DBG_ASSERT( bRegistered, "Interface not registered!" );
    if ( bRegistered )
    {
        if ( pMod )
            pMod->GetSlotPool()->ReleaseInterface(*this);
        else
            SFX_APP()->GetAppSlotPool_Impl().ReleaseInterface(*this);
    }
}

//--------------------------------------------------------------------

// searches for the specified func

const SfxSlot* SfxInterface::GetSlot( sal_uInt16 nFuncId ) const
{
    DBG_CHKTHIS(SfxInterface, 0);
    DBG_ASSERT( this && pSlots && nCount, "" );

    // find the id using binary search
    void* p = bsearch( &nFuncId, pSlots, nCount, sizeof(SfxSlot),
                       SfxCompareSlots_Impl );
    if ( !p && pGenoType )
        return pGenoType->GetSlot( nFuncId );

    return p ? (const SfxSlot*)p : 0;
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

    return pGenoType ? pGenoType->GetSlot( aCommand ) : NULL;
}

//--------------------------------------------------------------------

const SfxSlot* SfxInterface::GetRealSlot( const SfxSlot *pSlot ) const
{
    DBG_CHKTHIS(SfxInterface, 0);
    DBG_ASSERT( this && pSlots && nCount, "" );

    if ( !ContainsSlot_Impl(pSlot) )
    {
        if(pGenoType)
            return pGenoType->GetRealSlot(pSlot);
        OSL_FAIL("unknown Slot");
        return 0;
    }

    return pSlot->pLinkedSlot;
}

//--------------------------------------------------------------------

const SfxSlot* SfxInterface::GetRealSlot( sal_uInt16 nSlotId ) const
{
    DBG_CHKTHIS(SfxInterface, 0);
    DBG_ASSERT( this && pSlots && nCount, "" );

    const SfxSlot *pSlot = GetSlot(nSlotId);
    if ( !pSlot )
    {
        if(pGenoType)
            return pGenoType->GetRealSlot(nSlotId);
        OSL_FAIL("unkonown Slot");
        return 0;
    }

    return pSlot->pLinkedSlot;
}

//--------------------------------------------------------------------

void SfxInterface::RegisterPopupMenu( const ResId& rResId )
{
    DBG_CHKTHIS(SfxInterface, 0);
    pImpData->aPopupRes = rResId;
}

//--------------------------------------------------------------------

void SfxInterface::RegisterObjectBar( sal_uInt16 nPos, const ResId& rResId,
        const OUString *pStr )
{
    RegisterObjectBar( nPos, rResId, 0UL, pStr );
}


void SfxInterface::RegisterObjectBar( sal_uInt16 nPos, const ResId& rResId, sal_uInt32 nFeature, const OUString *pStr )
{
    SfxObjectUI_Impl* pUI = CreateObjectBarUI_Impl( nPos, rResId, nFeature, pStr );
    if ( pUI )
        pImpData->aObjectBars.push_back(pUI);
}

SfxObjectUI_Impl* CreateObjectBarUI_Impl( sal_uInt16 nPos, const ResId& rResId, sal_uInt32 nFeature, const OUString *pStr )
{
    if ((nPos & SFX_VISIBILITY_MASK) == 0)
        nPos |= SFX_VISIBILITY_STANDARD;

    SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(nPos, rResId, sal_True, nFeature);

    if (pStr == 0)
    {
        ResId aResId(rResId);
        aResId.SetRT(RSC_STRING);
        aResId.SetResMgr(rResId.GetResMgr());
        if( ! aResId.GetResMgr() )
            aResId.SetResMgr( SfxApplication::GetOrCreate()->GetOffResManager_Impl() );
        if ( !aResId.GetResMgr() || !aResId.GetResMgr()->IsAvailable(aResId) )
            pUI->pName = new OUString ("NoName");
        else
            pUI->pName = new OUString(aResId.toString());
    }
    else
        pUI->pName = new OUString(*pStr);

    return pUI;
}

const ResId& SfxInterface::GetObjectBarResId( sal_uInt16 nNo ) const
{
    sal_Bool bGenoType = (pGenoType != 0 && !pGenoType->HasName());
    if ( bGenoType )
    {
        // Are there toolbars in the super class?
        sal_uInt16 nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetObjectBarResId( nNo );
        else
            nNo = nNo - nBaseCount;
    }

#ifdef DBG_UTIL
    sal_uInt16 nObjBarCount = pImpData->aObjectBars.size();
    DBG_ASSERT( nNo<nObjBarCount,"Objectbar is unknown!" );
#endif
    return pImpData->aObjectBars[nNo]->aResId;
}

//--------------------------------------------------------------------


sal_uInt16 SfxInterface::GetObjectBarPos( sal_uInt16 nNo ) const
{
    sal_Bool bGenoType = (pGenoType != 0 && !pGenoType->HasName());
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

#ifdef DBG_UTIL
    sal_uInt16 nObjBarCount = pImpData->aObjectBars.size();
    DBG_ASSERT( nNo<nObjBarCount,"Objectbar is unknown!" );
#endif
    return pImpData->aObjectBars[nNo]->nPos;
}

//--------------------------------------------------------------------


sal_uInt16 SfxInterface::GetObjectBarCount() const
{
    if (pGenoType && ! pGenoType->HasName())
        return pImpData->aObjectBars.size() + pGenoType->GetObjectBarCount();
    else
        return pImpData->aObjectBars.size();
}

//--------------------------------------------------------------------
void SfxInterface::RegisterChildWindow(sal_uInt16 nId, sal_Bool bContext, const OUString* pChildWinName)
{
    RegisterChildWindow( nId, bContext, 0UL, pChildWinName );
}

void SfxInterface::RegisterChildWindow(sal_uInt16 nId, sal_Bool bContext, sal_uInt32 nFeature, const OUString*)
{
    SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(0, ResId(nId, *SfxApplication::GetOrCreate()->GetOffResManager_Impl()), sal_True, nFeature);
    pUI->bContext = bContext;
    pImpData->aChildWindows.push_back(pUI);
}

void SfxInterface::RegisterStatusBar(const ResId& rResId)
{
    pImpData->aStatBarRes = rResId;
}


sal_uInt32 SfxInterface::GetChildWindowId (sal_uInt16 nNo) const
{
    if ( pGenoType )
    {
        // Are there ChildWindows in der Superklasse?
        sal_uInt16 nBaseCount = pGenoType->GetChildWindowCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetChildWindowId( nNo );
        else
            nNo = nNo - nBaseCount;
    }

#ifdef DBG_UTIL
    sal_uInt16 nCWCount = pImpData->aChildWindows.size();
    DBG_ASSERT( nNo<nCWCount,"ChildWindow is unknown!" );
#endif
    sal_uInt32 nRet = pImpData->aChildWindows[nNo]->aResId.GetId();
    if ( pImpData->aChildWindows[nNo]->bContext )
        nRet += sal_uInt32( nClassId ) << 16;
    return nRet;
}

sal_uInt32 SfxInterface::GetChildWindowFeature (sal_uInt16 nNo) const
{
    if ( pGenoType )
    {
        // Are there ChildWindows in der Superklasse?
        sal_uInt16 nBaseCount = pGenoType->GetChildWindowCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetChildWindowFeature( nNo );
        else
            nNo = nNo - nBaseCount;
    }

#ifdef DBG_UTIL
    sal_uInt16 nCWCount = pImpData->aChildWindows.size();
    DBG_ASSERT( nNo<nCWCount,"ChildWindow is unknown!" );
#endif
    return pImpData->aChildWindows[nNo]->nFeature;
}

//--------------------------------------------------------------------


sal_uInt16 SfxInterface::GetChildWindowCount() const
{
    if (pGenoType)
        return pImpData->aChildWindows.size() + pGenoType->GetChildWindowCount();
    else
        return pImpData->aChildWindows.size();
}


const ResId& SfxInterface::GetPopupMenuResId() const
{
    return pImpData->aPopupRes;
}


const ResId& SfxInterface::GetStatusBarResId() const
{
    if (pImpData->aStatBarRes.GetId() == 0 && pGenoType)
        return pGenoType->GetStatusBarResId();
    else
        return pImpData->aStatBarRes;
}



const OUString* SfxInterface::GetObjectBarName ( sal_uInt16 nNo ) const
{
    sal_Bool bGenoType = (pGenoType != 0 && !pGenoType->HasName());
    if ( bGenoType )
    {
        // Are there toolbars in the super class?
        sal_uInt16 nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // The Super class comes first
            return pGenoType->GetObjectBarName( nNo );
        else
            nNo = nNo - nBaseCount;
    }

#ifdef DBG_UTIL
    sal_uInt16 nObjBarCount = pImpData->aObjectBars.size();
    DBG_ASSERT( nNo<nObjBarCount,"Objectbar is unknown!" );
#endif
    return pImpData->aObjectBars[nNo]->pName;
}

sal_uInt32 SfxInterface::GetObjectBarFeature ( sal_uInt16 nNo ) const
{
    sal_Bool bGenoType = (pGenoType != 0 && !pGenoType->HasName());
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

#ifdef DBG_UTIL
    sal_uInt16 nObjBarCount = pImpData->aObjectBars.size();
    DBG_ASSERT( nNo<nObjBarCount,"Objectbar is unknown!" );
#endif
    return pImpData->aObjectBars[nNo]->nFeature;
}

sal_Bool SfxInterface::IsObjectBarVisible(sal_uInt16 nNo) const
{
    sal_Bool bGenoType = (pGenoType != 0 && !pGenoType->HasName());
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

#ifdef DBG_UTIL
    sal_uInt16 nObjBarCount = pImpData->aObjectBars.size();
    DBG_ASSERT( nNo<nObjBarCount,"Objectbar is unknown!" );
#endif
    return pImpData->aObjectBars[nNo]->bVisible;
}

const SfxInterface* SfxInterface::GetRealInterfaceForSlot( const SfxSlot *pRealSlot ) const
{
    DBG_ASSERT( pImpData->bRegistered, "Interface not registered!" );
    const SfxInterface* pInterface = this;

    // The slot could also originate from the interface of a shell base class.
    do
    {
        const SfxSlot *pLastSlot  = (*pInterface)[pInterface->Count()-1];
        const SfxSlot *pFirstSlot = (*pInterface)[0];

        // Is pInterface the Owner of pRealSlot ?
        if ( pFirstSlot <= pRealSlot && pRealSlot <= pLastSlot )
            break;

        // Otherwise try the Interface of Super class
        pInterface = pInterface->pGenoType;
    }
    while ( pInterface );

    return pInterface;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
