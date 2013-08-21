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

#include <com/sun/star/animations/XAnimationNode.hpp>

#include "SlideTransitionPane.hxx"
#include "SlideTransitionPane.hrc"
#include "CustomAnimation.hrc"

#include "TransitionPreset.hxx"
#include "sdresid.hxx"
#include "ViewShellBase.hxx"
#include "DrawDocShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "drawdoc.hxx"
#include "filedlg.hxx"
#include "strings.hrc"
#include "DrawController.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>

#include <svtools/controldims.hrc>
#include <svx/gallery.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include "DrawViewShell.hxx"
#include "slideshow.hxx"
#include "drawview.hxx"
#include "sdundogr.hxx"
#include "undoanim.hxx"
#include "optsitem.hxx"
#include "sddll.hxx"
#include "framework/FrameworkHelper.hxx"

#include "DialogListBox.hxx"
#include <sfx2/sidebar/Theme.hxx>

#include <algorithm>
#include <memory>


using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;

using ::sd::framework::FrameworkHelper;

// ____________________________
//
// ::sd::impl::TransitionEffect
// ____________________________

namespace sd
{
namespace impl
{
struct TransitionEffect
{
    TransitionEffect() :
            mnType( 0 ),
            mnSubType( 0 ),
            mbDirection( sal_True ),
            mnFadeColor( 0 )
    {
        init();
    }
    explicit TransitionEffect( const ::sd::TransitionPreset & rPreset ) :
            mnType( rPreset.getTransition()),
            mnSubType( rPreset.getSubtype()),
            mbDirection( rPreset.getDirection()),
            mnFadeColor( rPreset.getFadeColor())
    {
        init();
    }
    explicit TransitionEffect( sal_Int16 nType, sal_Int16 nSubType,
                               sal_Bool  bDirection, sal_Int32 nFadeColor ) :
            mnType( nType),
            mnSubType( nSubType ),
            mbDirection( bDirection ),
            mnFadeColor( nFadeColor )
    {
        init();
    }
    explicit TransitionEffect( const SdPage & rPage ) :
            mnType( rPage.getTransitionType() ),
            mnSubType( rPage.getTransitionSubtype() ),
            mbDirection( rPage.getTransitionDirection() ),
            mnFadeColor( rPage.getTransitionFadeColor() )
    {
        init();

        mfDuration = rPage.getTransitionDuration();
        mfTime = rPage.GetTime();
        mePresChange = rPage.GetPresChange();
        mbSoundOn = rPage.IsSoundOn();
        maSound = rPage.GetSoundFile();
        mbLoopSound = rPage.IsLoopSound();
        mbStopSound = rPage.IsStopSound();
    }

    void init()
    {
        mfDuration = 2.0;
        mfTime = 0.0;
        mePresChange = PRESCHANGE_MANUAL;
        mbSoundOn = sal_False;
        mbLoopSound = sal_False;
        mbStopSound = sal_False;

        mbEffectAmbiguous = false;
        mbDurationAmbiguous = false;
        mbTimeAmbiguous = false;
        mbPresChangeAmbiguous = false;
        mbSoundAmbiguous = false;
        mbLoopSoundAmbiguous = false;
    }

    void setAllAmbiguous()
    {
        mbEffectAmbiguous = true;
        mbDurationAmbiguous = true;
        mbTimeAmbiguous = true;
        mbPresChangeAmbiguous = true;
        mbSoundAmbiguous = true;
        mbLoopSoundAmbiguous = true;
    }

    bool operator == ( const ::sd::TransitionPreset & rPreset ) const
    {
        return
            (mnType == rPreset.getTransition()) &&
            (mnSubType == rPreset.getSubtype()) &&
            (mbDirection == rPreset.getDirection()) &&
            (mnFadeColor ==  rPreset.getFadeColor());
    }

    void applyTo( SdPage & rOutPage ) const
    {
        if( ! mbEffectAmbiguous )
        {
            rOutPage.setTransitionType( mnType );
            rOutPage.setTransitionSubtype( mnSubType );
            rOutPage.setTransitionDirection( mbDirection );
            rOutPage.setTransitionFadeColor( mnFadeColor );
        }

        if( ! mbDurationAmbiguous )
            rOutPage.setTransitionDuration( mfDuration );
        if( ! mbTimeAmbiguous )
            rOutPage.SetTime( mfTime );
        if( ! mbPresChangeAmbiguous )
            rOutPage.SetPresChange( mePresChange );
        if( ! mbSoundAmbiguous )
        {
            if( mbStopSound )
            {
                rOutPage.SetStopSound( sal_True );
                rOutPage.SetSound( sal_False );
            }
            else
            {
                rOutPage.SetStopSound( sal_False );
                rOutPage.SetSound( mbSoundOn );
                rOutPage.SetSoundFile( maSound );
            }
        }
        if( ! mbLoopSoundAmbiguous )
            rOutPage.SetLoopSound( mbLoopSound );
    }

