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

#include <sal/config.h>

#include <cassert>

#include <uielement/toolbarmanager.hxx>

#include <uielement/generictoolbarcontroller.hxx>
#include "services.h"
#include "general.h"
#include "properties.h"
#include <framework/sfxhelperfunctions.hxx>
#include <classes/fwkresid.hxx>
#include <classes/resource.hrc>
#include <framework/addonsoptions.hxx>
#include <uielement/toolbarmerger.hxx>

#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/theToolbarControllerFactory.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <unotools/cmdoptions.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>

#include <svtools/menuoptions.hxx>

//  namespaces

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star;

namespace framework
{

static const char ITEM_DESCRIPTOR_COMMANDURL[] = "CommandURL";
static const char ITEM_DESCRIPTOR_VISIBLE[]    = "IsVisible";

static const sal_uInt16 STARTID_CUSTOMIZE_POPUPMENU = 1000;

static css::uno::Reference< css::frame::XLayoutManager > getLayoutManagerFromFrame(
    css::uno::Reference< css::frame::XFrame >& rFrame )
{
    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;

    Reference< XPropertySet > xPropSet( rFrame, UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            xPropSet->getPropertyValue("LayoutManager") >>= xLayoutManager;
        }
        catch (const RuntimeException&)
        {
            throw;
        }
        catch (const Exception&)
        {
        }
    }

    return xLayoutManager;
}
namespace
{

sal_Int16 getCurrentImageType()
{
    SvtMiscOptions aMiscOptions;
    sal_Int16 nImageType = css::ui::ImageType::SIZE_DEFAULT;
    if (aMiscOptions.GetCurrentSymbolsSize() == SFX_SYMBOLS_SIZE_LARGE)
        nImageType |= css::ui::ImageType::SIZE_LARGE;
    else if (aMiscOptions.GetCurrentSymbolsSize() == SFX_SYMBOLS_SIZE_32)
        nImageType |= css::ui::ImageType::SIZE_32;
    return nImageType;
}

} // end anonymous namespace

//  XInterface, XTypeProvider, XServiceInfo

ToolBarManager::ToolBarManager( const Reference< XComponentContext >& rxContext,
                                const Reference< XFrame >& rFrame,
                                const OUString& rResourceName,
                                ToolBox* pToolBar ) :
    m_bDisposed( false ),
    m_bAddedToTaskPaneList( true ),
    m_bFrameActionRegistered( false ),
    m_bUpdateControllers( false ),
    m_eSymbolSize(SvtMiscOptions().GetCurrentSymbolsSize()),
    m_pToolBar( pToolBar ),
    m_aResourceName( rResourceName ),
    m_xFrame( rFrame ),
    m_aListenerContainer( m_mutex ),
    m_xContext( rxContext ),
    m_sIconTheme( SvtMiscOptions().GetIconTheme() )
{
    OSL_ASSERT( m_xContext.is() );

    vcl::Window* pWindow = m_pToolBar;
    while ( pWindow && !pWindow->IsSystemWindow() )
        pWindow = pWindow->GetParent();

    if ( pWindow )
        static_cast<SystemWindow *>(pWindow)->GetTaskPaneList()->AddWindow( m_pToolBar );

    m_xToolbarControllerFactory = frame::theToolbarControllerFactory::get( m_xContext );
    m_xURLTransformer = URLTransformer::create( m_xContext );

    m_pToolBar->SetSelectHdl( LINK( this, ToolBarManager, Select) );
    m_pToolBar->SetClickHdl( LINK( this, ToolBarManager, Click ) );
    m_pToolBar->SetDropdownClickHdl( LINK( this, ToolBarManager, DropdownClick ) );
    m_pToolBar->SetDoubleClickHdl( LINK( this, ToolBarManager, DoubleClick ) );
    m_pToolBar->SetStateChangedHdl( LINK( this, ToolBarManager, StateChanged ) );
    m_pToolBar->SetDataChangedHdl( LINK( this, ToolBarManager, DataChanged ) );

    if (m_eSymbolSize == SFX_SYMBOLS_SIZE_LARGE)
        m_pToolBar->SetToolboxButtonSize(ToolBoxButtonSize::Large);
    else if (m_eSymbolSize == SFX_SYMBOLS_SIZE_32)
        m_pToolBar->SetToolboxButtonSize(ToolBoxButtonSize::Size32);
    else
        m_pToolBar->SetToolboxButtonSize(ToolBoxButtonSize::Small);

    // enables a menu for clipped items and customization
    SvtCommandOptions aCmdOptions;
    ToolBoxMenuType nMenuType = ToolBoxMenuType::ClippedItems;
    if ( !aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, "CreateDialog"))
         nMenuType |= ToolBoxMenuType::Customize;

    m_pToolBar->SetCommandHdl( LINK( this, ToolBarManager, Command ) );
    m_pToolBar->SetMenuType( nMenuType );
    m_pToolBar->SetMenuButtonHdl( LINK( this, ToolBarManager, MenuButton ) );

    // set name for testtool, the useful part is after the last '/'
    sal_Int32 idx = rResourceName.lastIndexOf('/');
    idx++; // will become 0 if '/' not found: use full string
    OString  aHelpIdAsString( ".HelpId:" );
    OUString  aToolbarName = rResourceName.copy( idx );
    aHelpIdAsString += OUStringToOString( aToolbarName, RTL_TEXTENCODING_UTF8 );
    m_pToolBar->SetHelpId( aHelpIdAsString );

    m_aAsyncUpdateControllersTimer.SetTimeout( 50 );
    m_aAsyncUpdateControllersTimer.SetInvokeHandler( LINK( this, ToolBarManager, AsyncUpdateControllersHdl ) );
    m_aAsyncUpdateControllersTimer.SetDebugName( "framework::ToolBarManager m_aAsyncUpdateControllersTimer" );

    SvtMiscOptions().AddListenerLink( LINK( this, ToolBarManager, MiscOptionsChanged ) );
}

ToolBarManager::~ToolBarManager()
{
    assert(!m_aAsyncUpdateControllersTimer.IsActive());
    assert(!m_pToolBar); // must be disposed by ToolbarLayoutManager
    OSL_ASSERT( !m_bAddedToTaskPaneList );
}

void ToolBarManager::Destroy()
{
    OSL_ASSERT( m_pToolBar != nullptr );
    SolarMutexGuard g;
    if ( m_bAddedToTaskPaneList )
    {
        vcl::Window* pWindow = m_pToolBar;
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
            static_cast<SystemWindow *>(pWindow)->GetTaskPaneList()->RemoveWindow( m_pToolBar );
        m_bAddedToTaskPaneList = false;
    }

    // Delete the additional add-ons data
    for ( sal_uInt16 i = 0; i < m_pToolBar->GetItemCount(); i++ )
    {
        sal_uInt16 nItemId = m_pToolBar->GetItemId( i );
        if ( nItemId > 0 )
            delete static_cast< AddonsParams* >( m_pToolBar->GetItemData( nItemId ));
    }

    // Hide toolbar as lazy delete can destroy the toolbar much later.
    m_pToolBar->Hide();
    /* #i99167# removed change for i93173 since there is some weird crash */
        // #i93173# delete toolbar lazily as we can still be in one of its handlers
    m_pToolBar->doLazyDelete();

    m_pToolBar->SetSelectHdl( Link<ToolBox *, void>() );
    m_pToolBar->SetActivateHdl( Link<ToolBox *, void>() );
    m_pToolBar->SetDeactivateHdl( Link<ToolBox *, void>() );
    m_pToolBar->SetClickHdl( Link<ToolBox *, void>() );
    m_pToolBar->SetDropdownClickHdl( Link<ToolBox *, void>() );
    m_pToolBar->SetDoubleClickHdl( Link<ToolBox *, void>() );
    m_pToolBar->SetStateChangedHdl( Link<StateChangedType const *, void>() );
    m_pToolBar->SetDataChangedHdl( Link<DataChangedEvent const *, void>() );
    m_pToolBar->SetCommandHdl( Link<CommandEvent const *, void>() );

    m_pToolBar.clear();

    SvtMiscOptions().RemoveListenerLink( LINK( this, ToolBarManager, MiscOptionsChanged ) );
}

