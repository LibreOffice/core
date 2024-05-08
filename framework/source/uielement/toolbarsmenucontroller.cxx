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

#include <uielement/toolbarsmenucontroller.hxx>

#include <algorithm>
#include <string_view>
#include <unordered_map>

#include <services.h>
#include <strings.hrc>
#include <classes/fwkresid.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <uiconfiguration/windowstateproperties.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>

#include <comphelper/propertyvalue.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <unotools/cmdoptions.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <cppuhelper/supportsservice.hxx>

//  Defines

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;

constexpr OUString CMD_RESTOREVISIBILITY = u".cmd:RestoreVisibility"_ustr;
constexpr OUStringLiteral CMD_LOCKTOOLBARS = u".uno:ToolbarLock";

constexpr OUString STATIC_CMD_PART    = u".uno:AvailableToolbars?Toolbar:string="_ustr;
const char STATIC_INTERNAL_CMD_PART[]    = ".cmd:";

namespace framework
{

typedef std::unordered_map< OUString, OUString > ToolbarHashMap;

namespace {

struct ToolBarEntry
{
    OUString           aUIName;
    OUString           aCommand;
    bool                bVisible;
    const CollatorWrapper*  pCollatorWrapper;
};

}

static bool CompareToolBarEntry( const ToolBarEntry& aOne, const ToolBarEntry& aTwo )
{
    sal_Int32 nComp = aOne.pCollatorWrapper->compareString( aOne.aUIName, aTwo.aUIName );

    return nComp < 0;
}

static Reference< XLayoutManager > getLayoutManagerFromFrame( const Reference< XFrame >& rFrame )
{
    Reference< XPropertySet >   xPropSet( rFrame, UNO_QUERY );
    Reference< XLayoutManager > xLayoutManager;

    try
    {
        xPropSet->getPropertyValue(u"LayoutManager"_ustr) >>= xLayoutManager;
    }
    catch ( const UnknownPropertyException& )
    {
    }

    return xLayoutManager;
}

namespace {

struct ToolBarInfo
{
    OUString aToolBarResName;
    OUString aToolBarUIName;
};

}

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL ToolbarsMenuController::getImplementationName()
{
    return u"com.sun.star.comp.framework.ToolBarsMenuController"_ustr;
}

sal_Bool SAL_CALL ToolbarsMenuController::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ToolbarsMenuController::getSupportedServiceNames()
{
    return { SERVICENAME_POPUPMENUCONTROLLER };
}

constexpr OUString g_aPropUIName( u"UIName"_ustr );
constexpr OUString g_aPropResourceURL( u"ResourceURL"_ustr );

ToolbarsMenuController::ToolbarsMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_xContext( xContext ),
    m_bResetActive( false ),
    m_aIntlWrapper(SvtSysLocale().GetUILanguageTag())
{
}

ToolbarsMenuController::~ToolbarsMenuController()
{
}

void ToolbarsMenuController::addCommand(
    Reference< css::awt::XPopupMenu > const & rPopupMenu, const OUString& rCommandURL, const OUString& rLabel )
{
    sal_uInt16        nItemId    = m_xPopupMenu->getItemCount()+1;

    OUString aLabel;
    if ( rLabel.isEmpty() )
    {
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(rCommandURL, m_aModuleName);
        aLabel = vcl::CommandInfoProvider::GetMenuLabelForCommand(aProperties);
    }
    else
        aLabel = rLabel;

    rPopupMenu->insertItem( nItemId, aLabel, 0, nItemId );
    rPopupMenu->setCommand( nItemId, rCommandURL );

    bool bInternal = rCommandURL.startsWith( STATIC_INTERNAL_CMD_PART );
    if ( !bInternal )
    {
        if ( !getDispatchFromCommandURL( rCommandURL ).is() )
            m_xPopupMenu->enableItem( nItemId, false );
    }

    SolarMutexGuard aSolarMutexGuard;

    css::uno::Reference<css::graphic::XGraphic> xGraphic;
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    if ( rSettings.GetUseImagesInMenus() )
        xGraphic = vcl::CommandInfoProvider::GetXGraphicForCommand(rCommandURL, m_xFrame);

    if (xGraphic.is())
        rPopupMenu->setItemImage(nItemId, xGraphic, false);

    m_aCommandVector.push_back( rCommandURL );
}

Reference< XDispatch > ToolbarsMenuController::getDispatchFromCommandURL( const OUString& rCommandURL )
{
    URL                          aTargetURL;
    Reference< XURLTransformer > xURLTransformer;
    Reference< XFrame >          xFrame;

    {
        SolarMutexGuard aSolarMutexGuard;
        xURLTransformer = m_xURLTransformer;
        xFrame = m_xFrame;
    }

    aTargetURL.Complete = rCommandURL;
    xURLTransformer->parseStrict( aTargetURL );
    Reference< XDispatchProvider > xDispatchProvider( xFrame, UNO_QUERY );
    if ( xDispatchProvider.is() )
        return xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
    else
        return Reference< XDispatch >();
}

static void fillHashMap( const Sequence< Sequence< css::beans::PropertyValue > >& rSeqToolBars,
                         ToolbarHashMap& rHashMap )
{
    for ( Sequence< css::beans::PropertyValue > const & props : rSeqToolBars )
    {
        OUString aResourceURL;
        OUString aUIName;
        for ( css::beans::PropertyValue const & prop : props )
        {
            if ( prop.Name == "ResourceURL" )
                prop.Value >>= aResourceURL;
            else if ( prop.Name == "UIName" )
                prop.Value >>= aUIName;
        }

        if ( !aResourceURL.isEmpty() &&
             rHashMap.find( aResourceURL ) == rHashMap.end() )
            rHashMap.emplace( aResourceURL, aUIName );
    }
}

// private function
// static
Sequence< Sequence< css::beans::PropertyValue > > ToolbarsMenuController::getLayoutManagerToolbars( const Reference< css::frame::XLayoutManager >& rLayoutManager )
{
    std::vector< ToolBarInfo > aToolBarArray;
    const Sequence< Reference< XUIElement > > aUIElements = rLayoutManager->getElements();
    for ( Reference< XUIElement > const & xUIElement : aUIElements )
    {
        Reference< XPropertySet > xPropSet( xUIElement, UNO_QUERY );
        if ( xPropSet.is() && xUIElement.is() )
        {
            try
            {
                OUString   aResName;
                sal_Int16       nType( -1 );
                xPropSet->getPropertyValue(u"Type"_ustr) >>= nType;
                xPropSet->getPropertyValue(u"ResourceURL"_ustr) >>= aResName;

                if (( nType == css::ui::UIElementType::TOOLBAR ) &&
                    !aResName.isEmpty() )
                {
                    ToolBarInfo aToolBarInfo;

                    aToolBarInfo.aToolBarResName = aResName;

                    SolarMutexGuard aGuard;
                    Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow )
                        aToolBarInfo.aToolBarUIName = pWindow->GetText();

                    aToolBarArray.push_back( aToolBarInfo );
                }
            }
            catch ( const Exception& )
            {
            }
        }
    }

