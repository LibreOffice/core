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

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <stdtypes.h>
#include <uielement/commandinfo.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
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

namespace framework
{

typedef ::cppu::WeakImplHelper<
           ::com::sun::star::frame::XFrameActionListener,
           ::com::sun::star::frame::XStatusListener,
           ::com::sun::star::lang::XComponent,
           ::com::sun::star::ui::XUIConfigurationListener
        > ToolbarManager_Base;

class ToolBarManager : public ToolbarManager_Base
{
    public:
        ToolBarManager( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                        const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                        const OUString& rResourceName,
                        ToolBox* pToolBar );
        virtual ~ToolBarManager();

        ToolBox* GetToolBar() const;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const com::sun::star::frame::FrameActionEvent& Action ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        void SAL_CALL addEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        void SAL_CALL removeEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        void CheckAndUpdateImages();
        virtual void RefreshImages();
        void FillToolbar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rToolBarData );
        void notifyRegisteredControllers( const OUString& aUIElementName, const OUString& aCommand );
        void Destroy();

        enum ExecuteCommand
        {
            EXEC_CMD_CLOSETOOLBAR,
            EXEC_CMD_DOCKTOOLBAR,
            EXEC_CMD_DOCKALLTOOLBARS,
            EXEC_CMD_NONE,
            EXEC_CMD_COUNT
        };

        struct ExecuteInfo
        {
            OUString   aToolbarResName;
            ExecuteCommand  nCmd;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xWindow;
        };
        struct ControllerParams
        {
            sal_Int16 nWidth;
        };
        typedef std::vector< ControllerParams > ControllerParamsVector;

    protected:
        DECL_LINK_TYPED( Command, CommandEvent const *, void );
        PopupMenu * GetToolBarCustomMenu(ToolBox* pToolBar);
        DECL_LINK_TYPED(Click, ToolBox *, void);
        DECL_LINK_TYPED(DropdownClick, ToolBox *, void);
        DECL_LINK_TYPED(DoubleClick, ToolBox *, void);
        DECL_LINK_TYPED(Select, ToolBox *, void);
        DECL_LINK_TYPED( StateChanged, StateChangedType const *, void );
        DECL_LINK_TYPED( DataChanged, DataChangedEvent const *, void );
        DECL_LINK_TYPED( MiscOptionsChanged, LinkParamNone*, void );

        DECL_LINK_TYPED( MenuButton, ToolBox *, void );
        DECL_LINK_TYPED( MenuSelect, Menu *, bool );
        DECL_LINK_TYPED( MenuDeactivate, Menu *, bool );
        DECL_LINK_TYPED(AsyncUpdateControllersHdl, Timer *, void);
        DECL_STATIC_LINK_TYPED( ToolBarManager, ExecuteHdl_Impl, void*, void );

        virtual bool MenuItemAllowed( sal_uInt16 ) const;

        void RemoveControllers();
        OUString RetrieveLabelFromCommand( const OUString& aCmdURL );
        sal_Int32 RetrievePropertiesFromCommand( const OUString& aCmdURL );
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetPropsForCommand( const OUString& rCmdURL );
        void CreateControllers();
        void UpdateControllers();
        //for update controller via Support Visiable
        void UpdateController( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XToolbarController > xController);
        //end
        void AddFrameActionListener();
        void AddImageOrientationListener();
        void UpdateImageOrientation();
        void ImplClearPopupMenu( ToolBox *pToolBar );
        void RequestImages();
        ToolBoxItemBits ConvertStyleToToolboxItemBits( sal_Int32 nStyle );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetModelFromFrame() const;
        bool IsPluginMode() const;
        Image QueryAddonsImage( const OUString& aCommandURL, bool bBigImages );
        long HandleClick(void ( SAL_CALL ::com::sun::star::frame::XToolbarController::*_pClick )(  ));
        void setToolBarImage(const Image& _aImage,const CommandToInfoMap::const_iterator& _pIter);
        void impl_elementChanged(bool _bRemove,const ::com::sun::star::ui::ConfigurationEvent& Event );

        static bool impl_RetrieveShortcutsFromConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >& rAccelCfg, const OUString& rCommand, OUString& rShortCut );
        bool RetrieveShortcut( const OUString& rCommandURL, OUString& rShortCut );

    protected:
        typedef std::unordered_map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > > ToolBarControllerMap;
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XSubToolbarController > > SubToolBarControllerVector;
        typedef BaseHash< SubToolBarControllerVector >                                                              SubToolBarToSubToolBarControllerMap;

        typedef std::unordered_map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > > MenuDescriptionMap;

        bool m_bDisposed : 1,
             m_bSmallSymbols : 1,
             m_bModuleIdentified : 1,
             m_bAddedToTaskPaneList : 1,
             m_bFrameActionRegistered : 1,
             m_bUpdateControllers : 1,
             m_bImageOrientationRegistered : 1,
             m_bImageMirrored : 1;

        long m_lImageRotation;

        VclPtr<ToolBox> m_pToolBar;

        OUString m_aModuleIdentifier;
        OUString m_aResourceName;

        com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >              m_xURLTransformer;
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame >                        m_xFrame;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >               m_xUICommandLabels;
        ToolBarControllerMap                                                                   m_aControllerMap;
        osl::Mutex m_mutex;
        ::cppu::OMultiTypeInterfaceContainerHelper                                             m_aListenerContainer;   /// container for ALL Listener
        ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >             m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUIControllerFactory > m_xToolbarControllerFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >                m_xModuleImageManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >                m_xDocImageManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >                 m_xImageOrientationListener;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >                m_xUICfgMgr;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >                m_xDocUICfgMgr;

        CommandToInfoMap                                                                       m_aCommandMap;
        SubToolBarToSubToolBarControllerMap                                                    m_aSubToolBarControllerMap;
        Timer                                                                                  m_aAsyncUpdateControllersTimer;
        OUString                                                                               m_sIconTheme;
        MenuDescriptionMap m_aMenuMap;
        bool                                                                               m_bAcceleratorCfg;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xDocAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xModuleAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xGlobalAcceleratorManager;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_TOOLBARMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
