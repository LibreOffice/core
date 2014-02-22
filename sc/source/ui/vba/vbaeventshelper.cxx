/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "vbaeventshelper.hxx"

#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/frame/XBorderResizeListener.hpp>
#include <com/sun/star/frame/XControllerBorder.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>

#include <cppuhelper/implbase4.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/eventcfg.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include "cellsuno.hxx"
#include "convuno.hxx"
#include "vbaapplication.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::script::vba::VBAEventId;
using namespace ::ooo::vba;




namespace {

/** Extracts a sheet index from the specified element of the passed sequence.
    The element may be an integer, a Calc range or ranges object, or a VBA Range object. */
SCTAB lclGetTabFromArgs( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) throw (lang::IllegalArgumentException)
{
    VbaEventsHelperBase::checkArgument( rArgs, nIndex );

    
    sal_Int32 nTab = -1;
    if( rArgs[ nIndex ] >>= nTab )
    {
        if( (nTab < 0) || (nTab > MAXTAB) )
            throw lang::IllegalArgumentException();
        return static_cast< SCTAB >( nTab );
    }

    
    uno::Reference< excel::XRange > xVbaRange = getXSomethingFromArgs< excel::XRange >( rArgs, nIndex );
    if( xVbaRange.is() )
    {
        uno::Reference< XHelperInterface > xVbaHelper( xVbaRange, uno::UNO_QUERY_THROW );
        
        uno::Reference< excel::XWorksheet > xVbaSheet( xVbaHelper->getParent(), uno::UNO_QUERY_THROW );
        
        return static_cast< SCTAB >( xVbaSheet->getIndex() - 1 );
    }

    
    uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable = getXSomethingFromArgs< sheet::XCellRangeAddressable >( rArgs, nIndex );
    if( xCellRangeAddressable.is() )
        return xCellRangeAddressable->getRangeAddress().Sheet;

    
    uno::Reference< sheet::XSheetCellRangeContainer > xRanges = getXSomethingFromArgs< sheet::XSheetCellRangeContainer >( rArgs, nIndex );
    if( xRanges.is() )
    {
        uno::Sequence< table::CellRangeAddress > aRangeAddresses = xRanges->getRangeAddresses();
        if( aRangeAddresses.getLength() > 0 )
            return aRangeAddresses[ 0 ].Sheet;
    }

    throw lang::IllegalArgumentException();
}

/** Returns the AWT container window of the passed controller. */
uno::Reference< awt::XWindow > lclGetWindowForController( const uno::Reference< frame::XController >& rxController )
{
    if( rxController.is() ) try
    {
        uno::Reference< frame::XFrame > xFrame( rxController->getFrame(), uno::UNO_SET_THROW );
        return xFrame->getContainerWindow();
    }
    catch( uno::Exception& )
    {
    }
    return 0;
}

} 



typedef ::cppu::WeakImplHelper4< awt::XTopWindowListener, awt::XWindowListener, frame::XBorderResizeListener, util::XChangesListener > ScVbaEventListener_BASE;


class ScVbaEventListener : public ScVbaEventListener_BASE
{
public :
    ScVbaEventListener( ScVbaEventsHelper& rVbaEvents, const uno::Reference< frame::XModel >& rxModel, ScDocShell* pDocShell );
    virtual ~ScVbaEventListener();

