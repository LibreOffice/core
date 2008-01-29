/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layoutmanager.hxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:22:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
#define __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef __FRAMEWORK_PROPERTIES_H_
#include <properties.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef __FRAMEWORK_UIELEMENT_MENUBARMANAGER_HXX_
#include <uielement/menubarmanager.hxx>
#endif

#ifndef __FRAMEWORK_UICONFIGURATION_WINDOWSTATECONFIGURATION_HXX_
#include <uiconfiguration/windowstateconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#include <classes/addonsoptions.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATION_HPP_
#include <com/sun/star/ui/XUIConfiguration.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUIELEMENTFACTORY_HPP_
#include <com/sun/star/ui/XUIElementFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XINPLACELAYOUT_HPP_
#include <com/sun/star/frame/XInplaceLayout.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DOCKINGAREA_HPP_
#include <com/sun/star/ui/DockingArea.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDOCKABLEWINDOW_HPP_
#include <com/sun/star/awt/XDockableWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDOCKABLEWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMENUBARMERGINGACCEPTOR_HPP_
#include <com/sun/star/frame/XMenuBarMergingAcceptor.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGEREVENTBROADCASTER_HPP_
#include <com/sun/star/frame/XLayoutManagerEventBroadcaster.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include <svtools/miscopt.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

class MenuBar;
namespace framework
{
    class GlobalSettings;
    class LayoutManager : public  css::lang::XTypeProvider                        ,
                          public  css::lang::XServiceInfo                         ,
                          public  ::com::sun::star::frame::XLayoutManager         ,
                          public  css::awt::XWindowListener                       ,
                          public  css::frame::XFrameActionListener                ,
                          public  ::com::sun::star::ui::XUIConfigurationListener  ,
                          public  ::com::sun::star::frame::XInplaceLayout         ,
                          public  css::awt::XDockableWindowListener               ,
                          public  ::com::sun::star::frame::XMenuBarMergingAcceptor,
                          public  css::frame::XLayoutManagerEventBroadcaster      ,
                          // base classes
                          // Order is neccessary for right initialization!
                          private ThreadHelpBase                        ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                          public  ::cppu::OBroadcastHelper              ,
                          public  ::cppu::OPropertySetHelper            ,   // => XPropertySet / XFastPropertySet / XMultiPropertySet
                          public  ::cppu::OWeakObject                       // => XWeak, XInterface
    {
        public:
            enum { DOCKINGAREAS_COUNT = 4 };

            LayoutManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMGR );
            virtual ~LayoutManager();

            /** declaration of XInterface, XTypeProvider, XServiceInfo */
            FWK_DECLARE_XINTERFACE
            FWK_DECLARE_XTYPEPROVIDER
            DECLARE_XSERVICEINFO

