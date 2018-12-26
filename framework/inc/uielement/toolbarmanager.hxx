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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMANAGER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMANAGER_HXX

#include <stdtypes.h>
#include <uielement/commandinfo.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <vcl/timer.hxx>

#include <unordered_map>

class PopupMenu;
class ToolBox;
class Menu;

namespace framework
{

typedef ::cppu::WeakImplHelper<
           css::frame::XFrameActionListener,
           css::lang::XComponent,
           css::ui::XUIConfigurationListener
        > ToolbarManager_Base;

class ToolBarManager : public ToolbarManager_Base
{
    public:
        ToolBarManager( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                        const css::uno::Reference< css::frame::XFrame >& rFrame,
                        const OUString& rResourceName,
                        ToolBox* pToolBar );
        virtual ~ToolBarManager() override;

        ToolBox* GetToolBar() const;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& Action ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& Event ) override;
        virtual void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& Event ) override;
        virtual void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& Event ) override;

        // XComponent
        void SAL_CALL dispose() override;
        void SAL_CALL addEventListener( const css::uno::Reference< XEventListener >& xListener ) override;
        void SAL_CALL removeEventListener( const css::uno::Reference< XEventListener >& xListener ) override;

        void CheckAndUpdateImages();
        virtual void RefreshImages();
        void FillToolbar( const css::uno::Reference< css::container::XIndexAccess >& rToolBarData );
        void FillOverflowToolbar( ToolBox const * pParent );
        void notifyRegisteredControllers( const OUString& aUIElementName, const OUString& aCommand );
        void Destroy();

        enum ExecuteCommand
        {
            EXEC_CMD_CLOSETOOLBAR,
            EXEC_CMD_DOCKTOOLBAR,
            EXEC_CMD_DOCKALLTOOLBARS
        };

        struct ExecuteInfo
        {
            OUString                                            aToolbarResName;
            ExecuteCommand                                      nCmd;
            css::uno::Reference< css::frame::XLayoutManager >   xLayoutManager;
            css::uno::Reference< css::awt::XWindow >            xWindow;
        };

    protected:
        DECL_LINK(Click, ToolBox *, void);
        DECL_LINK(DropdownClick, ToolBox *, void);
        DECL_LINK(DoubleClick, ToolBox *, void);
        DECL_LINK(Select, ToolBox *, void);
        DECL_LINK( StateChanged, StateChangedType const *, void );
        DECL_LINK( DataChanged, DataChangedEvent const *, void );
        DECL_LINK( MiscOptionsChanged, LinkParamNone*, void );

        DECL_LINK( MenuButton, ToolBox *, void );
        DECL_LINK( MenuPreExecute, ToolBox *, void );
        DECL_LINK( MenuSelect, Menu *, bool );
        DECL_LINK(AsyncUpdateControllersHdl, Timer *, void);
        DECL_LINK( OverflowEventListener, VclWindowEvent&, void );
        DECL_STATIC_LINK( ToolBarManager, ExecuteHdl_Impl, void*, void );

        virtual bool MenuItemAllowed( sal_uInt16 ) const;

        void AddCustomizeMenuItems(ToolBox const * pToolBar);
        void InitImageManager();
        void RemoveControllers();
        void CreateControllers();
        void UpdateControllers();
        //for update controller via Support Visible
        void UpdateController( const css::uno::Reference< css::frame::XToolbarController >& xController);
        //end
        void AddFrameActionListener();
        void RequestImages();
        ToolBoxItemBits ConvertStyleToToolboxItemBits( sal_Int32 nStyle );
        css::uno::Reference< css::frame::XModel > GetModelFromFrame() const;
        bool IsPluginMode() const;
        void HandleClick(void ( SAL_CALL css::frame::XToolbarController::*_pClick )(  ));
        void setToolBarImage(const Image& _aImage,const CommandToInfoMap::const_iterator& _pIter);
        void impl_elementChanged(bool _bRemove,const css::ui::ConfigurationEvent& Event );

    protected:
        typedef std::unordered_map< sal_uInt16, css::uno::Reference< css::frame::XStatusListener > >  ToolBarControllerMap;
        typedef ::std::vector< css::uno::Reference< css::frame::XSubToolbarController > >             SubToolBarControllerVector;
        typedef std::unordered_map<OUString, SubToolBarControllerVector>                                                SubToolBarToSubToolBarControllerMap;

        bool m_bDisposed : 1,
             m_bAddedToTaskPaneList : 1,
             m_bFrameActionRegistered : 1,
             m_bUpdateControllers : 1;

        sal_Int16 m_eSymbolSize;

        VclPtr<ToolBox>                                              m_pToolBar;

        OUString                                                     m_aModuleIdentifier;
        OUString                                                     m_aResourceName;

        css::uno::Reference< css::util::XURLTransformer >            m_xURLTransformer;
        css::uno::Reference< css::frame::XFrame >                    m_xFrame;
        ToolBarControllerMap                                         m_aControllerMap;
        osl::Mutex                                                   m_mutex;
        ::cppu::OMultiTypeInterfaceContainerHelper                   m_aListenerContainer;   /// container for ALL Listener
        css::uno::Reference< css::uno::XComponentContext >           m_xContext;
        css::uno::Reference< css::frame::XUIControllerFactory >      m_xToolbarControllerFactory;
        css::uno::Reference< css::ui::XImageManager >                m_xModuleImageManager;
        css::uno::Reference< css::ui::XImageManager >                m_xDocImageManager;

        CommandToInfoMap                                             m_aCommandMap;
        SubToolBarToSubToolBarControllerMap                          m_aSubToolBarControllerMap;
        Timer                                                        m_aAsyncUpdateControllersTimer;
        OUString                                                     m_sIconTheme;

        rtl::Reference< ToolBarManager >                             m_aOverflowManager;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
