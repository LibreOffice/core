/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: menubarmanager.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 12:04:08 $
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

#ifndef __FRAMEWORK_UIELEMENT_MENUBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_MENUBARMANAGER_HXX_

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
#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FEATURESTATEEVENT_HPP_
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XPOPUPMENUCONTROLLER_HPP_
#include <com/sun/star/frame/XPopupMenuController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSYSTEMDEPENDENTMENUPEER_HPP_
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XUICONTROLLERREGISTRATION_HPP_
#include <com/sun/star/frame/XUIControllerRegistration.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONLISTENER_HPP_
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XIMAGEMANAGER_HPP_
#include <com/sun/star/ui/XImageManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XACCELERATORCONFIGURATION_HPP_
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _SV_ACCEL_HXX
#include <vcl/accel.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXMENU_HXX_
#include <toolkit/awt/vclxmenu.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

// #110897#
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace framework
{

class BmkMenu;
class AddonMenu;
class AddonPopupMenu;
class MenuBarManager : public com::sun::star::frame::XStatusListener                ,
                       public com::sun::star::frame::XFrameActionListener           ,
                       public com::sun::star::ui::XUIConfigurationListener          ,
                       public com::sun::star::lang::XComponent                      ,
                       public com::sun::star::awt::XSystemDependentMenuPeer         ,
                       public ThreadHelpBase                                        ,
                       public ::cppu::OWeakObject
{
    protected:
        // #110897#
        MenuBarManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
            AddonMenu*          pAddonMenu,
            sal_Bool            bDelete,
            sal_Bool            bDeleteChildren );

        // #110897#
        MenuBarManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
            AddonPopupMenu*     pAddonMenu,
            sal_Bool            bDelete,
            sal_Bool            bDeleteChildren );

    public:
        // #110897#
        MenuBarManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
            const rtl::OUString& aModuleIdentifier,
            Menu* pMenu,
            sal_Bool bDelete,
            sal_Bool bDeleteChildren );

        // #110897#
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& getServiceFactory();

        virtual ~MenuBarManager();

        // XInterface
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw( ::com::sun::star::uno::RuntimeException );

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XFrameActionListener
        virtual void SAL_CALL frameAction( const com::sun::star::frame::FrameActionEvent& Action ) throw ( ::com::sun::star::uno::RuntimeException );

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XUIConfigurationListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

        // XSystemDependentMenuPeer
        virtual ::com::sun::star::uno::Any SAL_CALL getMenuHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw (::com::sun::star::uno::RuntimeException);

        DECL_LINK( Select, Menu * );

        Menu*   GetMenuBar() const { return m_pVCLMenu; }

        // Configuration methods
        static void FillMenuWithConfiguration( USHORT& nId, Menu* pMenu,
                                               const ::rtl::OUString& rModuleIdentifier,
                                               const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rItemContainer,
                                               const ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >& rTransformer );
        static void FillMenu( USHORT& nId, Menu* pMenu, const ::rtl::OUString& rModuleIdentifier, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rItemContainer );
        void FillMenuManager( Menu* pMenu, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, const rtl::OUString& rModuleIdentifier, sal_Bool bDelete, sal_Bool bDeleteChildren );
        void SetItemContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rItemContainer );

    protected:
        DECL_LINK( Highlight, Menu * );
        DECL_LINK( Activate, Menu * );
        DECL_LINK( Deactivate, Menu * );

        void RemoveListener();
        void RequestImages();
        void RetrieveImageManagers();
        static sal_Bool MustBeHidden( PopupMenu* pPopupMenu, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >& rTransformer );

    private:
        String RetrieveLabelFromCommand( const String& aCmdURL );
        void UpdateSpecialFileMenu( Menu* pMenu );
        void UpdateSpecialWindowMenu( Menu* pMenu );
        void Destroy();

        struct MenuItemHandler
        {
            MenuItemHandler( USHORT             aItemId,
                             ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xManager,
                             ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& rDispatch ) :
                             nItemId( aItemId ),
                             bCheckHide( sal_True ),
                             xSubMenuManager( xManager ),
                             xMenuItemDispatch( rDispatch ) {}

            USHORT                                                                                      nItemId;
            sal_Bool                                                                                    bCheckHide;
            ::rtl::OUString                                                                             aTargetFrame;
            ::rtl::OUString                                                                             aMenuItemURL;
            ::rtl::OUString                                                                             aFilter;
            ::rtl::OUString                                                                             aPassword;
            ::rtl::OUString                                                                             aTitle;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >                xSubMenuManager;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >                      xMenuItemDispatch;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XPopupMenuController >           xPopupMenuController;
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >                       xPopupMenu;
            KeyCode                                                                                     aKeyCode;
        };

        void             RetrieveShortcuts( std::vector< MenuItemHandler* >& aMenuShortCuts );
        void             CreatePicklistArguments(
                            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgsList,
                            const MenuItemHandler* );
        void             CheckAndAddMenuExtension( Menu* pMenu );
        static void      impl_RetrieveShortcutsFromConfiguration( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >& rAccelCfg,
                                                                  const ::com::sun::star::uno::Sequence< rtl::OUString >& rCommands,
                                                                  std::vector< MenuItemHandler* >& aMenuShortCuts );

        MenuItemHandler* GetMenuItemHandler( USHORT nItemId );
        sal_Bool         CreatePopupMenuController( MenuItemHandler* pMenuItemHandler );

        sal_Bool                                                                                       m_bDisposed : 1,
                                                                                                       m_bInitialized : 1,
                                                                                                       m_bDeleteMenu : 1,
                                                                                                       m_bDeleteChildren : 1,
                                                                                                       m_bActive : 1,
                                                                                                       m_bIsBookmarkMenu : 1,
                                                                                                       m_bWasHiContrast : 1,
                                                                                                       m_bShowMenuImages : 1;
        sal_Bool                                                                                       m_bModuleIdentified : 1,
                                                                                                       m_bRetrieveImages : 1,
                                                                                                       m_bAcceleratorCfg : 1;
        ::rtl::OUString                                                                                m_aMenuItemCommand;
        ::rtl::OUString                                                                                m_aModuleIdentifier;
        Menu*                                                                                          m_pVCLMenu;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                            m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >                   m_xUICommandLabels;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUIControllerRegistration >         m_xPopupMenuControllerRegistration;
        ::std::vector< MenuItemHandler* >                                                              m_aMenuItemHandlerVector;
        ::cppu::OMultiTypeInterfaceContainerHelper                                                     m_aListenerContainer;   /// container for ALL Listener
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >                        m_xDocImageManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >                        m_xModuleImageManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >            m_xDocAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >            m_xModuleAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >            m_xGlobalAcceleratorManager;
        // #110897#
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&        mxServiceFactory;
};

} // namespace

#endif
