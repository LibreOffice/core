/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newmenucontroller.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-03 12:01:23 $
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

#ifndef __FRAMEWORK_UIELEMENT_NEWMENUCONTROLLER_HXX_
#include <uielement/newmenucontroller.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif
#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif
#ifndef __FRAMEWORK_CLASSES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif
#ifndef __FRAMEWORK_CLASSES_FWKRESID_HXX_
#include <classes/fwkresid.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_BMKMENU_HXX
#include <classes/bmkmenu.hxx>
#endif
#ifndef __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
#include <helper/imageproducer.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <classes/menuconfiguration.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MENUITEMSTYLE_HPP_
#include <com/sun/star/awt/MenuItemStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <vcl/i18nhelp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#include <svtools/acceleratorexecute.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::ui;

static const char SFX_REFERER_USER[] = "private:user";

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   NewMenuController                           ,
                                            OWeakObject                                 ,
                                            SERVICENAME_POPUPMENUCONTROLLER             ,
                                            IMPLEMENTATIONNAME_NEWMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   NewMenuController, {} )

void NewMenuController::setMenuImages( PopupMenu* pPopupMenu, sal_Bool bSetImages, sal_Bool bHiContrast )
{
    int                 nItemCount = pPopupMenu->GetItemCount();
    Image               aImage;
    Reference< XFrame > xFrame( m_xFrame );

    for ( int i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = pPopupMenu->GetItemId( i );
        if ( nItemId != 0 )
        {
            if ( bSetImages )
            {
                sal_Bool        bImageSet( sal_False );
                ::rtl::OUString aImageId;

                AddInfoForId::const_iterator pInfo = m_aAddInfoForItem.find( nItemId );
                if ( pInfo != m_aAddInfoForItem.end() )
                    aImageId = pInfo->second.aImageId; // Retrieve image id for menu item

                if ( aImageId.getLength() > 0 )
                {
                    aImage = GetImageFromURL( xFrame, aImageId, FALSE, bHiContrast );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        pPopupMenu->SetItemImage( nItemId, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    String aCmd( pPopupMenu->GetItemCommand( nItemId ) );
                    if ( aCmd.Len() )
                        aImage = GetImageFromURL( xFrame, aCmd, FALSE, bHiContrast );

                    if ( !!aImage )
                        pPopupMenu->SetItemImage( nItemId, aImage );
                }
            }
            else
                pPopupMenu->SetItemImage( nItemId, aImage );
        }
    }
}

void NewMenuController::determineAndSetNewDocAccel( PopupMenu* pPopupMenu, const KeyCode& rKeyCode )
{
    USHORT        nCount( pPopupMenu->GetItemCount() );
    USHORT        nId( 0 );
    sal_Bool      bFound( sal_False );
    rtl::OUString aCommand;

    if ( m_aEmptyDocURL.getLength() > 0 )
    {
        // Search for the empty document URL

        for ( sal_uInt32 i = 0; i < sal_uInt32( nCount ); i++ )
        {
            nId = pPopupMenu->GetItemId( USHORT( i ));
            if ( nId != 0 && pPopupMenu->GetItemType( nId ) != MENUITEM_SEPARATOR )
            {
                aCommand = pPopupMenu->GetItemCommand( nId );
                if ( aCommand.indexOf( m_aEmptyDocURL ) == 0 )
                {
                    pPopupMenu->SetAccelKey( nId, rKeyCode );
                    bFound = sal_True;
                    break;
                }
            }
        }
    }

    if ( !bFound )
    {
        // Search for the default module name
        rtl::OUString aDefaultModuleName( SvtModuleOptions().GetDefaultModuleName() );
        if ( aDefaultModuleName.getLength() > 0 )
        {
            for ( sal_uInt32 i = 0; i < sal_uInt32( nCount ); i++ )
            {
                nId = pPopupMenu->GetItemId( USHORT( i ));
                if ( nId != 0 && pPopupMenu->GetItemType( nId ) != MENUITEM_SEPARATOR )
                {
                    aCommand = pPopupMenu->GetItemCommand( nId );
                    if ( aCommand.indexOf( aDefaultModuleName ) >= 0 )
                    {
                        pPopupMenu->SetAccelKey( nId, rKeyCode );
                        break;
                    }
                }
            }
        }
    }
}

void NewMenuController::setAccelerators( PopupMenu* pPopupMenu )
{
    if ( m_bModuleIdentified )
    {
        Reference< XAcceleratorConfiguration > xDocAccelCfg( m_xDocAcceleratorManager );
        Reference< XAcceleratorConfiguration > xModuleAccelCfg( m_xModuleAcceleratorManager );
        Reference< XAcceleratorConfiguration > xGlobalAccelCfg( m_xGlobalAcceleratorManager );

        if ( !m_bAcceleratorCfg )
        {
            // Retrieve references on demand
            m_bAcceleratorCfg = sal_True;
            if ( !xDocAccelCfg.is() )
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
                            if ( xDocUICfgMgr.is() )
                            {
                                xDocAccelCfg = Reference< XAcceleratorConfiguration >( xDocUICfgMgr->getShortCutManager(), UNO_QUERY );
                                m_xDocAcceleratorManager = xDocAccelCfg;
                            }
                        }
                    }
                }
            }

            if ( !xModuleAccelCfg.is() )
            {
                Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier( m_xServiceManager->createInstance(
                                                                                            SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ),
                                                                                        UNO_QUERY );
                Reference< XUIConfigurationManager > xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
                if ( xUICfgMgr.is() )
                {
                    xModuleAccelCfg = Reference< XAcceleratorConfiguration >( xUICfgMgr->getShortCutManager(), UNO_QUERY );
                    m_xModuleAcceleratorManager = xModuleAccelCfg;
                }
            }

            if ( !xGlobalAccelCfg.is() )
            {
                xGlobalAccelCfg = Reference< XAcceleratorConfiguration >( m_xServiceManager->createInstance(
                                                                            SERVICENAME_GLOBALACCELERATORCONFIGURATION ),
                                                                          UNO_QUERY );
                m_xGlobalAcceleratorManager = xGlobalAccelCfg;
            }
        }

        KeyCode                         aEmptyKeyCode;
        sal_uInt32                      nItemCount( pPopupMenu->GetItemCount() );
        std::vector< KeyCode >          aMenuShortCuts;
        std::vector< rtl::OUString >    aCmds;
        std::vector< sal_uInt32 >       aIds;
        for ( sal_uInt32 i = 0; i < nItemCount; i++ )
        {
            USHORT nId( pPopupMenu->GetItemId( USHORT( i )));
            if ( nId & ( pPopupMenu->GetItemType( nId ) != MENUITEM_SEPARATOR ))
            {
                aIds.push_back( nId );
                aMenuShortCuts.push_back( aEmptyKeyCode );
                aCmds.push_back( pPopupMenu->GetItemCommand( nId ));
            }
        }

        sal_uInt32 nSeqCount( aIds.size() );

        if ( m_bNewMenu )
            nSeqCount+=1;

        Sequence< rtl::OUString > aSeq( nSeqCount );

        // Add a special command for our "New" menu.
        if ( m_bNewMenu )
        {
            aSeq[nSeqCount-1] = m_aCommandURL;
            aMenuShortCuts.push_back( aEmptyKeyCode );
        }

        for ( sal_uInt32 i = 0; i < aCmds.size(); i++ )
            aSeq[i] = aCmds[i];

        if ( m_xGlobalAcceleratorManager.is() )
            retrieveShortcutsFromConfiguration( xGlobalAccelCfg, aSeq, aMenuShortCuts );
        if ( m_xModuleAcceleratorManager.is() )
            retrieveShortcutsFromConfiguration( xModuleAccelCfg, aSeq, aMenuShortCuts );
        if ( m_xDocAcceleratorManager.is() )
            retrieveShortcutsFromConfiguration( xGlobalAccelCfg, aSeq, aMenuShortCuts );

        for ( sal_uInt32 i = 0; i < aIds.size(); i++ )
            pPopupMenu->SetAccelKey( USHORT( aIds[i] ), aMenuShortCuts[i] );

        // Special handling for "New" menu short-cut should be set at the
        // document which will be opened using it.
        if ( m_bNewMenu )
        {
            if ( aMenuShortCuts[nSeqCount-1] != aEmptyKeyCode )
                determineAndSetNewDocAccel( pPopupMenu, aMenuShortCuts[nSeqCount-1] );
        }
    }
}

