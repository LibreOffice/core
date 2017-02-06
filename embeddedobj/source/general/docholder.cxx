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

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/frame/TaskCreator.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XControllerBorder.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/bridge/ModelDependent.hpp>
#include <com/sun/star/embed/XHatchWindow.hpp>
#include <com/sun/star/embed/HatchWindowFactory.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XMenuBarMergingAcceptor.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/ui/XDockingAreaAcceptor.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/embed/StateChangeInProgressException.hpp>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <osl/diagnose.h>
#include <rtl/process.h>
#include <vcl/svapp.hxx>
#include <svtools/embedhlp.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include "docholder.hxx"
#include "commonembobj.hxx"
#include "intercept.hxx"

#define HATCH_BORDER_WIDTH (((m_pEmbedObj->getStatus(embed::Aspects::MSOLE_CONTENT)&embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE) && \
                            m_pEmbedObj->getCurrentState()!=embed::EmbedStates::UI_ACTIVE) ? 0 : 4 )

using namespace ::com::sun::star;

class IntCounterGuard
{
    sal_Int32& m_rFlag;
public:
    explicit IntCounterGuard(sal_Int32& rFlag)
        : m_rFlag(rFlag)
    {
        ++m_rFlag;
    }

    ~IntCounterGuard()
    {
        if (m_rFlag)
            --m_rFlag;
    }
};

static void InsertMenu_Impl( const uno::Reference< container::XIndexContainer >& xTargetMenu,
                            sal_Int32 nTargetIndex,
                            const uno::Reference< container::XIndexAccess >& xSourceMenu,
                            sal_Int32 nSourceIndex,
                            const OUString& aContModuleName,
                            const uno::Reference< frame::XDispatchProvider >& xSourceDisp )
{
    sal_Int32 nInd = 0;
    OUString aModuleIdentPropName( "ModuleIdentifier" );
    OUString aDispProvPropName( "DispatchProvider" );
    bool bModuleNameSet = false;
    bool bDispProvSet = false;

    uno::Sequence< beans::PropertyValue > aSourceProps;
    xSourceMenu->getByIndex( nSourceIndex ) >>= aSourceProps;
    uno::Sequence< beans::PropertyValue > aTargetProps( aSourceProps.getLength() );
    for ( nInd = 0; nInd < aSourceProps.getLength(); nInd++ )
    {
        aTargetProps[nInd].Name = aSourceProps[nInd].Name;
        if ( !aContModuleName.isEmpty() && aTargetProps[nInd].Name.equals( aModuleIdentPropName ) )
        {
            aTargetProps[nInd].Value <<= aContModuleName;
            bModuleNameSet = true;
        }
        else if ( aTargetProps[nInd].Name.equals( aDispProvPropName ) )
        {
            aTargetProps[nInd].Value <<= xSourceDisp;
            bDispProvSet = true;
        }
        else
            aTargetProps[nInd].Value = aSourceProps[nInd].Value;
    }

    if ( !bModuleNameSet && !aContModuleName.isEmpty() )
    {
        aTargetProps.realloc( ++nInd );
        aTargetProps[nInd-1].Name = aModuleIdentPropName;
        aTargetProps[nInd-1].Value <<= aContModuleName;
    }

    if ( !bDispProvSet && xSourceDisp.is() )
    {
        aTargetProps.realloc( ++nInd );
        aTargetProps[nInd-1].Name = aDispProvPropName;
        aTargetProps[nInd-1].Value <<= xSourceDisp;
    }

    xTargetMenu->insertByIndex( nTargetIndex, uno::makeAny( aTargetProps ) );
}


DocumentHolder::DocumentHolder( const uno::Reference< uno::XComponentContext >& xContext,
                                OCommonEmbeddedObject* pEmbObj )
: m_pEmbedObj( pEmbObj ),
  m_xContext( xContext ),
  m_bReadOnly( false ),
  m_bWaitForClose( false ),
  m_bAllowClosing( false ),
  m_bDesktopTerminated( false ),
  m_nNoBorderResizeReact( 0 ),
  m_nNoResizeReact( 0 )
{
    m_aOutplaceFrameProps.realloc( 3 );
    beans::NamedValue aArg;

    aArg.Name = "TopWindow";
    aArg.Value <<= true;
    m_aOutplaceFrameProps[0] <<= aArg;

    aArg.Name = "MakeVisible";
    aArg.Value <<= false;
    m_aOutplaceFrameProps[1] <<= aArg;

    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( m_xContext );
    m_refCount++;
    try
    {
        xDesktop->addTerminateListener( this );
    }
    catch ( const uno::Exception& )
    {
    }
    m_refCount--;

    aArg.Name = "ParentFrame";
    aArg.Value <<= xDesktop; //TODO/LATER: should use parent document frame
    m_aOutplaceFrameProps[2] <<= aArg;
}


DocumentHolder::~DocumentHolder()
{
    m_refCount++; // to allow deregistration as a listener

    if( m_xFrame.is() )
        CloseFrame();

    if ( m_xComponent.is() )
    {
        try {
            CloseDocument( true, false );
        } catch( const uno::Exception& ) {}
    }

    if ( m_xInterceptor.is() )
    {
        m_xInterceptor->DisconnectDocHolder();
        m_xInterceptor.clear();
    }

    if ( !m_bDesktopTerminated )
        FreeOffice();
}


