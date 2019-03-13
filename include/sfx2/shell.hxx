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

#include <memory>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <svl/typedwhich.hxx>
#include <sfx2/dllapi.h>
#include <sfx2/sfxuno.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <o3tl/strong_int.hxx>

namespace vcl { class Window; }
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
class SfxModule;
class SfxUndoManager;

/**
    Id for <SfxInterface>s, gives a quasi-static access to the interface
    through an array to <SfxApplication>.
*/
typedef o3tl::strong_int<sal_uInt16, struct SfxInterfaceIdTag> SfxInterfaceId;

constexpr auto SFX_INTERFACE_NONE         = SfxInterfaceId(0);
constexpr auto SFX_INTERFACE_SFXAPP       = SfxInterfaceId(1);
constexpr auto SFX_INTERFACE_SFXDOCSH     = SfxInterfaceId(2);
constexpr auto SFX_INTERFACE_SFXVIEWSH    = SfxInterfaceId(3);
constexpr auto SFX_INTERFACE_SFXVIEWFRM   = SfxInterfaceId(4);
constexpr auto SFX_INTERFACE_SFXMODULE    = SfxInterfaceId(5);
constexpr auto SFX_INTERFACE_SC_START     = SfxInterfaceId(150);
constexpr auto SFX_INTERFACE_SD_START     = SfxInterfaceId(200);
constexpr auto SFX_INTERFACE_SW_START     = SfxInterfaceId(250);
constexpr auto SFX_INTERFACE_SMA_START    = SfxInterfaceId(340);
constexpr auto SFX_INTERFACE_IDE_START    = SfxInterfaceId(400);
constexpr auto SFX_INTERFACE_IDE_END      = SfxInterfaceId(409);

enum class SfxShellFeature
{
    NONE                    = 0x0000,
    // Writer only, class SwView
    SwChildWindowLabel      = 0x0001,
    // Basic only, class Shell
    BasicShowBrowser        = 0x0004,
    // Forms only, class FmFormShell
    FormShowDatabaseBar     = 0x0008,
    FormShowField           = 0x0010,
    FormShowProperies       = 0x0020,
    FormShowExplorer        = 0x0040,
    FormShowFilterBar       = 0x0080,
    FormShowFilterNavigator = 0x0100,
    FormShowTextControlBar  = 0x0200,
    FormTBControls          = 0x0400,
    FormTBDesign            = 0x1000,
    FormShowDataNavigator   = 0x2000,
    // masks to make sure modules don't use flags from another
    SwMask                  = 0x0001,
    BasicMask               = 0x0004,
    FormMask                = 0x3ff8
};
namespace o3tl
{
    template<> struct typed_flags<SfxShellFeature> : is_typed_flags<SfxShellFeature, 0x3ffd> {};
}

/* Flags that are being used in the slot definitions for the disable-features */
enum class SfxDisableFlags {
    NONE,
    SwOnProtectedCursor = 0x0001,
    SwOnMailboxEditor   = 0x0002,
};
namespace o3tl {
    template<> struct typed_flags<SfxDisableFlags> : is_typed_flags<SfxDisableFlags, 0x0003> {};
}

typedef void (*SfxExecFunc)(SfxShell *, SfxRequest &rReq);
typedef void (*SfxStateFunc)(SfxShell *, SfxItemSet &rSet);

/**
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
class SFX2_DLLPUBLIC SfxShell: public SfxBroadcaster
{
    friend class SfxObjectItem;

    std::unique_ptr< SfxShell_Impl >              pImpl;
    SfxItemPool*                pPool;
    SfxUndoManager*             pUndoMgr;

private:
                                SfxShell( const SfxShell & ) = delete;
    SfxShell&                                   operator = ( const SfxShell & ) = delete;

protected:
    /**
        The constructor of the SfxShell class initializes only simple types,
        the corresponding SbxObject is only created on-demand. Therefore,
        the application of a SfxShell instance is very cheap.
        */
    SfxShell();

    /**
        The constructor of the SfxShell class initializes only simple types,
        the corresponding SbxObject is only created on-demand. Therefore,
        the application of a SfxShell instance is very cheap.
        */
    SfxShell( SfxViewShell *pViewSh );

    void SetViewShell_Impl( SfxViewShell* pView );
    SAL_DLLPRIVATE void Invalidate_Impl( SfxBindings& rBindings, sal_uInt16 nId );
    static void HandleOpenXmlFilterSettings(SfxRequest &);