void NewMenuController::retrieveShortcutsFromConfiguration(
    const Reference< XAcceleratorConfiguration >& rAccelCfg,
    const Sequence< rtl::OUString >& rCommands,
    std::vector< KeyCode >& aMenuShortCuts )
{
    if ( rAccelCfg.is() )
    {
        try
        {
            com::sun::star::awt::KeyEvent aKeyEvent;
            Sequence< Any > aSeqKeyCode = rAccelCfg->getPreferredKeyEventsForCommandList( rCommands );
            for ( sal_Int32 i = 0; i < aSeqKeyCode.getLength(); i++ )
            {
                if ( aSeqKeyCode[i] >>= aKeyEvent )
                    aMenuShortCuts[i] = svt::AcceleratorExecute::st_AWTKey2VCLKey( aKeyEvent );
            }
        }
        catch ( IllegalArgumentException& )
        {
        }
    }
}

NewMenuController::NewMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager ),
    m_bShowImages( sal_True ),
    m_bHiContrast( sal_False ),
    m_bNewMenu( sal_False ),
    m_bModuleIdentified( sal_False ),
    m_bAcceleratorCfg( sal_False ),
    m_aTargetFrame( RTL_CONSTASCII_USTRINGPARAM( "_default" ))
{
    m_xURLTransformer = Reference< XURLTransformer >(
                            xServiceManager->createInstance(
                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                            UNO_QUERY );
}

NewMenuController::~NewMenuController()
{
}

// private function
void NewMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu* pPopupMenu    = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*     pVCLPopupMenu = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        MenuConfiguration aMenuCfg( m_xServiceManager );
        BmkMenu* pSubMenu( 0 );

        if ( m_bNewMenu )
            pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( m_xFrame, BOOKMARK_NEWMENU );
        else
            pSubMenu = (BmkMenu*)aMenuCfg.CreateBookmarkMenu( m_xFrame, BOOKMARK_WIZARDMENU );

        // copy entries as we have to use the provided popup menu
        *pVCLPopupMenu = *pSubMenu;

        int             nItemCount = pVCLPopupMenu->GetItemCount();
        Image           aImage;
        AddInfo         aAddInfo;

        // retrieve additional parameters from bookmark menu and
        // store it in a hash_map.
        for ( int i = 0; i < pSubMenu->GetItemCount(); i++ )
        {
            USHORT nItemId = pSubMenu->GetItemId( i );
            if (( nItemId != 0 ) &&
                ( pSubMenu->GetItemType( nItemId ) != MENUITEM_SEPARATOR ))
            {
                MenuConfiguration::Attributes* pBmkAttributes = (MenuConfiguration::Attributes *)(pSubMenu->GetUserValue( nItemId ));
                if ( pBmkAttributes != 0 )
                {
                    aAddInfo.aTargetFrame = pBmkAttributes->aTargetFrame;
                    aAddInfo.aImageId     = pBmkAttributes->aImageId;

                    m_aAddInfoForItem.insert( AddInfoForId::value_type( nItemId, aAddInfo ));
                }
            }
        }

        if ( m_bShowImages )
            setMenuImages( pVCLPopupMenu, m_bShowImages, m_bHiContrast );

        delete pSubMenu;
    }
}