ToolBox* ToolBarManager::GetToolBar() const
{
    SolarMutexGuard g;
    return m_pToolBar;
}

void ToolBarManager::CheckAndUpdateImages()
{
    SolarMutexGuard g;
    bool bRefreshImages = false;

    SvtMiscOptions aMiscOptions;
    sal_Int16 eNewSymbolSize = aMiscOptions.GetCurrentSymbolsSize();

    if (m_eSymbolSize != eNewSymbolSize )
    {
        bRefreshImages = true;
        m_eSymbolSize = eNewSymbolSize;
    }

    const OUString& sCurrentIconTheme = aMiscOptions.GetIconTheme();
    if ( m_sIconTheme != sCurrentIconTheme )
    {
        bRefreshImages = true;
        m_sIconTheme = sCurrentIconTheme;
    }

    // Refresh images if requested
    if ( bRefreshImages )
        RefreshImages();
}

void ToolBarManager::RefreshImages()
{
    SolarMutexGuard g;

    vcl::ImageType eImageType = vcl::ImageType::Size16;

    if (m_eSymbolSize == SFX_SYMBOLS_SIZE_LARGE)
    {
        m_pToolBar->SetToolboxButtonSize(ToolBoxButtonSize::Large);
        eImageType = vcl::ImageType::Size26;
    }
    else if (m_eSymbolSize == SFX_SYMBOLS_SIZE_32)
    {
        eImageType = vcl::ImageType::Size32;
        m_pToolBar->SetToolboxButtonSize(ToolBoxButtonSize::Size32);
    }
    else
    {
        m_pToolBar->SetToolboxButtonSize(ToolBoxButtonSize::Small);
    }

    for ( auto const& it : m_aControllerMap )
    {
        Reference< XSubToolbarController > xController( it.second, UNO_QUERY );
        if ( xController.is() && xController->opensSubToolbar() )
        {
            // The button should show the last function that was selected from the
            // dropdown. The controller should know better than us what it was.
            xController->updateImage();
        }
        else
        {
            OUString aCommandURL = m_pToolBar->GetItemCommand( it.first );
            Image aImage = vcl::CommandInfoProvider::Instance().GetImageForCommand(aCommandURL, m_xFrame, eImageType);
            // Try also to query for add-on images before giving up and use an
            // empty image.
            bool bBigImages = eImageType != vcl::ImageType::Size16;
            if ( !aImage )
                aImage = framework::AddonsOptions().GetImageFromURL(aCommandURL, bBigImages);
            m_pToolBar->SetItemImage( it.first, aImage );
        }
    }

    ::Size aSize = m_pToolBar->CalcWindowSizePixel();
    m_pToolBar->SetOutputSizePixel( aSize );
}

void ToolBarManager::UpdateControllers()
{

    if( SvtMiscOptions().DisableUICustomization() )
    {
        Any a;
        Reference< XLayoutManager > xLayoutManager;
        Reference< XPropertySet > xFramePropSet( m_xFrame, UNO_QUERY );
        if ( xFramePropSet.is() )
            a = xFramePropSet->getPropertyValue("LayoutManager");
        a >>= xLayoutManager;
        Reference< XDockableWindow > xDockable( VCLUnoHelper::GetInterface( m_pToolBar ), UNO_QUERY );
        if ( xLayoutManager.is() && xDockable.is() )
        {
            css::awt::Point aPoint;
            aPoint.X = aPoint.Y = SAL_MAX_INT32;
            xLayoutManager->dockWindow( m_aResourceName, DockingArea_DOCKINGAREA_DEFAULT, aPoint );
            xLayoutManager->lockWindow( m_aResourceName );
        }
    }

    if ( !m_bUpdateControllers )
    {
        m_bUpdateControllers = true;
        ToolBarControllerMap::iterator pIter = m_aControllerMap.begin();

        while ( pIter != m_aControllerMap.end() )
        {
            try
            {
                Reference< XUpdatable > xUpdatable( pIter->second, UNO_QUERY );
                if ( xUpdatable.is() )
                    xUpdatable->update();
            }
            catch (const Exception&)
            {
            }
            ++pIter;
        }
    }
    m_bUpdateControllers = false;
}

//for update toolbar controller via Support Visible
void ToolBarManager::UpdateController( const css::uno::Reference< css::frame::XToolbarController >& xController)
{

    if ( !m_bUpdateControllers )
    {
        m_bUpdateControllers = true;
        try
        {   if(xController.is())
            {
                Reference< XUpdatable > xUpdatable( xController, UNO_QUERY );
                if ( xUpdatable.is() )
                    xUpdatable->update();
            }
         }
         catch (const Exception&)
         {
         }

    }
    m_bUpdateControllers = false;
}

void ToolBarManager::frameAction( const FrameActionEvent& Action )
{
    SolarMutexGuard g;
    if ( Action.Action == FrameAction_CONTEXT_CHANGED && !m_bDisposed )
    {
        m_aAsyncUpdateControllersTimer.Start();
    }
}

void SAL_CALL ToolBarManager::disposing( const EventObject& Source )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return;

    RemoveControllers();

    if ( m_xDocImageManager.is() )
    {
        try
        {
            m_xDocImageManager->removeConfigurationListener(
                Reference< XUIConfigurationListener >(
                    static_cast< OWeakObject* >( this ), UNO_QUERY ));
        }
        catch (const Exception&)
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
        catch (const Exception&)
        {
        }
    }

    m_xDocImageManager.clear();
    m_xModuleImageManager.clear();

    if ( Source.Source == Reference< XInterface >( m_xFrame, UNO_QUERY ))
        m_xFrame.clear();

    m_xContext.clear();
}