    Sequence< Sequence< css::beans::PropertyValue > > aSeq( aToolBarArray.size() );
    auto pSeq = aSeq.getArray();
    const sal_uInt32 nCount = aToolBarArray.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        Sequence< css::beans::PropertyValue > aTbSeq{
            comphelper::makePropertyValue(g_aPropUIName, aToolBarArray[i].aToolBarUIName),
            comphelper::makePropertyValue(g_aPropResourceURL, aToolBarArray[i].aToolBarResName)
        };
        pSeq[i] = aTbSeq;
    }

    return aSeq;
}


void ToolbarsMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    if( officecfg::Office::Common::Misc::DisableUICustomization::get() )
        return;

    SolarMutexGuard aSolarMutexGuard;
    resetPopupMenu( rPopupMenu );

    m_aCommandVector.clear();

    // Retrieve layout manager for additional information
    Reference< XLayoutManager > xLayoutManager( getLayoutManagerFromFrame( m_xFrame ));

    m_bResetActive = false;
    if ( !xLayoutManager.is() )
        return;

    ToolbarHashMap aToolbarHashMap;

    if ( m_xDocCfgMgr.is() )
    {
        Sequence< Sequence< css::beans::PropertyValue > > aSeqDocToolBars =
            m_xDocCfgMgr->getUIElementsInfo( UIElementType::TOOLBAR );
        fillHashMap( aSeqDocToolBars, aToolbarHashMap );
    }

    if ( m_xModuleCfgMgr.is() )
    {
        Sequence< Sequence< css::beans::PropertyValue > > aSeqToolBars =
            m_xModuleCfgMgr->getUIElementsInfo( UIElementType::TOOLBAR );
        fillHashMap( aSeqToolBars, aToolbarHashMap );
    }

    std::vector< ToolBarEntry > aSortedTbs;
    OUString               aStaticCmdPart( STATIC_CMD_PART );

    Sequence< Sequence< css::beans::PropertyValue > > aSeqFrameToolBars = getLayoutManagerToolbars( xLayoutManager );
    fillHashMap( aSeqFrameToolBars, aToolbarHashMap );

    for (auto const& toolbar : aToolbarHashMap)
    {
        OUString aUIName = toolbar.second;
        bool      bHideFromMenu( false );
        bool      bContextSensitive( false );
        if ( aUIName.isEmpty() &&
             m_xPersistentWindowState.is() )
        {
            bool bVisible( false );

            try
            {
                Sequence< PropertyValue > aWindowState;
                Any                       a( m_xPersistentWindowState->getByName( toolbar.first ));

                if ( a >>= aWindowState )
                {
                    for (PropertyValue const& prop : aWindowState)
                    {
                        if ( prop.Name == WINDOWSTATE_PROPERTY_UINAME )
                            prop.Value >>= aUIName;
                        else if ( prop.Name == WINDOWSTATE_PROPERTY_HIDEFROMENU )
                            prop.Value >>= bHideFromMenu;
                        else if ( prop.Name == WINDOWSTATE_PROPERTY_CONTEXT )
                            prop.Value >>= bContextSensitive;
                        else if ( prop.Name == WINDOWSTATE_PROPERTY_VISIBLE )
                            prop.Value >>= bVisible;
                    }
                }
            }
            catch ( const Exception& )
            {
            }

            // Check if we have to enable/disable "Reset" menu item
            if ( bContextSensitive && !bVisible )
                m_bResetActive = true;

        }

        if ( !aUIName.isEmpty() && !bHideFromMenu )
        {
            ToolBarEntry aTbEntry;
            aTbEntry.aUIName = aUIName;
            aTbEntry.aCommand = toolbar.first;
            aTbEntry.bVisible = xLayoutManager->isElementVisible( toolbar.first );
            aTbEntry.pCollatorWrapper = m_aIntlWrapper.getCaseCollator();
            aSortedTbs.push_back( aTbEntry );
        }
    }

    // sort toolbars
    std::sort( aSortedTbs.begin(), aSortedTbs.end(), CompareToolBarEntry );

    sal_Int16 nIndex( 1 );
    const sal_uInt32 nCount = aSortedTbs.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        sal_uInt16 nItemCount = m_xPopupMenu->getItemCount();
        m_xPopupMenu->insertItem( nIndex, aSortedTbs[i].aUIName, css::awt::MenuItemStyle::CHECKABLE, nItemCount );
        if ( aSortedTbs[i].bVisible )
            m_xPopupMenu->checkItem( nIndex, true );

        OUStringBuffer aStrBuf( aStaticCmdPart );

        sal_Int32 n = aSortedTbs[i].aCommand.lastIndexOf( '/' );
        if (( n > 0 ) && (( n+1 ) < aSortedTbs[i].aCommand.getLength() ))
            aStrBuf.append( aSortedTbs[i].aCommand.subView(n+1) );

        OUString aCmd( aStrBuf.makeStringAndClear() );

        // Store complete uno-command so it can also be dispatched. This is necessary to support
        // the test tool!
        rPopupMenu->setCommand( nIndex, aCmd );
        ++nIndex;
    }

    // Create commands for non-toolbars

    bool          bAddCommand( true );
    SvtCommandOptions aCmdOptions;

    if ( aCmdOptions.HasEntriesDisabled() && aCmdOptions.LookupDisabled(u"ConfigureDialog"_ustr))
        bAddCommand = false;

    if ( bAddCommand )
    {
        // Create command for configure
        if ( m_xPopupMenu->getItemCount() > 0 )
        {
            sal_uInt16        nItemCount = m_xPopupMenu->getItemCount();
            m_xPopupMenu->insertSeparator( nItemCount+1 );
        }

        addCommand( m_xPopupMenu, u".uno:ConfigureDialog"_ustr, u""_ustr );
    }

    // Add separator if no configure has been added
    if ( !bAddCommand )
    {
        // Create command for configure
        if ( m_xPopupMenu->getItemCount() > 0 )
        {
            sal_uInt16        nItemCount = m_xPopupMenu->getItemCount();
            m_xPopupMenu->insertSeparator( nItemCount+1 );
        }
    }

    OUString aLabelStr(FwkResId(STR_RESTORE_TOOLBARS));
    addCommand( m_xPopupMenu, CMD_RESTOREVISIBILITY, aLabelStr );
    aLabelStr = FwkResId(STR_LOCK_TOOLBARS);
    addCommand( m_xPopupMenu, CMD_LOCKTOOLBARS, aLabelStr );
}