    /** Starts listening to the passed document controller. */
    void startControllerListening( const uno::Reference< frame::XController >& rxController );
    /** Stops listening to the passed document controller. */
    void stopControllerListening( const uno::Reference< frame::XController >& rxController );

    
    virtual void SAL_CALL windowOpened( const lang::EventObject& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowClosing( const lang::EventObject& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowClosed( const lang::EventObject& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowMinimized( const lang::EventObject& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowNormalized( const lang::EventObject& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowActivated( const lang::EventObject& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowDeactivated( const lang::EventObject& rEvent ) throw (uno::RuntimeException);

    
    virtual void SAL_CALL windowResized( const awt::WindowEvent& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const awt::WindowEvent& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowShown( const lang::EventObject& rEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const lang::EventObject& rEvent ) throw (uno::RuntimeException);

    
    virtual void SAL_CALL borderWidthsChanged( const uno::Reference< uno::XInterface >& rSource, const frame::BorderWidths& aNewSize ) throw (uno::RuntimeException);

    
    virtual void SAL_CALL changesOccurred( const util::ChangesEvent& rEvent ) throw (uno::RuntimeException);

    
    virtual void SAL_CALL disposing( const lang::EventObject& rEvent ) throw (uno::RuntimeException);

private:
    /** Starts listening to the document model. */
    void startModelListening();
    /** Stops listening to the document model. */
    void stopModelListening();

    /** Returns the controller for the passed VCL window. */
    uno::Reference< frame::XController > getControllerForWindow( Window* pWindow ) const;

    /** Calls the Workbook_Window[Activate|Deactivate] event handler. */
    void processWindowActivateEvent( Window* pWindow, bool bActivate );
    /** Posts a Workbook_WindowResize user event. */
    void postWindowResizeEvent( Window* pWindow );
    /** Callback link for Application::PostUserEvent(). */
    DECL_LINK( processWindowResizeEvent, Window* );

private:
    typedef ::std::map< Window*, uno::Reference< frame::XController > > WindowControllerMap;

    ::osl::Mutex        maMutex;
    ScVbaEventsHelper&  mrVbaEvents;
    uno::Reference< frame::XModel > mxModel;
    ScDocShell*         mpDocShell;
    WindowControllerMap maControllers;          
    Window*             mpActiveWindow;         
    bool                mbWindowResized;        
    bool                mbBorderChanged;        
    bool                mbDisposed;
};



ScVbaEventListener::ScVbaEventListener( ScVbaEventsHelper& rVbaEvents, const uno::Reference< frame::XModel >& rxModel, ScDocShell* pDocShell ) :
    mrVbaEvents( rVbaEvents ),
    mxModel( rxModel ),
    mpDocShell( pDocShell ),
    mpActiveWindow( 0 ),
    mbWindowResized( false ),
    mbBorderChanged( false ),
    mbDisposed( !rxModel.is() )
{
    if( !mxModel.is() )
        return;

    startModelListening();
    try
    {
        uno::Reference< frame::XController > xController( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
        startControllerListening( xController );
    }
    catch( uno::Exception& )
    {
    }
}

ScVbaEventListener::~ScVbaEventListener()
{
}

void ScVbaEventListener::startControllerListening( const uno::Reference< frame::XController >& rxController )
{
    ::osl::MutexGuard aGuard( maMutex );

    uno::Reference< awt::XWindow > xWindow = lclGetWindowForController( rxController );
    if( xWindow.is() )
        try { xWindow->addWindowListener( this ); } catch( uno::Exception& ) {}

    uno::Reference< awt::XTopWindow > xTopWindow( xWindow, uno::UNO_QUERY );
    if( xTopWindow.is() )
        try { xTopWindow->addTopWindowListener( this ); } catch( uno::Exception& ) {}

    uno::Reference< frame::XControllerBorder > xControllerBorder( rxController, uno::UNO_QUERY );
    if( xControllerBorder.is() )
        try { xControllerBorder->addBorderResizeListener( this ); } catch( uno::Exception& ) {}

    if( Window* pWindow = VCLUnoHelper::GetWindow( xWindow ) )
        maControllers[ pWindow ] = rxController;
}

void ScVbaEventListener::stopControllerListening( const uno::Reference< frame::XController >& rxController )
{
    ::osl::MutexGuard aGuard( maMutex );

    uno::Reference< awt::XWindow > xWindow = lclGetWindowForController( rxController );
    if( xWindow.is() )
        try { xWindow->removeWindowListener( this ); } catch( uno::Exception& ) {}

    uno::Reference< awt::XTopWindow > xTopWindow( xWindow, uno::UNO_QUERY );
    if( xTopWindow.is() )
        try { xTopWindow->removeTopWindowListener( this ); } catch( uno::Exception& ) {}

    uno::Reference< frame::XControllerBorder > xControllerBorder( rxController, uno::UNO_QUERY );
    if( xControllerBorder.is() )
        try { xControllerBorder->removeBorderResizeListener( this ); } catch( uno::Exception& ) {}

    if( Window* pWindow = VCLUnoHelper::GetWindow( xWindow ) )
    {
        maControllers.erase( pWindow );
        if( pWindow == mpActiveWindow )
            mpActiveWindow = 0;
    }
}

void SAL_CALL ScVbaEventListener::windowOpened( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
}

void SAL_CALL ScVbaEventListener::windowClosing( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
}

void SAL_CALL ScVbaEventListener::windowClosed( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
}

void SAL_CALL ScVbaEventListener::windowMinimized( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
}

void SAL_CALL ScVbaEventListener::windowNormalized( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
}

void SAL_CALL ScVbaEventListener::windowActivated( const lang::EventObject& rEvent ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    if( !mbDisposed )
    {
        uno::Reference< awt::XWindow > xWindow( rEvent.Source, uno::UNO_QUERY );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        OSL_TRACE( "ScVbaEventListener::windowActivated - pWindow = 0x%p, mpActiveWindow = 0x%p", pWindow, mpActiveWindow );
        
        if( pWindow && (pWindow != mpActiveWindow) )
        {
            
            if( mpActiveWindow )
                processWindowActivateEvent( mpActiveWindow, false );
            
            processWindowActivateEvent( pWindow, true );
            mpActiveWindow = pWindow;
        }
    }
}

void SAL_CALL ScVbaEventListener::windowDeactivated( const lang::EventObject& rEvent ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    if( !mbDisposed )
    {
        uno::Reference< awt::XWindow > xWindow( rEvent.Source, uno::UNO_QUERY );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        OSL_TRACE( "ScVbaEventListener::windowDeactivated - pWindow = 0x%p, mpActiveWindow = 0x%p", pWindow, mpActiveWindow );
        
        if( pWindow && (pWindow == mpActiveWindow) )
            processWindowActivateEvent( pWindow, false );
        
        mpActiveWindow = 0;
    }
}

void SAL_CALL ScVbaEventListener::windowResized( const awt::WindowEvent& rEvent ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    mbWindowResized = true;
    if( !mbDisposed && mbBorderChanged )
    {
        uno::Reference< awt::XWindow > xWindow( rEvent.Source, uno::UNO_QUERY );
        postWindowResizeEvent( VCLUnoHelper::GetWindow( xWindow ) );
    }
}

void SAL_CALL ScVbaEventListener::windowMoved( const awt::WindowEvent& /*rEvent*/ ) throw (uno::RuntimeException)
{
}

void SAL_CALL ScVbaEventListener::windowShown( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
}

void SAL_CALL ScVbaEventListener::windowHidden( const lang::EventObject& /*rEvent*/ ) throw (uno::RuntimeException)
{
}

void SAL_CALL ScVbaEventListener::borderWidthsChanged( const uno::Reference< uno::XInterface >& rSource, const frame::BorderWidths& /*aNewSize*/ ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    mbBorderChanged = true;
    if( !mbDisposed && mbWindowResized )
    {
        uno::Reference< frame::XController > xController( rSource, uno::UNO_QUERY );
        uno::Reference< awt::XWindow > xWindow = lclGetWindowForController( xController );
        postWindowResizeEvent( VCLUnoHelper::GetWindow( xWindow ) );
    }
}

void SAL_CALL ScVbaEventListener::changesOccurred( const util::ChangesEvent& rEvent ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    sal_Int32 nCount = rEvent.Changes.getLength();
    if( mbDisposed || !mpDocShell || (nCount == 0) )
        return;

    util::ElementChange aChange = rEvent.Changes[ 0 ];
    OUString sOperation;
    aChange.Accessor >>= sOperation;
    if( !sOperation.equalsIgnoreAsciiCase("cell-change") )
        return;

    if( nCount == 1 )
    {
        uno::Reference< table::XCellRange > xRangeObj;
        aChange.ReplacedElement >>= xRangeObj;
        if( xRangeObj.is() )
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[0] <<= xRangeObj;
            mrVbaEvents.processVbaEventNoThrow( WORKSHEET_CHANGE, aArgs );
        }
        return;
    }

    ScRangeList aRangeList;
    for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        aChange = rEvent.Changes[ nIndex ];
        aChange.Accessor >>= sOperation;
        uno::Reference< table::XCellRange > xRangeObj;
        aChange.ReplacedElement >>= xRangeObj;
        if( xRangeObj.is() && sOperation.equalsIgnoreAsciiCase("cell-change") )
        {
            uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable( xRangeObj, uno::UNO_QUERY );
            if( xCellRangeAddressable.is() )
            {
                ScRange aRange;
                ScUnoConversion::FillScRange( aRange, xCellRangeAddressable->getRangeAddress() );
                aRangeList.Append( aRange );
            }
        }
    }

    if (!aRangeList.empty())
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( mpDocShell, aRangeList ) );
        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<= xRanges;
        mrVbaEvents.processVbaEventNoThrow( WORKSHEET_CHANGE, aArgs );
    }
}

void SAL_CALL ScVbaEventListener::disposing( const lang::EventObject& rEvent ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    uno::Reference< frame::XModel > xModel( rEvent.Source, uno::UNO_QUERY );
    if( xModel.is() )
    {
        OSL_ENSURE( xModel.get() == mxModel.get(), "ScVbaEventListener::disposing - disposing from unknown model" );
        stopModelListening();
        mbDisposed = true;
        return;
    }

    uno::Reference< frame::XController > xController( rEvent.Source, uno::UNO_QUERY );
    if( xController.is() )
    {
        stopControllerListening( xController );
        return;
    }
}



void ScVbaEventListener::startModelListening()
{
    try
    {
        uno::Reference< util::XChangesNotifier > xChangesNotifier( mxModel, uno::UNO_QUERY_THROW );
        xChangesNotifier->addChangesListener( this );
    }
    catch( uno::Exception& )
    {
    }
}

void ScVbaEventListener::stopModelListening()
{
    try
    {
        uno::Reference< util::XChangesNotifier > xChangesNotifier( mxModel, uno::UNO_QUERY_THROW );
        xChangesNotifier->removeChangesListener( this );
    }
    catch( uno::Exception& )
    {
    }
}

uno::Reference< frame::XController > ScVbaEventListener::getControllerForWindow( Window* pWindow ) const
{
    WindowControllerMap::const_iterator aIt = maControllers.find( pWindow );
    return (aIt == maControllers.end()) ? uno::Reference< frame::XController >() : aIt->second;
}

void ScVbaEventListener::processWindowActivateEvent( Window* pWindow, bool bActivate )
{
    uno::Reference< frame::XController > xController = getControllerForWindow( pWindow );
    if( xController.is() )
    {
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[ 0 ] <<= xController;
        mrVbaEvents.processVbaEventNoThrow( bActivate ? WORKBOOK_WINDOWACTIVATE : WORKBOOK_WINDOWDEACTIVATE, aArgs );
    }
}

void ScVbaEventListener::postWindowResizeEvent( Window* pWindow )
{
    
    if( pWindow && (maControllers.count( pWindow ) > 0) )
    {
        mbWindowResized = mbBorderChanged = false;
        acquire();  
        Application::PostUserEvent( LINK( this, ScVbaEventListener, processWindowResizeEvent ), pWindow );
    }
}

IMPL_LINK( ScVbaEventListener, processWindowResizeEvent, Window*, EMPTYARG pWindow )
{
    ::osl::MutexGuard aGuard( maMutex );

    /*  Check that the passed window is still alive (it must be registered in
        maControllers). While closing a document, postWindowResizeEvent() may
        be called on the last window which posts a user event via
        Application::PostUserEvent to call this event handler. VCL will trigger
        the handler some time later. Sometimes, the window gets deleted before.
        This is handled via the disposing() function which removes the window
        pointer from the member maControllers. Thus, checking whether
        maControllers contains pWindow ensures that the window is still alive. */
    if( !mbDisposed && pWindow && (maControllers.count( pWindow ) > 0) )
    {
        
        Window::PointerState aPointerState = pWindow->GetPointerState();
        if( (aPointerState.mnState & (MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT)) == 0 )
        {
            uno::Reference< frame::XController > xController = getControllerForWindow( pWindow );
            if( xController.is() )
            {
                uno::Sequence< uno::Any > aArgs( 1 );
                aArgs[ 0 ] <<= xController;
                
                mrVbaEvents.processVbaEventNoThrow( WORKBOOK_WINDOWRESIZE, aArgs );
            }
        }
    }
    release();
    return 0;
}



ScVbaEventsHelper::ScVbaEventsHelper( const uno::Sequence< uno::Any >& rArgs, const uno::Reference< uno::XComponentContext >& xContext ) :
    VbaEventsHelperBase( rArgs, xContext ),
    mbOpened( false )
{
    mpDocShell = dynamic_cast< ScDocShell* >( mpShell ); 
    mpDoc = mpDocShell ? mpDocShell->GetDocument() : 0;

    if( !mxModel.is() || !mpDocShell || !mpDoc )
        return;

#define REGISTER_EVENT( eventid, moduletype, classname, eventname, cancelindex, worksheet ) \
    registerEventHandler( eventid, moduletype, classname "_" eventname, cancelindex, uno::Any( worksheet ) )
#define REGISTER_AUTO_EVENT( eventid, eventname ) \
    REGISTER_EVENT( AUTO_##eventid, script::ModuleType::NORMAL, "Auto", eventname, -1, false )
#define REGISTER_WORKBOOK_EVENT( eventid, eventname, cancelindex ) \
    REGISTER_EVENT( WORKBOOK_##eventid, script::ModuleType::DOCUMENT, "Workbook", eventname, cancelindex, false )
#define REGISTER_WORKSHEET_EVENT( eventid, eventname, cancelindex ) \
    REGISTER_EVENT( WORKSHEET_##eventid, script::ModuleType::DOCUMENT, "Worksheet", eventname, cancelindex, true ); \
    REGISTER_EVENT( (USERDEFINED_START + WORKSHEET_##eventid), script::ModuleType::DOCUMENT, "Workbook", "Sheet" eventname, (((cancelindex) >= 0) ? ((cancelindex) + 1) : -1), false )

    
    REGISTER_AUTO_EVENT( OPEN,  "Open" );
    REGISTER_AUTO_EVENT( CLOSE, "Close" );

    
    REGISTER_WORKBOOK_EVENT( ACTIVATE,            "Activate",           -1 );
    REGISTER_WORKBOOK_EVENT( DEACTIVATE,          "Deactivate",         -1 );
    REGISTER_WORKBOOK_EVENT( OPEN,                "Open",               -1 );
    REGISTER_WORKBOOK_EVENT( BEFORECLOSE,         "BeforeClose",        0 );
    REGISTER_WORKBOOK_EVENT( BEFOREPRINT,         "BeforePrint",        0 );
    REGISTER_WORKBOOK_EVENT( BEFORESAVE,          "BeforeSave",         1 );
    REGISTER_WORKBOOK_EVENT( AFTERSAVE,           "AfterSave",          -1 );
    REGISTER_WORKBOOK_EVENT( NEWSHEET,            "NewSheet",           -1 );
    REGISTER_WORKBOOK_EVENT( WINDOWACTIVATE,      "WindowActivate",     -1 );
    REGISTER_WORKBOOK_EVENT( WINDOWDEACTIVATE,    "WindowDeactivate",   -1 );
    REGISTER_WORKBOOK_EVENT( WINDOWRESIZE,        "WindowResize",       -1 );

    
    REGISTER_WORKSHEET_EVENT( ACTIVATE,           "Activate",           -1 );
    REGISTER_WORKSHEET_EVENT( DEACTIVATE,         "Deactivate",         -1 );
    REGISTER_WORKSHEET_EVENT( BEFOREDOUBLECLICK,  "BeforeDoubleClick",  1 );
    REGISTER_WORKSHEET_EVENT( BEFORERIGHTCLICK,   "BeforeRightClick",   1 );
    REGISTER_WORKSHEET_EVENT( CALCULATE,          "Calculate",          -1 );
    REGISTER_WORKSHEET_EVENT( CHANGE,             "Change",             -1 );
    REGISTER_WORKSHEET_EVENT( SELECTIONCHANGE,    "SelectionChange",    -1 );
    REGISTER_WORKSHEET_EVENT( FOLLOWHYPERLINK,    "FollowHyperlink",    -1 );

#undef REGISTER_WORKSHEET_EVENT
#undef REGISTER_WORKBOOK_EVENT
#undef REGISTER_AUTO_EVENT
#undef REGISTER_EVENT
}

ScVbaEventsHelper::~ScVbaEventsHelper()
{
}

void SAL_CALL ScVbaEventsHelper::notifyEvent( const css::document::EventObject& rEvent ) throw (css::uno::RuntimeException)
{
    static const uno::Sequence< uno::Any > saEmptyArgs;
    if( (rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_OPENDOC )) ||
        (rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_CREATEDOC )) )  
    {
        processVbaEventNoThrow( WORKBOOK_OPEN, saEmptyArgs );
    }
    else if( rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_ACTIVATEDOC ) )
    {
        processVbaEventNoThrow( WORKBOOK_ACTIVATE, saEmptyArgs );
    }
    else if( rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_DEACTIVATEDOC ) )
    {
        processVbaEventNoThrow( WORKBOOK_DEACTIVATE, saEmptyArgs );
    }
    else if( (rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_SAVEDOCDONE )) ||
             (rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_SAVEASDOCDONE )) ||
             (rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_SAVETODOCDONE )) )
    {
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[ 0 ] <<= true;
        processVbaEventNoThrow( WORKBOOK_AFTERSAVE, aArgs );
    }
    else if( (rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_SAVEDOCFAILED )) ||
             (rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_SAVEASDOCFAILED )) ||
             (rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_SAVETODOCFAILED )) )
    {
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[ 0 ] <<= false;
        processVbaEventNoThrow( WORKBOOK_AFTERSAVE, aArgs );
    }
    else if( rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_CLOSEDOC ) )
    {
        /*  Trigger the WORKBOOK_WINDOWDEACTIVATE and WORKBOOK_DEACTIVATE
            events and stop listening to the model (done in base class). */
        uno::Reference< frame::XController > xController( mxModel->getCurrentController() );
        if( xController.is() )
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= xController;
            processVbaEventNoThrow( WORKBOOK_WINDOWDEACTIVATE, aArgs );
        }
        processVbaEventNoThrow( WORKBOOK_DEACTIVATE, saEmptyArgs );
    }
    else if( rEvent.EventName == GlobalEventConfig::GetEventName( STR_EVENT_VIEWCREATED ) )
    {
        uno::Reference< frame::XController > xController( mxModel->getCurrentController() );
        if( mxListener.get() && xController.is() )
            mxListener->startControllerListening( xController );
    }
    VbaEventsHelperBase::notifyEvent( rEvent );
}



