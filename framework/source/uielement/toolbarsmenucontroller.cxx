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

#include <services.h>
#include <strings.hrc>
#include <classes/fwkresid.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <uiconfiguration/windowstateproperties.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <svtools/menuoptions.hxx>
#include <unotools/cmdoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/syslocale.hxx>

//  Defines

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;

static const char CMD_RESTOREVISIBILITY[]       = ".cmd:RestoreVisibility";

static const char STATIC_CMD_PART[]             = ".uno:AvailableToolbars?Toolbar:string=";
static const char STATIC_INTERNAL_CMD_PART[]    = ".cmd:";

namespace framework
{

typedef std::unordered_map< OUString, OUString > ToolbarHashMap;

struct ToolBarEntry
{
    OUString           aUIName;
    OUString           aCommand;
    bool                bVisible;
    bool                bContextSensitive;
    const CollatorWrapper*  pCollatorWrapper;
};

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
        xPropSet->getPropertyValue("LayoutManager") >>= xLayoutManager;
    }
    catch ( const UnknownPropertyException& )
    {
    }

    return xLayoutManager;
}

struct ToolBarInfo
{
    OUString aToolBarResName;
    OUString aToolBarUIName;
};

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   ToolbarsMenuController                  ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_TOOLBARSMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   ToolbarsMenuController, {} )

static constexpr OUStringLiteral g_aPropUIName( "UIName" );
static constexpr OUStringLiteral g_aPropResourceURL( "ResourceURL" );

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
        aLabel = vcl::CommandInfoProvider::GetMenuLabelForCommand( rCommandURL, m_aModuleName );
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

    Image                aImage;
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();

    if ( rSettings.GetUseImagesInMenus() )
        aImage = vcl::CommandInfoProvider::GetImageForCommand(rCommandURL, m_xFrame);

    VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(VCLXPopupMenu::GetImplementation( rPopupMenu ));
    if ( pPopupMenu )
    {
        PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());
        if ( !!aImage )
            pVCLPopupMenu->SetItemImage( nItemId, aImage );
    }

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
    for ( sal_Int32 i = 0; i < rSeqToolBars.getLength(); i++ )
    {
        OUString aResourceURL;
        OUString aUIName;
        const PropertyValue* pProperties = rSeqToolBars[i].getConstArray();
        for ( sal_Int32 j = 0; j < rSeqToolBars[i].getLength(); j++ )
        {
            if ( pProperties[j].Name == "ResourceURL" )
                pProperties[j].Value >>= aResourceURL;
            else if ( pProperties[j].Name == "UIName" )
                pProperties[j].Value >>= aUIName;
        }

        if ( !aResourceURL.isEmpty() &&
             rHashMap.find( aResourceURL ) == rHashMap.end() )
            rHashMap.emplace( aResourceURL, aUIName );
    }
}

