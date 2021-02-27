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
#include <svl/poolitem.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::frame { class XDispatchProvider; }
namespace com::sun::star::frame { class XFrame; }

class InterimItemWindow;
class SfxToolBoxControl;
class SfxModule;

rtl::Reference<svt::ToolboxController> SfxToolBoxControllerFactory( const css::uno::Reference< css::frame::XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const OUString& aCommandURL );

typedef rtl::Reference<SfxToolBoxControl> (*SfxToolBoxControlCtor)( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox );

struct SfxTbxCtrlFactory
{
    SfxToolBoxControlCtor       pCtor;
    const std::type_info&       nTypeId;
    sal_uInt16                  nSlotId;

    SfxTbxCtrlFactory( SfxToolBoxControlCtor pTheCtor,
            const std::type_info& nTheTypeId, sal_uInt16 nTheSlotId ):
        pCtor(pTheCtor),
        nTypeId(nTheTypeId),
        nSlotId(nTheSlotId)
    {}
};


#define SFX_DECL_TOOLBOX_CONTROL() \
        static rtl::Reference<SfxToolBoxControl> CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ); \
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
friend struct SfxTbxCtrlFactory;

    std::unique_ptr< SfxToolBoxControl_Impl>    pImpl;

protected:
    // old SfxToolBoxControl methods
    virtual void               StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void               Select( sal_uInt16 nSelectModifier );

    virtual void               DoubleClick();
    virtual void               Click();
    virtual void               CreatePopupWindow();
    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window *pParent);

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
    static rtl::Reference<SfxToolBoxControl> CreateControl( sal_uInt16 nSlotId, sal_uInt16 nTbxId, ToolBox *pBox, SfxModule const *pMod );
    static void                RegisterToolBoxControl( SfxModule*, const SfxTbxCtrlFactory&);
};

#define SFX_IMPL_TOOLBOX_CONTROL(Class, nItemClass) \
        rtl::Reference<SfxToolBoxControl> Class::CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ) \
               { return new Class( nSlotId, nId, rTbx ); } \
        void Class::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod) \
               { SfxToolBoxControl::RegisterToolBoxControl( pMod, SfxTbxCtrlFactory( \
                    Class::CreateImpl, typeid(nItemClass), nSlotId ) ); }

#define SFX_IMPL_TOOLBOX_CONTROL_ARG(Class, nItemClass, Arg) \
        rtl::Reference<SfxToolBoxControl> Class::CreateImpl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox &rTbx ) \
               { return new Class( nSlotId, nId, rTbx, Arg); } \
        void Class::RegisterControl(sal_uInt16 nSlotId, SfxModule *pMod) \
               { SfxToolBoxControl::RegisterToolBoxControl( pMod, SfxTbxCtrlFactory( \
                    Class::CreateImpl, typeid(nItemClass), nSlotId ) ); }


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
