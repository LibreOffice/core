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

#include "vbaeventshelper.hxx"

#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/frame/XBorderResizeListener.hpp>
#include <com/sun/star/frame/XControllerBorder.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/XCloseListener.hpp>

#include <ooo/vba/excel/XApplication.hpp>

#include <cppuhelper/implbase4.hxx>
#include <toolkit/unohlp.hxx>
#include <vbahelper/helperdecl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>

#include "cellsuno.hxx"
#include "convuno.hxx"
#include "vbaapplication.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::script::vba::VBAEventId;
using namespace ::ooo::vba;

// ============================================================================

namespace {

/** Extracts a sheet index from the specified element of the passed sequence.
    The element may be an integer, a Calc range or ranges object, or a VBA Range object. */
SCTAB lclGetTabFromArgs( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) throw (lang::IllegalArgumentException)
{
    VbaEventsHelperBase::checkArgument( rArgs, nIndex );

    // first try to extract a sheet index
    SCTAB nTab = -1;
    if( rArgs[ nIndex ] >>= nTab )
        return nTab;

    // try VBA Range object
    uno::Reference< excel::XRange > xVbaRange = getXSomethingFromArgs< excel::XRange >( rArgs, nIndex );
    if( xVbaRange.is() )
    {
        uno::Reference< XHelperInterface > xVbaHelper( xVbaRange, uno::UNO_QUERY_THROW );
        // TODO: in the future, the parent may be an excel::XChart (chart sheet) -> will there be a common base interface?
        uno::Reference< excel::XWorksheet > xVbaSheet( xVbaHelper->getParent(), uno::UNO_QUERY_THROW );
        // VBA sheet index is 1-based
        return static_cast< SCTAB >( xVbaSheet->getIndex() - 1 );
    }

    // try single UNO range object
    uno::Reference< sheet::XCellRangeAddressable > xCellRangeAddressable = getXSomethingFromArgs< sheet::XCellRangeAddressable >( rArgs, nIndex );
    if( xCellRangeAddressable.is() )
        return xCellRangeAddressable->getRangeAddress().Sheet;

    // at last, try UNO range list
    uno::Reference< sheet::XSheetCellRangeContainer > xRanges = getXSomethingFromArgs< sheet::XSheetCellRangeContainer >( rArgs, nIndex );
    if( xRanges.is() )
    {
        uno::Sequence< table::CellRangeAddress > aRangeAddresses = xRanges->getRangeAddresses();
        if( aRangeAddresses.getLength() > 0 )
            return aRangeAddresses[ 0 ].Sheet;
    }

    throw lang::IllegalArgumentException();
}

} // namespace

// ============================================================================

typedef ::cppu::WeakImplHelper4<
    awt::XWindowListener, util::XCloseListener, frame::XBorderResizeListener, util::XChangesListener > ScVbaEventsListener_BASE;

// This class is to process Workbook window related event
class ScVbaEventsListener : public ScVbaEventsListener_BASE
{
public :
    ScVbaEventsListener( ScVbaEventsHelper& rVbaEvents, const uno::Reference< frame::XModel >& rxModel, ScDocShell* pDocShell );
    virtual ~ScVbaEventsListener();

    void startListening();
    void stopListening();

    // XWindowListener
    virtual void SAL_CALL windowResized( const awt::WindowEvent& aEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const awt::WindowEvent& aEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowShown( const lang::EventObject& aEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const lang::EventObject& aEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) throw (uno::RuntimeException);

    // XCloseListener
    virtual void SAL_CALL queryClosing( const lang::EventObject& Source, ::sal_Bool GetsOwnership ) throw (util::CloseVetoException, uno::RuntimeException);
    virtual void SAL_CALL notifyClosing( const lang::EventObject& Source ) throw (uno::RuntimeException);

    // XBorderResizeListener
    virtual void SAL_CALL borderWidthsChanged( const uno::Reference< uno::XInterface >& aObject, const frame::BorderWidths& aNewSize ) throw (uno::RuntimeException);

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const util::ChangesEvent& aEvent ) throw (uno::RuntimeException);

private:
    uno::Reference< frame::XFrame > getFrame();
    uno::Reference< awt::XWindow > getContainerWindow();
    bool isMouseReleased();
    DECL_LINK( fireResizeMacro, void* );

private:
    ::osl::Mutex maMutex;
    ScVbaEventsHelper& mrVbaEvents;
    uno::Reference< frame::XModel > mxModel;
    ScDocShell* mpDocShell;
    bool mbWindowResized;
    bool mbBorderChanged;
    bool mbDisposed;
};

