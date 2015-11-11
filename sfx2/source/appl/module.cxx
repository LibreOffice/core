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
#include <sfx2/mnuitem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/intitem.hxx>
#include <sfx2/taskpane.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#define SfxModule
#include "sfxslots.hxx"
#include "childwinimpl.hxx"
#include <ctrlfactoryimpl.hxx>

class SfxModuleArr_Impl
{
    typedef ::std::vector<SfxModule*> DataType;
    DataType maData;
public:

    typedef DataType::iterator iterator;

    iterator begin()
    {
        return maData.begin();
    }

    void erase( iterator it )
    {
        maData.erase(it);
    }

    SfxModule* operator[] ( size_t i )
    {
        return maData[i];
    }

    void push_back( SfxModule* p )
    {
        maData.push_back(p);
    }

    size_t size() const
    {
        return maData.size();
    }
};

static SfxModuleArr_Impl* pModules=nullptr;

class SfxModule_Impl
{
public:

    SfxSlotPool*                pSlotPool;
    SfxTbxCtrlFactArr_Impl*     pTbxCtrlFac;
    SfxStbCtrlFactArr_Impl*     pStbCtrlFac;
    SfxMenuCtrlFactArr_Impl*    pMenuCtrlFac;
    SfxChildWinFactArr_Impl*    pFactArr;
    ImageList*                  pImgListSmall;
    ImageList*                  pImgListBig;

                                SfxModule_Impl();
                                ~SfxModule_Impl();
    ImageList*                  GetImageList( ResMgr* pResMgr, bool bBig );
};

SfxModule_Impl::SfxModule_Impl()
 : pSlotPool(nullptr), pTbxCtrlFac(nullptr), pStbCtrlFac(nullptr), pMenuCtrlFac(nullptr), pFactArr(nullptr), pImgListSmall(nullptr), pImgListBig(nullptr)
{
}

SfxModule_Impl::~SfxModule_Impl()
{
    delete pSlotPool;
    delete pTbxCtrlFac;
    delete pStbCtrlFac;
    delete pMenuCtrlFac;
    delete pFactArr;
    delete pImgListSmall;
    delete pImgListBig;
}

ImageList* SfxModule_Impl::GetImageList( ResMgr* pResMgr, bool bBig )
{
    ImageList*& rpList = bBig ? pImgListBig : pImgListSmall;
    if ( !rpList )
    {
        ResId aResId( bBig ? ( RID_DEFAULTIMAGELIST_LC ) : ( RID_DEFAULTIMAGELIST_SC ), *pResMgr );

        aResId.SetRT( RSC_IMAGELIST );

        DBG_ASSERT( pResMgr->IsAvailable(aResId), "No default ImageList!" );

        if ( pResMgr->IsAvailable(aResId) )
            rpList = new ImageList( aResId );
        else
            rpList = new ImageList();
    }

    return rpList; }


SFX_IMPL_SUPERCLASS_INTERFACE(SfxModule, SfxShell)

ResMgr* SfxModule::GetResMgr()
{
    return pResMgr;
}

SfxModule::SfxModule( ResMgr* pMgrP, bool bDummyP,
                      SfxObjectFactory* pFactoryP, ... )
    : pResMgr( pMgrP ), bDummy( bDummyP ), pImpl(nullptr)
{
    Construct_Impl();
    va_list pVarArgs;
    va_start( pVarArgs, pFactoryP );
    for ( SfxObjectFactory *pArg = pFactoryP; pArg;
         pArg = va_arg( pVarArgs, SfxObjectFactory* ) )
        pArg->SetModule_Impl( this );
    va_end(pVarArgs);
}

void SfxModule::Construct_Impl()
{
    if( !bDummy )
    {
        SfxApplication *pApp = SfxGetpApp();
        SfxModuleArr_Impl& rArr = GetModules_Impl();
        SfxModule* pPtr = this;
        rArr.push_back( pPtr );
        pImpl = new SfxModule_Impl;
        pImpl->pSlotPool = new SfxSlotPool(&pApp->GetAppSlotPool_Impl());

        pImpl->pTbxCtrlFac=nullptr;
        pImpl->pStbCtrlFac=nullptr;
        pImpl->pMenuCtrlFac=nullptr;
        pImpl->pFactArr=nullptr;
        pImpl->pImgListSmall=nullptr;
        pImpl->pImgListBig=nullptr;

        SetPool( &pApp->GetPool() );
    }
}



SfxModule::~SfxModule()
{
    if( !bDummy )
    {
        if ( SfxGetpApp()->Get_Impl() )
        {
            // The module will be destroyed before the Deinitialize,
            // so remove from the array
            SfxModuleArr_Impl& rArr = GetModules_Impl();
            for( sal_uInt16 nPos = rArr.size(); nPos--; )
            {
                if( rArr[ nPos ] == this )
                {
                    rArr.erase( rArr.begin() + nPos );
                    break;
                }
            }

            delete pImpl;
        }

        delete pResMgr;
    }
}



