/*************************************************************************
 *
 *  $RCSfile: layoutmanager.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 17:49:22 $
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
                          public  drafts::com::sun::star::frame::XMenuBarMergingAcceptor,
                          // base classes
                          // Order is neccessary for right initialization!
                          private ThreadHelpBase                        ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                          public  ::cppu::OBroadcastHelper              ,
                          public  ::cppu::OPropertySetHelper            ,   // => XPropertySet / XFastPropertySet / XMultiPropertySet
                          public  ::cppu::OWeakObject                       // => XWeak, XInterface
    {
        public:
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
            virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XUIElement > SAL_CALL getElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XUIElement > > SAL_CALL getElements(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL showElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL hideElement( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL dockWindow( const ::rtl::OUString& aName, ::drafts::com::sun::star::ui::DockingArea DockingArea, const ::com::sun::star::awt::Point& Pos ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL floatWindow( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementSize( const ::rtl::OUString& aName, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementPos( const ::rtl::OUString& aName, const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setElementPosSize( const ::rtl::OUString& aName, const ::com::sun::star::awt::Point& aPos, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementVisible( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementFloating( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isElementDocked( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
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

            DECL_LINK( MenuBarClose, MenuBar * );

        private:

            //---------------------------------------------------------------------------------------------------------
            //  helper
            //---------------------------------------------------------------------------------------------------------
            void impl_clearUpMenuBar();
            void implts_reset( sal_Bool bAttach );
            void implts_clearMenuBarCloser();
            void implts_setMenuBarCloser( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& StatusListener );
            void implts_updateMenuBarClose();
            void implts_updateUIElementsVisibleState( sal_Bool bShow );
            sal_Bool impl_parseResourceURL( const rtl::OUString aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName );

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

        private:
            struct UIElement
            {
                UIElement( const rtl::OUString& rName, const rtl::OUString& rType, const com::sun::star::uno::Reference< drafts::com::sun::star::ui::XUIElement >& rUIElement ) :
                    aName( rName ), aType( rType ), xUIElement( rUIElement ) {}

                sal_Bool                                                                 bMasterHide; // Hidden because layout manager is non-visible
                rtl::OUString                                                            aName;
                rtl::OUString                                                            aType;
                com::sun::star::uno::Reference< drafts::com::sun::star::ui::XUIElement > xUIElement;
            };

            typedef std::vector< UIElement > UIElementVector;

            css::uno::Reference< css::lang::XMultiServiceFactory >                      m_xSMGR; /** reference to factory, which has created this instance. */
            css::uno::Reference< css::util::XURLTransformer >                           m_xURLTransformer;
            css::uno::Reference< css::frame::XFrame >                                   m_xFrame;
            css::uno::Reference< drafts::com::sun::star::ui::XUIConfigurationManager >  m_xModuleCfgMgr;
            css::uno::Reference< drafts::com::sun::star::ui::XUIConfigurationManager >  m_xDocCfgMgr;
            css::uno::WeakReference< css::frame::XModel >                               m_xModel;
            css::uno::Reference< css::awt::XWindow >                                    m_xContainerWindow;
            css::uno::Reference< css::awt::XWindow >                                    m_xTopDockingWindow;
            sal_Int32                                                                   m_nLockCount;
            UIElementVector                                                             m_aUIElements;
            sal_Bool                                                                    m_bActive : 1,
                                                                                        m_bInplaceMenuSet : 1,
                                                                                        m_bVisible : 1,
                                                                                        m_bMenuVisible : 1;
            css::awt::Rectangle                                                         m_aDockingArea;
            css::uno::Reference< ::drafts::com::sun::star::ui::XDockingAreaAcceptor >   m_xDockingAreaAcceptor;
            css::uno::Reference< ::com::sun::star::lang::XComponent >                   m_xInplaceMenuBar;
            MenuBarManager*                                                             m_pInplaceMenuBar;
            css::uno::Reference< ::drafts::com::sun::star::ui::XUIElement >             m_xMenuBar;
            css::uno::Reference< ::drafts::com::sun::star::frame::XModuleManager >      m_xModuleManager;
            css::uno::Reference< ::drafts::com::sun::star::ui::XUIElementFactory >      m_xUIElementFactoryManager;
            css::uno::Reference< ::com::sun::star::frame::XStatusListener >             m_xMenuBarCloseListener;
    };

} // namespace framework

#endif // __FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
