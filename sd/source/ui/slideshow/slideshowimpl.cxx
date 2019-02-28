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

#include <sal/config.h>

#include <algorithm>

#include <config_features.h>

#include <com/sun/star/frame/theAutoRecovery.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
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
#include <com/sun/star/presentation/SlideShow.hpp>
#include <svl/aeitem.hxx>
#include <svl/urihelper.hxx>
#include <unotools/saveopt.hxx>
#include <basic/sbstar.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>

#include <sfx2/infobar.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/templdlg.hxx>
#include <svx/f3dchild.hxx>
#include <svx/imapdlg.hxx>
#include <svx/fontwork.hxx>
#include <svx/SvxColorChildWindow.hxx>
#include <svx/bmpmask.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/svxids.hrc>
#include <AnimationChildWindow.hxx>
#include <notifydocumentevent.hxx>
#include "slideshowimpl.hxx"
#include "slideshowviewimpl.hxx"
#include <pgjump.hxx>
#include "PaneHider.hxx"

#include <bitmaps.hlst>
#include <vcl/canvastools.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/settings.hxx>

#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <canvas/elapsedtime.hxx>
#include <avmedia/mediawindow.hxx>
#include <svtools/colrdlg.hxx>
#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include <PresentationViewShell.hxx>
#include <RemoteServer.hxx>
#include <customshowlist.hxx>
#include <unopage.hxx>
#include <sdpage.hxx>
#include <sdmod.hxx>
#include <app.hrc>
#include <cusshow.hxx>
#include <optsitem.hxx>

#define CM_SLIDES       21

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
using namespace ::com::sun::star::beans;

namespace sd
{
/** Slots, which will be disabled in the slide show and are managed by Sfx.
    Have to be sorted in the order of the SIDs */
static sal_uInt16 const pAllowed[] =
{
    SID_OPENDOC                             , //     5501   ///< that internally jumps work
    SID_JUMPTOMARK                          , //     5598
    SID_OPENHYPERLINK                       , //     6676
    SID_PRESENTATION_END                     //    27218
};

class AnimationSlideController
{
public:
    enum Mode { ALL, FROM, CUSTOM, PREVIEW };

public:
    AnimationSlideController( Reference< XIndexAccess > const & xSlides, Mode eMode );

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

