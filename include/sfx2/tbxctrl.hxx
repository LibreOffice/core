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
#ifndef _SFXTBXCTRL_HXX
#define _SFXTBXCTRL_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <vcl/timer.hxx>
#include <vcl/menu.hxx>
#include <vcl/fixed.hxx>
#include <vcl/floatwin.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/sfxstatuslistener.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <svtools/framestatuslistener.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>

//------------------------------------------------------------------

class SfxToolBoxControl;
class SfxBindings;
class SfxModule;
class SfxUnoControllerItem;

svt::ToolboxController* SAL_CALL SfxToolBoxControllerFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const OUString& aCommandURL );

typedef SfxToolBoxControl* (*SfxToolBoxControlCtor)( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );

struct SfxTbxCtrlFactory
{
    SfxToolBoxControlCtor   pCtor;
    TypeId                  nTypeId;
    sal_uInt16                  nSlotId;

    SfxTbxCtrlFactory( SfxToolBoxControlCtor pTheCtor,
            TypeId nTheTypeId, sal_uInt16 nTheSlotId ):
        pCtor(pTheCtor),
        nTypeId(nTheTypeId),
        nSlotId(nTheSlotId)
    {}
};

//------------------------------------------------------------------

enum SfxPopupWindowType
{
    SFX_POPUPWINDOW_NONE,
    SFX_POPUPWINDOW_ONCLICK,
    SFX_POPUPWINDOW_ONTIMEOUT,
    SFX_POPUPWINDOW_ONCLICKANDMOVE,
    SFX_POPUPWINDOW_ONTIMEOUTANDMOVE,
    SFX_POPUPWINDOW_CONTEXTMENU
};

//------------------------------------------------------------------

class SfxFrameStatusListener : public svt::FrameStatusListener
{
    public:
        SfxFrameStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                SfxStatusListenerInterface* pCallee );
        virtual ~SfxFrameStatusListener();

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
            throw ( ::com::sun::star::uno::RuntimeException );

    private:
        SfxStatusListenerInterface* m_pCallee;
};

//------------------------------------------------------------------

/* Floating windows that can be torn from tool boxes should be derived from
   this class. Since it is also derived from SfxControllerItem, its instances
   will also receive the StateChanged calls.
*/

class SFX2_DLLPUBLIC SfxPopupWindow: public FloatingWindow, public SfxStatusListenerInterface
{
    sal_Bool                                                                             m_bFloating;
    sal_uIntPtr                                                                            m_nEventId;
    sal_Bool                                                                             m_bCascading;
    Link                                                                             m_aDeleteLink;
    sal_uInt16                                                                           m_nId;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >              m_xFrame;
    SfxFrameStatusListener*                                                          m_pStatusListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >           m_xStatusListener;

private:
    SfxFrameStatusListener* GetOrCreateStatusListener();

    SAL_DLLPRIVATE SfxPopupWindow(SfxPopupWindow &); // not defined
    SAL_DLLPRIVATE void operator =(SfxPopupWindow &); // not defined
    void Delete();

protected:
    virtual void            PopupModeEnd();
    virtual sal_Bool            Close();
    virtual void            DeleteFloatingWindow();

    sal_uInt16                  GetId() const { return m_nId; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& GetFrame() const { return m_xFrame; }

    void                    BindListener();
    void                    UnbindListener();
    void                    AddStatusListener( const OUString& rCommandURL );

    // SfxStatusListenerInterface
    using FloatingWindow::StateChanged;
    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );

public:
                            SfxPopupWindow( sal_uInt16 nId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                            WinBits nBits );
                            SfxPopupWindow( sal_uInt16 nId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                            const ResId &rId );
                            SfxPopupWindow( sal_uInt16 nId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                            Window* pParentWindow,
                                            WinBits nBits );
                            SfxPopupWindow( sal_uInt16 nId,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                            Window* pParentWindow,
                                            const ResId &rId );
                            ~SfxPopupWindow();

    virtual SfxPopupWindow* Clone() const;
    virtual void            MouseMove( const MouseEvent& rMEvt );

    void                    StartCascading();
    SAL_DLLPRIVATE void SetDeleteLink_Impl( const Link& rLink )
                            {
                                m_aDeleteLink = rLink;
                            }
};

//------------------------------------------------------------------

#define SFX_DECL_TOOLBOX_CONTROL() \
        static SfxToolBoxControl* CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ); \
        static void RegisterControl(sal_uInt16 nSlotId = 0, SfxModule *pMod=NULL)

/*  For special ToolBox controls, such as a font selection box or toolbox
    tear-off floating windows, an appropriate Item-Subclass of SfxTooBoxControl
    has to be implemented.

    This class has to be registered in SfxApplication:Init() with the static
    control method RegisterControl(). The SFx then automatically creates these
    controls in the toolbox, if the associated slots are of the specific type.
 */

struct SfxToolBoxControl_Impl;
class SFX2_DLLPUBLIC SfxToolBoxControl:
                         public ::com::sun::star::awt::XDockableWindowListener,
                         public ::com::sun::star::frame::XSubToolbarController,
                         public svt::ToolboxController