bool ScVbaEventsHelper::implPrepareEvent( EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo, const uno::Sequence< uno::Any >& rArgs ) throw (uno::RuntimeException)
{
    
    if( !mpShell || !mpDoc )
        throw uno::RuntimeException();

    /*  For document events: check if events are enabled via the
        Application.EnableEvents symbol (this is an Excel-only attribute).
        Check this again for every event, as the event handler may change the
        state of the EnableEvents symbol. Global events such as AUTO_OPEN and
        AUTO_CLOSE are always enabled. */
    bool bExecuteEvent = (rInfo.mnModuleType != script::ModuleType::DOCUMENT) || ScVbaApplication::getDocumentEventsEnabled();

    
    if( bExecuteEvent )
        bExecuteEvent = (rInfo.mnEventId == WORKBOOK_OPEN) ? !mbOpened : mbOpened;

    
    if( bExecuteEvent ) switch( rInfo.mnEventId )
    {
        case WORKBOOK_OPEN:
        {
            
            rEventQueue.push_back( WORKBOOK_ACTIVATE );
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= mxModel->getCurrentController();
            rEventQueue.push_back( EventQueueEntry( WORKBOOK_WINDOWACTIVATE, aArgs ) );
            rEventQueue.push_back( AUTO_OPEN );
            
            maOldSelection <<= mxModel->getCurrentSelection();
        }
        break;
        case WORKSHEET_SELECTIONCHANGE:
            
            bExecuteEvent = isSelectionChanged( rArgs, 0 );
        break;
    }

    if( bExecuteEvent )
    {
        
        bool bSheetEvent = false;
        if( (rInfo.maUserData >>= bSheetEvent) && bSheetEvent )
            rEventQueue.push_back( EventQueueEntry( rInfo.mnEventId + USERDEFINED_START, rArgs ) );
    }

    return bExecuteEvent;
}