public:

    /**
        The connection to a possible corresponding SbxObject is dissolved.
        The SbxObject may continue to exist, but can not any longer perform
        any functions and can not provide any properties.
        */
    virtual                     ~SfxShell() override;

    /**
        With this virtual method, which is automatically overridden by each subclass
        with its own slots through the macro <SFX_DECL_INTERFACE>, one can access
        each of the <SfxInterface> instance belonging to the subclass.

        The class SfxShell itself has no own SfxInterface (no slots), therefore a
        NULL-pointer is returned.
        */
    virtual SfxInterface*       GetInterface() const;
    static SfxInterface*        GetStaticInterface() { return nullptr; }

    /**
        Sets the name of the Shell object. With this name, the SfxShell instance
        of BASIC can be expressed.
        */
    void                        SetName( const OUString &rName );

    /**
        Returns the name of the Shell object. With this name, the SfxShell instance
        of BASIC can be expressed.
        */
    const OUString&             GetName() const;

    /**
        Returns the SfxViewShell in which they are located in the subshells.
        Otherwise, and if not specified by the App developer, this method
        returns NULL.
        */
    SfxViewShell*               GetViewShell() const;

    void                        CallExec( SfxExecFunc pFunc, SfxRequest &rReq )
                                { (*pFunc)(this, rReq); }
    void                        CallState( SfxStateFunc pFunc, SfxItemSet &rSet )
                                { (*pFunc)(this, rSet); }

    static void                 EmptyExecStub(SfxShell *pShell, SfxRequest &);
    static void                 EmptyStateStub(SfxShell *pShell, SfxItemSet &);

    /**
        This method returns the status of the slot with the specified slot ID
        on the specified interface.

        If the slot is disabled or in this SfxShell (and their parent shells) are
        not known, a Null-pointer is returned.

        If the slot does not have a Status, a SfxVoidItem is returned.

        The status is set directly in this Set when pStateSet != 0 , so that
        overridden Slots of the <SfxShell> Subclasses and also in the Status
        method of the base implementation can be called.

        [Example]

        In a derived class of SfxViewShell the SID_PRINTDOCDIRECT will be
        intercepted. Under certain circumstances a query should appear before
        you print, and the request will be aborted if necessary.

        Also in the IDL of this subclass of the above slot is entered. The status
        method will contain in outline:

        void SubViewShell::PrintState( SfxItemSet &rState )
        {
        if ( rState.GetItemState( SID_PRINTDOCDIRECT ) != SfxItemState::UNKNOWN )
        GetSlotState( SID_PRINTDOCDIRECT, SfxViewShell::GetInterface(),
        &rState );
        ...
        }

        [Cross-reference]

        <SfxShell::ExecuteSlot(SfxRequest&)>
        */
    const SfxPoolItem*          GetSlotState( sal_uInt16 nSlotId, const SfxInterface *pIF = nullptr, SfxItemSet *pStateSet = nullptr );

    /**
        This method allows you to forward a <SfxRequest> to the specified
        base <SfxShell>.

        [Example]

        In a derived class of SfxViewShell the SID_PRINTDOCDIRECT will be
        intercepted. Under certain circumstances a query should appear before
        you print, and the request will be aborted if necessary.

        Also in the IDL of this subclass of the above slot is entered. The status
        method will contain in outline:

        void SubViewShell::Exec( SfxRequest &rReq )
        {
        if ( rReq.GetSlot() == SID_PRINTDOCDIRECT )
        {
        'dialog'
        if ( 'condition' )
        ExecuteSlot( rReq, SfxViewShell::GetInterface() );
        }
        }

        It usually takes no rReq.Done() to be called as that is already completed
        in implementation of the SfxViewShell, for instance it has been canceled.

        [Cross-reference]

        <SfxShell::GetSlotState(sal_uInt16,const SfxInterface*,SfxItemSet*)>
        */
    const SfxPoolItem*          ExecuteSlot( SfxRequest &rReq, const SfxInterface *pIF = nullptr );

    /**
        Asynchronous ExecuteSlot for the RELOAD
        */
    void                        ExecuteSlot( SfxRequest &rReq, bool bAsync );

    inline SfxItemPool&         GetPool() const;
    inline void                 SetPool( SfxItemPool *pNewPool ) ;

    /**
        Each Subclass of SfxShell can have a <SfxUndoManager>. This can be set in
        the derived class with <SfxShell:SetUndoManager()>.

        The class SfxShell itself does not have a SfxUndoManager, a NULL-pointer
        is therefore returned.
        */
    virtual SfxUndoManager*     GetUndoManager();

    /**
        Sets a <SfxUndoManager> for this <SfxShell> Instance. For the undo
        is only the undo-manager used for SfxShell at the top of the stack of each
        <SfxDispatcher>.

        On the given <SfxUndoManager> is automatically the current
        Max-Undo-Action-Count setting set form the options.

        'pNewUndoMgr' must exist until the Destructor of SfxShell instance is called
        or until the next 'SetUndoManager()'.
        */
    void                        SetUndoManager( SfxUndoManager *pNewUndoMgr );

    /**
        Returns a pointer to the <SfxRepeatTarget> instance that is used in
        SID_REPEAT as repeat target when it is addressed from the <SfxUndoManager>
        supplied by this SfxShell. The return value can be NULL.

        [Note]

        A derivation of <SfxShell> or one of its subclasses of <SfxRepeatTarget>
        is not recommended, as compiler errors are provoked.
        (due to Call-to-Pointer-to-Member-Function to the subclass).
        */
    SfxRepeatTarget*            GetRepeatTarget() const;

    /**
        Sets the <SfxRepeatTarget> instance that is used in SID_REPEAT as
        RepeatTarget, when the current supplied by this <SfxUndoManager> is
        addressed. By 'pTarget==0' the SID_REPEAT is disabled for this SfxShell.
        The instance '*pTarget' must live as long as it is registered.

        [Note]

        A derivation of <SfxShell> or one of its subclasses of <SfxRepeatTarget>
        is not recommended, as compiler errors are provoked.
        (due to Call-to-Pointer-to-Member-Function to the subclass).
        */
    void                        SetRepeatTarget( SfxRepeatTarget *pTarget );

    /**
        With this method can the slots of the subclasses be invalidated through the
        slot Id or alternatively through the Which ID. Slot IDs, which are
        inherited by the subclass are also invalidated.

        [Cross-reference]

        <SfxBindings::Invalidate(sal_uInt16)>
        <SfxBindings::InvalidateAll(sal_Bool)>
        */
    virtual void                Invalidate(sal_uInt16 nId = 0);

    virtual bool                IsDesignMode() const { return false; };

    bool                        IsActive() const;

    /**
        Virtual method that is called when enabling the SfxShell instance,
        in order to give the Subclasses the opportunity to respond to the
        to the enabling.

        [Cross-reference]

        StarView SystemWindow::Activate(bool)
        */
    virtual void                Activate(bool bMDI);

    /**
        Virtual method that is called when disabling the SfxShell instance,
        to give the Subclasses the opportunity to respond to the disabling.

        [Cross-reference]

        StarView SystemWindow::Deactivate(bool)
        */
    virtual void                Deactivate(bool bMDI);

    /**
        This method returns a pointer to the <SfxDispatcher>, when the SfxShell
        is currently <UI-active> or a NULL-pointer if it is not UI-active.

        The returned pointer is only valid in the immediate context of the method
        call.
        */
    SfxDispatcher*              GetDispatcher() const;

    /**
        This method returns a pointer to the <SfxViewFrame> to which this SfxShell
        instance is associated or in which they currently is <UI-active>.
        A NULL pointer is returned if this SfxShell instance is not UI-active at
        the moment and also no SfxViewFrame is permanently assigned.

        The returned pointer is only valid in the immediate context of the method
        call.

        [Note]

        Only instances of a subclass of SfxApplication and SfxObjectShell
        should here provide a NULL-pointer. Otherwise, there is an error in the
        application program (wrong constructor was called from SfxShell).

        [Cross-reference]

        <SfxViewShell::GetViewFrame()const>
        */
    SfxViewFrame*               GetFrame() const;

    virtual bool                HasUIFeature(SfxShellFeature nFeature) const;
    void                        UIFeatureChanged();

    // Items
    /**
        With this method any objects of <SfxPoolItemu> subclasses can be accessed.
        This exchange method is needed if, for example special <SfxToolBoxControl>
        subclasses need access to certain data such as the <SfxObjectShell>.

        The returned instance belongs to the particular SfxShell and may be
        used only in the immediate context of the method call.

        [Cross-reference]

        <SfxShell::PutItem(const SfxPoolItem&)>
        <SfxShell::RemoveItem(sal_uInt16)>
        */
    const SfxPoolItem*          GetItem( sal_uInt16 nSlotId ) const;
    template<class T> const T*  GetItem( TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T*>(GetItem(sal_uInt16(nWhich)));
    }

    /**
        With this method, any objects of subclasses of <SfxPoolItem> can be made
        available. This exchange technology is needed if, for example, special
        <SfxToolBoxControl> Subclasses need access to certain data such as the
        <SfxObjectShell>

        If a SfxPoolItem exists with the same slot ID, it is deleted automatically.

        [Cross-reference]

        <SfxShell::RemoveItem(sal_uInt16)>
        <SfxShell::GetItem(sal_uInt16)>
        */
    void                        PutItem( const SfxPoolItem& rItem );

    // TODO/CLEANUP: still needed?!
    void SetVerbs(const css::uno::Sequence < css::embed::VerbDescriptor >& aVerbs);
    const css::uno::Sequence < css::embed::VerbDescriptor >& GetVerbs() const;
    void                        VerbExec (SfxRequest&);
    static void                 VerbState (SfxItemSet&);
    SAL_DLLPRIVATE const SfxSlot* GetVerbSlot_Impl(sal_uInt16 nId) const;

    virtual SfxObjectShell*     GetObjectShell();
    void                        SetDisableFlags( SfxDisableFlags nFlags );
    SfxDisableFlags             GetDisableFlags() const;

    virtual std::unique_ptr<SfxItemSet> CreateItemSet( sal_uInt16 nId );
    virtual void                ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );

    /** Set the name of the sidebar context that is broadcast on calls
        to Activation().
    */
    void SetContextName (const OUString& rsContextName);

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

    /**

        This method determines by calling the status function whether 'rSlot'
        can be executed currently.
        */
    SAL_DLLPRIVATE bool CanExecuteSlot_Impl( const SfxSlot &rSlot );

    /**

        This method determines whether we need to execute without checking
        the disabled state of the slot. This is used for dynamic conditions
        while you can use SfxSlotMode::FASTCALL for a specific slotid in general.
        */
    virtual bool IsConditionalFastCall( const SfxRequest &rReq );

    /**

        This method controls the activation of SfxShell instance. First, by calling
        the virtual method <SfxShell::Activate(sal_Bool)> which gives the subclass the
        opportunity to respond to the event.

        When bMDI == TRUE, the associated SbxObject is being 'armed', so that
        unqualified methods of the object (without the name of the object)
        from BASIC are found.
        */
    SAL_DLLPRIVATE void DoActivate_Impl( SfxViewFrame *pFrame, bool bMDI);

    /**

        This method controls the deactivation of the SfxShell instance. When
        bMDI == TRUE the SbxObject is first set to a status that only qualified
        BASIC methods can be called.

        Then the subclass gets the opportunity in every case to respond to the
        event by calling the virtual method <SfxShell::Deactivate(sal_Bool)>.
        */
    SAL_DLLPRIVATE void DoDeactivate_Impl( SfxViewFrame const *pFrame, bool bMDI);
};

