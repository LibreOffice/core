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
#ifndef INCLUDED_SFX2_BINDINGS_HXX
#define INCLUDED_SFX2_BINDINGS_HXX

#include <sal/config.h>

#include <cstddef>
#include <memory>

#include <o3tl/typed_flags_set.hxx>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <tools/link.hxx>
#include <com/sun/star/uno/Reference.h>
#include <svl/poolitem.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <sfx2/shell.hxx>

namespace com::sun::star::frame { class XDispatch; }
namespace com::sun::star::frame { class XDispatchProvider; }
namespace com::sun::star::frame { class XDispatchRecorder; }
namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::util { struct URL; }

class SfxSlot;
class SfxSlotServer;
class SfxControllerItem;
class SfxStateCache;
class SfxItemSet;
class SfxDispatcher;
class SfxBindings_Impl;
class Timer;
class SfxWorkWindow;
struct SfxFoundCache_Impl;
class SfxFoundCacheArr_Impl;

enum class SfxCallMode : sal_uInt16
{
    SLOT      = 0x00,    // sync/async from Slot
    SYNCHRON  = 0x01,    // synchronously in the same Stackframe
    ASYNCHRON = 0x02,    // asynchronously via AppEvent
    RECORD    = 0x04,    // take into account while recording
    API       = 0x08     // API call (silent)
};

namespace o3tl {

template<> struct typed_flags<SfxCallMode>: is_typed_flags<SfxCallMode, 0x0F>
{};

}

class SFX2_DLLPUBLIC SfxBindings: public SfxBroadcaster

/*  [Description]

    In each SFx application one instance of the SfxBindings-Class will
    exists from <SfxApplication::Init()> until <SfxApplication::Exit()>.
    This instance is automatically created and destroyed by SfxApplication.
    However these instances will be handled by the Macro <SFX_BINDINGS>
    or the associated <SfxViewFrame>.

    The SfxBindings manages all of its Slot-Ids bound by the registered
    controllers and keeps a cache of the <Slot-Server> respectively.
    (it is what we call the combination of SfxShell instance and SfxSlot).
    In the SfxBindings it is stored, if and in this case which controllers
    that are dirty and which Slot-Server-Caches are dirty respectively.
    It summarizes status queries (calls to the status methods specified
    in the IDL) that are served by the same state methods, and handles
    the simulation of <Pseudo-Slots>.
*/