// ----------------------------------------------------------------------------

ScVbaEventsListener::ScVbaEventsListener( ScVbaEventsHelper& rVbaEvents, const uno::Reference< frame::XModel >& rxModel, ScDocShell* pDocShell ) :
    mrVbaEvents( rVbaEvents ),
    mxModel( rxModel ),
    mpDocShell( pDocShell ),
    mbWindowResized( false ),
    mbBorderChanged( false ),
    mbDisposed( !rxModel.is() )
{
    OSL_TRACE( "ScVbaEventsListener::ScVbaEventsListener( 0x%x ) - ctor ", this );
}

ScVbaEventsListener::~ScVbaEventsListener()
{
    OSL_TRACE( "ScVbaEventsListener::~ScVbaEventsListener( 0x%x ) - dtor ", this );
    stopListening();
}

void ScVbaEventsListener::startListening()
{
    if( !mbDisposed )
    {
        // add window listener
        try
        {
            uno::Reference< awt::XWindow > xWindow( getContainerWindow(), uno::UNO_QUERY_THROW );
            xWindow->addWindowListener( this );
        }
        catch( uno::Exception& )
        {
        }
        // add close listener
        try
        {
            uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( mxModel, uno::UNO_QUERY_THROW );
            xCloseBroadcaster->addCloseListener( this );
        }
        catch( uno::Exception& )
        {
        }
        // add Border resize listener
        try
        {
            uno::Reference< frame::XControllerBorder > xControllerBorder( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
            xControllerBorder->addBorderResizeListener( this );
        }
        catch( uno::Exception& )
        {
        }
        // add content change listener
        try
        {
            uno::Reference< util::XChangesNotifier > xChangesNotifier( mxModel, uno::UNO_QUERY_THROW );
            xChangesNotifier->addChangesListener( this );
        }
        catch( uno::Exception& )
        {
        }
    }
}

void ScVbaEventsListener::stopListening()
{
    if( !mbDisposed )
    {
        try
        {
            uno::Reference< awt::XWindow > xWindow( getContainerWindow(), uno::UNO_QUERY_THROW );
            xWindow->removeWindowListener( this );
        }
        catch( uno::Exception& )
        {
        }
        try
        {
            uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( mxModel, uno::UNO_QUERY_THROW );
            xCloseBroadcaster->removeCloseListener( this );
        }
        catch( uno::Exception& )
        {
        }
        try
        {
            uno::Reference< frame::XControllerBorder > xControllerBorder( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
            xControllerBorder->removeBorderResizeListener( this );
        }
        catch( uno::Exception& )
        {
        }
        try
        {
            uno::Reference< util::XChangesNotifier > xChangesNotifier( mxModel, uno::UNO_QUERY_THROW );
            xChangesNotifier->removeChangesListener( this );
        }
        catch( uno::Exception& )
        {
        }
    }
    mbDisposed = true;
}

void SAL_CALL ScVbaEventsListener::windowResized(  const awt::WindowEvent& /*aEvent*/ ) throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    // Workbook_window_resize event
    mbWindowResized = true;
    if( !mbDisposed && mbBorderChanged )
    {
        if( /*Window* pWindow =*/ VCLUnoHelper::GetWindow( getContainerWindow() ) )
        {
            mbBorderChanged = mbWindowResized = false;
            acquire(); // ensure we don't get deleted before the event is handled
            Application::PostUserEvent( LINK( this, ScVbaEventsListener, fireResizeMacro ), 0 );
        }
    }
}

void SAL_CALL ScVbaEventsListener::windowMoved( const awt::WindowEvent& /*aEvent*/ ) throw ( uno::RuntimeException )
{
    // not interest this time
}

void SAL_CALL ScVbaEventsListener::windowShown( const lang::EventObject& /*aEvent*/ ) throw ( uno::RuntimeException )
{
    // not interest this time
}

void SAL_CALL ScVbaEventsListener::windowHidden( const lang::EventObject& /*aEvent*/ ) throw ( uno::RuntimeException )
{
    // not interest this time
}

void SAL_CALL ScVbaEventsListener::disposing( const lang::EventObject& /*aEvent*/ ) throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    OSL_TRACE( "ScVbaEventsListener::disposing( 0x%x )", this );
    mbDisposed = true;
}

