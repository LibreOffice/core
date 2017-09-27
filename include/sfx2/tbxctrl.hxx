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
#ifndef INCLUDED_SFX2_TBXCTRL_HXX
#define INCLUDED_SFX2_TBXCTRL_HXX

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
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
#include <rtl/ref.hxx>


class SfxToolBoxControl;
class SfxModule;

svt::ToolboxController* SAL_CALL SfxToolBoxControllerFactory( const css::uno::Reference< css::frame::XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const OUString& aCommandURL );

typedef SfxToolBoxControl* (*SfxToolBoxControlCtor)( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );

struct SfxTbxCtrlFactory
{
    SfxToolBoxControlCtor   pCtor;
    const std::type_info&       nTypeId;
    sal_uInt16                  nSlotId;

    SfxTbxCtrlFactory( SfxToolBoxControlCtor pTheCtor,
            const std::type_info& nTheTypeId, sal_uInt16 nTheSlotId ):
        pCtor(pTheCtor),
        nTypeId(nTheTypeId),
        nSlotId(nTheSlotId)
    {}
};


/* Floating windows that can be torn from tool boxes should be derived from
   this class. Since it is also derived from SfxControllerItem, its instances
   will also receive the StateChanged calls.
*/
class SfxFrameStatusListener;
class SFX2_DLLPUBLIC SfxPopupWindow: public FloatingWindow
{
friend class SfxFrameStatusListener;
    bool                                                   m_bFloating;
    bool                                                   m_bCascading;
    Link<SfxPopupWindow*,void>                             m_aDeleteLink;
    sal_uInt16                                             m_nId;
    css::uno::Reference< css::frame::XFrame > const        m_xFrame;
    rtl::Reference<SfxFrameStatusListener>                 m_xStatusListener;

private:
    SfxPopupWindow(SfxPopupWindow &) = delete;
    void operator =(SfxPopupWindow &) = delete;
    void Delete();

protected:
    virtual void            PopupModeEnd() override;
    virtual bool            Close() override;

    sal_uInt16              GetId() const { return m_nId; }
    const css::uno::Reference< css::frame::XFrame >& GetFrame() const { return m_xFrame; }

    void                    AddStatusListener( const OUString& rCommandURL );

    virtual void            statusChanged( const css::frame::FeatureStateEvent& rEvent );

public:
                            SfxPopupWindow(sal_uInt16 nId, vcl::Window *pParent,
                                            const css::uno::Reference< css::frame::XFrame >& rFrame,
                                            WinBits nBits );
                            SfxPopupWindow(sal_uInt16 nId, vcl::Window *pParent,
                                           const OString& rID, const OUString& rUIXMLDescription,
                                           const css::uno::Reference<css::frame::XFrame> &rFrame =
                                               css::uno::Reference<css::frame::XFrame>());
                            virtual ~SfxPopupWindow() override;
    virtual void            dispose() override;

    virtual void            MouseMove( const MouseEvent& rMEvt ) override;

    void                    StartCascading();
    SAL_DLLPRIVATE void SetDeleteLink_Impl( const Link<SfxPopupWindow*,void>& rLink )
                            {
                                m_aDeleteLink = rLink;
                            }
};


#define SFX_DECL_TOOLBOX_CONTROL() \
        static SfxToolBoxControl* CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ); \
        static void RegisterControl(sal_uInt16 nSlotId = 0, SfxModule *pMod=nullptr)

/*  For special ToolBox controls, such as a font selection box or toolbox
    tear-off floating windows, an appropriate Item-Subclass of SfxTooBoxControl
    has to be implemented.

    This class has to be registered in SfxApplication:Init() with the static
    control method RegisterControl(). The SFx then automatically creates these
    controls in the toolbox, if the associated slots are of the specific type.
 */

struct SfxToolBoxControl_Impl;
class SFX2_DLLPUBLIC SfxToolBoxControl: public svt::ToolboxController
{
friend class SfxPopupWindow;
friend struct SfxTbxCtrlFactory;

    std::unique_ptr< SfxToolBoxControl_Impl>    pImpl;

protected:
    DECL_LINK( PopupModeEndHdl, FloatingWindow*, void );
    DECL_LINK( ClosePopupWindow, SfxPopupWindow *, void );

    // old SfxToolBoxControl methods
    virtual void               StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void               Select( sal_uInt16 nSelectModifier );

    virtual void               DoubleClick();
    virtual void               Click();
    virtual VclPtr<SfxPopupWindow> CreatePopupWindow();
    virtual VclPtr<vcl::Window> CreateItemWindow( vcl::Window *pParent );

    // Must be called by subclass to set a new popup window instance
    void                       SetPopupWindow( SfxPopupWindow* pWindow );

public:
    // XComponent
    virtual void SAL_CALL dispose() override;

    // new controller API
    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;
    virtual void SAL_CALL click() override;
    virtual void SAL_CALL doubleClick() override;
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& rParent ) override;

public:
                               SFX_DECL_TOOLBOX_CONTROL();

                               SfxToolBoxControl( sal_uInt16 nSlotID, sal_uInt16 nId, ToolBox& rBox, bool bShowStrings = false );
    virtual                    ~SfxToolBoxControl() override;

    ToolBox&                   GetToolBox() const;
    unsigned short             GetId() const;
    unsigned short             GetSlotId() const;

    void                       Dispatch( const OUString& aCommand,
                                         css::uno::Sequence< css::beans::PropertyValue > const & aArgs );
    static void                Dispatch( const css::uno::Reference< css::frame::XDispatchProvider >& rDispatchProvider,
                                         const OUString& rCommand,
                                         css::uno::Sequence< css::beans::PropertyValue > const & aArgs );

    static SfxItemState        GetItemState( const SfxPoolItem* pState );
    static SfxToolBoxControl*  CreateControl( sal_uInt16 nSlotId, sal_uInt16 nTbxId, ToolBox *pBox, SfxModule const *pMod );
    static void                RegisterToolBoxControl( SfxModule*, const SfxTbxCtrlFactory&);
};

#define SFX_IMPL_TOOLBOX_CONTROL(Class, nItemClass) \
        SfxToolBoxControl* Class::CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ) \
               { return new Class( nSlotId, nId, rTbx ); } \
        void Class::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod) \
               { SfxToolBoxControl::RegisterToolBoxControl( pMod, SfxTbxCtrlFactory( \
                    Class::CreateImpl, typeid(nItemClass), nSlotId ) ); }

#define SFX_IMPL_TOOLBOX_CONTROL_ARG(Class, nItemClass, Arg) \
        SfxToolBoxControl* Class::CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ) \
               { return new Class( nSlotId, nId, rTbx, Arg); } \
        void Class::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod) \
               { SfxToolBoxControl::RegisterToolBoxControl( pMod, SfxTbxCtrlFactory( \
                    Class::CreateImpl, typeid(nItemClass), nSlotId ) ); }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
