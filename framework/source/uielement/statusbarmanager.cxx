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

#include <uielement/statusbarmanager.hxx>
#include <uielement/genericstatusbarcontroller.hxx>

#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/resetableguard.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <framework/addonsoptions.hxx>
#include <uielement/statusbarmerger.hxx>
#include <uielement/statusbaritem.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <stdtypes.h>
#include "services.h"
#include "general.h"
#include "properties.h"
#include <helper/mischelper.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/StatusbarControllerFactory.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Command.hpp>
#include <com/sun/star/ui/XStatusbarItem.hdl>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/statusbarcontroller.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/status.hxx>
#include <vcl/svapp.hxx>

#include <functional>

using namespace ::com::sun::star;

// Property names of a menu/menu item ItemDescriptor
static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_OFFSET[]      = "Offset";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";
static const char ITEM_DESCRIPTOR_WIDTH[]       = "Width";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";

namespace framework
{

namespace
{

template< class MAP >
struct lcl_UpdateController : public std::unary_function< typename MAP::value_type, void >
{
    void operator()( typename MAP::value_type &rElement ) const
    {
        try
        {
            if ( rElement.second.is() )
                rElement.second->update();
        }
        catch ( uno::Exception& )
        {
        }
    }
};

template< class MAP >
struct lcl_RemoveController : public std::unary_function< typename MAP::value_type, void >
{
    void operator()( typename MAP::value_type &rElement ) const
    {
        try
        {
            if ( rElement.second.is() )
                rElement.second->dispose();
        }
        catch ( uno::Exception& )
        {
        }
    }
};

static sal_uInt16 impl_convertItemStyleToItemBits( sal_Int16 nStyle )
{
    sal_uInt16 nItemBits( 0 );

    if (( nStyle & ::com::sun::star::ui::ItemStyle::ALIGN_RIGHT ) == ::com::sun::star::ui::ItemStyle::ALIGN_RIGHT )
        nItemBits |= SIB_RIGHT;
    else if ( nStyle & ::com::sun::star::ui::ItemStyle::ALIGN_LEFT )
        nItemBits |= SIB_LEFT;
    else
        nItemBits |= SIB_CENTER;

    if (( nStyle & ::com::sun::star::ui::ItemStyle::DRAW_FLAT ) == ::com::sun::star::ui::ItemStyle::DRAW_FLAT )
        nItemBits |= SIB_FLAT;
    else if ( nStyle & ::com::sun::star::ui::ItemStyle::DRAW_OUT3D )
        nItemBits |= SIB_OUT;
    else
        nItemBits |= SIB_IN;

    if (( nStyle & ::com::sun::star::ui::ItemStyle::AUTO_SIZE ) == ::com::sun::star::ui::ItemStyle::AUTO_SIZE )
        nItemBits |= SIB_AUTOSIZE;
    if ( nStyle & ::com::sun::star::ui::ItemStyle::OWNER_DRAW )
        nItemBits |= SIB_USERDRAW;

    return nItemBits;
}

}

StatusBarManager::StatusBarManager(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Reference< frame::XFrame >& rFrame,
    const OUString& rResourceName,
    StatusBar* pStatusBar ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_bDisposed( sal_False ),
    m_bFrameActionRegistered( sal_False ),
    m_bUpdateControllers( sal_False ),
    m_bModuleIdentified( sal_False ),
    m_pStatusBar( pStatusBar ),
    m_aResourceName( rResourceName ),
    m_xFrame( rFrame ),
    m_aListenerContainer( m_aLock.getShareableOslMutex() ),
    m_xContext( rxContext )
{

    m_xStatusbarControllerFactory = frame::StatusbarControllerFactory::create(
        ::comphelper::getProcessComponentContext());

    m_pStatusBar->SetClickHdl( LINK( this, StatusBarManager, Click ) );
    m_pStatusBar->SetDoubleClickHdl( LINK( this, StatusBarManager, DoubleClick ) );
}

StatusBarManager::~StatusBarManager()
{
}

StatusBar* StatusBarManager::GetStatusBar() const
{
    ResetableGuard aGuard( m_aLock );
    return m_pStatusBar;
}

void StatusBarManager::frameAction( const frame::FrameActionEvent& Action )
throw ( uno::RuntimeException )
{
    ResetableGuard aGuard( m_aLock );
    if ( Action.Action == frame::FrameAction_CONTEXT_CHANGED )
        UpdateControllers();
}

void SAL_CALL StatusBarManager::disposing( const lang::EventObject& Source ) throw ( uno::RuntimeException )
{
    {
        ResetableGuard aGuard( m_aLock );
        if ( m_bDisposed )
            return;
    }

    RemoveControllers();

    {
        ResetableGuard aGuard( m_aLock );
        if ( Source.Source == uno::Reference< uno::XInterface >( m_xFrame, uno::UNO_QUERY ))
            m_xFrame.clear();

        m_xContext.clear();
    }
}

// XComponent
void SAL_CALL StatusBarManager::dispose() throw( uno::RuntimeException )
{
    uno::Reference< lang::XComponent > xThis(
        static_cast< OWeakObject* >(this), uno::UNO_QUERY );

    lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
        if ( !m_bDisposed )
        {
            RemoveControllers();

            // destroy the item data
            for ( sal_uInt16 n = 0; n < m_pStatusBar->GetItemCount(); n++ )
            {
                AddonStatusbarItemData *pUserData = static_cast< AddonStatusbarItemData *>(
                    m_pStatusBar->GetItemData( m_pStatusBar->GetItemId( n ) ) );
                if ( pUserData )
                    delete pUserData;
            }

            delete m_pStatusBar;
            m_pStatusBar = 0;

            if ( m_bFrameActionRegistered && m_xFrame.is() )
            {
                try
                {
                    m_xFrame->removeFrameActionListener( uno::Reference< frame::XFrameActionListener >(
                                                            static_cast< ::cppu::OWeakObject *>( this ),
                                                            uno::UNO_QUERY ));
                }
                catch ( const uno::Exception& )
                {
                }
            }

            m_xFrame.clear();
            m_xContext.clear();

            m_bDisposed = sal_True;
        }
    }
}

void SAL_CALL StatusBarManager::addEventListener( const uno::Reference< lang::XEventListener >& xListener ) throw( uno::RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw lang::DisposedException();

    m_aListenerContainer.addInterface( ::getCppuType(
        ( const uno::Reference< lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL StatusBarManager::removeEventListener( const uno::Reference< lang::XEventListener >& xListener ) throw( uno::RuntimeException )
{
    m_aListenerContainer.removeInterface( ::getCppuType(
        ( const uno::Reference< lang::XEventListener >* ) NULL ), xListener );
}

// XUIConfigurationListener
void SAL_CALL StatusBarManager::elementInserted( const css::ui::ConfigurationEvent& ) throw ( uno::RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;
}

void SAL_CALL StatusBarManager::elementRemoved( const css::ui::ConfigurationEvent& ) throw ( uno::RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;
}

void SAL_CALL StatusBarManager::elementReplaced( const css::ui::ConfigurationEvent& ) throw ( uno::RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;
}

void StatusBarManager::UpdateControllers()
{
    if ( !m_bUpdateControllers )
    {
        m_bUpdateControllers = sal_True;
        std::for_each( m_aControllerMap.begin(),
                       m_aControllerMap.end(),
                       lcl_UpdateController< StatusBarControllerMap >() );
    }
    m_bUpdateControllers = sal_False;
}

void StatusBarManager::RemoveControllers()
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    std::for_each( m_aControllerMap.begin(),
                   m_aControllerMap.end(),
                   lcl_RemoveController< StatusBarControllerMap >() );
    m_aControllerMap.clear();
}

OUString StatusBarManager::RetrieveLabelFromCommand( const OUString& aCmdURL )
{
    return framework::RetrieveLabelFromCommand(aCmdURL, m_xContext, m_xUICommandLabels,m_xFrame,m_aModuleIdentifier,m_bModuleIdentified,"Name");
}

void StatusBarManager::CreateControllers()
{
    uno::Reference< awt::XWindow > xStatusbarWindow = VCLUnoHelper::GetInterface( m_pStatusBar );

    for ( sal_uInt16 i = 0; i < m_pStatusBar->GetItemCount(); i++ )
    {
        sal_uInt16 nId = m_pStatusBar->GetItemId( i );
        if ( nId == 0 )
            continue;

        OUString aCommandURL( m_pStatusBar->GetItemCommand( nId ));
        sal_Bool bInit( sal_True );
        uno::Reference< frame::XStatusbarController > xController;
        AddonStatusbarItemData *pItemData = static_cast< AddonStatusbarItemData *>( m_pStatusBar->GetItemData( nId ) );
        uno::Reference< ui::XStatusbarItem > xStatusbarItem(
            static_cast< cppu::OWeakObject *>( new StatusbarItem( m_pStatusBar, pItemData, nId, aCommandURL ) ),
            uno::UNO_QUERY );

        beans::PropertyValue aPropValue;
        std::vector< uno::Any > aPropVector;

        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
        aPropValue.Value    <<= aCommandURL;
        aPropVector.push_back( uno::makeAny( aPropValue ) );

        aPropValue.Name     = OUString( "ModuleIdentifier" );
        aPropValue.Value    <<= m_aModuleIdentifier;
        aPropVector.push_back( uno::makeAny( aPropValue ) );

        aPropValue.Name     = OUString( "Frame" );
        aPropValue.Value    <<= m_xFrame;
        aPropVector.push_back( uno::makeAny( aPropValue ) );

        // TODO remove this
        aPropValue.Name     = OUString( "ServiceManager" );
        aPropValue.Value    = uno::makeAny( uno::Reference<lang::XMultiServiceFactory>(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW) );
        aPropVector.push_back( uno::makeAny( aPropValue ) );

        aPropValue.Name     = OUString( "ParentWindow" );
        aPropValue.Value    <<= xStatusbarWindow;
        aPropVector.push_back( uno::makeAny( aPropValue ) );

        // TODO still needing with the css::ui::XStatusbarItem?
        aPropValue.Name     = OUString( "Identifier" );
        aPropValue.Value    <<= nId;
        aPropVector.push_back( uno::makeAny( aPropValue ) );

        aPropValue.Name     = OUString( "StatusbarItem" );
        aPropValue.Value    <<= xStatusbarItem;
        aPropVector.push_back( uno::makeAny( aPropValue ) );

        uno::Sequence< uno::Any > aArgs( comphelper::containerToSequence( aPropVector ) );

        // 1) UNO Statusbar controllers, registered in Controllers.xcu
        if ( m_xStatusbarControllerFactory.is() &&
             m_xStatusbarControllerFactory->hasController( aCommandURL, m_aModuleIdentifier ))
        {
            xController = uno::Reference< frame::XStatusbarController >(
                            m_xStatusbarControllerFactory->createInstanceWithArgumentsAndContext(
                                aCommandURL, aArgs, m_xContext ),
                            uno::UNO_QUERY );
            bInit = sal_False; // Initialization is done through the factory service
        }

        if ( !xController.is() )
        {
            svt::StatusbarController* pController( 0 );

            // 2) Old SFX2 Statusbar controllers
            pController = CreateStatusBarController( m_xFrame, m_pStatusBar, nId, aCommandURL );
            if ( !pController )
            {
                // 3) Is Add-on? Generic statusbar controller
                if ( pItemData )
                {
                    pController = new GenericStatusbarController( m_xContext,
                                                                  m_xFrame,
                                                                  xStatusbarItem,
                                                                  pItemData );
                }
                else
                {
                    // 4) Default Statusbar controller
                    pController = new svt::StatusbarController( m_xContext, m_xFrame, aCommandURL, nId );
                }
            }

            if ( pController )
                xController = uno::Reference< frame::XStatusbarController >(
                                static_cast< ::cppu::OWeakObject *>( pController ),
                                uno::UNO_QUERY );
        }

        m_aControllerMap[nId] = xController;
        if ( bInit )
        {
            xController->initialize( aArgs );
        }
    }

    AddFrameActionListener();
}

void StatusBarManager::AddFrameActionListener()
{
    if ( !m_bFrameActionRegistered && m_xFrame.is() )
    {
        m_bFrameActionRegistered = sal_True;
        m_xFrame->addFrameActionListener( uno::Reference< frame::XFrameActionListener >(
            static_cast< ::cppu::OWeakObject *>( this ), uno::UNO_QUERY ));
    }
}

void StatusBarManager::FillStatusBar( const uno::Reference< container::XIndexAccess >& rItemContainer )
{

    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed || !m_pStatusBar )
        return;

    sal_uInt16         nId( 1 );

    RemoveControllers();

    // reset and fill command map
    m_pStatusBar->Clear();
    m_aControllerMap.clear();// TODO already done in RemoveControllers

    for ( sal_Int32 n = 0; n < rItemContainer->getCount(); n++ )
    {
        uno::Sequence< beans::PropertyValue >   aProp;
        OUString                           aCommandURL;
        OUString                           aHelpURL;
        sal_Int16                               nOffset( 0 );
        sal_Int16                               nStyle( 0 );
        sal_Int16                               nWidth( 0 );
        sal_uInt16                              nType( ::com::sun::star::ui::ItemType::DEFAULT );

        try
        {
            if ( rItemContainer->getByIndex( n ) >>= aProp )
            {
                for ( int i = 0; i < aProp.getLength(); i++ )
                {
                    if ( aProp[i].Name == ITEM_DESCRIPTOR_COMMANDURL )
                    {
                        aProp[i].Value >>= aCommandURL;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_HELPURL )
                    {
                        aProp[i].Value >>= aHelpURL;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_STYLE )
                    {
                        aProp[i].Value >>= nStyle;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_TYPE )
                    {
                        aProp[i].Value >>= nType;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_WIDTH )
                    {
                        aProp[i].Value >>= nWidth;
                    }
                    else if ( aProp[i].Name == ITEM_DESCRIPTOR_OFFSET )
                    {
                        aProp[i].Value >>= nOffset;
                    }
                }

                if (( nType == ::com::sun::star::ui::ItemType::DEFAULT ) && !aCommandURL.isEmpty() )
                {
                    OUString aString( RetrieveLabelFromCommand( aCommandURL ));
                    sal_uInt16        nItemBits( impl_convertItemStyleToItemBits( nStyle ));

                    m_pStatusBar->InsertItem( nId, nWidth, nItemBits, nOffset );
                    m_pStatusBar->SetItemCommand( nId, aCommandURL );
                    m_pStatusBar->SetAccessibleName( nId, aString );
                    ++nId;
                }
            }
        }
        catch ( const ::com::sun::star::lang::IndexOutOfBoundsException& )
        {
            break;
        }
    }

    // Statusbar Merging
    const sal_uInt16 STATUSBAR_ITEM_STARTID = 1000;
    MergeStatusbarInstructionContainer aMergeInstructions = AddonsOptions().GetMergeStatusbarInstructions();
    if ( !aMergeInstructions.empty() )
    {
        const sal_uInt32 nCount = aMergeInstructions.size();
        sal_uInt16 nItemId( STATUSBAR_ITEM_STARTID );

        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            MergeStatusbarInstruction &rInstruction = aMergeInstructions[i];
            if ( !StatusbarMerger::IsCorrectContext( rInstruction.aMergeContext, m_aModuleIdentifier ) )
                continue;

            AddonStatusbarItemContainer aItems;
            StatusbarMerger::ConvertSeqSeqToVector( rInstruction.aMergeStatusbarItems, aItems );

            sal_uInt16 nRefPos = StatusbarMerger::FindReferencePos( m_pStatusBar, rInstruction.aMergePoint );
            if ( nRefPos != STATUSBAR_ITEM_NOTFOUND )
            {
                StatusbarMerger::ProcessMergeOperation( m_pStatusBar,
                                                        nRefPos,
                                                        nItemId,
                                                        m_aModuleIdentifier,
                                                        rInstruction.aMergeCommand,
                                                        rInstruction.aMergeCommandParameter,
                                                        aItems );
            }
            else
            {
                StatusbarMerger::ProcessMergeFallback( m_pStatusBar,
                                                       nRefPos,
                                                       nItemId,
                                                       m_aModuleIdentifier,
                                                       rInstruction.aMergeCommand,
                                                       rInstruction.aMergeCommandParameter,
                                                       aItems );
            }
        }
    }

    // Create controllers
    CreateControllers();

    // Notify controllers that they are now correctly initialized and can start listening
    UpdateControllers();
}

void StatusBarManager::StateChanged( StateChangedType )
{
}

void StatusBarManager::DataChanged( const DataChangedEvent& rDCEvt )
{
    ResetableGuard aGuard( m_aLock );

    if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS         ) ||
         ( rDCEvt.GetType() == DATACHANGED_FONTS            ) ||
         ( rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION ) ||
         ( rDCEvt.GetType() == DATACHANGED_DISPLAY          ))  &&
         ( rDCEvt.GetFlags() & SETTINGS_STYLE               ))
    {
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        css::uno::Reference< css::beans::XPropertySet > xPropSet( m_xFrame, css::uno::UNO_QUERY );
        if ( xPropSet.is() )
            xPropSet->getPropertyValue("LayoutManager") >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            aGuard.unlock();
            xLayoutManager->doLayout();
        }
    }
}

