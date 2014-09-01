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
#ifndef INCLUDED_SFX2_SHELL_HXX
#define INCLUDED_SFX2_SHELL_HXX

#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <sfx2/dllapi.h>
#include <sfx2/sfxuno.hxx>
#include <svl/brdcst.hxx>
#include <tools/debug.hxx>
#include <tools/rtti.hxx>

class ResMgr;
class Window;
class ToolBox;
class SfxItemPool;
class SfxPoolItem;
class SfxRequest;
class SfxItemSet;
struct SfxFormalArgument;
class StatusBar;
class SfxInterface;
class SfxViewShell;
class SfxObjectShell;
class SfxSlotPool;
class SvGlobalName;

class SfxShellObject;
class SfxShell;
struct SfxShell_Impl;
class SfxShellObject;
class SfxDispatcher;
class SfxViewFrame;
class SfxSlot;
class SfxRepeatTarget;
class SbxVariable;
class SbxBase;
class SfxBindings;

namespace svl
{
    class IUndoManager;
}



enum SfxInterfaceId

/*  [Description]

    Id for <SfxInterface>s, gives a quasi-static access to the interface
    through an array to <SfxApplication>.
*/

{
    SFX_INTERFACE_NONE,
    SFX_INTERFACE_SFXAPP,
    SFX_INTERFACE_SFXDOCSH,
    SFX_INTERFACE_SFXIPFRM,
    SFX_INTERFACE_SFXVIEWSH,
    SFX_INTERFACE_SFXVIEWFRM,
    SFX_INTERFACE_SFXPLUGINFRM,
    SFX_INTERFACE_SFXPLUGINOBJ,
    SFX_INTERFACE_SFXPLUGINVIEWSH,
    SFX_INTERFACE_SFXFRAMESETOBJ,
    SFX_INTERFACE_SFXFRAMESETVIEWSH,
    SFX_INTERFACE_SFXINTERNALFRM,
    SFX_INTERFACE_SFXCOMPONENTDOCSH,
    SFX_INTERFACE_SFXGENERICOBJ,
    SFX_INTERFACE_SFXGENERICVIEWSH,
    SFX_INTERFACE_SFXEXPLOBJ,
    SFX_INTERFACE_SFXEXPLVIEWSH,
    SFX_INTERFACE_SFXPLUGINVIEWSHDYNAMIC,
    SFX_INTERFACE_SFXEXTERNALVIEWFRM,
    SFX_INTERFACE_SFXMODULE,
    SFX_INTERFACE_SFXFRAMESETVIEW,
    SFX_INTERFACE_SFXFRAMESETSOURCEVIEW,
    SFX_INTERFACE_SFXHELP_DOCSH,
    SFX_INTERFACE_SFXHELP_VIEWSH,
    SFX_INTERFACE_SFXTASK,
    SFX_INTERFACE_OFA_START         =  100,
    SFX_INTERFACE_OFA_END           =  100,
    SFX_INTERFACE_SC_START          =  150,
    SFX_INTERFACE_SC_END            =  199,
    SFX_INTERFACE_SD_START          =  200,
    SFX_INTERFACE_SD_END            =  249,
    SFX_INTERFACE_SW_START          =  250,
    SFX_INTERFACE_SW_END            =  299,
    SFX_INTERFACE_SIM_START         =  300,
    SFX_INTERFACE_SIM_END           =  319,
    SFX_INTERFACE_SCH_START         =  320,
    SFX_INTERFACE_SCH_END           =  339,
    SFX_INTERFACE_SMA_START         =  340,
    SFX_INTERFACE_SMA_END           =  359,
    SFX_INTERFACE_SBA_START         =  360,
    SFX_INTERFACE_SBA_END           =  399,
    SFX_INTERFACE_IDE_START         =  400,
    SFX_INTERFACE_IDE_END           =  409,
    //-if one is still needed
    SFX_INTERFACE_APP               =  SFX_INTERFACE_SW_START,
    SFX_INTERFACE_LIB               =  450
};

//TODO/CLEANUP: replace by UNO constant
#define SVVERB_SHOW -1



typedef void (*SfxExecFunc)(SfxShell *, SfxRequest &rReq);
typedef void (*SfxStateFunc)(SfxShell *, SfxItemSet &rSet);

class SFX2_DLLPUBLIC SfxShell: public SfxBroadcaster