void DocumentHolder::CloseFrame()
{
    uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xFrame, uno::UNO_QUERY );
    if ( xCloseBroadcaster.is() )
        xCloseBroadcaster->removeCloseListener( static_cast<util::XCloseListener*>(this) );

    uno::Reference<util::XCloseable> xCloseable(
        m_xFrame,uno::UNO_QUERY );
    if( xCloseable.is() )
        try {
            xCloseable->close( true );
        }
        catch( const uno::Exception& ) {
        }
    else {
        uno::Reference<lang::XComponent> xComp( m_xFrame,uno::UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();
    }

    uno::Reference< lang::XComponent > xComp( m_xHatchWindow, uno::UNO_QUERY );
    if ( xComp.is() )
        xComp->dispose();

    m_xHatchWindow.clear();
    m_xOwnWindow.clear();
    m_xFrame.clear();
}


void DocumentHolder::FreeOffice()
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( m_xContext );
    xDesktop->removeTerminateListener( this );

    // the following code is commented out since for now there is still no completely correct way to detect
    // whether the office can be terminated, so it is better to have unnecessary process running than
    // to lose any data

//      uno::Reference< frame::XFramesSupplier > xFramesSupplier( xDesktop, uno::UNO_QUERY );
//      if ( xFramesSupplier.is() )
//      {
//          uno::Reference< frame::XFrames > xFrames = xFramesSupplier->getFrames();
//          if ( xFrames.is() && !xFrames->hasElements() )
//          {
//              try
//              {
//                  xDesktop->terminate();
//              }
//              catch( uno::Exception & )
//              {}
//          }
//      }
}


void DocumentHolder::CloseDocument( bool bDeliverOwnership, bool bWaitForClose )
{
    uno::Reference< util::XCloseBroadcaster > xBroadcaster( m_xComponent, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
    {
        uno::Reference< document::XEventBroadcaster > xEventBroadcaster( m_xComponent, uno::UNO_QUERY );
        if ( xEventBroadcaster.is() )
            xEventBroadcaster->removeEventListener( static_cast<document::XEventListener*>(this) );
        else
        {
            // the object does not support document::XEventBroadcaster interface
            // use the workaround, register for modified events
            uno::Reference< util::XModifyBroadcaster > xModifyBroadcaster( m_xComponent, uno::UNO_QUERY );
            if ( xModifyBroadcaster.is() )
                xModifyBroadcaster->removeModifyListener( static_cast<util::XModifyListener*>(this) );
        }

        uno::Reference< util::XCloseable > xCloseable( xBroadcaster, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            m_bAllowClosing = true;
            m_bWaitForClose = bWaitForClose;
            xCloseable->close( bDeliverOwnership );
        }
    }

    m_xComponent = nullptr;
}


void DocumentHolder::PlaceFrame( const awt::Rectangle& aNewRect )
{
    OSL_ENSURE( m_xFrame.is() && m_xOwnWindow.is(),
                "The object does not have windows required for inplace mode!" );

    //TODO: may need mutex locking???
    if ( m_xFrame.is() && m_xOwnWindow.is() )
    {
        // the frame can be replaced only in inplace mode
        frame::BorderWidths aOldWidths;
        IntCounterGuard aGuard( m_nNoBorderResizeReact );

        do
        {
            aOldWidths = m_aBorderWidths;

            awt::Rectangle aHatchRect = AddBorderToArea( aNewRect );

            ResizeWindows_Impl( aHatchRect );

        } while ( aOldWidths.Left != m_aBorderWidths.Left
               || aOldWidths.Top != m_aBorderWidths.Top
               || aOldWidths.Right != m_aBorderWidths.Right
               || aOldWidths.Bottom != m_aBorderWidths.Bottom );

        m_aObjRect = aNewRect;
    }
}


void DocumentHolder::ResizeWindows_Impl( const awt::Rectangle& aHatchRect )
{
    OSL_ENSURE( m_xFrame.is() && m_xOwnWindow.is() /*&& m_xHatchWindow.is()*/,
                "The object does not have windows required for inplace mode!" );
    if ( m_xHatchWindow.is() )
    {
        m_xOwnWindow->setPosSize( HATCH_BORDER_WIDTH,
                                  HATCH_BORDER_WIDTH,
                                  aHatchRect.Width - 2*HATCH_BORDER_WIDTH,
                                  aHatchRect.Height - 2*HATCH_BORDER_WIDTH,
                                  awt::PosSize::POSSIZE );


        m_xHatchWindow->setPosSize( aHatchRect.X,
                                    aHatchRect.Y,
                                    aHatchRect.Width,
                                    aHatchRect.Height,
                                    awt::PosSize::POSSIZE );
    }
    else
        m_xOwnWindow->setPosSize( aHatchRect.X + HATCH_BORDER_WIDTH,
                                  aHatchRect.Y + HATCH_BORDER_WIDTH,
                                  aHatchRect.Width - 2*HATCH_BORDER_WIDTH,
                                  aHatchRect.Height - 2*HATCH_BORDER_WIDTH,
                                  awt::PosSize::POSSIZE );
}


bool DocumentHolder::SetFrameLMVisibility( const uno::Reference< frame::XFrame >& xFrame, bool bVisible )
{
    bool bResult = false;

    try
    {
        uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        uno::Reference< beans::XPropertySet > xPropSet( xFrame, uno::UNO_QUERY_THROW );
        xPropSet->getPropertyValue("LayoutManager") >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            xLayoutManager->setVisible( bVisible );

            // MBA: locking is done only on the container LM, because it is not about hiding windows, it's about
            // giving up control over the component window (and stopping to listen for resize events of the container window)
            if ( bVisible )
                xLayoutManager->unlock();
            else
                xLayoutManager->lock();

            bResult = true;
        }
    }
    catch( const uno::Exception& )
    {}

    return bResult;
}