// XEventListener
void SAL_CALL ToolbarsMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xDocCfgMgr.clear();
    m_xModuleCfgMgr.clear();
    m_xContext.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ToolbarsMenuController::statusChanged( const FeatureStateEvent& Event )
{
    OUString aFeatureURL( Event.FeatureURL.Complete );

    // All other status events will be processed here
    std::unique_lock aLock( m_aMutex );
    Reference< css::awt::XPopupMenu > xPopupMenu( m_xPopupMenu );
    aLock.unlock();

    if ( !xPopupMenu.is() )
        return;

    SolarMutexGuard aGuard;

    bool bSetCheckmark      = false;
    bool bCheckmark         = false;
    for (sal_Int16 i = 0, nCount = xPopupMenu->getItemCount(); i < nCount; ++i)
    {
        sal_Int16 nId = xPopupMenu->getItemId(i);
        if ( nId == 0 )
            continue;

        OUString aCmd = xPopupMenu->getCommand(nId);
        if ( aCmd == aFeatureURL )
        {
            // Enable/disable item
            xPopupMenu->enableItem(nId, Event.IsEnabled);

            // Checkmark
            if ( Event.State >>= bCheckmark )
                bSetCheckmark = true;

            if ( bSetCheckmark )
                xPopupMenu->checkItem(nId, bCheckmark);
            else
            {
                OUString aItemText;

                if ( Event.State >>= aItemText )
                    xPopupMenu->setItemText(nId, aItemText);
            }
        }
    }
}