    void compareWith( const SdPage & rPage )
    {
        TransitionEffect aOtherEffect( rPage );
        mbEffectAmbiguous = mbEffectAmbiguous || aOtherEffect.mbEffectAmbiguous
                                              || (mnType != aOtherEffect.mnType)
                                              || (mnSubType != aOtherEffect.mnSubType)
                                              || (mbDirection != aOtherEffect.mbDirection)
                                              || (mnFadeColor != aOtherEffect.mnFadeColor);

        mbDurationAmbiguous = mbDurationAmbiguous || aOtherEffect.mbDurationAmbiguous || mfDuration != aOtherEffect.mfDuration;
        mbTimeAmbiguous = mbTimeAmbiguous || aOtherEffect.mbTimeAmbiguous || mfTime != aOtherEffect.mfTime;
        mbPresChangeAmbiguous = mbPresChangeAmbiguous || aOtherEffect.mbPresChangeAmbiguous || mePresChange != aOtherEffect.mePresChange;
        mbSoundAmbiguous = mbSoundAmbiguous || aOtherEffect.mbSoundAmbiguous || mbSoundOn != aOtherEffect.mbSoundOn;
#if 0
                        // Weird leftover isolated expression with no effect, introduced in 2007 in
                        // CWS impress122. Ifdeffed out to avoid compiler warning, kept here in case
                        // somebody who understands this code notices and understands what the
                        // "right" thing to do might be.
                        (!mbStopSound && !aOtherEffect.mbStopSound && maSound != aOtherEffect.maSound) || (mbStopSound != aOtherEffect.mbStopSound);
#endif
        mbLoopSoundAmbiguous = mbLoopSoundAmbiguous || aOtherEffect.mbLoopSoundAmbiguous || mbLoopSound != aOtherEffect.mbLoopSound;
    }

    // effect
    sal_Int16 mnType;
    sal_Int16 mnSubType;
    sal_Bool  mbDirection;
    sal_Int32 mnFadeColor;

    // other settings
    double      mfDuration;
    double       mfTime;
    PresChange  mePresChange;
    sal_Bool        mbSoundOn;
    OUString    maSound;
    bool        mbLoopSound;
    bool        mbStopSound;

    bool mbEffectAmbiguous;
    bool mbDurationAmbiguous;
    bool mbTimeAmbiguous;
    bool mbPresChangeAmbiguous;
    bool mbSoundAmbiguous;
    bool mbLoopSoundAmbiguous;
};

} // namespace impl
} // namespace sd

// ______________________
//
// Local Helper Functions
// ______________________

namespace
{

void lcl_ApplyToPages(
    const ::sd::slidesorter::SharedPageSelection& rpPages,
    const ::sd::impl::TransitionEffect & rEffect )
{
    ::std::vector< SdPage * >::const_iterator aIt( rpPages->begin());
    const ::std::vector< SdPage * >::const_iterator aEndIt( rpPages->end());
    for( ; aIt != aEndIt; ++aIt )
    {
        rEffect.applyTo( *(*aIt) );
    }
}

void lcl_CreateUndoForPages(
    const ::sd::slidesorter::SharedPageSelection& rpPages,
    ::sd::ViewShellBase& rBase )
{
    ::sd::DrawDocShell* pDocSh      = rBase.GetDocShell();
    ::svl::IUndoManager* pManager   = pDocSh->GetUndoManager();
    SdDrawDocument* pDoc            = pDocSh->GetDoc();
    if( pManager && pDocSh && pDoc )
    {
        OUString aComment( SdResId(STR_UNDO_SLIDE_PARAMS) );
        pManager->EnterListAction(aComment, aComment);
        SdUndoGroup* pUndoGroup = new SdUndoGroup( pDoc );
        pUndoGroup->SetComment( aComment );

        ::std::vector< SdPage * >::const_iterator aIt( rpPages->begin());
        const ::std::vector< SdPage * >::const_iterator aEndIt( rpPages->end());
        for( ; aIt != aEndIt; ++aIt )
        {
            pUndoGroup->AddAction( new sd::UndoTransition( pDoc, (*aIt) ) );
        }

        pManager->AddUndoAction( pUndoGroup );
        pManager->LeaveListAction();
    }
}

sal_uInt16 lcl_getTransitionEffectIndex(
    SdDrawDocument * pDoc,
    const ::sd::impl::TransitionEffect & rTransition )
{
    // first entry: "<none>"
    sal_uInt16 nResultIndex = LISTBOX_ENTRY_NOTFOUND;

    if( pDoc )
    {
        sal_uInt16 nCurrentIndex = 0;
        const ::sd::TransitionPresetList & rPresetList = ::sd::TransitionPreset::getTransitionPresetList();
        ::sd::TransitionPresetList::const_iterator aIt( rPresetList.begin());
        const ::sd::TransitionPresetList::const_iterator aEndIt( rPresetList.end());
        for( ; aIt != aEndIt; ++aIt, ++nCurrentIndex )
        {
            if( rTransition.operator==( *(*aIt) ))
            {
                nResultIndex = nCurrentIndex;
                break;
            }
        }
    }

    return nResultIndex;
}

::sd::TransitionPresetPtr lcl_getTransitionPresetByUIName(
    SdDrawDocument * pDoc,
    const OUString & rUIName )
{
    ::sd::TransitionPresetPtr pResult;
    if( pDoc )
    {
        const ::sd::TransitionPresetList& rPresetList = ::sd::TransitionPreset::getTransitionPresetList();
        ::sd::TransitionPresetList::const_iterator aIter( rPresetList.begin() );
        const ::sd::TransitionPresetList::const_iterator aEnd( rPresetList.end() );
        for( ; aIter != aEnd; ++aIter )
        {
            if( (*aIter)->getUIName().equals( rUIName ))
            {
                pResult = *aIter;
                break;
            }
        }
    }

    return pResult;
}

struct lcl_EqualsSoundFileName : public ::std::unary_function< OUString, bool >
{
    explicit lcl_EqualsSoundFileName( const OUString & rStr ) :
            maStr( rStr )
    {}