void SAL_CALL ScVbaEventsListener::queryClosing( const lang::EventObject& /*Source*/, sal_Bool /*GetsOwnership*/ ) throw (util::CloseVetoException, uno::RuntimeException)
{
     // it can cancel the close, but need to throw a CloseVetoException, and it will be transmit to caller.
}

void SAL_CALL ScVbaEventsListener::notifyClosing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );
    stopListening();
}

void SAL_CALL ScVbaEventsListener::borderWidthsChanged( const uno::Reference< uno::XInterface >& /*aObject*/, const frame::BorderWidths& /*aNewSize*/ ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );
    // work with WindowResized event to guard Window Resize event.
    mbBorderChanged = true;
    if( !mbDisposed && mbWindowResized )
    {
        if( /*Window* pWindow =*/ VCLUnoHelper::GetWindow( getContainerWindow() ) )
        {
            mbWindowResized = mbBorderChanged = false;
            acquire(); // ensure we don't get deleted before the timer fires.
            Application::PostUserEvent( LINK( this, ScVbaEventsListener, fireResizeMacro ), 0 );
        }
    }
}

void SAL_CALL ScVbaEventsListener::changesOccurred( const util::ChangesEvent& aEvent ) throw (uno::RuntimeException)
{
    sal_Int32 nCount = aEvent.Changes.getLength();
    if( nCount == 0 )
        return;

    util::ElementChange aChange = aEvent.Changes[ 0 ];
    rtl::OUString sOperation;
    aChange.Accessor >>= sOperation;
    if( !sOperation.equalsIgnoreAsciiCaseAscii("cell-change") )
        return;

    if( nCount == 1 )
    {
        uno::Reference< table::XCellRange > xRangeObj;
        aChange.ReplacedElement >>= xRangeObj;
        if( xRangeObj.is() )
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[0] <<= xRangeObj;
            mrVbaEvents.processVbaEvent( WORKSHEET_CHANGE, aArgs );
        }
        return;
    }

    ScRangeList aRangeList;
    for( sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        aChange = aEvent.Changes[ nIndex ];
        aChange.Accessor >>= sOperation;
        uno::Reference< table::XCellRange > xRangeObj;
        aChange.ReplacedElement >>= xRangeObj;
        if( xRangeObj.is() && sOperation.equalsIgnoreAsciiCaseAscii("cell-change") )
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

    if( ( !aRangeList.empty() ) && mpDocShell )
    {
        uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( mpDocShell, aRangeList ) );
        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<= xRanges;
        mrVbaEvents.processVbaEvent( WORKSHEET_CHANGE, aArgs );
    }
}

// ----------------------------------------------------------------------------

uno::Reference< frame::XFrame > ScVbaEventsListener::getFrame()
{
    if( !mbDisposed && mxModel.is() ) try
    {
        uno::Reference< frame::XController > xController( mxModel->getCurrentController(), uno::UNO_QUERY_THROW );
        return xController->getFrame();
    }
    catch( uno::Exception& )
    {
    }
    return uno::Reference< frame::XFrame >();
}

uno::Reference< awt::XWindow > ScVbaEventsListener::getContainerWindow()
{
    try
    {
        uno::Reference< frame::XFrame > xFrame( getFrame(), uno::UNO_SET_THROW );
        return xFrame->getContainerWindow();
    }
    catch( uno::Exception& )
    {
    }
    return uno::Reference< awt::XWindow >();
}

bool ScVbaEventsListener::isMouseReleased()
{
    if( Window* pWindow = VCLUnoHelper::GetWindow( getContainerWindow() ) )
    {
        Window::PointerState aPointerState = pWindow->GetPointerState();
        return (aPointerState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT )) == 0;
    }
    return false;
}

IMPL_LINK( ScVbaEventsListener, fireResizeMacro, void*, EMPTYARG )
{
    if( !mbDisposed && isMouseReleased() ) try
    {
        mrVbaEvents.processVbaEvent( WORKBOOK_WINDOWRESIZE, uno::Sequence< uno::Any >() );
    }
    catch( uno::Exception& )
    {
        // #163419# do not throw exceptions into application core
    }
    release();
    return 0;
}

