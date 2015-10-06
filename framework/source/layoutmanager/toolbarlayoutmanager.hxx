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

#ifndef INCLUDED_FRAMEWORK_SOURCE_LAYOUTMANAGER_TOOLBARLAYOUTMANAGER_HXX
#define INCLUDED_FRAMEWORK_SOURCE_LAYOUTMANAGER_TOOLBARLAYOUTMANAGER_HXX

/** Attention: stl headers must(!) be included at first. Otherwise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <properties.h>
#include <uiconfiguration/globalsettings.hxx>
#include <framework/addonsoptions.hxx>
#include <uielement/uielement.hxx>
#include <services/layoutmanager.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/awt/XToolkit2.hpp>
#include <com/sun/star/awt/XTopWindow2.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/awt/XDockableWindowListener.hpp>

#include <cppuhelper/implbase.hxx>

namespace framework
{

class ToolbarLayoutManager : public ::cppu::WeakImplHelper< ::com::sun::star::awt::XDockableWindowListener,
                                                             ::com::sun::star::ui::XUIConfigurationListener,
                                                             ::com::sun::star::awt::XWindowListener >
{
    public:
        enum { DOCKINGAREAS_COUNT = 4 };

        enum PreviewFrameDetection
        {
            PREVIEWFRAME_UNKNOWN,
            PREVIEWFRAME_NO,
            PREVIEWFRAME_YES
        };

        ToolbarLayoutManager( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElementFactory >& xUIElementFactory,
                              LayoutManager* pParentLayouter );
        virtual ~ToolbarLayoutManager();

        void reset();
        void attach( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >& xModuleCfgMgr,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >& xDocCfgMgr,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xPersistentWindowState );

        void setParentWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& xParentWindow );
        void setDockingAreaOffsets(const ::Rectangle& rOffsets);

        void resetDockingArea();

        ::com::sun::star::awt::Rectangle getDockingArea();
        void setDockingArea( const ::com::sun::star::awt::Rectangle& rDockingArea );

        bool isPreviewFrame();

        // layouting
        bool isLayoutDirty() { return m_bLayoutDirty;}
        void doLayout(const ::Size& aContainerSize);

        // creation/destruction
        void createStaticToolbars();
        void destroyToolbars();

        bool requestToolbar( const OUString& rResourceURL );
        bool createToolbar( const OUString& rResourceURL );
        bool destroyToolbar( const OUString& rResourceURL );

        // visibility
        bool showToolbar( const OUString& rResourceURL );
        bool hideToolbar( const OUString& rResourceURL );

        void refreshToolbarsVisibility( bool bAutomaticToolbars );
        void setFloatingToolbarsVisibility( bool bVisible );
        void setVisible(bool bVisible);

        // docking and further functions
        bool dockToolbar( const OUString& rResourceURL, ::com::sun::star::ui::DockingArea eDockingArea, const ::com::sun::star::awt::Point& aPos );
        bool dockAllToolbars();
        bool floatToolbar( const OUString& rResoureURL );
        bool lockToolbar( const OUString& rResourceURL );
        bool unlockToolbar( const OUString& rResourceURL );
        void setToolbarPos( const OUString& rResourceURL, const ::com::sun::star::awt::Point& aPos );
        void setToolbarSize( const OUString& rResourceURL, const ::com::sun::star::awt::Size& aSize );
        void setToolbarPosSize( const OUString& rResourceURL, const ::com::sun::star::awt::Point& aPos, const ::com::sun::star::awt::Size& aSize );
        bool isToolbarVisible( const OUString& rResourceURL );
        bool isToolbarFloating( const OUString& rResourceURL );
        bool isToolbarDocked( const OUString& rResourceURL );
        bool isToolbarLocked( const OUString& rResourceURL );
        ::com::sun::star::awt::Point getToolbarPos( const OUString& rResourceURL );
        ::com::sun::star::awt::Size getToolbarSize( const OUString& rResourceURL );
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > getToolbar( const OUString& aName );
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > > getToolbars();

        // child window notifications
        long childWindowEvent( VclSimpleEvent* pEvent );

        // XInterface

        virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
        virtual void SAL_CALL release() throw() SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XWindowListener
        virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

        // XDockableWindowListener
        virtual void SAL_CALL startDocking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::awt::DockingData SAL_CALL docking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL endDocking( const ::com::sun::star::awt::EndDockingEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL prepareToggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL toggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL closed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        enum DockingOperation
        {
            DOCKOP_BEFORE_COLROW,
            DOCKOP_ON_COLROW,
            DOCKOP_AFTER_COLROW
        };

        typedef std::vector< UIElement > UIElementVector;
        struct SingleRowColumnWindowData
        {
            SingleRowColumnWindowData()
                : nVarSize(0)
                , nStaticSize(0)
                , nSpace(0)
                , nRowColumn(0)
            {}

            std::vector< OUString >                                                      aUIElementNames;
            std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > aRowColumnWindows;
            std::vector< ::com::sun::star::awt::Rectangle >                                   aRowColumnWindowSizes;
            std::vector< sal_Int32 >                                                          aRowColumnSpace;
            ::com::sun::star::awt::Rectangle                                                  aRowColumnRect;
            sal_Int32                                                                         nVarSize;
            sal_Int32                                                                         nStaticSize;
            sal_Int32                                                                         nSpace;
            sal_Int32                                                                         nRowColumn;
        };

        // internal helper methods

        bool             implts_isParentWindowVisible() const;
        ::Rectangle      implts_calcDockingArea();
        void             implts_sortUIElements();
        void             implts_reparentToolbars();
        OUString    implts_generateGenericAddonToolbarTitle( sal_Int32 nNumber ) const;
        void             implts_setElementData( UIElement& rUIElement, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindow >& rDockWindow );
        void             implts_destroyDockingAreaWindows();

        // layout methods

        void             implts_setDockingAreaWindowSizes( const ::com::sun::star::awt::Rectangle& rBorderSpace );
        ::com::sun::star::awt::Point implts_findNextCascadeFloatingPos();
        void             implts_renumberRowColumnData( ::com::sun::star::ui::DockingArea eDockingArea, DockingOperation eDockingOperation, const UIElement& rUIElement );
        void             implts_calcWindowPosSizeOnSingleRowColumn( sal_Int32 nDockingArea,
                                                                    sal_Int32 nOffset,
                                                                    SingleRowColumnWindowData& rRowColumnWindowData,
                                                                    const ::Size& rContainerSize );
        void             implts_setLayoutDirty();
        void             implts_setLayoutInProgress( bool bInProgress = true );

        // lookup/container methods

        UIElement        implts_findToolbar( const OUString& aName );
        UIElement        implts_findToolbar( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xToolbar );
        UIElement&       impl_findToolbar( const OUString& aName );
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > implts_getXWindow( const OUString& aName );
        vcl::Window*          implts_getWindow( const OUString& aName );
        bool             implts_insertToolbar( const UIElement& rUIElement );
        void             implts_setToolbar( const UIElement& rUIElement );
        ::Size           implts_getTopBottomDockingAreaSizes();
        void             implts_getUIElementVectorCopy( UIElementVector& rCopy );

        // internal docking methods

        ::Rectangle      implts_calcHotZoneRect( const ::Rectangle& rRect, sal_Int32 nHotZoneOffset );
        void             implts_calcDockingPosSize( UIElement& aUIElement, DockingOperation& eDockOperation, ::Rectangle& rTrackingRect, const Point& rMousePos );
        DockingOperation implts_determineDockingOperation( ::com::sun::star::ui::DockingArea DockingArea, const ::Rectangle& rRowColRect, const Point& rMousePos );
        ::Rectangle      implts_getWindowRectFromRowColumn( ::com::sun::star::ui::DockingArea DockingArea, const SingleRowColumnWindowData& rRowColumnWindowData, const ::Point& rMousePos, const OUString& rExcludeElementName );
        ::Rectangle      implts_determineFrontDockingRect( ::com::sun::star::ui::DockingArea eDockingArea,
                                                           sal_Int32 nRowCol,
                                                           const ::Rectangle& rDockedElementRect,
                                                           const OUString& rMovedElementName,
                                                           const ::Rectangle& rMovedElementRect );
        ::Rectangle      implts_calcTrackingAndElementRect( ::com::sun::star::ui::DockingArea eDockingArea,
                                                            sal_Int32 nRowCol,
                                                            UIElement& rUIElement,
                                                            const ::Rectangle& rTrackingRect,
                                                            const ::Rectangle& rRowColumnRect,
                                                            const ::Size& rContainerWinSize );

        void             implts_getDockingAreaElementInfos( ::com::sun::star::ui::DockingArea DockingArea, std::vector< SingleRowColumnWindowData >& rRowColumnsWindowData );
        void             implts_getDockingAreaElementInfoOnSingleRowCol( ::com::sun::star::ui::DockingArea, sal_Int32 nRowCol, SingleRowColumnWindowData& rRowColumnWindowData );
        void             implts_findNextDockingPos( ::com::sun::star::ui::DockingArea DockingArea, const ::Size& aUIElementSize, ::com::sun::star::awt::Point& rVirtualPos, ::Point& rPixelPos );
        void             implts_setTrackingRect( ::com::sun::star::ui::DockingArea eDockingArea, const ::Point& rMousePos, ::Rectangle& rTrackingRect );

        // creation methods

        void             implts_createAddonsToolBars();
        void             implts_createCustomToolBars();
        void             implts_createNonContextSensitiveToolBars();
        void             implts_createCustomToolBars( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& aCustomTbxSeq );
        void             implts_createCustomToolBar( const OUString& aTbxResName, const OUString& aTitle );
        void             implts_setToolbarCreation( bool bStart = true );
        bool             implts_isToolbarCreationActive();

        // persistence methods

        bool         implts_readWindowStateData( const OUString& aName, UIElement& rElementData );
        void             implts_writeWindowStateData( const UIElement& rElementData );

        // members

        css::uno::Reference< css::uno::XComponentContext >                   m_xContext;
        css::uno::Reference< css::frame::XFrame >                            m_xFrame;
        css::uno::Reference< css::awt::XWindow2 >                            m_xContainerWindow;
        css::uno::Reference< css::awt::XWindow >                             m_xDockAreaWindows[DOCKINGAREAS_COUNT];
        css::uno::Reference< ::com::sun::star::ui::XUIElementFactory >       m_xUIElementFactoryManager;
        css::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager > m_xModuleCfgMgr;
        css::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager > m_xDocCfgMgr;
        css::uno::Reference< ::com::sun::star::container::XNameAccess >      m_xPersistentWindowState;
        LayoutManager*                                                       m_pParentLayouter;

        UIElementVector                                                      m_aUIElements;
        UIElement                                                            m_aDockUIElement;
        Point                                                                m_aStartDockMousePos;
        Rectangle                                                            m_aDockingArea;
        Rectangle                                                            m_aDockingAreaOffsets;
        DockingOperation                                                     m_eDockOperation;
        PreviewFrameDetection                                                m_ePreviewDetection;

        AddonsOptions*                                                       m_pAddonOptions;
        GlobalSettings*                                                      m_pGlobalSettings;

        bool                                                                 m_bComponentAttached;
        bool                                                                 m_bLayoutDirty;
        bool                                                                 m_bStoreWindowState;
        bool                                                                 m_bGlobalSettings;
        bool                                                                 m_bDockingInProgress;
        bool                                                                 m_bVisible;
        bool                                                                 m_bLayoutInProgress;
        bool                                                                 m_bToolbarCreation;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_SOURCE_LAYOUTMANAGER_TOOLBARLAYOUTMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