bool DocumentHolder::ShowInplace( const uno::Reference< awt::XWindowPeer >& xParent,
                                      const awt::Rectangle& aRectangleToShow,
                                      const uno::Reference< frame::XDispatchProvider >& xContDisp )
{
    OSL_ENSURE( !m_xFrame.is(), "A frame exists already!" );

    if ( !m_xFrame.is() )
    {
        uno::Reference < frame::XModel > xModel( GetComponent(), uno::UNO_QUERY );
        awt::Rectangle aHatchRectangle = AddBorderToArea( aRectangleToShow );

        awt::Rectangle aOwnRectangle(  HATCH_BORDER_WIDTH,
                                    HATCH_BORDER_WIDTH,
                                    aHatchRectangle.Width - 2*HATCH_BORDER_WIDTH,
                                    aHatchRectangle.Height - 2*HATCH_BORDER_WIDTH );
        uno::Reference< awt::XWindow > xHWindow;
        uno::Reference< awt::XWindowPeer > xMyParent( xParent );

        if ( xModel.is() )
        {

            uno::Reference< embed::XHatchWindowFactory > xHatchFactory =
                    embed::HatchWindowFactory::create(m_xContext);

            uno::Reference< embed::XHatchWindow > xHatchWindow =
                            xHatchFactory->createHatchWindowInstance( xParent,
                                                                      aHatchRectangle,
                                                                      awt::Size( HATCH_BORDER_WIDTH, HATCH_BORDER_WIDTH ) );

            uno::Reference< awt::XWindowPeer > xHatchWinPeer( xHatchWindow, uno::UNO_QUERY );
            xHWindow.set( xHatchWinPeer, uno::UNO_QUERY );
            if ( !xHWindow.is() )
                throw uno::RuntimeException(); // TODO: can not create own window

            xHatchWindow->setController( uno::Reference< embed::XHatchWindowController >(
                                                static_cast< embed::XHatchWindowController* >( this ) ) );

            xMyParent = xHatchWinPeer;
        }
        else
        {
            aOwnRectangle.X += aHatchRectangle.X;
            aOwnRectangle.Y += aHatchRectangle.Y;
        }

        awt::WindowDescriptor aOwnWinDescriptor( awt::WindowClass_TOP,
                                                "dockingwindow",
                                                xMyParent,
                                                0,
                                                awt::Rectangle(),//aOwnRectangle,
                                                awt::WindowAttribute::SHOW | awt::VclWindowPeerAttribute::CLIPCHILDREN );

        uno::Reference< awt::XToolkit2 > xToolkit = awt::Toolkit::create(m_xContext);

        uno::Reference< awt::XWindowPeer > xNewWinPeer = xToolkit->createWindow( aOwnWinDescriptor );
        uno::Reference< awt::XWindow > xOwnWindow( xNewWinPeer, uno::UNO_QUERY );
        if ( !xOwnWindow.is() )
            throw uno::RuntimeException(); // TODO: can not create own window

        // create a frame based on the specified window
        uno::Reference< lang::XSingleServiceFactory > xFrameFact = frame::TaskCreator::create(m_xContext);

        uno::Sequence< uno::Any > aArgs( 2 );
        beans::NamedValue aArg;

        aArg.Name    = "ContainerWindow";
        aArg.Value <<= xOwnWindow;
        aArgs[0] <<= aArg;

        uno::Reference< frame::XFrame > xContFrame( xContDisp, uno::UNO_QUERY );
        if ( xContFrame.is() )
        {
            aArg.Name    = "ParentFrame";
            aArg.Value <<= xContFrame;
            aArgs[1] <<= aArg;
        }
        else
            aArgs.realloc( 1 );

        // the call will create, initialize the frame, and register it in the parent
        m_xFrame.set( xFrameFact->createInstanceWithArguments( aArgs ), uno::UNO_QUERY_THROW );

        m_xHatchWindow = xHWindow;
        m_xOwnWindow = xOwnWindow;

        if ( !SetFrameLMVisibility( m_xFrame, false ) )
        {
            OSL_FAIL( "Can't deactivate LayoutManager!\n" );
            // TODO/LATER: error handling?
        }

        // m_bIsInplace = sal_True; TODO: ?

        uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xFrame, uno::UNO_QUERY );
        if ( xCloseBroadcaster.is() )
            xCloseBroadcaster->addCloseListener( static_cast<util::XCloseListener*>(this) );

        // TODO: some listeners to the frame and the window ( resize for example )
    }

    if ( m_xComponent.is() )
    {
        if ( !LoadDocToFrame( true ) )
        {
            CloseFrame();
            return false;
        }

        uno::Reference< frame::XControllerBorder > xControllerBorder( m_xFrame->getController(), uno::UNO_QUERY );
        if ( xControllerBorder.is() )
        {
            m_aBorderWidths = xControllerBorder->getBorder();
            xControllerBorder->addBorderResizeListener( static_cast<frame::XBorderResizeListener*>(this) );
        }

        PlaceFrame( aRectangleToShow );

        if ( m_xHatchWindow.is() )
            m_xHatchWindow->setVisible( true );

        return true;
    }

    return false;
}