// XComponent
void SAL_CALL ToolBarManager::dispose()
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        SolarMutexGuard g;

        if (m_bDisposed)
        {
            return;
        }

        RemoveControllers();

        if ( m_xDocImageManager.is() )
        {
            try
            {
                m_xDocImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch (const Exception&)
            {
            }
        }
        m_xDocImageManager.clear();
        if ( m_xModuleImageManager.is() )
        {
            try
            {
                m_xModuleImageManager->removeConfigurationListener(
                    Reference< XUIConfigurationListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch (const Exception&)
            {
            }
        }
        m_xModuleImageManager.clear();

        ImplClearPopupMenu( m_pToolBar );

        // We have to destroy our toolbar instance now.
        Destroy();
        m_pToolBar.clear();

        if ( m_bFrameActionRegistered && m_xFrame.is() )
        {
            try
            {
                m_xFrame->removeFrameActionListener( Reference< XFrameActionListener >(
                                                        static_cast< ::cppu::OWeakObject *>( this ), UNO_QUERY ));
            }
            catch (const Exception&)
            {
            }
        }

        m_xFrame.clear();
        m_xContext.clear();

        // stop timer to prevent timer events after dispose
        // do it last because other calls could restart timer in StateChanged()
        m_aAsyncUpdateControllersTimer.Stop();

        m_bDisposed = true;
    }
}

void SAL_CALL ToolBarManager::addEventListener( const Reference< XEventListener >& xListener )
{
    SolarMutexGuard g;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    m_aListenerContainer.addInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

void SAL_CALL ToolBarManager::removeEventListener( const Reference< XEventListener >& xListener )
{
    m_aListenerContainer.removeInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

// XUIConfigurationListener
void SAL_CALL ToolBarManager::elementInserted( const css::ui::ConfigurationEvent& Event )
{
    impl_elementChanged(false,Event);
}

void SAL_CALL ToolBarManager::elementRemoved( const css::ui::ConfigurationEvent& Event )
{
    impl_elementChanged(true,Event);
}
void ToolBarManager::impl_elementChanged(bool const isRemove,
        const css::ui::ConfigurationEvent& Event)
{
    SolarMutexGuard g;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        return;

    Reference< XNameAccess > xNameAccess;
    sal_Int16                nImageType = sal_Int16();
    sal_Int16                nCurrentImageType = getCurrentImageType();

    if (( Event.aInfo >>= nImageType ) &&
        ( nImageType == nCurrentImageType ) &&
        ( Event.Element >>= xNameAccess ))
    {
        sal_Int16 nImageInfo( 1 );
        Reference< XInterface > xIfacDocImgMgr( m_xDocImageManager, UNO_QUERY );
        if ( xIfacDocImgMgr == Event.Source )
            nImageInfo = 0;

        Sequence< OUString > aSeq = xNameAccess->getElementNames();
        for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
        {
            CommandToInfoMap::iterator pIter = m_aCommandMap.find( aSeq[i] );
            if ( pIter != m_aCommandMap.end() && ( pIter->second.nImageInfo >= nImageInfo ))
            {
                if (isRemove)
                {
                    Image aImage;
                    if (( pIter->second.nImageInfo == 0 ) && ( pIter->second.nImageInfo == nImageInfo ))
                    {
                        // Special case: An image from the document image manager has been removed.
                        // It is possible that we have a image at our module image manager. Before
                        // we can remove our image we have to ask our module image manager.
                        Sequence< OUString > aCmdURLSeq( 1 );
                        Sequence< Reference< XGraphic > > aGraphicSeq;
                        aCmdURLSeq[0] = pIter->first;
                        aGraphicSeq = m_xModuleImageManager->getImages( nImageType, aCmdURLSeq );
                        aImage = Image( aGraphicSeq[0] );
                    }

                    setToolBarImage(aImage,pIter);
                } // if (isRemove)
                else
                {
                    Reference< XGraphic > xGraphic;
                    if ( xNameAccess->getByName( aSeq[i] ) >>= xGraphic )
                    {
                        Image aImage( xGraphic );
                        setToolBarImage(aImage,pIter);
                    }
                    pIter->second.nImageInfo = nImageInfo;
                }
            }
        }
    }
}
void ToolBarManager::setToolBarImage(const Image& rImage,
        const CommandToInfoMap::const_iterator& rIter)
{
    const ::std::vector<sal_uInt16>& rIDs = rIter->second.aIds;
    m_pToolBar->SetItemImage( rIter->second.nId, rImage );
    for (auto const& it : rIDs)
    {
        m_pToolBar->SetItemImage(it, rImage);
    }
}

void SAL_CALL ToolBarManager::elementReplaced( const css::ui::ConfigurationEvent& Event )
{
    impl_elementChanged(false,Event);
}

void ToolBarManager::RemoveControllers()
{
    DBG_TESTSOLARMUTEX();
    assert(!m_bDisposed);

    m_aSubToolBarControllerMap.clear();

    // i90033
    // Remove item window pointers from the toolbar. They were
    // destroyed by the dispose() at the XComponent. This is needed
    // as VCL code later tries to access the item window data in certain
    // dtors where the item window is already invalid!
    for ( sal_uInt16 i = 0; i < m_pToolBar->GetItemCount(); i++ )
    {
        sal_uInt16 nItemId = m_pToolBar->GetItemId( i );
        if ( nItemId > 0 )
        {
            Reference< XComponent > xComponent( m_aControllerMap[ nItemId ], UNO_QUERY );
            if ( xComponent.is() )
            {
                try
                {
                    xComponent->dispose();
                }
                catch (const Exception&)
                {
                }
            }
            m_pToolBar->SetItemWindow(nItemId, nullptr);
        }
    }
    m_aControllerMap.clear();
}

void ToolBarManager::CreateControllers()
{
    Reference< XWindow > xToolbarWindow = VCLUnoHelper::GetInterface( m_pToolBar );

    css::util::URL      aURL;
    bool                bHasDisabledEntries = SvtCommandOptions().HasEntries( SvtCommandOptions::CMDOPTION_DISABLED );
    SvtCommandOptions   aCmdOptions;

    for ( sal_uInt16 i = 0; i < m_pToolBar->GetItemCount(); i++ )
    {
        sal_uInt16 nId = m_pToolBar->GetItemId( i );
        if ( nId == 0 )
            continue;

        OUString                 aLoadURL( ".uno:OpenUrl" );
        bool                     bInit( true );
        bool                     bCreate( true );
        Reference< XStatusListener > xController;

        svt::ToolboxController* pController( nullptr );

        OUString aCommandURL( m_pToolBar->GetItemCommand( nId ) );
        // Command can be just an alias to another command.
        OUString aRealCommandURL( vcl::CommandInfoProvider::Instance().GetRealCommandForCommand( aCommandURL, m_xFrame ) );
        if ( !aRealCommandURL.isEmpty() )
            aCommandURL = aRealCommandURL;

        if ( bHasDisabledEntries )
        {
            aURL.Complete = aCommandURL;
            m_xURLTransformer->parseStrict( aURL );
            if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, aURL.Path ))
            {
                m_aControllerMap[ nId ] = xController;
                m_pToolBar->HideItem( nId );
                continue;
            }
        }

        if ( m_xToolbarControllerFactory.is() &&
             m_xToolbarControllerFactory->hasController( aCommandURL, m_aModuleIdentifier ))
        {
            PropertyValue aPropValue;
            std::vector< Any > aPropertyVector;

            aPropValue.Name     = "ModuleIdentifier";
            aPropValue.Value    <<= m_aModuleIdentifier;
            aPropertyVector.push_back( makeAny( aPropValue ));
            aPropValue.Name     = "Frame";
            aPropValue.Value    <<= m_xFrame;
            aPropertyVector.push_back( makeAny( aPropValue ));
            aPropValue.Name     = "ServiceManager";
            Reference<XMultiServiceFactory> xMSF(m_xContext->getServiceManager(), UNO_QUERY_THROW);
            aPropValue.Value    <<= xMSF;
            aPropertyVector.push_back( makeAny( aPropValue ));
            aPropValue.Name     = "ParentWindow";
            aPropValue.Value    <<= xToolbarWindow;
            aPropertyVector.push_back( makeAny( aPropValue ));
            aPropValue.Name     = "Identifier";
            aPropValue.Value    = uno::makeAny( nId );
            aPropertyVector.push_back( uno::makeAny( aPropValue ) );

            Sequence< Any > aArgs( comphelper::containerToSequence( aPropertyVector ));
            xController.set( m_xToolbarControllerFactory->createInstanceWithArgumentsAndContext( aCommandURL, aArgs, m_xContext ),
                             UNO_QUERY );
            bInit = false; // Initialization is done through the factory service
        }

        if (( aCommandURL == aLoadURL ) && ( !m_pToolBar->IsItemVisible(nId)))
            bCreate = false;

        if ( !xController.is() && bCreate )
        {
            pController = CreateToolBoxController( m_xFrame, m_pToolBar, nId, aCommandURL );
            if ( !pController )
            {
                if ( m_pToolBar->GetItemData( nId ) != nullptr )
                {
                    // retrieve additional parameters
                    OUString aControlType = static_cast< AddonsParams* >( m_pToolBar->GetItemData( nId ))->aControlType;
                    sal_uInt16 nWidth = static_cast< AddonsParams* >( m_pToolBar->GetItemData( nId ))->nWidth;

                    Reference< XStatusListener > xStatusListener(
                        ToolBarMerger::CreateController( m_xContext,
                                                         m_xFrame,
                                                         m_pToolBar,
                                                         aCommandURL,
                                                         nId,
                                                         nWidth,
                                                         aControlType ), UNO_QUERY );

                    xController = xStatusListener;
                }
                else
                {
                    MenuDescriptionMap::iterator it = m_aMenuMap.find( nId );
                    if ( it == m_aMenuMap.end() )
                    {
                        xController.set(
                            new GenericToolbarController( m_xContext, m_xFrame, m_pToolBar, nId, aCommandURL ));

                        // Accessibility support: Set toggle button role for specific commands
                        sal_Int32 nProps = vcl::CommandInfoProvider::Instance().GetPropertiesForCommand(aCommandURL, m_xFrame);
                        if ( nProps & UICOMMANDDESCRIPTION_PROPERTIES_TOGGLEBUTTON )
                            m_pToolBar->SetItemBits( nId, m_pToolBar->GetItemBits( nId ) | ToolBoxItemBits::CHECKABLE );
                    }
                    else
                        xController.set(
                            new MenuToolbarController( m_xContext, m_xFrame, m_pToolBar, nId, aCommandURL, m_aModuleIdentifier, m_aMenuMap[ nId ] ));
                }
            }
            else if ( pController )
            {
                xController.set( static_cast< ::cppu::OWeakObject *>( pController ), UNO_QUERY );
            }
        }

        // Associate ID and controller to be able to retrieve
        // the controller from the ID later.
        m_aControllerMap[ nId ] = xController;

        // Fill sub-toolbars into our hash-map
        Reference< XSubToolbarController > xSubToolBar( xController, UNO_QUERY );
        if ( xSubToolBar.is() && xSubToolBar->opensSubToolbar() )
        {
            OUString aSubToolBarName = xSubToolBar->getSubToolbarName();
            if ( !aSubToolBarName.isEmpty() )
            {
                SubToolBarToSubToolBarControllerMap::iterator pIter =
                    m_aSubToolBarControllerMap.find( aSubToolBarName );
                if ( pIter == m_aSubToolBarControllerMap.end() )
                {
                    SubToolBarControllerVector aSubToolBarVector;
                    aSubToolBarVector.push_back( xSubToolBar );
                    m_aSubToolBarControllerMap.insert(
                        SubToolBarToSubToolBarControllerMap::value_type(
                            aSubToolBarName, aSubToolBarVector ));
                }
                else
                    pIter->second.push_back( xSubToolBar );
            }
        }

        Reference< XInitialization > xInit( xController, UNO_QUERY );
        if ( xInit.is() )
        {
            if ( bInit )
            {
                PropertyValue aPropValue;
                std::vector< Any > aPropertyVector;

                aPropValue.Name = "Frame";
                aPropValue.Value <<= m_xFrame;
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name = "CommandURL";
                aPropValue.Value <<= aCommandURL;
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name = "ServiceManager";
                Reference<XMultiServiceFactory> xMSF(m_xContext->getServiceManager(), UNO_QUERY_THROW);
                aPropValue.Value <<= xMSF;
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name = "ParentWindow";
                aPropValue.Value <<= xToolbarWindow;
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name = "ModuleIdentifier";
                aPropValue.Value <<= m_aModuleIdentifier;
                aPropertyVector.push_back( makeAny( aPropValue ));
                aPropValue.Name     = "Identifier";
                aPropValue.Value    = uno::makeAny( nId );
                aPropertyVector.push_back( uno::makeAny( aPropValue ) );

                Sequence< Any > aArgs( comphelper::containerToSequence( aPropertyVector ));
                xInit->initialize( aArgs );

                if (pController)
                {
                    if(aCommandURL == ".uno:SwitchXFormsDesignMode" ||
                       aCommandURL == ".uno:ViewDataSourceBrowser" ||
                       aCommandURL == ".uno:ParaLeftToRight" ||
                       aCommandURL == ".uno:ParaRightToLeft"
                       )
                        pController->setFastPropertyValue_NoBroadcast(1, makeAny(true));
                }
            }

            // Request a item window from the toolbar controller and set it at the VCL toolbar
            Reference< XToolbarController > xTbxController( xController, UNO_QUERY );
            if ( xTbxController.is() && xToolbarWindow.is() )
            {
                Reference< XWindow > xWindow = xTbxController->createItemWindow( xToolbarWindow );
                if ( xWindow.is() )
                {
                    VclPtr<vcl::Window> pItemWin = VCLUnoHelper::GetWindow( xWindow );
                    if ( pItemWin )
                    {
                        WindowType nType = pItemWin->GetType();
                        if ( nType == WINDOW_LISTBOX || nType == WINDOW_MULTILISTBOX || nType == WINDOW_COMBOBOX )
                            pItemWin->SetAccessibleName( m_pToolBar->GetItemText( nId ) );
                        m_pToolBar->SetItemWindow( nId, pItemWin );
                    }
                }
            }
        }

        //for update Controller via support visiable state
        Reference< XPropertySet > xPropSet( xController, UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                bool bSupportVisible = true;
                Any a( xPropSet->getPropertyValue("SupportsVisible") );
                a >>= bSupportVisible;
                if (bSupportVisible)
                {
                    Reference< XToolbarController > xTbxController( xController, UNO_QUERY );
                    UpdateController(xTbxController);
                }
            }
            catch (const RuntimeException&)
            {
                throw;
            }
            catch (const Exception&)
            {
            }
        }
    }

    AddFrameActionListener();
}

