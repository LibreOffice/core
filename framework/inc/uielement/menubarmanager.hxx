/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_UIELEMENT_MENUBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_MENUBARMANAGER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>
#include <vcl/accel.hxx>
#include <vcl/timer.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <framework/addonsoptions.hxx>

namespace framework
{

struct PopupControllerEntry
{
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XDispatchProvider > m_xDispatchProvider;
};

typedef std::hash_map< rtl::OUString, PopupControllerEntry, rtl::OUStringHash, ::std::equal_to< rtl::OUString > > PopupControllerCache;

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
            const ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >& _xURLTransformer,
            AddonMenu*          pAddonMenu,
            sal_Bool            bDelete,
            sal_Bool            bDeleteChildren );

        // #110897#
        MenuBarManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >& _xURLTransformer,
            AddonPopupMenu*     pAddonMenu,
            sal_Bool            bDelete,
            sal_Bool            bDeleteChildren );

    public:
        // #110897#
        MenuBarManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >& _xURLTransformer,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider,
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
        static void FillMenuWithConfiguration( sal_uInt16& nId, Menu* pMenu,
                                               const ::rtl::OUString& rModuleIdentifier,
                                               const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rItemContainer,
                                               const ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >& rTransformer );
        static void FillMenu( sal_uInt16& nId,
                              Menu* pMenu,
                              const ::rtl::OUString& rModuleIdentifier,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rItemContainer,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider );

        void FillMenuManager( Menu* pMenu,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider,
                              const rtl::OUString& rModuleIdentifier,
                              sal_Bool bDelete,
                              sal_Bool bDeleteChildren );
        void SetItemContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rItemContainer );
        void GetPopupController( PopupControllerCache& rPopupController );

    protected:
        DECL_LINK( Highlight, Menu * );
        DECL_LINK( Activate, Menu * );
        DECL_LINK( Deactivate, Menu * );
        DECL_LINK( AsyncSettingsHdl, Timer * );

        void RemoveListener();
        void RequestImages();
        void RetrieveImageManagers();
        static sal_Bool MustBeHidden( PopupMenu* pPopupMenu, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >& rTransformer );
        String RetrieveLabelFromCommand( const String& aCmdURL );

    private:



        void Destroy();

        struct MenuItemHandler
        {
            MenuItemHandler( sal_uInt16             aItemId,
                             ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xManager,
                             ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& rDispatch ) :
                             nItemId( aItemId ),
                             bCheckHide( sal_True ),
                             xSubMenuManager( xManager ),
                             xMenuItemDispatch( rDispatch ) {}

            sal_uInt16                                                                                      nItemId;
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
        static void      MergeAddonMenus( Menu* pMenuBar, const MergeMenuInstructionContainer&, const ::rtl::OUString& aModuleIdentifier );

        MenuItemHandler* GetMenuItemHandler( sal_uInt16 nItemId );
        sal_Bool         CreatePopupMenuController( MenuItemHandler* pMenuItemHandler );
        void             AddMenu(MenuBarManager* pSubMenuManager,const ::rtl::OUString& _sItemCommand,sal_uInt16 _nItemId);
        sal_uInt16           FillItemCommand(::rtl::OUString& _rItemCommand,Menu* _pMenu,sal_uInt16 _nIndex) const;
        void             Init(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,AddonMenu* pAddonMenu,sal_Bool bDelete,sal_Bool bDeleteChildren,bool _bHandlePopUp = false);
        void             SetHdl();

        sal_Bool                                                                               m_bDisposed : 1,
                                                                                               m_bInitialized : 1,
                                                                                               m_bDeleteMenu : 1,
                                                                                               m_bDeleteChildren : 1,
                                                                                               m_bActive : 1,
                                                                                               m_bIsBookmarkMenu : 1,
                                                                                               m_bWasHiContrast : 1,
                                                                                               m_bShowMenuImages : 1;
        sal_Bool                                                                               m_bRetrieveImages : 1,
                                                                                               m_bAcceleratorCfg : 1;
        sal_Bool                                                                               m_bModuleIdentified;
        ::rtl::OUString                                                                        m_aMenuItemCommand;
        ::rtl::OUString                                                                        m_aModuleIdentifier;
        Menu*                                                                                  m_pVCLMenu;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                    m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >           m_xUICommandLabels;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUIControllerFactory > m_xPopupMenuControllerFactory;
        ::std::vector< MenuItemHandler* >                                                      m_aMenuItemHandlerVector;
        ::cppu::OMultiTypeInterfaceContainerHelper                                             m_aListenerContainer;   /// container for ALL Listener
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >         m_xDispatchProvider;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >                m_xDocImageManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >                m_xModuleImageManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xDocAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xModuleAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XAcceleratorConfiguration >    m_xGlobalAcceleratorManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >       mxServiceFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >            m_xURLTransformer;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >          m_xDeferedItemContainer;
        sal_Int16                                                                              m_nSymbolsStyle;
        Timer                                                                                  m_aAsyncSettingsTimer;
};

} // namespace

#endif