// XEventListener
void SAL_CALL NewMenuController::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL NewMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
}

// XMenuListener
void SAL_CALL NewMenuController::highlight( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

void SAL_CALL NewMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu > xPopupMenu;
    Reference< XDispatch >            xDispatch;
    Reference< XDispatchProvider >    xDispatchProvider;
    Reference< XMultiServiceFactory > xServiceManager;
    Reference< XURLTransformer >      xURLTransformer;

    ResetableGuard aLock( m_aLock );
    xPopupMenu          = m_xPopupMenu;
    xDispatchProvider   = Reference< XDispatchProvider >( m_xFrame, UNO_QUERY );
    xServiceManager     = m_xServiceManager;
    xURLTransformer     = m_xURLTransformer;
    aLock.unlock();

    css::util::URL aTargetURL;
    Sequence< PropertyValue > aArgsList( 1 );

    if ( xPopupMenu.is() && xDispatchProvider.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );

            aArgsList[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
            aArgsList[0].Value = makeAny( ::rtl::OUString::createFromAscii( SFX_REFERER_USER ));

            rtl::OUString aTargetFrame( m_aTargetFrame );
            AddInfoForId::const_iterator pItem = m_aAddInfoForItem.find( rEvent.MenuId );
            if ( pItem != m_aAddInfoForItem.end() )
                aTargetFrame = pItem->second.aTargetFrame;

            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, aTargetFrame, 0 );
        }
    }

    if ( xDispatch.is() )
    {
        // Call dispatch asychronously as we can be destroyed while dispatch is
        // executed. VCL is not able to survive this as it wants to call listeners
        // after select!!!
        NewDocument* pNewDocument = new NewDocument;
        pNewDocument->xDispatch  = xDispatch;
        pNewDocument->aTargetURL = aTargetURL;
        pNewDocument->aArgSeq    = aArgsList;
        Application::PostUserEvent( STATIC_LINK(0, NewMenuController, ExecuteHdl_Impl), pNewDocument );
    }
}