void ScVbaEventsListener::processWindowResizeMacro()
{
    OSL_TRACE( "**** Attempt to FIRE MACRO **** " );
    if( !mbDisposed )
    {
        try
        {
            mrVbaEvents.processVbaEvent( WORKBOOK_WINDOWRESIZE, uno::Sequence< uno::Any >() );
        }
        catch( uno::Exception& )
        {
        }
    }
}

// ============================================================================

ScVbaEventsHelper::ScVbaEventsHelper( const uno::Sequence< uno::Any >& rArgs, const uno::Reference< uno::XComponentContext >& xContext ) :
    VbaEventsHelperBase( rArgs, xContext ),
    mbOpened( false )
{
    mpDocShell = dynamic_cast< ScDocShell* >( mpShell ); // mpShell from base class
    mpDoc = mpDocShell ? mpDocShell->GetDocument() : 0;

    if( !mxModel.is() || !mpDocShell || !mpDoc )
        return;

#define REGISTER_EVENT( eventid, eventname, type, cancelindex, worksheet ) \
    registerEventHandler( eventid, eventname, type, cancelindex, uno::Any( worksheet ) )

#define REGISTER_WORKBOOK_EVENT( eventid, eventname, cancelindex ) \
    REGISTER_EVENT( WORKBOOK_##eventid, "Workbook_" eventname, EVENTHANDLER_DOCUMENT, cancelindex, false )

#define REGISTER_WORKSHEET_EVENT( eventid, eventname, cancelindex ) \
    REGISTER_EVENT( WORKSHEET_##eventid, "Worksheet_" eventname, EVENTHANDLER_DOCUMENT, cancelindex, true ); \
    REGISTER_EVENT( (USERDEFINED_START + WORKSHEET_##eventid), "Workbook_Sheet" eventname, EVENTHANDLER_DOCUMENT, (((cancelindex) >= 0) ? ((cancelindex) + 1) : -1), false )

    // global
    REGISTER_EVENT( AUTO_OPEN,  "Auto_Open",  EVENTHANDLER_GLOBAL, -1, false );
    REGISTER_EVENT( AUTO_CLOSE, "Auto_Close", EVENTHANDLER_GLOBAL, -1, false );

    // Workbook
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

    // Worksheet events. All events have a corresponding workbook event.
    REGISTER_WORKSHEET_EVENT( ACTIVATE,           "Activate",           -1 );
    REGISTER_WORKSHEET_EVENT( DEACTIVATE,         "Deactivate",         -1 );
    REGISTER_WORKSHEET_EVENT( BEFOREDOUBLECLICK,  "BeforeDoubleClick",  1 );
    REGISTER_WORKSHEET_EVENT( BEFORERIGHTCLICK,   "BeforeRightClick",   1 );
    REGISTER_WORKSHEET_EVENT( CALCULATE,          "Calculate",          -1 );
    REGISTER_WORKSHEET_EVENT( CHANGE,             "Change",             -1 );
    REGISTER_WORKSHEET_EVENT( SELECTIONCHANGE,    "SelectionChange",    -1 );
    REGISTER_WORKSHEET_EVENT( FOLLOWHYPERLINK,    "FollowHyperlink",    -1 );

#undef REGISTER_EVENT
#undef REGISTER_WORKBOOK_EVENT
#undef REGISTER_WORKSHEET_EVENT
}

ScVbaEventsHelper::~ScVbaEventsHelper()
{
}

void SAL_CALL ScVbaEventsHelper::disposing( const lang::EventObject& rSource ) throw (uno::RuntimeException)
{
    mxListener.clear();
    VbaEventsHelperBase::disposing( rSource );
}

// protected ------------------------------------------------------------------

bool ScVbaEventsHelper::implPrepareEvent( EventQueue& rEventQueue,
        const EventHandlerInfo& rInfo, const uno::Sequence< uno::Any >& rArgs ) throw (uno::RuntimeException)
{
    // document and document shell are needed during event processing
    if( !mpShell || !mpDoc )
        throw uno::RuntimeException();

    // framework and Calc fire a few events before 'opened', ignore them
    bool bExecuteEvent = mbOpened;

    // special handling for some events
    switch( rInfo.mnEventId )
    {
        case WORKBOOK_OPEN:
            bExecuteEvent = !mbOpened;
            if( bExecuteEvent )
            {
                // execute delayed Activate event too (see above)
                rEventQueue.push_back( WORKBOOK_ACTIVATE );
                rEventQueue.push_back( WORKBOOK_WINDOWACTIVATE );
                rEventQueue.push_back( AUTO_OPEN );
            }
        break;
        case WORKSHEET_SELECTIONCHANGE:
            // if selection is not changed, then do not fire the event
            bExecuteEvent = bExecuteEvent && isSelectionChanged( rArgs, 0 );
        break;
    }

    if( bExecuteEvent )
    {
        // add workbook event associated to a sheet event
        bool bSheetEvent = false;
        if( (rInfo.maUserData >>= bSheetEvent) && bSheetEvent )
            rEventQueue.push_back( EventQueueEntry( rInfo.mnEventId + USERDEFINED_START, rArgs ) );

        /*  For document events: check if events are enabled via the
            Application.EnableEvents symbol (this is an Excel-only attribute).
            Check this again for every event, as the event handler may change
            the state of the EnableEvents symbol. Global events such as
            AUTO_OPEN and AUTO_CLOSE are always enabled. */
        if( rInfo.meType == EVENTHANDLER_DOCUMENT )
            bExecuteEvent = ScVbaApplication::getDocumentEventsEnabled();
    }

    return bExecuteEvent;
}

uno::Sequence< uno::Any > ScVbaEventsHelper::implBuildArgumentList( const EventHandlerInfo& rInfo,
        const uno::Sequence< uno::Any >& rArgs ) throw (lang::IllegalArgumentException)
{
    // fill arguments for workbook events associated to sheet events according to sheet events, sheet will be added below
    bool bSheetEventAsBookEvent = rInfo.mnEventId > USERDEFINED_START;
    sal_Int32 nEventId = bSheetEventAsBookEvent ? (rInfo.mnEventId - USERDEFINED_START) : rInfo.mnEventId;

    uno::Sequence< uno::Any > aVbaArgs;
    switch( nEventId )
    {
        // *** Workbook ***

        // no arguments
        case WORKBOOK_ACTIVATE:
        case WORKBOOK_DEACTIVATE:
        case WORKBOOK_OPEN:
        break;
        // 1 arg: cancel
        case WORKBOOK_BEFORECLOSE:
        case WORKBOOK_BEFOREPRINT:
            aVbaArgs.realloc( 1 );
            // current cancel state will be inserted by caller
        break;
        // 2 args: saveAs, cancel
        case WORKBOOK_BEFORESAVE:
            aVbaArgs.realloc( 2 );
            checkArgumentType< bool >( rArgs, 0 );
            aVbaArgs[ 0 ] = rArgs[ 0 ];
            // current cancel state will be inserted by caller
        break;
        // 1 arg: success
        case WORKBOOK_AFTERSAVE:
            aVbaArgs.realloc( 1 );
            checkArgumentType< bool >( rArgs, 0 );
            aVbaArgs[ 0 ] = rArgs[ 0 ];
        break;
        // 1 arg: window
        case WORKBOOK_WINDOWACTIVATE:
        case WORKBOOK_WINDOWDEACTIVATE:
        case WORKBOOK_WINDOWRESIZE:
            aVbaArgs.realloc( 1 );
            aVbaArgs[ 0 ] = createWindow();
        break;
        // 1 arg: worksheet
        case WORKBOOK_NEWSHEET:
            aVbaArgs.realloc( 1 );
            aVbaArgs[ 0 ] = createWorksheet( rArgs, 0 );
        break;

        // *** Worksheet ***

        // no arguments
        case WORKSHEET_ACTIVATE:
        case WORKSHEET_CALCULATE:
        case WORKSHEET_DEACTIVATE:
        break;
        // 1 arg: range
        case WORKSHEET_CHANGE:
        case WORKSHEET_SELECTIONCHANGE:
            aVbaArgs.realloc( 1 );
            aVbaArgs[ 0 ] = createRange( rArgs, 0 );
        break;
        // 2 args: range, cancel
        case WORKSHEET_BEFOREDOUBLECLICK:
        case WORKSHEET_BEFORERIGHTCLICK:
            aVbaArgs.realloc( 2 );
            aVbaArgs[ 0 ] = createRange( rArgs, 0 );
            // current cancel state will be inserted by caller
        break;
        // 1 arg: hyperlink
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
        const EventHandlerInfo& rInfo, bool /*bSuccess*/, bool bCancel ) throw (uno::RuntimeException)
{
    switch( rInfo.mnEventId )
    {
        case WORKBOOK_OPEN:
            mbOpened = true;
            // register the listeners
            if( !mxListener.is() )
            {
                mxListener = new ScVbaEventsListener( *this, mxModel, mpDocShell );
                mxListener->startListening();
            }
        break;
        case WORKBOOK_BEFORECLOSE:
            /*  Execute Auto_Close only if not cancelled by event handler, but
                before UI asks user whether to cancel closing the document. */
            if( !bCancel )
                rEventQueue.push_back( AUTO_CLOSE );
        break;
    }
}

::rtl::OUString ScVbaEventsHelper::implGetDocumentModuleName( const EventHandlerInfo& rInfo,
        const uno::Sequence< uno::Any >& rArgs ) const throw (lang::IllegalArgumentException)
{
    bool bSheetEvent = false;
    rInfo.maUserData >>= bSheetEvent;
    SCTAB nTab = bSheetEvent ? lclGetTabFromArgs( rArgs, 0 ) : -1;
    if( bSheetEvent && (nTab < 0) )
        throw lang::IllegalArgumentException();

    String aCodeName;
    if( bSheetEvent )
        mpDoc->GetCodeName( nTab, aCodeName );
    else
        aCodeName = mpDoc->GetCodeName();
    return aCodeName;
}

// private --------------------------------------------------------------------

namespace {

/** Compares the passed range lists representing sheet selections. Ignores
    selections that refer to different sheets (returns false in this case). */
bool lclSelectionChanged( const ScRangeList& rLeft, const ScRangeList& rRight )
{
    // one of the range lists empty? -> return false, if both lists empty
    bool bLeftEmpty = rLeft.Count() == 0;
    bool bRightEmpty = rRight.Count() == 0;
    if( bLeftEmpty || bRightEmpty )
        return !(bLeftEmpty && bRightEmpty);

    // check sheet indexes of the range lists (assuming that all ranges in a list are on the same sheet)
    if( rLeft.GetObject( 0 )->aStart.Tab() != rRight.GetObject( 0 )->aStart.Tab() )
        return false;

    // compare all ranges
    return rLeft != rRight;
}

} // namespace

bool ScVbaEventsHelper::isSelectionChanged( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xOldSelection( maOldSelection, uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xNewSelection = getXSomethingFromArgs< uno::XInterface >( rArgs, nIndex, false );
    if( ScCellRangesBase* pNewCellRanges = ScCellRangesBase::getImplementation( xNewSelection ) )
    {
        bool bChanged = maOldSelection != pNewCellRanges->GetRangeList();
        maOldSelection = pNewCellRanges->GetRangeList();
        return bChanged;
    }
    maOldSelection.RemoveAll();
    return true;
}

uno::Any ScVbaEventsHelper::createWorksheet( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) const
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    // extract sheet index, will throw, if parameter is invalid
    SCTAB nTab = lclGetTabFromArgs( rArgs, nIndex );
    return uno::Any( excel::getUnoSheetModuleObj( mxModel, nTab ) );
}

uno::Any ScVbaEventsHelper::createRange( const uno::Sequence< uno::Any >& rArgs, sal_Int32 nIndex ) const
        throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    // it is possible to pass an existing VBA Range object
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

uno::Any ScVbaEventsHelper::createWindow() const throw (uno::RuntimeException)
{
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[ 0 ] <<= createVBAUnoAPIService( mpShell, "ooo.vba.Application" );
    aArgs[ 1 ] <<= mxModel;
    uno::Reference< uno::XInterface > xWindow( createVBAUnoAPIServiceWithArgs( mpShell, "ooo.vba.excel.Window", aArgs ), uno::UNO_SET_THROW );
    return uno::Any( xWindow );
}

// ============================================================================

namespace vbaeventshelper
{
namespace sdecl = comphelper::service_decl;
sdecl::class_<ScVbaEventsHelper, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaEventsHelper",
    "com.sun.star.script.vba.VBASpreadsheetEventProcessor" );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
