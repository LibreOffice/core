/*************************************************************************
 *
 *  $RCSfile: toolbarsmenucontroller.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:13:09 $
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

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARSMENUCONTROLLER_HXX_
#include <uielement/toolbarsmenucontroller.hxx>
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
#ifndef __FRAMEWORK_UICONFIGURATION_WINDOWSTATECONFIGURATION_HXX_
#include <uiconfiguration/windowstateconfiguration.hxx>
#endif
#ifndef __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
#include <helper/imageproducer.hxx>
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
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <drafts/com/sun/star/frame/XLayoutManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUIELEMENTSETTINGS_HPP_
#include <drafts/com/sun/star/ui/XUIElementSettings.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_UIElementType_HPP_
#include <drafts/com/sun/star/ui/UIElementType.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <vcl/i18nhelp.hxx>
#endif
#include <vcl/image.hxx>
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
//#include <tools/solar.hrc>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::drafts::com::sun::star::frame;
using namespace ::drafts::com::sun::star::ui;

static const char CONFIGURE_TOOLBARS[]          = ".uno:ConfigureDialog";
static const char CMD_COLORBAR[]                = ".uno:ColorControl";
static const char CMD_HYPERLINKBAR[]            = ".uno:InsertHyperlink";
static const char CMD_FORMULABAR[]              = ".uno:InsertFormula";
static const char CMD_INPUTLINEBAR[]            = ".uno:InputLineVisible";
static const char ITEM_DESCRIPTOR_RESOURCEURL[] = "ResourceURL";
static const char ITEM_DESCRIPTOR_UINAME[]      = "UIName";

namespace framework
{

typedef std::hash_map< rtl::OUString, rtl::OUString, OUStringHashCode, ::std::equal_to< ::rtl::OUString > > ToolbarHashMap;

DEFINE_XSERVICEINFO_MULTISERVICE        (   ToolbarsMenuController                  ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_TOOLBARSMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   ToolbarsMenuController, {} )

ToolbarsMenuController::ToolbarsMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager ),
    m_bModuleIdentified( sal_False )
{
}

ToolbarsMenuController::~ToolbarsMenuController()
{
}

void ToolbarsMenuController::addCommand( Reference< css::awt::XPopupMenu >& rPopupMenu, const rtl::OUString& rCommandURL, USHORT nHelpId )
{
    USHORT        nItemId    = m_xPopupMenu->getItemCount()+1;
    rtl::OUString aLabel     = getUINameFromCommand( rCommandURL );
    rPopupMenu->insertItem( nItemId, aLabel, 0, nItemId );
    Reference< awt::XMenuExtended > xMenuExtended( m_xPopupMenu, UNO_QUERY );
    xMenuExtended->setCommand( nItemId, rCommandURL );

    if ( !getDispatchFromCommandURL( rCommandURL ).is() )
        m_xPopupMenu->enableItem( nItemId, sal_False );

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    Image aImage = GetImageFromURL( m_xFrame, rCommandURL, FALSE, rSettings.GetMenuColor().IsDark() );

    VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( rPopupMenu );
    if ( pPopupMenu )
    {
        PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
        if ( !!aImage )
            pVCLPopupMenu->SetItemImage( nItemId, aImage );
        pVCLPopupMenu->SetHelpId( nItemId, nHelpId );
    }

    m_aCommandVector.push_back( rCommandURL );
}

Reference< XDispatch > ToolbarsMenuController::getDispatchFromCommandURL( const rtl::OUString& rCommandURL )
{
    URL                          aTargetURL;
    Sequence<PropertyValue>      aArgs;
    Reference< XURLTransformer > xURLTransformer;
    Reference< XFrame >          xFrame;

    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
        xURLTransformer = m_xURLTransformer;
        xFrame = m_xFrame;
    }

    aTargetURL.Complete = rCommandURL;
    xURLTransformer->parseStrict( aTargetURL );
    Reference< XDispatchProvider > xDispatchProvider( xFrame, UNO_QUERY );
    if ( xDispatchProvider.is() )
        return xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
    else
        return Reference< XDispatch >();
}

// private function
rtl::OUString ToolbarsMenuController::getUINameFromCommand( const rtl::OUString& rCommandURL )
{
    rtl::OUString aLabel;

    if ( !m_bModuleIdentified  )
    {
        try
        {
            Reference< XModuleManager > xModuleManager( m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );
            m_aModuleIdentifier = xModuleManager->identify( m_xFrame );
            Reference< XNameAccess > xNameAccess( m_xServiceManager->createInstance(
                                                                    SERVICENAME_UICOMMANDDESCRIPTION ),
                                                                UNO_QUERY );
            Any a = xNameAccess->getByName( m_aModuleIdentifier );
            a >>= m_xUICommandDescription;
        }
        catch ( Exception& )
        {
        }
    }

    if ( m_xUICommandDescription.is() )
    {
        try
        {
            Sequence< PropertyValue > aPropSeq;
            rtl::OUString             aStr;
            Any a( m_xUICommandDescription->getByName( rCommandURL ));
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
        catch ( Exception& )
        {
        }
    }

    return aLabel;
}

static void fillHashMap( const Sequence< Sequence< ::com::sun::star::beans::PropertyValue > >& rSeqToolBars,
                         ToolbarHashMap& rHashMap )
{
    for ( sal_Int32 i = 0; i < rSeqToolBars.getLength(); i++ )
    {
        rtl::OUString aResourceURL;
        rtl::OUString aUIName;
        const PropertyValue* pProperties = rSeqToolBars[i].getConstArray();
        for ( sal_Int32 j = 0; j < rSeqToolBars[i].getLength(); j++ )
        {
            if ( pProperties[j].Name.equalsAscii( ITEM_DESCRIPTOR_RESOURCEURL) )
                pProperties[j].Value >>= aResourceURL;
            else if ( pProperties[j].Name.equalsAscii( ITEM_DESCRIPTOR_UINAME) )
                pProperties[j].Value >>= aUIName;
        }

        if ( aResourceURL.getLength() > 0 &&
             rHashMap.find( aResourceURL ) == rHashMap.end() )
            rHashMap.insert( ToolbarHashMap::value_type( aResourceURL, aUIName ));
    }
}

// private function
void ToolbarsMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    resetPopupMenu( rPopupMenu );

    m_aCommandVector.clear();

    // Retrieve layout manager for additional information
    sal_Int16                       nIndex( 1 );
    Reference< XPropertySet >       xPropSet( m_xFrame, UNO_QUERY );
    Reference< XLayoutManager >     xLayoutManager;
    Reference< awt::XMenuExtended > xMenuExtended( rPopupMenu, UNO_QUERY );
    Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
    aValue >>= xLayoutManager;

    if ( xLayoutManager.is() )
    {
        ToolbarHashMap aToolbarHashMap;
        if ( m_xDocCfgMgr.is() )
        {
            Sequence< Sequence< com::sun::star::beans::PropertyValue > > aSeqDocToolBars =
                m_xDocCfgMgr->getUIElementsInfo( UIElementType::TOOLBAR );
            fillHashMap( aSeqDocToolBars, aToolbarHashMap );
        }

        if ( m_xModuleCfgMgr.is() )
        {
            Sequence< Sequence< com::sun::star::beans::PropertyValue > > aSeqToolBars =
                m_xModuleCfgMgr->getUIElementsInfo( UIElementType::TOOLBAR );
            fillHashMap( aSeqToolBars, aToolbarHashMap );
        }

        ToolbarHashMap::const_iterator pIter = aToolbarHashMap.begin();
        while ( pIter != aToolbarHashMap.end() )
        {
            rtl::OUString aUIName = pIter->second;
            if ( aUIName.getLength() == 0 &&
                 m_xPersistentWindowState.is() )
            {
                try
                {
                    Sequence< PropertyValue > aWindowState;
                    Any                       a( m_xPersistentWindowState->getByName( pIter->first ));

                    if ( a >>= aWindowState )
                    {
                        for ( sal_Int32 i = 0; i < aWindowState.getLength(); i++ )
                        {
                            if ( aWindowState[i].Name.equalsAscii( WINDOWSTATE_PROPERTY_UINAME ))
                                aWindowState[i].Value >>= aUIName;
                        }
                    }
                }
                catch ( Exception& )
                {
                }
            }

            if ( aUIName.getLength() > 0 )
            {
                sal_Bool bVisible = xLayoutManager->isElementVisible( pIter->first );
                USHORT nItemCount = m_xPopupMenu->getItemCount();
                m_xPopupMenu->insertItem( nIndex, aUIName, css::awt::MenuItemStyle::CHECKABLE, nItemCount );
                if ( bVisible )
                    m_xPopupMenu->checkItem( nIndex, sal_True );

                // use VCL popup menu pointer to set vital information that are not part of the awt implementation
                xMenuExtended->setCommand( nIndex, pIter->first ); // Store toolbar resource name
                ++nIndex;
            }
            pIter++;
        }

        // Create commands for non-toolbars
        if ( m_aModuleIdentifier.equalsAscii( "com.sun.star.text.TextDocument" ) ||
             m_aModuleIdentifier.equalsAscii( "com.sun.star.text.WebDocument" ) ||
             m_aModuleIdentifier.equalsAscii( "com.sun.star.text.GlobalDocument" ) ||
             m_aModuleIdentifier.equalsAscii( "com.sun.star.drawing.DrawingDocument" ) ||
             m_aModuleIdentifier.equalsAscii( "com.sun.star.presentation.PresentationDocument" ) ||
             m_aModuleIdentifier.equalsAscii( "com.sun.star.sheet.SpreadsheetDocument" ))
        {
            addCommand( m_xPopupMenu, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CMD_HYPERLINKBAR )), 10360 );
            if ( m_aModuleIdentifier.equalsAscii( "com.sun.star.drawing.DrawingDocument" ) ||
                 m_aModuleIdentifier.equalsAscii( "com.sun.star.presentation.PresentationDocument" ))
                addCommand( m_xPopupMenu, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CMD_COLORBAR )), 10417 );
            else if ( m_aModuleIdentifier.equalsAscii( "com.sun.star.sheet.SpreadsheetDocument" ))
                addCommand( m_xPopupMenu, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CMD_INPUTLINEBAR )), 26241 );
            else
                addCommand( m_xPopupMenu, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CMD_FORMULABAR )), 20128 );
        }

        // Create command for configure
        if ( m_xPopupMenu->getItemCount() > 0 )
        {
            USHORT        nItemCount = m_xPopupMenu->getItemCount();
            m_xPopupMenu->insertSeparator( nItemCount+1 );
        }

        addCommand( m_xPopupMenu, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CONFIGURE_TOOLBARS )), 5904 );
    }
}

// XEventListener
void SAL_CALL ToolbarsMenuController::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xDocCfgMgr.clear();
    m_xModuleCfgMgr.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ToolbarsMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    rtl::OUString aFeatureURL( Event.FeatureURL.Complete );

    ResetableGuard aLock( m_aLock );

    // All other status events will be processed here
    sal_Bool bSetCheckmark      = sal_False;
    sal_Bool bCheckmark         = sal_False;

    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
        VCLXPopupMenu* pXPopupMenu = (VCLXPopupMenu *)VCLXMenu::GetImplementation( m_xPopupMenu );
        PopupMenu*     pVCLPopupMenu = (PopupMenu *)pXPopupMenu->GetMenu();

        for ( USHORT i = 0; i < pVCLPopupMenu->GetItemCount(); i++ )
        {
            USHORT nId = pVCLPopupMenu->GetItemId( i );
            if ( nId == 0 )
                continue;

            rtl::OUString aCmd = pVCLPopupMenu->GetItemCommand( nId );
            if ( aCmd == aFeatureURL )
            {
                // Enable/disable item
                pVCLPopupMenu->EnableItem( nId, Event.IsEnabled );

                // Checkmark
                if ( Event.State >>= bCheckmark )
                    bSetCheckmark = sal_True;

                if ( bSetCheckmark )
                    pVCLPopupMenu->CheckItem( nId, bCheckmark );
                else
                {
                    rtl::OUString aItemText;

                    if ( Event.State >>= aItemText )
                        pVCLPopupMenu->SetItemText( nId, aItemText );
                }
            }
        }
    }
}

// XMenuListener
void SAL_CALL ToolbarsMenuController::highlight( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

void SAL_CALL ToolbarsMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    static const char UNO_CMD[] = ".uno:";

    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;
    Reference< XURLTransformer >        xURLTransformer;
    Reference< XFrame >                 xFrame;

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    xDispatch       = m_xDispatch;
    xServiceManager = m_xServiceManager;
    xURLTransformer = m_xURLTransformer;
    xFrame          = m_xFrame;
    aLock.unlock();

    if ( xPopupMenu.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            css::util::URL               aTargetURL;
            Sequence<PropertyValue>      aArgs( 1 );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

                rtl::OUString aCmd = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
                if ( aCmd.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UNO_CMD ))) == 0 )
                {
                    URL                     aTargetURL;
                    Sequence<PropertyValue> aArgs;

                    aTargetURL.Complete = aCmd;
                    xURLTransformer->parseStrict( aTargetURL );
                    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
                    if ( xDispatchProvider.is() )
                    {
                        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch(
                                                                aTargetURL, ::rtl::OUString(), 0 );

                        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
                        pExecuteInfo->xDispatch     = xDispatch;
                        pExecuteInfo->aTargetURL    = aTargetURL;
                        pExecuteInfo->aArgs         = aArgs;
                        Application::PostUserEvent( STATIC_LINK(0, ToolbarsMenuController, ExecuteHdl_Impl), pExecuteInfo );
                    }
                }
                else
                {
                    Reference< XLayoutManager > xLayoutManager;
                    Reference< XPropertySet > xPropSet( xFrame, UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        try
                        {
                            Any a = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                            if ( a >>= xLayoutManager )
                            {
                                rtl::OUString aToolBarName( aCmd );
                                sal_Bool      bShow( !pVCLPopupMenu->IsItemChecked( rEvent.MenuId ));

                                if ( bShow )
                                {
                                    xLayoutManager->createElement( aToolBarName );
                                    xLayoutManager->showElement( aToolBarName );
                                }
                                else
                                    xLayoutManager->hideElement( aToolBarName );
                            }
                        }
                        catch ( Exception& )
                        {
                        }
                    }
                }
            }
        }
    }
}

void SAL_CALL ToolbarsMenuController::activate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    std::vector< rtl::OUString >   aCmdVector;
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    Reference< XURLTransformer >   xURLTransformer( m_xURLTransformer );
    {
        ResetableGuard aLock( m_aLock );
        fillPopupMenu( m_xPopupMenu );
        aCmdVector = m_aCommandVector;
    }

    // Update status for all commands inside our toolbars popup menu
    for ( sal_uInt32 i=0; i < aCmdVector.size(); i++ )
    {
        URL aTargetURL;
        aTargetURL.Complete = aCmdVector[i];
        xURLTransformer->parseStrict( aTargetURL );
        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        if ( xDispatch.is() )
        {
            xDispatch->addStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
            xDispatch->removeStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
        }
    }
}

void SAL_CALL ToolbarsMenuController::deactivate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL ToolbarsMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));
        fillPopupMenu( m_xPopupMenu );
    }
}

// XInitialization
void SAL_CALL ToolbarsMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
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
            m_bInitialized = sal_True;

            Reference< XModuleManager > xModuleManager( m_xServiceManager->createInstance(
                                                            SERVICENAME_MODULEMANAGER ),
                                                        UNO_QUERY );
            Reference< XNameAccess > xPersistentWindowStateSupplier( m_xServiceManager->createInstance(
                                                                        SERVICENAME_WINDOWSTATECONFIGURATION ),
                                                                     UNO_QUERY );

            // Retrieve persistent window state reference for our module
            if ( xPersistentWindowStateSupplier.is() && xModuleManager.is() )
            {
                rtl::OUString aModuleIdentifier;
                try
                {
                    aModuleIdentifier = xModuleManager->identify( m_xFrame );
                    xPersistentWindowStateSupplier->getByName( aModuleIdentifier ) >>= m_xPersistentWindowState;

                    Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier;
                    if ( m_xServiceManager.is() )
                        xModuleCfgSupplier = Reference< XModuleUIConfigurationManagerSupplier >(
                            m_xServiceManager->createInstance( SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ), UNO_QUERY );
                    m_xModuleCfgMgr = xModuleCfgSupplier->getUIConfigurationManager( aModuleIdentifier );

                    Reference< XController > xController = m_xFrame->getController();
                    Reference< XModel >      xModel;
                    if ( xController.is() )
                        xModel = xController->getModel();
                    if ( xModel.is() )
                    {
                        Reference< XUIConfigurationManagerSupplier > xUIConfigurationManagerSupplier( xModel, UNO_QUERY );
                        if ( xUIConfigurationManagerSupplier.is() )
                            m_xDocCfgMgr = xUIConfigurationManagerSupplier->getUIConfigurationManager();
                    }
                }
                catch ( Exception& )
                {
                }
            }

            m_xURLTransformer = Reference< XURLTransformer >( m_xServiceManager->createInstance(
                                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                        "com.sun.star.util.URLTransformer" ))),
                                                                   UNO_QUERY );
        }
    }
}

IMPL_STATIC_LINK( ToolbarsMenuController, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
    }
    catch ( Exception& )
    {
    }

    delete pExecuteInfo;
    return 0;
}

}