// private function
Sequence< Sequence< css::beans::PropertyValue > > ToolbarsMenuController::getLayoutManagerToolbars( const Reference< css::frame::XLayoutManager >& rLayoutManager )
{
    std::vector< ToolBarInfo > aToolBarArray;
    Sequence< Reference< XUIElement > > aUIElements = rLayoutManager->getElements();
    for ( sal_Int32 i = 0; i < aUIElements.getLength(); i++ )
    {
        Reference< XUIElement > xUIElement( aUIElements[i] );
        Reference< XPropertySet > xPropSet( aUIElements[i], UNO_QUERY );
        if ( xPropSet.is() && xUIElement.is() )
        {
            try
            {
                OUString   aResName;
                sal_Int16       nType( -1 );
                xPropSet->getPropertyValue("Type") >>= nType;
                xPropSet->getPropertyValue("ResourceURL") >>= aResName;

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

    Sequence< css::beans::PropertyValue > aTbSeq( 2 );
    aTbSeq[0].Name = g_aPropUIName;
    aTbSeq[1].Name = g_aPropResourceURL;

    Sequence< Sequence< css::beans::PropertyValue > > aSeq( aToolBarArray.size() );
    const sal_uInt32 nCount = aToolBarArray.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        aTbSeq[0].Value <<= aToolBarArray[i].aToolBarUIName;
        aTbSeq[1].Value <<= aToolBarArray[i].aToolBarResName;
        aSeq[i] = aTbSeq;
    }

    return aSeq;
}


void ToolbarsMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    if( SvtMiscOptions().DisableUICustomization() )
        return;

    SolarMutexGuard aSolarMutexGuard;
    resetPopupMenu( rPopupMenu );

    m_aCommandVector.clear();

    // Retrieve layout manager for additional information
    Reference< XLayoutManager > xLayoutManager( getLayoutManagerFromFrame( m_xFrame ));

    m_bResetActive = false;
    if ( xLayoutManager.is() )
    {
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
                        for ( sal_Int32 i = 0; i < aWindowState.getLength(); i++ )
                        {
                            if ( aWindowState[i].Name == WINDOWSTATE_PROPERTY_UINAME )
                                aWindowState[i].Value >>= aUIName;
                            else if ( aWindowState[i].Name == WINDOWSTATE_PROPERTY_HIDEFROMENU )
                                aWindowState[i].Value >>= bHideFromMenu;
                            else if ( aWindowState[i].Name == WINDOWSTATE_PROPERTY_CONTEXT )
                                aWindowState[i].Value >>= bContextSensitive;
                            else if ( aWindowState[i].Name == WINDOWSTATE_PROPERTY_VISIBLE )
                                aWindowState[i].Value >>= bVisible;
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
                aTbEntry.bContextSensitive = bContextSensitive;
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

            {
                SolarMutexGuard aGuard;
                VCLXPopupMenu* pXPopupMenu = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( m_xPopupMenu ));
                PopupMenu* pVCLPopupMenu = pXPopupMenu ? static_cast<PopupMenu *>(pXPopupMenu->GetMenu()) : nullptr;
                assert(pVCLPopupMenu);
                if (pVCLPopupMenu)
                    pVCLPopupMenu->SetUserValue( nIndex, reinterpret_cast<void*>( aSortedTbs[i].bContextSensitive ? 1 : 0 ));
            }

            // use VCL popup menu pointer to set vital information that are not part of the awt implementation
            OUStringBuffer aStrBuf( aStaticCmdPart );

            sal_Int32 n = aSortedTbs[i].aCommand.lastIndexOf( '/' );
            if (( n > 0 ) && (( n+1 ) < aSortedTbs[i].aCommand.getLength() ))
                aStrBuf.append( std::u16string_view(aSortedTbs[i].aCommand).substr(n+1) );

            OUString aCmd( aStrBuf.makeStringAndClear() );

            // Store complete uno-command so it can also be dispatched. This is necessary to support
            // the test tool!
            rPopupMenu->setCommand( nIndex, aCmd );
            ++nIndex;
        }

        // Create commands for non-toolbars
        if ( m_aModuleIdentifier == "com.sun.star.text.TextDocument" ||
             m_aModuleIdentifier == "com.sun.star.text.WebDocument" ||
             m_aModuleIdentifier == "com.sun.star.text.GlobalDocument" ||
             m_aModuleIdentifier == "com.sun.star.drawing.DrawingDocument" ||
             m_aModuleIdentifier == "com.sun.star.presentation.PresentationDocument" ||
             m_aModuleIdentifier == "com.sun.star.sheet.SpreadsheetDocument" )
        {
            if ( m_aModuleIdentifier == "com.sun.star.drawing.DrawingDocument" ||
                 m_aModuleIdentifier == "com.sun.star.presentation.PresentationDocument" )
                addCommand( m_xPopupMenu, ".uno:ColorControl", "" );
            else if ( m_aModuleIdentifier == "com.sun.star.sheet.SpreadsheetDocument" )
                addCommand( m_xPopupMenu, ".uno:InputLineVisible", "" );
            else
                addCommand( m_xPopupMenu, ".uno:InsertFormula", "" );
        }

        bool          bAddCommand( true );
        SvtCommandOptions aCmdOptions;

        if ( aCmdOptions.HasEntries( SvtCommandOptions::CMDOPTION_DISABLED ))
        {
            if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED,
                                     "ConfigureDialog"))
                bAddCommand = false;
        }

        if ( bAddCommand )
        {
            // Create command for configure
            if ( m_xPopupMenu->getItemCount() > 0 )
            {
                sal_uInt16        nItemCount = m_xPopupMenu->getItemCount();
                m_xPopupMenu->insertSeparator( nItemCount+1 );
            }

            addCommand( m_xPopupMenu, ".uno:ConfigureDialog", "" );
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
        OUString aRestoreCmd( CMD_RESTOREVISIBILITY );
        addCommand( m_xPopupMenu, aRestoreCmd, aLabelStr );
    }
}