// XMenuListener
void SAL_CALL ToolbarsMenuController::itemSelected( const css::awt::MenuEvent& rEvent )
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XComponentContext >      xContext;
    Reference< XURLTransformer >        xURLTransformer;
    Reference< XFrame >                 xFrame;
    Reference< XNameAccess >            xPersistentWindowState;

    {
        std::unique_lock aLock(m_aMutex);
        xPopupMenu = m_xPopupMenu;
        xContext = m_xContext;
        xURLTransformer = m_xURLTransformer;
        xFrame = m_xFrame;
        xPersistentWindowState = m_xPersistentWindowState;
    }

    if ( !xPopupMenu.is() )
        return;

    SolarMutexGuard aSolarMutexGuard;

    OUString aCmd(xPopupMenu->getCommand(rEvent.MenuId));
    if ( aCmd.startsWith( STATIC_INTERNAL_CMD_PART ) )
    {
        // Command to restore the visibility of all context sensitive toolbars
        Reference< XNameReplace > xNameReplace( xPersistentWindowState, UNO_QUERY );
        if ( xPersistentWindowState.is() && xNameReplace.is() )
        {
            try
            {
                Sequence< OUString > aElementNames = xPersistentWindowState->getElementNames();
                sal_Int32 nCount = aElementNames.getLength();
                bool      bRefreshToolbars( false );

                for ( sal_Int32 i = 0; i < nCount; i++ )
                {
                    try
                    {
                        OUString aElementName = aElementNames[i];
                        Sequence< PropertyValue > aWindowState;

                        if ( xPersistentWindowState->getByName( aElementName ) >>= aWindowState )
                        {
                            bool  bVisible( false );
                            bool  bContextSensitive( false );
                            sal_Int32 nVisibleIndex( -1 );
                            for ( sal_Int32 j = 0; j < aWindowState.getLength(); j++ )
                            {
                                if ( aWindowState[j].Name == WINDOWSTATE_PROPERTY_VISIBLE )
                                {
                                    aWindowState[j].Value >>= bVisible;
                                    nVisibleIndex = j;
                                }
                                else if ( aWindowState[j].Name == WINDOWSTATE_PROPERTY_CONTEXT )
                                    aWindowState[j].Value >>= bContextSensitive;
                            }

                            if ( !bVisible && bContextSensitive && nVisibleIndex >= 0 )
                            {
                                // Default is: Every context sensitive toolbar is visible
                                aWindowState.getArray()[nVisibleIndex].Value <<= true;
                                xNameReplace->replaceByName( aElementName, Any( aWindowState ));
                                bRefreshToolbars = true;
                            }
                        }
                    }
                    catch ( const NoSuchElementException& )
                    {
                    }
                }

                if ( bRefreshToolbars )
                {
                    Reference< XLayoutManager > xLayoutManager( getLayoutManagerFromFrame( xFrame ));
                    if ( xLayoutManager.is() )
                    {
                        Reference< XPropertySet > xPropSet( xLayoutManager, UNO_QUERY );
                        if ( xPropSet.is() )
                        {
                            try
                            {
                                xPropSet->setPropertyValue(u"RefreshContextToolbarVisibility"_ustr, Any( true ));
                            }
                            catch ( const RuntimeException& )
                            {
                            }
                            catch ( const Exception& )
                            {
                            }
                        }
                    }
                    RefreshToolbars( xFrame );
                }
            }
            catch ( const RuntimeException& )
            {
                throw;
            }
            catch ( const Exception& )
            {
            }
        }
    }
    else if ( aCmd.indexOf( STATIC_CMD_PART ) < 0 )
    {
        URL                     aTargetURL;

        aTargetURL.Complete = aCmd;
        xURLTransformer->parseStrict( aTargetURL );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( xDispatchProvider.is() )
        {
            ExecuteInfo* pExecuteInfo = new ExecuteInfo;
            pExecuteInfo->xDispatch = xDispatchProvider->queryDispatch(aTargetURL, OUString(), 0);
            pExecuteInfo->aTargetURL = aTargetURL;
            Application::PostUserEvent( LINK(nullptr, ToolbarsMenuController, ExecuteHdl_Impl), pExecuteInfo );
        }
    }
    else
    {
        Reference< XLayoutManager > xLayoutManager( getLayoutManagerFromFrame( xFrame ));
        if ( xLayoutManager.is() )
        {
            // Extract toolbar name from the combined uno-command.
            sal_Int32 nIndex = aCmd.indexOf( '=' );
            if (( nIndex > 0 ) && (( nIndex+1 ) < aCmd.getLength() ))
            {
                OUString aToolBarResName = OUString::Concat("private:resource/toolbar/") + aCmd.subView(nIndex+1);

                const bool bShow(!xPopupMenu->isItemChecked(rEvent.MenuId));
                if ( bShow )
                {
                    xLayoutManager->createElement( aToolBarResName );
                    xLayoutManager->showElement( aToolBarResName );
                }
                else
                {
                    // closing means:
                    // hide and destroy element
                    xLayoutManager->hideElement( aToolBarResName );
                    xLayoutManager->destroyElement( aToolBarResName );
                }
            }
        }
    }
}

