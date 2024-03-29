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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/presentation/SlideShow.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Recovery.hxx>
#include <svl/stritem.hxx>
#include <svl/urihelper.hxx>
#include <basic/sbstar.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/sequence.hxx>

#include <sfx2/infobar.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/svdoole2.hxx>
#include <svx/f3dchild.hxx>
#include <svx/imapdlg.hxx>
#include <svx/fontwork.hxx>
#include <svx/SvxColorChildWindow.hxx>
#include <svx/bmpmask.hxx>
#include <svx/srchdlg.hxx>
#include <svx/hyperdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/unoapi.hxx>
#include <AnimationChildWindow.hxx>
#include <notifydocumentevent.hxx>
#include "slideshowimpl.hxx"
#include "slideshowviewimpl.hxx"
#include "PaneHider.hxx"

#include <bitmaps.hlst>
#include <strings.hrc>
#include <sdresid.hxx>
#include <utility>
#include <vcl/canvastools.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/weldutils.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <rtl/ref.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
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
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::beans;

namespace sd
{
/** Slots, which will be disabled in the slide show and are managed by Sfx.
    Have to be sorted in the order of the SIDs */
sal_uInt16 const pAllowed[] =
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

    // for InteractiveSlideShow we need to temporarily change the program
    // and mode, so allow save/restore that settings
    void pushForPreview();
    void popFromPreview();
private:
    bool getSlideAPI( sal_Int32 nSlideNumber, Reference< XDrawPage >& xSlide, Reference< XAnimationNode >& xAnimNode );
    sal_Int32 findSlideIndex( sal_Int32 nSlideNumber ) const;

    bool isValidIndex( sal_Int32 nIndex ) const { return (nIndex >= 0) && (o3tl::make_unsigned(nIndex) < maSlideNumbers.size()); }
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

    // IASS data for push/pop
    std::vector< sal_Int32 > maSlideNumbers2;
    std::vector< bool > maSlideVisible2;
    std::vector< bool > maSlideVisited2;
    Reference< XAnimationNode > mxPreviewNode2;
    Mode meMode2;
};

void AnimationSlideController::pushForPreview()
{
    maSlideNumbers2 = maSlideNumbers;
    maSlideVisible2 = maSlideVisible;
    maSlideVisited2 = maSlideVisited;
    maSlideNumbers.clear();
    maSlideVisible.clear();
    maSlideVisited.clear();
    mxPreviewNode2 = mxPreviewNode;
    meMode2 = meMode;
    meMode = AnimationSlideController::PREVIEW;
}

void AnimationSlideController::popFromPreview()
{
    maSlideNumbers = maSlideNumbers2;
    maSlideVisible = maSlideVisible2;
    maSlideVisited = maSlideVisited2;
    maSlideNumbers2.clear();
    maSlideVisible2.clear();
    maSlideVisited2.clear();
    mxPreviewNode = mxPreviewNode2;
    meMode = meMode2;
}

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
,   meMode2( eMode )
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::AnimationSlideController::getSlideAPI()" );
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
        Sequence< Any > aValue{ Any(xSlide), Any(xAnimNode) };
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

constexpr OUString gsOnClick( u"OnClick"_ustr );
constexpr OUString gsBookmark( u"Bookmark"_ustr );
constexpr OUString gsVerb( u"Verb"_ustr );

SlideshowImpl::SlideshowImpl( const Reference< XPresentation2 >& xPresentation, ViewShell* pViewSh, ::sd::View* pView, SdDrawDocument* pDoc, vcl::Window* pParentWindow )
: mxShow()
, mxView()
, mxModel(pDoc->getUnoModel())
, maUpdateTimer("SlideShowImpl maUpdateTimer")
, maInputFreezeTimer("SlideShowImpl maInputFreezeTimer")
, maDeactivateTimer("SlideShowImpl maDeactivateTimer")
, mpView(pView)
, mpViewShell(pViewSh)
, mpDocSh(pDoc->GetDocSh())
, mpDoc(pDoc)
, mpParentWindow(pParentWindow)
, mpShowWindow(nullptr)
, mpSlideController()
, mnRestoreSlide(0)
, maPopupMousePos()
, maPresSize( -1, -1 )
, meAnimationMode(ANIMATIONMODE_SHOW)
, maCharBuffer()
, mpOldActiveWindow(nullptr)
, maStarBASICGlobalErrorHdl()
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
, maShapeEventMap()
, mxPreviewDrawPage()
, mxPreviewAnimationNode()
, mxPlayer()
, mpPaneHider()
, mnEndShowEvent(nullptr)
, mnContextMenuEvent(nullptr)
, mnEventObjectChange(nullptr)
, mnEventObjectInserted(nullptr)
, mnEventObjectRemoved(nullptr)
, mnEventPageOrderChange(nullptr)
, mxPresentation( xPresentation )
, mxListenerProxy()
, mxShow2()
, mxView2()
, meAnimationMode2()
, mbInterActiveSetup(false)
, maPresSettings2()
, mxPreviewDrawPage2()
, mxPreviewAnimationNode2()
, mnSlideIndex(0)
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

        // no autosave during show
    if (officecfg::Office::Recovery::AutoSave::Enabled::get())
        mbAutoSaveWasOn = true;

    Application::AddEventListener( LINK( this, SlideshowImpl, EventListenerHdl ) );

    mbUsePen = maPresSettings.mbMouseAsPen;

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
    if( pOptions )
    {
        mnUserPaintColor = pOptions->GetPresentationPenColor();
        mdUserPaintStrokeWidth = pOptions->GetPresentationPenWidth();
    }

    // to be able to react on various changes in the DrawModel, this class
    // is now derived from SfxListener and registers itself at the DrawModel
    if (nullptr != mpDoc)
        StartListening(*mpDoc);
}