void SAL_CALL NewMenuController::activate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    if ( m_xFrame.is() && m_xPopupMenu.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( m_xPopupMenu );
        if ( pPopupMenu )
        {
            const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
            sal_Bool bShowImages( SvtMenuOptions().IsMenuIconsEnabled() );
            sal_Bool bHiContrast( rSettings.GetMenuColor().IsDark() );

            PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

            if (( m_bShowImages != bShowImages ) ||
                ( m_bHiContrast != bHiContrast ))
            {
                m_bShowImages = bShowImages;
                m_bHiContrast = bHiContrast;

                setMenuImages( pVCLPopupMenu, m_bShowImages, m_bHiContrast );
            }

            setAccelerators( pVCLPopupMenu );
        }
    }
}

void SAL_CALL NewMenuController::deactivate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL NewMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));

        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        m_xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        if ( m_xPopupMenu.is() )
            fillPopupMenu( m_xPopupMenu );

        // Identify module that we are attach to. It's our context that we need to know.
        Reference< XModuleManager > xModuleManager( m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ),UNO_QUERY );
        if ( xModuleManager.is() )
        {
            try
            {
                m_aModuleIdentifier = xModuleManager->identify( m_xFrame );
                m_bModuleIdentified = sal_True;

                Reference< XNameAccess > xNameAccess( xModuleManager, UNO_QUERY );
                if (( m_aModuleIdentifier.getLength() > 0 ) && xNameAccess.is() )
                {
                    Sequence< PropertyValue > aSeq;

                    Any a = xNameAccess->getByName( m_aModuleIdentifier );
                    if ( a >>= aSeq )
                    {
                        for ( sal_Int32 y = 0; y < aSeq.getLength(); y++ )
                        {
                            if ( aSeq[y].Name.equalsAscii("ooSetupFactoryEmptyDocumentURL") )
                            {
                                aSeq[y].Value >>= m_aEmptyDocURL;
                                break;
                            }
                        }
                    }
                }
            }
            catch ( RuntimeException& e )
            {
                throw e;
            }
            catch ( Exception& )
            {
            }
        }
    }
}

void SAL_CALL NewMenuController::updatePopupMenu() throw (RuntimeException)
{
}

// XInitialization
void SAL_CALL NewMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    const rtl::OUString aFrameName( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    const rtl::OUString aCommandURLName( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));

    ResetableGuard aLock( m_aLock );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        rtl::OUString       aCommandURL;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "Frame" ))
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name.equalsAscii( "CommandURL" ))
                    aPropValue.Value >>= aCommandURL;
            }
        }

        if ( xFrame.is() && aCommandURL.getLength() )
        {
            ResetableGuard aLock( m_aLock );
            m_xFrame        = xFrame;
            m_aCommandURL   = aCommandURL;
            m_bInitialized  = sal_True;
            m_bShowImages   = SvtMenuOptions().IsMenuIconsEnabled();

            const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
            m_bHiContrast   = rSettings.GetMenuColor().IsDark();
            m_bNewMenu      = m_aCommandURL.equalsAscii( ".uno:AddDirect" );
        }
    }
}

IMPL_STATIC_LINK( NewMenuController, ExecuteHdl_Impl, NewDocument*, pNewDocument )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pNewDocument->xDispatch->dispatch( pNewDocument->aTargetURL, pNewDocument->aArgSeq );
    }
    catch ( Exception& )
    {
    }

    delete pNewDocument;
    return 0;
}

}