uno::Sequence< uno::Any > ScVbaEventsHelper::implBuildArgumentList( const EventHandlerInfo& rInfo,
        const uno::Sequence< uno::Any >& rArgs ) throw (lang::IllegalArgumentException)
{
    
    bool bSheetEventAsBookEvent = rInfo.mnEventId > USERDEFINED_START;
    sal_Int32 nEventId = bSheetEventAsBookEvent ? (rInfo.mnEventId - USERDEFINED_START) : rInfo.mnEventId;

    uno::Sequence< uno::Any > aVbaArgs;
    switch( nEventId )
    {
        

        
        case WORKBOOK_ACTIVATE:
        case WORKBOOK_DEACTIVATE:
        case WORKBOOK_OPEN:
        break;
        
        case WORKBOOK_BEFORECLOSE:
        case WORKBOOK_BEFOREPRINT:
            aVbaArgs.realloc( 1 );
            
        break;
        
        case WORKBOOK_BEFORESAVE:
            aVbaArgs.realloc( 2 );
            checkArgumentType< bool >( rArgs, 0 );
            aVbaArgs[ 0 ] = rArgs[ 0 ];
            
        break;
        
        case WORKBOOK_AFTERSAVE:
            aVbaArgs.realloc( 1 );
            checkArgumentType< bool >( rArgs, 0 );
            aVbaArgs[ 0 ] = rArgs[ 0 ];
        break;
        
        case WORKBOOK_WINDOWACTIVATE:
        case WORKBOOK_WINDOWDEACTIVATE:
        case WORKBOOK_WINDOWRESIZE:
            aVbaArgs.realloc( 1 );
            aVbaArgs[ 0 ] = createWindow( rArgs, 0 );
        break;
        
        case WORKBOOK_NEWSHEET:
            aVbaArgs.realloc( 1 );
            aVbaArgs[ 0 ] = createWorksheet( rArgs, 0 );
        break;

        

        
        case WORKSHEET_ACTIVATE:
        case WORKSHEET_CALCULATE:
        case WORKSHEET_DEACTIVATE:
        break;
        
        case WORKSHEET_CHANGE:
        case WORKSHEET_SELECTIONCHANGE:
            aVbaArgs.realloc( 1 );
            aVbaArgs[ 0 ] = createRange( rArgs, 0 );
        break;
        
        case WORKSHEET_BEFOREDOUBLECLICK:
        case WORKSHEET_BEFORERIGHTCLICK:
            aVbaArgs.realloc( 2 );
            aVbaArgs[ 0 ] = createRange( rArgs, 0 );
            
        break;
        
        case WORKSHEET_FOLLOWHYPERLINK:
            aVbaArgs.realloc( 1 );
            aVbaArgs[ 0 ] = createHyperlink( rArgs, 0 );
        break;
    }

    /*  For workbook events associated to sheet events, the workbook event gets
        the same arguments but with a Worksheet object in front of them. */
    if( bSheetEventAsBookEvent )
    {
        sal_Int32 nLength = aVbaArgs.getLength();
        uno::Sequence< uno::Any > aVbaArgs2( nLength + 1 );
        aVbaArgs2[ 0 ] = createWorksheet( rArgs, 0 );
        for( sal_Int32 nIndex = 0; nIndex < nLength; ++nIndex )
            aVbaArgs2[ nIndex + 1 ] = aVbaArgs[ nIndex ];
        aVbaArgs = aVbaArgs2;
    }

    return aVbaArgs;
}