SlideshowImpl::~SlideshowImpl()
{
    // stop listening to DrawModel (see above)
    if (nullptr != mpDoc)
        EndListening(*mpDoc);

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
        std::unique_lock g(m_aMutex);
        disposing(g);
    }
}

void SlideshowImpl::disposing(std::unique_lock<std::mutex>&)
{
#ifdef ENABLE_SDREMOTE
    RemoteServer::presentationStopped();
#endif
    // IASS: This is the central methodology to 'steer' the
    // PresenterConsole - in this case, to shut it down
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
    if( mnEventObjectChange )
        Application::RemoveUserEvent( mnEventObjectChange );
    if( mnEventObjectInserted )
        Application::RemoveUserEvent( mnEventObjectInserted );
    if( mnEventObjectRemoved )
        Application::RemoveUserEvent( mnEventObjectRemoved );
    if( mnEventPageOrderChange )
        Application::RemoveUserEvent( mnEventPageOrderChange );

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
            mxShow->removeView( mxView );

        Reference< XComponent > xComponent( mxShow, UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();

        if( mxView.is() )
            mxView->dispose();
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::stop()" );
    }

    mxShow.clear();
    mxView.clear();
    mxListenerProxy.clear();
    mpSlideController.reset();

    // take DrawView from presentation window, but give the old window back
    if( mpShowWindow && mpView )
        mpView->DeleteDeviceFromPaintView( *mpShowWindow->GetOutDev() );

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
                    mpView->VisAreaChanged(pActWin->GetOutDev());
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

bool SlideshowImpl::isInteractiveSetup() const
{
    return mbInterActiveSetup;
}

void SlideshowImpl::startInteractivePreview( const Reference< XDrawPage >& xDrawPage, const Reference< XAnimationNode >& xAnimationNode )
{
    // set flag that we are in IASS mode
    mbInterActiveSetup = true;

    // save stuff that will be replaced temporarily
    mxShow2 = mxShow;
    mxView2 = mxView;
    mxPreviewDrawPage2 = mxPreviewDrawPage;
    mxPreviewAnimationNode2 = mxPreviewAnimationNode;
    meAnimationMode2 = meAnimationMode;
    maPresSettings2 = maPresSettings;

    // remember slide shown before preview
    mnSlideIndex = getCurrentSlideIndex();

    // set DrawPage/AnimationNode
    mxPreviewDrawPage = xDrawPage;
    mxPreviewAnimationNode = xAnimationNode;
    meAnimationMode = ANIMATIONMODE_PREVIEW;

    // set PresSettings for preview
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

    // create a new temporary AnimationSlideController
    mpSlideController->pushForPreview();
    // Reference< XDrawPagesSupplier > xDrawPages( mpDoc->getUnoModel(), UNO_QUERY_THROW );
    // Reference< XIndexAccess > xSlides( xDrawPages->getDrawPages(), UNO_QUERY_THROW );
    // mpSlideController = std::make_shared<AnimationSlideController>( xSlides, AnimationSlideController::PREVIEW );
    sal_Int32 nSlideNumber = 0;
    Reference< XPropertySet > xSet( xDrawPage, UNO_QUERY_THROW );
    xSet->getPropertyValue( "Number" ) >>= nSlideNumber;
    mpSlideController->insertSlideNumber( nSlideNumber-1 );
    mpSlideController->setPreviewNode( xAnimationNode );

    // prepare properties
    sal_Int32 nPropertyCount = 1;
    if( xAnimationNode.is() )
        nPropertyCount++;
    Sequence< beans::PropertyValue > aProperties(nPropertyCount);
    auto pProperties = aProperties.getArray();
    pProperties[0].Name = "AutomaticAdvancement";
    pProperties[0].Value <<= 1.0; // one second timeout

    if( xAnimationNode.is() )
    {
        pProperties[1].Name = "NoSlideTransitions";
        pProperties[1].Value <<= true;
    }

    // start preview
    startShowImpl( aProperties );
}

void SlideshowImpl::endInteractivePreview()
{
    if (!mbInterActiveSetup)
        // not in use, nothing to do
        return;

    // cleanup Show/View
    try
    {
        if( mxView.is() )
            mxShow->removeView( mxView );

        Reference< XComponent > xComponent( mxShow, UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();

        if( mxView.is() )
            mxView->dispose();
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::stop()" );
    }
    mxShow.clear();
    mxView.clear();
    mxView = mxView2;
    mxShow = mxShow2;

    // restore SlideController
    mpSlideController->popFromPreview();

    // restore other settings and cleanup temporary incarnations
    maPresSettings = maPresSettings2;
    meAnimationMode = meAnimationMode2;
    mxPreviewAnimationNode = mxPreviewAnimationNode2;
    mxPreviewAnimationNode2.clear();
    mxPreviewDrawPage = mxPreviewDrawPage2;
    mxPreviewDrawPage2.clear();

    // go back to slide shown before preview
    gotoSlideIndex(mnSlideIndex);

    // reset IASS mode flag
    mbInterActiveSetup = false;
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
            if (comphelper::findValue(supportedServices, "com.sun.star.drawing.MasterPage") != -1) {
                OSL_FAIL("sd::SlideshowImpl::startPreview() "
                          "not allowed on master page!");
                return false;
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
        mpSlideController = std::make_shared<AnimationSlideController>( xSlides, AnimationSlideController::PREVIEW );

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
            mpView->AddDeviceToPaintView( *mpShowWindow->GetOutDev(), nullptr );
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
        auto pProperties = aProperties.getArray();
        pProperties[0].Name = "AutomaticAdvancement";
        pProperties[0].Value <<= 1.0; // one second timeout

        if( mxPreviewAnimationNode.is() )
        {
            pProperties[1].Name = "NoSlideTransitions";
            pProperties[1].Value <<= true;
        }

        bRet = startShowImpl( aProperties );

        if( mpShowWindow != nullptr && meAnimationMode == ANIMATIONMODE_PREVIEW )
            mpShowWindow->SetPreviewMode();

    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::startPreview()" );
        bRet = false;
    }

    return bRet;
}

bool SlideshowImpl::startShow( PresentationSettingsEx const * pPresSettings )
{
    const rtl::Reference<SlideshowImpl> xKeepAlive(this);

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
            if( mpDocSh && pPresSettings && !pPresSettings->mbInteractive) // IASS
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
                mpView->AddDeviceToPaintView( *mpShowWindow->GetOutDev(), nullptr );
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
                                      aProperties.data(), aProperties.size() ) );

        }

        setActiveXToolbarsVisible( false );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::startShow()" );
        bRet = false;
    }

    return bRet;
}