    bool operator() ( const OUString & rStr ) const
    {
        // note: formerly this was a case insensitive search for all
        // platforms. It seems more sensible to do this platform-dependent
#if defined( WNT )
        return maStr.equalsIgnoreAsciiCase( rStr );
#else
        return maStr == rStr;
#endif
    }

private:
    OUString maStr;
};

// returns -1 if no object was found
bool lcl_findSoundInList( const ::std::vector< OUString > & rSoundList,
                          const OUString & rFileName,
                          ::std::vector< OUString >::size_type & rOutPosition )
{
    ::std::vector< OUString >::const_iterator aIt =
          ::std::find_if( rSoundList.begin(), rSoundList.end(),
                          lcl_EqualsSoundFileName( rFileName ));
    if( aIt != rSoundList.end())
    {
        rOutPosition = ::std::distance( rSoundList.begin(), aIt );
        return true;
    }

    return false;
}

OUString lcl_getSoundFileURL(
    const ::std::vector< OUString > & rSoundList,
    const ListBox* rListBox )
{
    if( rListBox->GetSelectEntryCount() > 0 )
    {
        sal_uInt16 nPos = rListBox->GetSelectEntryPos();
        // the first three entries are no actual sounds
        if( nPos >= 3 )
        {
            DBG_ASSERT( (sal_uInt32)(rListBox->GetEntryCount() - 3) == rSoundList.size(),
                        "Sound list-box is not synchronized to sound list" );
            nPos -= 3;
            if( rSoundList.size() > nPos )
                return rSoundList[ nPos ];
        }
    }

    return OUString();
}

struct lcl_AppendSoundToListBox : public ::std::unary_function< OUString, void >
{
    lcl_AppendSoundToListBox( ListBox*  rListBox ) :
            mrListBox( rListBox )
    {}

    void operator() ( const OUString & rString ) const
    {
        INetURLObject aURL( rString );
        mrListBox->InsertEntry( aURL.GetBase(), LISTBOX_APPEND );
    }

private:
    ListBox*  mrListBox;
};

void lcl_FillSoundListBox(
    const ::std::vector< OUString > & rSoundList,
    ListBox*  rOutListBox )
{
    sal_uInt16 nCount = rOutListBox->GetEntryCount();

    // keep first three entries
    for( sal_uInt16 i=nCount - 1; i>=3; --i )
        rOutListBox->RemoveEntry( i );

    ::std::for_each( rSoundList.begin(), rSoundList.end(),
                     lcl_AppendSoundToListBox( rOutListBox ));
}

} // anonymous namespace

namespace sd
{

// ___________________
//
// SlideTransitionPane
// ___________________

SlideTransitionPane::SlideTransitionPane(
    ::Window * pParent,
    ViewShellBase & rBase,
    const Size& rMinSize,
    SdDrawDocument* pDoc,
    const cssu::Reference<css::frame::XFrame>& rxFrame ) :
        PanelLayout( pParent, "SlideTransitionsPanel", "modules/simpress/ui/slidetransitionspanel.ui", rxFrame ),

        mrBase( rBase ),
        mpDrawDoc( pDoc ),
        maMinSize( rMinSize ),
        mbHasSelection( false ),
        mbUpdatingControls( false ),
        mbIsMainViewChangePending( false ),
        maLateInitTimer()
{
    get(mpLB_SLIDE_TRANSITIONS, "transitions_list");
    get(mpFT_SPEED, "speed_label");
    get(mpLB_SPEED, "speed_list");
    get(mpFT_SOUND, "sound_label");
    get(mpLB_SOUND, "sound_list");
    get(mpCB_LOOP_SOUND, "loop_sound" );
    get(mpRB_ADVANCE_ON_MOUSE, "rb_mouse_click");
    get(mpRB_ADVANCE_AUTO, "rb_auto_after");
    get(mpMF_ADVANCE_AUTO_AFTER, "auto_after_value");
    get(mpPB_APPLY_TO_ALL, "apply_to_all");
    get(mpPB_PLAY, "play");
    get(mpPB_SLIDE_SHOW, "slide_show");
    get(mpCB_AUTO_PREVIEW, "auto_preview");

    String maSTR_NO_TRANSITION( SdResId(STR_SLIDETRANSITION_NONE) );
    mpLB_SLIDE_TRANSITIONS->set_width_request(mpLB_SLIDE_TRANSITIONS->approximate_char_width() * 16);
    mpLB_SLIDE_TRANSITIONS->set_height_request(mpLB_SLIDE_TRANSITIONS->GetTextHeight() * 16);

    if( pDoc )
        mxModel.set( pDoc->getUnoModel(), uno::UNO_QUERY );
    // TODO: get correct view
    if( mxModel.is())
        mxView.set( mxModel->getCurrentController(), uno::UNO_QUERY );

    // fill list box of slide transitions
    mpLB_SLIDE_TRANSITIONS->InsertEntry( maSTR_NO_TRANSITION );

    // set defaults
    mpCB_AUTO_PREVIEW->Check();      // automatic preview on

    // update control states before adding handlers
    //updateLayout();
    updateControls();

    // set handlers
    mpPB_APPLY_TO_ALL->SetClickHdl( LINK( this, SlideTransitionPane, ApplyToAllButtonClicked ));
    mpPB_PLAY->SetClickHdl( LINK( this, SlideTransitionPane, PlayButtonClicked ));
    mpPB_SLIDE_SHOW->SetClickHdl( LINK( this, SlideTransitionPane, SlideShowButtonClicked ));

    mpLB_SLIDE_TRANSITIONS->SetSelectHdl( LINK( this, SlideTransitionPane, TransitionSelected ));

    mpLB_SPEED->SetSelectHdl( LINK( this, SlideTransitionPane, SpeedListBoxSelected ));
    mpLB_SOUND->SetSelectHdl( LINK( this, SlideTransitionPane, SoundListBoxSelected ));
    mpCB_LOOP_SOUND->SetClickHdl( LINK( this, SlideTransitionPane, LoopSoundBoxChecked ));

    mpRB_ADVANCE_ON_MOUSE->SetToggleHdl( LINK( this, SlideTransitionPane, AdvanceSlideRadioButtonToggled ));
    mpRB_ADVANCE_AUTO->SetToggleHdl( LINK( this, SlideTransitionPane, AdvanceSlideRadioButtonToggled ));
    mpMF_ADVANCE_AUTO_AFTER->SetModifyHdl( LINK( this, SlideTransitionPane, AdvanceTimeModified ));
    mpCB_AUTO_PREVIEW->SetClickHdl( LINK( this, SlideTransitionPane, AutoPreviewClicked ));
    addListener();

    maLateInitTimer.SetTimeout(200);
    maLateInitTimer.SetTimeoutHdl(LINK(this, SlideTransitionPane, LateInitCallback));
    maLateInitTimer.Start();

    UpdateLook();
}

SlideTransitionPane::~SlideTransitionPane()
{
    maLateInitTimer.Stop();
    removeListener();
}

void SlideTransitionPane::Resize()
{
    //updateLayout();
}




void SlideTransitionPane::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;
    UpdateLook();
}




