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

#include <uielement/addonstoolbarmanager.hxx>
#include <uielement/toolbarmerger.hxx>

#include <uielement/generictoolbarcontroller.hxx>
#include "services.h"
#include <framework/sfxhelperfunctions.hxx>
#include <classes/fwkresid.hxx>
#include <classes/resource.hrc>
#include <framework/addonsoptions.hxx>
#include <uielement/comboboxtoolbarcontroller.hxx>
#include <uielement/imagebuttontoolbarcontroller.hxx>
#include <uielement/togglebuttontoolbarcontroller.hxx>
#include <uielement/buttontoolbarcontroller.hxx>
#include <uielement/spinfieldtoolbarcontroller.hxx>
#include <uielement/edittoolbarcontroller.hxx>
#include <uielement/dropdownboxtoolbarcontroller.hxx>

#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <svtools/imgdef.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>

//  namespaces

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;

namespace framework
{

AddonsToolBarManager::AddonsToolBarManager( const Reference< XComponentContext >& rxContext,
                                const Reference< XFrame >& rFrame,
                                const OUString& rResourceName,
                                ToolBox* pToolBar ) :
    ToolBarManager( rxContext, rFrame, rResourceName, pToolBar )
{
    m_pToolBar->SetMenuType( ToolBoxMenuType::ClippedItems );
    m_pToolBar->SetSelectHdl( LINK( this, AddonsToolBarManager, Select) );
    m_pToolBar->SetClickHdl( LINK( this, AddonsToolBarManager, Click ) );
    m_pToolBar->SetDoubleClickHdl( LINK( this, AddonsToolBarManager, DoubleClick ) );
    m_pToolBar->SetCommandHdl( LINK( this, AddonsToolBarManager, Command ) );
    m_pToolBar->SetStateChangedHdl( LINK( this, AddonsToolBarManager, StateChanged ) );
    m_pToolBar->SetDataChangedHdl( LINK( this, AddonsToolBarManager, DataChanged ) );
}

AddonsToolBarManager::~AddonsToolBarManager()
{
}

static bool IsCorrectContext( const OUString& rModuleIdentifier, const OUString& aContextList )
{
    if ( aContextList.isEmpty() )
        return true;

    if ( !rModuleIdentifier.isEmpty() )
    {
        sal_Int32 nIndex = aContextList.indexOf( rModuleIdentifier );
        return ( nIndex >= 0 );
    }

    return false;
}

static Image RetrieveImage( Reference< css::frame::XFrame >& rFrame,
                            const OUString& aImageId,
                            const OUString& aURL,
                            bool bBigImage
)
{
    vcl::ImageType eImageType = vcl::ImageType::Size16;
    if (bBigImage)
        eImageType = vcl::ImageType::Size26;

    Image aImage;

    if ( !aImageId.isEmpty() )
    {
        aImage = framework::AddonsOptions().GetImageFromURL( aImageId, bBigImage );
        if ( !!aImage )
            return aImage;
        else
            aImage = vcl::CommandInfoProvider::GetImageForCommand(aImageId, rFrame, eImageType);
        if ( !!aImage )
            return aImage;
    }

    aImage = framework::AddonsOptions().GetImageFromURL( aURL, bBigImage );
    if ( !aImage )
        aImage = vcl::CommandInfoProvider::GetImageForCommand(aImageId, rFrame, eImageType);

    return aImage;
}

// XComponent
void SAL_CALL AddonsToolBarManager::dispose()
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        // Remove addon specific data from toolbar items.
        SolarMutexGuard g;
        for ( sal_uInt16 n = 0; n < m_pToolBar->GetItemCount(); n++ )
        {
            sal_uInt16 nId( m_pToolBar->GetItemId( n ) );

            if ( nId > 0 )
            {
                AddonsParams* pRuntimeItemData = static_cast<AddonsParams*>(m_pToolBar->GetItemData( nId ));
                delete pRuntimeItemData;
                m_pToolBar->SetItemData( nId, nullptr );
            }
        }
    }

    // Base class will destroy our m_pToolBar member
    ToolBarManager::dispose();
}

bool AddonsToolBarManager::MenuItemAllowed( sal_uInt16 nId ) const
{
    if (( nId == MENUITEM_TOOLBAR_VISIBLEBUTTON ) ||
        ( nId == MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR ))
        return false;
    else
        return true;
}