bool SlideshowImpl::startShowImpl( const Sequence< beans::PropertyValue >& aProperties )
{
    try
    {
        mxShow.set( createSlideShow(), UNO_SET_THROW );

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
                        Any( xBitmap ),
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
                        Any( xPointerBitmap ),
                        beans::PropertyState_DIRECT_VALUE ) );
            }
            if (officecfg::Office::Impress::Misc::Start::ShowNavigationPanel::get())
            {
                NavbarButtonSize btnScale = static_cast<NavbarButtonSize>(officecfg::Office::Impress::Layout::Display::NavigationBtnScale::get());
                OUString prevSlidePath = "";
                OUString nextSlidePath = "";
                OUString menuPath = "";
                switch (btnScale)
                {
                    case NavbarButtonSize::Large:
                    {
                        prevSlidePath = BMP_PREV_SLIDE_LARGE;
                        nextSlidePath = BMP_NEXT_SLIDE_LARGE;
                        menuPath = BMP_MENU_SLIDE_LARGE;
                        break;
                    }
                    case NavbarButtonSize::XLarge:
                    {
                        prevSlidePath = BMP_PREV_SLIDE_EXTRALARGE;
                        nextSlidePath = BMP_NEXT_SLIDE_EXTRALARGE;
                        menuPath = BMP_MENU_SLIDE_EXTRALARGE;
                        break;
                    }
                    case NavbarButtonSize::Auto:
                    case NavbarButtonSize::Small:
                    default:
                    {
                        prevSlidePath = BMP_PREV_SLIDE_SMALL;
                        nextSlidePath = BMP_NEXT_SLIDE_SMALL;
                        menuPath = BMP_MENU_SLIDE_SMALL;
                        break;
                    }
                }
                BitmapEx prevSlideBm(prevSlidePath);
                const Reference<rendering::XBitmap> xPrevSBitmap(
                    vcl::unotools::xBitmapFromBitmapEx(prevSlideBm));
                if (xPrevSBitmap.is())
                {
                    mxShow->setProperty(beans::PropertyValue("NavigationSlidePrev", -1,
                                                             Any(xPrevSBitmap),
                                                             beans::PropertyState_DIRECT_VALUE));
                }
                BitmapEx menuSlideBm(menuPath);
                const Reference<rendering::XBitmap> xMenuSBitmap(
                    vcl::unotools::xBitmapFromBitmapEx(menuSlideBm));
                if (xMenuSBitmap.is())
                {
                    mxShow->setProperty(beans::PropertyValue("NavigationSlideMenu", -1,
                                                             Any(xMenuSBitmap),
                                                             beans::PropertyState_DIRECT_VALUE));
                }
                BitmapEx nextSlideBm(nextSlidePath);
                const Reference<rendering::XBitmap> xNextSBitmap(
                    vcl::unotools::xBitmapFromBitmapEx(nextSlideBm));
                if (xNextSBitmap.is())
                {
                    mxShow->setProperty(beans::PropertyValue("NavigationSlideNext", -1,
                                                             Any(xNextSBitmap),
                                                             beans::PropertyState_DIRECT_VALUE));
                }
            }
        }

        for( const auto& rProp : aProperties )
            mxShow->setProperty( rProp );

        mxShow->addView( mxView );

        mxListenerProxy.set( new SlideShowListenerProxy( this, mxShow ) );
        mxListenerProxy->addAsSlideShowListener();

        // IASS: Do only startup the PresenterConsole if this is not
        // the SlideShow Preview mode (else would be double)
        if (!mbInterActiveSetup)
        {
            // IASS: This is the central methodology to 'steer' the
            // PresenterConsole - in this case, to start it up and make
            // it visible (if activated)
            NotifyDocumentEvent(
                *mpDoc,
                "OnStartPresentation");
        }

        displaySlideIndex( mpSlideController->getStartSlideIndex() );

        return true;
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::startShowImpl()" );
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::paint()" );
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

bool SlideshowImpl::swipe(const CommandGestureSwipeData &rSwipeData)
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

bool SlideshowImpl::longpress(const CommandGestureLongPressData &rLongPressData)
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::removeShapeEvents()" );
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
                Reference< XShapes > xMasterPage = xMasterPageTarget->getMasterPage();
                if( xMasterPage.is() )
                    registerShapeEvents( xMasterPage );
            }
            registerShapeEvents( xDrawPage );
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::registerShapeEvents()" );
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

            WrappedShapeEventImplPtr pEvent = std::make_shared<WrappedShapeEventImpl>();
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::registerShapeEvents()" );
    }
}

void SlideshowImpl::displayCurrentSlide (const bool bSkipAllMainSequenceEffects)
{
    stopSound();
    removeShapeEvents();

    if( mpSlideController && mxShow.is() )
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

    stopSound();

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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::pause()" );
    }
}

