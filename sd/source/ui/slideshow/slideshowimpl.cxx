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

#include "com/sun/star/frame/XComponentLoader.hpp"
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <svl/aeitem.hxx>
#include <svl/urihelper.hxx>

#include <toolkit/unohlp.hxx>

#include <sfx2/infobar.hxx>
#include <sfx2/imagemgr.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdoole2.hxx>

// for child window ids
#include <sfx2/templdlg.hxx>
#include <svx/f3dchild.hxx>
#include <svx/imapdlg.hxx>
#include <svx/fontwork.hxx>
#include <svx/colrctrl.hxx>
#include <svx/bmpmask.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/galbrws.hxx>
#include "NavigatorChildWindow.hxx"
#include "AnimationChildWindow.hxx"
#include <slideshowimpl.hxx>
#include <slideshowviewimpl.hxx>
#include <pgjump.hxx>
#include "PaneHider.hxx"

#include "glob.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "vcl/canvastools.hxx"
#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "rtl/ref.hxx"
#include "slideshow.hrc"
#include "canvas/elapsedtime.hxx"
#include "avmedia/mediawindow.hxx"
#include  "svtools/colrdlg.hxx"
#include "RemoteServer.hxx"
#include "customshowlist.hxx"

#include <boost/bind.hpp>

using ::rtl::OUString;
using ::rtl::OString;
using ::cppu::OInterfaceContainerHelper;
using ::comphelper::ImplementationReference;
using ::com::sun::star::animations::XAnimationNode;
using ::com::sun::star::animations::XAnimationListener;
using ::com::sun::star::awt::XWindow;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;

extern void NotifyDocumentEvent( SdDrawDocument* pDocument, const rtl::OUString& rEventName );
extern String getUiNameFromPageApiNameImpl( const OUString& rApiName );

namespace sd
{
///////////////////////////////////////////////////////////////////////

// Slots, welche im Sfx verwaltet werden und in der SlideShow disabled
// werden sollen (muss in Reihenfolge der SIDs geordnet sein)
static sal_uInt16 const pAllowed[] =
{
    SID_OPENDOC                             , //     5501   // damit interne Spruenge klappen
    SID_JUMPTOMARK                          , //     5598
    SID_OPENHYPERLINK                       , //     6676
    SID_NAVIGATOR                           , //    10366
    SID_PRESENTATION_END                    , //    27218
    SID_NAVIGATOR_PAGENAME                  , //    27287
    SID_NAVIGATOR_STATE                     , //    27288
    SID_NAVIGATOR_INIT                      , //    27289
    SID_NAVIGATOR_PEN                       , //    27291
    SID_NAVIGATOR_PAGE                      , //    27292
    SID_NAVIGATOR_OBJECT                      //    27293
};

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// AnimationSlideController
///////////////////////////////////////////////////////////////////////

class AnimationSlideController
{
public:
    enum Mode { ALL, FROM, CUSTOM, PREVIEW };

public:
    AnimationSlideController( Reference< XIndexAccess > xSlides, Mode eMode );

    void setStartSlideNumber( sal_Int32 nSlideNumber ) { mnStartSlideNumber = nSlideNumber; }
    sal_Int32 getStartSlideIndex() const;

    sal_Int32 getCurrentSlideNumber() const;
    sal_Int32 getCurrentSlideIndex() const;

    sal_Int32 getSlideIndexCount() const { return maSlideNumbers.size(); }
    sal_Int32 getSlideNumberCount() const { return mnSlideCount; }

    sal_Int32 getSlideNumber( sal_Int32 nSlideIndex ) const;

    void insertSlideNumber( sal_Int32 nSlideNumber, bool bVisible = true );
    void setPreviewNode( const Reference< XAnimationNode >& xPreviewNode );

    bool jumpToSlideIndex( sal_Int32 nNewSlideIndex );
    bool jumpToSlideNumber( sal_Int32 nNewSlideIndex );

    bool nextSlide();
    bool previousSlide();

    void displayCurrentSlide( const Reference< XSlideShow >& xShow,
                              const Reference< XDrawPagesSupplier>& xDrawPages,
                              const bool bSkipAllMainSequenceEffects );

    sal_Int32 getNextSlideIndex() const;
    sal_Int32 getPreviousSlideIndex() const;

    bool isVisibleSlideNumber( sal_Int32 nSlideNumber ) const;

    Reference< XDrawPage > getSlideByNumber( sal_Int32 nSlideNumber ) const;

    sal_Int32 getNextSlideNumber() const;

    bool hasSlides() const { return !maSlideNumbers.empty(); }

private:
    bool getSlideAPI( sal_Int32 nSlideNumber, Reference< XDrawPage >& xSlide, Reference< XAnimationNode >& xAnimNode );
    sal_Int32 findSlideIndex( sal_Int32 nSlideNumber ) const;

