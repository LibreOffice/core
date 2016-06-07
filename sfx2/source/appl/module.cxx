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

#include <tools/rcid.h>

#include <cstdarg>
#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/stbitem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/tabdlg.hxx>
#include <svl/intitem.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#define SfxModule
#include "sfxslots.hxx"
#include "childwinimpl.hxx"
#include <ctrlfactoryimpl.hxx>
#include <o3tl/make_unique.hxx>

static std::vector<SfxModule*>* pModules=nullptr;

class SfxModule_Impl
{
public:

    std::unique_ptr<SfxSlotPool>                pSlotPool;
    std::unique_ptr<SfxTbxCtrlFactArr_Impl>     pTbxCtrlFac;
    std::unique_ptr<SfxStbCtrlFactArr_Impl>     pStbCtrlFac;
    std::unique_ptr<SfxChildWinFactArr_Impl>    pFactArr;
    std::unique_ptr<ImageList>                  pImgListSmall;
    std::unique_ptr<ImageList>                  pImgListBig;

    ImageList*                  GetImageList( ResMgr* pResMgr, bool bBig );
};


ImageList* SfxModule_Impl::GetImageList( ResMgr* pResMgr, bool bBig )
{
    auto &rpList = bBig ? pImgListBig : pImgListSmall;
    if ( !rpList )
    {
        ResId aResId( bBig ? ( RID_DEFAULTIMAGELIST_LC ) : ( RID_DEFAULTIMAGELIST_SC ), *pResMgr );

        aResId.SetRT( RSC_IMAGELIST );

        DBG_ASSERT( pResMgr->IsAvailable(aResId), "No default ImageList!" );

        if ( pResMgr->IsAvailable(aResId) )
            rpList = o3tl::make_unique<ImageList>( aResId );
        else
            rpList = o3tl::make_unique<ImageList>();
    }

    return rpList.get();
}

SFX_IMPL_SUPERCLASS_INTERFACE(SfxModule, SfxShell)

ResMgr* SfxModule::GetResMgr()
{
    return pResMgr;
}

SfxModule::SfxModule( ResMgr* pMgrP, SfxObjectFactory* pFactoryP, ... )
    : pResMgr( pMgrP ), pImpl( new SfxModule_Impl )
{
    SfxApplication *pApp = SfxGetpApp();
    std::vector<SfxModule*> &rArr = GetModules_Impl();
    rArr.push_back( this );

    pImpl->pSlotPool = o3tl::make_unique<SfxSlotPool>(&pApp->GetAppSlotPool_Impl());
    SetPool( &pApp->GetPool() );

    va_list pVarArgs;
    va_start( pVarArgs, pFactoryP );
    for ( SfxObjectFactory *pArg = pFactoryP; pArg;
         pArg = va_arg( pVarArgs, SfxObjectFactory* ) )
        pArg->SetModule_Impl( this );
    va_end(pVarArgs);
}

SfxModule::~SfxModule()
{
    if ( SfxGetpApp()->Get_Impl() )
    {
        // The module will be destroyed before the Deinitialize,
        // so remove from the array
        std::vector<SfxModule*>& rArr = GetModules_Impl();
        for( sal_uInt16 nPos = rArr.size(); nPos--; )
        {
            if( rArr[ nPos ] == this )
            {
                rArr.erase( rArr.begin() + nPos );
                break;
            }
        }

    }

     delete pResMgr;
}


SfxSlotPool* SfxModule::GetSlotPool() const
{
    return pImpl->pSlotPool.get();
}

void SfxModule::RegisterChildWindow(SfxChildWinFactory *pFact)
{
    DBG_ASSERT( pImpl, "No real Module!" );

    if (!pImpl->pFactArr)
        pImpl->pFactArr = o3tl::make_unique<SfxChildWinFactArr_Impl>();

    for (size_t nFactory=0; nFactory<pImpl->pFactArr->size(); ++nFactory)
    {
        if (pFact->nId ==  (*pImpl->pFactArr)[nFactory].nId)
        {
            pImpl->pFactArr->erase( pImpl->pFactArr->begin() + nFactory );
            SAL_WARN("sfx.appl", "ChildWindow registered multiple times!");
            return;
        }
    }

    pImpl->pFactArr->push_back( pFact );
}


void SfxModule::RegisterToolBoxControl( const SfxTbxCtrlFactory& rFact )
{
    if (!pImpl->pTbxCtrlFac)
        pImpl->pTbxCtrlFac = o3tl::make_unique<SfxTbxCtrlFactArr_Impl>();

#ifdef DBG_UTIL
    for ( size_t n=0; n<pImpl->pTbxCtrlFac->size(); n++ )
    {
        SfxTbxCtrlFactory *pF = &(*pImpl->pTbxCtrlFac)[n];
        if ( pF->nTypeId == rFact.nTypeId &&
            (pF->nSlotId == rFact.nSlotId || pF->nSlotId == 0) )
        {
            SAL_INFO("sfx.appl", "TbxController-Registering is not clearly defined!");
        }
    }
#endif

    pImpl->pTbxCtrlFac->push_back( rFact );
}