void StatusBarManager::UserDraw( const UserDrawEvent& rUDEvt )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    sal_uInt16 nId( rUDEvt.GetItemId() );
    StatusBarControllerMap::const_iterator it = m_aControllerMap.find( nId );
    if (( nId > 0 ) && ( it != m_aControllerMap.end() ))
    {
        uno::Reference< frame::XStatusbarController > xController( it->second );
        if ( xController.is() && rUDEvt.GetDevice() )
        {
            uno::Reference< awt::XGraphics > xGraphics =
                rUDEvt.GetDevice()->CreateUnoGraphics();

            awt::Rectangle aRect( rUDEvt.GetRect().Left(),
                                  rUDEvt.GetRect().Top(),
                                  rUDEvt.GetRect().GetWidth(),
                                  rUDEvt.GetRect().GetHeight() );
            aGuard.unlock();
            xController->paint( xGraphics, aRect, rUDEvt.GetStyle() );
        }
    }
}

void StatusBarManager::Command( const CommandEvent& rEvt )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    if ( rEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        sal_uInt16 nId = m_pStatusBar->GetItemId( rEvt.GetMousePosPixel() );
        StatusBarControllerMap::const_iterator it = m_aControllerMap.find( nId );
        if (( nId > 0 ) && ( it != m_aControllerMap.end() ))
        {
            uno::Reference< frame::XStatusbarController > xController( it->second );
            if ( xController.is() )
            {
                awt::Point aPos;
                aPos.X = rEvt.GetMousePosPixel().X();
                aPos.Y = rEvt.GetMousePosPixel().Y();
                xController->command( aPos, awt::Command::CONTEXTMENU, sal_True, uno::Any() );
            }
        }
    }
}

