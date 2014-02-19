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

#include "sal/config.h"

#include <algorithm>

#include <officecfg/Office/Common.hxx>
#include <svtools/langtab.hxx>
#include <svl/zforlist.hxx>
#include <svtools/grfmgr.hxx>
#include <svl/flagitem.hxx>
#include <sfx2/dispatch.hxx>
#include <unotools/lingucfg.hxx>
#include <svl/szitem.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/msgbox.hxx>
#include <rtl/math.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/fontoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/configitem.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/types.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/langitem.hxx>
#include "cuioptgenrl.hxx"
#include "optpath.hxx"
#include "optsave.hxx"
#include "optlingu.hxx"
#include <svx/xpool.hxx>
#include <svx/dlgutil.hxx>
#include "cuitabarea.hxx"
#include "optmemory.hxx"
#include <svx/ofaitem.hxx>
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>

#include <config_vclplug.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::utl;
using namespace ::rtl;
using namespace ::sfx2;


#define NF2BYTES        104857.6                        // 2^20/10, used for M_pNfGraphicObjectCache-unit -> Byte
#define BYTES2NF        (1.0/NF2BYTES)                  // 10/2^20


sal_Int32 OfaMemoryOptionsPage::GetNfGraphicCacheVal() const
{
    return m_pNfGraphicCache->GetValue() << 20;
}

inline void OfaMemoryOptionsPage::SetNfGraphicCacheVal( long nSizeInBytes )
{
    m_pNfGraphicCache->SetValue( nSizeInBytes >> 20 );
}

long OfaMemoryOptionsPage::GetNfGraphicObjectCacheVal( void ) const
{
    return long( ::rtl::math::round( double( m_pNfGraphicObjectCache->GetValue() ) * NF2BYTES ) );
}

void OfaMemoryOptionsPage::SetNfGraphicObjectCacheVal( long nSizeInBytes )
{
    m_pNfGraphicObjectCache->SetValue( long( ::rtl::math::round( double( nSizeInBytes ) * BYTES2NF ) ) );
}

inline void OfaMemoryOptionsPage::SetNfGraphicObjectCacheMax( long nSizeInBytes )
{
    m_pNfGraphicObjectCache->SetMax( long( double( nSizeInBytes ) * BYTES2NF ) );
}

inline void OfaMemoryOptionsPage::SetNfGraphicObjectCacheLast( long nSizeInBytes )
{
    m_pNfGraphicObjectCache->SetLast( long( double( nSizeInBytes ) * BYTES2NF ) );
}

int OfaMemoryOptionsPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

OfaMemoryOptionsPage::OfaMemoryOptionsPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptMemoryPage", "cui/ui/optmemorypage.ui", rSet)
{
    get(m_pUndoEdit, "undo");
    get(m_pNfGraphicCache, "graphiccache");
    get(m_pNfGraphicObjectCache, "objectcache");
    get(m_pTfGraphicObjectTime,"objecttime");
    get(m_pNfOLECache, "olecache");
    get(m_pQuickStarterFrame, "quickstarter");

#if defined(UNX)
    get(m_pQuickLaunchCB, "systray");
#else
    get(m_pQuickLaunchCB, "quicklaunch");
#endif
    m_pQuickLaunchCB->Show();

    //Only available in Win or if building the gtk systray
#if !defined(WNT) && ! ENABLE_GTK
    m_pQuickStarterFrame->Hide();
#endif

    m_pTfGraphicObjectTime->SetExtFormat( EXTTIMEF_24H_SHORT );

    SetExchangeSupport();

    m_pNfGraphicCache->SetModifyHdl( LINK( this, OfaMemoryOptionsPage, GraphicCacheConfigHdl ) );
}

// -----------------------------------------------------------------------

OfaMemoryOptionsPage::~OfaMemoryOptionsPage()
{
}

// -----------------------------------------------------------------------

SfxTabPage* OfaMemoryOptionsPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new OfaMemoryOptionsPage( pParent, rAttrSet );
}

// -----------------------------------------------------------------------