void SlideTransitionPane::UpdateLook (void)
{
    SetBackground(::sfx2::sidebar::Theme::GetWallpaper(::sfx2::sidebar::Theme::Paint_PanelBackground));
    mpFT_SPEED->SetBackground(Wallpaper());
    mpFT_SOUND->SetBackground(Wallpaper());
}




void SlideTransitionPane::onSelectionChanged()
{
    updateControls();
}

void SlideTransitionPane::onChangeCurrentPage()
{
    updateControls();
}

::sd::slidesorter::SharedPageSelection SlideTransitionPane::getSelectedPages (void) const
{
    ::sd::slidesorter::SlideSorterViewShell * pSlideSorterViewShell
        = ::sd::slidesorter::SlideSorterViewShell::GetSlideSorter(mrBase);
    ::boost::shared_ptr<sd::slidesorter::SlideSorterViewShell::PageSelection> pSelection;

    if( pSlideSorterViewShell )
    {
        pSelection = pSlideSorterViewShell->GetPageSelection();
    }
    else
    {
        pSelection.reset(new sd::slidesorter::SlideSorterViewShell::PageSelection());
        if( mxView.is() )
        {
            SdPage* pPage = SdPage::getImplementation( mxView->getCurrentPage() );
            if( pPage )
                pSelection->push_back(pPage);
        }
    }

    return pSelection;
}