void ScVbaEventsHelper::implPostProcessEvent( EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo, bool bCancel ) throw (uno::RuntimeException)
{
    switch( rInfo.mnEventId )
    {
        case WORKBOOK_OPEN:
            mbOpened = true;
            
            if( !mxListener.is() )
                mxListener = new ScVbaEventListener( *this, mxModel, mpDocShell );
        break;
        case WORKBOOK_BEFORECLOSE:
            /*  Execute Auto_Close only if not cancelled by event handler, but
                before UI asks user whether to cancel closing the document. */
            if( !bCancel )
                rEventQueue.push_back( AUTO_CLOSE );
        break;
    }
}

OUString ScVbaEventsHelper::implGetDocumentModuleName( const EventHandlerInfo& rInfo,
        const uno::Sequence< uno::Any >& rArgs ) const throw (lang::IllegalArgumentException)
{
    bool bSheetEvent = false;
    rInfo.maUserData >>= bSheetEvent;
    SCTAB nTab = bSheetEvent ? lclGetTabFromArgs( rArgs, 0 ) : -1;
    if( bSheetEvent && (nTab < 0) )
        throw lang::IllegalArgumentException();

    OUString aCodeName;
    if( bSheetEvent )
        mpDoc->GetCodeName( nTab, aCodeName );
    else
        aCodeName = mpDoc->GetCodeName();
    return aCodeName;
}