void ToolBarManager::AddFrameActionListener()
{
    if ( !m_bFrameActionRegistered && m_xFrame.is() )
    {
        m_bFrameActionRegistered = true;
        m_xFrame->addFrameActionListener( Reference< XFrameActionListener >(
                                            static_cast< ::cppu::OWeakObject *>( this ), UNO_QUERY ));
    }
}

ToolBoxItemBits ToolBarManager::ConvertStyleToToolboxItemBits( sal_Int32 nStyle )
{
    ToolBoxItemBits nItemBits( ToolBoxItemBits::NONE );
    if ( nStyle & css::ui::ItemStyle::RADIO_CHECK )
        nItemBits |= ToolBoxItemBits::RADIOCHECK;
    if ( nStyle & css::ui::ItemStyle::ALIGN_LEFT )
        nItemBits |= ToolBoxItemBits::LEFT;
    if ( nStyle & css::ui::ItemStyle::AUTO_SIZE )
        nItemBits |= ToolBoxItemBits::AUTOSIZE;
    if ( nStyle & css::ui::ItemStyle::DROP_DOWN )
        nItemBits |= ToolBoxItemBits::DROPDOWN;
    if ( nStyle & css::ui::ItemStyle::REPEAT )
        nItemBits |= ToolBoxItemBits::REPEAT;
    if ( nStyle & css::ui::ItemStyle::DROPDOWN_ONLY )
        nItemBits |= ToolBoxItemBits::DROPDOWNONLY;
    if ( nStyle & css::ui::ItemStyle::TEXT )
        nItemBits |= ToolBoxItemBits::TEXT_ONLY;
    if ( nStyle & css::ui::ItemStyle::ICON )
        nItemBits |= ToolBoxItemBits::ICON_ONLY;

    return nItemBits;
}