    bool isValidIndex( sal_Int32 nIndex ) const { return (nIndex >= 0) && (nIndex < (sal_Int32)maSlideNumbers.size()); }
    bool isValidSlideNumber( sal_Int32 nSlideNumber ) const { return (nSlideNumber >= 0) && (nSlideNumber < mnSlideCount); }

private:
    Mode meMode;
    sal_Int32 mnStartSlideNumber;
    std::vector< sal_Int32 > maSlideNumbers;
    std::vector< bool > maSlideVisible;
    std::vector< bool > maSlideVisited;
    Reference< XAnimationNode > mxPreviewNode;
    sal_Int32 mnSlideCount;
    sal_Int32 mnCurrentSlideIndex;
    sal_Int32 mnHiddenSlideNumber;
    Reference< XIndexAccess > mxSlides;
};

Reference< XDrawPage > AnimationSlideController::getSlideByNumber( sal_Int32 nSlideNumber ) const
{
    Reference< XDrawPage > xSlide;
    if( mxSlides.is() && (nSlideNumber >= 0) && (nSlideNumber < mxSlides->getCount()) )
        mxSlides->getByIndex( nSlideNumber ) >>= xSlide;
    return xSlide;
}

bool AnimationSlideController::isVisibleSlideNumber( sal_Int32 nSlideNumber ) const
{
    sal_Int32 nIndex = findSlideIndex( nSlideNumber );

    if( nIndex != -1 )
        return maSlideVisible[ nIndex ];
    else
        return false;
}


void AnimationSlideController::setPreviewNode( const Reference< XAnimationNode >& xPreviewNode )
{
    mxPreviewNode = xPreviewNode;
}

AnimationSlideController::AnimationSlideController( Reference< XIndexAccess > xSlides, Mode eMode  )
:   meMode( eMode )
,   mnStartSlideNumber(-1)
,   mnSlideCount( 0 )
,   mnCurrentSlideIndex(0)
,   mnHiddenSlideNumber( -1 )
,   mxSlides( xSlides )
{
    if( mxSlides.is() )
        mnSlideCount = xSlides->getCount();
}

sal_Int32 AnimationSlideController::getStartSlideIndex() const
{
    if( mnStartSlideNumber >= 0 )
    {
        sal_Int32 nIndex;
        const sal_Int32 nCount = maSlideNumbers.size();

        for( nIndex = 0; nIndex < nCount; nIndex++ )
        {
            if( maSlideNumbers[nIndex] == mnStartSlideNumber )
                return nIndex;
        }
    }

    return 0;
}

sal_Int32 AnimationSlideController::getCurrentSlideNumber() const
{
    if( mnHiddenSlideNumber != -1 )
        return mnHiddenSlideNumber;
    else if( !maSlideNumbers.empty() )
        return maSlideNumbers[mnCurrentSlideIndex];
    else
        return 0;
}

sal_Int32 AnimationSlideController::getCurrentSlideIndex() const
{
    if( mnHiddenSlideNumber != -1 )
        return -1;
    else
        return mnCurrentSlideIndex;
}

bool AnimationSlideController::jumpToSlideIndex( sal_Int32 nNewSlideIndex )
{
    if( isValidIndex( nNewSlideIndex ) )
    {
        mnCurrentSlideIndex = nNewSlideIndex;
        mnHiddenSlideNumber = -1;
        maSlideVisited[mnCurrentSlideIndex] = true;
        return true;
    }
    else
    {
        return false;
    }
}

bool AnimationSlideController::jumpToSlideNumber( sal_Int32 nNewSlideNumber )
{
    sal_Int32 nIndex = findSlideIndex( nNewSlideNumber );
    if( isValidIndex( nIndex ) )
    {
        return jumpToSlideIndex( nIndex );
    }
    else if( (nNewSlideNumber >= 0) && (nNewSlideNumber < mnSlideCount) )
    {
        // jump to a hidden slide
        mnHiddenSlideNumber = nNewSlideNumber;
        return true;
    }
    else
    {
        return false;
    }
}

sal_Int32 AnimationSlideController::getSlideNumber( sal_Int32 nSlideIndex ) const
{
    if( isValidIndex( nSlideIndex ) )
        return maSlideNumbers[nSlideIndex];
    else
        return -1;
}

void AnimationSlideController::insertSlideNumber( sal_Int32 nSlideNumber, bool bVisible /* = true */ )
{
    DBG_ASSERT( isValidSlideNumber( nSlideNumber ), "sd::AnimationSlideController::insertSlideNumber(), illegal index" );
    if( isValidSlideNumber( nSlideNumber ) )
    {
        maSlideNumbers.push_back( nSlideNumber );
        maSlideVisible.push_back( bVisible );
        maSlideVisited.push_back( false );
    }
}

bool AnimationSlideController::getSlideAPI( sal_Int32 nSlideNumber, Reference< XDrawPage >& xSlide, Reference< XAnimationNode >& xAnimNode )
{
    if( isValidSlideNumber( nSlideNumber ) ) try
    {
        xSlide = Reference< XDrawPage >( mxSlides->getByIndex(nSlideNumber), UNO_QUERY_THROW );

        if( meMode == PREVIEW )
        {
            xAnimNode = mxPreviewNode;
        }
        else
        {
               Reference< animations::XAnimationNodeSupplier > xAnimNodeSupplier( xSlide, UNO_QUERY_THROW );
            xAnimNode = xAnimNodeSupplier->getAnimationNode();
        }

        return true;
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::AnimationSlideController::getSlideAPI(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );

    }

    return false;
}

sal_Int32 AnimationSlideController::findSlideIndex( sal_Int32 nSlideNumber ) const
{
    sal_Int32 nIndex;
    const sal_Int32 nCount = maSlideNumbers.size();

    for( nIndex = 0; nIndex < nCount; nIndex++ )
    {
        if( maSlideNumbers[nIndex] == nSlideNumber )
            return nIndex;
    }

    return -1;
}

sal_Int32 AnimationSlideController::getNextSlideIndex() const
{
    switch( meMode )
    {
    case ALL:
        {
            sal_Int32 nNewSlideIndex = mnCurrentSlideIndex + 1;
            if( isValidIndex( nNewSlideIndex ) )
            {
                // if the current slide is not excluded, make sure the
                // next slide is also not excluded.
                // if the current slide is excluded, we want to go
                // to the next slide, even if this is also excluded.
                if( maSlideVisible[mnCurrentSlideIndex] )
                {
                    while( isValidIndex( nNewSlideIndex ) )
                    {
                        if( maSlideVisible[nNewSlideIndex] )
                            break;

                        nNewSlideIndex++;
                    }
                }
            }
            return isValidIndex( nNewSlideIndex ) ? nNewSlideIndex : -1;
        }

    case FROM:
    case CUSTOM:
        return mnHiddenSlideNumber == -1 ? mnCurrentSlideIndex + 1 : mnCurrentSlideIndex;

    default:
    case PREVIEW:
        return -1;

    }
}

sal_Int32 AnimationSlideController::getNextSlideNumber() const
{
    sal_Int32 nNextSlideIndex = getNextSlideIndex();
    if( isValidIndex( nNextSlideIndex ) )
    {
        return maSlideNumbers[nNextSlideIndex];
    }
    else
    {
        return -1;
    }
}


bool AnimationSlideController::nextSlide()
{
    return jumpToSlideIndex( getNextSlideIndex() );
}

sal_Int32 AnimationSlideController::getPreviousSlideIndex() const
{
    sal_Int32 nNewSlideIndex = mnCurrentSlideIndex - 1;

    switch( meMode )
    {
        case ALL:
        {
            // make sure the previous slide is visible
            // or was already visited
            while( isValidIndex( nNewSlideIndex ) )
            {
                if( maSlideVisible[nNewSlideIndex] || maSlideVisited[nNewSlideIndex] )
                    break;

                nNewSlideIndex--;
            }

            break;
        }

        case PREVIEW:
            return -1;

        default:
            break;
    }

    return nNewSlideIndex;
}

bool AnimationSlideController::previousSlide()
{
    return jumpToSlideIndex( getPreviousSlideIndex() );
}

void AnimationSlideController::displayCurrentSlide( const Reference< XSlideShow >& xShow,
                                                    const Reference< XDrawPagesSupplier>& xDrawPages,
                                                    const bool bSkipAllMainSequenceEffects )
{
    const sal_Int32 nCurrentSlideNumber = getCurrentSlideNumber();

    if( xShow.is() && (nCurrentSlideNumber != -1 ) )
    {
        Reference< XDrawPage > xSlide;
        Reference< XAnimationNode > xAnimNode;
        ::std::vector<PropertyValue> aProperties;

        const sal_Int32 nNextSlideNumber = getNextSlideNumber();
        if( getSlideAPI( nNextSlideNumber, xSlide, xAnimNode )  )
        {
            Sequence< Any > aValue(2);
            aValue[0] <<= xSlide;
            aValue[1] <<= xAnimNode;
            aProperties.push_back(
                PropertyValue( "Prefetch" ,
                    -1,
                    Any(aValue),
                    PropertyState_DIRECT_VALUE));
        }
        if (bSkipAllMainSequenceEffects)
        {
            // Add one property that prevents the slide transition from being
            // shown (to speed up the transition to the previous slide) and
            // one to show all main sequence effects so that the user can
            // continue to undo effects.
            aProperties.push_back(
                PropertyValue( "SkipAllMainSequenceEffects",
                    -1,
                    Any(sal_True),
                    PropertyState_DIRECT_VALUE));
            aProperties.push_back(
                PropertyValue("SkipSlideTransition",
                    -1,
                    Any(sal_True),
                    PropertyState_DIRECT_VALUE));
        }

        // Convert vector into uno Sequence.
        Sequence< PropertyValue > aPropertySequence (aProperties.size());
        for (int nIndex=0,nCount=aProperties.size();nIndex<nCount; ++nIndex)
            aPropertySequence[nIndex] = aProperties[nIndex];

        if( getSlideAPI( nCurrentSlideNumber, xSlide, xAnimNode ) )
            xShow->displaySlide( xSlide, xDrawPages, xAnimNode, aPropertySequence );
    }
}

///////////////////////////////////////////////////////////////////////
// class SlideshowImpl
///////////////////////////////////////////////////////////////////////

SlideshowImpl::SlideshowImpl( const Reference< XPresentation2 >& xPresentation, ViewShell* pViewSh, ::sd::View* pView, SdDrawDocument* pDoc, ::Window* pParentWindow )
: SlideshowImplBase( m_aMutex )
, mxModel(pDoc->getUnoModel(),UNO_QUERY_THROW)
, mpView(pView)
, mpViewShell(pViewSh)
, mpDocSh(pDoc->GetDocSh())
, mpDoc(pDoc)
, mpNewAttr(0)
, mpParentWindow(pParentWindow)
, mpShowWindow(0)
, mpTimeButton(0)
, mnRestoreSlide(0)
, maPresSize( -1, -1 )
, meAnimationMode(ANIMATIONMODE_SHOW)
, mpOldActiveWindow(0)
, mnChildMask( 0 )
, mbGridVisible(false)
, mbBordVisible(false)
, mbSlideBorderVisible(false)
, mbSetOnlineSpelling(false)
, mbDisposed(false)
, mbRehearseTimings(false)
, mbDesignMode(false)
, mbIsPaused(false)
, mbInputFreeze(false)
, mbActive(sal_False)
, maPresSettings( pDoc->getPresentationSettings() )
, mnUserPaintColor( 0x80ff0000L )
, mbUsePen(false)
, mdUserPaintStrokeWidth ( 150.0 )
#ifdef ENABLE_ERASER_UI
, mbSwitchEraserMode(false)
, mnEraseInkSize(100)
#endif
, mnEntryCounter(0)
, mnLastSlideNumber(-1)
, msOnClick( "OnClick" )
, msBookmark( "Bookmark" )
, msVerb( "Verb" )
, mnEndShowEvent(0)
, mnContextMenuEvent(0)
, mnUpdateEvent(0)
, mxPresentation( xPresentation )
{
    if( mpViewShell )
        mpOldActiveWindow = mpViewShell->GetActiveWindow();

    maUpdateTimer.SetTimeoutHdl(LINK(this, SlideshowImpl, updateHdl));

    maDeactivateTimer.SetTimeoutHdl(LINK(this, SlideshowImpl, deactivateHdl));
    maDeactivateTimer.SetTimeout( 20 );

    maInputFreezeTimer.SetTimeoutHdl( LINK( this, SlideshowImpl, ReadyForNextInputHdl ) );
    maInputFreezeTimer.SetTimeout( 20 );

    SvtSaveOptions aOptions;

        // no autosave during show
    if( aOptions.IsAutoSave() )
        mbAutoSaveWasOn = true;

    Application::AddEventListener( LINK( this, SlideshowImpl, EventListenerHdl ) );

    mbUsePen = maPresSettings.mbMouseAsPen;

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    if( pOptions )
    {
        mnUserPaintColor = pOptions->GetPresentationPenColor();
        mdUserPaintStrokeWidth = pOptions->GetPresentationPenWidth();
    }
}

SlideshowImpl::~SlideshowImpl()
{
    SdModule *pModule = SD_MOD();
    //rhbz#806663 SlideshowImpl can outlive SdModule
    SdOptions* pOptions = pModule ?
        pModule->GetSdOptions(DOCUMENT_TYPE_IMPRESS) : NULL;
    if( pOptions )
    {
        pOptions->SetPresentationPenColor(mnUserPaintColor);
        pOptions->SetPresentationPenWidth(mdUserPaintStrokeWidth);
    }

    Application::RemoveEventListener( LINK( this, SlideshowImpl, EventListenerHdl ) );

    maDeactivateTimer.Stop();

    if( !mbDisposed )
    {
        OSL_FAIL("SlideshowImpl::~SlideshowImpl(), component was not disposed!");
        disposing();
    }
}

void SAL_CALL SlideshowImpl::disposing()
{
#ifdef ENABLE_SDREMOTE
    RemoteServer::presentationStopped();
#endif
    if( mxShow.is() && mpDoc )
        NotifyDocumentEvent( mpDoc, "OnEndPresentation" );

    if( mbAutoSaveWasOn )
        setAutoSaveState( true );

    if( mnEndShowEvent )
        Application::RemoveUserEvent( mnEndShowEvent );
    if( mnContextMenuEvent )
        Application::RemoveUserEvent( mnContextMenuEvent );

    maInputFreezeTimer.Stop();

    SolarMutexGuard aSolarGuard;

    if( !mxShow.is() )
        return;

    if( mxPresentation.is() )
        mxPresentation->end();

    maUpdateTimer.Stop();

    if( mnUpdateEvent )
    {
        Application::RemoveUserEvent( mnUpdateEvent );
        mnUpdateEvent = 0;
    }

    removeShapeEvents();

    if( mxListenerProxy.is() )
        mxListenerProxy->removeAsSlideShowListener();

    try
    {
        if( mxView.is() )
            mxShow->removeView( mxView.getRef() );

        Reference< XComponent > xComponent( mxShow, UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();

        if( mxView.is() )
            mxView->dispose();
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::stop(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );

    }

    mxShow.clear();
    mxView.reset();
    mxListenerProxy.clear();
    mpSlideController.reset();

    // der DrawView das Praesentationfenster wegnehmen und ihr dafuer ihre alten Fenster wiedergeben
    if( mpShowWindow && mpView )
        mpView->DeleteWindowFromPaintView( mpShowWindow );

    if( mpView )
        mpView->SetAnimationPause( sal_False );

    if( mpViewShell )
    {
        mpViewShell->SetActiveWindow(mpOldActiveWindow);
        mpShowWindow->SetViewShell( NULL );
    }

    if( mpView )
        mpView->InvalidateAllWin();

    if( maPresSettings.mbFullScreen )
    {
#ifndef DISABLE_SCRIPTING
        // restore StarBASICErrorHdl
        StarBASIC::SetGlobalErrorHdl(maStarBASICGlobalErrorHdl);
        maStarBASICGlobalErrorHdl = Link();
#endif
    }
    else
    {
        if( mpShowWindow )
            mpShowWindow->Hide();
    }

    if( meAnimationMode == ANIMATIONMODE_SHOW )
    {
        mpDocSh->SetSlotFilter();
        mpDocSh->ApplySlotFilter();

        Help::EnableContextHelp();
        Help::EnableExtHelp();

        showChildWindows();
        mnChildMask = 0UL;
    }

    // aktuelle Fenster wieder einblenden
    if( mpViewShell && !mpViewShell->ISA(PresentationViewShell))
    {
        if( meAnimationMode == ANIMATIONMODE_SHOW )
        {
            mpViewShell->GetViewShellBase().ShowUIControls (true);
            mpPaneHider.reset();
        }
        else if( meAnimationMode == ANIMATIONMODE_PREVIEW )
        {
            mpViewShell->ShowUIControls (true);
        }
    }

    if( mpTimeButton )
    {
        mpTimeButton->Hide();
        delete mpTimeButton;
        mpTimeButton = 0;
    }

    if( mpShowWindow )
        mpShowWindow->Hide();

    if ( mpViewShell )
    {
        if( meAnimationMode == ANIMATIONMODE_SHOW )
        {
            ::sd::Window* pActWin = mpViewShell->GetActiveWindow();

            if (pActWin)
            {
                Size aVisSizePixel = pActWin->GetOutputSizePixel();
                Rectangle aVisAreaWin = pActWin->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
                mpViewShell->VisAreaChanged(aVisAreaWin);
                mpView->VisAreaChanged(pActWin);
                pActWin->GrabFocus();
            }
        }

        // restart the custom show dialog if he started us
        if( mpViewShell->IsStartShowWithDialog() && getDispatcher() )
        {
            mpViewShell->SetStartShowWithDialog( sal_False );
            getDispatcher()->Execute( SID_CUSTOMSHOW_DLG, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        }

        mpViewShell->GetViewShellBase().UpdateBorder(true);
    }

    if( mpShowWindow )
    {
        delete mpShowWindow;
        mpShowWindow = 0;
    }

    setActiveXToolbarsVisible( sal_True );

    Application::EnableNoYieldMode(false);
    Application::RemovePostYieldListener(LINK(this, SlideshowImpl, PostYieldListener));

    mbDisposed = true;
}

bool SlideshowImpl::startPreview(
        const Reference< XDrawPage >& xDrawPage,
        const Reference< XAnimationNode >& xAnimationNode,
        ::Window* pParent )
{
    bool bRet = false;

    try
    {
        const Reference<lang::XServiceInfo> xServiceInfo( xDrawPage, UNO_QUERY );
        if (xServiceInfo.is()) {
            const Sequence<OUString> supportedServices(
                xServiceInfo->getSupportedServiceNames() );
            for ( sal_Int32 pos = supportedServices.getLength(); pos--; ) {
                if ( supportedServices[pos] == "com.sun.star.drawing.MasterPage" ) {
                    OSL_FAIL("sd::SlideshowImpl::startPreview() "
                              "not allowed on master page!");
                    return false;
                }
            }
        }

        mxPreviewDrawPage = xDrawPage;
        mxPreviewAnimationNode = xAnimationNode;
        meAnimationMode = ANIMATIONMODE_PREVIEW;

        maPresSettings.mbAll = sal_False;
        maPresSettings.mbEndless = sal_False;
        maPresSettings.mbCustomShow = sal_False;
        maPresSettings.mbManual = sal_False;
        maPresSettings.mbMouseVisible = sal_False;
        maPresSettings.mbMouseAsPen = sal_False;
        maPresSettings.mbLockedPages = sal_False;
        maPresSettings.mbAlwaysOnTop = sal_False;
        maPresSettings.mbFullScreen = sal_False;
        maPresSettings.mbAnimationAllowed = sal_True;
        maPresSettings.mnPauseTimeout = 0;
        maPresSettings.mbShowPauseLogo = sal_False;
        maPresSettings.mbStartWithNavigator = sal_False;

        Reference< XDrawPagesSupplier > xDrawPages( mpDoc->getUnoModel(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xSlides( xDrawPages->getDrawPages(), UNO_QUERY_THROW );
        mpSlideController.reset( new AnimationSlideController( xSlides, AnimationSlideController::PREVIEW ) );

        sal_Int32 nSlideNumber = 0;
        Reference< XPropertySet > xSet( mxPreviewDrawPage, UNO_QUERY_THROW );
        xSet->getPropertyValue( "Number" ) >>= nSlideNumber;
        mpSlideController->insertSlideNumber( nSlideNumber-1 );
        mpSlideController->setPreviewNode( xAnimationNode );

        mpShowWindow = new ShowWindow( this, ((pParent == 0) && mpViewShell) ?  mpParentWindow : pParent );
        if( mpViewShell )
        {
            mpViewShell->SetActiveWindow( mpShowWindow );
            mpShowWindow->SetViewShell (mpViewShell);
            mpViewShell->ShowUIControls (false);
        }

        if( mpView )
        {
            mpView->AddWindowToPaintView( mpShowWindow );
            mpView->SetAnimationPause( sal_True );
        }

        // call resize handler
        if( pParent )
        {
            maPresSize = pParent->GetSizePixel();
        }
        else if( mpViewShell )
        {
            Rectangle aContentRect (mpViewShell->GetViewShellBase().getClientRectangle());
            if (Application::GetSettings().GetLayoutRTL())
            {
                aContentRect.nLeft = aContentRect.nRight;
                aContentRect.nRight += aContentRect.nRight;
            }
            maPresSize = aContentRect.GetSize();
            mpShowWindow->SetPosPixel( aContentRect.TopLeft() );
        }
        else
        {
            OSL_FAIL("sd::SlideshowImpl::startPreview(), I need either a parent window or a viewshell!");
        }
        resize( maPresSize );

        sal_Int32 nPropertyCount = 1;
        if( mxPreviewAnimationNode.is() )
            nPropertyCount++;

        Sequence< beans::PropertyValue > aProperties(nPropertyCount);
        aProperties[0].Name = "AutomaticAdvancement";
        aProperties[0].Value = uno::makeAny( (double)1.0 ); // one second timeout

        if( mxPreviewAnimationNode.is() )
        {
            aProperties[1].Name = "NoSlideTransitions";
            aProperties[1].Value = uno::makeAny( sal_True );
        }

        bRet = startShowImpl( aProperties );

        if( mpShowWindow != 0 && meAnimationMode == ANIMATIONMODE_PREVIEW )
            mpShowWindow->SetPreviewMode();

    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::startPreview(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
        bRet = false;
    }

    return bRet;
}

bool SlideshowImpl::startShow( PresentationSettingsEx* pPresSettings )
{
    const rtl::Reference<SlideshowImpl> this_(this);

    DBG_ASSERT( !mxShow.is(), "sd::SlideshowImpl::startShow(), called twice!" );
    if( mxShow.is() )
        return true;
    DBG_ASSERT( mpParentWindow!=NULL, "sd::SlideshowImpl::startShow() called without parent window" );
    if (mpParentWindow == NULL)
        return false;

    bool bRet = false;

    try
    {
        if( pPresSettings )
        {
            maPresSettings = *pPresSettings;
            mbRehearseTimings = pPresSettings->mbRehearseTimings;
        }

        // ---

        String  aPresSlide( maPresSettings.maPresPage );
        SdPage* pStartPage = mpViewShell ? mpViewShell->GetActualPage() : 0;
        bool    bStartWithActualSlide =  pStartPage &&
                                        ( (meAnimationMode != ANIMATIONMODE_SHOW) ||
                                           SD_MOD()->GetSdOptions( mpDoc->GetDocumentType() )->IsStartWithActualPage() );

        // sollen Zeiten gestoppt werden?
        if( mbRehearseTimings )
        {
            maPresSettings.mbEndless = sal_False;
            maPresSettings.mbManual = sal_True;
            maPresSettings.mbMouseVisible = sal_True;
            maPresSettings.mbMouseAsPen = sal_False;
            maPresSettings.mnPauseTimeout = 0;
            maPresSettings.mbShowPauseLogo = sal_False;
            maPresSettings.mbStartWithNavigator = sal_False;
        }

        if( pStartPage )
        {
            if( pStartPage->GetPageKind() == PK_NOTES )
            {
                // we are in notes page mode, so get
                // the corresponding draw page
                const sal_uInt16 nPgNum = ( pStartPage->GetPageNum() - 2 ) >> 1;
                pStartPage = mpDoc->GetSdPage( nPgNum, PK_STANDARD );
            }
        }

        if( bStartWithActualSlide )
        {
            if( meAnimationMode != ANIMATIONMODE_SHOW )
            {
                if( pStartPage->GetPageKind() == PK_STANDARD )
                {
                    aPresSlide = pStartPage->GetName();
                    maPresSettings.mbAll = false;
                }
                else
                {
                    bStartWithActualSlide = false;
                }
            }
        }
        else
        {
            if( pStartPage->GetPageKind() != PK_STANDARD )
            {
                bStartWithActualSlide = false;
            }
        }

        // build page list
        createSlideList( maPresSettings.mbAll, false, aPresSlide );

        if( bStartWithActualSlide )
        {
            sal_Int32 nSlideNum = ( pStartPage->GetPageNum() - 1 ) >> 1;

            if( !maPresSettings.mbAll && !maPresSettings.mbCustomShow )
            {
                // its start from dia, find out if it is located before our current Slide
                const sal_Int32 nSlideCount = mpDoc->GetSdPageCount( PK_STANDARD );
                sal_Int32 nSlide;
                for( nSlide = 0; (nSlide < nSlideCount); nSlide++ )
                {
                    if( mpDoc->GetSdPage( (sal_uInt16) nSlide, PK_STANDARD )->GetName() == aPresSlide )
                        break;
                }

                if( nSlide > nSlideNum )
                    nSlideNum = -1;
            }

            if( nSlideNum != -1 )
                mpSlideController->setStartSlideNumber( nSlideNum );
        }

        // remember Slide number from where the show was started
        if( pStartPage )
            mnRestoreSlide = ( pStartPage->GetPageNum() - 1 ) / 2;

        if( mpSlideController->hasSlides() )
        {
            // hide child windows
            hideChildWindows();

            mpShowWindow = new ShowWindow( this, mpParentWindow );
            mpShowWindow->SetMouseAutoHide( !maPresSettings.mbMouseVisible );
            if( mpViewShell )
            {
                mpViewShell->SetActiveWindow( mpShowWindow );
                mpShowWindow->SetViewShell (mpViewShell);
                mpViewShell->GetViewShellBase().ShowUIControls (false);
                // Hide the side panes for in-place presentations.
                if ( ! maPresSettings.mbFullScreen)
                    mpPaneHider.reset(new PaneHider(*mpViewShell,this));

                if( getViewFrame() )
                    getViewFrame()->SetChildWindow( SID_NAVIGATOR, maPresSettings.mbStartWithNavigator );
            }

            // these Slots are forbiden in other views for this document
            if( mpDocSh )
            {
                mpDocSh->SetSlotFilter( sal_True, sizeof( pAllowed ) / sizeof( sal_uInt16 ), pAllowed );
                mpDocSh->ApplySlotFilter();
            }

            Help::DisableContextHelp();
            Help::DisableExtHelp();

            if( maPresSettings.mbFullScreen )
            {
#ifndef DISABLE_SCRIPTING
                // disable basic ide error handling
                maStarBASICGlobalErrorHdl = StarBASIC::GetGlobalErrorHdl();
                StarBASIC::SetGlobalErrorHdl( Link() );
#endif
            }

            // call resize handler
            maPresSize = mpParentWindow->GetSizePixel();
            if( !maPresSettings.mbFullScreen && mpViewShell )
            {
                const Rectangle& aClientRect = mpViewShell->GetViewShellBase().getClientRectangle();
                maPresSize = aClientRect.GetSize();
                mpShowWindow->SetPosPixel( aClientRect.TopLeft() );
                resize( maPresSize );
            }

            // #i41824#
            // Note: In FullScreen Mode the OS (window manager) sends a resize to
            // the WorkWindow once it actually resized it to full size.  The
            // WorkWindow propagates the resize to the DrawViewShell which calls
            // resize() at the SlideShow (this).  Calling resize here results in a
            // temporary display of a black window in the window's default size

            if( mpView )
            {
                mpView->AddWindowToPaintView( mpShowWindow );
                mpView->SetAnimationPause( sal_True );
            }

            SfxBindings* pBindings = getBindings();
            if( pBindings )
            {
                pBindings->Invalidate( SID_PRESENTATION );
                pBindings->Invalidate( SID_REHEARSE_TIMINGS );
            }

            mpShowWindow->GrabFocus();

            std::vector<beans::PropertyValue> aProperties;
            aProperties.reserve( 4 );

            aProperties.push_back(
                beans::PropertyValue( "AdvanceOnClick" ,
                    -1, Any( ! (maPresSettings.mbLockedPages != sal_False) ),
                    beans::PropertyState_DIRECT_VALUE ) );

            aProperties.push_back(
                beans::PropertyValue( "ImageAnimationsAllowed" ,
                    -1, Any( maPresSettings.mbAnimationAllowed != sal_False ),
                    beans::PropertyState_DIRECT_VALUE ) );

            const sal_Bool bZOrderEnabled(
                SD_MOD()->GetSdOptions( mpDoc->GetDocumentType() )->IsSlideshowRespectZOrder() );
            aProperties.push_back(
                beans::PropertyValue( "DisableAnimationZOrder" ,
                    -1, Any( bZOrderEnabled == sal_False ),
                    beans::PropertyState_DIRECT_VALUE ) );

            aProperties.push_back(
                beans::PropertyValue( "ForceManualAdvance" ,
                    -1, Any( maPresSettings.mbManual != sal_False ),
                    beans::PropertyState_DIRECT_VALUE ) );

            if( mbUsePen )
             {
                aProperties.push_back(
                    beans::PropertyValue( "UserPaintColor" ,
                        // User paint color is black by default.
                        -1, Any( mnUserPaintColor ),
                        beans::PropertyState_DIRECT_VALUE ) );

                aProperties.push_back(
                    beans::PropertyValue( "UserPaintStrokeWidth" ,
                        // User paint color is black by default.
                        -1, Any( mdUserPaintStrokeWidth ),
                        beans::PropertyState_DIRECT_VALUE ) );
            }

            if (mbRehearseTimings) {
                aProperties.push_back(
                    beans::PropertyValue( "RehearseTimings" ,
                        -1, Any(true), beans::PropertyState_DIRECT_VALUE ) );
            }

            bRet = startShowImpl( Sequence<beans::PropertyValue>(
                                      &aProperties[0], aProperties.size() ) );

        }

        setActiveXToolbarsVisible( sal_False );
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::startShow(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
        bRet = false;
    }

    return bRet;
}

bool SlideshowImpl::startShowImpl( const Sequence< beans::PropertyValue >& aProperties )
{
    try
    {
        mxShow = Reference< XSlideShow >( createSlideShow(), UNO_QUERY_THROW );
        mxView = mxView.createFromQuery( new SlideShowView(
                                             *mpShowWindow,
                                             mpDoc,
                                             meAnimationMode,
                                             this,
                                             maPresSettings.mbFullScreen) );

        // try add wait symbol to properties:
        const Reference<rendering::XSpriteCanvas> xSpriteCanvas(
            mxView->getCanvas() );
        if (xSpriteCanvas.is())
        {
            BitmapEx waitSymbolBitmap( SdResId(BMP_WAIT_ICON) );
            const Reference<rendering::XBitmap> xBitmap(
                vcl::unotools::xBitmapFromBitmapEx(
                    xSpriteCanvas->getDevice(), waitSymbolBitmap ) );
            if (xBitmap.is())
            {
                mxShow->setProperty(
                    beans::PropertyValue( "WaitSymbolBitmap" ,
                        -1,
                        makeAny( xBitmap ),
                        beans::PropertyState_DIRECT_VALUE ) );
            }
        }

        const sal_Int32 nCount = aProperties.getLength();
        sal_Int32 nIndex;
        for( nIndex = 0; nIndex < nCount; nIndex++ )
            mxShow->setProperty( aProperties[nIndex] );

        mxShow->addView( mxView.getRef() );

        mxListenerProxy.set( new SlideShowListenerProxy( this, mxShow ) );
        mxListenerProxy->addAsSlideShowListener();


        NotifyDocumentEvent( mpDoc, "OnStartPresentation");
        displaySlideIndex( mpSlideController->getStartSlideIndex() );

        return true;
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::startShowImpl(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
        return false;
    }
}

/** called only by the slideshow view when the first paint event occurs.
    This actually starts the slideshow. */
void SlideshowImpl::onFirstPaint()
{
    if( mpShowWindow )
    {
        /*
        mpShowWindow->SetBackground( Wallpaper( Color( COL_BLACK ) ) );
        mpShowWindow->Erase();
        mpShowWindow->SetBackground();
        */
    }

    SolarMutexGuard aSolarGuard;
    maUpdateTimer.SetTimeout( (sal_uLong)100 );
    maUpdateTimer.Start();
}

void SlideshowImpl::paint( const Rectangle& /* rRect */ )
{
    if( mxView.is() ) try
    {
        awt::PaintEvent aEvt;
        // aEvt.UpdateRect = TODO
        mxView->paint( aEvt );
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::paint(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::addSlideShowListener( const Reference< XSlideShowListener >& xListener ) throw (RuntimeException)
{
    if( mxListenerProxy.is() )
        mxListenerProxy->addSlideShowListener( xListener );
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::removeSlideShowListener( const Reference< XSlideShowListener >& xListener ) throw (RuntimeException)
{
    if( mxListenerProxy.is() )
        mxListenerProxy->removeSlideShowListener( xListener );
}

// ---------------------------------------------------------

void SlideshowImpl::slideEnded(const bool bReverse)
{
    if (bReverse)
        gotoPreviousSlide(true);
    else
        gotoNextSlide();
}

// ---------------------------------------------------------

void SlideshowImpl::removeShapeEvents()
{
    if( mxShow.is() && mxListenerProxy.is() ) try
    {
        WrappedShapeEventImplMap::iterator aIter;
        const WrappedShapeEventImplMap::iterator aEnd( maShapeEventMap.end() );

        for( aIter = maShapeEventMap.begin(); aIter != aEnd; ++aIter )
        {
            mxListenerProxy->removeShapeEventListener( (*aIter).first );
            mxShow->setShapeCursor( (*aIter).first, awt::SystemPointer::ARROW );
        }

        maShapeEventMap.clear();
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::removeShapeEvents(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// ---------------------------------------------------------

void SlideshowImpl::registerShapeEvents(sal_Int32 nSlideNumber)
{
    if( nSlideNumber >= 0 ) try
    {
        Reference< XDrawPagesSupplier > xDrawPages( mxModel, UNO_QUERY_THROW );
        Reference< XIndexAccess > xPages( xDrawPages->getDrawPages(), UNO_QUERY_THROW );

        Reference< XShapes > xDrawPage;
        xPages->getByIndex(nSlideNumber) >>= xDrawPage;

        if( xDrawPage.is() )
        {
            Reference< XMasterPageTarget > xMasterPageTarget( xDrawPage, UNO_QUERY );
            if( xMasterPageTarget.is() )
            {
                Reference< XShapes > xMasterPage( xMasterPageTarget->getMasterPage(), UNO_QUERY );
                if( xMasterPage.is() )
                    registerShapeEvents( xMasterPage );
            }
            registerShapeEvents( xDrawPage );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::registerShapeEvents(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// ---------------------------------------------------------

void SlideshowImpl::registerShapeEvents( Reference< XShapes >& xShapes ) throw( Exception )
{
    try
    {
        const sal_Int32 nShapeCount = xShapes->getCount();
        sal_Int32 nShape;
        for( nShape = 0; nShape < nShapeCount; nShape++ )
        {
            Reference< XShape > xShape;
            xShapes->getByIndex( nShape ) >>= xShape;

            if( xShape.is() && xShape->getShapeType() == "com.sun.star.drawing.GroupShape" )
            {
                Reference< XShapes > xSubShapes( xShape, UNO_QUERY );
                if( xSubShapes.is() )
                    registerShapeEvents( xSubShapes );
            }

            Reference< XPropertySet > xSet( xShape, UNO_QUERY );
            if( !xSet.is() )
                continue;

            Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
            if( !xSetInfo.is() || !xSetInfo->hasPropertyByName( msOnClick ) )
                continue;

            WrappedShapeEventImplPtr pEvent( new WrappedShapeEventImpl );
            xSet->getPropertyValue( msOnClick ) >>= pEvent->meClickAction;

            switch( pEvent->meClickAction )
            {
            case ClickAction_PREVPAGE:
            case ClickAction_NEXTPAGE:
            case ClickAction_FIRSTPAGE:
            case ClickAction_LASTPAGE:
            case ClickAction_STOPPRESENTATION:
                break;
            case ClickAction_BOOKMARK:
                if( xSetInfo->hasPropertyByName( msBookmark ) )
                    xSet->getPropertyValue( msBookmark ) >>= pEvent->maStrBookmark;
                if( getSlideNumberForBookmark( pEvent->maStrBookmark ) == -1 )
                    continue;
                break;
            case ClickAction_DOCUMENT:
            case ClickAction_SOUND:
            case ClickAction_PROGRAM:
            case ClickAction_MACRO:
                if( xSetInfo->hasPropertyByName( msBookmark ) )
                    xSet->getPropertyValue( msBookmark ) >>= pEvent->maStrBookmark;
                break;
            case ClickAction_VERB:
                if( xSetInfo->hasPropertyByName( msVerb ) )
                    xSet->getPropertyValue( msVerb ) >>= pEvent->mnVerb;
                break;
            default:
                continue; // skip all others
            }

            maShapeEventMap[ xShape ] = pEvent;

            if( mxListenerProxy.is() )
                mxListenerProxy->addShapeEventListener( xShape );
            mxShow->setShapeCursor( xShape, awt::SystemPointer::REFHAND );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::registerShapeEvents(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// ---------------------------------------------------------

void SlideshowImpl::displayCurrentSlide (const bool bSkipAllMainSequenceEffects)
{
    stopSound();
    removeShapeEvents();

    if( mpSlideController.get() && mxShow.is() )
    {
        Reference< XDrawPagesSupplier > xDrawPages( mpDoc->getUnoModel(),
                                                    UNO_QUERY_THROW );
        mpSlideController->displayCurrentSlide( mxShow, xDrawPages, bSkipAllMainSequenceEffects );
        registerShapeEvents(mpSlideController->getCurrentSlideNumber());
        update();

        SfxBindings* pBindings = getBindings();
        if( pBindings )
        {
            pBindings->Invalidate( SID_NAVIGATOR_STATE );
            pBindings->Invalidate( SID_NAVIGATOR_PAGENAME );
        }
    }
}

// ---------------------------------------------------------

void SlideshowImpl::endPresentation()
{
    if( !mnEndShowEvent )
        mnEndShowEvent = Application::PostUserEvent( LINK(this, SlideshowImpl, endPresentationHdl) );
}

// ---------------------------------------------------------

IMPL_LINK_NOARG(SlideshowImpl, endPresentationHdl)
{
    mnEndShowEvent = 0;

    if( mxPresentation.is() )
        mxPresentation->end();
    return 0;
}

// ---------------------------------------------------------

void SAL_CALL SlideshowImpl::pause() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( !mbIsPaused ) try
    {
        mbIsPaused = sal_True;
        if( mxShow.is() )
        {
            mxShow->pause(sal_True);

            if( mxListenerProxy.is() )
                mxListenerProxy->paused();
        }
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::pause(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// ---------------------------------------------------------

void SAL_CALL SlideshowImpl::resume() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mbIsPaused ) try
    {
        if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_BLANK )
        {
            mpShowWindow->RestartShow();
        }
        else
        {
            mbIsPaused = sal_False;;
            if( mxShow.is() )
            {
                mxShow->pause(sal_False);
                update();

                if( mxListenerProxy.is() )
                    mxListenerProxy->resumed();
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::resume(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
#ifdef ENABLE_SDREMOTE
    RemoteServer::presentationStarted( this );
#endif
}

// ---------------------------------------------------------

sal_Bool SAL_CALL SlideshowImpl::isPaused() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mbIsPaused;
}

// ---------------------------------------------------------

void SAL_CALL SlideshowImpl::blankScreen( sal_Int32 nColor ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mpShowWindow && mpSlideController )
    {
        if( mpShowWindow->SetBlankMode( mpSlideController->getCurrentSlideIndex(), nColor ) )
        {
            pause();
        }
    }
}

// ---------------------------------------------------------
// XShapeEventListener
// ---------------------------------------------------------

void SlideshowImpl::click( const Reference< XShape >& xShape, const ::com::sun::star::awt::MouseEvent& /* aOriginalEvent */ )
{
    SolarMutexGuard aSolarGuard;

    WrappedShapeEventImplPtr pEvent = maShapeEventMap[xShape];
    if( !pEvent.get() )
        return;

    switch( pEvent->meClickAction )
    {
    case ClickAction_PREVPAGE:          gotoPreviousSlide();        break;
    case ClickAction_NEXTPAGE:          gotoNextSlide();            break;
    case ClickAction_FIRSTPAGE:         gotoFirstSlide();           break;
    case ClickAction_LASTPAGE:          gotoLastSlide();            break;
    case ClickAction_STOPPRESENTATION:  endPresentation();          break;
    case ClickAction_BOOKMARK:
    {
        gotoBookmark( pEvent->maStrBookmark );
    }
    break;
    case ClickAction_SOUND:
    {
        try
        {
            mxPlayer.set(avmedia::MediaWindow::createPlayer(pEvent->maStrBookmark), uno::UNO_QUERY_THROW );
            mxPlayer->start();
        }
        catch( uno::Exception& )
        {
            OSL_FAIL("sd::SlideshowImpl::click(), exception caught!" );
        }
    }
    break;

    case ClickAction_DOCUMENT:
    {
        OUString aBookmark( pEvent->maStrBookmark );

        sal_Int32 nPos = aBookmark.indexOf( sal_Unicode('#') );
        if( nPos >= 0 )
        {
            OUString aURL( aBookmark.copy( 0, nPos+1 ) );
            OUString aName( aBookmark.copy( nPos+1 ) );
            aURL += getUiNameFromPageApiNameImpl( aName );
            aBookmark = aURL;
        }

        mpDocSh->OpenBookmark( aBookmark );
    }
    break;

    case ClickAction_PROGRAM:
    {
        INetURLObject aURL(
            ::URIHelper::SmartRel2Abs(
                INetURLObject(mpDocSh->GetMedium()->GetBaseURL()),
                pEvent->maStrBookmark, ::URIHelper::GetMaybeFileHdl(), true,
                false, INetURLObject::WAS_ENCODED,
                INetURLObject::DECODE_UNAMBIGUOUS ) );

        if( INET_PROT_FILE == aURL.GetProtocol() )
        {
            SfxStringItem aUrl( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            SfxBoolItem aBrowsing( SID_BROWSE, sal_True );

            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if (pViewFrm)
                pViewFrm->GetDispatcher()->Execute( SID_OPENDOC,
                                              SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                            &aUrl,
                                            &aBrowsing,
                                            0L );
        }
    }
    break;

    case presentation::ClickAction_MACRO:
    {
        const String aMacro( pEvent->maStrBookmark );

        if ( SfxApplication::IsXScriptURL( aMacro ) )
        {
            Any aRet;
            Sequence< sal_Int16 > aOutArgsIndex;
            Sequence< Any > aOutArgs;
            Sequence< Any >* pInArgs = new Sequence< Any >(0);
            mpDocSh->CallXScript( aMacro, *pInArgs, aRet, aOutArgsIndex, aOutArgs);
        }
        else
        {
            // aMacro has the following syntax:
            // "Macroname.Modulname.Libname.Dokumentname" or
            // "Macroname.Modulname.Libname.Applikationsname"
            String aMacroName = aMacro.GetToken(0, sal_Unicode('.'));
            String aModulName = aMacro.GetToken(1, sal_Unicode('.'));
            String aLibName   = aMacro.GetToken(2, sal_Unicode('.'));
            String aDocName   = aMacro.GetToken(3, sal_Unicode('.'));

            // todo: is the limitation still given that only
            // Modulname+Macroname can be used here?
            String aExecMacro(aModulName);
            aExecMacro.Append( sal_Unicode('.') );
            aExecMacro.Append( aMacroName );
            mpDocSh->GetBasic()->Call(aExecMacro);
        }
    }
    break;

    case ClickAction_VERB:
    {
        // todo, better do it async?
        SdrObject* pObj = GetSdrObjectFromXShape( xShape );
        SdrOle2Obj* pOleObject = PTR_CAST(SdrOle2Obj, pObj);
        if (pOleObject && mpViewShell )
            mpViewShell->ActivateObject(pOleObject, pEvent->mnVerb);
    }
    break;
    default:
        break;
    }
}

// ---------------------------------------------------------

sal_Int32 SlideshowImpl::getSlideNumberForBookmark( const OUString& rStrBookmark )
{
    sal_Bool bIsMasterPage;
    OUString aBookmark = getUiNameFromPageApiNameImpl( rStrBookmark );
    sal_uInt16 nPgNum = mpDoc->GetPageByName( aBookmark, bIsMasterPage );

    if( nPgNum == SDRPAGE_NOTFOUND )
    {
        // Is the bookmark an object?
        SdrObject* pObj = mpDoc->GetObj( aBookmark );

        if( pObj )
        {
            nPgNum = pObj->GetPage()->GetPageNum();
            bIsMasterPage = (sal_Bool)pObj->GetPage()->IsMasterPage();
        }
    }

    if( (nPgNum == SDRPAGE_NOTFOUND) || bIsMasterPage || static_cast<SdPage*>(mpDoc->GetPage(nPgNum))->GetPageKind() != PK_STANDARD )
        return -1;

    return ( nPgNum - 1) >> 1;
}

// ---------------------------------------------------------

void SlideshowImpl::hyperLinkClicked( rtl::OUString const& aHyperLink ) throw (RuntimeException)
{
    OUString aBookmark( aHyperLink );

    sal_Int32 nPos = aBookmark.indexOf( sal_Unicode('#') );
    if( nPos >= 0 )
    {
        OUString aURL( aBookmark.copy( 0, nPos+1 ) );
        OUString aName( aBookmark.copy( nPos+1 ) );
        aURL += getUiNameFromPageApiNameImpl( aName );
        aBookmark = aURL;
    }

    mpDocSh->OpenBookmark( aBookmark );
}

// ---------------------------------------------------------

void SlideshowImpl::displaySlideNumber( sal_Int32 nSlideNumber )
{
    if( mpSlideController.get() )
    {
        if( mpSlideController->jumpToSlideNumber( nSlideNumber ) )
        {
            displayCurrentSlide();
        }
    }
}

// ---------------------------------------------------------

/** nSlideIndex == -1 displays current slide again */
void SlideshowImpl::displaySlideIndex( sal_Int32 nSlideIndex )
{
    if( mpSlideController.get() )
    {
        if( (nSlideIndex == -1) || mpSlideController->jumpToSlideIndex( nSlideIndex ) )
        {
            displayCurrentSlide();
        }
    }
}

// ---------------------------------------------------------

void SlideshowImpl::jumpToBookmark( const String& sBookmark )
{
    sal_Int32 nSlideNumber = getSlideNumberForBookmark( sBookmark );
    if( nSlideNumber != -1 )
        displaySlideNumber( nSlideNumber );
}

// ---------------------------------------------------------

sal_Int32 SlideshowImpl::getCurrentSlideNumber()
{
    return mpSlideController.get() ? mpSlideController->getCurrentSlideNumber() : -1;
}

// ---------------------------------------------------------

sal_Int32 SlideshowImpl::getFirstSlideNumber()
{
    sal_Int32 nRet = 0;
    if( mpSlideController.get() )
    {
        sal_Int32 nSlideIndexCount = mpSlideController->getSlideIndexCount() - 1;
        if( nSlideIndexCount >= 0 )
        {
            nRet = mpSlideController->getSlideNumber( nSlideIndexCount );
            while( nSlideIndexCount-- )
            {
                sal_Int32 nTemp = mpSlideController->getSlideNumber( nSlideIndexCount );
                if( nRet > nTemp )
                    nRet = nTemp;
            }
        }
    }

    return nRet;
}

// ---------------------------------------------------------

sal_Int32 SlideshowImpl::getLastSlideNumber()
{
    sal_Int32 nRet = 0;
    if( mpSlideController.get() )
    {
        sal_Int32 nSlideIndexCount = mpSlideController->getSlideIndexCount() - 1;
        if( nSlideIndexCount >= 0 )
        {
            nRet = mpSlideController->getSlideNumber( nSlideIndexCount );
            while( nSlideIndexCount-- )
            {
                sal_Int32 nTemp = mpSlideController->getSlideNumber( nSlideIndexCount );
                if( nRet < nTemp )
                    nRet = nTemp;
            }
        }
    }

    return nRet;
}

// ---------------------------------------------------------

sal_Bool SAL_CALL SlideshowImpl::isEndless() throw( RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    return maPresSettings.mbEndless;
}

// ---------------------------------------------------------

double SlideshowImpl::update()
{
    startUpdateTimer();
    return -1;
}

// ---------------------------------------------------------

void SlideshowImpl::startUpdateTimer()
{
    SolarMutexGuard aSolarGuard;
    maUpdateTimer.SetTimeout( 0 );
    maUpdateTimer.Start();
}

// ---------------------------------------------------------

/** this timer is called 20ms after a new slide was displayed.
    This is used to unfreeze user input that was disabled after
    slide change to skip input that was buffered during slide
    transition preperation */
IMPL_LINK_NOARG(SlideshowImpl, ReadyForNextInputHdl)
{
    mbInputFreeze = false;
    return 0;
}

// ---------------------------------------------------------

/** if I catch someone someday who calls this method by hand
    and not by using the timer, I will personaly punish this
    person seriously, even if this person is me.
*/
IMPL_LINK_NOARG(SlideshowImpl, updateHdl)
{
    mnUpdateEvent = 0;

    return updateSlideShow();
}




IMPL_LINK_NOARG(SlideshowImpl, PostYieldListener)
{
    // prevent me from deletion when recursing (App::Reschedule does)
    const rtl::Reference<SlideshowImpl> this_(this);

    Application::EnableNoYieldMode(false);
    Application::RemovePostYieldListener(LINK(this, SlideshowImpl, PostYieldListener));
    Application::Reschedule(true); // fix for fdo#32861 - process
                                   // *all* outstanding events after
                                   // yield is done.
    if (mbDisposed)
        return 0;
    return updateSlideShow();
}




sal_Int32 SlideshowImpl::updateSlideShow (void)
{
    // prevent me from deletion when recursing (App::EnableYieldMode does)
    const rtl::Reference<SlideshowImpl> this_(this);

    Reference< XSlideShow > xShow( mxShow );
    if ( ! xShow.is())
        return 0;

    try
    {
        // TODO(Q3): Evaluate under various systems and setups,
        // whether this is really necessary. Under WinXP and Matrox
        // G550, the frame rates were much more steadier with this
        // tweak, although.

        // currently no solution, because this kills sound (at least on Windows)

        double fUpdate = 0.0;
        if( !xShow->update(fUpdate) )
            fUpdate = -1.0;

        if (mxShow.is() && (fUpdate >= 0.0))
        {
            if (::basegfx::fTools::equalZero(fUpdate))
            {
                // Use post yield listener for short update intervalls.
                Application::EnableNoYieldMode(true);
                Application::AddPostYieldListener(LINK(this, SlideshowImpl, PostYieldListener));
            }
            else
            {
                // Avoid busy loop when the previous call to update()
                // returns a small positive number but not 0 (which is
                // handled above).  Also, make sure that calls to update()
                // have a minimum frequency.
                // => Allow up to 60 frames per second.  Call at least once
                // every 4 seconds.
                const static sal_Int32 mnMaximumFrameCount (60);
                const static double mnMinimumTimeout (1.0 / mnMaximumFrameCount);
                const static double mnMaximumTimeout (4.0);
                fUpdate = ::basegfx::clamp(fUpdate, mnMinimumTimeout, mnMaximumTimeout);

                // Make sure that the maximum frame count has not been set
                // too high (only then conversion to milliseconds and long
                // integer may lead to zero value.)
                OSL_ASSERT(static_cast<sal_uLong>(fUpdate * 1000.0) > 0);

                Application::EnableNoYieldMode(false);
                Application::RemovePostYieldListener(LINK(this, SlideshowImpl, PostYieldListener));

                // Use a timer for the asynchronous callback.
                maUpdateTimer.SetTimeout(static_cast<sal_uLong>(fUpdate * 1000.0));
                maUpdateTimer.Start();
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::updateSlideShow(), exception caught: ")
                + rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
    }
    return 0;
}

// ---------------------------------------------------------

bool SlideshowImpl::keyInput(const KeyEvent& rKEvt)
{
    if( !mxShow.is() || mbInputFreeze )
        return false;

    bool bRet = true;

    try
    {
        const int nKeyCode = rKEvt.GetKeyCode().GetCode();
        switch( nKeyCode )
        {
            case awt::Key::CONTEXTMENU:
                if( !mnContextMenuEvent )
                {
                    if( mpShowWindow )
                        maPopupMousePos = mpShowWindow->GetPointerState().maPos;
                    mnContextMenuEvent = Application::PostUserEvent( LINK( this, SlideshowImpl, ContextMenuHdl ) );
                }
                break;

            // cancel show
            case KEY_ESCAPE:
            case KEY_SUBTRACT:
                // in case the user cancels the presentation, switch to current slide
                // in edit mode
                if( mpSlideController.get() && (ANIMATIONMODE_SHOW == meAnimationMode) )
                {
                    if( mpSlideController->getCurrentSlideNumber() != -1 )
                        mnRestoreSlide = mpSlideController->getCurrentSlideNumber();
                }
                endPresentation();
                break;

            // advance show
            case KEY_PAGEDOWN:
                if(rKEvt.GetKeyCode().IsMod2())
                {
                    gotoNextSlide();
                    break;
                }
                // warning, fall through!
            case KEY_SPACE:
            case KEY_RIGHT:
            case KEY_DOWN:
            case KEY_N:
                gotoNextEffect();
                break;

            case KEY_RETURN:
            {
                if( maCharBuffer.Len() )
                {
                    if( mpSlideController.get() )
                    {
                        if( mpSlideController->jumpToSlideNumber( maCharBuffer.ToInt32() - 1 ) )
                            displayCurrentSlide();
                    }
                    maCharBuffer.Erase();
                }
                else
                {
                    gotoNextEffect();
                }
            }
            break;

            // numeric: add to buffer
            case KEY_0:
            case KEY_1:
            case KEY_2:
            case KEY_3:
            case KEY_4:
            case KEY_5:
            case KEY_6:
            case KEY_7:
            case KEY_8:
            case KEY_9:
                maCharBuffer.Append( rKEvt.GetCharCode() );
                break;

            case KEY_PAGEUP:
                if(rKEvt.GetKeyCode().IsMod2())
                {
                    gotoPreviousSlide();
                    break;
                }
                // warning, fall through!
            case KEY_LEFT:
            case KEY_UP:
            case KEY_P:
            case KEY_BACKSPACE:
                gotoPreviousEffect();
                break;

            case KEY_HOME:
                gotoFirstSlide();
                break;

            case KEY_END:
                gotoLastSlide();
                break;

            case KEY_B:
            case KEY_W:
            case KEY_POINT:
            case KEY_COMMA:
            {
                blankScreen( ((nKeyCode == KEY_W ) || (nKeyCode == KEY_COMMA)) ? 0x00ffffff : 0x00000000 );
            }
            break;

            default:
                bRet = false;
            break;
        }
    }
    catch( Exception& )
    {
        bRet = false;
        OSL_FAIL(
            (OString("sd::SlideshowImpl::keyInput(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    return bRet;
}

IMPL_LINK( SlideshowImpl, EventListenerHdl, VclSimpleEvent*, pEvent )
{
    if( !mxShow.is() || mbInputFreeze )
        return 0;

    if( pEvent && (pEvent->GetId() == VCLEVENT_WINDOW_COMMAND) && static_cast<VclWindowEvent*>(pEvent)->GetData() )
    {
        const CommandEvent& rEvent = *(const CommandEvent*)static_cast<VclWindowEvent*>(pEvent)->GetData();

        if( rEvent.GetCommand() == COMMAND_MEDIA )
        {
            switch( rEvent.GetMediaCommand() )
            {
#if defined( QUARTZ )
            case MEDIA_COMMAND_MENU:
                if( !mnContextMenuEvent )
                {
                if( mpShowWindow )
                    maPopupMousePos = mpShowWindow->GetPointerState().maPos;
                mnContextMenuEvent = Application::PostUserEvent( LINK( this, SlideshowImpl, ContextMenuHdl ) );
                }
                break;
            case MEDIA_COMMAND_VOLUME_DOWN:
                gotoPreviousSlide();
                break;
            case MEDIA_COMMAND_VOLUME_UP:
                gotoNextEffect();
                break;
#endif
            case MEDIA_COMMAND_NEXTTRACK:
                gotoNextEffect();
                break;
            case MEDIA_COMMAND_PAUSE:
                if( !mbIsPaused )
                    blankScreen(0);
                break;
            case MEDIA_COMMAND_PLAY:
                if( mbIsPaused )
                    resume();
                break;

            case MEDIA_COMMAND_PLAY_PAUSE:
                if( mbIsPaused )
                    resume();
                else
                    blankScreen(0);
                break;
            case MEDIA_COMMAND_PREVIOUSTRACK:
                gotoPreviousSlide();
                break;
            case MEDIA_COMMAND_NEXTTRACK_HOLD:
                gotoLastSlide();
                break;

            case MEDIA_COMMAND_REWIND:
                gotoFirstSlide();
                break;
            case MEDIA_COMMAND_STOP:
                // in case the user cancels the presentation, switch to current slide
                // in edit mode
                if( mpSlideController.get() && (ANIMATIONMODE_SHOW == meAnimationMode) )
                {
                    if( mpSlideController->getCurrentSlideNumber() != -1 )
                        mnRestoreSlide = mpSlideController->getCurrentSlideNumber();
                }
                endPresentation();
                break;
            }
        }
    }

    return 0;
}

// ---------------------------------------------------------

void SlideshowImpl::mouseButtonUp(const MouseEvent& rMEvt)
{
    if( rMEvt.IsRight() && !mnContextMenuEvent )
    {
        maPopupMousePos = rMEvt.GetPosPixel();
        mnContextMenuEvent = Application::PostUserEvent( LINK( this, SlideshowImpl, ContextMenuHdl ) );
    }
}

// ---------------------------------------------------------

IMPL_LINK_NOARG(SlideshowImpl, ContextMenuHdl)
{
    mnContextMenuEvent = 0;

    if( mpSlideController.get() == 0 )
        return 0;

    mbWasPaused = mbIsPaused;
    if( !mbWasPaused )
        pause();

    PopupMenu* pMenu = new PopupMenu( SdResId( RID_SLIDESHOW_CONTEXTMENU ) );

    // Adding button to display if in Pen  mode
    pMenu->CheckItem( CM_PEN_MODE, mbUsePen);

    const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
    pMenu->EnableItem( CM_NEXT_SLIDE, ( mpSlideController->getNextSlideIndex() != -1 ) );
    pMenu->EnableItem( CM_PREV_SLIDE, ( mpSlideController->getPreviousSlideIndex() != -1 ) || (eMode == SHOWWINDOWMODE_END) || (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) );

    PopupMenu* pPageMenu = pMenu->GetPopupMenu( CM_GOTO );

    SfxViewFrame* pViewFrame = getViewFrame();
    if( pViewFrame )
    {
        Reference< ::com::sun::star::frame::XFrame > xFrame( pViewFrame->GetFrame().GetFrameInterface() );
        if( xFrame.is() )
        {
            pMenu->SetItemImage( CM_NEXT_SLIDE, GetImage( xFrame, "slot:10617" , sal_False ) );
            pMenu->SetItemImage( CM_PREV_SLIDE, GetImage( xFrame, "slot:10618" , sal_False ) );

            if( pPageMenu )
            {
                pPageMenu->SetItemImage( CM_FIRST_SLIDE, GetImage( xFrame, "slot:10616" , sal_False ) );
                pPageMenu->SetItemImage( CM_LAST_SLIDE, GetImage( xFrame, "slot:10619" , sal_False ) );
            }
        }
    }

    // populate slide goto list
    if( pPageMenu )
    {
        const sal_Int32 nPageNumberCount = mpSlideController->getSlideNumberCount();
        if( nPageNumberCount <= 1 )
        {
            pMenu->EnableItem( CM_GOTO, sal_False );
        }
        else
        {
            sal_Int32 nCurrentSlideNumber = mpSlideController->getCurrentSlideNumber();
            if( (eMode == SHOWWINDOWMODE_END) || (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) )
                nCurrentSlideNumber = -1;

            pPageMenu->EnableItem( CM_FIRST_SLIDE, ( mpSlideController->getSlideNumber(0) != nCurrentSlideNumber ) );
            pPageMenu->EnableItem( CM_LAST_SLIDE, ( mpSlideController->getSlideNumber( mpSlideController->getSlideIndexCount() - 1) != nCurrentSlideNumber ) );

            sal_Int32 nPageNumber;

            for( nPageNumber = 0; nPageNumber < nPageNumberCount; nPageNumber++ )
            {
                if( mpSlideController->isVisibleSlideNumber( nPageNumber ) )
                {
                    SdPage* pPage = mpDoc->GetSdPage((sal_uInt16)nPageNumber, PK_STANDARD);
                    if (pPage)
                    {
                        pPageMenu->InsertItem( (sal_uInt16)(CM_SLIDES + nPageNumber), pPage->GetName() );
                        if( nPageNumber == nCurrentSlideNumber )
                            pPageMenu->CheckItem( (sal_uInt16)(CM_SLIDES + nPageNumber) );
                    }
                }
            }
        }
    }

    if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_BLANK )
    {
        PopupMenu* pBlankMenu = pMenu->GetPopupMenu( CM_SCREEN );
        if( pBlankMenu )
        {
            pBlankMenu->CheckItem( ( mpShowWindow->GetBlankColor() == Color( COL_WHITE ) ) ? CM_SCREEN_WHITE : CM_SCREEN_BLACK  );
        }
    }

    PopupMenu* pWidthMenu = pMenu->GetPopupMenu( CM_WIDTH_PEN);

    // populate color width list
    if( pWidthMenu )
    {
        sal_Int32 nIterator;
        double nWidth;

        nWidth = 4.0;
        for( nIterator = 1; nIterator < 6; nIterator++)
        {
            switch(nIterator)
            {
                case 1:
                    nWidth = 4.0;
                    break;
                case 2:
                    nWidth = 100.0;
                    break;
                case 3:
                    nWidth = 150.0;
                    break;
                case 4:
                    nWidth = 200.0;
                    break;
                case 5:
                    nWidth = 400.0;
                    break;
                default:
                    break;
            }

            pWidthMenu->EnableItem( (sal_uInt16)(CM_WIDTH_PEN + nIterator), sal_True);
            if( nWidth ==  mdUserPaintStrokeWidth)
                pWidthMenu->CheckItem( (sal_uInt16)(CM_WIDTH_PEN + nIterator) );
        }
    }

    pMenu->SetSelectHdl( LINK( this, SlideshowImpl, ContextMenuSelectHdl ) );
    pMenu->Execute( mpShowWindow, maPopupMousePos );
    delete pMenu;

    if( mxView.is() )
        mxView->ignoreNextMouseReleased();

    if( !mbWasPaused )
        resume();
    return 0;
}

// ---------------------------------------------------------

IMPL_LINK( SlideshowImpl, ContextMenuSelectHdl, Menu *, pMenu )
{
    if( pMenu )
    {
        sal_uInt16 nMenuId = pMenu->GetCurItemId();

        switch( nMenuId )
        {
        case CM_PREV_SLIDE:
            gotoPreviousSlide();
            mbWasPaused = false;
            break;
        case CM_NEXT_SLIDE:
            gotoNextSlide();
            mbWasPaused = false;
            break;
        case CM_FIRST_SLIDE:
            gotoFirstSlide();
            mbWasPaused = false;
            break;
        case CM_LAST_SLIDE:
            gotoLastSlide();
            mbWasPaused = false;
            break;
        case CM_SCREEN_BLACK:
        case CM_SCREEN_WHITE:
        {
            const Color aBlankColor( (nMenuId == CM_SCREEN_WHITE) ? COL_WHITE : COL_BLACK );
            if( mbWasPaused )
            {
                if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_BLANK )
                {
                    if( mpShowWindow->GetBlankColor() == aBlankColor )
                    {
                        mbWasPaused = false;
                        mpShowWindow->RestartShow();
                        break;
                    }
                }
                mpShowWindow->RestartShow();
            }
            if( mpShowWindow->SetBlankMode( mpSlideController->getCurrentSlideIndex(), aBlankColor ) )
            {
                pause();
                mbWasPaused = true;
            }
        }
        break;
        case CM_COLOR_PEN:
            {
                //Open a color picker based on SvColorDialog
                ::Color aColor( mnUserPaintColor );
                SvColorDialog aColorDlg( mpShowWindow);
                aColorDlg.SetColor( aColor );

                if (aColorDlg.Execute() )
                {
                    aColor = aColorDlg.GetColor();
                    setPenColor(aColor.GetColor());
                }
                mbWasPaused = false;
            }
            break;

        case CM_WIDTH_PEN_VERY_THIN:
            {
                setPenWidth(4.0);
                mbWasPaused = false;
            }
            break;

        case CM_WIDTH_PEN_THIN:
            {
                setPenWidth(100.0);
                mbWasPaused = false;
            }
            break;

        case CM_WIDTH_PEN_NORMAL:
            {
                setPenWidth(150.0);
                mbWasPaused = false;
            }
            break;

        case CM_WIDTH_PEN_THICK:
            {
                setPenWidth(200.0);
                mbWasPaused = false;
            }
            break;

        case CM_WIDTH_PEN_VERY_THICK:
            {
                setPenWidth(400.0);
                mbWasPaused = false;
            }
            break;
        case CM_ERASE_ALLINK:
            {
                setEraseAllInk(true);
                    mbWasPaused = false;
            }
            break;
        case CM_PEN_MODE:
            {
                setUsePen(!mbUsePen);
                mbWasPaused = false;
            }
            break;
        case CM_ENDSHOW:
            // in case the user cancels the presentation, switch to current slide
            // in edit mode
            if( mpSlideController.get() && (ANIMATIONMODE_SHOW == meAnimationMode) )
            {
                if( mpSlideController->getCurrentSlideNumber() != -1 )
                {
                    mnRestoreSlide = mpSlideController->getCurrentSlideNumber();
                }
            }
            endPresentation();
            break;
        default:
            sal_Int32 nPageNumber = nMenuId - CM_SLIDES;
            const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
            if( (eMode == SHOWWINDOWMODE_END) || (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) )
            {
                mpShowWindow->RestartShow( nPageNumber );
            }
            else if( nPageNumber != mpSlideController->getCurrentSlideNumber() )
            {
                displaySlideNumber( nPageNumber );
            }
            mbWasPaused = false;
            break;
        }
    }

    return 0;
}

// ---------------------------------------------------------

Reference< XSlideShow > SlideshowImpl::createSlideShow() const
{
    Reference< XSlideShow > xShow;

    try
    {
        Reference< lang::XMultiServiceFactory > xFactory(
            ::comphelper::getProcessServiceFactory(),
            UNO_QUERY_THROW );

        Reference< XInterface > xInt( xFactory->createInstance( "com.sun.star.presentation.SlideShow" ) );

        xShow.set( xInt, UNO_QUERY_THROW );
    }
    catch( uno::Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::createSlideShow(), "
                     "exception caught: ") +
             rtl::OUStringToOString(
                 comphelper::anyToString( cppu::getCaughtException() ),
                 RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    return xShow;
}

// ---------------------------------------------------------

void SlideshowImpl::createSlideList( bool bAll, bool bStartWithActualSlide, const String& rPresSlide )
{
    const long nSlideCount = mpDoc->GetSdPageCount( PK_STANDARD );

    if( nSlideCount )
    {
        SdCustomShow*   pCustomShow;

        if( !bStartWithActualSlide && mpDoc->GetCustomShowList() && maPresSettings.mbCustomShow )
            pCustomShow = mpDoc->GetCustomShowList()->GetCurObject();
        else
            pCustomShow = NULL;

        // create animation slide controller
        AnimationSlideController::Mode eMode =
            ( pCustomShow && pCustomShow->PagesVector().size() ) ? AnimationSlideController::CUSTOM :
                (bAll ? AnimationSlideController::ALL : AnimationSlideController::FROM);

        Reference< XDrawPagesSupplier > xDrawPages( mpDoc->getUnoModel(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xSlides( xDrawPages->getDrawPages(), UNO_QUERY_THROW );
        mpSlideController.reset( new AnimationSlideController( xSlides, eMode ) );

        if( eMode != AnimationSlideController::CUSTOM )
        {
            sal_Int32 nFirstSlide = 0;

            // normale Praesentation
            if( eMode == AnimationSlideController::FROM )
            {
                if( rPresSlide.Len() )
                {
                    sal_Int32 nSlide;
                    sal_Bool bTakeNextAvailable = sal_False;

                    for( nSlide = 0, nFirstSlide = -1; ( nSlide < nSlideCount ) && ( -1 == nFirstSlide ); nSlide++ )
                    {
                        SdPage* pTestSlide = mpDoc->GetSdPage( (sal_uInt16)nSlide, PK_STANDARD );

                        if( pTestSlide->GetName() == rPresSlide )
                        {
                            if( pTestSlide->IsExcluded() )
                                bTakeNextAvailable = sal_True;
                            else
                                nFirstSlide = nSlide;
                        }
                        else if( bTakeNextAvailable && !pTestSlide->IsExcluded() )
                            nFirstSlide = nSlide;
                    }

                    if( -1 == nFirstSlide )
                        nFirstSlide = 0;
                }
            }

            for( sal_Int32 i = 0; i < nSlideCount; i++ )
            {
                bool bVisible = ( mpDoc->GetSdPage( (sal_uInt16)i, PK_STANDARD ) )->IsExcluded() ? false : true;
                if( bVisible || (eMode == AnimationSlideController::ALL) )
                    mpSlideController->insertSlideNumber( i, bVisible );
            }

            mpSlideController->setStartSlideNumber( nFirstSlide );
        }
        else
        {
            if( meAnimationMode != ANIMATIONMODE_SHOW && rPresSlide.Len() )
            {
                sal_Int32 nSlide;
                for( nSlide = 0; nSlide < nSlideCount; nSlide++ )
                    if( rPresSlide == mpDoc->GetSdPage( (sal_uInt16) nSlide, PK_STANDARD )->GetName() )
                        break;

                if( nSlide < nSlideCount )
                    mpSlideController->insertSlideNumber( (sal_uInt16) nSlide );
            }

            sal_Int32 nSlideIndex = 0;
            for( SdCustomShow::PageVec::iterator it = pCustomShow->PagesVector().begin();
                 it != pCustomShow->PagesVector().end(); ++it, nSlideIndex++ )
            {
                const sal_uInt16 nSdSlide = ( ( (SdPage*) (*it) )->GetPageNum() - 1 ) / 2;

                if( !( mpDoc->GetSdPage( nSdSlide, PK_STANDARD ) )->IsExcluded())
                    mpSlideController->insertSlideNumber( nSdSlide );
            }
        }
    }
}

// ---------------------------------------------------------

typedef sal_uInt16 (*FncGetChildWindowId)();

FncGetChildWindowId aShowChildren[] =
{
    &AnimationChildWindow::GetChildWindowId,
    &Svx3DChildWindow::GetChildWindowId,
    &SvxFontWorkChildWindow::GetChildWindowId,
    &SvxColorChildWindow::GetChildWindowId,
    &SvxSearchDialogWrapper::GetChildWindowId,
    &SvxBmpMaskChildWindow::GetChildWindowId,
    &SvxIMapDlgChildWindow::GetChildWindowId,
    &SvxHlinkDlgWrapper::GetChildWindowId,
    &SfxTemplateDialogWrapper::GetChildWindowId,
    &GalleryChildWindow::GetChildWindowId,
    &SfxInfoBarContainerChild::GetChildWindowId
};

#define NAVIGATOR_CHILD_MASK        0x80000000UL

void SlideshowImpl::hideChildWindows()
{
    mnChildMask = 0UL;

    if( ANIMATIONMODE_SHOW == meAnimationMode )
    {
        SfxViewFrame* pViewFrame = getViewFrame();

        if( pViewFrame )
        {
            if( pViewFrame->GetChildWindow( SID_NAVIGATOR ) != NULL )
                mnChildMask |= NAVIGATOR_CHILD_MASK;

            for( sal_uLong i = 0, nCount = sizeof( aShowChildren ) / sizeof( FncGetChildWindowId ); i < nCount; i++ )
            {
                const sal_uInt16 nId = ( *aShowChildren[ i ] )();

                if( pViewFrame->GetChildWindow( nId ) )
                {
                    pViewFrame->SetChildWindow( nId, sal_False );
                    mnChildMask |= 1 << i;
                }
            }
        }
    }
}

// ---------------------------------------------------------

void SlideshowImpl::showChildWindows()
{
    if( ANIMATIONMODE_SHOW == meAnimationMode )
    {
        SfxViewFrame* pViewFrame = getViewFrame();
        if( pViewFrame )
        {
            pViewFrame->SetChildWindow( SID_NAVIGATOR, ( mnChildMask & NAVIGATOR_CHILD_MASK ) != 0 );

            for( sal_uLong i = 0, nCount = sizeof( aShowChildren ) / sizeof( FncGetChildWindowId ); i < nCount; i++ )
            {
                if( mnChildMask & ( 1 << i ) )
                    pViewFrame->SetChildWindow( ( *aShowChildren[ i ] )(), sal_True );
            }
        }
    }
}

// ---------------------------------------------------------

SfxViewFrame* SlideshowImpl::getViewFrame() const
{
    return mpViewShell ? mpViewShell->GetViewFrame() : 0;
}

// ---------------------------------------------------------

SfxDispatcher* SlideshowImpl::getDispatcher() const
{
    return (mpViewShell && mpViewShell->GetViewFrame()) ? mpViewShell->GetViewFrame()->GetDispatcher() : 0;
}

// ---------------------------------------------------------

SfxBindings* SlideshowImpl::getBindings() const
{
    return (mpViewShell && mpViewShell->GetViewFrame()) ? &mpViewShell->GetViewFrame()->GetBindings() : 0;
}

// ---------------------------------------------------------

void SlideshowImpl::resize( const Size& rSize )
{
    maPresSize = rSize;

    if( mpShowWindow && (ANIMATIONMODE_VIEW != meAnimationMode) )
    {
        mpShowWindow->SetSizePixel( maPresSize );
        mpShowWindow->Show();
    }

    if( mxView.is() ) try
    {
        awt::WindowEvent aEvt;
        mxView->windowResized(aEvt);
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::resize(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// -----------------------------------------------------------------------------

void SlideshowImpl::setActiveXToolbarsVisible( sal_Bool bVisible )
{
    // in case of ActiveX control the toolbars should not be visible if slide show runs in window mode
    // actually it runs always in window mode in case of ActiveX control
    if ( !maPresSettings.mbFullScreen && mpDocSh && mpDocSh->GetMedium() )
    {
        SFX_ITEMSET_ARG( mpDocSh->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_VIEWONLY, sal_False );
        if ( pItem && pItem->GetValue() )
        {
            // this is a plugin/activex mode, no toolbars should be visible during slide show
            // after the end of slide show they should be visible again
            SfxViewFrame* pViewFrame = getViewFrame();
            if( pViewFrame )
            {
                try
                {
                    Reference< frame::XLayoutManager > xLayoutManager;
                    Reference< beans::XPropertySet > xFrameProps( pViewFrame->GetFrame().GetTopFrame().GetFrameInterface(), UNO_QUERY_THROW );
                    if ( ( xFrameProps->getPropertyValue( "LayoutManager" )
                                >>= xLayoutManager )
                      && xLayoutManager.is() )
                    {
                        xLayoutManager->setVisible( bVisible );
                    }
                }
                catch( uno::Exception& )
                {}
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL SlideshowImpl::activate() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    maDeactivateTimer.Stop();

    if( !mbActive && mxShow.is() )
    {
        mbActive = sal_True;

        if( ANIMATIONMODE_SHOW == meAnimationMode )
        {
            if( mbAutoSaveWasOn )
                setAutoSaveState( false );

            if( mpShowWindow )
            {
                SfxViewFrame* pViewFrame = getViewFrame();
                SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : 0;

                hideChildWindows();

                if( pDispatcher )
                {
                    // filter all forbiden slots
                    pDispatcher->SetSlotFilter( sal_True, sizeof(pAllowed) / sizeof(sal_uInt16), pAllowed );
                }

                if( getBindings() )
                    getBindings()->InvalidateAll(sal_True);

                mpShowWindow->GrabFocus();
            }
        }

        resume();
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL SlideshowImpl::deactivate() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mbActive && mxShow.is() )
    {
        maDeactivateTimer.Start();
    }
}

// -----------------------------------------------------------------------------

IMPL_LINK_NOARG(SlideshowImpl, deactivateHdl)
{
    if( mbActive && mxShow.is() )
    {
        mbActive = sal_False;

        pause();

        if( ANIMATIONMODE_SHOW == meAnimationMode )
        {
            if( mbAutoSaveWasOn )
                setAutoSaveState( true );

            if( mpShowWindow )
            {
                showChildWindows();
            }
        }
    }
    return 0;
}

// ---------------------------------------------------------

sal_Bool SAL_CALL SlideshowImpl::isActive() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mbActive;
}

// -----------------------------------------------------------------------------

void SlideshowImpl::receiveRequest(SfxRequest& rReq)
{
    const SfxItemSet* pArgs      = rReq.GetArgs();

    switch ( rReq.GetSlot() )
    {
        case SID_NAVIGATOR_PEN:
            setUsePen(!mbUsePen);
        break;

        case SID_NAVIGATOR_PAGE:
        {
            PageJump    eJump = (PageJump)((SfxAllEnumItem&) pArgs->Get(SID_NAVIGATOR_PAGE)).GetValue();
            switch( eJump )
            {
                case PAGE_FIRST:        gotoFirstSlide(); break;
                case PAGE_LAST:         gotoLastSlide(); break;
                case PAGE_NEXT:         gotoNextSlide(); break;
                case PAGE_PREVIOUS:     gotoPreviousSlide(); break;
                case PAGE_NONE:         break;
            }
        }
        break;

        case SID_NAVIGATOR_OBJECT:
        {
            const String aTarget( ((SfxStringItem&) pArgs->Get(SID_NAVIGATOR_OBJECT)).GetValue() );

            // is the bookmark a Slide?
            sal_Bool        bIsMasterPage;
            sal_uInt16      nPgNum = mpDoc->GetPageByName( aTarget, bIsMasterPage );
            SdrObject*  pObj   = NULL;

            if( nPgNum == SDRPAGE_NOTFOUND )
            {
                // is the bookmark an object?
                pObj = mpDoc->GetObj( aTarget );

                if( pObj )
                    nPgNum = pObj->GetPage()->GetPageNum();
            }

            if( nPgNum != SDRPAGE_NOTFOUND )
            {
                nPgNum = ( nPgNum - 1 ) >> 1;
                displaySlideNumber( nPgNum );
            }
        }
        break;
    }
}

// ---------------------------------------------------------

void SlideshowImpl::setAutoSaveState( bool bOn)
{
    try
    {
        uno::Reference<lang::XMultiServiceFactory> xFac( ::comphelper::getProcessServiceFactory() );
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

        uno::Reference< util::XURLTransformer > xParser(util::URLTransformer::create(xContext));
        util::URL aURL;
        aURL.Complete = "vnd.sun.star.autorecovery:/setAutoSaveState";
        xParser->parseStrict(aURL);

        Sequence< beans::PropertyValue > aArgs(1);
        aArgs[0].Name = "AutoSaveState";
        aArgs[0].Value <<= bOn ? sal_True : sal_False;

        uno::Reference< frame::XDispatch > xAutoSave(
            xFac->createInstance( "com.sun.star.frame.AutoRecovery" ),
            uno::UNO_QUERY_THROW);
        xAutoSave->dispatch(aURL, aArgs);
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::SlideshowImpl::setAutoSaveState(), exception caught!");
    }
}

// ---------------------------------------------------------

Reference< XDrawPage > SAL_CALL SlideshowImpl::getCurrentSlide() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XDrawPage > xSlide;
    if( mxShow.is() && mpSlideController.get() )
    {
        sal_Int32 nSlide = getCurrentSlideNumber();
        if( (nSlide >= 0) && (nSlide < mpSlideController->getSlideNumberCount() ) )
            xSlide = mpSlideController->getSlideByNumber( nSlide );
    }

    return xSlide;
}

// ---------------------------------------------------------

sal_Int32 SAL_CALL SlideshowImpl::getNextSlideIndex() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mxShow.is() )
    {
        return mpSlideController->getNextSlideIndex();
    }
    else
    {
        return -1;
    }
}

// ---------------------------------------------------------

sal_Int32 SAL_CALL SlideshowImpl::getCurrentSlideIndex() throw (RuntimeException)
{
    return mpSlideController.get() ? mpSlideController->getCurrentSlideIndex() : -1;
}

// --------------------------------------------------------------------
// ::com::sun::star::presentation::XSlideShowController:
// --------------------------------------------------------------------

::sal_Int32 SAL_CALL SlideshowImpl::getSlideCount() throw (RuntimeException)
{
    return mpSlideController.get() ? mpSlideController->getSlideIndexCount() : 0;
}

// --------------------------------------------------------------------

Reference< XDrawPage > SAL_CALL SlideshowImpl::getSlideByIndex(::sal_Int32 Index) throw (RuntimeException, css::lang::IndexOutOfBoundsException)
{
    if( (mpSlideController.get() == 0 ) || (Index < 0) || (Index >= mpSlideController->getSlideIndexCount() ) )
        throw IndexOutOfBoundsException();

    return mpSlideController->getSlideByNumber( mpSlideController->getSlideNumber( Index ) );
}

sal_Bool SAL_CALL SlideshowImpl::getAlwaysOnTop() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return maPresSettings.mbAlwaysOnTop;
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::setAlwaysOnTop( sal_Bool bAlways ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    if( maPresSettings.mbAlwaysOnTop != bAlways )
    {
        maPresSettings.mbAlwaysOnTop = bAlways;
        // todo, can this be changed while running?
    }
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL SlideshowImpl::isFullScreen() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return maPresSettings.mbFullScreen;
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL SlideshowImpl::getMouseVisible() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return maPresSettings.mbMouseVisible;
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::setMouseVisible( sal_Bool bVisible ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    if( maPresSettings.mbMouseVisible != bVisible )
    {
        maPresSettings.mbMouseVisible = bVisible;
        if( mpShowWindow )
            mpShowWindow->SetMouseAutoHide( !maPresSettings.mbMouseVisible );
    }
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL SlideshowImpl::getUsePen() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mbUsePen;
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::setUsePen( sal_Bool bMouseAsPen ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    mbUsePen = bMouseAsPen;
    if( mxShow.is() ) try
    {
        // For Pencolor;
        Any aValue;
        if( mbUsePen )
            aValue <<= mnUserPaintColor;
        beans::PropertyValue aPenProp;
        aPenProp.Name = "UserPaintColor";
        aPenProp.Value = aValue;
        mxShow->setProperty( aPenProp );

        //for StrokeWidth :
        if( mbUsePen )
        {
            beans::PropertyValue aPenPropWidth;
            aPenPropWidth.Name = "UserPaintStrokeWidth";
            aPenPropWidth.Value <<= mdUserPaintStrokeWidth;
            mxShow->setProperty( aPenPropWidth );

            // for Pen Mode
            beans::PropertyValue aPenPropSwitchPenMode;
            aPenPropSwitchPenMode.Name = "SwitchPenMode";
            aPenPropSwitchPenMode.Value <<= sal_True;
            mxShow->setProperty( aPenPropSwitchPenMode );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::setUsePen(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// --------------------------------------------------------------------

double SAL_CALL SlideshowImpl::getPenWidth() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mdUserPaintStrokeWidth;
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::setPenWidth( double dStrokeWidth ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    mdUserPaintStrokeWidth = dStrokeWidth;
    setUsePen( true ); // enable pen mode, update color and width
}

// --------------------------------------------------------------------

sal_Int32 SAL_CALL SlideshowImpl::getPenColor() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mnUserPaintColor;
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::setPenColor( sal_Int32 nColor ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    mnUserPaintColor = nColor;
    setUsePen( true ); // enable pen mode, update color
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::setUseEraser( ::sal_Bool /*_usepen*/ ) throw (css::uno::RuntimeException)
{
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::setPenMode( bool bSwitchPenMode ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    setUsePen( bSwitchPenMode ); // SwitchPen Mode

}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::setEraseAllInk(bool bEraseAllInk) throw (RuntimeException)
{
    if( bEraseAllInk )
    {
        SolarMutexGuard aSolarGuard;
        if( mxShow.is() ) try
        {
            beans::PropertyValue aPenPropEraseAllInk;
            aPenPropEraseAllInk.Name = "EraseAllInk";
            aPenPropEraseAllInk.Value <<= bEraseAllInk;
            mxShow->setProperty( aPenPropEraseAllInk );
        }
        catch( Exception& )
        {
            OSL_TRACE(
                (OString("sd::SlideshowImpl::setEraseAllInk(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }
}

void SAL_CALL SlideshowImpl::setEraseInk( sal_Int32 /*nEraseInkSize*/ ) throw (css::uno::RuntimeException)
{
}

void SAL_CALL SlideshowImpl::setEraserMode( bool /*bSwitchEraserMode*/ ) throw (css::uno::RuntimeException)
{
}

// --------------------------------------------------------------------
// XSlideShowController Methods
// --------------------------------------------------------------------

sal_Bool SAL_CALL SlideshowImpl::isRunning(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return mxShow.is();
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoNextEffect(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mxShow.is() && mpSlideController.get() && mpShowWindow )
    {
        if( mbIsPaused )
            resume();

        const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
        if( eMode == SHOWWINDOWMODE_END )
        {
            endPresentation();
        }
        else if( (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) )
        {
            mpShowWindow->RestartShow();
        }
        else
        {
            mxShow->nextEffect();
            update();
        }
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoPreviousEffect(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mxShow.is() && mpSlideController.get() && mpShowWindow )
    {
        if( mbIsPaused )
            resume();

        const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
        if( (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) )
        {
            mpShowWindow->RestartShow();
        }
        else
        {
            mxShow->previousEffect();
            update();
        }
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoFirstSlide(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mpShowWindow && mpSlideController.get() )
    {
        if( mbIsPaused )
            resume();

        if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_END )
        {
            if( mpSlideController->getSlideIndexCount() )
                mpShowWindow->RestartShow( 0);
        }
        else
        {
            displaySlideIndex( 0 );
        }
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoNextSlide(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mbIsPaused )
        resume();

    const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
    if( (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) )
    {
        mpShowWindow->RestartShow();
    }
    else
    {
        // if this is a show, ignore user inputs and
        // start 20ms timer to reenable inputs to fiter
        // buffered inputs during slide transition
        if( meAnimationMode == ANIMATIONMODE_SHOW )
        {
            mbInputFreeze = true;
            maInputFreezeTimer.Start();
        }

        if( mpSlideController.get() )
        {
            if( mpSlideController->nextSlide() )
            {
                displayCurrentSlide();
            }
            else
            {
                stopSound();

                if( meAnimationMode == ANIMATIONMODE_PREVIEW )
                {
                    endPresentation();
                }
                else if( maPresSettings.mbEndless )
                {
                    if( maPresSettings.mnPauseTimeout )
                    {
                        if( mpShowWindow )
                        {
                            Graphic aGraphic( SfxApplication::GetApplicationLogo().GetBitmapEx() );
                            mpShowWindow->SetPauseMode( 0, maPresSettings.mnPauseTimeout, &aGraphic );
                        }
                    }
                    else
                    {
                        displaySlideIndex( 0 );
                    }
                }
                else
                {
                    if( mpShowWindow )
                    {
                        mpShowWindow->SetEndMode();
                        if( !mpViewShell->GetDoc()->IsStartWithPresentation() )
                            pause();
                    }
                }
            }
        }
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoPreviousSlide(  ) throw (RuntimeException)
{
    gotoPreviousSlide(false);
}

void SlideshowImpl::gotoPreviousSlide (const bool bSkipAllMainSequenceEffects)
{
    SolarMutexGuard aSolarGuard;

    if( mxShow.is() && mpSlideController.get() ) try
    {
        if( mbIsPaused )
            resume();

        const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
        if( eMode == SHOWWINDOWMODE_END )
        {
            const sal_Int32 nLastSlideIndex = mpSlideController->getSlideIndexCount() - 1;
            if( nLastSlideIndex >= 0 )
                mpShowWindow->RestartShow( nLastSlideIndex );
        }
        else if( (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) )
        {
            mpShowWindow->RestartShow();
        }
        else
        {
            if( mpSlideController->previousSlide())
                displayCurrentSlide(bSkipAllMainSequenceEffects);
            else if (bSkipAllMainSequenceEffects)
            {
                // We could not go to the previous slide (probably because
                // the current slide is already the first one).  We still
                // have to call displayCurrentSlide because the calling
                // slideshow can not determine whether there is a previous
                // slide or not and has already prepared for a slide change.
                // This slide change has to be completed now, even when
                // changing to the same slide.
                // Note that in this special case we do NOT pass
                // bSkipAllMainSequenceEffects because we display the same
                // slide as before and do not want to show all its effects.
                displayCurrentSlide(false);
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::gotoPreviousSlide(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoLastSlide() throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mpSlideController.get() )
    {
        if( mbIsPaused )
            resume();

        const sal_Int32 nLastSlideIndex = mpSlideController->getSlideIndexCount() - 1;
        if( nLastSlideIndex >= 0 )
        {
            if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_END )
            {
                mpShowWindow->RestartShow( nLastSlideIndex );
            }
            else
            {
                displaySlideIndex( nLastSlideIndex );
            }
        }
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoBookmark( const OUString& rBookmark ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mbIsPaused )
        resume();

    sal_Int32 nSlideNumber = getSlideNumberForBookmark( rBookmark );
    if( nSlideNumber != -1 )
        displaySlideNumber( nSlideNumber );
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoSlide( const Reference< XDrawPage >& xSlide )
    throw(IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mpSlideController.get() && xSlide.is() )
    {
        if( mbIsPaused )
            resume();

        const sal_Int32 nSlideCount = mpSlideController->getSlideNumberCount();
        for( sal_Int32 nSlide = 0; nSlide < nSlideCount; nSlide++ )
        {
            if( mpSlideController->getSlideByNumber( nSlide ) == xSlide )
            {
                displaySlideNumber( nSlide );
            }
        }
    }
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::gotoSlideIndex( sal_Int32 nIndex ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    if( mbIsPaused )
        resume();

    displaySlideIndex( nIndex );
}

// --------------------------------------------------------------------

void SAL_CALL SlideshowImpl::stopSound(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    try
    {
        if( mxPlayer.is() )
        {
            mxPlayer->stop();
            mxPlayer.clear();
        }
    }
    catch( Exception& )
    {
        OSL_FAIL(
            (OString("sd::SlideshowImpl::stopSound(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

// --------------------------------------------------------------------
// XIndexAccess
// --------------------------------------------------------------------

::sal_Int32 SAL_CALL SlideshowImpl::getCount(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return getSlideCount();
}

// --------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL SlideshowImpl::getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    return Any( getSlideByIndex( Index ) );
}

// --------------------------------------------------------------------

::com::sun::star::uno::Type SAL_CALL SlideshowImpl::getElementType(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return XDrawPage::static_type();
}

// --------------------------------------------------------------------

::sal_Bool SAL_CALL SlideshowImpl::hasElements(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return getSlideCount() != 0;
}

// --------------------------------------------------------------------

Reference< XSlideShow > SAL_CALL SlideshowImpl::getSlideShow() throw (RuntimeException)
{
    return mxShow;
}

// --------------------------------------------------------------------


PresentationSettingsEx::PresentationSettingsEx( const PresentationSettingsEx& r )
: PresentationSettings( r )
, mbRehearseTimings(r.mbRehearseTimings)
, mbPreview(r.mbPreview)
, mpParentWindow( 0 )
{
}

PresentationSettingsEx::PresentationSettingsEx( PresentationSettings& r )
: PresentationSettings( r )
, mbRehearseTimings(sal_False)
, mbPreview(sal_False)
, mpParentWindow(0)
{
}

void PresentationSettingsEx::SetArguments( const Sequence< PropertyValue >& rArguments ) throw (IllegalArgumentException)
{
    sal_Int32 nArguments = rArguments.getLength();
    const PropertyValue* pValue = rArguments.getConstArray();

    while( nArguments-- )
    {
        SetPropertyValue( pValue->Name, pValue->Value );
        pValue++;
    }
}

void PresentationSettingsEx::SetPropertyValue( const OUString& rProperty, const Any& rValue ) throw (IllegalArgumentException)
{
    if ( rProperty == "RehearseTimings" )
    {
        if( rValue >>= mbRehearseTimings )
            return;
    }
    else if ( rProperty == "Preview" )
    {
        if( rValue >>= mbPreview )
            return;
    }
    else if ( rProperty == "AnimationNode" )
    {
        if( rValue >>= mxAnimationNode )
            return;
    }
    else if ( rProperty == "ParentWindow" )
    {
        Reference< XWindow > xWindow;
        if( rValue >>= xWindow )
        {
            mpParentWindow = xWindow.is() ? VCLUnoHelper::GetWindow( xWindow ) : 0;
            return;
        }
    }
    else if ( rProperty == "AllowAnimations" )
    {
        if( rValue >>= mbAnimationAllowed )
            return;
    }
    else if ( rProperty == "FirstPage" )
    {
        OUString aPresPage;
        if( rValue >>= aPresPage )
        {
            maPresPage = getUiNameFromPageApiNameImpl(aPresPage);
            mbCustomShow = sal_False;
            mbAll = sal_False;
            return;
        }
        else
        {
            if( rValue >>= mxStartPage )
                return;
        }
    }
    else if ( rProperty == "IsAlwaysOnTop" )
    {
        if( rValue >>= mbAlwaysOnTop )
            return;
    }
    else if ( rProperty == "IsAutomatic" )
    {
        if( rValue >>= mbManual )
            return;
    }
    else if ( rProperty == "IsEndless" )
    {
        if( rValue >>= mbEndless )
            return;
    }
    else if ( rProperty == "IsFullScreen" )
    {
        if( rValue >>= mbFullScreen )
            return;
    }
    else if ( rProperty == "IsMouseVisible" )
    {
        if( rValue >>= mbMouseVisible )
            return;
    }
    else if ( rProperty == "Pause" )
    {
        sal_Int32 nPause = -1;
        if( (rValue >>= nPause) && (nPause >= 0) )
        {
            mnPauseTimeout = nPause;
            return;
        }
    }
    else if ( rProperty == "StartWithNavigator" )
    {
        if( rValue >>= mbStartWithNavigator )
            return;
    }
    else if ( rProperty == "UsePen" )
    {
        if( rValue >>= mbMouseAsPen )
            return;
    }
    throw IllegalArgumentException();
}

////////////////////////////////

// ---------------------------------------------------------
// XAnimationListener
// ---------------------------------------------------------

SlideShowListenerProxy::SlideShowListenerProxy( const rtl::Reference< SlideshowImpl >& xController, const css::uno::Reference< css::presentation::XSlideShow >& xSlideShow )
: maListeners( m_aMutex )
, mxController( xController )
, mxSlideShow( xSlideShow )
{
}

// ---------------------------------------------------------

SlideShowListenerProxy::~SlideShowListenerProxy()
{
}

// ---------------------------------------------------------

void SlideShowListenerProxy::addAsSlideShowListener()
{
    if( mxSlideShow.is() )
    {
        Reference< XSlideShowListener > xSlideShowListener( this );
        mxSlideShow->addSlideShowListener( xSlideShowListener );
    }
}

// ---------------------------------------------------------

void SlideShowListenerProxy::removeAsSlideShowListener()
{
    if( mxSlideShow.is() )
    {
        Reference< XSlideShowListener > xSlideShowListener( this );
        mxSlideShow->removeSlideShowListener( xSlideShowListener );
    }
}

// ---------------------------------------------------------

void SlideShowListenerProxy::addShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape )
{
    if( mxSlideShow.is() )
    {
        Reference< XShapeEventListener > xListener( this );
        mxSlideShow->addShapeEventListener( xListener, xShape );
    }
}

// ---------------------------------------------------------

void SlideShowListenerProxy::removeShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape )
{
    if( mxSlideShow.is() )
    {
        Reference< XShapeEventListener > xListener( this );
        mxSlideShow->removeShapeEventListener( xListener, xShape );
    }
}

// ---------------------------------------------------------

void SlideShowListenerProxy::addSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& xListener )
{
    maListeners.addInterface(xListener);
}

// ---------------------------------------------------------

void SlideShowListenerProxy::removeSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& xListener )
{
    maListeners.removeInterface(xListener);
}

// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::beginEvent( const Reference< XAnimationNode >& xNode ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
        maListeners.forEach<XSlideShowListener>( boost::bind( &XAnimationListener::beginEvent, _1,  boost::cref(xNode) ));
}

// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::endEvent( const Reference< XAnimationNode >& xNode ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
        maListeners.forEach<XSlideShowListener>( boost::bind( &XAnimationListener::endEvent, _1, boost::cref(xNode) ));
}

// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::repeat( const Reference< XAnimationNode >& xNode, ::sal_Int32 nRepeat ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
        maListeners.forEach<XSlideShowListener>( boost::bind( &XAnimationListener::repeat, _1,  boost::cref(xNode), boost::cref(nRepeat) ));
}

// ---------------------------------------------------------
// ::com::sun::star::presentation::XSlideShowListener:
// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::paused(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
        maListeners.forEach<XSlideShowListener>( boost::mem_fn( &XSlideShowListener::paused ) );
}

// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::resumed(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
        maListeners.forEach<XSlideShowListener>( boost::mem_fn( &XSlideShowListener::resumed ) );
}

// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::slideTransitionStarted( ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
        maListeners.forEach<XSlideShowListener>( boost::mem_fn( &XSlideShowListener::slideTransitionStarted ) );
}

// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::slideTransitionEnded( ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
        maListeners.forEach<XSlideShowListener>( boost::mem_fn( &XSlideShowListener::slideTransitionEnded ) );
}

// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::slideAnimationsEnded(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
        maListeners.forEach<XSlideShowListener>( boost::mem_fn( &XSlideShowListener::slideAnimationsEnded ) );
}

// ---------------------------------------------------------

void SlideShowListenerProxy::slideEnded(sal_Bool bReverse) throw (RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if( maListeners.getLength() >= 0 )
            maListeners.forEach<XSlideShowListener>(
                boost::bind( &XSlideShowListener::slideEnded, _1, bReverse) );
    }

    {
        SolarMutexGuard aSolarGuard;
        if( mxController.is() )
            mxController->slideEnded(bReverse);
    }
}

// ---------------------------------------------------------

void SlideShowListenerProxy::hyperLinkClicked( rtl::OUString const& aHyperLink ) throw (RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if( maListeners.getLength() >= 0 )
            maListeners.forEach<XSlideShowListener>( boost::bind( &XSlideShowListener::hyperLinkClicked, _1, boost::cref(aHyperLink) ));
    }

    {
        SolarMutexGuard aSolarGuard;
        if( mxController.is() )
            mxController->hyperLinkClicked(aHyperLink);
    }
}

// ---------------------------------------------------------
// XEventListener
// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::disposing( const ::com::sun::star::lang::EventObject& aDisposeEvent ) throw (RuntimeException)
{
    maListeners.disposeAndClear( aDisposeEvent );
    mxController.clear();
    mxSlideShow.clear();
}

// ---------------------------------------------------------
// XShapeEventListener
// ---------------------------------------------------------

void SAL_CALL SlideShowListenerProxy::click( const Reference< XShape >& xShape, const ::com::sun::star::awt::MouseEvent& aOriginalEvent ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    if( mxController.is() )
        mxController->click(xShape, aOriginalEvent );
}

} // namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