namespace {

/** Compares the passed range lists representing sheet selections. Ignores
    selections that refer to different sheets (returns false in this case). */
bool lclSelectionChanged( const ScRangeList& rLeft, const ScRangeList& rRight )
{
    
    bool bLeftEmpty = rLeft.empty();
    bool bRightEmpty = rRight.empty();
    if( bLeftEmpty || bRightEmpty )
        return !(bLeftEmpty && bRightEmpty);

    
    if (rLeft[0]->aStart.Tab() != rRight[0]->aStart.Tab())
        return false;

    
    return rLeft != rRight;
}

} 

bool ScVbaEventsHelper::isSelectionChanged( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xOldSelection( maOldSelection, uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xNewSelection = getXSomethingFromArgs< uno::XInterface >( rArgs, nIndex, false );
    ScCellRangesBase* pOldCellRanges = ScCellRangesBase::getImplementation( xOldSelection );
    ScCellRangesBase* pNewCellRanges = ScCellRangesBase::getImplementation( xNewSelection );
    bool bChanged = !pOldCellRanges || !pNewCellRanges || lclSelectionChanged( pOldCellRanges->GetRangeList(), pNewCellRanges->GetRangeList() );
    maOldSelection <<= xNewSelection;
    return bChanged;
}

uno::Any ScVbaEventsHelper::createWorksheet( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) const
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    
    SCTAB nTab = lclGetTabFromArgs( rArgs, nIndex );
    return uno::Any( excel::getUnoSheetModuleObj( mxModel, nTab ) );
}