void ToolBarManager::FillToolbar( const Reference< XIndexAccess >& rItemContainer )
{
    OString aTbxName = OUStringToOString( m_aResourceName, RTL_TEXTENCODING_ASCII_US );
    SAL_INFO( "fwk.uielement", "framework (cd100003) ::ToolBarManager::FillToolbar " << aTbxName.getStr() );

    SolarMutexGuard g;

    if ( m_bDisposed )
        return;

    sal_uInt16    nId( 1 );

    Reference< XModuleManager2 > xModuleManager = ModuleManager::create( m_xContext );
    if ( !m_xDocImageManager.is() )
    {
        Reference< XModel > xModel( GetModelFromFrame() );
        if ( xModel.is() )
        {
            Reference< XUIConfigurationManagerSupplier > xSupplier( xModel, UNO_QUERY );
            if ( xSupplier.is() )
            {
                m_xDocUICfgMgr.set( xSupplier->getUIConfigurationManager(), UNO_QUERY );
                m_xDocImageManager.set( m_xDocUICfgMgr->getImageManager(), UNO_QUERY );
                m_xDocImageManager->addConfigurationListener(
                                        Reference< XUIConfigurationListener >(
                                            static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
        }
    }

    try
    {
        m_aModuleIdentifier = xModuleManager->identify( Reference< XInterface >( m_xFrame, UNO_QUERY ) );
    }
    catch (const Exception&)
    {
    }

    if ( !m_xModuleImageManager.is() )
    {
        Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier =
            theModuleUIConfigurationManagerSupplier::get( m_xContext );
        m_xUICfgMgr = xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleIdentifier );
        m_xModuleImageManager.set( m_xUICfgMgr->getImageManager(), UNO_QUERY );
        m_xModuleImageManager->addConfigurationListener( Reference< XUIConfigurationListener >(
                                                            static_cast< OWeakObject* >( this ), UNO_QUERY ));
    }

    RemoveControllers();

    // reset and fill command map
    m_pToolBar->Clear();
    m_aControllerMap.clear();
    m_aCommandMap.clear();

    m_aMenuMap.clear();

    CommandInfo aCmdInfo;
    for ( sal_Int32 n = 0; n < rItemContainer->getCount(); n++ )
    {
        Sequence< PropertyValue >   aProp;
        OUString                    aCommandURL;
        OUString                    aLabel;
        sal_uInt16                  nType( css::ui::ItemType::DEFAULT );
        sal_uInt32                  nStyle( 0 );

        Reference< XIndexAccess >   aMenuDesc;
        try
        {
            if ( rItemContainer->getByIndex( n ) >>= aProp )
            {
                bool bIsVisible( true );
                for ( int i = 0; i < aProp.getLength(); i++ )
                {
                    if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                    {
                        aProp[i].Value >>= aCommandURL;
                        if ( aCommandURL.startsWith("private:resource/menubar/") )
                        {
                            try
                            {
                                Reference< XIndexAccess > xMenuContainer;
                                if ( m_xDocUICfgMgr.is() &&
                                     m_xDocUICfgMgr->hasSettings( aCommandURL ) )
                                    xMenuContainer  = m_xDocUICfgMgr->getSettings( aCommandURL, false );
                                if ( !xMenuContainer.is() &&
                                     m_xUICfgMgr.is() &&
                                     m_xUICfgMgr->hasSettings( aCommandURL ) )
                                    xMenuContainer = m_xUICfgMgr->getSettings( aCommandURL, false );
                                if ( xMenuContainer.is() && xMenuContainer->getCount() )
                                {
                                    Sequence< PropertyValue > aProps;
                                    // drop down menu info is currently
                                    // the first ( and only ) menu
                                    // in the menusettings container
                                    xMenuContainer->getByIndex(0) >>= aProps;
                                    for ( sal_Int32 index=0; index<aProps.getLength(); ++index )
                                    {
                                        if ( aProps[ index ].Name == "ItemDescriptorContainer" )

                                        {
                                            aProps[ index ].Value >>= aMenuDesc;
                                            break;
                                        }
                                    }
                                }
                            }
                            catch (const Exception&)
                            {
                            }
                        }
                    }
                    else if ( aProp[i].Name == "Label" )
                        aProp[i].Value >>= aLabel;
                    else if ( aProp[i].Name == "Type" )
                        aProp[i].Value >>= nType;
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_VISIBLE )
                        aProp[i].Value >>= bIsVisible;
                    else if ( aProp[i].Name == "Style" )
                        aProp[i].Value >>= nStyle;
                }

                if (vcl::CommandInfoProvider::Instance().IsExperimental(aCommandURL, m_aModuleIdentifier) &&
                    !SvtMiscOptions().IsExperimentalMode())
                {
                    continue;
                }

                if (( nType == css::ui::ItemType::DEFAULT ) && !aCommandURL.isEmpty() )
                {
                    OUString aString(vcl::CommandInfoProvider::Instance().GetLabelForCommand(aCommandURL, m_xFrame));

                    ToolBoxItemBits nItemBits = ConvertStyleToToolboxItemBits( nStyle );
                    if ( aMenuDesc.is() )
                    {
                        m_aMenuMap[ nId ] = aMenuDesc;
                        nItemBits |= ToolBoxItemBits::DROPDOWNONLY;
                    }
                    m_pToolBar->InsertItem( nId, aString, nItemBits );
                    m_pToolBar->SetItemCommand( nId, aCommandURL );
                    OUString sTooltip = vcl::CommandInfoProvider::Instance().GetTooltipForCommand(aCommandURL, m_xFrame);
                    if (!sTooltip.isEmpty())
                        m_pToolBar->SetQuickHelpText( nId, sTooltip );

                    if ( !aLabel.isEmpty() )
                    {
                        m_pToolBar->SetItemText( nId, aLabel );
                    }
                    else
                    {
                        m_pToolBar->SetItemText( nId, aString );
                    }
                    m_pToolBar->EnableItem( nId );
                    m_pToolBar->SetItemState( nId, TRISTATE_FALSE );

                    // Fill command map. It stores all our commands and from what
                    // image manager we got our image. So we can decide if we have to use an
                    // image from a notification message.
                    CommandToInfoMap::iterator pIter = m_aCommandMap.find( aCommandURL );
                    if ( pIter == m_aCommandMap.end())
                    {
                        aCmdInfo.nId = nId;
                        const CommandToInfoMap::value_type aValue( aCommandURL, aCmdInfo );
                        m_aCommandMap.insert( aValue );
                    }
                    else
                    {
                        pIter->second.aIds.push_back( nId );
                    }

                    if ( !bIsVisible )
                        m_pToolBar->HideItem( nId );

                    ++nId;
                }
                else if ( nType == css::ui::ItemType::SEPARATOR_LINE )
                {
                    m_pToolBar->InsertSeparator();
                }
                else if ( nType == css::ui::ItemType::SEPARATOR_SPACE )
                {
                    m_pToolBar->InsertSpace();
                }
                else if ( nType == css::ui::ItemType::SEPARATOR_LINEBREAK )
                {
                    m_pToolBar->InsertBreak();
                }
            }
        }
        catch (const css::lang::IndexOutOfBoundsException&)
        {
            break;
        }
    }

    // Support add-on toolbar merging here. Working directly on the toolbar object is much
    // simpler and faster.
    const sal_uInt16 TOOLBAR_ITEM_STARTID = 1000;

    MergeToolbarInstructionContainer aMergeInstructionContainer;

    // Retrieve the toolbar name from the resource name
    OUString aToolbarName( m_aResourceName );
    sal_Int32 nIndex = aToolbarName.lastIndexOf( '/' );
    if (( nIndex > 0 ) && ( nIndex < aToolbarName.getLength() ))
        aToolbarName = aToolbarName.copy( nIndex+1 );

    AddonsOptions().GetMergeToolbarInstructions( aToolbarName, aMergeInstructionContainer );

    if ( !aMergeInstructionContainer.empty() )
    {
        sal_uInt16 nItemId( TOOLBAR_ITEM_STARTID );
        const sal_uInt32 nCount = aMergeInstructionContainer.size();
        for ( sal_uInt32 i=0; i < nCount; i++ )
        {
            MergeToolbarInstruction& rInstruction = aMergeInstructionContainer[i];
            if ( ToolBarMerger::IsCorrectContext( rInstruction.aMergeContext, m_aModuleIdentifier ))
            {
                ReferenceToolbarPathInfo aRefPoint = ToolBarMerger::FindReferencePoint( m_pToolBar, rInstruction.aMergePoint );

                // convert the sequence< sequence< propertyvalue > > structure to
                // something we can better handle. A vector with item data
                AddonToolbarItemContainer aItems;
                ToolBarMerger::ConvertSeqSeqToVector( rInstruction.aMergeToolbarItems, aItems );

                if ( aRefPoint.bResult )
                {
                    ToolBarMerger::ProcessMergeOperation( m_pToolBar,
                                                          aRefPoint.nPos,
                                                          nItemId,
                                                          m_aCommandMap,
                                                          m_aModuleIdentifier,
                                                          rInstruction.aMergeCommand,
                                                          rInstruction.aMergeCommandParameter,
                                                          aItems );
                }
                else
                {
                    ToolBarMerger::ProcessMergeFallback( m_pToolBar,
                                                         aRefPoint.nPos,
                                                         nItemId,
                                                         m_aCommandMap,
                                                         m_aModuleIdentifier,
                                                         rInstruction.aMergeCommand,
                                                         rInstruction.aMergeFallback,
                                                         aItems );
                }
            }
        }
    }

    // Request images for all toolbar items. Must be done before CreateControllers as
    // some controllers need access to the image.
    RequestImages();

    // Create controllers after we set the images. There are controllers which needs
    // an image at the toolbar at creation time!
    CreateControllers();

    // Notify controllers that they are now correctly initialized and can start listening
    // toolbars that will open in popup mode will be updated immediately to avoid flickering
    if( m_pToolBar->WillUsePopupMode() )
        UpdateControllers();
    else if ( m_pToolBar->IsReallyVisible() )
    {
        m_aAsyncUpdateControllersTimer.Start();
    }

    // Try to retrieve UIName from the container property set and set it as the title
    // if it is not empty.
    Reference< XPropertySet > xPropSet( rItemContainer, UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            OUString aUIName;
            xPropSet->getPropertyValue("UIName") >>= aUIName;
            if ( !aUIName.isEmpty() )
                m_pToolBar->SetText( aUIName );
        }
        catch (const Exception&)
        {
        }
    }
}