SfxSlotPool* SfxModule::GetSlotPool() const
{
    return pImpl->pSlotPool;
}



void SfxModule::RegisterChildWindow(SfxChildWinFactory *pFact)
{
    DBG_ASSERT( pImpl, "No real Module!" );

    if (!pImpl->pFactArr)
        pImpl->pFactArr = new SfxChildWinFactArr_Impl;

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



void SfxModule::RegisterToolBoxControl( SfxTbxCtrlFactory *pFact )
{
    if (!pImpl->pTbxCtrlFac)
        pImpl->pTbxCtrlFac = new SfxTbxCtrlFactArr_Impl;

#ifdef DBG_UTIL
    for ( size_t n=0; n<pImpl->pTbxCtrlFac->size(); n++ )
    {
        SfxTbxCtrlFactory *pF = &(*pImpl->pTbxCtrlFac)[n];
        if ( pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            SAL_INFO("sfx2.appl", "TbxController-Registering is not clearly defined!");
        }
    }
#endif

    pImpl->pTbxCtrlFac->push_back( pFact );
}



void SfxModule::RegisterStatusBarControl( SfxStbCtrlFactory *pFact )
{
    if (!pImpl->pStbCtrlFac)
        pImpl->pStbCtrlFac = new SfxStbCtrlFactArr_Impl;

#ifdef DBG_UTIL
    for ( size_t n=0; n<pImpl->pStbCtrlFac->size(); n++ )
    {
        SfxStbCtrlFactory *pF = &(*pImpl->pStbCtrlFac)[n];
        if ( pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            SAL_INFO("sfx2.appl", "TbxController-Registering is not clearly defined!");
        }
    }
#endif

    pImpl->pStbCtrlFac->push_back( pFact );
}



void SfxModule::RegisterMenuControl( const SfxMenuCtrlFactory& rFact )
{
    if (!pImpl->pMenuCtrlFac)
        pImpl->pMenuCtrlFac = new SfxMenuCtrlFactArr_Impl;

#ifdef DBG_UTIL
    for ( size_t n=0; n<pImpl->pMenuCtrlFac->size(); n++ )
    {
        SfxMenuCtrlFactory *pF = &(*pImpl->pMenuCtrlFac)[n];
        if ( pF->nTypeId == rFact.nTypeId &&
            (pF->nSlotId == rFact.nSlotId || pF->nSlotId == 0) )
        {
            SAL_INFO("sfx2.appl", "MenuController-Registering is not clearly defined!");
        }
    }
#endif

    pImpl->pMenuCtrlFac->push_back( rFact );
}



SfxTbxCtrlFactArr_Impl*  SfxModule::GetTbxCtrlFactories_Impl() const
{
    return pImpl->pTbxCtrlFac;
}



SfxStbCtrlFactArr_Impl*  SfxModule::GetStbCtrlFactories_Impl() const
{
    return pImpl->pStbCtrlFac;
}



SfxMenuCtrlFactArr_Impl* SfxModule::GetMenuCtrlFactories_Impl() const
{
    return pImpl->pMenuCtrlFac;
}



SfxChildWinFactArr_Impl* SfxModule::GetChildWinFactories_Impl() const
{
    return pImpl->pFactArr;
}

ImageList* SfxModule::GetImageList_Impl( bool bBig )
{
    return pImpl->GetImageList( pResMgr, bBig );
}

VclPtr<SfxTabPage> SfxModule::CreateTabPage( sal_uInt16, vcl::Window*, const SfxItemSet& )
{
    return VclPtr<SfxTabPage>();
}

SfxModuleArr_Impl& SfxModule::GetModules_Impl()
{
    if( !pModules )
        pModules = new SfxModuleArr_Impl;
    return *pModules;
};

void SfxModule::DestroyModules_Impl()
{
    if ( pModules )
    {
        SfxModuleArr_Impl& rModules = *pModules;
        for( sal_uInt16 nPos = rModules.size(); nPos--; )
        {
            SfxModule* pMod = rModules[nPos];
            delete pMod;
        }
        delete pModules, pModules = nullptr;
    }
}

void SfxModule::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        if ( pFrame->GetObjectShell()->GetModule() == this )
            Invalidate_Impl( pFrame->GetBindings(), nId );
}

bool SfxModule::IsChildWindowAvailable( const sal_uInt16 i_nId, const SfxViewFrame* i_pViewFrame ) const
{
    if ( i_nId != SID_TASKPANE )
        // by default, assume it is
        return true;

    const SfxViewFrame* pViewFrame = i_pViewFrame ? i_pViewFrame : GetFrame();
    ENSURE_OR_RETURN( pViewFrame, "SfxModule::IsChildWindowAvailable: no frame to ask for the module identifier!", false );
    return ::sfx2::ModuleTaskPane::ModuleHasToolPanels( pViewFrame->GetFrame().GetFrameInterface() );
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
