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

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_

#include <threadhelp/threadhelpbase.hxx>
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
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <tools/link.hxx>
#include <vcl/window.hxx>
#include <vcl/timer.hxx>

class PopupMenu;
class ToolBox;

namespace framework
{

typedef ::cppu::WeakImplHelper4<
           ::com::sun::star::frame::XFrameActionListener,
           ::com::sun::star::frame::XStatusListener,
           ::com::sun::star::lang::XComponent,
           ::com::sun::star::ui::XUIConfigurationListener
        > ToolbarManager_Base;

class ToolBarManager : public ToolbarManager_Base,
                       public ThreadHelpBase
{
    public:
        ToolBarManager( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                        const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                        const OUString& rResourceName,
                        ToolBox* pToolBar );
        virtual ~ToolBarManager();

        ToolBox* GetToolBar() const;

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const com::sun::star::frame::FrameActionEvent& Action ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );
        void SAL_CALL addEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );
        void SAL_CALL removeEventListener( const com::sun::star::uno::Reference< XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );

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
        DECL_LINK( Command, CommandEvent * );
        PopupMenu * GetToolBarCustomMenu(ToolBox* pToolBar);
        DECL_LINK(Click, void *);
        DECL_LINK(DropdownClick, void *);
        DECL_LINK(DoubleClick, void *);
        DECL_LINK(Select, void *);
        DECL_LINK(Activate, void *);
        DECL_LINK(Deactivate, void *);
        DECL_LINK( StateChanged, StateChangedType* );
        DECL_LINK( DataChanged, DataChangedEvent* );
        DECL_LINK( MiscOptionsChanged, void* );

        DECL_LINK( MenuButton, ToolBox * );
        DECL_LINK( MenuSelect, Menu * );
        DECL_LINK( MenuDeactivate, Menu * );
        DECL_LINK(AsyncUpdateControllersHdl, void *);
        DECL_STATIC_LINK( ToolBarManager, ExecuteHdl_Impl, ExecuteInfo* );

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
        sal_uInt16 ConvertStyleToToolboxItemBits( sal_Int32 nStyle );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetModelFromFrame() const;
        sal_Bool IsPluginMode() const;
        Image QueryAddonsImage( const OUString& aCommandURL, bool bBigImages );
        long HandleClick(void ( SAL_CALL ::com::sun::star::frame::XToolbarController::*_pClick )(  ));
        void setToolBarImage(const Image& _aImage,const CommandToInfoMap::const_iterator& _pIter);
        void impl_elementChanged(bool _bRemove,const ::com::sun::star::ui::ConfigurationEvent& Event );

        static bool impl_RetrieveShortcutsFromConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >& rAccelCfg, const OUString& rCommand, OUString& rShortCut );
        bool RetrieveShortcut( const OUString& rCommandURL, OUString& rShortCut );

    protected:
        typedef ::boost::unordered_map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > > ToolBarControllerMap;
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XSubToolbarController > > SubToolBarControllerVector;
        typedef BaseHash< SubToolBarControllerVector >                                                              SubToolBarToSubToolBarControllerMap;

        typedef ::boost::unordered_map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > > MenuDescriptionMap;

        bool m_bDisposed : 1,
             m_bSmallSymbols : 1,
             m_bModuleIdentified : 1,
             m_bAddedToTaskPaneList : 1,
             m_bFrameActionRegistered : 1,
             m_bUpdateControllers : 1,
             m_bImageOrientationRegistered : 1,
             m_bImageMirrored : 1;

        long m_lImageRotation;

        ToolBox* m_pToolBar;

        OUString m_aModuleIdentifier;
        OUString m_aResourceName;

        com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >              m_xURLTransformer;
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame >                        m_xFrame;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >               m_xUICommandLabels;
        ToolBarControllerMap                                                                   m_aControllerMap;
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
        sal_Int16                                                                              m_nSymbolsStyle;
        MenuDescriptionMap m_aMenuMap;
        sal_Bool                                                                               m_bAcceleratorCfg;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xDocAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xModuleAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xGlobalAcceleratorManager;
};

}

#endif // __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