sal_Bool OfaMemoryOptionsPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;

    boost::shared_ptr< comphelper::ConfigurationChanges > batch(
        comphelper::ConfigurationChanges::create());

    if ( m_pUndoEdit->GetText() != m_pUndoEdit->GetSavedValue() )
        officecfg::Office::Common::Undo::Steps::set(
            m_pUndoEdit->GetValue(), batch);

    // GraphicCache
    sal_Int32 totalCacheSize = GetNfGraphicCacheVal();
    officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::set(
        totalCacheSize, batch);
    sal_Int32 objectCacheSize = GetNfGraphicObjectCacheVal();
    officecfg::Office::Common::Cache::GraphicManager::ObjectCacheSize::set(
        objectCacheSize, batch);

    const Time aTime( m_pTfGraphicObjectTime->GetTime() );
    sal_Int32 objectReleaseTime =
        aTime.GetSec() + aTime.GetMin() * 60 + aTime.GetHour() * 3600;
    officecfg::Office::Common::Cache::GraphicManager::ObjectReleaseTime::set(
        objectReleaseTime, batch);

    // create a dummy graphic object to get access to the common GraphicManager
    GraphicObject       aDummyObject;
    GraphicManager&     rGrfMgr = aDummyObject.GetGraphicManager();

    rGrfMgr.SetMaxCacheSize(totalCacheSize);
    rGrfMgr.SetMaxObjCacheSize(objectCacheSize, true);
    rGrfMgr.SetCacheTimeout(objectReleaseTime);

    // OLECache
    officecfg::Office::Common::Cache::Writer::OLE_Objects::set(
        m_pNfOLECache->GetValue(), batch);
    officecfg::Office::Common::Cache::DrawingEngine::OLE_Objects::set(
        m_pNfOLECache->GetValue(), batch);

    batch->commit();

    if( TriState(m_pQuickLaunchCB->IsChecked()) != m_pQuickLaunchCB->GetSavedValue())
    {
        rSet.Put(SfxBoolItem(SID_ATTR_QUICKLAUNCHER, m_pQuickLaunchCB->IsChecked()));
        bModified = sal_True;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void OfaMemoryOptionsPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem*  pItem;

    m_pUndoEdit->SetValue(officecfg::Office::Common::Undo::Steps::get());
    m_pUndoEdit->SaveValue();

    // GraphicCache
    long n =
        officecfg::Office::Common::Cache::GraphicManager::TotalCacheSize::get();
    SetNfGraphicCacheVal( n );
    SetNfGraphicObjectCacheVal(
        std::min(
            GetNfGraphicCacheVal(),
            (officecfg::Office::Common::Cache::GraphicManager::ObjectCacheSize::
             get())));

    sal_Int32 nTime =
        officecfg::Office::Common::Cache::GraphicManager::ObjectReleaseTime::
        get();
    Time aTime( (sal_uInt16)( nTime / 3600 ), (sal_uInt16)( ( nTime % 3600 ) / 60 ), (sal_uInt16)( ( nTime % 3600 ) % 60 ) );
    m_pTfGraphicObjectTime->SetTime( aTime );

    GraphicCacheConfigHdl(m_pNfGraphicCache);

    // OLECache
    m_pNfOLECache->SetValue(
        std::max(
            officecfg::Office::Common::Cache::Writer::OLE_Objects::get(),
            (officecfg::Office::Common::Cache::DrawingEngine::OLE_Objects::
             get())));

    SfxItemState eState = rSet.GetItemState( SID_ATTR_QUICKLAUNCHER, false, &pItem );
    if ( SFX_ITEM_SET == eState )
        m_pQuickLaunchCB->Check( ( (SfxBoolItem*)pItem )->GetValue() );
    else if ( SFX_ITEM_DISABLED == eState )
    {
        // quickstart not installed
        m_pQuickStarterFrame->Hide();
    }

    m_pQuickLaunchCB->SaveValue();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(OfaMemoryOptionsPage, GraphicCacheConfigHdl)
{
    sal_Int32 n = GetNfGraphicCacheVal();
    SetNfGraphicObjectCacheMax( n );
    SetNfGraphicObjectCacheLast( n );

    if( GetNfGraphicObjectCacheVal() > n )
        SetNfGraphicObjectCacheVal( n );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