            //---------------------------------------------------------------------------------------------------------
            // XLayoutManager
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL attachFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& Frame ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL reset(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::awt::Rectangle SAL_CALL getCurrentDockingArea(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor > SAL_CALL getDockingAreaAcceptor() throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setDockingAreaAcceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor >& xDockingAreaAcceptor ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL createElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL destroyElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL requestElement( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > SAL_CALL getElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > > SAL_CALL getElements(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL showElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hideElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL dockWindow( const ::rtl::OUString& aName, ::com::sun::star::ui::DockingArea DockingArea, const ::com::sun::star::awt::Point& Pos ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL dockAllWindows( ::sal_Int16 nElementType ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL floatWindow( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL lockWindow( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL unlockWindow( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementSize( const ::rtl::OUString& aName, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementPos( const ::rtl::OUString& aName, const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementPosSize( const ::rtl::OUString& aName, const ::com::sun::star::awt::Point& aPos, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementVisible( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementFloating( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementDocked( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL isElementLocked( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::awt::Size SAL_CALL getElementSize( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::awt::Point SAL_CALL getElementPos( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL lock(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL unlock(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL doLayout(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setVisible( sal_Bool bVisible ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isVisible() throw (::com::sun::star::uno::RuntimeException);

            //---------------------------------------------------------------------------------------------------------
            //  XInplaceLayout
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL setInplaceMenuBar( sal_Int64 pInplaceMenuBarPointer ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL resetInplaceMenuBar(  ) throw (::com::sun::star::uno::RuntimeException);

            //---------------------------------------------------------------------------------------------------------
            // XMenuBarMergingAcceptor
            //---------------------------------------------------------------------------------------------------------
            virtual sal_Bool SAL_CALL setMergedMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMergedMenuBar )
                                                       throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeMergedMenuBar(  ) throw (::com::sun::star::uno::RuntimeException);

            //---------------------------------------------------------------------------------------------------------
            //  XWindowListener
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException );
            virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException );
            virtual void SAL_CALL windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );
            virtual void SAL_CALL windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

            //---------------------------------------------------------------------------------------------------------
            //   XFrameActionListener
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& aEvent ) throw ( css::uno::RuntimeException );

            //---------------------------------------------------------------------------------------------------------
            //  XEventListener
            //---------------------------------------------------------------------------------------------------------
            using cppu::OPropertySetHelper::disposing;
            virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

            //---------------------------------------------------------------------------------------------------------
            //  XUIConfigurationListener
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

            //---------------------------------------------------------------------------------------------------------
            //  XDockableWindowListener
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL startDocking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::awt::DockingData SAL_CALL docking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL endDocking( const ::com::sun::star::awt::EndDockingEvent& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL prepareToggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL toggleFloatingMode( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL closed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& e ) throw (::com::sun::star::uno::RuntimeException);

            //---------------------------------------------------------------------------------------------------------
            //  XLayoutManagerEventBroadcaster
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL addLayoutManagerEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManagerListener >& aLayoutManagerListener ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeLayoutManagerEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManagerListener >& aLayoutManagerListener ) throw (::com::sun::star::uno::RuntimeException);

            DECL_LINK( MenuBarClose, MenuBar * );
            DECL_LINK( WindowEventListener, VclSimpleEvent* );

            struct DockedData
            {
                DockedData() : m_aPos( LONG_MAX, LONG_MAX ),
                               m_nDockedArea( ::com::sun::star::ui::DockingArea_DOCKINGAREA_TOP ),
                               m_bLocked( sal_False ) {}

                Point       m_aPos;
                Size        m_aSize;
                sal_Int16   m_nDockedArea;
                sal_Bool    m_bLocked;
            };
            struct FloatingData
            {
                FloatingData() : m_aPos( LONG_MAX, LONG_MAX ),
                                 m_nLines( 1 ),
                                 m_bIsHorizontal( sal_True ) {}

                Point       m_aPos;
                Size        m_aSize;
                sal_Int16   m_nLines;
                sal_Bool    m_bIsHorizontal;
            };
            struct SingleRowColumnWindowData
            {
                SingleRowColumnWindowData() : nVarSize( 0 ), nStaticSize( 0 ), nSpace( 0 ) {}

                std::vector< rtl::OUString >                                                      aUIElementNames;
                std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > > aRowColumnWindows;
                std::vector< ::com::sun::star::awt::Rectangle >                                   aRowColumnWindowSizes;
                std::vector< sal_Int32 >                                                          aRowColumnSpace;
                ::com::sun::star::awt::Rectangle                                                  aRowColumnRect;
                sal_Int32                                                                         nVarSize;
                sal_Int32                                                                         nStaticSize;
                sal_Int32                                                                         nSpace;
                sal_Int32                                                                         nRowColumn;
            };

        protected:
            DECL_LINK( AsyncLayoutHdl, Timer * );

        private:
            enum DockingOperation
            {
                DOCKOP_BEFORE_COLROW,
                DOCKOP_ON_COLROW,
                DOCKOP_AFTER_COLROW
            };
            struct UIElement
            {
                UIElement() : m_bFloating( sal_False ),
                              m_bVisible( sal_True ),
                              m_bUserActive( sal_False ),
                              m_bCreateNewRowCol0( sal_False ),
                              m_bDeactiveHide( sal_False ),
                              m_bMasterHide( sal_False ),
                              m_bContextSensitive( sal_False ),
                              m_bContextActive( sal_True ),
                              m_bNoClose( sal_False ),
                              m_bSoftClose( sal_False ),
                              m_bStateRead( sal_False ),
                              m_nStyle( BUTTON_SYMBOL )
                              {}

                UIElement( const rtl::OUString& rName,
                           const rtl::OUString& rType,
                           const com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& rUIElement,
                           sal_Bool bFloating = sal_False
                           ) : m_aType( rType ),
                               m_aName( rName ),
                               m_xUIElement( rUIElement ),
                               m_bFloating( bFloating ),
                               m_bVisible( sal_True ),
                               m_bUserActive( sal_False ),
                               m_bCreateNewRowCol0( sal_False ),
                               m_bDeactiveHide( sal_False ),
                               m_bMasterHide( sal_False ),
                               m_bContextSensitive( sal_False ),
                               m_bContextActive( sal_True ),
                               m_bNoClose( sal_False ),
                               m_bSoftClose( sal_False ),
                               m_bStateRead( sal_False ),
                               m_nStyle( BUTTON_SYMBOL ) {}

                bool operator< ( const UIElement& aUIElement ) const;
                UIElement& operator=( const UIElement& rUIElement );

                rtl::OUString                                                            m_aType;
                rtl::OUString                                                            m_aName;
                rtl::OUString                                                            m_aUIName;
                com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >       m_xUIElement;
                sal_Bool                                                                 m_bFloating,
                                                                                         m_bVisible,
                                                                                         m_bUserActive,
                                                                                         m_bCreateNewRowCol0,
                                                                                         m_bDeactiveHide,
                                                                                         m_bMasterHide,
                                                                                         m_bContextSensitive,
                                                                                         m_bContextActive;
                sal_Bool                                                                 m_bNoClose,
                                                                                         m_bSoftClose,
                                                                                         m_bStateRead;
                sal_Int16                                                                m_nStyle;
                DockedData                                                               m_aDockedData;
                FloatingData                                                             m_aFloatingData;
            };

            typedef std::vector< UIElement > UIElementVector;

            //---------------------------------------------------------------------------------------------------------
            //  helper
            //---------------------------------------------------------------------------------------------------------

            //---------------------------------------------------------------------------------------------------------
            //  helper
            //---------------------------------------------------------------------------------------------------------
            void impl_clearUpMenuBar();
            void implts_reset( sal_Bool bAttach );
            void implts_setMenuBarCloser(sal_Bool bCloserState);
            void implts_updateMenuBarClose();
            sal_Bool implts_resetMenuBar();

            void implts_lock();
            sal_Bool implts_unlock();

            sal_Bool implts_findElement( const rtl::OUString& aName, rtl::OUString& aElementType, rtl::OUString& aElementName, ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& xSettings );
            sal_Bool implts_findElement( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xUIElement, UIElement& aElementData );
            sal_Bool implts_findElement( const rtl::OUString& aName, UIElement& aElementData );
            UIElement& impl_findElement( const rtl::OUString& aName );
            sal_Bool implts_insertUIElement( const UIElement& rUIElement );

            void implts_refreshContextToolbarsVisibility();
            void implts_writeNewStateData( const rtl::OUString aName, const ::com::sun::star::uno::Reference< com::sun::star::awt::XWindow >& xWindow );
            sal_Bool implts_readWindowStateData( const rtl::OUString& rName, UIElement& rElementData );
            void implts_writeWindowStateData( const rtl::OUString& rName, const UIElement& rElementData );
            void implts_setElementData( UIElement& rUIElement, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindow >& rDockWindow );
            void implts_sortUIElements();
            void implts_destroyElements();
            void implts_destroyDockingAreaWindows();
            void implts_createAddonsToolBars();
            void implts_createCustomToolBars();
            void implts_createNonContextSensitiveToolBars();
            void implts_createCustomToolBars( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& aCustomTbxSeq );
            void implts_createCustomToolBar( const rtl::OUString& aTbxResName, const rtl::OUString& aTitle );
            void implts_toggleFloatingUIElementsVisibility( sal_Bool bActive );
            void implts_reparentChildWindows();

            sal_Bool implts_isEmbeddedLayoutManager() const;
            sal_Int16 implts_getCurrentSymbolsSize();
            sal_Int16 implts_getCurrentSymbolsStyle();
            ::com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer > implts_createToolkitWindow( const ::com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer >& rParent );
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > implts_createElement( const rtl::OUString& aName );
            rtl::OUString implts_generateGenericAddonToolbarTitle( sal_Int32 nNumber ) const;

            // docking methods
            ::Rectangle      implts_calcHotZoneRect( const ::Rectangle& rRect, sal_Int32 nHotZoneOffset );
            void             implts_calcDockingPosSize( UIElement& aUIElement, DockingOperation& eDockOperation, ::Rectangle& rTrackingRect, const Point& rMousePos );
            DockingOperation implts_determineDockingOperation( ::com::sun::star::ui::DockingArea DockingArea, const ::Rectangle& rRowColRect, const Point& rMousePos );
            ::Rectangle      implts_getWindowRectFromRowColumn( ::com::sun::star::ui::DockingArea DockingArea, const SingleRowColumnWindowData& rRowColumnWindowData, const ::Point& rMousePos, const rtl::OUString& rExcludeElementName );
            ::Rectangle      implts_determineFrontDockingRect( ::com::sun::star::ui::DockingArea eDockingArea,
                                                               sal_Int32 nRowCol,
                                                               const ::Rectangle& rDockedElementRect,
                                                               const ::rtl::OUString& rMovedElementName,
                                                               const ::Rectangle& rMovedElementRect );
            void             implts_calcWindowPosSizeOnSingleRowColumn( sal_Int32 nDockingArea,
                                                                        sal_Int32 nOffset,
                                                                        SingleRowColumnWindowData& rRowColumnWindowData,
                                                                        const ::Size& rContainerSize );
            ::Rectangle      implts_calcTrackingAndElementRect( ::com::sun::star::ui::DockingArea eDockingArea,
                                                                sal_Int32 nRowCol,
                                                                UIElement& rUIElement,
                                                                const ::Rectangle& rTrackingRect,
                                                                const ::Rectangle& rRowColumnRect,
                                                                const ::Size& rContainerWinSize );
            void             implts_renumberRowColumnData( ::com::sun::star::ui::DockingArea eDockingArea, DockingOperation eDockingOperation, const UIElement& rUIElement );

            // layouting methods
            sal_Bool implts_compareRectangles( const ::com::sun::star::awt::Rectangle& rRect1, const ::com::sun::star::awt::Rectangle& rRect2 );
            ::Size  implts_getTopBottomDockingAreaSizes();
            ::Size  implts_getContainerWindowOutputSize();
            ::com::sun::star::awt::Rectangle implts_getDockingAreaWindowSizes();
            void    implts_getDockingAreaElementInfos( ::com::sun::star::ui::DockingArea DockingArea, std::vector< SingleRowColumnWindowData >& rRowColumnsWindowData );
            void    implts_getDockingAreaElementInfoOnSingleRowCol( ::com::sun::star::ui::DockingArea,
                                                                     sal_Int32 nRowCol,
                                                                     SingleRowColumnWindowData& rRowColumnWindowData );
            ::Point implts_findNextCascadeFloatingPos();
            void    implts_findNextDockingPos( ::com::sun::star::ui::DockingArea DockingArea, const ::Size& aUIElementSize, ::Point& rVirtualPos, ::Point& rPixelPos );
            void    implts_sortActiveElement( const UIElement& aElementData );
            ::com::sun::star::awt::Rectangle implts_calcDockingAreaSizes();
            void    implts_setDockingAreaWindowSizes( const com::sun::star::awt::Rectangle& rBorderSpace );
            sal_Bool implts_doLayout( sal_Bool bForceRequestBorderSpace );

            // internal methods to control status/progress bar
            ::Size      implts_getStatusBarSize();
            void        implts_destroyStatusBar();
            void        implts_createStatusBar( const rtl::OUString& rStatusBarName );
            void        implts_createProgressBar();
            void        implts_destroyProgressBar();
            void        implts_setStatusBarPosSize( const ::Point& rPos, const ::Size& rSize );
            sal_Bool    implts_showStatusBar( sal_Bool bStoreState=sal_False );
            sal_Bool    implts_hideStatusBar( sal_Bool bStoreState=sal_False );
            void        implts_readStatusBarState( const rtl::OUString& rStatusBarName );
            sal_Bool    implts_showProgressBar();
            sal_Bool    implts_hideProgressBar();
            void        implts_backupProgressBarWrapper();

            void    implts_setInplaceMenuBar(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xMergedMenuBar )
                            throw (::com::sun::star::uno::RuntimeException);
            void    implts_resetInplaceMenuBar()
                            throw (::com::sun::star::uno::RuntimeException);

            void    implts_setVisibleState( sal_Bool bShow );
            void    implts_updateUIElementsVisibleState( sal_Bool bShow );
            void    implts_setCurrentUIVisibility( sal_Bool bShow );
            sal_Bool impl_parseResourceURL( const rtl::OUString aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName );

            void    implts_notifyListeners( short nEvent, ::com::sun::star::uno::Any aInfoParam );

            void    implts_checkElementContainer();

            DECL_LINK( OptionsChanged, void* );
            DECL_LINK( SettingsChanged, void* );

            //---------------------------------------------------------------------------------------------------------
            //  OPropertySetHelper
            //---------------------------------------------------------------------------------------------------------
            virtual sal_Bool                                            SAL_CALL convertFastPropertyValue        ( com::sun::star::uno::Any&        aConvertedValue ,
                                                                                                                com::sun::star::uno::Any&        aOldValue       ,
                                                                                                                sal_Int32                        nHandle         ,
                                                                                                                const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException );
            virtual void                                                SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32                        nHandle         ,
                                                                                                                const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::uno::Exception                 );
            using cppu::OPropertySetHelper::getFastPropertyValue;
            virtual void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                    sal_Int32                    nHandle         ) const;
            virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper();
            virtual ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException);

            static const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

            css::uno::Reference< css::lang::XMultiServiceFactory >                      m_xSMGR; /** reference to factory, which has created this instance. */
            css::uno::Reference< css::util::XURLTransformer >                           m_xURLTransformer;
            css::uno::Reference< css::frame::XFrame >                                   m_xFrame;
            css::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >        m_xModuleCfgMgr;
            css::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >        m_xDocCfgMgr;
            css::uno::WeakReference< css::frame::XModel >                               m_xModel;
            css::uno::Reference< css::awt::XWindow >                                    m_xContainerWindow;
            css::uno::Reference< css::awt::XWindow >                                    m_xDockAreaWindows[DOCKINGAREAS_COUNT];
            sal_Int32                                                                   m_nLockCount;
            UIElementVector                                                             m_aUIElements;
            bool                                                                        m_bActive;
            bool                                                                        m_bInplaceMenuSet;
            bool                                                                        m_bDockingInProgress;
            bool                                                                        m_bMenuVisible;
            bool                                                                        m_bComponentAttached;
            bool                                                                        m_bDoLayout;
            bool                                                                        m_bVisible;
            bool                                                                        m_bParentWindowVisible;
            bool                                                                        m_bMustDoLayout;
            bool                                                                        m_bAutomaticToolbars;
            bool                                                                        m_bStoreWindowState;
            bool                                                                        m_bHideCurrentUI;
            bool                                                                        m_bGlobalSettings;
            DockingOperation                                                            m_eDockOperation;
            UIElement                                                                   m_aDockUIElement;
            css::awt::Rectangle                                                         m_aDockingArea;
            css::uno::Reference< ::com::sun::star::ui::XDockingAreaAcceptor >           m_xDockingAreaAcceptor;
            Point                                                                       m_aStartDockMousePos;
            css::uno::Reference< ::com::sun::star::lang::XComponent >                   m_xInplaceMenuBar;
            MenuBarManager*                                                             m_pInplaceMenuBar;
            css::uno::Reference< ::com::sun::star::ui::XUIElement >                     m_xMenuBar;
            UIElement                                                                   m_aStatusBarElement;
            UIElement                                                                   m_aProgressBarElement;
            com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >          m_xProgressBarBackup;
            css::uno::Reference< ::com::sun::star::frame::XModuleManager >              m_xModuleManager;
            css::uno::Reference< ::com::sun::star::ui::XUIElementFactory >              m_xUIElementFactoryManager;
            bool                                                                        m_bMenuBarCloser;
            css::uno::Reference< ::com::sun::star::container::XNameAccess >             m_xPersistentWindowState;
            css::uno::Reference< ::com::sun::star::container::XNameAccess >             m_xPersistentWindowStateSupplier;
            GlobalSettings*                                                             m_pGlobalSettings;
            rtl::OUString                                                               m_aModuleIdentifier;
            rtl::OUString                                                               m_aCustomTbxPrefix;
            rtl::OUString                                                               m_aFullCustomTbxPrefix;
            rtl::OUString                                                               m_aFullAddonTbxPrefix;
            rtl::OUString                                                               m_aStatusBarAlias;
            rtl::OUString                                                               m_aProgressBarAlias;
            rtl::OUString                                                               m_aPropDocked;
            rtl::OUString                                                               m_aPropVisible;
            rtl::OUString                                                               m_aPropDockingArea;
            rtl::OUString                                                               m_aPropDockPos;
            rtl::OUString                                                               m_aPropPos;
            rtl::OUString                                                               m_aPropSize;
            rtl::OUString                                                               m_aPropUIName;
            rtl::OUString                                                               m_aPropStyle;
            rtl::OUString                                                               m_aPropLocked;
            rtl::OUString                                                               m_aCustomizeCmd;
            AddonsOptions*                                                              m_pAddonOptions;
            SvtMiscOptions*                                                             m_pMiscOptions;
            sal_Int16                                                                   m_eSymbolsSize;
            sal_Int16                                                                   m_eSymbolsStyle;
            Timer                                                                       m_aAsyncLayoutTimer;
            ::cppu::OMultiTypeInterfaceContainerHelper                                  m_aListenerContainer; // container for ALL Listener
    };

} // namespace framework

#endif // __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