/**
    Each Subclass of SfxShell must reference a pool. This is partly set by
    SFx's own set of subclasses (eg <SfxViewShell>). In particular however
    this must be set directly from one derived SfxShell class and ny
    derivatives of SfxObjectShell.

    The SfxShell class itself does not have any SfxItemPool, therefore a
    null-pointer is returned.
*/
SfxItemPool& SfxShell::GetPool() const
{
    assert(pPool && "no pool");
    return *pPool;
}

/**
    With this method, the subclasses register their special <SfxItemPool>
    in the SfxShell. Each SfxShell instance must have access to a SfxItemPool.
    Usually this is the SfxItemPool of the SfxDocumentShell. The SfxShell
    subclass does not take ownership of the orphaned pool. Before it is
    deleted it has to be deregistered with SetPool(0).
*/
inline void SfxShell::SetPool
(
    SfxItemPool*        pNewPool        // Pointer to the new Pool or null
)
{
    pPool = pNewPool;
}

#define SFX_DECL_INTERFACE(nId)                                  \
            static SfxInterface*     pInterface;                 \
            static SfxInterface*     GetStaticInterface();       \
            static SfxInterfaceId    GetInterfaceId() {return nId;} \
            static void              RegisterInterface(SfxModule* pMod=nullptr); \
            virtual SfxInterface*    GetInterface() const override;