    bool isValidIndex( sal_Int32 nIndex ) const { return (nIndex >= 0) && (nIndex < static_cast<sal_Int32>(maSlideNumbers.size())); }
    bool isValidSlideNumber( sal_Int32 nSlideNumber ) const { return (nSlideNumber >= 0) && (nSlideNumber < mnSlideCount); }

private:
    Mode const meMode;
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

AnimationSlideController::AnimationSlideController( Reference< XIndexAccess > const & xSlides, Mode eMode  )
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
        xSlide.set( mxSlides->getByIndex(nSlideNumber), UNO_QUERY_THROW );

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
        SAL_WARN( "sd", "sd::AnimationSlideController::getSlideAPI(), "
                    "exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
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

    if( !(xShow.is() && (nCurrentSlideNumber != -1 )) )
        return;

    Reference< XDrawPage > xSlide;
    Reference< XAnimationNode > xAnimNode;
    ::std::vector<PropertyValue> aProperties;

    const sal_Int32 nNextSlideNumber = getNextSlideNumber();
    if( getSlideAPI( nNextSlideNumber, xSlide, xAnimNode )  )
    {
        Sequence< Any > aValue(2);
        aValue[0] <<= xSlide;
        aValue[1] <<= xAnimNode;
        aProperties.emplace_back( "Prefetch" ,
                -1,
                Any(aValue),
                PropertyState_DIRECT_VALUE);
    }
    if (bSkipAllMainSequenceEffects)
    {
        // Add one property that prevents the slide transition from being
        // shown (to speed up the transition to the previous slide) and
        // one to show all main sequence effects so that the user can
        // continue to undo effects.
        aProperties.emplace_back( "SkipAllMainSequenceEffects",
                -1,
                Any(true),
                PropertyState_DIRECT_VALUE);
        aProperties.emplace_back("SkipSlideTransition",
                -1,
                Any(true),
                PropertyState_DIRECT_VALUE);
    }

    if( getSlideAPI( nCurrentSlideNumber, xSlide, xAnimNode ) )
        xShow->displaySlide( xSlide, xDrawPages, xAnimNode, comphelper::containerToSequence(aProperties) );
}

static constexpr OUStringLiteral gsOnClick( "OnClick" );
static constexpr OUStringLiteral gsBookmark( "Bookmark" );
static constexpr OUStringLiteral gsVerb( "Verb" );

SlideshowImpl::SlideshowImpl( const Reference< XPresentation2 >& xPresentation, ViewShell* pViewSh, ::sd::View* pView, SdDrawDocument* pDoc, vcl::Window* pParentWindow )
: SlideshowImplBase( m_aMutex )
, mxModel(pDoc->getUnoModel(),UNO_QUERY_THROW)
, mpView(pView)
, mpViewShell(pViewSh)
, mpDocSh(pDoc->GetDocSh())
, mpDoc(pDoc)
, mpParentWindow(pParentWindow)
, mpShowWindow(nullptr)
, mnRestoreSlide(0)
, maPresSize( -1, -1 )
, meAnimationMode(ANIMATIONMODE_SHOW)
, mpOldActiveWindow(nullptr)
, mnChildMask( 0 )
, mbDisposed(false)
, mbAutoSaveWasOn(false)
, mbRehearseTimings(false)
, mbIsPaused(false)
, mbWasPaused(false)
, mbInputFreeze(false)
, mbActive(false)
, maPresSettings( pDoc->getPresentationSettings() )
, mnUserPaintColor( 0x80ff0000L )
, mbUsePen(false)
, mdUserPaintStrokeWidth ( 150.0 )
, mnEndShowEvent(nullptr)
, mnContextMenuEvent(nullptr)
, mxPresentation( xPresentation )
{
    if( mpViewShell )
        mpOldActiveWindow = mpViewShell->GetActiveWindow();

    maUpdateTimer.SetInvokeHandler(LINK(this, SlideshowImpl, updateHdl));
    // Priority must not be too high or we'll starve input handling etc.
    maUpdateTimer.SetPriority(TaskPriority::REPAINT);

    maDeactivateTimer.SetInvokeHandler(LINK(this, SlideshowImpl, deactivateHdl));
    maDeactivateTimer.SetTimeout( 20 );

    maInputFreezeTimer.SetInvokeHandler( LINK( this, SlideshowImpl, ReadyForNextInputHdl ) );
    maInputFreezeTimer.SetTimeout( 20 );

    SvtSaveOptions aOptions;

        // no autosave during show
    if( aOptions.IsAutoSave() )
        mbAutoSaveWasOn = true;

    Application::AddEventListener( LINK( this, SlideshowImpl, EventListenerHdl ) );

    mbUsePen = maPresSettings.mbMouseAsPen;

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
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
        pModule->GetSdOptions(DocumentType::Impress) : nullptr;
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
        NotifyDocumentEvent(
            *mpDoc,
            "OnEndPresentation" );

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

    removeShapeEvents();

    if( mxListenerProxy.is() )
        mxListenerProxy->removeAsSlideShowListener();

    try
    {
        if( mxView.is() )
            mxShow->removeView( mxView.get() );

        Reference< XComponent > xComponent( mxShow, UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();

        if( mxView.is() )
            mxView->dispose();
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::stop(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }

    mxShow.clear();
    mxView.clear();
    mxListenerProxy.clear();
    mpSlideController.reset();

    // take DrawView from presentation window, but give the old window back
    if( mpShowWindow && mpView )
        mpView->DeleteWindowFromPaintView( mpShowWindow );

    if( mpView )
        mpView->SetAnimationPause( false );

    if( mpViewShell )
    {
        mpViewShell->SetActiveWindow(mpOldActiveWindow);
        if (mpShowWindow)
            mpShowWindow->SetViewShell( nullptr );
    }

    if( mpView )
        mpView->InvalidateAllWin();

    if( maPresSettings.mbFullScreen )
    {
#if HAVE_FEATURE_SCRIPTING
        // restore StarBASICErrorHdl
        StarBASIC::SetGlobalErrorHdl(maStarBASICGlobalErrorHdl);
        maStarBASICGlobalErrorHdl = Link<StarBASIC*,bool>();
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
        mnChildMask = 0;
    }

    // show current window again
    if( mpViewShell && dynamic_cast< PresentationViewShell *>( mpViewShell ) ==  nullptr)
    {
        if( meAnimationMode == ANIMATIONMODE_SHOW )
        {
            mpViewShell->GetViewShellBase().ShowUIControls (true);
            mpPaneHider.reset();
        }
        else if( meAnimationMode == ANIMATIONMODE_PREVIEW )
        {
            mpViewShell->ShowUIControls(true);
        }
    }

    if( mpShowWindow )
        mpShowWindow->Hide();
    mpShowWindow.disposeAndClear();

    if ( mpViewShell )
    {
        if( meAnimationMode == ANIMATIONMODE_SHOW )
        {
            ::sd::Window* pActWin = mpViewShell->GetActiveWindow();

            if (pActWin)
            {
                Size aVisSizePixel = pActWin->GetOutputSizePixel();
                ::tools::Rectangle aVisAreaWin = pActWin->PixelToLogic( ::tools::Rectangle( Point(0,0), aVisSizePixel) );
                mpViewShell->VisAreaChanged(aVisAreaWin);
                if (mpView)
                    mpView->VisAreaChanged(pActWin);
                pActWin->GrabFocus();
            }
        }

        // restart the custom show dialog if he started us
        if( mpViewShell->IsStartShowWithDialog() && getDispatcher() )
        {
            mpViewShell->SetStartShowWithDialog( false );
            getDispatcher()->Execute( SID_CUSTOMSHOW_DLG, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
        }

        mpViewShell->GetViewShellBase().UpdateBorder(true);
    }

    if( mpShowWindow )
    {
        mpShowWindow.disposeAndClear();
    }

    setActiveXToolbarsVisible( true );

    mbDisposed = true;
}

bool SlideshowImpl::startPreview(
        const Reference< XDrawPage >& xDrawPage,
        const Reference< XAnimationNode >& xAnimationNode,
        vcl::Window * pParent )
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

        maPresSettings.mbAll = false;
        maPresSettings.mbEndless = false;
        maPresSettings.mbCustomShow = false;
        maPresSettings.mbManual = false;
        maPresSettings.mbMouseVisible = false;
        maPresSettings.mbMouseAsPen = false;
        maPresSettings.mbLockedPages = false;
        maPresSettings.mbAlwaysOnTop = false;
        maPresSettings.mbFullScreen = false;
        maPresSettings.mbAnimationAllowed = true;
        maPresSettings.mnPauseTimeout = 0;
        maPresSettings.mbShowPauseLogo = false;

        Reference< XDrawPagesSupplier > xDrawPages( mpDoc->getUnoModel(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xSlides( xDrawPages->getDrawPages(), UNO_QUERY_THROW );
        mpSlideController.reset( new AnimationSlideController( xSlides, AnimationSlideController::PREVIEW ) );

        sal_Int32 nSlideNumber = 0;
        Reference< XPropertySet > xSet( mxPreviewDrawPage, UNO_QUERY_THROW );
        xSet->getPropertyValue( "Number" ) >>= nSlideNumber;
        mpSlideController->insertSlideNumber( nSlideNumber-1 );
        mpSlideController->setPreviewNode( xAnimationNode );

        mpShowWindow = VclPtr<ShowWindow>::Create( this, ((pParent == nullptr) && mpViewShell) ?  mpParentWindow.get() : pParent );
        if( mpViewShell )
        {
            mpViewShell->SetActiveWindow( mpShowWindow );
            mpShowWindow->SetViewShell (mpViewShell);
            mpViewShell->ShowUIControls (false);
        }

        if( mpView )
        {
            mpView->AddWindowToPaintView( mpShowWindow, nullptr );
            mpView->SetAnimationPause( true );
        }

        // call resize handler
        if( pParent )
        {
            maPresSize = pParent->GetSizePixel();
        }
        else if( mpViewShell )
        {
            ::tools::Rectangle aContentRect (mpViewShell->GetViewShellBase().getClientRectangle());
            if (AllSettings::GetLayoutRTL())
            {
                aContentRect.SetLeft( aContentRect.Right() );
                aContentRect.AdjustRight(aContentRect.Right() );
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
        aProperties[0].Value <<= 1.0; // one second timeout

        if( mxPreviewAnimationNode.is() )
        {
            aProperties[1].Name = "NoSlideTransitions";
            aProperties[1].Value <<= true;
        }

        bRet = startShowImpl( aProperties );

        if( mpShowWindow != nullptr && meAnimationMode == ANIMATIONMODE_PREVIEW )
            mpShowWindow->SetPreviewMode();

    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::startPreview(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
        bRet = false;
    }

    return bRet;
}

bool SlideshowImpl::startShow( PresentationSettingsEx const * pPresSettings )
{
    const rtl::Reference<SlideshowImpl> this_(this);

    DBG_ASSERT( !mxShow.is(), "sd::SlideshowImpl::startShow(), called twice!" );
    if( mxShow.is() )
        return true;
    DBG_ASSERT( mpParentWindow!=nullptr, "sd::SlideshowImpl::startShow() called without parent window" );
    if (mpParentWindow == nullptr)
        return false;

    // Autoplay (pps/ppsx)
    if (mpViewShell->GetDoc()->IsStartWithPresentation()){
        mpViewShell->GetDoc()->SetExitAfterPresenting(true);
    }

    bool bRet = false;

    try
    {
        if( pPresSettings )
        {
            maPresSettings = *pPresSettings;
            mbRehearseTimings = pPresSettings->mbRehearseTimings;
        }

        OUString  aPresSlide( maPresSettings.maPresPage );
        SdPage* pStartPage = mpViewShell->GetActualPage();
        bool    bStartWithActualSlide =  pStartPage;

        // times should be measured?
        if( mbRehearseTimings )
        {
            maPresSettings.mbEndless = false;
            maPresSettings.mbManual = true;
            maPresSettings.mbMouseVisible = true;
            maPresSettings.mbMouseAsPen = false;
            maPresSettings.mnPauseTimeout = 0;
            maPresSettings.mbShowPauseLogo = false;
        }

        if( pStartPage )
        {
            if( pStartPage->GetPageKind() == PageKind::Notes )
            {
                // we are in notes page mode, so get
                // the corresponding draw page
                const sal_uInt16 nPgNum = ( pStartPage->GetPageNum() - 2 ) >> 1;
                pStartPage = mpDoc->GetSdPage( nPgNum, PageKind::Standard );
            }
        }

        if( bStartWithActualSlide )
        {
            if ( aPresSlide.isEmpty())
            {
                // no preset slide yet, so pick current on one
                aPresSlide = pStartPage->GetName();
                // if the starting slide is hidden, we can't set slide controller to ALL mode
                maPresSettings.mbAll = !pStartPage->IsExcluded();
            }

            if( meAnimationMode != ANIMATIONMODE_SHOW )
            {
                if( pStartPage->GetPageKind() == PageKind::Standard )
                {
                    maPresSettings.mbAll = false;
                }
            }
        }

        // build page list
        createSlideList( maPresSettings.mbAll, aPresSlide );

        // remember Slide number from where the show was started
        if( pStartPage )
            mnRestoreSlide = ( pStartPage->GetPageNum() - 1 ) / 2;

        if( mpSlideController->hasSlides() )
        {
            // hide child windows
            hideChildWindows();

            mpShowWindow = VclPtr<ShowWindow>::Create( this, mpParentWindow );
            mpShowWindow->SetMouseAutoHide( !maPresSettings.mbMouseVisible );
            mpViewShell->SetActiveWindow( mpShowWindow );
            mpShowWindow->SetViewShell (mpViewShell);
            mpViewShell->GetViewShellBase().ShowUIControls (false);
            // Hide the side panes for in-place presentations.
            if ( ! maPresSettings.mbFullScreen)
                mpPaneHider.reset(new PaneHider(*mpViewShell,this));

            // these Slots are forbidden in other views for this document
            if( mpDocSh )
            {
                mpDocSh->SetSlotFilter( true, pAllowed );
                mpDocSh->ApplySlotFilter();
            }

            Help::DisableContextHelp();
            Help::DisableExtHelp();

            if( maPresSettings.mbFullScreen )
            {
#if HAVE_FEATURE_SCRIPTING
                // disable basic ide error handling
                maStarBASICGlobalErrorHdl = StarBASIC::GetGlobalErrorHdl();
                StarBASIC::SetGlobalErrorHdl( Link<StarBASIC*,bool>() );
#endif
            }

            // call resize handler
            maPresSize = mpParentWindow->GetSizePixel();
            if (!maPresSettings.mbFullScreen)
            {
                const ::tools::Rectangle& aClientRect = mpViewShell->GetViewShellBase().getClientRectangle();
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
                mpView->AddWindowToPaintView( mpShowWindow, nullptr );
                mpView->SetAnimationPause( true );
            }

            SfxBindings* pBindings = getBindings();
            if( pBindings )
            {
                pBindings->Invalidate( SID_PRESENTATION );
                pBindings->Invalidate( SID_REHEARSE_TIMINGS );
            }

            // Defer the sd::ShowWindow's GrabFocus to SlideShow::activate. so that the accessible event can be fired correctly.
            //mpShowWindow->GrabFocus();

            std::vector<beans::PropertyValue> aProperties;
            aProperties.reserve( 4 );

            aProperties.emplace_back( "AdvanceOnClick" ,
                    -1, Any( !maPresSettings.mbLockedPages ),
                    beans::PropertyState_DIRECT_VALUE );

            aProperties.emplace_back( "ImageAnimationsAllowed" ,
                    -1, Any( maPresSettings.mbAnimationAllowed ),
                    beans::PropertyState_DIRECT_VALUE );

            const bool bZOrderEnabled(
                SD_MOD()->GetSdOptions( mpDoc->GetDocumentType() )->IsSlideshowRespectZOrder() );
            aProperties.emplace_back( "DisableAnimationZOrder" ,
                    -1, Any( !bZOrderEnabled ),
                    beans::PropertyState_DIRECT_VALUE );

            aProperties.emplace_back( "ForceManualAdvance" ,
                    -1, Any( maPresSettings.mbManual ),
                    beans::PropertyState_DIRECT_VALUE );

            if( mbUsePen )
             {
                aProperties.emplace_back( "UserPaintColor" ,
                        // User paint color is black by default.
                        -1, Any( mnUserPaintColor ),
                        beans::PropertyState_DIRECT_VALUE );

                aProperties.emplace_back( "UserPaintStrokeWidth" ,
                        // User paint color is black by default.
                        -1, Any( mdUserPaintStrokeWidth ),
                        beans::PropertyState_DIRECT_VALUE );
            }

            if (mbRehearseTimings) {
                aProperties.emplace_back( "RehearseTimings" ,
                        -1, Any(true), beans::PropertyState_DIRECT_VALUE );
            }

            bRet = startShowImpl( Sequence<beans::PropertyValue>(
                                      &aProperties[0], aProperties.size() ) );

        }

        setActiveXToolbarsVisible( false );
    }
    catch (const Exception&)
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::startShow(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
        bRet = false;
    }

    return bRet;
}

bool SlideshowImpl::startShowImpl( const Sequence< beans::PropertyValue >& aProperties )
{
    try
    {
        mxShow.set( createSlideShow(), UNO_QUERY_THROW );

        mxView = new SlideShowView(
                                             *mpShowWindow,
                                             mpDoc,
                                             meAnimationMode,
                                             this,
                                             maPresSettings.mbFullScreen);

        // try add wait symbol to properties:
        const Reference<rendering::XSpriteCanvas> xSpriteCanvas(
            mxView->getCanvas() );
        if (xSpriteCanvas.is())
        {
            BitmapEx waitSymbolBitmap(BMP_WAIT_ICON);
            const Reference<rendering::XBitmap> xBitmap(
                vcl::unotools::xBitmapFromBitmapEx( waitSymbolBitmap ) );
            if (xBitmap.is())
            {
                mxShow->setProperty(
                    beans::PropertyValue( "WaitSymbolBitmap" ,
                        -1,
                        makeAny( xBitmap ),
                        beans::PropertyState_DIRECT_VALUE ) );
            }

            BitmapEx pointerSymbolBitmap(BMP_POINTER_ICON);
            const Reference<rendering::XBitmap> xPointerBitmap(
                vcl::unotools::xBitmapFromBitmapEx( pointerSymbolBitmap ) );
            if (xPointerBitmap.is())
            {
                mxShow->setProperty(
                    beans::PropertyValue( "PointerSymbolBitmap" ,
                        -1,
                        makeAny( xPointerBitmap ),
                        beans::PropertyState_DIRECT_VALUE ) );
            }
        }

        const sal_Int32 nCount = aProperties.getLength();
        sal_Int32 nIndex;
        for( nIndex = 0; nIndex < nCount; nIndex++ )
            mxShow->setProperty( aProperties[nIndex] );

        mxShow->addView( mxView.get() );

        mxListenerProxy.set( new SlideShowListenerProxy( this, mxShow ) );
        mxListenerProxy->addAsSlideShowListener();

        NotifyDocumentEvent(
            *mpDoc,
            "OnStartPresentation");
        displaySlideIndex( mpSlideController->getStartSlideIndex() );

        return true;
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::startShowImpl(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
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
        mpShowWindow->SetBackground( Wallpaper( COL_BLACK ) );
        mpShowWindow->Erase();
        mpShowWindow->SetBackground();
        */
    }

    SolarMutexGuard aSolarGuard;
    maUpdateTimer.SetTimeout( sal_uLong(100) );
    maUpdateTimer.Start();
}

void SlideshowImpl::paint()
{
    if( mxView.is() ) try
    {
        awt::PaintEvent aEvt;
        // aEvt.UpdateRect = TODO
        mxView->paint( aEvt );
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::paint(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

void SAL_CALL SlideshowImpl::addSlideShowListener( const Reference< XSlideShowListener >& xListener )
{
    if( mxListenerProxy.is() )
        mxListenerProxy->addSlideShowListener( xListener );
}

void SAL_CALL SlideshowImpl::removeSlideShowListener( const Reference< XSlideShowListener >& xListener )
{
    if( mxListenerProxy.is() )
        mxListenerProxy->removeSlideShowListener( xListener );
}

void SlideshowImpl::slideEnded(const bool bReverse)
{
    if (bReverse)
        gotoPreviousSlide(true);
    else
        gotoNextSlide();
}

bool SlideshowImpl::swipe(const CommandSwipeData &rSwipeData)
{
    if (mbUsePen || mnContextMenuEvent)
        return false;
    double nVelocityX = rSwipeData.getVelocityX();
    // tdf#108475 make it swipe only if some reasonable movement was involved
    if (fabs(nVelocityX) < 50)
        return false;
    if (nVelocityX > 0)
    {
        gotoPreviousSlide();
    }
    else
    {
        gotoNextEffect();
    }
    //a swipe is followed by a mouse up, tell the view to ignore that mouse up as we've reacted
    //to the swipe instead
    mxView->ignoreNextMouseReleased();
    return true;
}

bool SlideshowImpl::longpress(const CommandLongPressData &rLongPressData)
{
    if (mnContextMenuEvent)
        return false;

    maPopupMousePos = Point(rLongPressData.getX(), rLongPressData.getY());
    mnContextMenuEvent = Application::PostUserEvent( LINK( this, SlideshowImpl, ContextMenuHdl ) );

    return true;
}

void SlideshowImpl::removeShapeEvents()
{
    if( !(mxShow.is() && mxListenerProxy.is()) )
        return;

    try
    {
        for( const auto& rEntry : maShapeEventMap )
        {
            mxListenerProxy->removeShapeEventListener( rEntry.first );
            mxShow->setShapeCursor( rEntry.first, awt::SystemPointer::ARROW );
        }

        maShapeEventMap.clear();
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::removeShapeEvents(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

void SlideshowImpl::registerShapeEvents(sal_Int32 nSlideNumber)
{
    if( nSlideNumber < 0 )
        return;

    try
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
        SAL_WARN( "sd", "sd::SlideshowImpl::registerShapeEvents(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

void SlideshowImpl::registerShapeEvents( Reference< XShapes > const & xShapes )
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
            if( !xSetInfo.is() || !xSetInfo->hasPropertyByName( gsOnClick ) )
                continue;

            WrappedShapeEventImplPtr pEvent( new WrappedShapeEventImpl );
            xSet->getPropertyValue( gsOnClick ) >>= pEvent->meClickAction;

            switch( pEvent->meClickAction )
            {
            case ClickAction_PREVPAGE:
            case ClickAction_NEXTPAGE:
            case ClickAction_FIRSTPAGE:
            case ClickAction_LASTPAGE:
            case ClickAction_STOPPRESENTATION:
                break;
            case ClickAction_BOOKMARK:
                if( xSetInfo->hasPropertyByName( gsBookmark ) )
                    xSet->getPropertyValue( gsBookmark ) >>= pEvent->maStrBookmark;
                if( getSlideNumberForBookmark( pEvent->maStrBookmark ) == -1 )
                    continue;
                break;
            case ClickAction_DOCUMENT:
            case ClickAction_SOUND:
            case ClickAction_PROGRAM:
            case ClickAction_MACRO:
                if( xSetInfo->hasPropertyByName( gsBookmark ) )
                    xSet->getPropertyValue( gsBookmark ) >>= pEvent->maStrBookmark;
                break;
            case ClickAction_VERB:
                if( xSetInfo->hasPropertyByName( gsVerb ) )
                    xSet->getPropertyValue( gsVerb ) >>= pEvent->mnVerb;
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
        SAL_WARN( "sd", "sd::SlideshowImpl::registerShapeEvents(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

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

    }
    // send out page change event and notify to update all acc info for current page
    if (mpViewShell)
    {
        sal_Int32 currentPageIndex = getCurrentSlideIndex();
        mpViewShell->fireSwitchCurrentPage(currentPageIndex);
        mpViewShell->NotifyAccUpdate();
    }
}

void SlideshowImpl::endPresentation()
{
    if( maPresSettings.mbMouseAsPen)
    {
        Reference< XMultiServiceFactory > xDocFactory(mpDoc->getUnoModel(), UNO_QUERY );
        if( xDocFactory.is() )
            mxShow->registerUserPaintPolygons(xDocFactory);
    }

    if( !mnEndShowEvent )
        mnEndShowEvent = Application::PostUserEvent( LINK(this, SlideshowImpl, endPresentationHdl) );
}

IMPL_LINK_NOARG(SlideshowImpl, endPresentationHdl, void*, void)
{
    mnEndShowEvent = nullptr;

    if( mxPresentation.is() )
        mxPresentation->end();
}

void SAL_CALL SlideshowImpl::pause()
{
    SolarMutexGuard aSolarGuard;

    if( mbIsPaused )
        return;

    try
    {
        mbIsPaused = true;
        if( mxShow.is() )
        {
            mxShow->pause(true);

            if( mxListenerProxy.is() )
                mxListenerProxy->paused();
        }
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::pause(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

void SAL_CALL SlideshowImpl::resume()
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
            mbIsPaused = false;
            if( mxShow.is() )
            {
                mxShow->pause(false);
                update();

                if( mxListenerProxy.is() )
                    mxListenerProxy->resumed();
            }
        }
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::resume(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
#ifdef ENABLE_SDREMOTE
    RemoteServer::presentationStarted( this );
#endif
}

sal_Bool SAL_CALL SlideshowImpl::isPaused()
{
    SolarMutexGuard aSolarGuard;
    return mbIsPaused;
}

void SAL_CALL SlideshowImpl::blankScreen( sal_Int32 nColor )
{
    SolarMutexGuard aSolarGuard;

    if( mpShowWindow && mpSlideController )
    {
        if( mpShowWindow->SetBlankMode( mpSlideController->getCurrentSlideIndex(), Color(nColor) ) )
        {
            pause();
        }
    }
}

// XShapeEventListener

void SlideshowImpl::click( const Reference< XShape >& xShape )
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
#if HAVE_FEATURE_AVMEDIA
        try
        {
            mxPlayer.set(avmedia::MediaWindow::createPlayer(pEvent->maStrBookmark, ""/*TODO?*/), uno::UNO_QUERY_THROW );
            mxPlayer->start();
        }
        catch( uno::Exception& )
        {
            OSL_FAIL("sd::SlideshowImpl::click(), exception caught!" );
        }
#endif
    }
    break;

    case ClickAction_DOCUMENT:
    {
        OUString aBookmark( pEvent->maStrBookmark );

        sal_Int32 nPos = aBookmark.indexOf( '#' );
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
                false, INetURLObject::EncodeMechanism::WasEncoded,
                INetURLObject::DecodeMechanism::Unambiguous ) );

        if( INetProtocol::File == aURL.GetProtocol() )
        {
            SfxStringItem aUrl( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            SfxBoolItem aBrowsing( SID_BROWSE, true );

            SfxViewFrame* pViewFrm = SfxViewFrame::Current();
            if (pViewFrm)
            {
                pViewFrm->GetDispatcher()->ExecuteList( SID_OPENDOC,
                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                    { &aUrl, &aBrowsing });
            }
        }
    }
    break;

    case presentation::ClickAction_MACRO:
    {
        const OUString aMacro( pEvent->maStrBookmark );

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
            // "Macroname.Modulname.Libname.Documentname" or
            // "Macroname.Modulname.Libname.Applicationname"
            sal_Int32 nIdx{ 0 };
            const OUString aMacroName = aMacro.getToken(0, '.', nIdx);
            const OUString aModulName = aMacro.getToken(0, '.', nIdx);

            // todo: is the limitation still given that only
            // Modulname+Macroname can be used here?
            OUString aExecMacro = aModulName + "." + aMacroName;
            mpDocSh->GetBasic()->Call(aExecMacro);
        }
    }
    break;

    case ClickAction_VERB:
    {
        // todo, better do it async?
        SdrObject* pObj = GetSdrObjectFromXShape( xShape );
        SdrOle2Obj* pOleObject = dynamic_cast< SdrOle2Obj* >(pObj);
        if (pOleObject && mpViewShell )
            mpViewShell->ActivateObject(pOleObject, pEvent->mnVerb);
    }
    break;
    default:
        break;
    }
}

sal_Int32 SlideshowImpl::getSlideNumberForBookmark( const OUString& rStrBookmark )
{
    bool bIsMasterPage;
    OUString aBookmark = getUiNameFromPageApiNameImpl( rStrBookmark );
    sal_uInt16 nPgNum = mpDoc->GetPageByName( aBookmark, bIsMasterPage );

    if( nPgNum == SDRPAGE_NOTFOUND )
    {
        // Is the bookmark an object?
        SdrObject* pObj = mpDoc->GetObj( aBookmark );

        if( pObj )
        {
            nPgNum = pObj->getSdrPageFromSdrObject()->GetPageNum();
            bIsMasterPage = pObj->getSdrPageFromSdrObject()->IsMasterPage();
        }
    }

    if( (nPgNum == SDRPAGE_NOTFOUND) || bIsMasterPage || static_cast<SdPage*>(mpDoc->GetPage(nPgNum))->GetPageKind() != PageKind::Standard )
        return -1;

    return ( nPgNum - 1) >> 1;
}

void SlideshowImpl::hyperLinkClicked( OUString const& aHyperLink )
{
    OUString aBookmark( aHyperLink );

    sal_Int32 nPos = aBookmark.indexOf( '#' );
    if( nPos >= 0 )
    {
        OUString aURL( aBookmark.copy( 0, nPos+1 ) );
        OUString aName( aBookmark.copy( nPos+1 ) );
        aURL += getUiNameFromPageApiNameImpl( aName );
        aBookmark = aURL;
    }

    mpDocSh->OpenBookmark( aBookmark );
}

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

void SlideshowImpl::jumpToBookmark( const OUString& sBookmark )
{
    sal_Int32 nSlideNumber = getSlideNumberForBookmark( sBookmark );
    if( nSlideNumber != -1 )
        displaySlideNumber( nSlideNumber );
}

sal_Int32 SlideshowImpl::getCurrentSlideNumber()
{
    return mpSlideController.get() ? mpSlideController->getCurrentSlideNumber() : -1;
}

sal_Bool SAL_CALL SlideshowImpl::isEndless()
{
    SolarMutexGuard aSolarGuard;
    return maPresSettings.mbEndless;
}

void SlideshowImpl::update()
{
    startUpdateTimer();
}

void SlideshowImpl::startUpdateTimer()
{
    SolarMutexGuard aSolarGuard;
    maUpdateTimer.SetTimeout( 0 );
    maUpdateTimer.Start();
}

/** this timer is called 20ms after a new slide was displayed.
    This is used to unfreeze user input that was disabled after
    slide change to skip input that was buffered during slide
    transition preparation */
IMPL_LINK_NOARG(SlideshowImpl, ReadyForNextInputHdl, Timer *, void)
{
    mbInputFreeze = false;
}

/** if I catch someone someday who calls this method by hand
    and not by using the timer, I will personally punish this
    person seriously, even if this person is me.
*/
IMPL_LINK_NOARG(SlideshowImpl, updateHdl, Timer *, void)
{
    updateSlideShow();
}

void SlideshowImpl::updateSlideShow()
{
    // prevent me from deletion when recursing (App::EnableYieldMode does)
    const rtl::Reference<SlideshowImpl> this_(this);

    Reference< XSlideShow > xShow( mxShow );
    if ( ! xShow.is())
        return;

    try
    {
        double fUpdate = 0.0;
        if( !xShow->update(fUpdate) )
            fUpdate = -1.0;

        if (mxShow.is() && (fUpdate >= 0.0))
        {
            if (!::basegfx::fTools::equalZero(fUpdate))
            {
                // Avoid busy loop when the previous call to update()
                // returns a small positive number but not 0 (which is
                // handled above).  Also, make sure that calls to update()
                // have a minimum frequency.
                // => Allow up to 60 frames per second.  Call at least once
                // every 4 seconds.
                const static sal_Int32 nMaximumFrameCount (60);
                const static double nMinimumTimeout (1.0 / nMaximumFrameCount);
                const static double nMaximumTimeout (4.0);
                fUpdate = std::clamp(fUpdate, nMinimumTimeout, nMaximumTimeout);

                // Make sure that the maximum frame count has not been set
                // too high (only then conversion to milliseconds and long
                // integer may lead to zero value.)
                OSL_ASSERT(static_cast<sal_uLong>(fUpdate * 1000.0) > 0);
            }

            // Use our high resolution timers for the asynchronous callback.
            maUpdateTimer.SetTimeout(static_cast<sal_uLong>(fUpdate * 1000.0));
            maUpdateTimer.Start();
        }
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::updateSlideShow(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

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
                [[fallthrough]];
            case KEY_SPACE:
            case KEY_RIGHT:
            case KEY_DOWN:
            case KEY_N:
                gotoNextEffect();
                break;

            case KEY_RETURN:
            {
                if( !maCharBuffer.isEmpty() )
                {
                    if( mpSlideController.get() )
                    {
                        if( mpSlideController->jumpToSlideNumber( maCharBuffer.toInt32() - 1 ) )
                            displayCurrentSlide();
                    }
                    maCharBuffer.clear();
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
                maCharBuffer += OUStringLiteral1( rKEvt.GetCharCode() );
                break;

            case KEY_PAGEUP:
                if(rKEvt.GetKeyCode().IsMod2())
                {
                    gotoPreviousSlide();
                    break;
                }
                [[fallthrough]];
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
        SAL_WARN( "sd", "sd::SlideshowImpl::keyInput(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }

    return bRet;
}

IMPL_LINK( SlideshowImpl, EventListenerHdl, VclSimpleEvent&, rSimpleEvent, void )
{
    if( !mxShow.is() || mbInputFreeze )
        return;

    if( !((rSimpleEvent.GetId() == VclEventId::WindowCommand) && static_cast<VclWindowEvent*>(&rSimpleEvent)->GetData()) )
        return;

    const CommandEvent& rEvent = *static_cast<const CommandEvent*>(static_cast<VclWindowEvent*>(&rSimpleEvent)->GetData());

    if( rEvent.GetCommand() != CommandEventId::Media )
        return;

    CommandMediaData* pMediaData = rEvent.GetMediaData();
    pMediaData->SetPassThroughToOS(false);
    switch (pMediaData->GetMediaId())
    {
#if defined( MACOSX )
    case MediaCommand::Menu:
        if( !mnContextMenuEvent )
        {
        if( mpShowWindow )
            maPopupMousePos = mpShowWindow->GetPointerState().maPos;
        mnContextMenuEvent = Application::PostUserEvent( LINK( this, SlideshowImpl, ContextMenuHdl ) );
        }
        break;
    case MediaCommand::VolumeDown:
        gotoPreviousSlide();
        break;
    case MediaCommand::VolumeUp:
        gotoNextEffect();
        break;
#endif
    case MediaCommand::NextTrack:
        gotoNextEffect();
        break;
    case MediaCommand::Pause:
        if( !mbIsPaused )
            blankScreen(0);
        break;
    case MediaCommand::Play:
        if( mbIsPaused )
            resume();
        break;

    case MediaCommand::PlayPause:
        if( mbIsPaused )
            resume();
        else
            blankScreen(0);
        break;
    case MediaCommand::PreviousTrack:
        gotoPreviousSlide();
        break;
    case MediaCommand::NextTrackHold:
        gotoLastSlide();
        break;

    case MediaCommand::Rewind:
        gotoFirstSlide();
        break;
    case MediaCommand::Stop:
        // in case the user cancels the presentation, switch to current slide
        // in edit mode
        if( mpSlideController.get() && (ANIMATIONMODE_SHOW == meAnimationMode) )
        {
            if( mpSlideController->getCurrentSlideNumber() != -1 )
                mnRestoreSlide = mpSlideController->getCurrentSlideNumber();
        }
        endPresentation();
        break;
    default:
        pMediaData->SetPassThroughToOS(true);
        break;
    }
}

void SlideshowImpl::mouseButtonUp(const MouseEvent& rMEvt)
{
    if( rMEvt.IsRight() && !mnContextMenuEvent )
    {
        maPopupMousePos = rMEvt.GetPosPixel();
        mnContextMenuEvent = Application::PostUserEvent( LINK( this, SlideshowImpl, ContextMenuHdl ) );
    }
}

IMPL_LINK_NOARG(SlideshowImpl, ContextMenuHdl, void*, void)
{
    mnContextMenuEvent = nullptr;

    if (mpSlideController == nullptr)
        return;

    mbWasPaused = mbIsPaused;
    if( !mbWasPaused )
        pause();

    VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/simpress/ui/slidecontextmenu.ui", "");
    VclPtr<PopupMenu> pMenu(aBuilder.get_menu("menu"));

    // Adding button to display if in Pen  mode
    pMenu->CheckItem("pen", mbUsePen);

    const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
    pMenu->EnableItem(pMenu->GetItemId("next"), mpSlideController->getNextSlideIndex() != -1);
    pMenu->EnableItem(pMenu->GetItemId("prev"), (mpSlideController->getPreviousSlideIndex() != -1 ) || (eMode == SHOWWINDOWMODE_END) || (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK));
    pMenu->EnableItem(pMenu->GetItemId("edit"), mpViewShell->GetDoc()->IsStartWithPresentation());

    PopupMenu* pPageMenu = pMenu->GetPopupMenu(pMenu->GetItemId("goto"));

    SfxViewFrame* pViewFrame = getViewFrame();
    if( pViewFrame )
    {
        Reference< css::frame::XFrame > xFrame( pViewFrame->GetFrame().GetFrameInterface() );
        if( xFrame.is() )
        {
            pMenu->SetItemImage(pMenu->GetItemId("next"), vcl::CommandInfoProvider::GetImageForCommand(".uno:NextRecord", xFrame));
            pMenu->SetItemImage(pMenu->GetItemId("prev"), vcl::CommandInfoProvider::GetImageForCommand(".uno:PrevRecord", xFrame));

            if( pPageMenu )
            {
                pPageMenu->SetItemImage(pPageMenu->GetItemId("first"), vcl::CommandInfoProvider::GetImageForCommand(".uno:FirstRecord", xFrame));
                pPageMenu->SetItemImage(pPageMenu->GetItemId("last"), vcl::CommandInfoProvider::GetImageForCommand(".uno:LastRecord", xFrame));
            }
        }
    }

    // populate slide goto list
    if( pPageMenu )
    {
        const sal_Int32 nPageNumberCount = mpSlideController->getSlideNumberCount();
        if( nPageNumberCount <= 1 )
        {
            pMenu->EnableItem(pMenu->GetItemId("goto"), false);
        }
        else
        {
            sal_Int32 nCurrentSlideNumber = mpSlideController->getCurrentSlideNumber();
            if( (eMode == SHOWWINDOWMODE_END) || (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) )
                nCurrentSlideNumber = -1;

            pPageMenu->EnableItem(pPageMenu->GetItemId("first"), mpSlideController->getSlideNumber(0) != nCurrentSlideNumber);
            pPageMenu->EnableItem(pPageMenu->GetItemId("last"), mpSlideController->getSlideNumber(mpSlideController->getSlideIndexCount() - 1) != nCurrentSlideNumber);

            sal_Int32 nPageNumber;

            for( nPageNumber = 0; nPageNumber < nPageNumberCount; nPageNumber++ )
            {
                if( mpSlideController->isVisibleSlideNumber( nPageNumber ) )
                {
                    SdPage* pPage = mpDoc->GetSdPage(static_cast<sal_uInt16>(nPageNumber), PageKind::Standard);
                    if (pPage)
                    {
                        pPageMenu->InsertItem( static_cast<sal_uInt16>(CM_SLIDES + nPageNumber), pPage->GetName() );
                        if( nPageNumber == nCurrentSlideNumber )
                            pPageMenu->CheckItem( static_cast<sal_uInt16>(CM_SLIDES + nPageNumber) );
                    }
                }
            }
        }
    }

    if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_BLANK )
    {
        PopupMenu* pBlankMenu = pMenu->GetPopupMenu(pMenu->GetItemId("screen"));
        if( pBlankMenu )
        {
            pBlankMenu->CheckItem((mpShowWindow->GetBlankColor() == COL_WHITE) ? "white" : "black");
        }
    }

    PopupMenu* pWidthMenu = pMenu->GetPopupMenu(pMenu->GetItemId("width"));

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

            if (nWidth == mdUserPaintStrokeWidth)
                pWidthMenu->CheckItem(OString::number(nWidth));
        }
    }

    pMenu->SetSelectHdl( LINK( this, SlideshowImpl, ContextMenuSelectHdl ) );
    pMenu->Execute( mpShowWindow, maPopupMousePos );

    if( mxView.is() )
        mxView->ignoreNextMouseReleased();

    if( !mbWasPaused )
        resume();
}

IMPL_LINK( SlideshowImpl, ContextMenuSelectHdl, Menu *, pMenu, bool )
{
    if (!pMenu)
        return false;

    OString sMenuId = pMenu->GetCurItemIdent();

    if (sMenuId == "prev")
    {
        gotoPreviousSlide();
        mbWasPaused = false;
    }
    else if(sMenuId == "next")
    {
        gotoNextSlide();
        mbWasPaused = false;
    }
    else if (sMenuId == "first")
    {
        gotoFirstSlide();
        mbWasPaused = false;
    }
    else if (sMenuId == "last")
    {
        gotoLastSlide();
        mbWasPaused = false;
    }
    else if (sMenuId == "black" || sMenuId == "white")
    {
        const Color aBlankColor(sMenuId == "white" ? COL_WHITE : COL_BLACK);
        if( mbWasPaused )
        {
            if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_BLANK )
            {
                if( mpShowWindow->GetBlankColor() == aBlankColor )
                {
                    mbWasPaused = false;
                    mpShowWindow->RestartShow();
                    return false;
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
    else if (sMenuId == "color")
    {
        //Open a color picker based on SvColorDialog
        ::Color aColor( mnUserPaintColor );
        SvColorDialog aColorDlg;
        aColorDlg.SetColor( aColor );

        if (aColorDlg.Execute(mpShowWindow->GetFrameWeld()))
        {
            aColor = aColorDlg.GetColor();
            setPenColor(sal_Int32(aColor));
        }
        mbWasPaused = false;
    }
    else if (sMenuId == "4")
    {
        setPenWidth(4.0);
        mbWasPaused = false;
    }
    else if (sMenuId == "100")
    {
        setPenWidth(100.0);
        mbWasPaused = false;
    }
    else if (sMenuId == "150")
    {
        setPenWidth(150.0);
        mbWasPaused = false;
    }
    else if (sMenuId == "200")
    {
        setPenWidth(200.0);
        mbWasPaused = false;
    }
    else if (sMenuId == "400")
    {
        setPenWidth(400.0);
        mbWasPaused = false;
    }
    else if (sMenuId == "erase")
    {
        setEraseAllInk(true);
        mbWasPaused = false;
    }
    else if (sMenuId == "pen")
    {
        setUsePen(!mbUsePen);
        mbWasPaused = false;
    }
    else if (sMenuId == "edit")
    {
        // When in autoplay mode (pps/ppsx), offer editing of the presentation
        // Turn autostart off, else Impress will close when exiting the Presentation
        mpViewShell->GetDoc()->SetExitAfterPresenting(false);
        if( mpSlideController.get() && (ANIMATIONMODE_SHOW == meAnimationMode) )
        {
            if( mpSlideController->getCurrentSlideNumber() != -1 )
            {
                mnRestoreSlide = mpSlideController->getCurrentSlideNumber();
            }
        }
        endPresentation();
    }
    else if (sMenuId == "end")
    {
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
    }
    else
    {
        sal_Int32 nPageNumber = pMenu->GetCurItemId() - CM_SLIDES;
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
    }

    return false;
}

Reference< XSlideShow > SlideshowImpl::createSlideShow()
{
    Reference< XSlideShow > xShow;

    try
    {
        Reference< uno::XComponentContext > xContext =
            ::comphelper::getProcessComponentContext();

        xShow.set( presentation::SlideShow::create(xContext), UNO_QUERY_THROW );
    }
    catch( uno::Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::createSlideShow(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }

    return xShow;
}

void SlideshowImpl::createSlideList( bool bAll, const OUString& rPresSlide )
{
    const sal_uInt16 nSlideCount = mpDoc->GetSdPageCount( PageKind::Standard );

    if( !nSlideCount )
        return;

    SdCustomShow*   pCustomShow;

    if( mpDoc->GetCustomShowList() && maPresSettings.mbCustomShow )
        pCustomShow = mpDoc->GetCustomShowList()->GetCurObject();
    else
        pCustomShow = nullptr;

    // create animation slide controller
    AnimationSlideController::Mode eMode =
        ( pCustomShow && !pCustomShow->PagesVector().empty() ) ? AnimationSlideController::CUSTOM :
            (bAll ? AnimationSlideController::ALL : AnimationSlideController::FROM);

    Reference< XDrawPagesSupplier > xDrawPages( mpDoc->getUnoModel(), UNO_QUERY_THROW );
    Reference< XIndexAccess > xSlides( xDrawPages->getDrawPages(), UNO_QUERY_THROW );
    mpSlideController.reset( new AnimationSlideController( xSlides, eMode ) );

    if( eMode != AnimationSlideController::CUSTOM )
    {
        sal_Int32 nFirstVisibleSlide = 0;

        // normal presentation
        if( !rPresSlide.isEmpty() )
        {
            sal_Int32 nSlide;
            bool bTakeNextAvailable = false;

            for( nSlide = 0, nFirstVisibleSlide = -1;
                ( nSlide < nSlideCount ) && ( -1 == nFirstVisibleSlide ); nSlide++ )
            {
                SdPage* pTestSlide = mpDoc->GetSdPage( static_cast<sal_uInt16>(nSlide), PageKind::Standard );

                if( pTestSlide->GetName() == rPresSlide )
                {
                    if( pTestSlide->IsExcluded() )
                        bTakeNextAvailable = true;
                    else
                        nFirstVisibleSlide = nSlide;
                }
                else if( bTakeNextAvailable && !pTestSlide->IsExcluded() )
                    nFirstVisibleSlide = nSlide;
            }

            if( -1 == nFirstVisibleSlide )
                nFirstVisibleSlide = 0;
        }

        for( sal_Int32 i = 0; i < nSlideCount; i++ )
        {
            bool bVisible = ! mpDoc->GetSdPage( static_cast<sal_uInt16>(i), PageKind::Standard )->IsExcluded();
            if( bVisible || (eMode == AnimationSlideController::ALL) )
                mpSlideController->insertSlideNumber( i, bVisible );
        }

        mpSlideController->setStartSlideNumber( nFirstVisibleSlide );
    }
    else
    {
        if( meAnimationMode != ANIMATIONMODE_SHOW && !rPresSlide.isEmpty() )
        {
            sal_Int32 nSlide;
            for( nSlide = 0; nSlide < nSlideCount; nSlide++ )
                if( rPresSlide == mpDoc->GetSdPage( static_cast<sal_uInt16>(nSlide), PageKind::Standard )->GetName() )
                    break;

            if( nSlide < nSlideCount )
                mpSlideController->insertSlideNumber( static_cast<sal_uInt16>(nSlide) );
        }

        for( const auto& rpPage : pCustomShow->PagesVector() )
        {
            const sal_uInt16 nSdSlide = ( rpPage->GetPageNum() - 1 ) / 2;

            if( ! mpDoc->GetSdPage( nSdSlide, PageKind::Standard )->IsExcluded())
                mpSlideController->insertSlideNumber( nSdSlide );
        }
    }
}

typedef sal_uInt16 (*FncGetChildWindowId)();

static const FncGetChildWindowId aShowChildren[] =
{
    &AnimationChildWindow::GetChildWindowId,
    &Svx3DChildWindow::GetChildWindowId,
    &SvxFontWorkChildWindow::GetChildWindowId,
    &SvxColorChildWindow::GetChildWindowId,
    &SvxSearchDialogWrapper::GetChildWindowId,
    &SvxBmpMaskChildWindow::GetChildWindowId,
    &SvxIMapDlgChildWindow::GetChildWindowId,
    &SvxHlinkDlgWrapper::GetChildWindowId,
    &SfxInfoBarContainerChild::GetChildWindowId
};

void SlideshowImpl::hideChildWindows()
{
    mnChildMask = 0;

    if( ANIMATIONMODE_SHOW != meAnimationMode )
        return;

    SfxViewFrame* pViewFrame = getViewFrame();

    if( !pViewFrame )
        return;

    for( sal_uLong i = 0; i < SAL_N_ELEMENTS( aShowChildren ); i++ )
    {
        const sal_uInt16 nId = ( *aShowChildren[ i ] )();

        if( pViewFrame->GetChildWindow( nId ) )
        {
            pViewFrame->SetChildWindow( nId, false );
            mnChildMask |= 1 << i;
        }
    }
}

void SlideshowImpl::showChildWindows()
{
    if( ANIMATIONMODE_SHOW == meAnimationMode )
    {
        SfxViewFrame* pViewFrame = getViewFrame();
        if( pViewFrame )
        {
            for( sal_uLong i = 0; i < SAL_N_ELEMENTS(aShowChildren); i++ )
            {
                if( mnChildMask & ( 1 << i ) )
                    pViewFrame->SetChildWindow( ( *aShowChildren[ i ] )(), true );
            }
        }
    }
}

SfxViewFrame* SlideshowImpl::getViewFrame() const
{
    return mpViewShell ? mpViewShell->GetViewFrame() : nullptr;
}

SfxDispatcher* SlideshowImpl::getDispatcher() const
{
    return (mpViewShell && mpViewShell->GetViewFrame()) ? mpViewShell->GetViewFrame()->GetDispatcher() : nullptr;
}

SfxBindings* SlideshowImpl::getBindings() const
{
    return (mpViewShell && mpViewShell->GetViewFrame()) ? &mpViewShell->GetViewFrame()->GetBindings() : nullptr;
}

void SlideshowImpl::resize( const Size& rSize )
{
    maPresSize = rSize;

    if(mpShowWindow)
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
        SAL_WARN( "sd", "sd::SlideshowImpl::resize(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

void SlideshowImpl::setActiveXToolbarsVisible( bool bVisible )
{
    // in case of ActiveX control the toolbars should not be visible if slide show runs in window mode
    // actually it runs always in window mode in case of ActiveX control
    if ( !(!maPresSettings.mbFullScreen && mpDocSh && mpDocSh->GetMedium()) )
        return;

    const SfxBoolItem* pItem = SfxItemSet::GetItem<SfxBoolItem>(mpDocSh->GetMedium()->GetItemSet(), SID_VIEWONLY, false);
    if ( !(pItem && pItem->GetValue()) )
        return;

    // this is a plugin/activex mode, no toolbars should be visible during slide show
    // after the end of slide show they should be visible again
    SfxViewFrame* pViewFrame = getViewFrame();
    if( !pViewFrame )
        return;

    try
    {
        Reference< frame::XLayoutManager > xLayoutManager;
        Reference< beans::XPropertySet > xFrameProps( pViewFrame->GetFrame().GetFrameInterface(), UNO_QUERY_THROW );
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

void SAL_CALL SlideshowImpl::activate()
{
    SolarMutexGuard aSolarGuard;

    maDeactivateTimer.Stop();

    if( !(!mbActive && mxShow.is()) )
        return;

    mbActive = true;

    if( ANIMATIONMODE_SHOW == meAnimationMode )
    {
        if( mbAutoSaveWasOn )
            setAutoSaveState( false );

        if( mpShowWindow )
        {
            SfxViewFrame* pViewFrame = getViewFrame();
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : nullptr;

            hideChildWindows();

            if( pDispatcher )
            {
                // filter all forbidden slots
                pDispatcher->SetSlotFilter( SfxSlotFilterState::ENABLED, pAllowed );
            }

            if( getBindings() )
                getBindings()->InvalidateAll(true);

            mpShowWindow->GrabFocus();
        }
    }

    resume();
}

void SAL_CALL SlideshowImpl::deactivate()
{
    SolarMutexGuard aSolarGuard;

    if( mbActive && mxShow.is() )
    {
        maDeactivateTimer.Start();
    }
}

IMPL_LINK_NOARG(SlideshowImpl, deactivateHdl, Timer *, void)
{
    if( !(mbActive && mxShow.is()) )
        return;

    mbActive = false;

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

sal_Bool SAL_CALL SlideshowImpl::isActive()
{
    SolarMutexGuard aSolarGuard;
    return mbActive;
}

void SlideshowImpl::setAutoSaveState( bool bOn)
{
    try
    {
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

        uno::Reference< util::XURLTransformer > xParser(util::URLTransformer::create(xContext));
        util::URL aURL;
        aURL.Complete = "vnd.sun.star.autorecovery:/setAutoSaveState";
        xParser->parseStrict(aURL);

        Sequence< beans::PropertyValue > aArgs(1);
        aArgs[0].Name = "AutoSaveState";
        aArgs[0].Value <<= bOn;

        uno::Reference< frame::XDispatch > xAutoSave = frame::theAutoRecovery::get(xContext);
        xAutoSave->dispatch(aURL, aArgs);
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::SlideshowImpl::setAutoSaveState(), exception caught!");
    }
}

Reference< XDrawPage > SAL_CALL SlideshowImpl::getCurrentSlide()
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

sal_Int32 SAL_CALL SlideshowImpl::getNextSlideIndex()
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

sal_Int32 SAL_CALL SlideshowImpl::getCurrentSlideIndex()
{
    return mpSlideController.get() ? mpSlideController->getCurrentSlideIndex() : -1;
}

// css::presentation::XSlideShowController:

::sal_Int32 SAL_CALL SlideshowImpl::getSlideCount()
{
    return mpSlideController.get() ? mpSlideController->getSlideIndexCount() : 0;
}

Reference< XDrawPage > SAL_CALL SlideshowImpl::getSlideByIndex(::sal_Int32 Index)
{
    if ((mpSlideController == nullptr) || (Index < 0)
        || (Index >= mpSlideController->getSlideIndexCount()))
        throw IndexOutOfBoundsException();

    return mpSlideController->getSlideByNumber( mpSlideController->getSlideNumber( Index ) );
}

sal_Bool SAL_CALL SlideshowImpl::getAlwaysOnTop()
{
    SolarMutexGuard aSolarGuard;
    return maPresSettings.mbAlwaysOnTop;
}

void SAL_CALL SlideshowImpl::setAlwaysOnTop( sal_Bool bAlways )
{
    SolarMutexGuard aSolarGuard;
    if( maPresSettings.mbAlwaysOnTop != bool(bAlways) )
    {
        maPresSettings.mbAlwaysOnTop = bAlways;
        // todo, can this be changed while running?
    }
}

sal_Bool SAL_CALL SlideshowImpl::isFullScreen()
{
    SolarMutexGuard aSolarGuard;
    return maPresSettings.mbFullScreen;
}

sal_Bool SAL_CALL SlideshowImpl::getMouseVisible()
{
    SolarMutexGuard aSolarGuard;
    return maPresSettings.mbMouseVisible;
}

void SAL_CALL SlideshowImpl::setMouseVisible( sal_Bool bVisible )
{
    SolarMutexGuard aSolarGuard;
    if( maPresSettings.mbMouseVisible != bool(bVisible) )
    {
        maPresSettings.mbMouseVisible = bVisible;
        if( mpShowWindow )
            mpShowWindow->SetMouseAutoHide( !maPresSettings.mbMouseVisible );
    }
}

sal_Bool SAL_CALL SlideshowImpl::getUsePen()
{
    SolarMutexGuard aSolarGuard;
    return mbUsePen;
}

void SAL_CALL SlideshowImpl::setUsePen( sal_Bool bMouseAsPen )
{
    SolarMutexGuard aSolarGuard;
    mbUsePen = bMouseAsPen;
    if( !mxShow.is() )
        return;

    try
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
            aPenPropSwitchPenMode.Value <<= true;
            mxShow->setProperty( aPenPropSwitchPenMode );
        }
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::setUsePen(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

double SAL_CALL SlideshowImpl::getPenWidth()
{
    SolarMutexGuard aSolarGuard;
    return mdUserPaintStrokeWidth;
}

void SAL_CALL SlideshowImpl::setPenWidth( double dStrokeWidth )
{
    SolarMutexGuard aSolarGuard;
    mdUserPaintStrokeWidth = dStrokeWidth;
    setUsePen( true ); // enable pen mode, update color and width
}

sal_Int32 SAL_CALL SlideshowImpl::getPenColor()
{
    SolarMutexGuard aSolarGuard;
    return mnUserPaintColor;
}

void SAL_CALL SlideshowImpl::setPenColor( sal_Int32 nColor )
{
    SolarMutexGuard aSolarGuard;
    mnUserPaintColor = nColor;
    setUsePen( true ); // enable pen mode, update color
}

void SlideshowImpl::setEraseAllInk(bool bEraseAllInk)
{
    if( !bEraseAllInk )
        return;

    SolarMutexGuard aSolarGuard;
    if( !mxShow.is() )
        return;

    try
    {
        beans::PropertyValue aPenPropEraseAllInk;
        aPenPropEraseAllInk.Name = "EraseAllInk";
        aPenPropEraseAllInk.Value <<= bEraseAllInk;
        mxShow->setProperty( aPenPropEraseAllInk );
    }
    catch( Exception& )
    {
        SAL_WARN( "sd.slideshow", "sd::SlideshowImpl::setEraseAllInk(), "
            "exception caught: " << exceptionToString( cppu::getCaughtException() ));
    }
}

// XSlideShowController Methods
sal_Bool SAL_CALL SlideshowImpl::isRunning(  )
{
    SolarMutexGuard aSolarGuard;
    return mxShow.is();
}

void SAL_CALL SlideshowImpl::gotoNextEffect(  )
{
    SolarMutexGuard aSolarGuard;

    if( !(mxShow.is() && mpSlideController.get() && mpShowWindow) )
        return;

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

void SAL_CALL SlideshowImpl::gotoPreviousEffect(  )
{
    SolarMutexGuard aSolarGuard;

    if( !(mxShow.is() && mpSlideController.get() && mpShowWindow) )
        return;

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

void SAL_CALL SlideshowImpl::gotoFirstSlide(  )
{
    SolarMutexGuard aSolarGuard;

    if( !(mpShowWindow && mpSlideController.get()) )
        return;

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

void SAL_CALL SlideshowImpl::gotoNextSlide(  )
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
        // start 20ms timer to reenable inputs to filter
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
                            if ( maPresSettings.mbShowPauseLogo )
                            {
                                Graphic aGraphic(SfxApplication::GetApplicationLogo(360));
                                mpShowWindow->SetPauseMode( maPresSettings.mnPauseTimeout, &aGraphic );
                            }
                            else
                                mpShowWindow->SetPauseMode( maPresSettings.mnPauseTimeout );
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

void SAL_CALL SlideshowImpl::gotoPreviousSlide(  )
{
    gotoPreviousSlide(false);
}

void SlideshowImpl::gotoPreviousSlide (const bool bSkipAllMainSequenceEffects)
{
    SolarMutexGuard aSolarGuard;

    if( !(mxShow.is() && mpSlideController.get()) )
        return;

    try
    {
        if( mbIsPaused )
            resume();

        const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
        if( eMode == SHOWWINDOWMODE_END )
        {
            mpShowWindow->RestartShow( mpSlideController->getCurrentSlideIndex() );
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
                displayCurrentSlide();
            }
        }
    }
    catch( Exception& )
    {
        SAL_WARN( "sd", "sd::SlideshowImpl::gotoPreviousSlide(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

void SAL_CALL SlideshowImpl::gotoLastSlide()
{
    SolarMutexGuard aSolarGuard;

    if( !mpSlideController.get() )
        return;

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

void SAL_CALL SlideshowImpl::gotoBookmark( const OUString& rBookmark )
{
    SolarMutexGuard aSolarGuard;

    if( mbIsPaused )
        resume();

    sal_Int32 nSlideNumber = getSlideNumberForBookmark( rBookmark );
    if( nSlideNumber != -1 )
        displaySlideNumber( nSlideNumber );
}

void SAL_CALL SlideshowImpl::gotoSlide( const Reference< XDrawPage >& xSlide )
{
    SolarMutexGuard aSolarGuard;

    if( !(mpSlideController.get() && xSlide.is()) )
        return;

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

void SAL_CALL SlideshowImpl::gotoSlideIndex( sal_Int32 nIndex )
{
    SolarMutexGuard aSolarGuard;

    if( mbIsPaused )
        resume();

    displaySlideIndex( nIndex );
}

void SAL_CALL SlideshowImpl::stopSound(  )
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
        SAL_WARN( "sd", "sd::SlideshowImpl::stopSound(), exception caught: "
                    << exceptionToString( cppu::getCaughtException() ) );
    }
}

// XIndexAccess

::sal_Int32 SAL_CALL SlideshowImpl::getCount(  )
{
    return getSlideCount();
}

css::uno::Any SAL_CALL SlideshowImpl::getByIndex( ::sal_Int32 Index )
{
    return Any( getSlideByIndex( Index ) );
}

css::uno::Type SAL_CALL SlideshowImpl::getElementType(  )
{
    return cppu::UnoType<XDrawPage>::get();
}

sal_Bool SAL_CALL SlideshowImpl::hasElements(  )
{
    return getSlideCount() != 0;
}

Reference< XSlideShow > SAL_CALL SlideshowImpl::getSlideShow()
{
    return mxShow;
}

PresentationSettingsEx::PresentationSettingsEx( const PresentationSettingsEx& r )
: PresentationSettings( r )
, mbRehearseTimings(r.mbRehearseTimings)
, mbPreview(r.mbPreview)
, mpParentWindow( nullptr )
{
}

PresentationSettingsEx::PresentationSettingsEx( PresentationSettings const & r )
: PresentationSettings( r )
, mbRehearseTimings(false)
, mbPreview(false)
, mpParentWindow(nullptr)
{
}

void PresentationSettingsEx::SetArguments( const Sequence< PropertyValue >& rArguments )
{
    sal_Int32 nArguments = rArguments.getLength();
    const PropertyValue* pValue = rArguments.getConstArray();

    while( nArguments-- )
    {
        SetPropertyValue( pValue->Name, pValue->Value );
        pValue++;
    }
}

void PresentationSettingsEx::SetPropertyValue( const OUString& rProperty, const Any& rValue )
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
            mpParentWindow = xWindow.is() ? VCLUnoHelper::GetWindow( xWindow )
                                          : VclPtr<vcl::Window>();
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
            mbCustomShow = false;
            mbAll = false;
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
    else if ( rProperty == "UsePen" )
    {
        if( rValue >>= mbMouseAsPen )
            return;
    }
    throw IllegalArgumentException();
}

// XAnimationListener

SlideShowListenerProxy::SlideShowListenerProxy( const rtl::Reference< SlideshowImpl >& xController, const css::uno::Reference< css::presentation::XSlideShow >& xSlideShow )
: maListeners( m_aMutex )
, mxController( xController )
, mxSlideShow( xSlideShow )
{
}

SlideShowListenerProxy::~SlideShowListenerProxy()
{
}

void SlideShowListenerProxy::addAsSlideShowListener()
{
    if( mxSlideShow.is() )
    {
        Reference< XSlideShowListener > xSlideShowListener( this );
        mxSlideShow->addSlideShowListener( xSlideShowListener );
    }
}

void SlideShowListenerProxy::removeAsSlideShowListener()
{
    if( mxSlideShow.is() )
    {
        Reference< XSlideShowListener > xSlideShowListener( this );
        mxSlideShow->removeSlideShowListener( xSlideShowListener );
    }
}

void SlideShowListenerProxy::addShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape )
{
    if( mxSlideShow.is() )
    {
        Reference< XShapeEventListener > xListener( this );
        mxSlideShow->addShapeEventListener( xListener, xShape );
    }
}

void SlideShowListenerProxy::removeShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape )
{
    if( mxSlideShow.is() )
    {
        Reference< XShapeEventListener > xListener( this );
        mxSlideShow->removeShapeEventListener( xListener, xShape );
    }
}

void SlideShowListenerProxy::addSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& xListener )
{
    maListeners.addInterface(xListener);
}

void SlideShowListenerProxy::removeSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& xListener )
{
    maListeners.removeInterface(xListener);
}

void SAL_CALL SlideShowListenerProxy::beginEvent( const Reference< XAnimationNode >& xNode )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
    {
        maListeners.forEach<XSlideShowListener>(
            [&] (Reference<XAnimationListener> const& xListener) {
                return xListener->beginEvent(xNode);
            } );
    }
}

void SAL_CALL SlideShowListenerProxy::endEvent( const Reference< XAnimationNode >& xNode )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
    {
        maListeners.forEach<XSlideShowListener>(
            [&] (Reference<XAnimationListener> const& xListener) {
                return xListener->endEvent(xNode);
            } );
    }
}

void SAL_CALL SlideShowListenerProxy::repeat( const Reference< XAnimationNode >& xNode, ::sal_Int32 nRepeat )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( maListeners.getLength() >= 0 )
    {
        maListeners.forEach<XSlideShowListener>(
            [&] (Reference<XAnimationListener> const& xListener) {
                return xListener->repeat(xNode, nRepeat);
            } );
    }
}

// css::presentation::XSlideShowListener:

void SAL_CALL SlideShowListenerProxy::paused(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    maListeners.forEach<XSlideShowListener>(
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->paused();
        });
}

void SAL_CALL SlideShowListenerProxy::resumed(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    maListeners.forEach<XSlideShowListener>(
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->resumed();
        });
}

void SAL_CALL SlideShowListenerProxy::slideTransitionStarted( )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    maListeners.forEach<XSlideShowListener>(
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideTransitionStarted();
        });
}

void SAL_CALL SlideShowListenerProxy::slideTransitionEnded( )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    maListeners.forEach<XSlideShowListener>(
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideTransitionEnded ();
        });
}

void SAL_CALL SlideShowListenerProxy::slideAnimationsEnded(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    maListeners.forEach<XSlideShowListener>(
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideAnimationsEnded ();
        });
}

void SlideShowListenerProxy::slideEnded(sal_Bool bReverse)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if( maListeners.getLength() >= 0 )
        {
            maListeners.forEach<XSlideShowListener>(
                [&] (Reference<XSlideShowListener> const& xListener) {
                    return xListener->slideEnded(bReverse);
                } );
        }
    }

    {
        SolarMutexGuard aSolarGuard;
        if( mxController.is() )
            mxController->slideEnded(bReverse);
    }
}

void SlideShowListenerProxy::hyperLinkClicked( OUString const& aHyperLink )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if( maListeners.getLength() >= 0 )
        {
            maListeners.forEach<XSlideShowListener>(
                [&] (Reference<XSlideShowListener> const& xListener) {
                    return xListener->hyperLinkClicked(aHyperLink);
                } );
        }
    }

    {
        SolarMutexGuard aSolarGuard;
        if( mxController.is() )
            mxController->hyperLinkClicked(aHyperLink);
    }
}

// XEventListener

void SAL_CALL SlideShowListenerProxy::disposing( const css::lang::EventObject& aDisposeEvent )
{
    maListeners.disposeAndClear( aDisposeEvent );
    mxController.clear();
    mxSlideShow.clear();
}

// XShapeEventListener

void SAL_CALL SlideShowListenerProxy::click( const Reference< XShape >& xShape, const css::awt::MouseEvent& /*aOriginalEvent*/ )
{
    SolarMutexGuard aSolarGuard;
    if( mxController.is() )
        mxController->click(xShape );
}

} // namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