void SAL_CALL SlideshowImpl::resume()
{
    SolarMutexGuard aSolarGuard;

    if( mbIsPaused ) try
    {
        if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_BLANK || mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_END )
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::resume()" );
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
        if( mpShowWindow->SetBlankMode( mpSlideController->getCurrentSlideIndex(), Color(ColorTransparency, nColor) ) )
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
    if( !pEvent )
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
            mxPlayer.set(avmedia::MediaWindow::createPlayer(pEvent->maStrBookmark, ""/*TODO?*/), uno::UNO_SET_THROW );
            mxPlayer->start();
        }
        catch( uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::click()" );
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

            if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
            {
                SfxUnoFrameItem aDocFrame(SID_FILLFRAME, pViewFrm->GetFrame().GetFrameInterface());
                pViewFrm->GetDispatcher()->ExecuteList( SID_OPENDOC,
                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                    { &aUrl, &aBrowsing }, { &aDocFrame } );
            }
        }
    }
    break;

#if HAVE_FEATURE_SCRIPTING
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
            const std::u16string_view aMacroName = o3tl::getToken(aMacro, 0, '.', nIdx);
            const std::u16string_view aModulName = o3tl::getToken(aMacro, 0, '.', nIdx);

            // todo: is the limitation still given that only
            // Modulname+Macroname can be used here?
            OUString aExecMacro = OUString::Concat(aModulName) + "." + aMacroName;
            mpDocSh->GetBasic()->Call(aExecMacro);
        }
    }
    break;
#endif

    case ClickAction_VERB:
    {
        // todo, better do it async?
        SdrObject* pObj = SdrObject::getSdrObjectFromXShape(xShape);
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

void SlideshowImpl::contextMenuShow(const css::awt::Point& point)
{
    maPopupMousePos = { point.X, point.Y };
    mnContextMenuEvent = Application::PostUserEvent(LINK(this, SlideshowImpl, ContextMenuHdl));
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
    if( mpSlideController )
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
    if( mpSlideController )
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

sal_Int32 SlideshowImpl::getCurrentSlideNumber() const
{
    return mpSlideController ? mpSlideController->getCurrentSlideNumber() : -1;
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
    const rtl::Reference<SlideshowImpl> xKeepAlive(this);

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
            if (::basegfx::fTools::equalZero(fUpdate))
            {
                // Make sure idle tasks don't starve when we don't have to wait.
                // Don't process any events generated after invoking the function.
                Application::Reschedule(/*bHandleAllCurrentEvents=*/true);
            }
            else
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::updateSlideShow()" );
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
                if( mpSlideController && (ANIMATIONMODE_SHOW == meAnimationMode) )
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
            case KEY_XF86FORWARD:
                gotoNextEffect();
                break;

            case KEY_RETURN:
            {
                if( !maCharBuffer.isEmpty() )
                {
                    if( mpSlideController )
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
                maCharBuffer += OUStringChar( rKEvt.GetCharCode() );
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
            case KEY_BACKSPACE:
            case KEY_XF86BACK:
                gotoPreviousEffect();
                break;

            case KEY_P:
                setUsePen( !mbUsePen );
                break;

            // tdf#149351 Ctrl+A disables pointer as pen mode
            case KEY_A:
                if(rKEvt.GetKeyCode().IsMod1())
                {
                    setUsePen( false );
                    break;
                }
            break;

            case KEY_E:
                setEraseAllInk( true );
                updateSlideShow();
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::keyInput()" );
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
        if( mpSlideController && (ANIMATIONMODE_SHOW == meAnimationMode) )
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

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "modules/simpress/ui/slidecontextmenu.ui"));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu("menu"));
    OUString sNextImage(BMP_MENU_NEXT), sPrevImage(BMP_MENU_PREV);
    xMenu->insert(0, "next", SdResId(RID_SVXSTR_MENU_NEXT), &sNextImage, nullptr, nullptr, TRISTATE_INDET);
    xMenu->insert(1, "prev", SdResId(RID_SVXSTR_MENU_PREV), &sPrevImage, nullptr, nullptr, TRISTATE_INDET);

    // Adding button to display if in Pen  mode
    xMenu->set_active("pen", mbUsePen);

    const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
    xMenu->set_visible("next", mpSlideController->getNextSlideIndex() != -1);
    xMenu->set_visible("prev", (mpSlideController->getPreviousSlideIndex() != -1 ) || (eMode == SHOWWINDOWMODE_END) || (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK));
    xMenu->set_visible("edit", mpViewShell->GetDoc()->IsStartWithPresentation());

    std::unique_ptr<weld::Menu> xPageMenu(xBuilder->weld_menu("gotomenu"));
    OUString sFirstImage(BMP_MENU_FIRST), sLastImage(BMP_MENU_LAST);
    xPageMenu->insert(0, "first", SdResId(RID_SVXSTR_MENU_FIRST), &sFirstImage, nullptr, nullptr, TRISTATE_INDET);
    xPageMenu->insert(1, "last", SdResId(RID_SVXSTR_MENU_LAST), &sLastImage, nullptr, nullptr, TRISTATE_INDET);

    // populate slide goto list
    const sal_Int32 nPageNumberCount = mpSlideController->getSlideNumberCount();
    if( nPageNumberCount <= 1 )
    {
        xMenu->set_visible("goto", false);
    }
    else
    {
        sal_Int32 nCurrentSlideNumber = mpSlideController->getCurrentSlideNumber();
        if( (eMode == SHOWWINDOWMODE_END) || (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) )
            nCurrentSlideNumber = -1;

        xPageMenu->set_visible("first", mpSlideController->getSlideNumber(0) != nCurrentSlideNumber);
        xPageMenu->set_visible("last", mpSlideController->getSlideNumber(mpSlideController->getSlideIndexCount() - 1) != nCurrentSlideNumber);

        sal_Int32 nPageNumber;

        for( nPageNumber = 0; nPageNumber < nPageNumberCount; nPageNumber++ )
        {
            if( mpSlideController->isVisibleSlideNumber( nPageNumber ) )
            {
                SdPage* pPage = mpDoc->GetSdPage(static_cast<sal_uInt16>(nPageNumber), PageKind::Standard);
                if (pPage)
                {
                    OUString sId(OUString::number(CM_SLIDES + nPageNumber));
                    xPageMenu->append_check(sId, pPage->GetName());
                    if (nPageNumber == nCurrentSlideNumber)
                        xPageMenu->set_active(sId, true);
                }
            }
        }
    }

    std::unique_ptr<weld::Menu> xBlankMenu(xBuilder->weld_menu("screenmenu"));

    if (mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_BLANK)
    {
        xBlankMenu->set_active((mpShowWindow->GetBlankColor() == COL_WHITE) ? "white" : "black", true);
    }

    std::unique_ptr<weld::Menu> xWidthMenu(xBuilder->weld_menu("widthmenu"));

    // populate color width list
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
            xWidthMenu->set_active(OUString::number(nWidth), true);
    }

    ::tools::Rectangle aRect(maPopupMousePos, Size(1,1));
    weld::Window* pParent = weld::GetPopupParent(*mpShowWindow, aRect);
    ContextMenuSelectHdl(xMenu->popup_at_rect(pParent, aRect));

    if( mxView.is() )
        mxView->ignoreNextMouseReleased();

    if( !mbWasPaused )
        resume();
}