void AddonsToolBarManager::RefreshImages()
{
    bool  bBigImages( SvtMiscOptions().AreCurrentSymbolsLarge() );
    for ( sal_uInt16 nPos = 0; nPos < m_pToolBar->GetItemCount(); nPos++ )
    {
        sal_uInt16 nId( m_pToolBar->GetItemId( nPos ) );

        if ( nId > 0 )
        {
            OUString aCommandURL = m_pToolBar->GetItemCommand( nId );
            OUString aImageId;
            AddonsParams* pRuntimeItemData = static_cast<AddonsParams*>(m_pToolBar->GetItemData( nId ));
            if ( pRuntimeItemData )
                aImageId  = pRuntimeItemData->aImageId;

            m_pToolBar->SetItemImage(
                nId,
                RetrieveImage( m_xFrame, aImageId, aCommandURL, bBigImages )
            );
        }
    }
    m_pToolBar->SetToolboxButtonSize( bBigImages ? ToolBoxButtonSize::Large : ToolBoxButtonSize::Small );
    ::Size aSize = m_pToolBar->CalcWindowSizePixel();
    m_pToolBar->SetOutputSizePixel( aSize );
}

void AddonsToolBarManager::FillToolbar( const Sequence< Sequence< PropertyValue > >& rAddonToolbar )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        return;

    sal_uInt16 nId( 1 );

    RemoveControllers();

    m_pToolBar->Clear();
    m_aControllerMap.clear();

    OUString aModuleIdentifier;
    try
    {
        Reference< XModuleManager2 > xModuleManager = ModuleManager::create( m_xContext );
        aModuleIdentifier = xModuleManager->identify( m_xFrame );
    }
    catch ( const Exception& )
    {
    }

    sal_uInt32  nElements( 0 );
    bool    bAppendSeparator( false );
    Reference< XWindow > xToolbarWindow = VCLUnoHelper::GetInterface( m_pToolBar );
    for ( sal_uInt32 n = 0; n < (sal_uInt32)rAddonToolbar.getLength(); n++ )
    {
        OUString   aURL;
        OUString   aTitle;
        OUString   aImageId;
        OUString   aContext;
        OUString   aTarget;
        OUString   aControlType;
        sal_uInt16 nWidth( 0 );

        const Sequence< PropertyValue >& rSeq = rAddonToolbar[n];

        ToolBarMerger::ConvertSequenceToValues( rSeq, aURL, aTitle, aImageId, aTarget, aContext, aControlType, nWidth );

        if ( IsCorrectContext( aModuleIdentifier, aContext ))
        {
            if ( aURL == "private:separator" ) // toolbox item separator
            {
                sal_uInt16 nCount = m_pToolBar->GetItemCount();
                if ( nCount > 0 && ( m_pToolBar->GetItemType( nCount-1 ) != ToolBoxItemType::SEPARATOR ) && nElements > 0 )
                {
                    nElements = 0;
                    m_pToolBar->InsertSeparator();
                }
            }
            else
            {
                sal_uInt16 nCount = m_pToolBar->GetItemCount();
                if ( bAppendSeparator && nCount > 0 && ( m_pToolBar->GetItemType( nCount-1 ) != ToolBoxItemType::SEPARATOR ))
                {
                    // We have to append a separator first if the last item is not a separator
                    m_pToolBar->InsertSeparator();
                }
                bAppendSeparator = false;


                m_pToolBar->InsertItem( nId, aTitle );

                OUString aShortcut(vcl::CommandInfoProvider::GetCommandShortcut(aURL, m_xFrame));
                if (!aShortcut.isEmpty())
                    m_pToolBar->SetQuickHelpText(nId, aTitle + " (" + aShortcut + ")");

                // don't setup images yet, AddonsToolbarWrapper::populateImages does that.

                // Create TbRuntimeItemData to hold additional information we will need in the future
                AddonsParams* pRuntimeItemData = new AddonsParams;
                pRuntimeItemData->aImageId  = aImageId;
                pRuntimeItemData->aTarget   = aTarget;
                pRuntimeItemData->nWidth    = nWidth;
                m_pToolBar->SetItemData( nId, pRuntimeItemData );
                m_pToolBar->SetItemCommand( nId, aURL );

                Reference< XStatusListener > xController;

                bool bMustBeInit( true );

                // Support external toolbar controller for add-ons!
                if ( m_xToolbarControllerFactory.is() &&
                     m_xToolbarControllerFactory->hasController( aURL, m_aModuleIdentifier ))
                {
                    Sequence< Any > aArgs(5);
                    PropertyValue   aPropValue;

                    aPropValue.Name     = "ModuleIdentifier";
                    aPropValue.Value    <<= m_aModuleIdentifier;
                    aArgs[0] <<= aPropValue;
                    aPropValue.Name     = "Frame";
                    aPropValue.Value    <<= m_xFrame;
                    aArgs[1] <<= aPropValue;
                    aPropValue.Name     = "ServiceManager";
                    aPropValue.Value    <<= Reference<XMultiServiceFactory>(m_xContext->getServiceManager(), UNO_QUERY_THROW);
                    aArgs[2] <<= aPropValue;
                    aPropValue.Name     = "ParentWindow";
                    aPropValue.Value    <<= xToolbarWindow;
                    aArgs[3] <<= aPropValue;
                    aPropValue.Name     = "ItemId";
                    aPropValue.Value    = makeAny( sal_Int32( nId ));
                    aArgs[4] <<= aPropValue;

                    try
                    {
                        xController.set( m_xToolbarControllerFactory->createInstanceWithArgumentsAndContext(
                                            aURL, aArgs, m_xContext ),
                                         UNO_QUERY );
                    }
                    catch ( uno::Exception& )
                    {
                    }
                    bMustBeInit = false; // factory called init already!
                }
                else
                {
                    ::cppu::OWeakObject* pController = nullptr;

                    pController = ToolBarMerger::CreateController( m_xContext, m_xFrame, m_pToolBar, aURL, nId, nWidth, aControlType );
                    xController.set( pController, UNO_QUERY );
                }

                // insert controller to the map
                m_aControllerMap[nId] = xController;

                Reference< XInitialization > xInit( xController, UNO_QUERY );
                if ( xInit.is() && bMustBeInit )
                {
                    PropertyValue aPropValue;
                    Sequence< Any > aArgs( 3 );
                    aPropValue.Name = "Frame";
                    aPropValue.Value <<= m_xFrame;
                    aArgs[0] <<= aPropValue;
                    aPropValue.Name = "CommandURL";
                    aPropValue.Value <<= aURL;
                    aArgs[1] <<= aPropValue;
                    aPropValue.Name = "ServiceManager";
                    aPropValue.Value <<= Reference<XMultiServiceFactory>(m_xContext->getServiceManager(), UNO_QUERY_THROW);
                    aArgs[2] <<= aPropValue;
                    try
                    {
                        xInit->initialize( aArgs );
                    }
                    catch ( const uno::Exception& )
                    {
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

                // Notify controller implementation to its listeners. Controller is now useable from outside.
                Reference< XUpdatable > xUpdatable( xController, UNO_QUERY );
                if ( xUpdatable.is() )
                {
                    try
                    {
                        xUpdatable->update();
                    }
                    catch ( const uno::Exception& )
                    {
                    }
                }

                ++nId;
                ++nElements;
            }
        }
    }

    AddFrameActionListener();
}

IMPL_LINK_NOARG(AddonsToolBarManager, Click, ToolBox *, void)
{
    if ( m_bDisposed )
        return;

    sal_uInt16 nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->click();
    }
}

IMPL_LINK_NOARG(AddonsToolBarManager, DoubleClick, ToolBox *, void)
{
    if ( m_bDisposed )
        return;

    sal_uInt16 nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->doubleClick();
    }
}

IMPL_LINK_NOARG(AddonsToolBarManager, Select, ToolBox *, void)
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

IMPL_LINK( AddonsToolBarManager, StateChanged, StateChangedType const *, pStateChangedType, void )
{
    if ( *pStateChangedType == StateChangedType::ControlBackground )
    {
        CheckAndUpdateImages();
    }
}

IMPL_LINK( AddonsToolBarManager, DataChanged, DataChangedEvent const *, pDataChangedEvent, void )
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
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