/*void SlideTransitionPane::updateLayout()
{
    ::Size aPaneSize( GetSizePixel() );
    if( aPaneSize.Width() < maMinSize.Width() )
        aPaneSize.Width() = maMinSize.Width();

    if( aPaneSize.Height() < maMinSize.Height() )
        aPaneSize.Height() = maMinSize.Height();

    // start layouting elements from bottom to top.  The remaining space is used
    // for the topmost list box
    ::Point aOffset( LogicToPixel( Point( 3, 3 ), MAP_APPFONT ) );
    long nOffsetX = aOffset.getX();
    long nOffsetY = aOffset.getY();
    long nOffsetBtnX = LogicToPixel( Point( 6, 1 ), MAP_APPFONT ).getX();

    const long nMinCtrlWidth = LogicToPixel( ::Point( 32, 1 ), MAP_APPFONT ).getX();
    const long nTextIndent = LogicToPixel( ::Point( RSC_SP_CHK_TEXTINDENT, 1 ), MAP_APPFONT ).getX();

    ::Point aUpperLeft( nOffsetX, aPaneSize.getHeight() - nOffsetY );
    long nMaxWidth = aPaneSize.getWidth() - 2 * nOffsetX;

    // auto preview check-box
    ::Size aCtrlSize = mpCB_AUTO_PREVIEW.GetSizePixel();
    aCtrlSize.setWidth( mpCB_AUTO_PREVIEW.CalcMinimumSize( nMaxWidth ).getWidth());
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    mpCB_AUTO_PREVIEW.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line above check-box
    aCtrlSize = maFL_EMPTY2.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    maFL_EMPTY2.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // buttons "Play" and "Slide Show"
    long nPlayButtonWidth = mpPB_PLAY.CalcMinimumSize().getWidth() + 2 * nOffsetBtnX;
    long nSlideShowButtonWidth = mpPB_SLIDE_SHOW.CalcMinimumSize().getWidth() + 2 * nOffsetBtnX;

    if( (nPlayButtonWidth + nSlideShowButtonWidth + nOffsetX) <= nMaxWidth )
    {
        // place buttons side by side
        aCtrlSize = mpPB_PLAY.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nPlayButtonWidth );
        mpPB_PLAY.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aUpperLeft.setX( aUpperLeft.getX() + nPlayButtonWidth + nOffsetX );
        aCtrlSize.setWidth( nSlideShowButtonWidth );
        mpPB_SLIDE_SHOW.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }
    else
    {
        // place buttons on top of each other
        aCtrlSize = mpPB_SLIDE_SHOW.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nSlideShowButtonWidth );
        mpPB_SLIDE_SHOW.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = mpPB_PLAY.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
        aCtrlSize.setWidth( nPlayButtonWidth );
        mpPB_PLAY.SetPosSizePixel( aUpperLeft, aCtrlSize );
    }

    // "Apply to All Slides" button
    aCtrlSize = mpPB_APPLY_TO_ALL.GetSizePixel();
    aCtrlSize.setWidth( mpPB_APPLY_TO_ALL.CalcMinimumSize( nMaxWidth ).getWidth() + 2 * nOffsetBtnX );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    mpPB_APPLY_TO_ALL.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line above "Apply to All Slides" button
    aCtrlSize = maFL_EMPTY1.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    maFL_EMPTY1.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // advance automatically after ... seconds
    long nItemWidth = mpRB_ADVANCE_AUTO.CalcMinimumSize().getWidth();

    if( (nItemWidth + nMinCtrlWidth + nOffsetX) <= nMaxWidth )
    {
        long nBase = aUpperLeft.getY();

        // place controls side by side
        aCtrlSize = mpRB_ADVANCE_AUTO.GetSizePixel();
        aUpperLeft.setY( nBase - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nItemWidth );
        mpRB_ADVANCE_AUTO.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = mpMF_ADVANCE_AUTO_AFTER.GetSizePixel();
        aUpperLeft.setY( nBase - aCtrlSize.getHeight() );
        aUpperLeft.setX( aUpperLeft.getX() + nItemWidth + nOffsetX );
        aCtrlSize.setWidth( nMinCtrlWidth );
        mpMF_ADVANCE_AUTO_AFTER.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }
    else
    {
        // place controls on top of each other
        aCtrlSize = mpMF_ADVANCE_AUTO_AFTER.GetSizePixel();
        aUpperLeft.setX( nOffsetX + nTextIndent );
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nMinCtrlWidth );
        mpMF_ADVANCE_AUTO_AFTER.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = mpRB_ADVANCE_AUTO.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
        aUpperLeft.setX( nOffsetX );
        aCtrlSize.setWidth( nItemWidth );
        mpRB_ADVANCE_AUTO.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }

    // check box "On mouse click"
    aCtrlSize = mpRB_ADVANCE_ON_MOUSE.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    mpRB_ADVANCE_ON_MOUSE.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line "Advance slide"
    aCtrlSize = maFL_ADVANCE_SLIDE.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maFL_ADVANCE_SLIDE.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // check box "Loop until next sound"
    long nFTSpeedWidth = mpFT_SPEED->CalcMinimumSize().getWidth() + 2 * nOffsetX;
    long nFTSoundWidth = mpFT_SOUND.CalcMinimumSize().getWidth() + 2 * nOffsetX;
    long nIndent = ::std::max( nFTSoundWidth, nFTSpeedWidth );

    bool bStack = ( (nIndent + nMinCtrlWidth + nOffsetX) > nMaxWidth );

    if( bStack )
        nIndent = nTextIndent;

    aCtrlSize = mpCB_LOOP_SOUND.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth - nIndent );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    aUpperLeft.setX( nIndent );
    mpCB_LOOP_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );

    aCtrlSize = mpLB_SOUND.GetSizePixel();
    aCtrlSize.setWidth( ::std::max( nMaxWidth - nIndent, nMinCtrlWidth ) );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    mpLB_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );
    mpLB_SOUND.SetDropDownLineCount( 8 );
    aUpperLeft.setX( nOffsetX );

    aCtrlSize = mpFT_SOUND.GetSizePixel();
    if( bStack )
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    aCtrlSize.setWidth( nFTSoundWidth - 2 * nOffsetX );
    mpFT_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );

    aUpperLeft.setX( nIndent );
    aCtrlSize = mpLB_SPEED.GetSizePixel();
    aCtrlSize.setWidth( ::std::max( nMaxWidth - nIndent, nMinCtrlWidth ) );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    mpLB_SPEED.SetPosSizePixel( aUpperLeft, aCtrlSize );
    mpLB_SPEED.SetDropDownLineCount( 3 );
    aUpperLeft.setX( nOffsetX );

    aCtrlSize = mpFT_SPEED.GetSizePixel();
    if( bStack )
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    aCtrlSize.setWidth( nFTSpeedWidth - 2 * nOffsetX );
    mpFT_SPEED.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line "Modify Transition"
    aCtrlSize = maFL_MODIFY_TRANSITION.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maFL_MODIFY_TRANSITION.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line "Apply to selected slides"
    aCtrlSize = maFL_APPLY_TRANSITION.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    ::Point aUpperLeftCorner( nOffsetX, nOffsetY );
    maFL_APPLY_TRANSITION.SetPosSizePixel( aUpperLeftCorner, aCtrlSize );
    aUpperLeftCorner.setY( aUpperLeftCorner.getY() + aCtrlSize.getHeight() + nOffsetY );
    aUpperLeft.setY( aUpperLeft.getY() - nOffsetY );

    // list box slide transitions
    aCtrlSize.setWidth( nMaxWidth );
    aCtrlSize.setHeight( aUpperLeft.getY() - aUpperLeftCorner.getY() );
    mpLB_SLIDE_TRANSITIONS->SetPosSizePixel( aUpperLeftCorner, aCtrlSize );
}*/

