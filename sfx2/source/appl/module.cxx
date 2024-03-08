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

#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/stbitem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/intitem.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/resmgr.hxx>
#include <sal/log.hxx>
#include <comphelper/lok.hxx>
#include <unotools/localedatawrapper.hxx>

#define ShellClass_SfxModule
#include <sfxslots.hxx>
#include <optional>

class SfxModule_Impl
{
public:

    std::optional<SfxSlotPool>              pSlotPool;
    std::vector<SfxTbxCtrlFactory>          maTbxCtrlFactories;
    std::vector<SfxStbCtrlFactory>          maStbCtrlFactories;
    std::vector<SfxChildWinFactory>         maFactories;
    OString                     maResName;

                                SfxModule_Impl();
                                ~SfxModule_Impl();
};

SfxModule_Impl::SfxModule_Impl()
{
}

SfxModule_Impl::~SfxModule_Impl()
{
    pSlotPool.reset();
    maTbxCtrlFactories.clear();
    maStbCtrlFactories.clear();
}

SFX_IMPL_SUPERCLASS_INTERFACE(SfxModule, SfxShell)

SfxModule::SfxModule(const OString& rResName, std::initializer_list<SfxObjectFactory*> pFactoryList)
    : pImpl(nullptr)
{
    Construct_Impl(rResName);
    for (auto pFactory : pFactoryList)
    {
        if (pFactory)
            pFactory->SetModule_Impl( this );
    }
}

void SfxModule::Construct_Impl(const OString& rResName)
{
    SfxApplication *pApp = SfxApplication::GetOrCreate();
    pImpl = new SfxModule_Impl;
    pImpl->pSlotPool.emplace(&pApp->GetAppSlotPool_Impl());
    pImpl->maResName = rResName;

    SetPool( &pApp->GetPool() );
}

SfxModule::~SfxModule()
{
    //TODO how to silence useuniqueptr
    if (true)
    {
        delete pImpl;
    }
}

std::locale SfxModule::GetResLocale() const
{
    return Translate::Create(pImpl->maResName);
}

SfxSlotPool* SfxModule::GetSlotPool() const
{
    return &*pImpl->pSlotPool;
}


void SfxModule::RegisterChildWindow(const SfxChildWinFactory& rFact)
{
    DBG_ASSERT( pImpl, "No real Module!" );

    for (size_t nFactory=0; nFactory<pImpl->maFactories.size(); ++nFactory)
    {
        if (rFact.nId == pImpl->maFactories[nFactory].nId)
        {
            pImpl->maFactories.erase( pImpl->maFactories.begin() + nFactory );
            SAL_WARN("sfx.appl", "ChildWindow registered multiple times!");
            return;
        }
    }

    pImpl->maFactories.push_back( rFact );
}


void SfxModule::RegisterToolBoxControl( const SfxTbxCtrlFactory& rFact )
{
#ifdef DBG_UTIL
    for ( size_t n=0; n<pImpl->maTbxCtrlFactories.size(); n++ )
    {
        SfxTbxCtrlFactory *pF = &pImpl->maTbxCtrlFactories[n];
        if ( pF->nTypeId == rFact.nTypeId &&
            (pF->nSlotId == rFact.nSlotId || pF->nSlotId == 0) )
        {
            SAL_INFO("sfx.appl", "TbxController-Registering is not clearly defined!");
        }
    }
#endif

    pImpl->maTbxCtrlFactories.push_back( rFact );
}


void SfxModule::RegisterStatusBarControl( const SfxStbCtrlFactory& rFact )
{
#ifdef DBG_UTIL
    for ( size_t n=0; n<pImpl->maStbCtrlFactories.size(); n++ )
    {
        SfxStbCtrlFactory *pF = &pImpl->maStbCtrlFactories[n];
        if ( pF->nTypeId == rFact.nTypeId &&
            (pF->nSlotId == rFact.nSlotId || pF->nSlotId == 0) )
        {
            SAL_INFO("sfx.appl", "TbxController-Registering is not clearly defined!");
        }
    }
#endif

    pImpl->maStbCtrlFactories.push_back( rFact );
}


SfxTbxCtrlFactory* SfxModule::GetTbxCtrlFactory(const std::type_info& rSlotType, sal_uInt16 nSlotID) const
{
    // search for a factory with the given slot id
    for (auto& rFactory : pImpl->maTbxCtrlFactories)
        if( rFactory.nTypeId == rSlotType && rFactory.nSlotId == nSlotID )
            return &rFactory;

    // if no factory exists for the given slot id, see if we
    // have a generic factory with the correct slot type and slot id == 0
    for (auto& rFactory : pImpl->maTbxCtrlFactories)
        if( rFactory.nTypeId == rSlotType && rFactory.nSlotId == 0 )
            return &rFactory;

    return nullptr;
}


SfxStbCtrlFactory* SfxModule::GetStbCtrlFactory(const std::type_info& rSlotType, sal_uInt16 nSlotID) const
{
    for (auto& rFactory : pImpl->maStbCtrlFactories)
        if ( rFactory.nTypeId == rSlotType &&
             ( rFactory.nSlotId == 0 || rFactory.nSlotId == nSlotID ) )
            return &rFactory;
    return nullptr;
}

SfxChildWinFactory* SfxModule::GetChildWinFactoryById(sal_uInt16 nId) const
{
    for (auto& rFactory : pImpl->maFactories)
        if (rFactory.nId == nId)
            return &rFactory;
    return nullptr;
}

std::unique_ptr<SfxTabPage> SfxModule::CreateTabPage(sal_uInt16, weld::Container*, weld::DialogController*, const SfxItemSet&)
{
    return nullptr;
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
    ENSURE_OR_RETURN( i_frame.is(), "SfxModule::GetModuleFieldUnit: invalid frame!", FieldUnit::MM_100TH );

    // find SfxViewFrame for the given XFrame
    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    while ( pViewFrame != nullptr )
    {
        if ( pViewFrame->GetFrame().GetFrameInterface() == i_frame )
            break;
        pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
    }
    ENSURE_OR_RETURN(
        pViewFrame != nullptr,
        "SfxModule::GetModuleFieldUnit: unable to find an SfxViewFrame for the given XFrame",
        FieldUnit::MM_100TH);

    // find the module
    SfxModule const * pModule = GetActiveModule( pViewFrame );
    ENSURE_OR_RETURN(pModule != nullptr,
                     "SfxModule::GetModuleFieldUnit: no SfxModule for the given frame!",
                     FieldUnit::MM_100TH);
    return pModule->GetFieldUnit();
}

FieldUnit SfxModule::GetCurrentFieldUnit()
{
    FieldUnit eUnit = FieldUnit::INCH;
    SfxModule* pModule = GetActiveModule();
    if ( pModule )
        return pModule->GetFieldUnit();
    else
        SAL_WARN( "sfx.appl", "GetModuleFieldUnit(): no module found" );
    return eUnit;
}

FieldUnit SfxModule::GetFieldUnit() const
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        MeasurementSystem eSystem
            = LocaleDataWrapper(comphelper::LibreOfficeKit::getLocale()).getMeasurementSystemEnum();
        return MeasurementSystem::Metric == eSystem ? FieldUnit::CM : FieldUnit::INCH;
    }
    FieldUnit eUnit = FieldUnit::INCH;
    const SfxPoolItem* pItem = GetItem( SID_ATTR_METRIC );
    if ( pItem )
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
    return eUnit;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