{
friend class SfxApplication;
friend class SfxShell;
friend class SfxBindings_Impl;

    std::unique_ptr< SfxBindings_Impl> pImpl;           // Data of the Bindings instance
    SfxDispatcher*   pDispatcher;    // Dispatcher, to be used
    sal_uInt16       nRegLevel;      // Lock-Level while Reconfig

private:
    SAL_DLLPRIVATE const SfxPoolItem*  Execute_Impl( sal_uInt16 nSlot, const SfxPoolItem **pArgs, sal_uInt16 nModi,
                                    SfxCallMode nCall, const SfxPoolItem **pInternalArgs, bool bGlobalOnly=false);
    SAL_DLLPRIVATE void SetSubBindings_Impl( SfxBindings* );
    SAL_DLLPRIVATE void UpdateSlotServer_Impl(); // Update SlotServer
    SAL_DLLPRIVATE std::unique_ptr<SfxItemSet> CreateSet_Impl(SfxStateCache& rCache, const SfxSlot* &pRealSlot,
                                              const SfxSlotServer**, SfxFoundCacheArr_Impl&);
    SAL_DLLPRIVATE std::size_t GetSlotPos( sal_uInt16 nId, std::size_t nStartSearchAt = 0 );
    SAL_DLLPRIVATE void Update_Impl(SfxStateCache& rCache);
    static SAL_DLLPRIVATE void UpdateControllers_Impl(
                            const SfxFoundCache_Impl& rFound,
                            const SfxPoolItem *pItem,
                            SfxItemState eItemState );
    SAL_DLLPRIVATE SfxStateCache* GetStateCache( sal_uInt16 nId, std::size_t * pPos);
    DECL_DLLPRIVATE_LINK( NextJob, Timer *, void );
    SAL_DLLPRIVATE bool NextJob_Impl(Timer const * pTimer);

public:
                     SfxBindings();
                     virtual ~SfxBindings() override;

    void             HidePopups( bool bHide = true );

    void             SetDispatcher(SfxDispatcher *pDisp);

    void             Update( sal_uInt16 nId ); // For example, from  Menu::Activate
    void             Update();
    SAL_DLLPRIVATE void StartUpdate_Impl(bool bComplete=false);
    void             Invalidate( sal_uInt16 nId );
    void             Invalidate( const sal_uInt16* pIds );
    void             InvalidateShell( const SfxShell &rSh, bool bDeep = false );
    void             InvalidateAll( bool bWithMsg );
    void             SetState( const SfxItemSet &rSet );
    void             SetState( const SfxPoolItem &rItem );
    void             Invalidate( sal_uInt16 nId, bool bWithItem, bool bWithMsg=false);
    bool             IsInUpdate() const;
    void             SetVisibleState( sal_uInt16 nId, bool bShow );

    SfxStateCache*   GetStateCache( sal_uInt16 nId);
    SAL_DLLPRIVATE SfxStateCache* GetAnyStateCache_Impl( sal_uInt16 nId );
    /**
     * @param rpState the caller has to delete the pointer
     */
    SfxItemState     QueryState( sal_uInt16 nSID, std::unique_ptr<SfxPoolItem> &rpState );

    void             QueryControlState ( sal_uInt16 nSID, boost::property_tree::ptree& rState );

    const SfxPoolItem*  ExecuteSynchron( sal_uInt16 nSlot,
                                 const SfxPoolItem **pArgs = nullptr);
    bool             Execute( sal_uInt16 nSlot,
                                 const SfxPoolItem **pArgs = nullptr,
                                 SfxCallMode nCall = SfxCallMode::SLOT);

    SAL_DLLPRIVATE void SetDispatchProvider_Impl( const css::uno::Reference< css::frame::XDispatchProvider > & rFrame );
    void             SetActiveFrame( const css::uno::Reference< css::frame::XFrame > & rFrame );
    css::uno::Reference< css::frame::XFrame > GetActiveFrame() const;
                     // Reconfig
    sal_uInt16       EnterRegistrations(const char *pFile = nullptr, int nLine = 0);
    void             LeaveRegistrations( const char *pFile = nullptr, int nLine = 0 );
    void             Register( SfxControllerItem& rBinding );
    void             Release( SfxControllerItem& rBinding );
    SfxDispatcher*   GetDispatcher() const
                     { return pDispatcher; }
    const css::uno::Reference< css::frame::XDispatchRecorder >& GetRecorder() const;
    css::uno::Reference < css::frame::XDispatch >
                    GetDispatch( const SfxSlot*, const css::util::URL& aURL, bool bMasterCommand );
    SAL_DLLPRIVATE void ContextChanged_Impl();
    SAL_DLLPRIVATE void Execute_Impl( SfxRequest& rReq, const SfxSlot* pSlot, SfxShell* pShell );
    SAL_DLLPRIVATE void DeleteControllers_Impl();
    SAL_DLLPRIVATE SfxDispatcher* GetDispatcher_Impl()  { return pDispatcher; }
    SAL_DLLPRIVATE void ClearCache_Impl( sal_uInt16 nSlotId );
    SAL_DLLPRIVATE void RegisterInternal_Impl( SfxControllerItem& rBinding );
    SAL_DLLPRIVATE void Register_Impl( SfxControllerItem& rBinding, bool );
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl() const;
    SAL_DLLPRIVATE void SetWorkWindow_Impl( std::unique_ptr<SfxWorkWindow> );
    SAL_DLLPRIVATE SfxBindings* GetSubBindings_Impl() const;
    SAL_DLLPRIVATE void SetRecorder_Impl( css::uno::Reference< css::frame::XDispatchRecorder > const & );
    SAL_DLLPRIVATE void InvalidateSlotsInMap_Impl();
    SAL_DLLPRIVATE void AddSlotToInvalidateSlotsMap_Impl( sal_uInt16 nId );
};

#ifdef DBG_UTIL
#define ENTERREGISTRATIONS() EnterRegistrations(__FILE__, __LINE__)
#define LEAVEREGISTRATIONS() LeaveRegistrations(__FILE__, __LINE__)
#define DENTERREGISTRATIONS() \
        EnterRegistrations( OStringBuffer(__FILE__).append('(').append(reinterpret_cast<sal_Int64>(this)).append(')').getStr(), __LINE__ )
#define DLEAVEREGISTRATIONS(  ) \
        LeaveRegistrations( OStringBuffer(__FILE__).append('(').append(reinterpret_cast<sal_Int64>(this)).append(')').getStr(), __LINE__ )
#else
#define ENTERREGISTRATIONS() EnterRegistrations()
#define LEAVEREGISTRATIONS() LeaveRegistrations()
#define DENTERREGISTRATIONS() EnterRegistrations()
#define DLEAVEREGISTRATIONS() LeaveRegistrations()
#endif


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