uno::Reference< container::XIndexAccess > DocumentHolder::RetrieveOwnMenu_Impl()
{
    uno::Reference< container::XIndexAccess > xResult;

    uno::Reference< css::ui::XUIConfigurationManagerSupplier > xUIConfSupplier(
                m_xComponent,
                uno::UNO_QUERY );
    uno::Reference< css::ui::XUIConfigurationManager > xUIConfigManager;
    if( xUIConfSupplier.is())
    {
        xUIConfigManager.set(
            xUIConfSupplier->getUIConfigurationManager(),
            uno::UNO_QUERY_THROW );
    }

    try
    {
        if( xUIConfigManager.is())
        {
            xResult = xUIConfigManager->getSettings(
                "private:resource/menubar/menubar",
                false );
        }
    }
    catch( const uno::Exception& )
    {}

    if ( !xResult.is() )
    {
        // no internal document configuration, use the one from the module
        uno::Reference< frame::XModuleManager2 > xModuleMan = frame::ModuleManager::create(m_xContext);
        OUString aModuleIdent =
            xModuleMan->identify( uno::Reference< uno::XInterface >( m_xComponent, uno::UNO_QUERY ) );

        if ( !aModuleIdent.isEmpty() )
        {
            uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xModConfSupplier =
                    ui::theModuleUIConfigurationManagerSupplier::get(m_xContext);
            uno::Reference< css::ui::XUIConfigurationManager > xModUIConfMan(
                    xModConfSupplier->getUIConfigurationManager( aModuleIdent ),
                    uno::UNO_QUERY_THROW );
            xResult = xModUIConfMan->getSettings(
                    "private:resource/menubar/menubar",
                    false );
        }
    }

    if ( !xResult.is() )
        throw uno::RuntimeException();

    return xResult;
}


void DocumentHolder::FindConnectPoints(
        const uno::Reference< container::XIndexAccess >& xMenu,
        sal_Int32 nConnectPoints[2] )
{
    nConnectPoints[0] = -1;
    nConnectPoints[1] = -1;
    for ( sal_Int32 nInd = 0; nInd < xMenu->getCount(); nInd++ )
    {
        uno::Sequence< beans::PropertyValue > aProps;
        xMenu->getByIndex( nInd ) >>= aProps;
        OUString aCommand;
        for ( sal_Int32 nSeqInd = 0; nSeqInd < aProps.getLength(); nSeqInd++ )
            if ( aProps[nSeqInd].Name == "CommandURL" )
            {
                aProps[nSeqInd].Value >>= aCommand;
                break;
            }

        if ( aCommand.isEmpty() )
            throw uno::RuntimeException();

        if ( aCommand == ".uno:PickList" )
            nConnectPoints[0] = nInd;
        else if ( aCommand == ".uno:WindowList" )
            nConnectPoints[1] = nInd;
    }
}


uno::Reference< container::XIndexAccess > DocumentHolder::MergeMenusForInplace(
        const uno::Reference< container::XIndexAccess >& xContMenu,
        const uno::Reference< frame::XDispatchProvider >& xContDisp,
        const OUString& aContModuleName,
        const uno::Reference< container::XIndexAccess >& xOwnMenu,
        const uno::Reference< frame::XDispatchProvider >& xOwnDisp )
{
    // TODO/LATER: use dispatch providers on merge

    sal_Int32 nContPoints[2];
    sal_Int32 nOwnPoints[2];

    uno::Reference< lang::XSingleComponentFactory > xIndAccessFact( xContMenu, uno::UNO_QUERY_THROW );

    uno::Reference< container::XIndexContainer > xMergedMenu(
            xIndAccessFact->createInstanceWithContext(
                comphelper::getProcessComponentContext() ),
            uno::UNO_QUERY_THROW );

    FindConnectPoints( xContMenu, nContPoints );
    FindConnectPoints( xOwnMenu, nOwnPoints );

    for ( sal_Int32 nInd = 0; nInd < xOwnMenu->getCount(); nInd++ )
    {
        if ( nOwnPoints[0] == nInd )
        {
            if ( nContPoints[0] >= 0 && nContPoints[0] < xContMenu->getCount() )
            {
                InsertMenu_Impl( xMergedMenu, nInd, xContMenu, nContPoints[0], aContModuleName, xContDisp );
            }
        }
        else if ( nOwnPoints[1] == nInd )
        {
            if ( nContPoints[1] >= 0 && nContPoints[1] < xContMenu->getCount() )
            {
                InsertMenu_Impl( xMergedMenu, nInd, xContMenu, nContPoints[1], aContModuleName, xContDisp );
            }
        }
        else
            InsertMenu_Impl( xMergedMenu, nInd, xOwnMenu, nInd, OUString(), xOwnDisp );
    }

    return uno::Reference< container::XIndexAccess >( xMergedMenu, uno::UNO_QUERY_THROW );
}