#define SFX_TMPL_INTERFACE(Class,SuperClass,Abstract)                       \
                                                                            \
    SfxInterface* Class::pInterface = nullptr;                              \
    SfxInterface* Class::GetStaticInterface()                               \
    {                                                                       \
        if ( !pInterface )                                                  \
        {                                                                   \
            pInterface =                                                    \
                new SfxInterface(                                           \
            #Class, Abstract, GetInterfaceId(),                             \
            SuperClass::GetStaticInterface(),                               \
            a##Class##Slots_Impl[0],                                        \
            sal_uInt16(sizeof(a##Class##Slots_Impl) / sizeof(SfxSlot) ) );   \
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

#define SFX_IMPL_INTERFACE(Class,SuperClass)                                \
SFX_TMPL_INTERFACE(Class,SuperClass,false)                                  \

#define SFX_IMPL_SUPERCLASS_INTERFACE(Class,SuperClass)                     \
SFX_TMPL_INTERFACE(Class,SuperClass,true)                                   \

enum class SfxVisibilityFlags {
    Invisible        = 0x0000, // Never visible
    Viewer           = 0x0040,
    ReadonlyDoc      = 0x0400,
    Standard         = 0x1000,
    FullScreen       = 0x2000,
    Client           = 0x4000,
    Server           = 0x8000,
};
namespace o3tl {
    template<> struct typed_flags<SfxVisibilityFlags> : is_typed_flags<SfxVisibilityFlags, 0xf440> {};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
