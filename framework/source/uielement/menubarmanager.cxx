/*************************************************************************
 *
 *  $RCSfile: menubarmanager.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:27:41 $
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


//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UIELEMENT_MENUBARMANAGER_HXX_
#include <uielement/menubarmanager.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <classes/menuconfiguration.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
#include <classes/bmkmenu.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#include <classes/addonmenu.hxx>
#endif
#ifndef __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
#include <helper/imageproducer.hxx>
#endif
#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#include "classes/addonsoptions.hxx"
#endif
#ifndef __FRAMEWORK_CLASSES_FWKRESID_HXX_
#include <classes/fwkresid.hxx>
#endif
#ifndef __FRAMEWORK_HELPER_MODULEIDENTIFIER_HXX_
#include <helper/moduleidentifier.hxx>
#endif
#ifndef __FRAMEWORK_HELPER_ACCELERATORINFO_HXX_
#include <helper/acceleratorinfo.hxx>
#endif
#include <classes/resource.hrc>
#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGWIDTH_HPP_
#include <com/sun/star/util/XStringWidth.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XPOPUPMENUCONTROLLER_HPP_
#include <drafts/com/sun/star/frame/XPopupMenuController.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XUICONTROLLERREGISTRATION_HPP_
#include <drafts/com/sun/star/frame/XUIControllerRegistration.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSYSTEMDEPENDENT_HPP_
#include <com/sun/star/lang/SystemDependent.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_ITEMTYPE_HPP_
#include <drafts/com/sun/star/ui/ItemType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <drafts/com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLECOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <drafts/com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include <comphelper/extract.hxx>
#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vos/mutex.hxx>
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::drafts::com::sun::star::frame;
using namespace ::drafts::com::sun::star::ui;

static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_CONTAINER[]   = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";

struct SystemMenuData
{
    unsigned long   nSize;          // size in bytes of this structure
    long            aMenu;          // ???
};

class StringLength : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XStringWidth >
{
    public:
        StringLength() {}
        virtual ~StringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& aString )
            throw (::com::sun::star::uno::RuntimeException)
        {
            return aString.getLength();
        }
};

namespace framework
{

// special menu ids/command ids for dynamic popup menus
#define SID_SFX_START           5000
#define SID_NEWDOCDIRECT        (SID_SFX_START + 537)
#define SID_AUTOPILOTMENU       (SID_SFX_START + 1381)
#define SID_PICKLIST            (SID_SFX_START + 510)
#define SID_MDIWINDOWLIST       (SID_SFX_START + 610)
#define SID_ADDONLIST           (SID_SFX_START + 1677)
#define SID_HELPMENU            (SID_SFX_START + 410)

#define SFX_REFERER_USER        "private:user"
#define DESKTOP_SERVICE         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))

const ::rtl::OUString aSlotString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
const ::rtl::OUString aSlotNewDocDirect( RTL_CONSTASCII_USTRINGPARAM( "slot:5537" ));
const ::rtl::OUString aCmdNewDocDirect( RTL_CONSTASCII_USTRINGPARAM( ".uno:AddDirect" ));
const ::rtl::OUString aSlotAutoPilot( RTL_CONSTASCII_USTRINGPARAM( "slot:6381" ));
const ::rtl::OUString aCmdAutoPilot( RTL_CONSTASCII_USTRINGPARAM( ".uno:AutoPilotMenu" ));

const ::rtl::OUString aSpecialFileMenu( RTL_CONSTASCII_USTRINGPARAM( "file" ));
const ::rtl::OUString aSpecialWindowMenu( RTL_CONSTASCII_USTRINGPARAM( "window" ));
const ::rtl::OUString aSlotSpecialFileMenu( RTL_CONSTASCII_USTRINGPARAM( "slot:5510" ));
const ::rtl::OUString aSlotSpecialWindowMenu( RTL_CONSTASCII_USTRINGPARAM( "slot:5610" ));
const ::rtl::OUString aSlotSpecialToolsMenu( RTL_CONSTASCII_USTRINGPARAM( "slot:6677" ));

// special uno commands for picklist and window list
const ::rtl::OUString aSpecialFileCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:PickList" ));
const ::rtl::OUString aSpecialWindowCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:WindowList" ));

const ::rtl::OUString UNO_COMMAND( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));


static sal_Int16 getImageTypeFromBools( sal_Bool bBig, sal_Bool bHighContrast )
{
    sal_Int16 n( 0 );
    if ( bBig )
        n |= ::drafts::com::sun::star::ui::ImageType::SIZE_LARGE;
    if ( bHighContrast )
        n |= ::drafts::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST;
    return n;
}

// #110897#
MenuBarManager::MenuBarManager(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    Reference< XFrame >& rFrame, Menu* pMenu, sal_Bool bDelete, sal_Bool bDeleteChildren )
:   // #110897#
    mxServiceFactory(xServiceFactory),
    ThreadHelpBase( &Application::GetSolarMutex() ), OWeakObject()
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , m_bDisposed( sal_False )
    , m_bModuleIdentified( sal_False )
    , m_bRetrieveImages( sal_False )
{
    m_xPopupMenuControllerRegistration = Reference< ::drafts::com::sun::star::frame::XUIControllerRegistration >(
        // #110897# ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.PopupMenuControllerFactory" ))),
        getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.PopupMenuControllerFactory" ))),
        UNO_QUERY );
    FillMenuManager( pMenu, rFrame, bDelete, bDeleteChildren );
}


// #110897#
MenuBarManager::MenuBarManager(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    Reference< XFrame >& rFrame, BmkMenu* pBmkMenu, sal_Bool bDelete, sal_Bool bDeleteChildren )
:   // #110897#
    mxServiceFactory(xServiceFactory),
    ThreadHelpBase( &Application::GetSolarMutex() )
    , OWeakObject()
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , m_bDisposed( sal_False )
    , m_bModuleIdentified( sal_False )
    , m_bRetrieveImages( sal_False )
{
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pBmkMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_True;

    m_xPopupMenuControllerRegistration = Reference< drafts::com::sun::star::frame::XUIControllerRegistration >(
        // #110897# ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.PopupMenuControllerFactory" ))),
        getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.PopupMenuControllerFactory" ))),
        UNO_QUERY );

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bWasHiContrast    = rSettings.GetMenuColor().IsDark();

    Reference< XDispatch > xDispatch;
    Reference< XStatusListener > xStatusListener;
    int nItemCount = pBmkMenu->GetItemCount();
    for ( int i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = pBmkMenu->GetItemId( i );

        ::rtl::OUString aItemCommand = pBmkMenu->GetItemCommand( nItemId );
        if ( !aItemCommand.getLength() )
        {
            aItemCommand = aSlotString;
            aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)nItemId );
            pBmkMenu->SetItemCommand( nItemId, aItemCommand );
        }

        PopupMenu* pPopupMenu = pBmkMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            // #110897# MenuBarManager* pSubMenuManager = new MenuBarManager( rFrame, pPopupMenu, bDeleteChildren, bDeleteChildren );
            MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, pPopupMenu, bDeleteChildren, bDeleteChildren );

            Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );

            // store menu item command as we later have to know which menu is active (see Acivate handler)
            pSubMenuManager->m_aMenuItemCommand = aItemCommand;

            MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                        nItemId,
                                                        xSubMenuManager,
                                                        xDispatch );
            m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
        }
        else
        {
            if ( pBmkMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
            {
                MenuConfiguration::Attributes* pBmkAttributes = (MenuConfiguration::Attributes *)(pBmkMenu->GetUserValue( nItemId ));
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );

                if ( pBmkAttributes )
                {
                    // read additional attributes from attributes struct and BmkMenu implementation will delete all attributes itself!!
                    pMenuItemHandler->aTargetFrame = pBmkAttributes->aTargetFrame;
                }

                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }

    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuBarManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuBarManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuBarManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuBarManager, Select ));
}


// #110897#
MenuBarManager::MenuBarManager(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    Reference< XFrame >& rFrame, AddonMenu* pAddonMenu, sal_Bool bDelete, sal_Bool bDeleteChildren )
:   // #110897#
    mxServiceFactory(xServiceFactory),
    ThreadHelpBase( &Application::GetSolarMutex() )
    , OWeakObject()
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , m_bDisposed( sal_False )
    , m_bModuleIdentified( sal_False )
    , m_bRetrieveImages( sal_False )
{
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pAddonMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_True;

    m_xPopupMenuControllerRegistration = Reference< ::drafts::com::sun::star::frame::XUIControllerRegistration >(
        // #110897# ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.PopupMenuControllerFactory" ))),
        getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.PopupMenuControllerFactory" ))),
        UNO_QUERY );

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bWasHiContrast    = rSettings.GetMenuColor().IsDark();

    Reference< XStatusListener > xStatusListener;
    Reference< XDispatch > xDispatch;
    int nItemCount = pAddonMenu->GetItemCount();
    for ( int i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = pAddonMenu->GetItemId( i );

        ::rtl::OUString aItemCommand = pAddonMenu->GetItemCommand( nItemId );
        if ( !aItemCommand.getLength() )
        {
            aItemCommand = aSlotString;
            aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)nItemId );
            pAddonMenu->SetItemCommand( nItemId, aItemCommand );
        }

        PopupMenu* pPopupMenu = pAddonMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            // #110897# MenuBarManager* pSubMenuManager = new MenuBarManager( rFrame, pPopupMenu, bDeleteChildren, bDeleteChildren );
            MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, pPopupMenu, bDeleteChildren, bDeleteChildren );

            Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );

            // store menu item command as we later have to know which menu is active (see Acivate handler)
            pSubMenuManager->m_aMenuItemCommand = aItemCommand;

            MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                        nItemId,
                                                        xSubMenuManager,
                                                        xDispatch );
            m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
        }
        else
        {
            if ( pAddonMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
            {
                MenuConfiguration::Attributes* pAddonAttributes = (MenuConfiguration::Attributes *)(pAddonMenu->GetUserValue( nItemId ));
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );

                if ( pAddonAttributes )
                {
                    // read additional attributes from attributes struct and AddonMenu implementation will delete all attributes itself!!
                    pMenuItemHandler->aTargetFrame = pAddonAttributes->aTargetFrame;
                }

                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }

    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuBarManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuBarManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuBarManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuBarManager, Select ));
}


// #110897#
MenuBarManager::MenuBarManager(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    Reference< XFrame >& rFrame, AddonPopupMenu* pAddonPopupMenu, sal_Bool bDelete, sal_Bool bDeleteChildren )
:   // #110897#
    mxServiceFactory(xServiceFactory),
    ThreadHelpBase( &Application::GetSolarMutex() )
    , OWeakObject()
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
    , m_bDisposed( sal_False )
    , m_bModuleIdentified( sal_False )
    , m_bRetrieveImages( sal_False )
{
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pAddonPopupMenu;
    m_xFrame            = rFrame;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_True;

    m_xPopupMenuControllerRegistration = Reference< ::drafts::com::sun::star::frame::XUIControllerRegistration >(
        // #110897# ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.PopupMenuControllerFactory" ))),
        getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.frame.PopupMenuControllerFactory" ))),
        UNO_QUERY );

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bWasHiContrast    = rSettings.GetMenuColor().IsDark();

    Reference< XDispatch > xDispatch;
    Reference< XStatusListener > xStatusListener;
    int nItemCount = pAddonPopupMenu->GetItemCount();
    for ( int i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = pAddonPopupMenu->GetItemId( i );

        ::rtl::OUString aItemCommand = pAddonPopupMenu->GetItemCommand( nItemId );
        if ( !aItemCommand.getLength() )
        {
            aItemCommand = aSlotString;
            aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)nItemId );
            pAddonPopupMenu->SetItemCommand( nItemId, aItemCommand );
        }

        PopupMenu* pPopupMenu = pAddonPopupMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            // #110897# MenuBarManager* pSubMenuManager = new MenuBarManager( rFrame, pPopupMenu, sal_False, sal_False );
            MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, pPopupMenu, sal_False, sal_False );

            Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );

            // store menu item command as we later have to know which menu is active (see Acivate handler)
            pSubMenuManager->m_aMenuItemCommand = aItemCommand;

            MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                        nItemId,
                                                        xSubMenuManager,
                                                        xDispatch );
            m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
        }
        else
        {
            if ( pAddonPopupMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
            {
                MenuConfiguration::Attributes* pAttributes = (MenuConfiguration::Attributes *)(pAddonPopupMenu->GetUserValue( nItemId ));
                MenuItemHandler* pMenuItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );

                if ( pAttributes )
                {
                    // read additional attributes from attributes struct and BmkMenu implementation will delete all attributes itself!!
                    pMenuItemHandler->aTargetFrame = pAttributes->aTargetFrame;
                }

                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }

    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuBarManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuBarManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuBarManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuBarManager, Select ));
}

Any SAL_CALL MenuBarManager::queryInterface( const ::com::sun::star::uno::Type & rType ) throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST( ::com::sun::star::frame::XStatusListener*, this ),
                SAL_STATIC_CAST( ::com::sun::star::frame::XFrameActionListener*, this ),
                SAL_STATIC_CAST( ::drafts::com::sun::star::ui::XUIConfigurationListener*, this ),
                SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, (XStatusListener *)this ),
                SAL_STATIC_CAST( ::com::sun::star::lang::XComponent*, this ),
                SAL_STATIC_CAST( ::com::sun::star::awt::XSystemDependentMenuPeer*, this ));

    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}


void SAL_CALL MenuBarManager::acquire() throw()
{
    OWeakObject::acquire();
}


void SAL_CALL MenuBarManager::release() throw()
{
    OWeakObject::release();
}


Any SAL_CALL MenuBarManager::getMenuHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw com::sun::star::lang::DisposedException();

    Any a;

    if ( m_pVCLMenu )
    {
        OGuard  aGuard( Application::GetSolarMutex() );

        SystemMenuData aSystemMenuData;
        aSystemMenuData.nSize = sizeof( SystemMenuData );

        m_pVCLMenu->GetSystemMenuData( &aSystemMenuData );
#ifdef UNX
        if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW )
        {
        }
#elif (defined WNT)
        if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_WIN32 )
        {
            a <<= aSystemMenuData.aMenu;
        }
#endif
    }

    return a;
}

MenuBarManager::~MenuBarManager()
{
    DBG_ASSERT( OWeakObject::m_refCount == 0, "Who wants to delete an object with refcount > 0!" );
    Destroy();
}

void MenuBarManager::Destroy()
{
    OGuard  aGuard( Application::GetSolarMutex() );

    if ( !m_bDisposed )
    {
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
        {
            MenuItemHandler* pItemHandler = *p;
            pItemHandler->xMenuItemDispatch.clear();
            pItemHandler->xSubMenuManager.clear();
            pItemHandler->xPopupMenu.clear();
            delete pItemHandler;
        }
        m_aMenuItemHandlerVector.clear();

        if ( m_bDeleteMenu )
            delete m_pVCLMenu;
    }
}

// XComponent
void SAL_CALL MenuBarManager::dispose() throw( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
        RemoveListener();
        Destroy();
        m_bDisposed = sal_True;

        if ( m_xDocImageManager.is() )
        {
            try
            {
                m_xDocImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch ( Exception& )
            {
            }
        }
        if ( m_xModuleImageManager.is() )
        {
            try
            {
                m_xModuleImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch ( Exception& )
            {
            }
        }
        m_xDocImageManager.clear();
        m_xModuleImageManager.clear();
    }
}

void SAL_CALL MenuBarManager::addEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL MenuBarManager::removeEventListener( const Reference< XEventListener >& xListener ) throw( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL MenuBarManager::elementInserted( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    sal_Int16 nImageType;
    sal_Int16 nCurrentImageType = getImageTypeFromBools( sal_False, m_bWasHiContrast );
    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ))
        RequestImages();
}

void SAL_CALL MenuBarManager::elementRemoved( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    sal_Int16 nImageType;
    sal_Int16 nCurrentImageType = getImageTypeFromBools( sal_False, m_bWasHiContrast );
    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ))
        RequestImages();
}

void SAL_CALL MenuBarManager::elementReplaced( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    sal_Int16 nImageType;
    sal_Int16 nCurrentImageType = getImageTypeFromBools( sal_False, m_bWasHiContrast );
    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ))
        RequestImages();
}

// XFrameActionListener
void SAL_CALL MenuBarManager::frameAction( const FrameActionEvent& Action )
throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw com::sun::star::lang::DisposedException();

    if ( Action.Action == FrameAction_CONTEXT_CHANGED )
    {
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
        {
            // Clear dispatch reference as we will requery it later o
            MenuItemHandler* pItemHandler = *p;
            pItemHandler->xMenuItemDispatch.clear();
        }
    }
}

// XStatusListener
void SAL_CALL MenuBarManager::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    ::rtl::OUString aFeatureURL = Event.FeatureURL.Complete;
    MenuItemHandler* pStatusChangedMenu = NULL;

    {
        ResetableGuard aGuard( m_aLock );

        if ( m_bDisposed )
            return;

        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
        {
            MenuItemHandler* pMenuItemHandler = *p;
            if ( pMenuItemHandler->aMenuItemURL == aFeatureURL )
            {
                pStatusChangedMenu = pMenuItemHandler;
                break;
            }
        }
    }

    if ( pStatusChangedMenu )
    {
        OGuard  aGuard( Application::GetSolarMutex() );
        {
            ResetableGuard aGuard( m_aLock );

            sal_Bool bSetCheckmark      = sal_False;
            sal_Bool bCheckmark         = sal_False;
            sal_Bool bMenuItemEnabled   = m_pVCLMenu->IsItemEnabled( pStatusChangedMenu->nItemId );

            // Enable/disable item
            if ( Event.IsEnabled != bMenuItemEnabled )
                m_pVCLMenu->EnableItem( pStatusChangedMenu->nItemId, Event.IsEnabled );

            // Checkmark
            if ( Event.State >>= bCheckmark )
                bSetCheckmark = sal_True;

            if ( bSetCheckmark )
                m_pVCLMenu->CheckItem( pStatusChangedMenu->nItemId, bCheckmark );
            else
            {
                rtl::OUString aItemText;

                if ( Event.State >>= aItemText )
                {
                    // Replacement for place holders
                    if ( aItemText.matchAsciiL( "($1)", 4 ))
                    {
                        String aResStr = String( FwkResId( STR_UPDATEDOC ));
                        rtl::OUString aTmp( aResStr );
                        aTmp += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ));
                        aTmp += aItemText.copy( 4 );
                        aItemText = aTmp;
                    }
                    else if ( aItemText.matchAsciiL( "($2)", 4 ))
                    {
                        String aResStr = String( FwkResId( STR_CLOSEDOC_ANDRETURN ));
                        rtl::OUString aTmp( aResStr );
                        aTmp += aItemText.copy( 4 );
                        aItemText = aTmp;
                    }
                    else if ( aItemText.matchAsciiL( "($3)", 4 ))
                    {
                        String aResStr = String( FwkResId( STR_SAVECOPYDOC ));
                        rtl::OUString aTmp( aResStr );
                        aTmp += aItemText.copy( 4 );
                        aItemText = aTmp;
                    }

                    m_pVCLMenu->SetItemText( pStatusChangedMenu->nItemId, aItemText );
                }
            }

        }

        if ( Event.Requery )
        {
            // Release dispatch object - will be requeried on the next activate!
            pStatusChangedMenu->xMenuItemDispatch.clear();
        }
    }
}

// Helper to retrieve own structure from item ID
MenuBarManager::MenuItemHandler* MenuBarManager::GetMenuItemHandler( USHORT nItemId )
{
    ResetableGuard aGuard( m_aLock );

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
    {
        MenuItemHandler* pItemHandler = *p;
        if ( pItemHandler->nItemId == nItemId )
            return pItemHandler;
    }

    return 0;
}

// Helper to set request images flag
void MenuBarManager::RequestImages()
{
    ResetableGuard aGuard( m_aLock );

    m_bRetrieveImages = sal_True;
    for ( sal_uInt32 i = 0; i < m_aMenuItemHandlerVector.size(); i++ )
    {
        MenuItemHandler* pItemHandler = m_aMenuItemHandlerVector[i];
        if ( pItemHandler->xSubMenuManager.is() )
        {
            MenuBarManager* pMenuBarManager = (MenuBarManager*)(pItemHandler->xSubMenuManager.get());
            pMenuBarManager->RequestImages();
        }
    }
}

// Helper to reset objects to prepare shutdown
void MenuBarManager::RemoveListener()
{
    ResetableGuard aGuard( m_aLock );

    // #110897#
    // Reference< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
    Reference< XURLTransformer > xTrans( getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
    {
        MenuItemHandler* pItemHandler = *p;
        if ( pItemHandler->xMenuItemDispatch.is() )
        {
            URL aTargetURL;
            aTargetURL.Complete = pItemHandler->aMenuItemURL;
            xTrans->parseStrict( aTargetURL );

            pItemHandler->xMenuItemDispatch->removeStatusListener(
                static_cast< XStatusListener* >( this ), aTargetURL );
        }

        pItemHandler->xMenuItemDispatch.clear();
        if ( pItemHandler->xPopupMenuController.is() )
        {
            {
                // Remove popup menu from menu structure
                OGuard  aGuard( Application::GetSolarMutex() );
                m_pVCLMenu->SetPopupMenu( pItemHandler->nItemId, 0 );
            }

            Reference< com::sun::star::lang::XEventListener > xEventListener( pItemHandler->xPopupMenuController, UNO_QUERY );
            if ( xEventListener.is() )
            {
                EventObject aEventObject;
                aEventObject.Source = (OWeakObject *)this;
                xEventListener->disposing( aEventObject );
            }

            // Release references to destroy controller and popup menu
            pItemHandler->xPopupMenuController.clear();
            pItemHandler->xPopupMenu.clear();
        }

        Reference< XComponent > xComponent( pItemHandler->xSubMenuManager, UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
    }

    try
    {
        if ( m_xFrame.is() )
            m_xFrame->removeFrameActionListener( Reference< XFrameActionListener >(
                                                    static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }
    catch ( Exception& )
    {
    }

    m_xFrame = 0;
}

void SAL_CALL MenuBarManager::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    MenuItemHandler* pMenuItemDisposing = NULL;

    ResetableGuard aGuard( m_aLock );

    std::vector< MenuItemHandler* >::iterator p;
    for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
    {
        MenuItemHandler* pMenuItemHandler = *p;
        if ( pMenuItemHandler->xMenuItemDispatch.is() &&
                pMenuItemHandler->xMenuItemDispatch == Source.Source )
        {
            // disposing called from menu item dispatcher, remove listener
            pMenuItemDisposing = pMenuItemHandler;
            break;
        }
    }

    if ( pMenuItemDisposing )
    {
        // Release references to the dispatch object
        URL aTargetURL;
        aTargetURL.Complete = pMenuItemDisposing->aMenuItemURL;

        // #110897#
        // Reference< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
        Reference< XURLTransformer > xTrans( getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
        xTrans->parseStrict( aTargetURL );

        pMenuItemDisposing->xMenuItemDispatch->removeStatusListener(
            static_cast< XStatusListener* >( this ), aTargetURL );
        pMenuItemDisposing->xMenuItemDispatch = Reference< XDispatch >();
        if ( pMenuItemDisposing->xPopupMenuController.is() )
        {
            Reference< com::sun::star::lang::XEventListener > xEventListener( pMenuItemDisposing->xPopupMenuController, UNO_QUERY );
            if ( xEventListener.is() )
                xEventListener->disposing( Source );

            if ( pMenuItemDisposing->xPopupMenuController.is() )
            {
                // Remove popup menu from menu structure as we release our reference to
                // the controller.
                OGuard  aGuard( Application::GetSolarMutex() );
                m_pVCLMenu->SetPopupMenu( pMenuItemDisposing->nItemId, 0 );
            }

            pMenuItemDisposing->xPopupMenuController.clear();
            pMenuItemDisposing->xPopupMenu.clear();
        }
        return;
    }
    else if ( Source.Source == m_xFrame )
    {
        // Our frame gets disposed. We have to remove all our listeners
        RemoveListener();
    }
    else if ( Source.Source == Reference< XInterface >( m_xDocImageManager, UNO_QUERY ))
        m_xDocImageManager.clear();
    else if ( Source.Source == Reference< XInterface >( m_xModuleImageManager, UNO_QUERY ))
        m_xModuleImageManager.clear();
}

void MenuBarManager::UpdateSpecialFileMenu( Menu* pMenu )
{
    // update picklist
    Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
    ::std::vector< MenuItemHandler* > aNewPickVector;
    Reference< XStringWidth > xStringLength( new StringLength );

    USHORT  nPickItemId = START_ITEMID_PICKLIST;
    int     nPickListMenuItems = ( aHistoryList.getLength() > 99 ) ? 99 : aHistoryList.getLength();

    Reference< XStatusListener > xStatusListener;
    Reference< XDispatch > xDispatch;
    for ( int i = 0; i < nPickListMenuItems; i++ )
    {
        Sequence< PropertyValue >& rPickListEntry = aHistoryList[i];

        MenuItemHandler* pNewMenuItemHandler = new MenuItemHandler(
                                                    nPickItemId++,
                                                    xStatusListener,
                                                    xDispatch );

        for ( int j = 0; j < rPickListEntry.getLength(); j++ )
        {
            Any a = rPickListEntry[j].Value;

            if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_URL )
                a >>= pNewMenuItemHandler->aMenuItemURL;
            else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_FILTER )
                a >>= pNewMenuItemHandler->aFilter;
            else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_TITLE )
                a >>= pNewMenuItemHandler->aTitle;
            else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_PASSWORD )
                a >>= pNewMenuItemHandler->aPassword;
        }

        aNewPickVector.push_back( pNewMenuItemHandler );
    }

    if ( aNewPickVector.size() > 0 )
    {
        URL aTargetURL;
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        // #110897#
        // Reference< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
        Reference< XURLTransformer > xTrans( getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );

        Reference< XDispatch > xMenuItemDispatch;

        // query for dispatcher
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = aNewPickVector.begin(); p != aNewPickVector.end(); p++ )
        {
            MenuItemHandler* pMenuItemHandler = *p;

            aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
            xTrans->parseStrict( aTargetURL );

            if ( !xMenuItemDispatch.is() )
            {
                // attention: this code assume that "_blank" can only be consumed by desktop service
                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_default"), 0 );
            }

            if ( xMenuItemDispatch.is() )
            {
                pMenuItemHandler->xMenuItemDispatch = xMenuItemDispatch;
                pMenuItemHandler->aMenuItemURL      = aTargetURL.Complete;
            }
        }

        {
            ResetableGuard aGuard( m_aLock );

            int nRemoveItemCount = 0;
            int nItemCount       = pMenu->GetItemCount();

            if ( nItemCount > 0 )
            {
                // remove all old picklist entries from menu
                sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_PICKLIST );
                for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                {
                    pMenu->RemoveItem( n );
                    ++nRemoveItemCount;
                }

                if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
                    pMenu->RemoveItem( pMenu->GetItemCount()-1 );

                // remove all old picklist entries from menu handler
                if ( nRemoveItemCount > 0 )
                {
                    for( sal_uInt32 nIndex = m_aMenuItemHandlerVector.size() - nRemoveItemCount;
                         nIndex < m_aMenuItemHandlerVector.size();  )
                    {
                        delete m_aMenuItemHandlerVector.at( nIndex );
                        m_aMenuItemHandlerVector.erase( m_aMenuItemHandlerVector.begin() + nIndex );
                    }
                }
            }

            // append new picklist menu entries
            pMenu->InsertSeparator();
            for ( sal_uInt32 i = 0; i < aNewPickVector.size(); i++ )
            {
                char menuShortCut[5] = "~n: ";

                ::rtl::OUString aMenuShortCut;
                if ( i <= 9 )
                {
                    if ( i == 9 )
                        aMenuShortCut = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "1~0: " ));
                    else
                    {
                        menuShortCut[1] = (char)( '1' + i );
                        aMenuShortCut = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( menuShortCut ));
                    }
                }
                else
                {
                    aMenuShortCut = rtl::OUString::valueOf((sal_Int32)( i + 1 ));
                    aMenuShortCut += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": " ));
                }

                // Abbreviate URL
                rtl::OUString   aURLString( aNewPickVector.at( i )->aMenuItemURL );
                rtl::OUString   aTipHelpText;
                rtl::OUString   aMenuTitle;
                INetURLObject   aURL( aURLString );

                if ( aURL.GetProtocol() == INET_PROT_FILE )
                {
                    // Do handle file URL differently => convert it to a system
                    // path and abbreviate it with a special function:
                    String aFileSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

                    ::rtl::OUString aSystemPath( aFileSystemPath );
                    ::rtl::OUString aCompactedSystemPath;

                    aTipHelpText = aSystemPath;
                    oslFileError nError = osl_abbreviateSystemPath( aSystemPath.pData, &aCompactedSystemPath.pData, 46, NULL );
                    if ( !nError )
                        aMenuTitle = String( aCompactedSystemPath );
                    else
                        aMenuTitle = aSystemPath;
                }
                else
                {
                    // Use INetURLObject to abbreviate all other URLs
                    String  aShortURL;
                    aShortURL = aURL.getAbbreviated( xStringLength, 46, INetURLObject::DECODE_UNAMBIGUOUS );
                    aMenuTitle += aShortURL;
                    aTipHelpText = aURLString;
                }

                ::rtl::OUString aTitle( aMenuShortCut + aMenuTitle );

                MenuItemHandler* pMenuItemHandler = aNewPickVector.at( i );
                pMenu->InsertItem( pMenuItemHandler->nItemId, aTitle );
                pMenu->SetTipHelpText( pMenuItemHandler->nItemId, aTipHelpText );
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
        }
    }
}


void MenuBarManager::UpdateSpecialWindowMenu( Menu* pMenu )
{
    // update window list
    ::std::vector< ::rtl::OUString > aNewWindowListVector;

    // #110897#
    // Reference< XDesktop > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( DESKTOP_SERVICE ), UNO_QUERY );
    Reference< XDesktop > xDesktop( getServiceFactory()->createInstance( DESKTOP_SERVICE ), UNO_QUERY );

    USHORT  nActiveItemId = 0;
    USHORT  nItemId = START_ITEMID_WINDOWLIST;

    if ( xDesktop.is() )
    {
        Reference< XFramesSupplier > xTasksSupplier( xDesktop, UNO_QUERY );
        Reference< XFrame > xCurrentFrame = xDesktop->getCurrentFrame();
        Reference< XIndexAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY );
        sal_Int32 nCount = xList->getCount();
        for (sal_Int32 i=0; i<nCount; ++i )
        {
            Any aItem = xList->getByIndex(i);
            Reference< XFrame > xFrame;
            aItem >>= xFrame;
            if (xFrame.is())
            {
                if ( xFrame == xCurrentFrame )
                    nActiveItemId = nItemId;

                Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                if ( pWin && pWin->IsVisible() )
                {
                    aNewWindowListVector.push_back( pWin->GetText() );
                    ++nItemId;
                }
            }
        }
    }

    {
        ResetableGuard aGuard( m_aLock );

        int nRemoveItemCount = 0;
        int nItemCount       = pMenu->GetItemCount();

        if ( nItemCount > 0 )
        {
            // remove all old window list entries from menu
            sal_uInt16 nPos = pMenu->GetItemPos( START_ITEMID_WINDOWLIST );
            for ( sal_uInt16 n = nPos; n < pMenu->GetItemCount(); )
                pMenu->RemoveItem( n );

            if ( pMenu->GetItemType( pMenu->GetItemCount()-1 ) == MENUITEM_SEPARATOR )
                pMenu->RemoveItem( pMenu->GetItemCount()-1 );
        }

        if ( aNewWindowListVector.size() > 0 )
        {
            // append new window list entries to menu
            pMenu->InsertSeparator();
            nItemId = START_ITEMID_WINDOWLIST;
            for ( sal_uInt32 i = 0; i < aNewWindowListVector.size(); i++ )
            {
                pMenu->InsertItem( nItemId, aNewWindowListVector.at( i ), MIB_RADIOCHECK );
                if ( nItemId == nActiveItemId )
                    pMenu->CheckItem( nItemId );
                ++nItemId;
            }
        }
    }
}


void MenuBarManager::CreatePicklistArguments( Sequence< PropertyValue >& aArgsList, const MenuItemHandler* pMenuItemHandler )
{
    int NUM_OF_PICKLIST_ARGS = 3;

    Any a;
    aArgsList.realloc( NUM_OF_PICKLIST_ARGS );

    aArgsList[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ));
    a <<= pMenuItemHandler->aMenuItemURL;
    aArgsList[0].Value = a;

    aArgsList[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
    a <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER ));
    aArgsList[1].Value = a;

    ::rtl::OUString aFilter( pMenuItemHandler->aFilter );

    sal_Int32 nPos = aFilter.indexOf( '|' );
    if ( nPos >= 0 )
    {
        ::rtl::OUString aFilterOptions;

        if ( nPos < ( aFilter.getLength() - 1 ) )
            aFilterOptions = aFilter.copy( nPos+1 );

        aArgsList[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterOptions" ));
        a <<= aFilterOptions;
        aArgsList[2].Value = a;

        aFilter = aFilter.copy( 0, nPos-1 );
        aArgsList.realloc( ++NUM_OF_PICKLIST_ARGS );
    }

    aArgsList[NUM_OF_PICKLIST_ARGS-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
    a <<= aFilter;
    aArgsList[NUM_OF_PICKLIST_ARGS-1].Value = a;
}


//_________________________________________________________________________________________________________________
// vcl handler
//_________________________________________________________________________________________________________________

IMPL_LINK( MenuBarManager, Activate, Menu *, pMenu )
{
    if ( pMenu == m_pVCLMenu )
    {
        // set/unset hiding disabled menu entries
        sal_Bool bDontHide          = SvtMenuOptions().IsEntryHidingEnabled();
        sal_Bool bShowMenuImages    = SvtMenuOptions().IsMenuIconsEnabled();

        ResetableGuard aGuard( m_aLock );

        sal_uInt16 nFlag = pMenu->GetMenuFlags();
        if ( bDontHide )
            nFlag &= ~MENU_FLAG_HIDEDISABLEDENTRIES;
        else
            nFlag |= MENU_FLAG_HIDEDISABLEDENTRIES;
        pMenu->SetMenuFlags( nFlag );

        if ( m_bActive )
            return 0;

        m_bActive = TRUE;

        ::rtl::OUString aCommand( m_aMenuItemCommand );
        if ( m_aMenuItemCommand == aSpecialFileMenu ||
             m_aMenuItemCommand == aSlotSpecialFileMenu ||
             aCommand == aSpecialFileCommand )
            UpdateSpecialFileMenu( pMenu );
        else if ( m_aMenuItemCommand == aSpecialWindowMenu ||
                  m_aMenuItemCommand == aSlotSpecialWindowMenu ||
                  aCommand == aSpecialWindowCommand )
            UpdateSpecialWindowMenu( pMenu );

        // Check if some modes have changed so we have to update our menu images
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        sal_Bool bIsHiContrast = rSettings.GetMenuColor().IsDark();

        if ( m_bRetrieveImages ||
             m_bWasHiContrast != bIsHiContrast ||
             bShowMenuImages != m_bShowMenuImages )
        {
            // The mode changed so we have to replace all images
            m_bWasHiContrast    = bIsHiContrast;
            m_bShowMenuImages   = bShowMenuImages;
            m_bRetrieveImages   = sal_False;
            AddonsOptions       aAddonOptions;

            for ( USHORT nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
            {
                USHORT nId = pMenu->GetItemId( nPos );
                if ( pMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR )
                {
                    if ( bShowMenuImages )
                    {
                        sal_Bool        bImageSet = sal_False;
                        ::rtl::OUString aImageId;

                        ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                            (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nId );

                        if ( pMenuAttributes )
                            aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                        if ( aImageId.getLength() > 0 )
                        {
                            Image aImage = GetImageFromURL( m_xFrame, aImageId, FALSE, bIsHiContrast );
                            if ( !!aImage )
                            {
                                bImageSet = sal_True;
                                pMenu->SetItemImage( nId, aImage );
                            }
                        }

                        if ( !bImageSet )
                        {
                            rtl::OUString aMenuItemCommand = pMenu->GetItemCommand( nId );
                            Image aImage = GetImageFromURL( m_xFrame, aMenuItemCommand, FALSE, bIsHiContrast );
                            if ( !aImage )
                                aImage = aAddonOptions.GetImageFromURL( aMenuItemCommand, FALSE, bIsHiContrast );

                            pMenu->SetItemImage( nId, aImage );
                        }
                    }
                    else
                        pMenu->SetItemImage( nId, Image() );
                }
            }
        }

        // Try to map commands to labels
        for ( USHORT nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
        {
            USHORT nItemId = pMenu->GetItemId( nPos );
            if (( pMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR ) &&
                ( pMenu->GetItemText( nItemId ).Len() == 0 ))
            {
                String aCommand = pMenu->GetItemCommand( nItemId );
                if ( aCommand.Len() > 0 )
                    pMenu->SetItemText( nItemId, RetrieveLabelFromCommand( aCommand ));
            }
        }

        URL aTargetURL;

        // #110897#
        // Reference< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
        Reference< XURLTransformer > xTrans( getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );

        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( xDispatchProvider.is() )
        {
            KeyCode aEmptyKeyCode;
            std::vector< MenuItemHandler* >::iterator p;
            for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
            {
                MenuItemHandler* pMenuItemHandler = *p;
                if ( pMenuItemHandler )
                {
                    if ( !pMenuItemHandler->xMenuItemDispatch.is() &&
                         !pMenuItemHandler->xSubMenuManager.is()      )
                    {
                        // There is no dispatch mechanism for the special window list menu items,
                        // because they are handled directly through XFrame->activate!!!
                        // Don't update dispatches for special file menu items.
                        if ( !(( pMenuItemHandler->nItemId >= START_ITEMID_WINDOWLIST &&
                                pMenuItemHandler->nItemId < END_ITEMID_WINDOWLIST ) ||
                            ( pMenuItemHandler->nItemId >= START_ITEMID_PICKLIST &&
                                pMenuItemHandler->nItemId < END_ITEMID_PICKLIST )))
                        {
                            Reference< XDispatch > xMenuItemDispatch;

                            ::rtl::OUString aItemCommand = pMenu->GetItemCommand( pMenuItemHandler->nItemId );
                            if ( !aItemCommand.getLength() )
                            {
                                aItemCommand = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                                aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)pMenuItemHandler->nItemId );
                                pMenu->SetItemCommand( pMenuItemHandler->nItemId, aItemCommand );
                            }

                            aTargetURL.Complete = aItemCommand;

                            xTrans->parseStrict( aTargetURL );

                            if ( m_bIsBookmarkMenu )
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, pMenuItemHandler->aTargetFrame, 0 );
                            else
                                xMenuItemDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

                            if ( !pMenuItemHandler->xPopupMenuController.is() &&
                                m_xPopupMenuControllerRegistration->hasController( aItemCommand, rtl::OUString() ))
                            {
                                // Try instanciate a popup menu controller. It is stored in the menu item handler.
                                Reference< XMultiComponentFactory > xPopupMenuControllerFactory( m_xPopupMenuControllerRegistration, UNO_QUERY );
                                if ( xPopupMenuControllerFactory.is() )
                                {
                                    Sequence< Any > aSeq( 2 );
                                    PropertyValue aPropValue;

                                    aPropValue.Name         = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
                                    aPropValue.Value      <<= m_aModuleIdentifier;
                                    aSeq[0] <<= aPropValue;
                                    aPropValue.Name         = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                                    aPropValue.Value      <<= m_xFrame;
                                    aSeq[1] <<= aPropValue;

                                    Reference< XComponentContext > xComponentContext;

                                    // #110897#
                                    // Reference< XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
                                    Reference< XPropertySet > xProps( getServiceFactory(), UNO_QUERY );

                                    xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>=
                                        xComponentContext;

                                    Reference< XPopupMenuController > xPopupMenuController(
                                                                            xPopupMenuControllerFactory->createInstanceWithArgumentsAndContext(
                                                                                aItemCommand,
                                                                                aSeq,
                                                                                xComponentContext ),
                                                                            UNO_QUERY );

                                    if ( xPopupMenuController.is() )
                                    {
                                        // Provide our awt popup menu to the popup menu controller
                                        pMenuItemHandler->xPopupMenuController = xPopupMenuController;
                                        xPopupMenuController->setPopupMenu( pMenuItemHandler->xPopupMenu );
                                    }
                                }
                            }
                            else if ( pMenuItemHandler->xPopupMenuController.is() )
                            {
                                // Force update of popup menu
                                pMenuItemHandler->xPopupMenuController->updatePopupMenu();
                            }

                            KeyCode aKeyCode = GetKeyCodeFromCommandURL( m_xFrame, aTargetURL.Complete );
                            pMenu->SetAccelKey( pMenuItemHandler->nItemId, aKeyCode );

                            if ( xMenuItemDispatch.is() )
                            {
                                pMenuItemHandler->xMenuItemDispatch = xMenuItemDispatch;
                                pMenuItemHandler->aMenuItemURL      = aTargetURL.Complete;

                                // We need only an update to reflect the current state
                                xMenuItemDispatch->addStatusListener( static_cast< XStatusListener* >( this ), aTargetURL );
                                xMenuItemDispatch->removeStatusListener( static_cast< XStatusListener* >( this ), aTargetURL );
                            }
                            else
                                pMenu->EnableItem( pMenuItemHandler->nItemId, sal_False );
                        }
                    }
                    else if ( pMenuItemHandler->xMenuItemDispatch.is() )
                    {
                        // We need an update to reflect the current state
                        try
                        {
                            aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
                            xTrans->parseStrict( aTargetURL );

                            pMenuItemHandler->xMenuItemDispatch->addStatusListener(
                                                                    static_cast< XStatusListener* >( this ), aTargetURL );
                            pMenuItemHandler->xMenuItemDispatch->removeStatusListener(
                                                                    static_cast< XStatusListener* >( this ), aTargetURL );
                        }
                        catch ( Exception& )
                        {
                        }
                    }
                }
            }
        }
    }

    return 1;
}


IMPL_LINK( MenuBarManager, Deactivate, Menu *, pMenu )
{
    if ( pMenu == m_pVCLMenu )
        m_bActive = sal_False;

    return 1;
}


IMPL_LINK( MenuBarManager, Select, Menu *, pMenu )
{
    URL                     aTargetURL;
    Sequence<PropertyValue> aArgs;
    Reference< XDispatch >  xDispatch;

    {
        ResetableGuard aGuard( m_aLock );

        USHORT nCurItemId = pMenu->GetCurItemId();
        USHORT nCurPos    = pMenu->GetItemPos( nCurItemId );
        if ( pMenu == m_pVCLMenu &&
             pMenu->GetItemType( nCurPos ) != MENUITEM_SEPARATOR )
        {
            if ( nCurItemId >= START_ITEMID_WINDOWLIST &&
                 nCurItemId <= END_ITEMID_WINDOWLIST )
            {
                // window list menu item selected

                // #110897#
                // Reference< XFramesSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( DESKTOP_SERVICE ), UNO_QUERY );
                Reference< XFramesSupplier > xDesktop( getServiceFactory()->createInstance( DESKTOP_SERVICE ), UNO_QUERY );

                USHORT  nWindowItemId = START_ITEMID_WINDOWLIST;

                if ( xDesktop.is() )
                {
                    USHORT nTaskId = START_ITEMID_WINDOWLIST;
                    Reference< XIndexAccess > xList( xDesktop->getFrames(), UNO_QUERY );
                    sal_Int32 nCount = xList->getCount();
                    for ( sal_Int32 i=0; i<nCount; ++i )
                    {
                        Any aItem = xList->getByIndex(i);
                        Reference< XFrame > xFrame;
                        aItem >>= xFrame;
                        if ( xFrame.is() && nTaskId == nCurItemId )
                        {
                            Window* pWin = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                            pWin->GrabFocus();
                            pWin->ToTop( TOTOP_RESTOREWHENMIN );
                            break;
                        }

                        nTaskId++;
                    }
                }
            }
            else
            {
                MenuItemHandler* pMenuItemHandler = GetMenuItemHandler( nCurItemId );
                if ( pMenuItemHandler && pMenuItemHandler->xMenuItemDispatch.is() )
                {
                    // #110897#
                    // Reference< XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );
                    Reference< XURLTransformer > xTrans( getServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))), UNO_QUERY );

                    aTargetURL.Complete = pMenuItemHandler->aMenuItemURL;
                    xTrans->parseStrict( aTargetURL );

                    if ( nCurItemId >= START_ITEMID_PICKLIST &&
                         nCurItemId <  START_ITEMID_WINDOWLIST )
                    {
                        // picklist menu item selected
                        CreatePicklistArguments( aArgs, pMenuItemHandler );
                    }
                    else if ( m_bIsBookmarkMenu )
                    {
                        // bookmark menu item selected
                        Any a;
                        aArgs.realloc( 1 );
                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
                        a <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER ));
                        aArgs[0].Value = a;
                    }

                    xDispatch = pMenuItemHandler->xMenuItemDispatch;
                }
            }
        }
    }

    if ( xDispatch.is() )
        xDispatch->dispatch( aTargetURL, aArgs );

    return 1;
}


IMPL_LINK( MenuBarManager, Highlight, Menu *, pMenu )
{
    return 0;
}

String MenuBarManager::RetrieveLabelFromCommand( const String& aCmdURL )
{
    String aLabel;

    // Retrieve popup menu labels
    if ( !m_xUICommandLabels.is() )
    {
        try
        {
            if ( !m_bModuleIdentified )
            {
                m_bModuleIdentified = sal_True;
                Reference< XModuleManager > xModuleManager;
                xModuleManager = Reference< XModuleManager >( getServiceFactory()->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );

                try
                {
                    m_aModuleIdentifier = xModuleManager->identify( m_xFrame );
                }
                catch( Exception& )
                {
                }
            }

            Reference< XNameAccess > xNameAccess( getServiceFactory()->createInstance( SERVICENAME_UICOMMANDDESCRIPTION ), UNO_QUERY );
            if ( xNameAccess.is() )
            {
                Any a = xNameAccess->getByName( m_aModuleIdentifier );
                Reference< XNameAccess > xUICommands;
                a >>= m_xUICommandLabels;
            }
        }
        catch ( Exception& )
        {
        }
    }

    if ( m_xUICommandLabels.is() )
    {
        try
        {
            if ( aCmdURL.Len() > 0 )
            {
                rtl::OUString aStr;
                Sequence< PropertyValue > aPropSeq;
                Any a( m_xUICommandLabels->getByName( aCmdURL ));
                if ( a >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name.equalsAscii( "Label" ))
                        {
                            aPropSeq[i].Value >>= aStr;
                            break;
                        }
                    }
                }
                aLabel = aStr;
            }
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }

    return aLabel;
}

void MenuBarManager::FillMenuManager( Menu* pMenu, Reference< XFrame >& rFrame, sal_Bool bDelete, sal_Bool bDeleteChildren )
{
    m_xFrame            = rFrame;
    m_bActive           = sal_False;
    m_bDeleteMenu       = bDelete;
    m_bDeleteChildren   = bDeleteChildren;
    m_pVCLMenu          = pMenu;
    m_bInitialized      = sal_False;
    m_bIsBookmarkMenu   = sal_False;

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bWasHiContrast    = rSettings.GetMenuColor().IsDark();
    m_bShowMenuImages   = SvtMenuOptions().IsMenuIconsEnabled();
    m_bRetrieveImages   = sal_False;

    sal_Int32 nAddonsURLPrefixLength = ADDONSPOPUPMENU_URL_PREFIX.getLength();
    ::std::vector< USHORT > aQueryLabelItemIdVector;

    // Add root as ui configuration listener
    RetrieveImageManagers();

    if ( pMenu->IsMenuBar() && rFrame.is() )
    {
        // First merge all addon popup menus into our structure
        USHORT nPos = 0;
        for ( nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
        {
            USHORT          nItemId  = pMenu->GetItemId( nPos );
            ::rtl::OUString aCommand = pMenu->GetItemCommand( nItemId );
            if ( nItemId == SID_MDIWINDOWLIST ||
                 aCommand == aSpecialWindowCommand )
            {
                // Retrieve addon popup menus and add them to our menu bar
                Reference< com::sun::star::frame::XModel >      xModel;
                Reference< com::sun::star::frame::XController > xController( rFrame->getController(), UNO_QUERY );
                if ( xController.is() )
                    xModel = Reference< com::sun::star::frame::XModel >( xController->getModel(), UNO_QUERY );
                framework::AddonMenuManager::MergeAddonPopupMenus( rFrame, xModel, nPos, (MenuBar *)pMenu );
                break;
            }
        }

        // Merge the Add-Ons help menu items into the Office help menu
        framework::AddonMenuManager::MergeAddonHelpMenu( rFrame, (MenuBar *)pMenu );
    }

    int nItemCount = pMenu->GetItemCount();
    for ( int i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = pMenu->GetItemId( i );

        ::rtl::OUString aItemCommand = pMenu->GetItemCommand( nItemId );
        if ( !aItemCommand.getLength() )
        {
            aItemCommand = aSlotString;
            aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)nItemId );
            pMenu->SetItemCommand( nItemId, aItemCommand );
        }

        if ( pMenu->IsMenuBar() && ( pMenu->GetItemText( nItemId ).Len() == 0 ))
        {
            if ( aItemCommand.getLength() > 0 )
                pMenu->SetItemText( nItemId, RetrieveLabelFromCommand( aItemCommand ));
        }

        Reference< XDispatch > xDispatch;
        Reference< XStatusListener > xStatusListener;
        PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nItemId );
        if ( pPopupMenu )
        {
            if ( m_xPopupMenuControllerRegistration.is() &&
                 m_xPopupMenuControllerRegistration->hasController( aItemCommand, rtl::OUString() ) &&
                 pPopupMenu->GetItemCount() == 0 )
            {
                // Check if we have to create a popup menu for a uno based popup menu controller.
                // We have to set an empty popup menu into our menu structure so the controller also
                // works with inplace OLE. Remove old dummy popup menu!
                MenuItemHandler* pItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );
                VCLXPopupMenu* pVCLXPopupMenu = new VCLXPopupMenu;
                PopupMenu* pNewPopupMenu = (PopupMenu *)pVCLXPopupMenu->GetMenu();
                pMenu->SetPopupMenu( nItemId, pNewPopupMenu );
                pItemHandler->xPopupMenu = Reference< com::sun::star::awt::XPopupMenu >( (OWeakObject *)pVCLXPopupMenu, UNO_QUERY );
                pItemHandler->aMenuItemURL = aItemCommand;
                m_aMenuItemHandlerVector.push_back( pItemHandler );
                delete pPopupMenu;
            }
            else if (( aItemCommand.getLength() > nAddonsURLPrefixLength ) &&
                ( aItemCommand.indexOf( ADDONSPOPUPMENU_URL_PREFIX ) == 0 ))
            {
                // A special addon popup menu, must be created with a different ctor
                // #110897# MenuBarManager* pSubMenuManager = new MenuBarManager( rFrame, (AddonPopupMenu *)pPopupMenu, bDeleteChildren, bDeleteChildren );
                MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, (AddonPopupMenu *)pPopupMenu, bDeleteChildren, bDeleteChildren );

                Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );
                rFrame->addFrameActionListener( Reference< XFrameActionListener >( xSubMenuManager, UNO_QUERY ));

                // store menu item command as we later have to know which menu is active (see Activate handler)
                pSubMenuManager->m_aMenuItemCommand = aItemCommand;

                MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                            nItemId,
                                                            xSubMenuManager,
                                                            xDispatch );
                pMenuItemHandler->aMenuItemURL = aItemCommand;
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );
            }
            else
            {
                // #110897# MenuBarManager* pSubMenuManager = new MenuBarManager( rFrame, pPopupMenu, bDeleteChildren, bDeleteChildren );
                MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, pPopupMenu, bDeleteChildren, bDeleteChildren );
                Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );
                rFrame->addFrameActionListener( Reference< XFrameActionListener >( xSubMenuManager, UNO_QUERY ));

                // store menu item command as we later have to know which menu is active (see Activate handler)
                pSubMenuManager->m_aMenuItemCommand = aItemCommand;

                MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                            nItemId,
                                                            xSubMenuManager,
                                                            xDispatch );
                pMenuItemHandler->aMenuItemURL = aItemCommand;
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );

                if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                    aQueryLabelItemIdVector.push_back( nItemId );

                // Create addon popup menu if there exist elements and this is the tools popup menu
                if (( nItemId == SID_ADDONLIST ||
                    aItemCommand == aSlotSpecialToolsMenu ) &&
                    AddonMenuManager::HasAddonMenuElements() )
                {
                    USHORT      nCount   = 0;
                    AddonMenu*  pSubMenu = AddonMenuManager::CreateAddonMenu( rFrame );
                    if ( pSubMenu && ( pSubMenu->GetItemCount() > 0 ))
                    {
                        if ( pPopupMenu->GetItemType( nCount-1 ) != MENUITEM_SEPARATOR )
                            pPopupMenu->InsertSeparator();

                        // Use resource to load popup menu title
                        String aAddonsStrRes = String( FwkResId( STR_MENU_ADDONS ));
                        pPopupMenu->InsertItem( ITEMID_ADDONLIST, aAddonsStrRes );
                        pPopupMenu->SetPopupMenu( ITEMID_ADDONLIST, pSubMenu );

                        // Set item command for popup menu to enable it for GetImageFromURL
                        aItemCommand = aSlotString;
                        aItemCommand += ::rtl::OUString::valueOf( (sal_Int32)ITEMID_ADDONLIST );
                        pPopupMenu->SetItemCommand( ITEMID_ADDONLIST, aItemCommand );

                        // #110897# MenuBarManager* pSubMenuManager = new MenuBarManager( rFrame, pSubMenu, sal_True, sal_False );
                        MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, pSubMenu, sal_True, sal_False );

                        Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );
                        rFrame->addFrameActionListener( Reference< XFrameActionListener >( xSubMenuManager, UNO_QUERY ));

                        MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                                    nItemId,
                                                                    xSubMenuManager,
                                                                    xDispatch );
                        pMenuItemHandler->aMenuItemURL = aItemCommand;
                        if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                            aQueryLabelItemIdVector.push_back( nItemId );
                        m_aMenuItemHandlerVector.push_back( pMenuItemHandler );

                        // Set image for the addon popup menu item
                        if ( m_bShowMenuImages && !pPopupMenu->GetItemImage( ITEMID_ADDONLIST ))
                        {
                            Image aImage = GetImageFromURL( rFrame, aItemCommand, FALSE, m_bWasHiContrast );
                            if ( !!aImage )
                                   pPopupMenu->SetItemImage( ITEMID_ADDONLIST, aImage );
                        }
                    }
                    else
                        delete pSubMenu;
                }
            }
        }
        else
        {
            if ( nItemId == SID_NEWDOCDIRECT ||
                 aItemCommand == aSlotNewDocDirect ||
                 aItemCommand == aCmdNewDocDirect )
            {
                // #110897#
                // Reference< ::com::sun::star::lang::XMultiServiceFactory > aMultiServiceFactory(::comphelper::getProcessServiceFactory());
                // MenuConfiguration aMenuCfg( aMultiServiceFactory );
                MenuConfiguration aMenuCfg( getServiceFactory() );

                BmkMenu* pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( rFrame, BOOKMARK_NEWMENU );
                pMenu->SetPopupMenu( nItemId, pSubMenu );

                // #110897# MenuBarManager* pSubMenuManager = new MenuBarManager( rFrame, pSubMenu, sal_True, sal_False );
                MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, pSubMenu, sal_True, sal_False );

                Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );
                rFrame->addFrameActionListener( Reference< XFrameActionListener >( xSubMenuManager, UNO_QUERY ));

                MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                            nItemId,
                                                            xSubMenuManager,
                                                            xDispatch );
                pMenuItemHandler->aMenuItemURL = aItemCommand;
                if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                    aQueryLabelItemIdVector.push_back( nItemId );
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );

                if ( m_bShowMenuImages && !pMenu->GetItemImage( nItemId ))
                {
                    Image aImage = GetImageFromURL( rFrame, aItemCommand, FALSE, m_bWasHiContrast );
                    if ( !!aImage )
                           pMenu->SetItemImage( nItemId, aImage );
                }
            }
            else if ( nItemId == SID_AUTOPILOTMENU ||
                      aItemCommand == aSlotAutoPilot ||
                      aItemCommand == aCmdAutoPilot )
            {
                // #110897#
                // Reference< ::com::sun::star::lang::XMultiServiceFactory > aMultiServiceFactory(::comphelper::getProcessServiceFactory());
                // MenuConfiguration aMenuCfg( aMultiServiceFactory );
                MenuConfiguration aMenuCfg( getServiceFactory() );

                BmkMenu* pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( rFrame, BOOKMARK_WIZARDMENU );
                pMenu->SetPopupMenu( nItemId, pSubMenu );

                // #110897# MenuBarManager* pSubMenuManager = new MenuBarManager( rFrame, pSubMenu, sal_True, sal_False );
                MenuBarManager* pSubMenuManager = new MenuBarManager( getServiceFactory(), rFrame, pSubMenu, sal_True, sal_False );

                Reference< XStatusListener > xSubMenuManager( static_cast< OWeakObject *>( pSubMenuManager ), UNO_QUERY );
                rFrame->addFrameActionListener( Reference< XFrameActionListener >( xSubMenuManager, UNO_QUERY ));

                MenuItemHandler* pMenuItemHandler = new MenuItemHandler(
                                                            nItemId,
                                                            xSubMenuManager,
                                                            xDispatch );
                pMenuItemHandler->aMenuItemURL = aItemCommand;
                if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                    aQueryLabelItemIdVector.push_back( nItemId );
                m_aMenuItemHandlerVector.push_back( pMenuItemHandler );

                if ( m_bShowMenuImages && !pMenu->GetItemImage( nItemId ))
                {
                    Image aImage = GetImageFromURL( rFrame, aItemCommand, FALSE, m_bWasHiContrast );
                    if ( !!aImage )
                           pMenu->SetItemImage( nItemId, aImage );
                }
            }
            else if ( pMenu->GetItemType( i ) != MENUITEM_SEPARATOR )
            {
                if ( m_bShowMenuImages )
                {
                    if ( AddonMenuManager::IsAddonMenuId( nItemId ))
                    {
                        // Add-Ons uses images from different places
                        Image           aImage;
                        rtl::OUString   aImageId;

                        MenuConfiguration::Attributes* pMenuAttributes =
                            (MenuConfiguration::Attributes*)pMenu->GetUserValue( nItemId );

                        if ( pMenuAttributes && pMenuAttributes->aImageId.getLength() > 0 )
                        {
                            // Retrieve image id from menu attributes
                            aImage = GetImageFromURL( rFrame, aImageId, FALSE, m_bWasHiContrast );
                        }

                        if ( !aImage )
                        {
                            aImage = GetImageFromURL( rFrame, aItemCommand, FALSE, m_bWasHiContrast );
                            if ( !aImage )
                                aImage = AddonsOptions().GetImageFromURL( aItemCommand, FALSE, m_bWasHiContrast );
                        }

                        if ( !!aImage )
                            pMenu->SetItemImage( nItemId, aImage );
                    }
                    else if ( !pMenu->GetItemImage( nItemId ))
                    {
                        Image aImage = GetImageFromURL( rFrame, aItemCommand, FALSE, m_bWasHiContrast );
                        if ( !!aImage )
                               pMenu->SetItemImage( nItemId, aImage );
                    }
                }

                MenuItemHandler* pItemHandler = new MenuItemHandler( nItemId, xStatusListener, xDispatch );

                if ( m_xPopupMenuControllerRegistration.is() &&
                     m_xPopupMenuControllerRegistration->hasController( aItemCommand, rtl::OUString() ))
                {
                    // Check if we have to create a popup menu for a uno based popup menu controller.
                    // We have to set an empty popup menu into our menu structure so the controller also
                    // works with inplace OLE.
                    VCLXPopupMenu* pVCLXPopupMenu = new VCLXPopupMenu;
                    PopupMenu* pPopupMenu = (PopupMenu *)pVCLXPopupMenu->GetMenu();
                    pMenu->SetPopupMenu( pItemHandler->nItemId, pPopupMenu );
                    pItemHandler->xPopupMenu = Reference< com::sun::star::awt::XPopupMenu >( (OWeakObject *)pVCLXPopupMenu, UNO_QUERY );
                }

                m_aMenuItemHandlerVector.push_back( pItemHandler );
                if ( pMenu->GetItemText( nItemId ).Len() == 0 )
                    aQueryLabelItemIdVector.push_back( nItemId );
            }
        }
    }

    m_pVCLMenu->SetHighlightHdl( LINK( this, MenuBarManager, Highlight ));
    m_pVCLMenu->SetActivateHdl( LINK( this, MenuBarManager, Activate ));
    m_pVCLMenu->SetDeactivateHdl( LINK( this, MenuBarManager, Deactivate ));
    m_pVCLMenu->SetSelectHdl( LINK( this, MenuBarManager, Select ));
}

void MenuBarManager::RetrieveImageManagers()
{
    if ( !m_xDocImageManager.is() )
    {
        Reference< XController > xController = m_xFrame->getController();
        Reference< XModel > xModel;
        if ( xController.is() )
        {
            xModel = xController->getModel();
            if ( xModel.is() )
            {
                Reference< XUIConfigurationManagerSupplier > xSupplier( xModel, UNO_QUERY );
                if ( xSupplier.is() )
                {
                    Reference< XUIConfigurationManager > xDocUICfgMgr( xSupplier->getUIConfigurationManager(), UNO_QUERY );
                    m_xDocImageManager = Reference< XImageManager >( xDocUICfgMgr->getImageManager(), UNO_QUERY );
                    m_xDocImageManager->addConfigurationListener(
                                            Reference< XUIConfigurationListener >(
                                                static_cast< OWeakObject* >( this ), UNO_QUERY ));
                }
            }
        }
    }

    Reference< XModuleManager > xModuleManager;
    if ( m_aModuleIdentifier.getLength() == 0 )
        xModuleManager = Reference< XModuleManager >( getServiceFactory()->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );

    try
    {
        if ( xModuleManager.is() )
            m_aModuleIdentifier = xModuleManager->identify( Reference< XInterface >( m_xFrame, UNO_QUERY ) );
    }
    catch( Exception& )
    {
    }

    if ( !m_xModuleImageManager.is() )
    {
        Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier( getServiceFactory()->createInstance(
                                                                                    SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ),
                                                                                  UNO_QUERY );
        Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
        m_xModuleImageManager = Reference< XImageManager >( xUICfgMgr->getImageManager(), UNO_QUERY );
        m_xModuleImageManager->addConfigurationListener( Reference< XUIConfigurationListener >(
                                                            static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }
}

void MenuBarManager::FillMenu( USHORT& nId, Menu* pMenu, const Reference< XIndexAccess >& rItemContainer )
{
    // Fill menu bar with container contents

    for ( sal_Int32 n = 0; n < rItemContainer->getCount(); n++ )
    {
        Sequence< PropertyValue >   aProp;
        rtl::OUString               aCommandURL;
        rtl::OUString               aLabel;
        rtl::OUString               aHelpURL;
        sal_uInt16                  nType;
        Reference< XIndexAccess >   xIndexContainer;

        try
        {
            if ( rItemContainer->getByIndex( n ) >>= aProp )
            {
                for ( int i = 0; i < aProp.getLength(); i++ )
                {
                    if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
                    {
                        aProp[i].Value >>= aCommandURL;
                    }
                    else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_HELPURL ))
                    {
                        aProp[i].Value >>= aHelpURL;
                    }
                    else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_CONTAINER ))
                    {
                        aProp[i].Value >>= xIndexContainer;
                    }
                    else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ))
                    {
                        aProp[i].Value >>= aLabel;
                    }
                    else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_TYPE ))
                    {
                        aProp[i].Value >>= nType;
                    }
                }

                if ( nType == drafts::com::sun::star::ui::ItemType::DEFAULT )
                {
                    pMenu->InsertItem( nId, aLabel );
                    pMenu->SetItemCommand( nId, aCommandURL );

                    sal_Int16 nHelpId = (sal_Int16)aHelpURL.toInt32();
                    if ( nHelpId > 0 )
                        pMenu->SetHelpId( nId, nHelpId );

                    if ( xIndexContainer.is() )
                    {
                        PopupMenu* pNewPopupMenu = new PopupMenu;
                        pMenu->SetPopupMenu( nId, pNewPopupMenu );

                        ++nId;
                        FillMenu( nId, pNewPopupMenu, xIndexContainer );
                    }
                    else
                        ++nId;
                }
                else
                {
                    pMenu->InsertSeparator();
                    ++nId;
                }
            }
        }
        catch ( ::com::sun::star::lang::IndexOutOfBoundsException& )
        {
            break;
        }
    }
}

void MenuBarManager::SetItemContainer( const Reference< XIndexAccess >& rItemContainer )
{
    ResetableGuard aGuard( m_aLock );

    Reference< XFrame > xFrame = m_xFrame;

    // Clear MenuBarManager structures
    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        RemoveListener();
        std::vector< MenuItemHandler* >::iterator p;
        for ( p = m_aMenuItemHandlerVector.begin(); p != m_aMenuItemHandlerVector.end(); p++ )
        {
            MenuItemHandler* pItemHandler = *p;
            pItemHandler->xMenuItemDispatch.clear();
            pItemHandler->xSubMenuManager.clear();
            delete pItemHandler;
        }
        m_aMenuItemHandlerVector.clear();

        // Remove top-level parts
        m_pVCLMenu->Clear();

        USHORT   nId = 1;

        // Fill menu bar with container contents
        FillMenu( nId, (Menu *)m_pVCLMenu, rItemContainer );

        // Refill menu manager again
        FillMenuManager( m_pVCLMenu, xFrame, sal_False, sal_True );

        // add itself as frame action listener
        m_xFrame->addFrameActionListener( Reference< XFrameActionListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }
}

// #110897#
const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& MenuBarManager::getServiceFactory()
{
    // #110897#
    return mxServiceFactory;
}

}