bool DocumentHolder::MergeMenus_Impl( const uno::Reference< css::frame::XLayoutManager >& xOwnLM,
                                            const uno::Reference< css::frame::XLayoutManager >& xContLM,
                                            const uno::Reference< frame::XDispatchProvider >& xContDisp,
                                            const OUString& aContModuleName )
{
    bool bMenuMerged = false;
    try
    {
        uno::Reference< css::ui::XUIElementSettings > xUISettings(
            xContLM->getElement( "private:resource/menubar/menubar" ),
            uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xContMenu = xUISettings->getSettings( true );
        if ( !xContMenu.is() )
            throw uno::RuntimeException();

        uno::Reference< container::XIndexAccess > xOwnMenu = RetrieveOwnMenu_Impl();
        uno::Reference< frame::XDispatchProvider > xOwnDisp( m_xFrame, uno::UNO_QUERY_THROW );

        uno::Reference< container::XIndexAccess > xMergedMenu = MergeMenusForInplace( xContMenu, xContDisp, aContModuleName, xOwnMenu, xOwnDisp );
        uno::Reference< css::frame::XMenuBarMergingAcceptor > xMerge( xOwnLM,
                                                                                         uno::UNO_QUERY_THROW );
        bMenuMerged = xMerge->setMergedMenuBar( xMergedMenu );
    }
    catch( const uno::Exception& )
    {}

    return bMenuMerged;
}

bool DocumentHolder::ShowUI( const uno::Reference< css::frame::XLayoutManager >& xContainerLM,
                                 const uno::Reference< frame::XDispatchProvider >& xContainerDP,
                                 const OUString& aContModuleName )
{
    bool bResult = false;
    if ( xContainerLM.is() )
    {
        // the LM of the embedded frame and its current DockingAreaAcceptor
        uno::Reference< css::frame::XLayoutManager > xOwnLM;
        uno::Reference< css::ui::XDockingAreaAcceptor > xDocAreaAcc;

        try
        {
            uno::Reference< beans::XPropertySet > xPropSet( m_xFrame, uno::UNO_QUERY_THROW );
            xPropSet->getPropertyValue("LayoutManager") >>= xOwnLM;
            xDocAreaAcc = xContainerLM->getDockingAreaAcceptor();
        }
        catch( const uno::Exception& ){}

        if ( xOwnLM.is() && xDocAreaAcc.is() )
        {
            // make sure that lock state of LM is correct even if an exception is thrown in between
            bool bUnlock = false;
            bool bLock = false;
            try
            {
                // take over the control over the containers window
                // as long as the LM is invisible and locked an empty tool space will be used on resizing
                xOwnLM->setDockingAreaAcceptor( xDocAreaAcc );

                // try to merge menus; don't do anything else if it fails
                if ( MergeMenus_Impl( xOwnLM, xContainerLM, xContainerDP, aContModuleName ) )
                {
                    // make sure that the container LM does not control the size of the containers window anymore
                    // this must be done after merging menus as we won't get the container menu otherwise
                    xContainerLM->setDockingAreaAcceptor( uno::Reference < ui::XDockingAreaAcceptor >() );

                    bool bIsChart = false;
                    uno::Reference< lang::XServiceInfo> xServiceInfo(m_xComponent, uno::UNO_QUERY);
                    if (xServiceInfo.is() && xServiceInfo->supportsService("com.sun.star.chart2.ChartDocument"))
                        bIsChart = true;
                    // prevent further changes at this LM
                    // TODO: moggi: why is this necessary?
                    if (!bIsChart)
                    {
                        xContainerLM->setVisible( false );
                        xContainerLM->lock();
                    }
                    bUnlock = true;

                    // by unlocking the LM each layout change will now resize the containers window; pending layouts will be processed now
                    xOwnLM->setVisible( true );

                    uno::Reference< frame::XFramesSupplier > xSupp( m_xFrame->getCreator(), uno::UNO_QUERY );
                    if ( xSupp.is() )
                        xSupp->setActiveFrame( m_xFrame );

                    xOwnLM->unlock();
                    bLock = true;
                    bResult = true;

                    // TODO/LATER: The following action should be done only if the window is not hidden
                    // otherwise the activation must fail, unfortunately currently it is not possible
                    // to detect whether the window is hidden using UNO API
                    m_xOwnWindow->setFocus();
                }
            }
            catch( const uno::Exception& )
            {
                // activation failed; reestablish old state
                try
                {
                    uno::Reference< frame::XFramesSupplier > xSupp( m_xFrame->getCreator(), uno::UNO_QUERY );
                    if ( xSupp.is() )
                        xSupp->setActiveFrame( nullptr );

                    // remove control about containers window from own LM
                    if ( bLock )
                        xOwnLM->lock();
                    xOwnLM->setVisible( false );
                    xOwnLM->setDockingAreaAcceptor( uno::Reference< css::ui::XDockingAreaAcceptor >() );

                    // unmerge menu
                    uno::Reference< css::frame::XMenuBarMergingAcceptor > xMerge( xOwnLM, uno::UNO_QUERY_THROW );
                    xMerge->removeMergedMenuBar();
                }
                catch( const uno::Exception& ) {}

                try
                {
                    // reestablish control of containers window
                    xContainerLM->setDockingAreaAcceptor( xDocAreaAcc );
                    xContainerLM->setVisible( true );
                    if ( bUnlock )
                        xContainerLM->unlock();
                }
                catch( const uno::Exception& ) {}
            }
        }
    }

    return bResult;
}


bool DocumentHolder::HideUI( const uno::Reference< css::frame::XLayoutManager >& xContainerLM )
{
    bool bResult = false;

    if ( xContainerLM.is() )
    {
        uno::Reference< css::frame::XLayoutManager > xOwnLM;

        try {
            uno::Reference< beans::XPropertySet > xPropSet( m_xFrame, uno::UNO_QUERY_THROW );
            xPropSet->getPropertyValue("LayoutManager") >>= xOwnLM;
        } catch( const uno::Exception& )
        {}

        if ( xOwnLM.is() )
        {
            try {
                uno::Reference< frame::XFramesSupplier > xSupp( m_xFrame->getCreator(), uno::UNO_QUERY );
                if ( xSupp.is() )
                    xSupp->setActiveFrame( nullptr );

                uno::Reference< css::ui::XDockingAreaAcceptor > xDocAreaAcc = xOwnLM->getDockingAreaAcceptor();

                xOwnLM->setDockingAreaAcceptor( uno::Reference < ui::XDockingAreaAcceptor >() );
                xOwnLM->lock();
                xOwnLM->setVisible( false );

                uno::Reference< css::frame::XMenuBarMergingAcceptor > xMerge( xOwnLM, uno::UNO_QUERY_THROW );
                xMerge->removeMergedMenuBar();

                xContainerLM->setDockingAreaAcceptor( xDocAreaAcc );
                xContainerLM->setVisible( true );
                xContainerLM->unlock();

                xContainerLM->doLayout();
                bResult = true;
            }
            catch( const uno::Exception& )
            {
                SetFrameLMVisibility( m_xFrame, true );
            }
        }
    }

    return bResult;
}


uno::Reference< frame::XFrame > const & DocumentHolder::GetDocFrame()
{
    // the frame for outplace activation
    if ( !m_xFrame.is() )
    {
        uno::Reference< lang::XSingleServiceFactory > xFrameFact = frame::TaskCreator::create(m_xContext);

        m_xFrame.set(xFrameFact->createInstanceWithArguments( m_aOutplaceFrameProps ), uno::UNO_QUERY_THROW);

        uno::Reference< frame::XDispatchProviderInterception > xInterception( m_xFrame, uno::UNO_QUERY );
        if ( xInterception.is() )
        {
            if ( m_xInterceptor.is() )
            {
                m_xInterceptor->DisconnectDocHolder();
                m_xInterceptor.clear();
            }

            m_xInterceptor = new Interceptor( this );

            xInterception->registerDispatchProviderInterceptor( m_xInterceptor.get() );

            // register interceptor from outside
            if ( m_xOutplaceInterceptor.is() )
                xInterception->registerDispatchProviderInterceptor( m_xOutplaceInterceptor );
        }

        uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xFrame, uno::UNO_QUERY );
        if ( xCloseBroadcaster.is() )
            xCloseBroadcaster->addCloseListener( static_cast<util::XCloseListener*>(this) );
    }

    if ( m_xComponent.is() )
    {
        uno::Reference< css::frame::XLayoutManager > xOwnLM;
        try {
            uno::Reference< beans::XPropertySet > xPropSet( m_xFrame, uno::UNO_QUERY_THROW );
            xPropSet->getPropertyValue("LayoutManager") >>= xOwnLM;
        } catch( const uno::Exception& )
        {}

        if ( xOwnLM.is() )
            xOwnLM->lock();

        // TODO/LATER: get it for the real aspect
        awt::Size aSize;
        GetExtent( embed::Aspects::MSOLE_CONTENT, &aSize );
        LoadDocToFrame(false);

        if ( xOwnLM.is() )
        {
            xOwnLM->unlock();
            xOwnLM->lock();
        }

        SetExtent( embed::Aspects::MSOLE_CONTENT, aSize );

        if ( xOwnLM.is() )
            xOwnLM->unlock();
    }

    try
    {
        uno::Reference< awt::XWindow > xHWindow = m_xFrame->getContainerWindow();

        if( xHWindow.is() )
        {
            sal_Int32 nDisplay = Application::GetDisplayBuiltInScreen();

            Rectangle aWorkRect = Application::GetScreenPosSizePixel( nDisplay );
            awt::Rectangle aWindowRect = xHWindow->getPosSize();

            if (( aWindowRect.Width < aWorkRect.GetWidth()) && ( aWindowRect.Height < aWorkRect.GetHeight() ))
            {
                int OffsetX = ( aWorkRect.GetWidth() - aWindowRect.Width ) / 2 + aWorkRect.Left();
                int OffsetY = ( aWorkRect.GetHeight() - aWindowRect.Height ) /2 + aWorkRect.Top();
                xHWindow->setPosSize( OffsetX, OffsetY, aWindowRect.Width, aWindowRect.Height, awt::PosSize::POS );
            }
            else
            {
                xHWindow->setPosSize( aWorkRect.Left(), aWorkRect.Top(), aWorkRect.GetWidth(), aWorkRect.GetHeight(), awt::PosSize::POSSIZE );
            }

            xHWindow->setVisible( true );
        }
    }
    catch ( const uno::Exception& )
    {
    }

    return m_xFrame;
}