void ToolBarManager::RequestImages()
{

    // Request images from image manager
    Sequence< OUString > aCmdURLSeq( comphelper::mapKeysToSequence(m_aCommandMap) );
    Sequence< Reference< XGraphic > > aDocGraphicSeq;
    Sequence< Reference< XGraphic > > aModGraphicSeq;

    SvtMiscOptions aMiscOptions;

    sal_Int16 nImageType = getCurrentImageType();

    if ( m_xDocImageManager.is() )
        aDocGraphicSeq = m_xDocImageManager->getImages(nImageType, aCmdURLSeq);
    aModGraphicSeq = m_xModuleImageManager->getImages(nImageType, aCmdURLSeq);

    sal_uInt32 i = 0;
    CommandToInfoMap::iterator pIter = m_aCommandMap.begin();
    CommandToInfoMap::iterator pEnd = m_aCommandMap.end();
    while ( pIter != pEnd )
    {
        Image aImage;
        if ( aDocGraphicSeq.getLength() > 0 )
            aImage = Image( aDocGraphicSeq[i] );
        if ( !aImage )
        {
            aImage = Image( aModGraphicSeq[i] );
            // Try also to query for add-on images before giving up and use an
            // empty image.
            if ( !aImage )
                aImage = framework::AddonsOptions().GetImageFromURL( aCmdURLSeq[i], aMiscOptions.AreCurrentSymbolsLarge());

            pIter->second.nImageInfo = 1; // mark image as module based
        }
        else
        {
            pIter->second.nImageInfo = 0; // mark image as document based
        }
        setToolBarImage(aImage,pIter);
        ++pIter;
        ++i;
    }
}

void ToolBarManager::notifyRegisteredControllers( const OUString& aUIElementName, const OUString& aCommand )
{
    SolarMutexClearableGuard aGuard;
    if ( !m_aSubToolBarControllerMap.empty() )
    {
        SubToolBarToSubToolBarControllerMap::const_iterator pIter =
            m_aSubToolBarControllerMap.find( aUIElementName );

        if ( pIter != m_aSubToolBarControllerMap.end() )
        {
            const SubToolBarControllerVector& rSubToolBarVector = pIter->second;
            if ( !rSubToolBarVector.empty() )
            {
                SubToolBarControllerVector aNotifyVector = rSubToolBarVector;
                aGuard.clear();

                const sal_uInt32 nCount = aNotifyVector.size();
                for ( sal_uInt32 i=0; i < nCount; i++ )
                {
                    try
                    {
                        Reference< XSubToolbarController > xController = aNotifyVector[i];
                        if ( xController.is() )
                            xController->functionSelected( aCommand );
                    }
                    catch (const RuntimeException&)
                    {
                        throw;
                    }
                    catch (const Exception&)
                    {
                    }
                }
            }
        }
    }
}

void ToolBarManager::HandleClick(void ( SAL_CALL XToolbarController::*_pClick )())
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return;

    sal_uInt16 nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            (xController.get()->*_pClick)( );
    }
}

IMPL_LINK_NOARG(ToolBarManager, Click, ToolBox *, void)
{
    HandleClick(&XToolbarController::click);
}

IMPL_LINK_NOARG(ToolBarManager, DropdownClick, ToolBox *, void)
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return;

    sal_uInt16 nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
        {
            Reference< XWindow > xWin = xController->createPopupWindow();
            if ( xWin.is() )
                xWin->setFocus();
        }
    }
}

IMPL_LINK_NOARG(ToolBarManager, DoubleClick, ToolBox *, void)
{
    HandleClick(&XToolbarController::doubleClick);
}

void ToolBarManager::ImplClearPopupMenu( ToolBox *pToolBar )
{
    if ( m_bDisposed )
        return;

    ::PopupMenu *pMenu = pToolBar->GetMenu();
    if (pMenu == nullptr) {
        return;
    }

    // remove config entries from menu, so we have a clean menu to start with
    // remove submenu first
    pMenu->SetPopupMenu( 1, nullptr );

    // remove all items that were not added by the toolbar itself
    sal_uInt16 i;
    for( i=0; i<pMenu->GetItemCount(); )
    {
        if( pMenu->GetItemId( i ) < TOOLBOX_MENUITEM_START
            && pMenu->GetItemId( i ) != 0 ) // Don't remove separators (Id == 0)
            pMenu->RemoveItem( i );
        else
            i++;
    }
}

void ToolBarManager::MenuDeactivated()
{
    if (m_bDisposed)
        return;
    ImplClearPopupMenu(m_pToolBar);
}

Reference< XModel > ToolBarManager::GetModelFromFrame() const
{
    Reference< XController > xController = m_xFrame->getController();
    Reference< XModel > xModel;
    if ( xController.is() )
        xModel = xController->getModel();

    return xModel;
}

bool ToolBarManager::IsPluginMode() const
{
    bool bPluginMode( false );

    if ( m_xFrame.is() )
    {
        Reference< XModel > xModel = GetModelFromFrame();
        if ( xModel.is() )
        {
            Sequence< PropertyValue > aSeq = xModel->getArgs();
            utl::MediaDescriptor aMediaDescriptor( aSeq );
            bPluginMode = aMediaDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_VIEWONLY(), false );
        }
    }

    return bPluginMode;
}

bool ToolBarManager::MenuItemAllowed( sal_uInt16 ) const
{
    return true;
}