/*  [Description]

    The class SfxShell is the base class for all classes, which provide
    the functionality of the form <Slot>s.

    Each instance has a reference to an interface description, which is
    obtainable through <SfxShell::GetInterface()const>. This interface
    provides the connection to specific methods and contains some other
    descriptive data for controllers like menus and toolboxes, but also
    for the various APIs. The main part of the interface description is in
    the form of a <Type-Library>, which is generated from an IDL-file by
    the <SVIDL-Compiler>. For each SfxShell Subclass-File there is one
    such IDL-file to write.
*/

{
    friend class SfxObjectItem;

    SfxShell_Impl*              pImp;
    SfxItemPool*                pPool;
    ::svl::IUndoManager*        pUndoMgr;

private:
                                SfxShell( const SfxShell & ); // internal
    SfxShell&                                   operator = ( const SfxShell & ); // internal

protected:
                                SfxShell();
                                SfxShell( SfxViewShell *pViewSh );

    void SetViewShell_Impl( SfxViewShell* pView );
    SAL_DLLPRIVATE void Invalidate_Impl( SfxBindings& rBindings, sal_uInt16 nId );
    SAL_DLLPRIVATE SfxShellObject* GetShellObj_Impl() const;
    SAL_DLLPRIVATE void SetShellObj_Impl( SfxShellObject* pObj );

public:
                                TYPEINFO_OVERRIDE();
    virtual                     ~SfxShell();

    virtual SfxInterface*       GetInterface() const;
    static SfxInterface*        GetStaticInterface() { return 0; }

    void                        SetName( const OUString &rName );
    const OUString&             GetName() const;

    SfxViewShell*               GetViewShell() const;

    void                        CallExec( SfxExecFunc pFunc, SfxRequest &rReq )
                                { (*pFunc)(this, rReq); }
    void                        CallState( SfxStateFunc pFunc, SfxItemSet &rSet )
                                { (*pFunc)(this, rSet); }

    static void                 EmptyExecStub(SfxShell *pShell, SfxRequest &);
    static void                 EmptyStateStub(SfxShell *pShell, SfxItemSet &);

    const SfxPoolItem*          GetSlotState( sal_uInt16 nSlotId, const SfxInterface *pIF = 0, SfxItemSet *pStateSet = 0 );
    const SfxPoolItem*          ExecuteSlot( SfxRequest &rReq, const SfxInterface *pIF = 0 );
    const SfxPoolItem*          ExecuteSlot( SfxRequest &rReq, bool bAsync );
    sal_uIntPtr                       ExecuteSlot( sal_uInt16 nSlot, sal_uInt16 nMemberId, SbxVariable& rRet, SbxBase* pArgs = 0 );

    inline SfxItemPool&         GetPool() const;
    inline void                 SetPool( SfxItemPool *pNewPool ) ;

    virtual ::svl::IUndoManager*
                                GetUndoManager();
    void                        SetUndoManager( ::svl::IUndoManager *pNewUndoMgr );

    SfxRepeatTarget*            GetRepeatTarget() const;
    void                        SetRepeatTarget( SfxRepeatTarget *pTarget );

    virtual void                Invalidate(sal_uInt16 nId = 0);

    virtual bool                IsDesignMode() const { return false; };

    bool                        IsActive() const;
    virtual void                Activate(bool bMDI);
    virtual void                Deactivate(bool bMDI);
    virtual void                ParentActivate();
    virtual void                ParentDeactivate();

    SfxDispatcher*              GetDispatcher() const;
    SfxViewFrame*               GetFrame() const;
    ResMgr*                     GetResMgr() const;
    virtual bool                HasUIFeature( sal_uInt32 nFeature );
    void                        UIFeatureChanged();

    // Items
    const SfxPoolItem*          GetItem( sal_uInt16 nSlotId ) const;
    void                        PutItem( const SfxPoolItem& rItem );

    // TODO/CLEANUP: still needed?!
    void SetVerbs(const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& aVerbs);
    const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& GetVerbs() const;
    void                        VerbExec (SfxRequest&);
    void                        VerbState (SfxItemSet&);
    SAL_DLLPRIVATE const SfxSlot* GetVerbSlot_Impl(sal_uInt16 nId) const;

    void                        SetHelpId(sal_uIntPtr nId);
    sal_uIntPtr                     GetHelpId() const;
    virtual SfxObjectShell*     GetObjectShell();
    void                        SetDisableFlags( sal_uIntPtr nFlags );
    sal_uIntPtr                     GetDisableFlags() const;

    virtual SfxItemSet*         CreateItemSet( sal_uInt16 nId );
    virtual void                ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );

    /** Set the name of the sidebar context that is broadcast on calls
        to Activation().
    */
    void SetContextName (const ::rtl::OUString& rsContextName);

    /** Broadcast a sidebar context change.
        This method is typically called from Activate() or
        Deactivate().
        @param bIsActivated
            When <TRUE/> then broadcast the context name that was
            defined with an earlier call to SetContextName().
            When <FALSE/> then broadcast the 'default' context.
    */
    void BroadcastContextForActivation (const bool bIsActivated);

    /** Enabled or disable the context broadcaster.  Returns the old state.
    */
    bool SetContextBroadcasterEnabled (const bool bIsEnabled);

    SAL_DLLPRIVATE bool CanExecuteSlot_Impl( const SfxSlot &rSlot );
    SAL_DLLPRIVATE void DoActivate_Impl( SfxViewFrame *pFrame, bool bMDI);
    SAL_DLLPRIVATE void DoDeactivate_Impl( SfxViewFrame *pFrame, bool bMDI);
};