void SlideTransitionPane::updateControls()
{
    ::sd::slidesorter::SharedPageSelection pSelectedPages(getSelectedPages());
    if( pSelectedPages->empty())
    {
        mbHasSelection = false;
        return;
    }
    mbHasSelection = true;

    DBG_ASSERT( ! mbUpdatingControls, "Multiple Control Updates" );
    mbUpdatingControls = true;

    // get model data for first page
    SdPage * pFirstPage = pSelectedPages->front();
    DBG_ASSERT( pFirstPage, "Invalid Page" );

    impl::TransitionEffect aEffect( *pFirstPage );

    // merge with other pages
    ::sd::slidesorter::SlideSorterViewShell::PageSelection::const_iterator aIt(
        pSelectedPages->begin());
    ::sd::slidesorter::SlideSorterViewShell::PageSelection::const_iterator aEndIt(
        pSelectedPages->end());

    // start with second page (note aIt != aEndIt, because ! aSelectedPages.empty())
    for( ++aIt ;aIt != aEndIt; ++aIt )
    {
        if( *aIt )
            aEffect.compareWith( *(*aIt) );
    }

    // detect current slide effect
    if( aEffect.mbEffectAmbiguous )
        mpLB_SLIDE_TRANSITIONS->SetNoSelection();
    else
    {
        // ToDo: That 0 is "no transition" is documented nowhere except in the
        // CTOR of sdpage
        if( aEffect.mnType == 0 )
            mpLB_SLIDE_TRANSITIONS->SelectEntryPos( 0 );
        else
        {
            sal_uInt16 nEntry = lcl_getTransitionEffectIndex( mpDrawDoc, aEffect );
            if( nEntry == LISTBOX_ENTRY_NOTFOUND )
                mpLB_SLIDE_TRANSITIONS->SetNoSelection();
            else
            {
                // first entry in list is "none", so add 1 after translation
                if( m_aPresetIndexes.find( nEntry ) != m_aPresetIndexes.end())
                    mpLB_SLIDE_TRANSITIONS->SelectEntryPos( m_aPresetIndexes[ nEntry ] + 1 );
                else
                    mpLB_SLIDE_TRANSITIONS->SetNoSelection();
            }
        }
    }

    if( aEffect.mbDurationAmbiguous )
        mpLB_SPEED->SetNoSelection();
    else
        mpLB_SPEED->SelectEntryPos(
            (aEffect.mfDuration > 2.0 )
            ? 0 : (aEffect.mfDuration < 2.0)
            ? 2 : 1 );       // else FADE_SPEED_FAST

    if( aEffect.mbSoundAmbiguous )
    {
        mpLB_SOUND->SetNoSelection();
        maCurrentSoundFile = "";
    }
    else
    {
        maCurrentSoundFile = "";
        if( aEffect.mbStopSound )
        {
            mpLB_SOUND->SelectEntryPos( 1 );
        }
        else if( aEffect.mbSoundOn && !aEffect.maSound.isEmpty() )
        {
            tSoundListType::size_type nPos = 0;
            if( lcl_findSoundInList( maSoundList, aEffect.maSound, nPos ))
            {
                // skip first three entries
                mpLB_SOUND->SelectEntryPos( (sal_uInt16)nPos + 3 );
                maCurrentSoundFile = aEffect.maSound;
            }
        }
        else
        {
            mpLB_SOUND->SelectEntryPos( 0 );
        }
    }

    if( aEffect.mbLoopSoundAmbiguous )
    {
        mpCB_LOOP_SOUND->SetState( STATE_DONTKNOW );
    }
    else
    {
        mpCB_LOOP_SOUND->Check( aEffect.mbLoopSound );
    }

    if( aEffect.mbPresChangeAmbiguous )
    {
        mpRB_ADVANCE_ON_MOUSE->Check( sal_False );
        mpRB_ADVANCE_AUTO->Check( sal_False );
    }
    else
    {
        mpRB_ADVANCE_ON_MOUSE->Check( aEffect.mePresChange == PRESCHANGE_MANUAL );
        mpRB_ADVANCE_AUTO->Check( aEffect.mePresChange == PRESCHANGE_AUTO );
        mpMF_ADVANCE_AUTO_AFTER->SetValue( aEffect.mfTime * 100.0);
    }

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    mpCB_AUTO_PREVIEW->Check( pOptions->IsPreviewTransitions() == sal_True );

    mbUpdatingControls = false;

    updateControlState();
}

void SlideTransitionPane::updateControlState()
{
    mpLB_SLIDE_TRANSITIONS->Enable( mbHasSelection );
    mpLB_SPEED->Enable( mbHasSelection );
    mpLB_SOUND->Enable( mbHasSelection );
    mpCB_LOOP_SOUND->Enable( mbHasSelection && (mpLB_SOUND->GetSelectEntryPos() > 2));
    mpRB_ADVANCE_ON_MOUSE->Enable( mbHasSelection );
    mpRB_ADVANCE_AUTO->Enable( mbHasSelection );
    mpMF_ADVANCE_AUTO_AFTER->Enable( mbHasSelection && mpRB_ADVANCE_AUTO->IsChecked());

    mpPB_APPLY_TO_ALL->Enable( mbHasSelection );
    mpPB_PLAY->Enable( mbHasSelection );
    mpCB_AUTO_PREVIEW->Enable( mbHasSelection );
}

void SlideTransitionPane::updateSoundList()
{
    maSoundList.clear();

    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, maSoundList );
    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, maSoundList );

    lcl_FillSoundListBox( maSoundList, mpLB_SOUND );
}