void SAL_CALL ToolbarsMenuController::itemActivated( const css::awt::MenuEvent& )
{
    std::vector< OUString >   aCmdVector;
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    Reference< XURLTransformer >   xURLTransformer( m_xURLTransformer );
    {
        std::unique_lock aLock( m_aMutex );
        fillPopupMenu( m_xPopupMenu );
        aCmdVector = m_aCommandVector;
    }

    // Update status for all commands inside our toolbars popup menu
    const sal_uInt32 nCount = aCmdVector.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        bool bInternal = aCmdVector[i].startsWith( STATIC_INTERNAL_CMD_PART );

        if ( !bInternal )
        {
            URL aTargetURL;
            aTargetURL.Complete = aCmdVector[i];
            xURLTransformer->parseStrict( aTargetURL );
            Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
            if ( xDispatch.is() )
            {
                xDispatch->addStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
                xDispatch->removeStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
            }
        }
        else if ( aCmdVector[i] == CMD_RESTOREVISIBILITY )
        {
            // Special code to determine the enable/disable state of this command
            FeatureStateEvent aFeatureStateEvent;
            aFeatureStateEvent.FeatureURL.Complete = aCmdVector[i];
            aFeatureStateEvent.IsEnabled = m_bResetActive; // is context sensitive toolbar non visible
            statusChanged( aFeatureStateEvent );
        }
    }
}