SfxItemPool& SfxShell::GetPool() const
/*
    [Description]

    Each Subclass of SfxShell must reference a pool. This is partly set by
    SFx's own set of subclasses (eg <SfxViewShell>). In particular however
    this must be set directly from one derived SfxShell class and ny
    derivatives of SfxObjectShell.

    The SfxShell class itself does not have any SfxItemPool, therfore a
    null-pointer is returned.
*/

{
    DBG_ASSERT( pPool, "no pool" );
    return *pPool;
}

inline void SfxShell::SetPool
(
    SfxItemPool*        pNewPool        // Pointer to the new Pool or null
)

/*  [Description]

    With this method, the subclasses register their special <SfxItemPool>
    in the SfxShell. Each SfxShell instance must have access to a SfxItemPool.
    Usually this is the SfxItemPool of the SfxDocumentShell. The SfxShell
    subclass does not take ownership of the orphaned pool. Before it is
    deleted it has to be deregisted with SetPool(0).
*/

{
    pPool = pNewPool;
}

#define SFX_SLOTMAP(ShellClass) static SfxFormalArgument a##ShellClass##Args_Impl[1]; \
                                static SfxSlot a##ShellClass##Slots_Impl[] =

#define SFX_DECL_INTERFACE(nId)                                             \
            static SfxInterface*                pInterface;                 \
            static const SfxFormalArgument*     pSfxFormalArgs_Impl;        \
            static SfxInterface*                GetStaticInterface();       \
            static SfxInterfaceId               GetInterfaceId() {return SfxInterfaceId(nId);} \
            static void                         RegisterInterface(SfxModule* pMod=NULL); \
            virtual SfxInterface*       GetInterface() const SAL_OVERRIDE;

#define SFX_IMPL_INTERFACE(Class,SuperClass,NameResId)                      \
                                                                            \
    SfxInterface* Class::pInterface = 0;                                    \
    const SfxFormalArgument* Class::pSfxFormalArgs_Impl = a##Class##Args_Impl;\
    SfxInterface* Class::GetStaticInterface()                      \
    {                                                                       \
        if ( !pInterface )                                                  \
        {                                                                   \
            pInterface =                                                    \
                new SfxInterface(                                           \
            #Class, NameResId, GetInterfaceId(),                            \
            SuperClass::GetStaticInterface(),                               \
            a##Class##Slots_Impl[0],                                        \
            (sal_uInt16) (sizeof(a##Class##Slots_Impl) / sizeof(SfxSlot) ) );   \
            InitInterface_Impl();                                           \
        }                                                                   \
        return pInterface;                                                  \
    }                                                                       \
                                                                            \
    SfxInterface* Class::GetInterface() const                               \
    {                                                                       \
        return GetStaticInterface();                                        \
    }                                                                       \
                                                                            \
    void Class::RegisterInterface(SfxModule* pMod)                          \
    {                                                                       \
        GetStaticInterface()->Register(pMod);                               \
    }

#define SFX_POSITION_MASK               0x000F
#define SFX_VISIBILITY_MASK             0xFFF0
#define SFX_VISIBILITY_UNVISIBLE        0x0000  // Never visible
#define SFX_VISIBILITY_VIEWER           0x0040
#define SFX_VISIBILITY_READONLYDOC      0x0400
#define SFX_VISIBILITY_DESKTOP          0x0800
#define SFX_VISIBILITY_STANDARD         0x1000
#define SFX_VISIBILITY_FULLSCREEN       0x2000
#define SFX_VISIBILITY_CLIENT           0x4000
#define SFX_VISIBILITY_SERVER           0x8000
#define SFX_VISIBILITY_NOCONTEXT        0xFFFF  // Always visable

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