void SlideTransitionPane::openSoundFileDialog()
{
    if( ! mpLB_SOUND->IsEnabled())
        return;

    SdOpenSoundFileDialog aFileDialog;

    OUString aFile;
    DBG_ASSERT( mpLB_SOUND->GetSelectEntryPos() == 2,
                "Dialog should only open when \"Other sound\" is selected" );
    aFile = SvtPathOptions().GetGraphicPath();

    aFileDialog.SetPath( aFile );

    bool bValidSoundFile( false );
    bool bQuitLoop( false );

    while( ! bQuitLoop &&
           aFileDialog.Execute() == ERRCODE_NONE )
    {
        aFile = aFileDialog.GetPath();
        tSoundListType::size_type nPos = 0;
        bValidSoundFile = lcl_findSoundInList( maSoundList, aFile, nPos );

        if( bValidSoundFile )
        {
            bQuitLoop = true;
        }
        else // not in sound list
        {
            // try to insert into gallery
            if( GalleryExplorer::InsertURL( GALLERY_THEME_USERSOUNDS, aFile ) )
            {
                updateSoundList();
                bValidSoundFile = lcl_findSoundInList( maSoundList, aFile, nPos );
                DBG_ASSERT( bValidSoundFile, "Adding sound to gallery failed" );

                bQuitLoop = true;
            }
            else
            {
                OUString aStrWarning(SD_RESSTR(STR_WARNING_NOSOUNDFILE));
                aStrWarning = aStrWarning.replaceFirst("%", aFile);
                WarningBox aWarningBox( NULL, WB_3DLOOK | WB_RETRY_CANCEL, aStrWarning );
                aWarningBox.SetModalInputMode (sal_True);
                bQuitLoop = (aWarningBox.Execute() != RET_RETRY);

                bValidSoundFile = false;
            }
        }

        if( bValidSoundFile )
            // skip first three entries in list
            mpLB_SOUND->SelectEntryPos( (sal_uInt16)nPos + 3 );
    }

    if( ! bValidSoundFile )
    {
        if( !maCurrentSoundFile.isEmpty() )
        {
            tSoundListType::size_type nPos = 0;
            if( lcl_findSoundInList( maSoundList, maCurrentSoundFile, nPos ))
                mpLB_SOUND->SelectEntryPos( (sal_uInt16)nPos + 3 );
            else
                mpLB_SOUND->SelectEntryPos( 0 );  // NONE
        }
        else
            mpLB_SOUND->SelectEntryPos( 0 );  // NONE
    }
}

impl::TransitionEffect SlideTransitionPane::getTransitionEffectFromControls() const
{
    impl::TransitionEffect aResult;
    aResult.setAllAmbiguous();

    // check first (aResult might be overwritten)
    if( mpLB_SLIDE_TRANSITIONS->IsEnabled() &&
        mpLB_SLIDE_TRANSITIONS->GetSelectEntryCount() > 0 )
    {
        TransitionPresetPtr pPreset = lcl_getTransitionPresetByUIName(
            mpDrawDoc, OUString( mpLB_SLIDE_TRANSITIONS->GetSelectEntry()));

        if( pPreset.get())
        {
            aResult = impl::TransitionEffect( *pPreset );
            aResult.setAllAmbiguous();
        }
        else
        {
            aResult.mnType = 0;
        }
        aResult.mbEffectAmbiguous = false;
    }

    // speed
    if( mpLB_SPEED->IsEnabled() &&
        mpLB_SPEED->GetSelectEntryCount() > 0 )
    {
        sal_uInt16 nPos = mpLB_SPEED->GetSelectEntryPos();
        aResult.mfDuration = (nPos == 0)
            ? 3.0
            : (nPos == 1)
            ? 2.0
            : 1.0;   // nPos == 2
        DBG_ASSERT( aResult.mfDuration != 1.0 || nPos == 2, "Invalid Listbox Entry" );

        aResult.mbDurationAmbiguous = false;
    }

    // slide-advance mode
    if( mpRB_ADVANCE_ON_MOUSE->IsEnabled() && mpRB_ADVANCE_AUTO->IsEnabled() &&
        (mpRB_ADVANCE_ON_MOUSE->IsChecked() || mpRB_ADVANCE_AUTO->IsChecked()))
    {
        if( mpRB_ADVANCE_ON_MOUSE->IsChecked())
            aResult.mePresChange = PRESCHANGE_MANUAL;
        else
        {
            aResult.mePresChange = PRESCHANGE_AUTO;
            if( mpMF_ADVANCE_AUTO_AFTER->IsEnabled())
            {
                aResult.mfTime = static_cast<double>(mpMF_ADVANCE_AUTO_AFTER->GetValue() ) / 100.0 ;
                aResult.mbTimeAmbiguous = false;
            }
        }

        aResult.mbPresChangeAmbiguous = false;
    }

    // sound
    if( mpLB_SOUND->IsEnabled())
    {
        maCurrentSoundFile = "";
        if( mpLB_SOUND->GetSelectEntryCount() > 0 )
        {
            sal_uInt16 nPos = mpLB_SOUND->GetSelectEntryPos();
            aResult.mbStopSound = nPos == 1;
            aResult.mbSoundOn = nPos > 1;
            if( aResult.mbStopSound )
            {
                aResult.maSound = OUString();
                aResult.mbSoundAmbiguous = false;
            }
            else
            {
                aResult.maSound = lcl_getSoundFileURL( maSoundList, mpLB_SOUND );
                aResult.mbSoundAmbiguous = false;
                maCurrentSoundFile = aResult.maSound;
            }
        }
    }

    // sound loop
    if( mpCB_LOOP_SOUND->IsEnabled() )
    {
        aResult.mbLoopSound = mpCB_LOOP_SOUND->IsChecked();
        aResult.mbLoopSoundAmbiguous = false;
    }

    return aResult;
}

void SlideTransitionPane::applyToSelectedPages()
{
    if( ! mbUpdatingControls )
    {
        ::sd::slidesorter::SharedPageSelection pSelectedPages( getSelectedPages());
        if( ! pSelectedPages->empty())
        {
            lcl_CreateUndoForPages( pSelectedPages, mrBase );
            lcl_ApplyToPages( pSelectedPages, getTransitionEffectFromControls() );
            mrBase.GetDocShell()->SetModified();
        }
        if( mpCB_AUTO_PREVIEW->IsEnabled() &&
            mpCB_AUTO_PREVIEW->IsChecked())
        {
            playCurrentEffect();
        }
    }
}