void StatusBarManager::MouseMove( const MouseEvent& rMEvt )
{
    MouseButton(rMEvt,&frame::XStatusbarController::mouseMove);
}

void StatusBarManager::MouseButton( const MouseEvent& rMEvt ,sal_Bool ( SAL_CALL frame::XStatusbarController::*_pMethod )(const ::com::sun::star::awt::MouseEvent&))
{
    ResetableGuard aGuard( m_aLock );

    if ( !m_bDisposed )
    {
        sal_uInt16 nId = m_pStatusBar->GetItemId( rMEvt.GetPosPixel() );
        StatusBarControllerMap::const_iterator it = m_aControllerMap.find( nId );
        if (( nId > 0 ) && ( it != m_aControllerMap.end() ))
        {
            uno::Reference< frame::XStatusbarController > xController( it->second );
            if ( xController.is() )
            {
                ::com::sun::star::awt::MouseEvent aMouseEvent;
                aMouseEvent.Buttons = rMEvt.GetButtons();
                aMouseEvent.X = rMEvt.GetPosPixel().X();
                aMouseEvent.Y = rMEvt.GetPosPixel().Y();
                aMouseEvent.ClickCount = rMEvt.GetClicks();
                (xController.get()->*_pMethod)( aMouseEvent);
            }
        }
    }
}

