/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <stdtypes.h>
#include <uielement/commandinfo.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XUIControllerRegistration.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

#include <com/sun/star/frame/XToolbarController.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/accel.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace frame
            {
                class XLayoutManager;
            }
        }
    }
}

namespace framework
{

class ToolBar;
class ToolBarManager : public ::com::sun::star::frame::XFrameActionListener         ,
                       public ::com::sun::star::frame::XStatusListener              ,
                       public ::com::sun::star::lang::XComponent                    ,
                       public ::com::sun::star::lang::XTypeProvider                 ,
                       public ::com::sun::star::ui::XUIConfigurationListener,
                       public ThreadHelpBase                                        ,
                       public ::cppu::OWeakObject
{
    public:
        ToolBarManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServicveManager,
                        const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                        const rtl::OUString& rResourceName,
                        ToolBar* pToolBar );
        virtual ~ToolBarManager();

        //  XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

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
        void notifyRegisteredControllers( const rtl::OUString& aUIElementName, const rtl::OUString& aCommand );
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
            rtl::OUString   aToolbarResName;
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
        rtl::OUString RetrieveLabelFromCommand( const rtl::OUString& aCmdURL );
        sal_Int32 RetrievePropertiesFromCommand( const rtl::OUString& aCmdURL );
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetPropsForCommand( const ::rtl::OUString& rCmdURL );
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
        Image QueryAddonsImage( const ::rtl::OUString& aCommandURL, bool bBigImages );
        long HandleClick(void ( SAL_CALL ::com::sun::star::frame::XToolbarController::*_pClick )(  ));
        void setToolBarImage(const Image& _aImage,const CommandToInfoMap::const_iterator& _pIter);
        void impl_elementChanged(bool _bRemove,const ::com::sun::star::ui::ConfigurationEvent& Event );

        static bool impl_RetrieveShortcutsFromConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >& rAccelCfg, const rtl::OUString& rCommand, rtl::OUString& rShortCut );
        bool RetrieveShortcut( const rtl::OUString& rCommandURL, rtl::OUString& rShortCut );

    protected:
        typedef ::boost::unordered_map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::frame::XStatusListener > > ToolBarControllerMap;
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XSubToolbarController > > SubToolBarControllerVector;
        typedef BaseHash< SubToolBarControllerVector >                                                              SubToolBarToSubToolBarControllerMap;

        typedef ::boost::unordered_map< sal_uInt16, ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > > MenuDescriptionMap;
        sal_Bool                                                                               m_bDisposed : 1,
                                                                                               m_bSmallSymbols : 1,
                                                                                               m_bModuleIdentified : 1,
                                                                                               m_bAddedToTaskPaneList : 1,
                                                                                               m_bVerticalTextEnabled : 1,
                                                                                               m_bFrameActionRegistered : 1,
                                                                                               m_bUpdateControllers : 1;
        sal_Bool                                                                               m_bImageOrientationRegistered : 1,
                                                                                               m_bImageMirrored : 1;
        long                                                                                   m_lImageRotation;
        ToolBar*                                                                               m_pToolBar;
        rtl::OUString                                                                          m_aModuleIdentifier;
        rtl::OUString                                                                          m_aResourceName;
        com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >              m_xURLTransformer;
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame >                        m_xFrame;
        com::sun::star::uno::Reference< com::sun::star::container::XNameAccess >               m_xUICommandLabels;
        ToolBarControllerMap                                                                   m_aControllerMap;
        ::cppu::OMultiTypeInterfaceContainerHelper                                             m_aListenerContainer;   /// container for ALL Listener
        ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >         m_xServiceManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUIControllerRegistration > m_xToolbarControllerRegistration;
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