void SlideshowImpl::ContextMenuSelectHdl(std::u16string_view rMenuId)
{
    if (rMenuId == u"prev")
    {
        gotoPreviousSlide();
        mbWasPaused = false;
    }
    else if(rMenuId == u"next")
    {
        gotoNextSlide();
        mbWasPaused = false;
    }
    else if (rMenuId == u"first")
    {
        gotoFirstSlide();
        mbWasPaused = false;
    }
    else if (rMenuId == u"last")
    {
        gotoLastSlide();
        mbWasPaused = false;
    }
    else if (rMenuId == u"black" || rMenuId == u"white")
    {
        const Color aBlankColor(rMenuId == u"white" ? COL_WHITE : COL_BLACK);
        if( mbWasPaused )
        {
            if( mpShowWindow->GetShowWindowMode() == SHOWWINDOWMODE_BLANK )
            {
                if( mpShowWindow->GetBlankColor() == aBlankColor )
                {
                    mbWasPaused = false;
                    mpShowWindow->RestartShow();
                    return;
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
    else if (rMenuId == u"color")
    {
        //Open a color picker based on SvColorDialog
        ::Color aColor( ColorTransparency, mnUserPaintColor );
        SvColorDialog aColorDlg;
        aColorDlg.SetColor( aColor );

        if (aColorDlg.Execute(mpShowWindow->GetFrameWeld()))
        {
            aColor = aColorDlg.GetColor();
            setPenColor(sal_Int32(aColor));
        }
        mbWasPaused = false;
    }
    else if (rMenuId == u"4")
    {
        setPenWidth(4.0);
        mbWasPaused = false;
    }
    else if (rMenuId == u"100")
    {
        setPenWidth(100.0);
        mbWasPaused = false;
    }
    else if (rMenuId == u"150")
    {
        setPenWidth(150.0);
        mbWasPaused = false;
    }
    else if (rMenuId == u"200")
    {
        setPenWidth(200.0);
        mbWasPaused = false;
    }
    else if (rMenuId == u"400")
    {
        setPenWidth(400.0);
        mbWasPaused = false;
    }
    else if (rMenuId == u"erase")
    {
        setEraseAllInk(true);
        mbWasPaused = false;
    }
    else if (rMenuId == u"pen")
    {
        setUsePen(!mbUsePen);
        mbWasPaused = false;
    }
    else if (rMenuId == u"edit")
    {
        // When in autoplay mode (pps/ppsx), offer editing of the presentation
        // Turn autostart off, else Impress will close when exiting the Presentation
        mpViewShell->GetDoc()->SetExitAfterPresenting(false);
        if( mpSlideController && (ANIMATIONMODE_SHOW == meAnimationMode) )
        {
            if( mpSlideController->getCurrentSlideNumber() != -1 )
            {
                mnRestoreSlide = mpSlideController->getCurrentSlideNumber();
            }
        }
        endPresentation();
    }
    else if (rMenuId == u"end")
    {
        // in case the user cancels the presentation, switch to current slide
        // in edit mode
        if( mpSlideController && (ANIMATIONMODE_SHOW == meAnimationMode) )
        {
            if( mpSlideController->getCurrentSlideNumber() != -1 )
            {
                mnRestoreSlide = mpSlideController->getCurrentSlideNumber();
            }
        }
        endPresentation();
    }
    else if (!rMenuId.empty())
    {
        sal_Int32 nPageNumber = o3tl::toInt32(rMenuId) - CM_SLIDES;
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
}

Reference< XSlideShow > SlideshowImpl::createSlideShow()
{
    Reference< XSlideShow > xShow;

    try
    {
        Reference< uno::XComponentContext > xContext =
            ::comphelper::getProcessComponentContext();

        xShow.set( presentation::SlideShow::create(xContext), UNO_SET_THROW );
    }
    catch( uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::createSlideShow()" );
    }

    return xShow;
}

void SlideshowImpl::createSlideList( bool bAll, std::u16string_view rPresSlide )
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
    mpSlideController = std::make_shared<AnimationSlideController>( xSlides, eMode );

    if( eMode != AnimationSlideController::CUSTOM )
    {
        sal_Int32 nFirstVisibleSlide = 0;

        // normal presentation
        if( !rPresSlide.empty() )
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
        if( meAnimationMode != ANIMATIONMODE_SHOW && !rPresSlide.empty() )
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

const FncGetChildWindowId aShowChildren[] =
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
            mnChildMask |= ::tools::ULong(1) << i;
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
                if( mnChildMask & ( ::tools::ULong(1) << i ) )
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::resize()" );
    }
}

void SlideshowImpl::setActiveXToolbarsVisible( bool bVisible )
{
    // in case of ActiveX control the toolbars should not be visible if slide show runs in window mode
    // actually it runs always in window mode in case of ActiveX control
    if ( !(!maPresSettings.mbFullScreen && mpDocSh && mpDocSh->GetMedium()) )
        return;

    const SfxBoolItem* pItem = mpDocSh->GetMedium()->GetItemSet().GetItem(SID_VIEWONLY, false);
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

    if( mbActive || !mxShow.is() )
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

        Sequence< beans::PropertyValue > aArgs{ comphelper::makePropertyValue("AutoSaveState", bOn) };

        uno::Reference< frame::XDispatch > xAutoSave = frame::theAutoRecovery::get(xContext);
        xAutoSave->dispatch(aURL, aArgs);
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::setAutoSaveState()");
    }
}

Reference< XDrawPage > SAL_CALL SlideshowImpl::getCurrentSlide()
{
    SolarMutexGuard aSolarGuard;

    Reference< XDrawPage > xSlide;
    if( mxShow.is() && mpSlideController )
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
    return mpSlideController ? mpSlideController->getCurrentSlideIndex() : -1;
}

// css::presentation::XSlideShowController:

::sal_Int32 SAL_CALL SlideshowImpl::getSlideCount()
{
    return mpSlideController ? mpSlideController->getSlideIndexCount() : 0;
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::setUsePen()" );
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

void SAL_CALL SlideshowImpl::setEraseAllInk(sal_Bool bEraseAllInk)
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
        TOOLS_WARN_EXCEPTION( "sd.slideshow", "sd::SlideshowImpl::setEraseAllInk()" );
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

    if( !(mxShow.is() && mpSlideController && mpShowWindow) )
        return;

    if( mbIsPaused && mpShowWindow->GetShowWindowMode() != SHOWWINDOWMODE_END )
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

    if( !(mxShow.is() && mpSlideController && mpShowWindow) )
        return;

    const ShowWindowMode eMode = mpShowWindow->GetShowWindowMode();
    if( (eMode == SHOWWINDOWMODE_PAUSE) || (eMode == SHOWWINDOWMODE_BLANK) || mbIsPaused )
    {
        resume();
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

    if( !(mpShowWindow && mpSlideController) )
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

        if( mpSlideController )
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

    if( !(mxShow.is() && mpSlideController) )
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::gotoPreviousSlide()" );
    }
}

void SAL_CALL SlideshowImpl::gotoLastSlide()
{
    SolarMutexGuard aSolarGuard;

    if( !mpSlideController )
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

    if( !(mpSlideController && xSlide.is()) )
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::SlideshowImpl::stopSound()" );
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

namespace
{
    class AsyncUpdateSlideshow_Impl
    {
    public:
        struct AsyncUpdateSlideshowData
        {
            SlideshowImpl* pSlideshowImpl;
            uno::Reference< css::drawing::XDrawPage > XCurrentSlide;
            SdrHintKind eHintKind;
        };

        static ImplSVEvent* AsyncUpdateSlideshow(
            SlideshowImpl* pSlideshowImpl,
            uno::Reference< css::drawing::XDrawPage >& rXCurrentSlide,
            SdrHintKind eHintKind)
        {
            AsyncUpdateSlideshowData* pNew(new AsyncUpdateSlideshowData);
            pNew->pSlideshowImpl = pSlideshowImpl;
            pNew->XCurrentSlide = rXCurrentSlide;
            pNew->eHintKind = eHintKind;
            return Application::PostUserEvent(LINK(nullptr, AsyncUpdateSlideshow_Impl, Update), pNew);
            // coverity[leaked_storage] - pDisruptor takes care of its own destruction at idle time
        }

        DECL_STATIC_LINK(AsyncUpdateSlideshow_Impl, Update, void*, void);
    };

    IMPL_STATIC_LINK(AsyncUpdateSlideshow_Impl, Update, void*, pData, void)
    {
        AsyncUpdateSlideshowData* pSlideData(static_cast<AsyncUpdateSlideshowData*>(pData));
        pSlideData->pSlideshowImpl->AsyncNotifyEvent(pSlideData->XCurrentSlide, pSlideData->eHintKind);
        delete pSlideData;
    }
}

void SlideshowImpl::AsyncNotifyEvent(
    const uno::Reference< css::drawing::XDrawPage >& rXCurrentSlide,
    const SdrHintKind eHintKind)
{
    switch (eHintKind)
    {
        case SdrHintKind::ObjectInserted:
        {
            mnEventObjectInserted = nullptr;

            // refresh single slide
            gotoSlide(rXCurrentSlide);
            break;
        }
        case SdrHintKind::ObjectRemoved:
        {
            mnEventObjectRemoved = nullptr;

            // refresh single slide
            gotoSlide(rXCurrentSlide);
            break;
        }
        case SdrHintKind::ObjectChange:
        {
            mnEventObjectChange = nullptr;

            // refresh single slide
            gotoSlide(rXCurrentSlide);
            break;
        }
        case SdrHintKind::PageOrderChange:
        {
            mnEventPageOrderChange = nullptr;

            // order of pages (object pages or master pages) changed (Insert/Remove/ChangePos)
            // rXCurrentSlide is the current slide before the change.
            Reference< XDrawPagesSupplier > xDrawPages( mpDoc->getUnoModel(), UNO_QUERY_THROW );
            Reference< XIndexAccess > xSlides( xDrawPages->getDrawPages(), UNO_QUERY_THROW );
            const sal_Int32 nNewSlideCount(xSlides.is() ? xSlides->getCount() : 0);

            if (nNewSlideCount != mpSlideController->getSlideNumberCount())
            {
                // need to reinitialize AnimationSlideController
                OUString aPresSlide( maPresSettings.maPresPage );
                createSlideList( maPresSettings.mbAll, aPresSlide );
            }

            // Check if current slide before change is still valid (maybe removed)
            const sal_Int32 nSlideCount(mpSlideController->getSlideNumberCount());
            bool bSlideStillValid(false);

            for (sal_Int32 nSlide(0); !bSlideStillValid && nSlide < nSlideCount; nSlide++)
            {
                if (rXCurrentSlide == mpSlideController->getSlideByNumber(nSlide))
                {
                    bSlideStillValid = true;
                }
            }

            if(bSlideStillValid)
            {
                // stay on that slide
                gotoSlide(rXCurrentSlide);
            }
            else
            {
                // not possible to stay on that slide, go to 1st slide (kinda restart)
                gotoFirstSlide();
            }
            break;
        }
        default:
            break;
    }
}

bool SlideshowImpl::isCurrentSlideInvolved(const SdrHint& rHint)
{
    // get current slide
    uno::Reference< css::drawing::XDrawPage > XCurrentSlide(getCurrentSlide());
    if (!XCurrentSlide.is())
        return false;

    SdrPage* pCurrentSlide(GetSdrPageFromXDrawPage(XCurrentSlide));
    if (nullptr == pCurrentSlide)
        return false;

    const SdrPage* pHintPage(rHint.GetPage());
    if (nullptr == pHintPage)
        return false;

    if (pHintPage->IsMasterPage())
    {
        if (pCurrentSlide->TRG_HasMasterPage())
        {
            // current slide uses MasterPage on which the change happened
            return pHintPage == &pCurrentSlide->TRG_GetMasterPage();
        }
    }

    // object on current slide was changed
    return pHintPage == pCurrentSlide;
}

void SlideshowImpl::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (SfxHintId::ThisIsAnSdrHint != rHint.GetId())
        // nothing to do for non-SdrHints
        return;

    if (nullptr == mpDoc)
        // better do nothing when no DrawModel (should not happen)
        return;

    // tdf#158664 I am surprised, but the 'this' instance keeps incarnated
    // when the slideshow was running once, so need to check for
    // SlideShow instance/running to be safe.
    // NOTE: isRunning() checks mxShow.is(), that is what we want
    if (!isRunning())
        // no SlideShow instance or not running, nothing to do
        return;

    const SdrHint& rSdrHint(static_cast<const SdrHint&>(rHint));
    const SdrHintKind eHintKind(rSdrHint.GetKind());

    switch (eHintKind)
    {
        case SdrHintKind::ObjectInserted:
        {
            if (nullptr != mnEventObjectInserted)
                // avoid multiple events
                return;

            if (!isCurrentSlideInvolved(rSdrHint))
                // nothing to do when current slide is not involved
                return;

            // Refresh current slide
            uno::Reference< css::drawing::XDrawPage > XCurrentSlide(getCurrentSlide());
            mnEventObjectInserted = AsyncUpdateSlideshow_Impl::AsyncUpdateSlideshow(this, XCurrentSlide, eHintKind);
            break;
        }
        case SdrHintKind::ObjectRemoved:
        {
            if (nullptr != mnEventObjectRemoved)
                // avoid multiple events
                return;

            if (!isCurrentSlideInvolved(rSdrHint))
                // nothing to do when current slide is not involved
                return;

            // Refresh current slide
            uno::Reference< css::drawing::XDrawPage > XCurrentSlide(getCurrentSlide());
            mnEventObjectRemoved = AsyncUpdateSlideshow_Impl::AsyncUpdateSlideshow(this, XCurrentSlide, eHintKind);
            break;
        }
        case SdrHintKind::ObjectChange:
        {
            if (nullptr != mnEventObjectChange)
                // avoid multiple events
                return;

            if (!isCurrentSlideInvolved(rSdrHint))
                // nothing to do when current slide is not involved
                return;

            // Refresh current slide. Need to do that asynchronous, else e.g.
            // text edit changes EditEngine/Outliner are not progressed far
            // enough (ObjectChanged broadcast which we are in here seems
            // too early for some cases)
            uno::Reference< css::drawing::XDrawPage > XCurrentSlide(getCurrentSlide());
            mnEventObjectChange = AsyncUpdateSlideshow_Impl::AsyncUpdateSlideshow(this, XCurrentSlide, eHintKind);
            break;
        }
        case SdrHintKind::PageOrderChange:
        {
            // Unfortunately we get multiple events, e.g. when drag/drop position change in
            // slide sorter on left side of EditView. This includes some with page number +1,
            // then again -1 (it's a position change). Problem is that in-between already
            // a re-schedule seems to happen, so indeed AsyncNotifyEvent will change to +1/-1
            // already. Since we get even more, at least try to take the last one. I found no
            // good solution yet for this.
            if (nullptr != mnEventPageOrderChange)
                Application::RemoveUserEvent( mnEventPageOrderChange );

            // order of pages (object pages or master pages) changed (Insert/Remove/ChangePos)
            uno::Reference< css::drawing::XDrawPage > XCurrentSlide(getCurrentSlide());
            mnEventPageOrderChange = AsyncUpdateSlideshow_Impl::AsyncUpdateSlideshow(this, XCurrentSlide, eHintKind);
            break;
        }
        case SdrHintKind::ModelCleared:
        {
            // immediately end presentation
            endPresentation();
            break;
        }
        default:
            break;
    }
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
    for( const PropertyValue& rValue : rArguments )
    {
        SetPropertyValue( rValue.Name, rValue.Value );
    }
}

void PresentationSettingsEx::SetPropertyValue( std::u16string_view rProperty, const Any& rValue )
{
    if ( rProperty == u"RehearseTimings" )
    {
        if( rValue >>= mbRehearseTimings )
            return;
    }
    else if ( rProperty == u"Preview" )
    {
        if( rValue >>= mbPreview )
            return;
    }
    else if ( rProperty == u"AnimationNode" )
    {
        if( rValue >>= mxAnimationNode )
            return;
    }
    else if ( rProperty == u"ParentWindow" )
    {
        Reference< XWindow > xWindow;
        if( rValue >>= xWindow )
        {
            mpParentWindow = xWindow.is() ? VCLUnoHelper::GetWindow( xWindow )
                                          : nullptr;
            return;
        }
    }
    else if ( rProperty == u"AllowAnimations" )
    {
        if( rValue >>= mbAnimationAllowed )
            return;
    }
    else if ( rProperty == u"FirstPage" )
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
    else if ( rProperty == u"IsAlwaysOnTop" )
    {
        if( rValue >>= mbAlwaysOnTop )
            return;
    }
    else if ( rProperty == u"IsAutomatic" )
    {
        if( rValue >>= mbManual )
            return;
    }
    else if ( rProperty == u"IsEndless" )
    {
        if( rValue >>= mbEndless )
            return;
    }
    else if ( rProperty == u"IsFullScreen" )
    {
        if( rValue >>= mbFullScreen )
            return;
    }
    else if ( rProperty == u"IsMouseVisible" )
    {
        if( rValue >>= mbMouseVisible )
            return;
    }
    else if ( rProperty == u"Pause" )
    {
        sal_Int32 nPause = -1;
        if( (rValue >>= nPause) && (nPause >= 0) )
        {
            mnPauseTimeout = nPause;
            return;
        }
    }
    else if ( rProperty == u"UsePen" )
    {
        if( rValue >>= mbMouseAsPen )
            return;
    }
    throw IllegalArgumentException();
}

// XAnimationListener

SlideShowListenerProxy::SlideShowListenerProxy( rtl::Reference< SlideshowImpl > xController, css::uno::Reference< css::presentation::XSlideShow > xSlideShow )
: mxController(std::move( xController ))
, mxSlideShow(std::move( xSlideShow ))
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
    std::unique_lock g(m_aMutex);
    maListeners.addInterface(g, xListener);
}

void SlideShowListenerProxy::removeSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    maListeners.removeInterface(g, xListener);
}

void SAL_CALL SlideShowListenerProxy::beginEvent( const Reference< XAnimationNode >& xNode )
{
    std::unique_lock aGuard( m_aMutex );

    if( maListeners.getLength(aGuard) >= 0 )
    {
        maListeners.forEach(aGuard,
            [&] (Reference<XAnimationListener> const& xListener) {
                return xListener->beginEvent(xNode);
            } );
    }
}

void SAL_CALL SlideShowListenerProxy::endEvent( const Reference< XAnimationNode >& xNode )
{
    std::unique_lock aGuard( m_aMutex );

    if( maListeners.getLength(aGuard) >= 0 )
    {
        maListeners.forEach(aGuard,
            [&] (Reference<XAnimationListener> const& xListener) {
                return xListener->endEvent(xNode);
            } );
    }
}

void SAL_CALL SlideShowListenerProxy::repeat( const Reference< XAnimationNode >& xNode, ::sal_Int32 nRepeat )
{
    std::unique_lock aGuard( m_aMutex );

    if( maListeners.getLength(aGuard) >= 0 )
    {
        maListeners.forEach(aGuard,
            [&] (Reference<XAnimationListener> const& xListener) {
                return xListener->repeat(xNode, nRepeat);
            } );
    }
}

// css::presentation::XSlideShowListener:

void SAL_CALL SlideShowListenerProxy::paused(  )
{
    std::unique_lock aGuard( m_aMutex );

    maListeners.forEach(aGuard,
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->paused();
        });
}