uno::Any ScVbaEventsHelper::createRange( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) const
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    
    uno::Reference< excel::XRange > xVbaRange = getXSomethingFromArgs< excel::XRange >( rArgs, nIndex );
    if( !xVbaRange.is() )
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges = getXSomethingFromArgs< sheet::XSheetCellRangeContainer >( rArgs, nIndex );
        uno::Reference< table::XCellRange > xRange = getXSomethingFromArgs< table::XCellRange >( rArgs, nIndex );
        if ( !xRanges.is() && !xRange.is() )
            throw lang::IllegalArgumentException();

        uno::Sequence< uno::Any > aArgs( 2 );
        if ( xRanges.is() )
        {
            aArgs[ 0 ] <<= excel::getUnoSheetModuleObj( xRanges );
            aArgs[ 1 ] <<= xRanges;
        }
        else
        {
            aArgs[ 0 ] <<= excel::getUnoSheetModuleObj( xRange );
            aArgs[ 1 ] <<= xRange;
        }
        xVbaRange.set( createVBAUnoAPIServiceWithArgs( mpShell, "ooo.vba.excel.Range", aArgs ), uno::UNO_QUERY_THROW );
    }
    return uno::Any( xVbaRange );
}

uno::Any ScVbaEventsHelper::createHyperlink( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) const
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Reference< table::XCell > xCell = getXSomethingFromArgs< table::XCell >( rArgs, nIndex, false );
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[ 0 ] <<= excel::getUnoSheetModuleObj( xCell );
    aArgs[ 1 ] <<= xCell;
    uno::Reference< uno::XInterface > xHyperlink( createVBAUnoAPIServiceWithArgs( mpShell, "ooo.vba.excel.Hyperlink", aArgs ), uno::UNO_SET_THROW );
    return uno::Any( xHyperlink );
}

uno::Any ScVbaEventsHelper::createWindow( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) const
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Sequence< uno::Any > aArgs( 3 );
    aArgs[ 0 ] <<= getVBADocument( mxModel );
    aArgs[ 1 ] <<= mxModel;
    aArgs[ 2 ] <<= getXSomethingFromArgs< frame::XController >( rArgs, nIndex, false );
    uno::Reference< uno::XInterface > xWindow( createVBAUnoAPIServiceWithArgs( mpShell, "ooo.vba.excel.Window", aArgs ), uno::UNO_SET_THROW );
    return uno::Any( xWindow );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
ScVbaEventsHelper_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new ScVbaEventsHelper(arguments, context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
