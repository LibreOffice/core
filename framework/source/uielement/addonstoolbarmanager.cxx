/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <uielement/addonstoolbarmanager.hxx>
#include <uielement/toolbarmerger.hxx>

#include <uielement/toolbar.hxx>
#include <uielement/generictoolbarcontroller.hxx>
#include <threadhelp/resetableguard.hxx>
#include "services.h"
#include <framework/imageproducer.hxx>
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
#include <uielement/toolbarmerger.hxx>

#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/DockingArea.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <svtools/imgdef.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/unohlp.hxx>

#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui;

namespace framework
{

static const char   TOOLBOXITEM_SEPARATOR_STR[] = "private:separator";
static const sal_uInt16 TOOLBOXITEM_SEPARATOR_STR_LEN = sizeof( TOOLBOXITEM_SEPARATOR_STR )-1;

AddonsToolBarManager::AddonsToolBarManager( const Reference< XMultiServiceFactory >& rServiceManager,
                                const Reference< XFrame >& rFrame,
                                const rtl::OUString& rResourceName,
                                ToolBar* pToolBar ) :
    ToolBarManager( rServiceManager, rFrame, rResourceName, pToolBar )
{
    m_pToolBar->SetMenuType( TOOLBOX_MENUTYPE_CLIPPEDITEMS );
    m_pToolBar->SetSelectHdl( LINK( this, AddonsToolBarManager, Select) );
    m_pToolBar->SetActivateHdl( LINK( this, AddonsToolBarManager, Activate) );
    m_pToolBar->SetDeactivateHdl( LINK( this, AddonsToolBarManager, Deactivate) );
    m_pToolBar->SetClickHdl( LINK( this, AddonsToolBarManager, Click ) );
    m_pToolBar->SetDoubleClickHdl( LINK( this, AddonsToolBarManager, DoubleClick ) );
    m_pToolBar->SetCommandHdl( LINK( this, AddonsToolBarManager, Command ) );
    m_pToolBar->SetStateChangedHdl( LINK( this, AddonsToolBarManager, StateChanged ) );
    m_pToolBar->SetDataChangedHdl( LINK( this, AddonsToolBarManager, DataChanged ) );
}

AddonsToolBarManager::~AddonsToolBarManager()
{
}

static sal_Bool IsCorrectContext( const ::rtl::OUString& rModuleIdentifier, const ::rtl::OUString& aContextList )
{
    if ( aContextList.isEmpty() )
        return sal_True;

    if ( !rModuleIdentifier.isEmpty() )
    {
        sal_Int32 nIndex = aContextList.indexOf( rModuleIdentifier );
        return ( nIndex >= 0 );
    }

    return sal_False;
}

static Image RetrieveImage( Reference< com::sun::star::frame::XFrame >& rFrame,
                            const rtl::OUString& aImageId,
                            const rtl::OUString& aURL,
                            sal_Bool bBigImage
)
{
    Image aImage;

    if ( !aImageId.isEmpty() )
    {
        aImage = framework::AddonsOptions().GetImageFromURL( aImageId, bBigImage );
        if ( !!aImage )
            return aImage;
        else
            aImage = GetImageFromURL( rFrame, aImageId, bBigImage );
        if ( !!aImage )
            return aImage;
    }

    aImage = framework::AddonsOptions().GetImageFromURL( aURL, bBigImage );
    if ( !aImage )
        aImage = GetImageFromURL( rFrame, aImageId, bBigImage );

    return aImage;
}

// XComponent
void SAL_CALL AddonsToolBarManager::dispose() throw( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        // Remove addon specific data from toolbar items.
        ResetableGuard aGuard( m_aLock );
        for ( sal_uInt16 n = 0; n < m_pToolBar->GetItemCount(); n++ )
        {
            sal_uInt16 nId( m_pToolBar->GetItemId( n ) );

            if ( nId > 0 )
            {
                AddonsParams* pRuntimeItemData = (AddonsParams*)m_pToolBar->GetItemData( nId );
                if ( pRuntimeItemData )
                    delete pRuntimeItemData;
                m_pToolBar->SetItemData( nId, NULL );
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
    sal_Bool  bBigImages( SvtMiscOptions().AreCurrentSymbolsLarge() );
    for ( sal_uInt16 nPos = 0; nPos < m_pToolBar->GetItemCount(); nPos++ )
    {
        sal_uInt16 nId( m_pToolBar->GetItemId( nPos ) );

        if ( nId > 0 )
        {
            ::rtl::OUString aCommandURL = m_pToolBar->GetItemCommand( nId );
            ::rtl::OUString aImageId;
            AddonsParams* pRuntimeItemData = (AddonsParams*)m_pToolBar->GetItemData( nId );
            if ( pRuntimeItemData )
                aImageId  = pRuntimeItemData->aImageId;

            m_pToolBar->SetItemImage(
                nId,
                RetrieveImage( m_xFrame, aImageId, aCommandURL, bBigImages )
            );
        }
    }
    m_pToolBar->SetToolboxButtonSize( bBigImages ? TOOLBOX_BUTTONSIZE_LARGE : TOOLBOX_BUTTONSIZE_SMALL );
    ::Size aSize = m_pToolBar->CalcWindowSizePixel();
    m_pToolBar->SetOutputSizePixel( aSize );
}

void AddonsToolBarManager::FillToolbar( const Sequence< Sequence< PropertyValue > >& rAddonToolbar )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    sal_uInt16    nId( 1 );

    RemoveControllers();

    m_pToolBar->Clear();
    m_aControllerMap.clear();

    ::rtl::OUString aModuleIdentifier;
    try
    {
        Reference< XModuleManager2 > xModuleManager = ModuleManager::create( comphelper::getComponentContext(m_xServiceManager) );
        aModuleIdentifier = xModuleManager->identify( m_xFrame );
    }
    catch ( const Exception& )
    {
    }

    Reference< XMultiComponentFactory > xToolbarControllerFactory( m_xToolbarControllerRegistration, UNO_QUERY );
    Reference< XComponentContext > xComponentContext(
        comphelper::getComponentContext( m_xServiceManager ) );

    sal_uInt32  nElements( 0 );
    sal_Bool    bAppendSeparator( sal_False );
    Reference< XWindow > xToolbarWindow = VCLUnoHelper::GetInterface( m_pToolBar );
    for ( sal_uInt32 n = 0; n < (sal_uInt32)rAddonToolbar.getLength(); n++ )
    {
        rtl::OUString   aURL;
        rtl::OUString   aTitle;
        rtl::OUString   aImageId;
        rtl::OUString   aContext;
        rtl::OUString   aTarget;
        rtl::OUString   aControlType;
        sal_uInt16      nWidth( 0 );

        const Sequence< PropertyValue >& rSeq = rAddonToolbar[n];

        ToolBarMerger::ConvertSequenceToValues( rSeq, aURL, aTitle, aImageId, aTarget, aContext, aControlType, nWidth );

        if ( IsCorrectContext( aModuleIdentifier, aContext ))
        {
            if ( aURL.equalsAsciiL( TOOLBOXITEM_SEPARATOR_STR, TOOLBOXITEM_SEPARATOR_STR_LEN ))
            {
                sal_uInt16 nCount = m_pToolBar->GetItemCount();
                if ( nCount > 0 && ( m_pToolBar->GetItemType( nCount-1 ) != TOOLBOXITEM_SEPARATOR ) && nElements > 0 )
                {
                    nElements = 0;
                    m_pToolBar->InsertSeparator();
                }
            }
            else
            {
                sal_uInt16 nCount = m_pToolBar->GetItemCount();
                if ( bAppendSeparator && nCount > 0 && ( m_pToolBar->GetItemType( nCount-1 ) != TOOLBOXITEM_SEPARATOR ))
                {
                    // We have to append a separator first if the last item is not a separator
                    m_pToolBar->InsertSeparator();
                }
                bAppendSeparator = sal_False;

                m_pToolBar->InsertItem( nId, aTitle );

                Image aImage = RetrieveImage( m_xFrame, aImageId, aURL, !m_bSmallSymbols );
                if ( !!aImage )
                    m_pToolBar->SetItemImage( nId, aImage );

                // Create TbRuntimeItemData to hold additional information we will need in the future
                AddonsParams* pRuntimeItemData = new AddonsParams;
                pRuntimeItemData->aImageId  = aImageId;
                pRuntimeItemData->aTarget   = aTarget;
                m_pToolBar->SetItemData( nId, pRuntimeItemData );
                m_pToolBar->SetItemCommand( nId, aURL );

                Reference< XStatusListener > xController;

                sal_Bool bMustBeInit( sal_True );

                // Support external toolbar controller for add-ons!
                if ( m_xToolbarControllerRegistration.is() &&
                     m_xToolbarControllerRegistration->hasController( aURL, m_aModuleIdentifier ))
                {
                    if ( xToolbarControllerFactory.is() )
                    {
                        Sequence< Any > aArgs(5);
                        PropertyValue   aPropValue;

                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
                        aPropValue.Value    <<= m_aModuleIdentifier;
                        aArgs[0] <<= aPropValue;
                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                        aPropValue.Value    <<= m_xFrame;
                        aArgs[1] <<= aPropValue;
                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                        aPropValue.Value    <<= m_xServiceManager;
                        aArgs[2] <<= aPropValue;
                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ));
                        aPropValue.Value    <<= xToolbarWindow;
                        aArgs[3] <<= aPropValue;
                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ItemId" ));
                        aPropValue.Value    = makeAny( sal_Int32( nId ));
                        aArgs[4] <<= aPropValue;

                        try
                        {
                            xController = Reference< XStatusListener >( xToolbarControllerFactory->createInstanceWithArgumentsAndContext(
                                                                            aURL, aArgs, xComponentContext ),
                                                                        UNO_QUERY );
                        }
                        catch ( const uno::Exception& )
                        {
                        }
                        bMustBeInit = sal_False; // factory called init already!
                    }
                }
                else
                {
                    ::cppu::OWeakObject* pController = 0;

                    pController = ToolBarMerger::CreateController( m_xServiceManager, m_xFrame, m_pToolBar, aURL, nId, nWidth, aControlType );
                    xController = Reference< XStatusListener >( pController, UNO_QUERY );
                }

                // insert controller to the map
                m_aControllerMap[nId] = xController;

                Reference< XInitialization > xInit( xController, UNO_QUERY );
                if ( xInit.is() && bMustBeInit )
                {
                    PropertyValue aPropValue;
                    Sequence< Any > aArgs( 3 );
                    aPropValue.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                    aPropValue.Value <<= m_xFrame;
                    aArgs[0] <<= aPropValue;
                    aPropValue.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
                    aPropValue.Value <<= aURL;
                    aArgs[1] <<= aPropValue;
                    aPropValue.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                    aPropValue.Value <<= m_xServiceManager;
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
                        Window* pItemWin = VCLUnoHelper::GetWindow( xWindow );
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

IMPL_LINK_NOARG(AddonsToolBarManager, Click)
{
    if ( m_bDisposed )
        return 1;

    sal_uInt16 nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->click();
    }

    return 1;
}

IMPL_LINK_NOARG(AddonsToolBarManager, DoubleClick)
{
    if ( m_bDisposed )
        return 1;

    sal_uInt16 nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->doubleClick();
    }

    return 1;
}

IMPL_LINK_NOARG(AddonsToolBarManager, Command)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    return 0;
}

IMPL_LINK_NOARG(AddonsToolBarManager, Select)
{
    if ( m_bDisposed )
        return 1;

    sal_Int16   nKeyModifier( (sal_Int16)m_pToolBar->GetModifier() );
    sal_uInt16      nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->execute( nKeyModifier );
    }

    return 1;
}

IMPL_LINK_NOARG(AddonsToolBarManager, Activate)
{
    return 1;
}

IMPL_LINK_NOARG(AddonsToolBarManager, Deactivate)
{
    return 1;
}

IMPL_LINK( AddonsToolBarManager, StateChanged, StateChangedType*, pStateChangedType )
{
    if ( *pStateChangedType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        CheckAndUpdateImages();
    }
    return 1;
}

IMPL_LINK( AddonsToolBarManager, DataChanged, DataChangedEvent*, pDataChangedEvent  )
{
    if ((( pDataChangedEvent->GetType() == DATACHANGED_SETTINGS )   ||
        (  pDataChangedEvent->GetType() == DATACHANGED_DISPLAY  ))  &&
        ( pDataChangedEvent->GetFlags() & SETTINGS_STYLE        ))
    {
        CheckAndUpdateImages();
    }

    for ( sal_uInt16 nPos = 0; nPos < m_pToolBar->GetItemCount(); ++nPos )
    {
        const sal_uInt16 nId = m_pToolBar->GetItemId(nPos);
        Window* pWindow = m_pToolBar->GetItemWindow( nId );
        if ( pWindow )
        {
            const DataChangedEvent& rDCEvt( *pDataChangedEvent );
            pWindow->DataChanged( rDCEvt );
        }
    }

    return 1;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