void SAL_CALL SlideShowListenerProxy::resumed(  )
{
    std::unique_lock aGuard( m_aMutex );

    maListeners.forEach(aGuard,
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->resumed();
        });
}

void SAL_CALL SlideShowListenerProxy::slideTransitionStarted( )
{
    std::unique_lock aGuard( m_aMutex );

    maListeners.forEach(aGuard,
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideTransitionStarted();
        });
}

void SAL_CALL SlideShowListenerProxy::slideTransitionEnded( )
{
    std::unique_lock aGuard( m_aMutex );

    maListeners.forEach(aGuard,
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideTransitionEnded ();
        });
}

void SAL_CALL SlideShowListenerProxy::slideAnimationsEnded(  )
{
    std::unique_lock aGuard( m_aMutex );

    maListeners.forEach(aGuard,
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideAnimationsEnded ();
        });
}

void SlideShowListenerProxy::slideEnded(sal_Bool bReverse)
{
    {
        std::unique_lock aGuard( m_aMutex );

        if( maListeners.getLength(aGuard) >= 0 )
        {
            maListeners.forEach(aGuard,
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
        std::unique_lock aGuard( m_aMutex );

        if( maListeners.getLength(aGuard) >= 0 )
        {
            maListeners.forEach(aGuard,
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
    std::unique_lock g(m_aMutex);
    maListeners.disposeAndClear( g, aDisposeEvent );
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

void SAL_CALL SlideShowListenerProxy::contextMenuShow(const css::awt::Point& point)
{
    SolarMutexGuard aSolarGuard;
    if (mxController.is())
        mxController->contextMenuShow(point);
}

} // namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