{
friend class SfxToolbox;
friend class SfxPopupWindow;
friend struct SfxTbxCtrlFactory;

    SfxToolBoxControl_Impl*    pImpl;

protected:
    DECL_LINK( PopupModeEndHdl, void * );
    DECL_LINK( ClosePopupWindow, SfxPopupWindow * );

    // old SfxToolBoxControl methods
    virtual void               StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void               Select( sal_Bool bMod1 = sal_False );
    virtual void               Select( sal_uInt16 nModifier );

    virtual void               DoubleClick();
    virtual void               Click();
    virtual SfxPopupWindowType GetPopupWindowType() const;
    virtual SfxPopupWindow*    CreatePopupWindow();
    virtual SfxPopupWindow*    CreatePopupWindowCascading();
    virtual Window*            CreateItemWindow( Window *pParent );

    // Must be called by subclass to set a new popup window instance
    void                       SetPopupWindow( SfxPopupWindow* pWindow );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void               SAL_CALL acquire() throw();
    virtual void               SAL_CALL release() throw();

    // XEventListener
    using ::cppu::OPropertySetHelper::disposing;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException );

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

    // new controller API
    // XStatusListener
    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL click()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL doubleClick()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& rParent )
        throw (::com::sun::star::uno::RuntimeException);

    // XSubToolbarController
    virtual ::sal_Bool SAL_CALL opensSubToolbar(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getSubToolbarName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL functionSelected( const OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL updateImage(  ) throw (::com::sun::star::uno::RuntimeException);

    //  XDockableWindowListener
    virtual void SAL_CALL startDocking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::DockingData SAL_CALL docking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocking( const ::com::sun::star::awt::EndDockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL prepareToggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL toggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    // helper methods
    void    createAndPositionSubToolBar( const OUString& rSubToolBarResName );
    ::Size  getPersistentFloatingSize( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, const OUString& rSubToolBarResName );
    bool    hasBigImages() const;

public:
                               SFX_DECL_TOOLBOX_CONTROL();

                               SfxToolBoxControl( sal_uInt16 nSlotID, sal_uInt16 nId, ToolBox& rBox, sal_Bool bShowStrings = sal_False );
    virtual                    ~SfxToolBoxControl();

    ToolBox&                   GetToolBox() const;
    unsigned short             GetId() const;
    unsigned short             GetSlotId() const;

    void                       Dispatch( const OUString& aCommand,
                                         ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );
    static void                Dispatch( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider,
                                         const OUString& rCommand,
                                         ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );

    static SfxItemState        GetItemState( const SfxPoolItem* pState );
    static SfxToolBoxControl*  CreateControl( sal_uInt16 nSlotId, sal_uInt16 nTbxId, ToolBox *pBox, SfxModule *pMod );
    static void                RegisterToolBoxControl( SfxModule*, SfxTbxCtrlFactory*);
};

#define SFX_IMPL_TOOLBOX_CONTROL(Class, nItemClass) \
        SfxToolBoxControl* Class::CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ) \
               { return new Class( nSlotId, nId, rTbx ); } \
        void Class::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod) \
               { SfxToolBoxControl::RegisterToolBoxControl( pMod, new SfxTbxCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }

#define SFX_IMPL_TOOLBOX_CONTROL_ARG(Class, nItemClass, Arg) \
        SfxToolBoxControl* Class::CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ) \
               { return new Class( nSlotId, nId, rTbx, Arg); } \
        void Class::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod) \
               { SfxToolBoxControl::RegisterToolBoxControl( pMod, new SfxTbxCtrlFactory( \
                    Class::CreateImpl, TYPE(nItemClass), nSlotId ) ); }

//=========================================================================


class SfxDragButton_Impl : public FixedImage
{
public:

                    SfxDragButton_Impl( Window *pParent );
    virtual void    Command ( const CommandEvent& rCEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
};

class SfxDragToolBoxControl_Impl : public SfxToolBoxControl
{
public:
                            SFX_DECL_TOOLBOX_CONTROL();
                            SfxDragToolBoxControl_Impl( sal_uInt16 nId, ToolBox& rBox );
    virtual Window*         CreateItemWindow( Window *pParent );
    using SfxToolBoxControl::Select;
    virtual void            Select( sal_Bool bMod1 = sal_False );
};

//------------------------------------------------------------------------

/** Toolbox that implements recent files menu for the Open file toolbar button.

To use that, the appropriate Sfx*Item (like Open, OpenFromCalc, or
OpenFromWriter) has to have SlotType = SfxStringItem, and the appropriate
module initialization has to call RegisterControl().
*/
class SfxRecentFilesToolBoxControl : public SfxToolBoxControl
{
public:
    // We don't use SFX_DECL_TOOLBOX_CONTROL() here as we need to have this
    // RegisterControl() marked as SFX2_DLLPUBLIC
    static SfxToolBoxControl* CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx );
    static void SFX2_DLLPUBLIC RegisterControl(sal_uInt16 nSlotId = 0, SfxModule *pMod=NULL);

    SfxRecentFilesToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
    virtual ~SfxRecentFilesToolBoxControl();

protected:
    virtual SfxPopupWindow* CreatePopupWindow();
};

class SfxReloadToolBoxControl_Impl : public SfxToolBoxControl
{
    protected:
        using SfxToolBoxControl::Select;
        virtual void Select( sal_uInt16 nSelectModifier );

    public:
    SFX_DECL_TOOLBOX_CONTROL();
        SfxReloadToolBoxControl_Impl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
};

class SfxPopupMenuManager;
class SfxAddonsToolBoxControl_Impl : public SfxToolBoxControl

/*  [Description]

    Internal helper class to provide the addons popup menu through the addons
    toolbox button.
*/

{
    sal_Bool        bBigImages;
    PopupMenu*  pMenu;
    sal_Bool        m_bShowMenuImages;

protected:
    virtual void            Click();
    using SfxToolBoxControl::Select;
    virtual void            Select( sal_Bool );
    virtual void            StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
                            DECL_LINK( Activate, Menu * );
public:
                            SFX_DECL_TOOLBOX_CONTROL();
                            SfxAddonsToolBoxControl_Impl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );
                            ~SfxAddonsToolBoxControl_Impl();

                            void RefreshMenuImages( Menu* pMenu );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
