/*************************************************************************
 *
 *  $RCSfile: layoutmanager.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-20 16:41:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <drafts/com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManager.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATION_HPP_
#include <drafts/com/sun/star/ui/XUIConfiguration.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
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

#ifndef _DRAFTS_COM_SUN_STAR_UI_XUIELEMENTFACTORY_HPP_
#include <drafts/com/sun/star/ui/XUIElementFactory.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XINPLACELAYOUT_HPP_
#include <drafts/com/sun/star/frame/XInplaceLayout.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_DOCKINGAREA_HPP_
#include <drafts/com/sun/star/ui/DockingArea.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDOCKABLEWINDOW_HPP_
#include <com/sun/star/awt/XDockableWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDOCKABLEWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XDockableWindowListener.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMENUBARMERGINGACCEPTOR_HPP_
#include <drafts/com/sun/star/frame/XMenuBarMergingAcceptor.hpp>
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
    class LayoutManager : public  css::lang::XTypeProvider                              ,
                          public  css::lang::XServiceInfo                               ,
                          public  drafts::com::sun::star::frame::XLayoutManager         ,
                          public  css::awt::XWindowListener                             ,
                          public  css::frame::XFrameActionListener                      ,
                          public  drafts::com::sun::star::ui::XUIConfigurationListener  ,
                          public  drafts::com::sun::star::frame::XInplaceLayout         ,
                          public  css::awt::XDockableWindowListener                     ,
                          public  drafts::com::sun::star::frame::XMenuBarMergingAcceptor,
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
            DECLARE_XINTERFACE
            DECLARE_XTYPEPROVIDER
            DECLARE_XSERVICEINFO

            //---------------------------------------------------------------------------------------------------------
            // XLayoutManager
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL attachFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& Frame ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL reset(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::awt::Rectangle SAL_CALL getCurrentDockingArea(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XDockingAreaAcceptor > SAL_CALL getDockingAreaAcceptor() throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setDockingAreaAcceptor( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XDockingAreaAcceptor >& xDockingAreaAcceptor ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL createElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL destroyElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL requestElement( const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XUIElement > SAL_CALL getElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XUIElement > > SAL_CALL getElements(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL showElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hideElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL dockWindow( const ::rtl::OUString& aName, ::drafts::com::sun::star::ui::DockingArea DockingArea, const ::com::sun::star::awt::Point& Pos ) throw (::com::sun::star::uno::RuntimeException);
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
            virtual sal_Bool SAL_CALL setMergeMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& ContainerMenuBar,
                                                       const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& ContainerDispatchProvider,
                                                       const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& EmbedObjectMenuBar,
                                                       const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& EmbedObjectDispatchProvider )
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
            virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException );

            //---------------------------------------------------------------------------------------------------------
            //  XUIConfigurationListener
            //---------------------------------------------------------------------------------------------------------
            virtual void SAL_CALL elementInserted( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL elementRemoved( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL elementReplaced( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

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

            DECL_LINK( MenuBarClose, MenuBar * );

            struct DockedData
            {
                DockedData() : m_aPos( LONG_MAX, LONG_MAX ),
                               m_nDockedArea( ::drafts::com::sun::star::ui::DockingArea_DOCKINGAREA_TOP ),
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
                sal_Int32                                                                         nVarSize;
                sal_Int32                                                                         nStaticSize;
                sal_Int32                                                                         nSpace;
                sal_Int32                                                                         nRowColumn;
            };

        protected:
            DECL_LINK( AsyncLayoutHdl, Timer * );

        private:
            struct UIElement
            {
                UIElement() {}
                UIElement( const rtl::OUString& rName,
                           const rtl::OUString& rType,
                           const com::sun::star::uno::Reference< drafts::com::sun::star::ui::XUIElement >& rUIElement,
                           sal_Bool bFloating = sal_False
                           ) : m_aName( rName ),
                               m_aType( rType ),
                               m_xUIElement( rUIElement ),
                               m_bFloating( bFloating ),
                               m_bVisible( sal_True ),
                               m_bUserActive( sal_False ),
                               m_bCreateNewRowCol0( sal_False ),
                               m_bDeactiveHide( sal_False ),
                               m_bMasterHide( sal_False ),
                               m_nStyle( BUTTON_SYMBOL ) {}

                bool operator< ( const UIElement& aUIElement ) const;

                rtl::OUString                                                            m_aType;
                rtl::OUString                                                            m_aName;
                rtl::OUString                                                            m_aUIName;
                com::sun::star::uno::Reference< drafts::com::sun::star::ui::XUIElement > m_xUIElement;
                sal_Bool                                                                 m_bFloating : 1,
                                                                                         m_bVisible : 1,
                                                                                         m_bUserActive : 1,
                                                                                         m_bCreateNewRowCol0 : 1,
                                                                                         m_bDeactiveHide : 1,
                                                                                         m_bMasterHide : 1;
                sal_Int16                                                                m_nStyle;
                DockedData                                                               m_aDockedData;
                FloatingData                                                             m_aFloatingData;
            };

            typedef std::vector< UIElement > UIElementVector;

            //---------------------------------------------------------------------------------------------------------
            //  helper
            //---------------------------------------------------------------------------------------------------------
            void impl_clearUpMenuBar();
            void implts_reset( sal_Bool bAttach );
            void implts_clearMenuBarCloser();
            void implts_setMenuBarCloser( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& StatusListener );
            void implts_updateMenuBarClose();

            sal_Bool implts_findElement( const rtl::OUString& aName, rtl::OUString& aElementType, rtl::OUString& aElementName, ::com::sun::star::uno::Reference< drafts::com::sun::star::ui::XUIElement >& xSettings );
            sal_Bool implts_findElement( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xUIElement, UIElement& aElementData );
            sal_Bool implts_findElement( const rtl::OUString& aName, UIElement& aElementData );
            UIElement& impl_findElement( const rtl::OUString& aName );

            void implts_writeNewStateData( const rtl::OUString aName, const ::com::sun::star::uno::Reference< com::sun::star::awt::XWindow >& xWindow );
            sal_Bool implts_readWindowStateData( const rtl::OUString& rName, UIElement& rElementData );
            void implts_writeWindowStateData( const rtl::OUString& rName, const UIElement& rElementData );
            void implts_setElementData( UIElement& rUIElement, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindow >& rDockWindow );
            void implts_sortUIElements();
            void implts_destroyElements();
            void implts_createAddonsToolBars();
            void implts_createCustomToolBars();
            void implts_createCustomToolBars( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& aCustomTbxSeq );
            void implts_createCustomToolBar( const rtl::OUString& aTbxResName, const rtl::OUString& aTitle );
            void implts_toggleFloatingUIElementsVisibility( sal_Bool bActive );
            sal_Int16 implts_getCurrentSymbolSet();
            ::com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer > implts_createToolkitWindow( const ::com::sun::star::uno::Reference< com::sun::star::awt::XWindowPeer >& rParent );
            ::com::sun::star::uno::Reference< drafts::com::sun::star::ui::XUIElement > implts_createElement( const rtl::OUString& aName );

            // docking methods
            ::Rectangle implts_calcHotZoneRect( const ::Rectangle& rRect, sal_Int32 nHotZoneOffset );
            ::Rectangle implts_calcDockingPosSize( UIElement& aUIElement, const ::Rectangle& rTrackingRect, const Point& rMousePos );
            ::Point     implts_convertVirtualToPhysicalPos( ::drafts::com::sun::star::ui::DockingArea DockingArea, const ::Point& aPoint ) const;

            // layouting methods
            sal_Bool implts_compareRectangles( const ::com::sun::star::awt::Rectangle& rRect1, const ::com::sun::star::awt::Rectangle& rRect2 );
            ::Size  implts_getTopBottomDockingAreaSizes();
            ::Size  implts_getContainerWindowOutputSize();
            void    implts_getDockingAreaElementInfos( ::drafts::com::sun::star::ui::DockingArea DockingArea, std::vector< SingleRowColumnWindowData >& rRowColumnsWindowData );
            ::Point implts_findNextCascadeFloatingPos();
            void    implts_findNextDockingPos( ::drafts::com::sun::star::ui::DockingArea DockingArea, const ::Size& aUIElementSize, ::Point& rVirtualPos, ::Point& rPixelPos );
            void    implts_sortActiveElement( const UIElement& aElementData );
            ::com::sun::star::awt::Rectangle implts_calcDockingAreaSizes();
            void    implts_setDockingAreaWindowSizes( const com::sun::star::awt::Rectangle& rBorderSpace );
            void    implts_calcWindowPosSizeOnSingleRowColumn( sal_Int32 nDockingArea,
                                                               sal_Int32 nOffset,
                                                               SingleRowColumnWindowData& rRowColumnWindowData,
                                                               const ::Size& rContainerSize );
            void    implts_doLayout( sal_Bool bForceRequestBorderSpace );

            void implts_updateUIElementsVisibleState( sal_Bool bShow );
            sal_Bool impl_parseResourceURL( const rtl::OUString aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName );

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
            virtual void                                                SAL_CALL getFastPropertyValue( com::sun::star::uno::Any&    aValue          ,
                                                                                                    sal_Int32                    nHandle         ) const;
            virtual ::cppu::IPropertyArrayHelper&                       SAL_CALL getInfoHelper();
            virtual ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException);

            static const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > impl_getStaticPropertyDescriptor();

            css::uno::Reference< css::lang::XMultiServiceFactory >                      m_xSMGR; /** reference to factory, which has created this instance. */
            css::uno::Reference< css::util::XURLTransformer >                           m_xURLTransformer;
            css::uno::Reference< css::frame::XFrame >                                   m_xFrame;
            css::uno::Reference< drafts::com::sun::star::ui::XUIConfigurationManager >  m_xModuleCfgMgr;
            css::uno::Reference< drafts::com::sun::star::ui::XUIConfigurationManager >  m_xDocCfgMgr;
            css::uno::WeakReference< css::frame::XModel >                               m_xModel;
            css::uno::Reference< css::awt::XWindow >                                    m_xContainerWindow;
            css::uno::Reference< css::awt::XWindow >                                    m_xDockAreaWindows[DOCKINGAREAS_COUNT];
            sal_Int32                                                                   m_nLockCount;
            UIElementVector                                                             m_aUIElements;
            sal_Bool                                                                    m_bActive : 1,
                                                                                        m_bInplaceMenuSet : 1,
                                                                                        m_bDockingInProgress : 1,
                                                                                        m_bMenuVisible : 1,
                                                                                        m_bComponentAttached : 1,
                                                                                        m_bDoLayout : 1,
                                                                                        m_bVisible : 1;
            UIElement                                                                   m_aDockUIElement;
            css::awt::Rectangle                                                         m_aDockingArea;
            css::uno::Reference< ::drafts::com::sun::star::ui::XDockingAreaAcceptor >   m_xDockingAreaAcceptor;
            Point                                                                       m_aStartDockMousePos;
            css::uno::Reference< ::com::sun::star::lang::XComponent >                   m_xInplaceMenuBar;
            MenuBarManager*                                                             m_pInplaceMenuBar;
            css::uno::Reference< ::drafts::com::sun::star::ui::XUIElement >             m_xMenuBar;
            css::uno::Reference< ::drafts::com::sun::star::frame::XModuleManager >      m_xModuleManager;
            css::uno::Reference< ::drafts::com::sun::star::ui::XUIElementFactory >      m_xUIElementFactoryManager;
            css::uno::Reference< ::com::sun::star::frame::XStatusListener >             m_xMenuBarCloseListener;
            css::uno::Reference< ::com::sun::star::container::XNameAccess >             m_xPersistentWindowState;
            css::uno::Reference< ::com::sun::star::container::XNameAccess >             m_xPersistentWindowStateSupplier;
            rtl::OUString                                                               m_aModuleIdentifier;
            rtl::OUString                                                               m_aCustomTbxPrefix;
            AddonsOptions*                                                              m_pAddonOptions;
            SvtMiscOptions*                                                             m_pMiscOptions;
            sal_Int16                                                                   m_eSymbolSet;
            Timer                                                                       m_aAsyncLayoutTimer;
    };

} // namespace framework

#endif // __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