void StatusBarManager::MouseButtonDown( const MouseEvent& rMEvt )
{
    MouseButton(rMEvt,&frame::XStatusbarController::mouseButtonDown);
}

void StatusBarManager::MouseButtonUp( const MouseEvent& rMEvt )
{
    MouseButton(rMEvt,&frame::XStatusbarController::mouseButtonUp);
}

IMPL_LINK_NOARG(StatusBarManager, Click)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    sal_uInt16 nId = m_pStatusBar->GetCurItemId();
    StatusBarControllerMap::const_iterator it = m_aControllerMap.find( nId );
    if (( nId > 0 ) && ( it != m_aControllerMap.end() ))
    {
        uno::Reference< frame::XStatusbarController > xController( it->second );
        if ( xController.is() )
        {
            const Point aVCLPos = m_pStatusBar->GetPointerPosPixel();
            const awt::Point aAWTPoint( aVCLPos.X(), aVCLPos.Y() );
            xController->click( aAWTPoint );
        }
    }

    return 1;
}

IMPL_LINK_NOARG(StatusBarManager, DoubleClick)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    sal_uInt16 nId = m_pStatusBar->GetCurItemId();
    StatusBarControllerMap::const_iterator it = m_aControllerMap.find( nId );
    if (( nId > 0 ) && ( it != m_aControllerMap.end() ))
    {
        uno::Reference< frame::XStatusbarController > xController( it->second );
        if ( xController.is() )
        {
            const Point aVCLPos = m_pStatusBar->GetPointerPosPixel();
            const awt::Point aAWTPoint( aVCLPos.X(), aVCLPos.Y() );
            xController->doubleClick( aAWTPoint );
        }
    }

    return 1;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