// XEventListener
void SAL_CALL ToolbarsMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xDocCfgMgr.clear();
    m_xModuleCfgMgr.clear();
    m_xContext.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ToolbarsMenuController::statusChanged( const FeatureStateEvent& Event )
{
    OUString aFeatureURL( Event.FeatureURL.Complete );

    // All other status events will be processed here
    osl::ClearableMutexGuard aLock( m_aMutex );
    Reference< css::awt::XPopupMenu > xPopupMenu( m_xPopupMenu );
    aLock.clear();

    if ( xPopupMenu.is() )
    {
        SolarMutexGuard aGuard;
        VCLXPopupMenu* pXPopupMenu = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( xPopupMenu ));
        PopupMenu*     pVCLPopupMenu = pXPopupMenu ? static_cast<PopupMenu *>(pXPopupMenu->GetMenu()) : nullptr;

        SAL_WARN_IF(!pVCLPopupMenu, "fwk.uielement", "worrying lack of popup menu");
        if (!pVCLPopupMenu)
            return;

        bool bSetCheckmark      = false;
        bool bCheckmark         = false;
        for ( sal_uInt16 i = 0; i < pVCLPopupMenu->GetItemCount(); i++ )
        {
            sal_uInt16 nId = pVCLPopupMenu->GetItemId( i );
            if ( nId == 0 )
                continue;

            OUString aCmd = pVCLPopupMenu->GetItemCommand( nId );
            if ( aCmd == aFeatureURL )
            {
                // Enable/disable item
                pVCLPopupMenu->EnableItem( nId, Event.IsEnabled );

                // Checkmark
                if ( Event.State >>= bCheckmark )
                    bSetCheckmark = true;

                if ( bSetCheckmark )
                    pVCLPopupMenu->CheckItem( nId, bCheckmark );
                else
                {
                    OUString aItemText;

                    if ( Event.State >>= aItemText )
                        pVCLPopupMenu->SetItemText( nId, aItemText );
                }
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

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu             = m_xPopupMenu;
    xContext               = m_xContext;
    xURLTransformer        = m_xURLTransformer;
    xFrame                 = m_xFrame;
    xPersistentWindowState = m_xPersistentWindowState;
    aLock.clear();

    if ( xPopupMenu.is() )
    {
        VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(VCLXPopupMenu::GetImplementation( xPopupMenu ));
        if ( pPopupMenu )
        {
            SolarMutexGuard aSolarMutexGuard;
            PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

            OUString aCmd( pVCLPopupMenu->GetItemCommand( rEvent.MenuId ));
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
                                        aWindowState[nVisibleIndex].Value <<= true;
                                        xNameReplace->replaceByName( aElementName, makeAny( aWindowState ));
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
                                        xPropSet->setPropertyValue("RefreshContextToolbarVisibility", makeAny( true ));
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
                Sequence<PropertyValue> aArgs;

                aTargetURL.Complete = aCmd;
                xURLTransformer->parseStrict( aTargetURL );
                Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
                if ( xDispatchProvider.is() )
                {
                    Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch(
                                                            aTargetURL, OUString(), 0 );

                    ExecuteInfo* pExecuteInfo = new ExecuteInfo;
                    pExecuteInfo->xDispatch     = xDispatch;
                    pExecuteInfo->aTargetURL    = aTargetURL;
                    pExecuteInfo->aArgs         = aArgs;
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
                        OUStringBuffer aBuf( "private:resource/toolbar/" );
                        aBuf.append( std::u16string_view(aCmd).substr(nIndex+1) );

                        bool      bShow( !pVCLPopupMenu->IsItemChecked( rEvent.MenuId ));
                        OUString aToolBarResName( aBuf.makeStringAndClear() );
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
    }
}

void SAL_CALL ToolbarsMenuController::itemActivated( const css::awt::MenuEvent& )
{
    std::vector< OUString >   aCmdVector;
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    Reference< XURLTransformer >   xURLTransformer( m_xURLTransformer );
    {
        osl::MutexGuard aLock( m_aMutex );
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
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        SolarMutexGuard aSolarMutexGuard;

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( static_cast<OWeakObject*>(this), UNO_QUERY ));
        fillPopupMenu( m_xPopupMenu );
    }
}

// XInitialization
void SAL_CALL ToolbarsMenuController::initialize( const Sequence< Any >& aArguments )
{
    osl::MutexGuard aLock( m_aMutex );
    bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        svt::PopupMenuControllerBase::initialize(aArguments);

        if ( m_bInitialized )
        {
            Reference< XModuleManager2 > xModuleManager = ModuleManager::create( m_xContext );
            Reference< XNameAccess > xPersistentWindowStateSupplier = css::ui::theWindowStateConfiguration::get( m_xContext );

            // Retrieve persistent window state reference for our module
            OUString aModuleIdentifier;
            try
            {
                aModuleIdentifier = xModuleManager->identify( m_xFrame );
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
                m_aModuleIdentifier = aModuleIdentifier;
            }
            catch ( const Exception& )
            {
            }
        }
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
            pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
        }
    }
    catch ( const Exception& )
    {
    }

    delete pExecuteInfo;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