::PopupMenu * ToolBarManager::GetToolBarCustomMenu(ToolBox* pToolBar)
{
    // update the list of hidden tool items first
    pToolBar->UpdateCustomMenu();

    ::PopupMenu *pMenu = pToolBar->GetMenu();
    // remove all entries before inserting new ones
    ImplClearPopupMenu( pToolBar );
    // No config menu entries if command ".uno:ConfigureDialog" is not enabled
    Reference< XDispatch > xDisp;
    css::util::URL aURL;
    if ( m_xFrame.is() )
    {
        Reference< XDispatchProvider > xProv( m_xFrame, UNO_QUERY );
        aURL.Complete = ".uno:ConfigureDialog";
        m_xURLTransformer->parseStrict( aURL );
        if ( xProv.is() )
            xDisp = xProv->queryDispatch( aURL, OUString(), 0 );

        if ( !xDisp.is() || IsPluginMode() )
            return nullptr;
    }

    // popup menu for quick customization
    bool bHideDisabledEntries = !SvtMenuOptions().IsEntryHidingEnabled();
    ScopedVclPtrInstance<::PopupMenu> aQuickCustomizationMenu( FwkResId( POPUPMENU_TOOLBAR_QUICKCUSTOMIZATION ));

    if ( m_pToolBar->IsCustomize() )
    {
        sal_uInt16    nPos( 0 );
        ::PopupMenu*  pVisibleItemsPopupMenu( aQuickCustomizationMenu->GetPopupMenu( 1 ));

        bool    bIsFloating( false );

        DockingManager* pDockMgr = vcl::Window::GetDockingManager();
        if ( pDockMgr )
            bIsFloating = pDockMgr->IsFloating( m_pToolBar );

        if ( !bIsFloating )
        {
            aQuickCustomizationMenu->EnableItem( MENUITEM_TOOLBAR_DOCKTOOLBAR, false );
            aQuickCustomizationMenu->EnableItem( MENUITEM_TOOLBAR_DOCKALLTOOLBAR, false );
            Reference< XDockableWindow > xDockable( VCLUnoHelper::GetInterface( m_pToolBar ), UNO_QUERY );
            if( xDockable.is() )
                aQuickCustomizationMenu->CheckItem( MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, xDockable->isLocked() );
        }
        else
            aQuickCustomizationMenu->EnableItem( MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, false );

        if ( SvtMiscOptions().DisableUICustomization() )
        {
            aQuickCustomizationMenu->EnableItem( MENUITEM_TOOLBAR_VISIBLEBUTTON, false );
            aQuickCustomizationMenu->EnableItem( MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR, false );
            aQuickCustomizationMenu->EnableItem( MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION, false );
        }

        // Disable menu item CLOSE if the toolbar has no closer
        if( !(pToolBar->GetFloatStyle() & WB_CLOSEABLE) )
            aQuickCustomizationMenu->EnableItem(MENUITEM_TOOLBAR_CLOSE, false);

        // Temporary stores a Command --> Url map to update contextual menu with the
        // correct icons. The popup icons are by default the same as those in the
        // toolbar. They are not correct for contextual popup menu.
        std::map< OUString, Image > commandToImage;

        // Go through all toolbar items and add them to the context menu
        for ( nPos = 0; nPos < m_pToolBar->GetItemCount(); ++nPos )
        {
            if ( m_pToolBar->GetItemType(nPos) == ToolBoxItemType::BUTTON )
            {
                sal_uInt16 nId = m_pToolBar->GetItemId(nPos);
                OUString aCommandURL = m_pToolBar->GetItemCommand( nId );
                pVisibleItemsPopupMenu->InsertItem( STARTID_CUSTOMIZE_POPUPMENU+nPos, m_pToolBar->GetItemText( nId ), MenuItemBits::CHECKABLE );
                pVisibleItemsPopupMenu->CheckItem( STARTID_CUSTOMIZE_POPUPMENU+nPos, m_pToolBar->IsItemVisible( nId ) );
                pVisibleItemsPopupMenu->SetItemCommand( STARTID_CUSTOMIZE_POPUPMENU+nPos, aCommandURL );
                Image aImage(vcl::CommandInfoProvider::Instance().GetImageForCommand(aCommandURL, m_xFrame));
                commandToImage[aCommandURL] = aImage;
                pVisibleItemsPopupMenu->SetItemImage( STARTID_CUSTOMIZE_POPUPMENU+nPos, aImage );
            }
            else
            {
                pVisibleItemsPopupMenu->InsertSeparator();
            }
        }

        // Now we go through all the contextual menu to update the icons
        std::map< OUString, Image >::iterator it;
        for ( nPos = 0; nPos < pMenu->GetItemCount(); ++nPos )
        {
            sal_uInt16 nId = pMenu->GetItemId( nPos );
            OUString cmdUrl = pMenu->GetItemCommand( nId );
            it = commandToImage.find( cmdUrl );
            if (it != commandToImage.end()) {
                pMenu->SetItemImage( nId, it->second );
            }
        }
    }
    else
    {
        sal_uInt16 nPos = aQuickCustomizationMenu->GetItemPos( MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR );
        if ( nPos != MENU_ITEM_NOTFOUND )
            aQuickCustomizationMenu->RemoveItem( nPos );
    }

    // copy all menu items 'Visible buttons, Customize toolbar, Dock toolbar,
    // Dock all Toolbars) from the loaded resource into the toolbar menu
    if( pMenu->GetItemCount() )
        pMenu->InsertSeparator();

    sal_uInt16 i;
    for( i=0; i< aQuickCustomizationMenu->GetItemCount(); i++)
    {
        sal_uInt16 nId = aQuickCustomizationMenu->GetItemId( i );
        if ( MenuItemAllowed( nId ))
            pMenu->CopyItem( *aQuickCustomizationMenu.get(), i );
    }

    // Set the title of the menu
    pMenu->SetText( pToolBar->GetText() );

    if ( bHideDisabledEntries )
        pMenu->RemoveDisabledEntries();

    return pMenu;
}

IMPL_LINK( ToolBarManager, Command, CommandEvent const *, pCmdEvt, void )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return;
    if ( pCmdEvt->GetCommand() != CommandEventId::ContextMenu )
        return;

    ::PopupMenu * pMenu = GetToolBarCustomMenu(m_pToolBar);
    if (pMenu)
    {
        // We only want to handle events for the context menu, but not events
        // on the toolbars overflow menu, hence we should only receive events
        // from the toolbox menu when we are actually showing it as our context
        // menu (the same menu retrieved with  GetMenu() is reused for both the
        // overflow and context menus). If we set these Hdls permanently rather
        // than just when the context menu is showing, then events are duplicated
        // when the menu is being used as an overflow menu.
        Menu *pManagerMenu = m_pToolBar->GetMenu();
        pManagerMenu->SetSelectHdl( LINK( this, ToolBarManager, MenuSelect ) );

        // make sure all disabled entries will be shown
        pMenu->SetMenuFlags( pMenu->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );
        ::Point aPoint( pCmdEvt->GetMousePosPixel() );
        pMenu->Execute( m_pToolBar, aPoint );

        //fdo#86820 We may have been disposed and so have a NULL m_pToolBar by
        //executing a menu entry, e.g. inserting a chart replaces the toolbars
        pManagerMenu = m_bDisposed ? nullptr : m_pToolBar->GetMenu();
        if (pManagerMenu)
        {
            // Unlink our listeners again -- see above for why.
            pManagerMenu->SetSelectHdl( Link<Menu*, bool>() );
            MenuDeactivated();
        }
    }
}

IMPL_LINK( ToolBarManager, MenuButton, ToolBox*, pToolBar, void )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return;

    pToolBar->UpdateCustomMenu();
    // remove all entries that do not come from the toolbar itself (fdo#38276)
    ImplClearPopupMenu( pToolBar );
 }