void SfxModule::RegisterStatusBarControl( const SfxStbCtrlFactory& rFact )
{
    if (!pImpl->pStbCtrlFac)
        pImpl->pStbCtrlFac = o3tl::make_unique<SfxStbCtrlFactArr_Impl>();

#ifdef DBG_UTIL
    for ( size_t n=0; n<pImpl->pStbCtrlFac->size(); n++ )
    {
        SfxStbCtrlFactory *pF = &(*pImpl->pStbCtrlFac)[n];
        if ( pF->nTypeId == rFact.nTypeId &&
            (pF->nSlotId == rFact.nSlotId || pF->nSlotId == 0) )
        {
            SAL_INFO("sfx.appl", "TbxController-Registering is not clearly defined!");
        }
    }
#endif

    pImpl->pStbCtrlFac->push_back( rFact );
}


SfxTbxCtrlFactArr_Impl*  SfxModule::GetTbxCtrlFactories_Impl() const
{
    return pImpl->pTbxCtrlFac.get();
}


SfxStbCtrlFactArr_Impl*  SfxModule::GetStbCtrlFactories_Impl() const
{
    return pImpl->pStbCtrlFac.get();
}

SfxChildWinFactArr_Impl* SfxModule::GetChildWinFactories_Impl() const
{
    return pImpl->pFactArr.get();
}

ImageList* SfxModule::GetImageList_Impl( bool bBig )
{
    return pImpl->GetImageList( pResMgr, bBig );
}

VclPtr<SfxTabPage> SfxModule::CreateTabPage( sal_uInt16, vcl::Window*, const SfxItemSet& )
{
    return VclPtr<SfxTabPage>();
}

std::vector<SfxModule*>& SfxModule::GetModules_Impl()
{
    if( !pModules )
        pModules = new std::vector<SfxModule*>;
    return *pModules;
};

void SfxModule::DestroyModules_Impl()
{
    if ( pModules )
    {
        for( sal_uInt16 nPos = pModules->size(); nPos--; )
        {
            SfxModule* pMod = (*pModules)[nPos];
            delete pMod;
        }
        delete pModules;
        pModules = nullptr;
    }
}

void SfxModule::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        if ( pFrame->GetObjectShell()->GetModule() == this )
            Invalidate_Impl( pFrame->GetBindings(), nId );
}

SfxModule* SfxModule::GetActiveModule( SfxViewFrame* pFrame )
{
    if ( !pFrame )
        pFrame = SfxViewFrame::Current();
    SfxObjectShell* pSh = nullptr;
    if( pFrame )
        pSh = pFrame->GetObjectShell();
    return pSh ? pSh->GetModule() : nullptr;
}

FieldUnit SfxModule::GetModuleFieldUnit( css::uno::Reference< css::frame::XFrame > const & i_frame )
{
    ENSURE_OR_RETURN( i_frame.is(), "SfxModule::GetModuleFieldUnit: invalid frame!", FUNIT_100TH_MM );

    // find SfxViewFrame for the given XFrame
    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    while ( pViewFrame != nullptr )
    {
        if ( pViewFrame->GetFrame().GetFrameInterface() == i_frame )
            break;
        pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
    }
    ENSURE_OR_RETURN( pViewFrame != nullptr, "SfxModule::GetModuleFieldUnit: unable to find an SfxViewFrame for the given XFrame", FUNIT_100TH_MM );

    // find the module
    SfxModule const * pModule = GetActiveModule( pViewFrame );
    ENSURE_OR_RETURN( pModule != nullptr, "SfxModule::GetModuleFieldUnit: no SfxModule for the given frame!", FUNIT_100TH_MM );
    return pModule->GetFieldUnit();
}

FieldUnit SfxModule::GetCurrentFieldUnit()
{
    FieldUnit eUnit = FUNIT_INCH;
    SfxModule* pModule = GetActiveModule();
    if ( pModule )
    {
        const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
        if ( pItem )
            eUnit = (FieldUnit) static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    }
    else
        SAL_WARN( "sfx.appl", "GetModuleFieldUnit(): no module found" );
    return eUnit;
}

FieldUnit SfxModule::GetFieldUnit() const
{
    FieldUnit eUnit = FUNIT_INCH;
    const SfxPoolItem* pItem = GetItem( SID_ATTR_METRIC );
    if ( pItem )
        eUnit = (FieldUnit) static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    return eUnit;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