void DocumentHolder::SetComponent( const uno::Reference< util::XCloseable >& xDoc, bool bReadOnly )
{
    if ( m_xComponent.is() )
    {
        // May be should be improved
        try {
            CloseDocument( true, false );
        } catch( const uno::Exception& )
        {}
    }

    m_xComponent = xDoc;

    m_bReadOnly = bReadOnly;
    m_bAllowClosing = false;

    uno::Reference< util::XCloseBroadcaster > xBroadcaster( m_xComponent, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
        xBroadcaster->addCloseListener( static_cast<util::XCloseListener*>(this) );

    uno::Reference< document::XEventBroadcaster > xEventBroadcaster( m_xComponent, uno::UNO_QUERY );
    if ( xEventBroadcaster.is() )
        xEventBroadcaster->addEventListener( static_cast<document::XEventListener*>(this) );
    else
    {
        // the object does not support document::XEventBroadcaster interface
        // use the workaround, register for modified events
        uno::Reference< util::XModifyBroadcaster > xModifyBroadcaster( m_xComponent, uno::UNO_QUERY );
        if ( xModifyBroadcaster.is() )
            xModifyBroadcaster->addModifyListener( static_cast<util::XModifyListener*>(this) );
    }

    if ( m_xFrame.is() )
        LoadDocToFrame(false);
}


bool DocumentHolder::LoadDocToFrame( bool bInPlace )
{
    if ( m_xFrame.is() && m_xComponent.is() )
    {
        uno::Reference < frame::XModel > xDoc( m_xComponent, uno::UNO_QUERY );
        if ( xDoc.is() )
        {
            // load new document in to the frame
            uno::Reference< frame::XComponentLoader > xComponentLoader( m_xFrame, uno::UNO_QUERY_THROW );

            ::comphelper::NamedValueCollection aArgs;
            aArgs.put( "Model", m_xComponent );
            aArgs.put( "ReadOnly", m_bReadOnly );
            if ( bInPlace )
                aArgs.put( "PluginMode", sal_Int16(1) );
            OUString sUrl;
            uno::Reference< lang::XServiceInfo> xServiceInfo(xDoc,uno::UNO_QUERY);
            if (    xServiceInfo.is()
                &&  xServiceInfo->supportsService("com.sun.star.report.ReportDefinition") )
            {
                sUrl = ".component:DB/ReportDesign";
            }
            else if( xServiceInfo.is()
                &&   xServiceInfo->supportsService("com.sun.star.chart2.ChartDocument"))
                sUrl = "private:factory/schart";
            else
                sUrl = "private:object";

            xComponentLoader->loadComponentFromURL( sUrl,
                                                        "_self",
                                                        0,
                                                        aArgs.getPropertyValues() );

            return true;
        }
        else
        {
            uno::Reference < frame::XSynchronousFrameLoader > xLoader( m_xComponent, uno::UNO_QUERY );
            if ( xLoader.is() )
                return xLoader->load( uno::Sequence < beans::PropertyValue >(), m_xFrame );
            else
                return false;
        }
    }

    return true;
}


void DocumentHolder::Show()
{
    if( m_xFrame.is() )
    {
        m_xFrame->activate();
        uno::Reference<awt::XTopWindow> xTopWindow( m_xFrame->getContainerWindow(), uno::UNO_QUERY );
        if( xTopWindow.is() )
            xTopWindow->toFront();
    }
    else
        GetDocFrame();
}


bool DocumentHolder::SetExtent( sal_Int64 nAspect, const awt::Size& aSize )
{
    uno::Reference< embed::XVisualObject > xDocVis( m_xComponent, uno::UNO_QUERY );
    if ( xDocVis.is() )
    {
        try
        {
            xDocVis->setVisualAreaSize( nAspect, aSize );
            return true;
        }
        catch( const uno::Exception& )
        {
            // TODO: Error handling
        }
    }

    return false;
}


bool DocumentHolder::GetExtent( sal_Int64 nAspect, awt::Size *pSize )
{
    uno::Reference< embed::XVisualObject > xDocVis( m_xComponent, uno::UNO_QUERY );
    if ( pSize && xDocVis.is() )
    {
        try
        {
            *pSize = xDocVis->getVisualAreaSize( nAspect );
            return true;
        }
        catch( const uno::Exception& )
        {
            // TODO: Error handling
        }
    }

    return false;
}


sal_Int32 DocumentHolder::GetMapUnit( sal_Int64 nAspect )
{
    uno::Reference< embed::XVisualObject > xDocVis( m_xComponent, uno::UNO_QUERY );
    if ( xDocVis.is() )
    {
        try
        {
            return xDocVis->getMapUnit( nAspect );
        }
        catch( const uno::Exception& )
        {
            // TODO: Error handling
        }
    }

    return 0;
}


awt::Rectangle DocumentHolder::CalculateBorderedArea( const awt::Rectangle& aRect )
{
    return awt::Rectangle( aRect.X + m_aBorderWidths.Left + HATCH_BORDER_WIDTH,
                             aRect.Y + m_aBorderWidths.Top + HATCH_BORDER_WIDTH,
                             aRect.Width - m_aBorderWidths.Left - m_aBorderWidths.Right - 2*HATCH_BORDER_WIDTH,
                             aRect.Height - m_aBorderWidths.Top - m_aBorderWidths.Bottom - 2*HATCH_BORDER_WIDTH );
}


awt::Rectangle DocumentHolder::AddBorderToArea( const awt::Rectangle& aRect )
{
    return awt::Rectangle( aRect.X - m_aBorderWidths.Left - HATCH_BORDER_WIDTH,
                             aRect.Y - m_aBorderWidths.Top - HATCH_BORDER_WIDTH,
                             aRect.Width + m_aBorderWidths.Left + m_aBorderWidths.Right + 2*HATCH_BORDER_WIDTH,
                             aRect.Height + m_aBorderWidths.Top + m_aBorderWidths.Bottom + 2*HATCH_BORDER_WIDTH );
}


void SAL_CALL DocumentHolder::disposing( const css::lang::EventObject& aSource )
{
    if ( m_xComponent.is() && m_xComponent == aSource.Source )
    {
        m_xComponent = nullptr;
        if ( m_bWaitForClose )
        {
            m_bWaitForClose = false;
            FreeOffice();
        }
    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
    {
        m_xHatchWindow.clear();
        m_xOwnWindow.clear();
        m_xFrame.clear();
    }
}


void SAL_CALL DocumentHolder::queryClosing( const lang::EventObject& aSource, sal_Bool /*bGetsOwnership*/ )
{
    if ( m_xComponent.is() && m_xComponent == aSource.Source && !m_bAllowClosing )
        throw util::CloseVetoException("To close an embedded document, close the document holder (document definition), not the document itself.", static_cast< ::cppu::OWeakObject*>(this));
}


void SAL_CALL DocumentHolder::notifyClosing( const lang::EventObject& aSource )
{
    if ( m_xComponent.is() && m_xComponent == aSource.Source )
    {
        m_xComponent = nullptr;
        if ( m_bWaitForClose )
        {
            m_bWaitForClose = false;
            FreeOffice();
        }
    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
    {
        m_xHatchWindow.clear();
        m_xOwnWindow.clear();
        m_xFrame.clear();
    }
}


void SAL_CALL DocumentHolder::queryTermination( const lang::EventObject& )
{
    if ( m_bWaitForClose )
        throw frame::TerminationVetoException();
}


void SAL_CALL DocumentHolder::notifyTermination( const lang::EventObject& aSource )
{
    OSL_ENSURE( !m_xComponent.is(), "Just a disaster..." );

    uno::Reference< frame::XDesktop > xDesktop( aSource.Source, uno::UNO_QUERY );
    m_bDesktopTerminated = true;
    if ( xDesktop.is() )
        xDesktop->removeTerminateListener( static_cast<frame::XTerminateListener*>(this) );
}


void SAL_CALL DocumentHolder::modified( const lang::EventObject& aEvent )
{
    // if the component does not support document::XEventBroadcaster
    // the modify notifications are used as workaround, but only for running state
    if( aEvent.Source == m_xComponent && m_pEmbedObj && m_pEmbedObj->getCurrentState() == embed::EmbedStates::RUNNING )
        m_pEmbedObj->PostEvent_Impl( "OnVisAreaChanged" );
}


void SAL_CALL DocumentHolder::notifyEvent( const document::EventObject& Event )
{
    if( m_pEmbedObj && Event.Source == m_xComponent )
    {
        // for now the ignored events are not forwarded, but sent by the object itself
        if ( !Event.EventName.startsWith( "OnSave" )
          && !Event.EventName.startsWith( "OnSaveDone" )
          && !Event.EventName.startsWith( "OnSaveAs" )
          && !Event.EventName.startsWith( "OnSaveAsDone" )
          && !( Event.EventName.startsWith( "OnVisAreaChanged" ) && m_nNoResizeReact ) )
            m_pEmbedObj->PostEvent_Impl( Event.EventName );
    }
}


void SAL_CALL DocumentHolder::borderWidthsChanged( const uno::Reference< uno::XInterface >& aObject,
                                                    const frame::BorderWidths& aNewSize )
{
    // TODO: may require mutex introduction ???
    if ( m_pEmbedObj && m_xFrame.is() && aObject == m_xFrame->getController() )
    {
        if ( m_aBorderWidths.Left != aNewSize.Left
          || m_aBorderWidths.Right != aNewSize.Right
          || m_aBorderWidths.Top != aNewSize.Top
          || m_aBorderWidths.Bottom != aNewSize.Bottom )
        {
            m_aBorderWidths = aNewSize;
            if ( !m_nNoBorderResizeReact )
                PlaceFrame( m_aObjRect );
        }
    }
}


void SAL_CALL DocumentHolder::requestPositioning( const awt::Rectangle& aRect )
{
    // TODO: may require mutex introduction ???
    if ( m_pEmbedObj )
    {
        // borders should not be counted
        awt::Rectangle aObjRect = CalculateBorderedArea( aRect );
        IntCounterGuard aGuard( m_nNoResizeReact );
        m_pEmbedObj->requestPositioning( aObjRect );
    }
}


awt::Rectangle SAL_CALL DocumentHolder::calcAdjustedRectangle( const awt::Rectangle& aRect )
{
    // Solar mutex should be locked already since this is a call from HatchWindow with focus
    awt::Rectangle aResult( aRect );

    if ( m_xFrame.is() )
    {
        // borders should not be counted
        uno::Reference< frame::XControllerBorder > xControllerBorder( m_xFrame->getController(), uno::UNO_QUERY );
        if ( xControllerBorder.is() )
        {
            awt::Rectangle aObjRect = CalculateBorderedArea( aRect );
            aObjRect = xControllerBorder->queryBorderedArea( aObjRect );
            aResult = AddBorderToArea( aObjRect );
        }
    }

    awt::Rectangle aMinRectangle = AddBorderToArea( awt::Rectangle() );
    if ( aResult.Width < aMinRectangle.Width + 2 )
        aResult.Width = aMinRectangle.Width + 2;
    if ( aResult.Height < aMinRectangle.Height + 2 )
        aResult.Height = aMinRectangle.Height + 2;

    return aResult;
}

void SAL_CALL DocumentHolder::activated(  )
{
    if ( (m_pEmbedObj->getStatus(embed::Aspects::MSOLE_CONTENT)&embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE) ||
        svt::EmbeddedObjectRef::IsGLChart(m_pEmbedObj) )
    {
        if ( m_pEmbedObj->getCurrentState() != embed::EmbedStates::UI_ACTIVE &&
        !(m_pEmbedObj->getStatus(embed::Aspects::MSOLE_CONTENT)&embed::EmbedMisc::MS_EMBED_NOUIACTIVATE) )
        {
            try
            {
                m_pEmbedObj->changeState( embed::EmbedStates::UI_ACTIVE );
            }
            catch ( const css::embed::StateChangeInProgressException& )
            {
                // must catch this exception because focus is grabbed while UI activation in doVerb()
            }
            catch ( const css::uno::Exception& )
            {
                // no outgoing exceptions specified here
            }
        }
        else
        {
            uno::Reference< frame::XFramesSupplier > xSupp( m_xFrame->getCreator(), uno::UNO_QUERY );
            if ( xSupp.is() )
                xSupp->setActiveFrame( m_xFrame );
        }
    }
}

void DocumentHolder::ResizeHatchWindow()
{
    awt::Rectangle aHatchRect = AddBorderToArea( m_aObjRect );
    ResizeWindows_Impl( aHatchRect );
    uno::Reference< embed::XHatchWindow > xHatchWindow( m_xHatchWindow, uno::UNO_QUERY );
    xHatchWindow->setHatchBorderSize( awt::Size( HATCH_BORDER_WIDTH, HATCH_BORDER_WIDTH ) );
}

void SAL_CALL DocumentHolder::deactivated(  )
{
    // deactivation is too unspecific to be useful; usually we only trigger code from activation
    // so UIDeactivation is actively triggered by the container
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