// XPopupMenuController
void SAL_CALL ToolbarsMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu )
{
    std::unique_lock aLock( m_aMutex );

    throwIfDisposed(aLock);

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        SolarMutexGuard aSolarMutexGuard;

        m_xPopupMenu = dynamic_cast<VCLXPopupMenu*>(xPopupMenu.get());
        assert(bool(xPopupMenu) == bool(m_xPopupMenu) && "we only support VCLXPopupMenu");
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >(this) );
        fillPopupMenu( m_xPopupMenu );
    }
}

// XInitialization
void ToolbarsMenuController::initializeImpl( std::unique_lock<std::mutex>& rGuard, const Sequence< Any >& aArguments )
{
    bool bInitialized( m_bInitialized );
    if ( bInitialized )
        return;

    svt::PopupMenuControllerBase::initializeImpl(rGuard, aArguments);

    if ( !m_bInitialized )
        return;

    Reference< XModuleManager2 > xModuleManager = ModuleManager::create( m_xContext );
    Reference< XNameAccess > xPersistentWindowStateSupplier = css::ui::theWindowStateConfiguration::get( m_xContext );

    // Retrieve persistent window state reference for our module
    try
    {
        OUString aModuleIdentifier = xModuleManager->identify( m_xFrame );
        xPersistentWindowStateSupplier->getByName( aModuleIdentifier ) >>= m_xPersistentWindowState;

        Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier =
            theModuleUIConfigurationManagerSupplier::get( m_xContext );
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
    catch ( const Exception& )
    {
    }
}

IMPL_STATIC_LINK( ToolbarsMenuController, ExecuteHdl_Impl, void*, p, void )
{
    ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        if ( pExecuteInfo->xDispatch.is() )
        {
            pExecuteInfo->xDispatch->dispatch(pExecuteInfo->aTargetURL, Sequence<PropertyValue>());
        }
    }
    catch ( const Exception& )
    {
    }

    delete pExecuteInfo;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_ToolbarsMenuController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::ToolbarsMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