void SlideTransitionPane::playCurrentEffect()
{
    if( mxView.is() )
    {

        Reference< ::com::sun::star::animations::XAnimationNode > xNode;
        SlideShow::StartPreview( mrBase, mxView->getCurrentPage(), xNode );
    }
}

void SlideTransitionPane::addListener()
{
    Link aLink( LINK(this,SlideTransitionPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener (
        aLink,
        tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION
        | tools::EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION
        | tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
        | tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED);
}

void SlideTransitionPane::removeListener()
{
    Link aLink( LINK(this,SlideTransitionPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

IMPL_LINK(SlideTransitionPane,EventMultiplexerListener,
    tools::EventMultiplexerEvent*,pEvent)
{
    switch (pEvent->meEventId)
    {
        case tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION:
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
        case tools::EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION:
            onChangeCurrentPage();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
            mxView = Reference<drawing::XDrawView>();
            onSelectionChanged();
            onChangeCurrentPage();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            mbIsMainViewChangePending = true;
            break;

        case tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED:
            if (mbIsMainViewChangePending)
            {
                mbIsMainViewChangePending = false;

                // At this moment the controller may not yet been set at
                // model or ViewShellBase.  Take it from the view shell
                // passed with the event.
                if (mrBase.GetMainViewShell() != 0)
                {
                    mxView = Reference<drawing::XDrawView>::query(mrBase.GetController());
                    onSelectionChanged();
                    onChangeCurrentPage();
                }
            }
            break;

        default:
            break;
    }
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, ApplyToAllButtonClicked)
{
    DBG_ASSERT( mpDrawDoc, "Invalid Draw Document!" );
    if( !mpDrawDoc )
        return 0;

    ::sd::slidesorter::SharedPageSelection pPages (
        new ::sd::slidesorter::SlideSorterViewShell::PageSelection());

    sal_uInt16 nPageCount = mpDrawDoc->GetSdPageCount( PK_STANDARD );
    pPages->reserve( nPageCount );
    for( sal_uInt16 i=0; i<nPageCount; ++i )
    {
        SdPage * pPage = mpDrawDoc->GetSdPage( i, PK_STANDARD );
        if( pPage )
            pPages->push_back( pPage );
    }

    if( ! pPages->empty())
    {
        lcl_CreateUndoForPages( pPages, mrBase );
        lcl_ApplyToPages( pPages, getTransitionEffectFromControls() );
    }

    return 0;
}


IMPL_LINK_NOARG(SlideTransitionPane, PlayButtonClicked)
{
    playCurrentEffect();
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, SlideShowButtonClicked)
{
    mrBase.StartPresentation();
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, TransitionSelected)
{
    applyToSelectedPages();
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, AdvanceSlideRadioButtonToggled)
{
    updateControlState();
    applyToSelectedPages();
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, AdvanceTimeModified)
{
    applyToSelectedPages();
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, SpeedListBoxSelected)
{
    applyToSelectedPages();
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, SoundListBoxSelected)
{
    if( mpLB_SOUND->GetSelectEntryCount() )
    {
        sal_uInt16 nPos = mpLB_SOUND->GetSelectEntryPos();
        if( nPos == 2 )
        {
            // other sound ...
            openSoundFileDialog();
        }
    }
    updateControlState();
    applyToSelectedPages();
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, LoopSoundBoxChecked)
{
    applyToSelectedPages();
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, AutoPreviewClicked)
{
    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    pOptions->SetPreviewTransitions( mpCB_AUTO_PREVIEW->IsChecked() ? sal_True : sal_False );
    return 0;
}

IMPL_LINK_NOARG(SlideTransitionPane, LateInitCallback)
{
    const TransitionPresetList& rPresetList = TransitionPreset::getTransitionPresetList();
    TransitionPresetList::const_iterator aIter( rPresetList.begin() );
    const TransitionPresetList::const_iterator aEnd( rPresetList.end() );
    sal_uInt16 nIndex = 0;
    ::std::size_t nUIIndex = 0;
    while( aIter != aEnd )
    {
        TransitionPresetPtr pPreset = (*aIter++);
        const OUString aUIName( pPreset->getUIName() );
         if( !aUIName.isEmpty() )
        {
            mpLB_SLIDE_TRANSITIONS->InsertEntry( aUIName );
            m_aPresetIndexes[ nIndex ] = (sal_uInt16)nUIIndex;
            ++nUIIndex;
        }
        ++nIndex;
    }

    updateSoundList();
    updateControls();

    return 0;
}

::Window * createSlideTransitionPanel( ::Window* pParent, ViewShellBase& rBase, const cssu::Reference<css::frame::XFrame>& rxFrame )
{
    DialogListBox* pWindow = 0;

    DrawDocShell* pDocSh = rBase.GetDocShell();
    if( pDocSh )
    {
        pWindow = new DialogListBox( pParent, WB_CLIPCHILDREN|WB_TABSTOP|WB_AUTOHSCROLL );

        Size aMinSize( pWindow->LogicToPixel( Size( 72, 216 ), MAP_APPFONT ) );
        ::Window* pPaneWindow = new SlideTransitionPane( pWindow, rBase, aMinSize, pDocSh->GetDoc(), rxFrame );
        pWindow->SetChildWindow( pPaneWindow, aMinSize );
        pWindow->SetText( pPaneWindow->GetText() );
    }

    return pWindow;
}




sal_Int32 getSlideTransitionPanelMinimumHeight (::Window* pDialog)
{
    if (pDialog != NULL)
        return pDialog->LogicToPixel(Size( 72, 216 ), MAP_APPFONT).Height();
    else
        return 0;
}


} //  namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