IMPL_LINK( ToolBarManager, MenuSelect, Menu*, pMenu, bool )
{
    // We have to hold a reference to ourself as it is possible that we will be disposed and
    // our refcount could be zero (destruction) otherwise.
    Reference< XInterface > xInterface( static_cast< OWeakObject* >( this ), UNO_QUERY );

    {
        // The guard must be in its own context as the we can get destroyed when our
        // own xInterface reference get destroyed!
        SolarMutexGuard g;

        if ( m_bDisposed )
            return true;

        switch ( pMenu->GetCurItemId() )
        {
            case MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR:
            {
                Reference< XDispatch > xDisp;
                css::util::URL aURL;
                if ( m_xFrame.is() )
                {
                    Reference< XDispatchProvider > xProv( m_xFrame, UNO_QUERY );
                    aURL.Complete = ".uno:ConfigureDialog";
                    m_xURLTransformer->parseStrict( aURL );
                    if ( xProv.is() )
                        xDisp = xProv->queryDispatch( aURL, OUString(), 0 );
                }

                if ( xDisp.is() )
                {
                    Sequence< PropertyValue > aPropSeq( 1 );

                    aPropSeq[ 0 ].Name = "ResourceURL";
                    aPropSeq[ 0 ].Value <<= m_aResourceName;

                    xDisp->dispatch( aURL, aPropSeq );
                }
                break;
            }

            case MENUITEM_TOOLBAR_DOCKTOOLBAR:
            {
                ExecuteInfo* pExecuteInfo = new ExecuteInfo;

                pExecuteInfo->aToolbarResName = m_aResourceName;
                pExecuteInfo->nCmd            = EXEC_CMD_DOCKTOOLBAR;
                pExecuteInfo->xLayoutManager  = getLayoutManagerFromFrame( m_xFrame );

                Application::PostUserEvent( LINK(nullptr, ToolBarManager, ExecuteHdl_Impl), pExecuteInfo );
                break;
            }

            case MENUITEM_TOOLBAR_DOCKALLTOOLBAR:
            {
                ExecuteInfo* pExecuteInfo = new ExecuteInfo;

                pExecuteInfo->aToolbarResName = m_aResourceName;
                pExecuteInfo->nCmd            = EXEC_CMD_DOCKALLTOOLBARS;
                pExecuteInfo->xLayoutManager  = getLayoutManagerFromFrame( m_xFrame );

                Application::PostUserEvent( LINK(nullptr, ToolBarManager, ExecuteHdl_Impl), pExecuteInfo );
                break;
            }

            case MENUITEM_TOOLBAR_LOCKTOOLBARPOSITION:
            {
                Reference< XLayoutManager > xLayoutManager = getLayoutManagerFromFrame( m_xFrame );
                if ( xLayoutManager.is() )
                {
                    Reference< XDockableWindow > xDockable( VCLUnoHelper::GetInterface( m_pToolBar ), UNO_QUERY );

                    if( xDockable->isLocked() )
                        xLayoutManager->unlockWindow( m_aResourceName );
                    else
                        xLayoutManager->lockWindow( m_aResourceName );
                }
                break;
            }

            case MENUITEM_TOOLBAR_CLOSE:
            {
                ExecuteInfo* pExecuteInfo = new ExecuteInfo;

                pExecuteInfo->aToolbarResName = m_aResourceName;
                pExecuteInfo->nCmd            = EXEC_CMD_CLOSETOOLBAR;
                pExecuteInfo->xLayoutManager  = getLayoutManagerFromFrame( m_xFrame );
                pExecuteInfo->xWindow         = VCLUnoHelper::GetInterface( m_pToolBar );

                Application::PostUserEvent( LINK(nullptr, ToolBarManager, ExecuteHdl_Impl), pExecuteInfo );
                break;
            }

            default:
            {
                sal_uInt16 nId = pMenu->GetCurItemId();
                if(( nId > 0 ) && ( nId < TOOLBOX_MENUITEM_START ))
                // Items in the "enable/disable" sub-menu
                {
                    // toggle toolbar button visibility
                    OUString aCommand = pMenu->GetItemCommand( nId );

                    Reference< XLayoutManager > xLayoutManager = getLayoutManagerFromFrame( m_xFrame );
                    if ( xLayoutManager.is() )
                    {
                        Reference< XUIElementSettings > xUIElementSettings( xLayoutManager->getElement( m_aResourceName ), UNO_QUERY );
                        if ( xUIElementSettings.is() )
                        {
                            Reference< XIndexContainer > xItemContainer( xUIElementSettings->getSettings( true ), UNO_QUERY );
                            sal_Int32 nCount = xItemContainer->getCount();
                            for ( sal_Int32 i = 0; i < nCount; i++ )
                            {
                                Sequence< PropertyValue > aProp;
                                sal_Int32                 nVisibleIndex( -1 );
                                OUString             aCommandURL;
                                bool                  bVisible( false );

                                if ( xItemContainer->getByIndex( i ) >>= aProp )
                                {
                                    for ( sal_Int32 j = 0; j < aProp.getLength(); j++ )
                                    {
                                        if ( aProp[j].Name == ITEM_DESCRIPTOR_COMMANDURL )
                                        {
                                            aProp[j].Value >>= aCommandURL;
                                        }
                                        else if ( aProp[j].Name == ITEM_DESCRIPTOR_VISIBLE )
                                        {
                                            aProp[j].Value >>= bVisible;
                                            nVisibleIndex = j;
                                        }
                                    }

                                    if (( aCommandURL == aCommand ) && ( nVisibleIndex >= 0 ))
                                    {
                                        // We have found the requested item, toggle the visible flag
                                        // and write back the configuration settings to the toolbar
                                        aProp[nVisibleIndex].Value = makeAny( !bVisible );
                                        try
                                        {
                                            xItemContainer->replaceByIndex( i, makeAny( aProp ));
                                            xUIElementSettings->setSettings( xItemContainer );
                                            Reference< XPropertySet > xPropSet( xUIElementSettings, UNO_QUERY );
                                            if ( xPropSet.is() )
                                            {
                                                Reference< XUIConfigurationPersistence > xUICfgMgr;
                                                if (( xPropSet->getPropertyValue("ConfigurationSource") >>= xUICfgMgr ) && ( xUICfgMgr.is() ))
                                                    xUICfgMgr->store();
                                            }
                                        }
                                        catch (const Exception&)
                                        {
                                        }

                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                // The list of "hidden items", i.e. items which are disabled on
                // the toolbar hence shown in the context menu for easier access,
                // which are managed by the owning toolbar.
                {
                    m_pToolBar->TriggerItem( pMenu->GetCurItemId()
                                             - TOOLBOX_MENUITEM_START );
                }
                break;
            }
        }

        // remove all entries - deactivate is not reliable
        // The method checks if we are already disposed and in that case does nothing!
        ImplClearPopupMenu( m_pToolBar );
    }

    return true;
}

IMPL_LINK_NOARG(ToolBarManager, Select, ToolBox *, void)
{
    if ( m_bDisposed )
        return;

    sal_Int16   nKeyModifier( (sal_Int16)m_pToolBar->GetModifier() );
    sal_uInt16      nId( m_pToolBar->GetCurItemId() );

    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->execute( nKeyModifier );
    }
}

IMPL_LINK( ToolBarManager, StateChanged, StateChangedType const *, pStateChangedType, void )
{
    if ( m_bDisposed )
        return;

    if ( *pStateChangedType == StateChangedType::ControlBackground )
    {
        CheckAndUpdateImages();
    }
    else if ( *pStateChangedType == StateChangedType::Visible )
    {
        if ( m_pToolBar->IsReallyVisible() )
        {
            m_aAsyncUpdateControllersTimer.Start();
        }
    }
    else if ( *pStateChangedType == StateChangedType::InitShow )
    {
        m_aAsyncUpdateControllersTimer.Start();
    }
}

IMPL_LINK( ToolBarManager, DataChanged, DataChangedEvent const *, pDataChangedEvent, void )
{
    if ((( pDataChangedEvent->GetType() == DataChangedEventType::SETTINGS )   ||
        (  pDataChangedEvent->GetType() == DataChangedEventType::DISPLAY  ))  &&
        ( pDataChangedEvent->GetFlags() & AllSettingsFlags::STYLE        ))
    {
        CheckAndUpdateImages();
    }

    for ( sal_uInt16 nPos = 0; nPos < m_pToolBar->GetItemCount(); ++nPos )
    {
        const sal_uInt16 nId = m_pToolBar->GetItemId(nPos);
        vcl::Window* pWindow = m_pToolBar->GetItemWindow( nId );
        if ( pWindow )
        {
            const DataChangedEvent& rDCEvt( *pDataChangedEvent );
            pWindow->DataChanged( rDCEvt );
        }
    }

    if ( !m_pToolBar->IsFloatingMode() &&
         m_pToolBar->IsVisible() )
    {
        // Resize toolbar, layout manager is resize listener and will calc
        // the layout automatically.
        ::Size aSize( m_pToolBar->CalcWindowSizePixel() );
        m_pToolBar->SetOutputSizePixel( aSize );
    }
}

IMPL_LINK_NOARG(ToolBarManager, MiscOptionsChanged, LinkParamNone*, void)
{
    CheckAndUpdateImages();
}

IMPL_LINK_NOARG(ToolBarManager, AsyncUpdateControllersHdl, Timer *, void)
{
    // The guard must be in its own context as the we can get destroyed when our
    // own xInterface reference get destroyed!
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    SolarMutexGuard g;

    if ( m_bDisposed )
        return;

    // Request to update our controllers
    m_aAsyncUpdateControllersTimer.Stop();
    UpdateControllers();
}

IMPL_STATIC_LINK( ToolBarManager, ExecuteHdl_Impl, void*, p, void )
{
    ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        if (( pExecuteInfo->nCmd == EXEC_CMD_CLOSETOOLBAR ) &&
            ( pExecuteInfo->xLayoutManager.is() ) &&
            ( pExecuteInfo->xWindow.is() ))
        {
            // Use docking window close to close the toolbar. The toolbar layout manager is
            // listener and will react correctly according to the context sensitive
            // flag of our toolbar.
            VclPtr<vcl::Window> pWin = VCLUnoHelper::GetWindow( pExecuteInfo->xWindow );
            DockingWindow* pDockWin = dynamic_cast< DockingWindow* >( pWin.get() );
            if ( pDockWin )
                pDockWin->Close();
        }
        else if (( pExecuteInfo->nCmd == EXEC_CMD_DOCKTOOLBAR ) &&
                 ( pExecuteInfo->xLayoutManager.is() ))
        {
            css::awt::Point aPoint;
            aPoint.X = aPoint.Y = SAL_MAX_INT32;
            pExecuteInfo->xLayoutManager->dockWindow( pExecuteInfo->aToolbarResName,
                                                      DockingArea_DOCKINGAREA_DEFAULT,
                                                      aPoint );
        }
        else if (( pExecuteInfo->nCmd == EXEC_CMD_DOCKALLTOOLBARS ) &&
                 ( pExecuteInfo->xLayoutManager.is() ))
        {
            pExecuteInfo->xLayoutManager->dockAllWindows( UIElementType::TOOLBAR );
        }
    }
    catch (const Exception&)
    {
    }

    delete pExecuteInfo;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
