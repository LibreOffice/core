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
    String      maSound;
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

struct lcl_EqualsSoundFileName : public ::std::unary_function< String, bool >
{
    explicit lcl_EqualsSoundFileName( const String & rStr ) :
            maStr( rStr )
    {}

    bool operator() ( const String & rStr ) const
    {
        // note: formerly this was a case insensitive search for all
        // platforms. It seems more sensible to do this platform-dependent
#if defined( WNT )
        return maStr.EqualsIgnoreCaseAscii( rStr );
#else
        return maStr.Equals( rStr );
#endif
    }

private:
    String maStr;
};

// returns -1 if no object was found
bool lcl_findSoundInList( const ::std::vector< String > & rSoundList,
                          const String & rFileName,
                          ::std::vector< String >::size_type & rOutPosition )
{
    ::std::vector< String >::const_iterator aIt =
          ::std::find_if( rSoundList.begin(), rSoundList.end(),
                          lcl_EqualsSoundFileName( rFileName ));
    if( aIt != rSoundList.end())
    {
        rOutPosition = ::std::distance( rSoundList.begin(), aIt );
        return true;
    }

    return false;
}

String lcl_getSoundFileURL(
    const ::std::vector< String > & rSoundList,
    const ListBox & rListBox )
{
    String aResult;

    if( rListBox.GetSelectEntryCount() > 0 )
    {
        sal_uInt16 nPos = rListBox.GetSelectEntryPos();
        // the first three entries are no actual sounds
        if( nPos >= 3 )
        {
            DBG_ASSERT( (sal_uInt32)(rListBox.GetEntryCount() - 3) == rSoundList.size(),
                        "Sound list-box is not synchronized to sound list" );
            nPos -= 3;
            if( rSoundList.size() > nPos )
                aResult = rSoundList[ nPos ];
        }
    }

    return aResult;
}

struct lcl_AppendSoundToListBox : public ::std::unary_function< String, void >
{
    lcl_AppendSoundToListBox( ListBox & rListBox ) :
            mrListBox( rListBox )
    {}

    void operator() ( const String & rString ) const
    {
        INetURLObject aURL( rString );
        mrListBox.InsertEntry( aURL.GetBase(), LISTBOX_APPEND );
    }

private:
    ListBox & mrListBox;
};

void lcl_FillSoundListBox(
    const ::std::vector< String > & rSoundList,
    ListBox & rOutListBox )
{
    sal_uInt16 nCount = rOutListBox.GetEntryCount();

    // keep first three entries
    for( sal_uInt16 i=nCount - 1; i>=3; --i )
        rOutListBox.RemoveEntry( i );

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
    SdDrawDocument* pDoc ) :
        Control( pParent, SdResId( DLG_SLIDE_TRANSITION_PANE ) ),

        mrBase( rBase ),
        mpDrawDoc( pDoc ),
        maMinSize( rMinSize ),
        maFL_APPLY_TRANSITION( this, SdResId( FL_APPLY_TRANSITION ) ),
        maLB_SLIDE_TRANSITIONS( this, SdResId( LB_SLIDE_TRANSITIONS ) ),
        maFL_MODIFY_TRANSITION( this, SdResId( FL_MODIFY_TRANSITION ) ),
        maFT_SPEED( this, SdResId( FT_SPEED ) ),
        maLB_SPEED( this, SdResId( LB_SPEED ) ),
        maFT_SOUND( this, SdResId( FT_SOUND ) ),
        maLB_SOUND( this, SdResId( LB_SOUND ) ),
        maCB_LOOP_SOUND( this, SdResId( CB_LOOP_SOUND ) ),
        maFL_ADVANCE_SLIDE( this, SdResId( FL_ADVANCE_SLIDE ) ),
        maRB_ADVANCE_ON_MOUSE( this, SdResId( RB_ADVANCE_ON_MOUSE ) ),
        maRB_ADVANCE_AUTO( this, SdResId( RB_ADVANCE_AUTO ) ),
        maMF_ADVANCE_AUTO_AFTER( this, SdResId( MF_ADVANCE_AUTO_AFTER ) ),
        maFL_EMPTY1( this, SdResId( FL_EMPTY1 ) ),
        maPB_APPLY_TO_ALL( this, SdResId( PB_APPLY_TO_ALL ) ),
        maPB_PLAY( this, SdResId( PB_PLAY ) ),
        maPB_SLIDE_SHOW( this, SdResId( PB_SLIDE_SHOW ) ),
        maFL_EMPTY2( this, SdResId( FL_EMPTY2 ) ),
        maCB_AUTO_PREVIEW( this, SdResId( CB_AUTO_PREVIEW ) ),

        maSTR_NO_TRANSITION( SdResId( STR_NO_TRANSITION ) ),
        mbHasSelection( false ),
        mbUpdatingControls( false ),
        mbIsMainViewChangePending( false ),
        maLateInitTimer()
{
    // use no resource ids from here on
    FreeResource();

    // use bold font for group headings (same font for all fixed lines):
    Font font( maFL_APPLY_TRANSITION.GetFont() );
    font.SetWeight( WEIGHT_BOLD );
    maFL_APPLY_TRANSITION.SetFont( font );
    maFL_MODIFY_TRANSITION.SetFont( font );
    maFL_ADVANCE_SLIDE.SetFont( font );

    if( pDoc )
        mxModel.set( pDoc->getUnoModel(), uno::UNO_QUERY );
    // TODO: get correct view
    if( mxModel.is())
        mxView.set( mxModel->getCurrentController(), uno::UNO_QUERY );

    // fill list box of slide transitions
    maLB_SLIDE_TRANSITIONS.InsertEntry( maSTR_NO_TRANSITION );

    // set defaults
    maCB_AUTO_PREVIEW.Check();      // automatic preview on

    // update control states before adding handlers
    updateLayout();
    updateControls();

    // set handlers
    maPB_APPLY_TO_ALL.SetClickHdl( LINK( this, SlideTransitionPane, ApplyToAllButtonClicked ));
    maPB_PLAY.SetClickHdl( LINK( this, SlideTransitionPane, PlayButtonClicked ));
    maPB_SLIDE_SHOW.SetClickHdl( LINK( this, SlideTransitionPane, SlideShowButtonClicked ));

    maLB_SLIDE_TRANSITIONS.SetSelectHdl( LINK( this, SlideTransitionPane, TransitionSelected ));

    maLB_SPEED.SetSelectHdl( LINK( this, SlideTransitionPane, SpeedListBoxSelected ));
    maLB_SOUND.SetSelectHdl( LINK( this, SlideTransitionPane, SoundListBoxSelected ));
    maCB_LOOP_SOUND.SetClickHdl( LINK( this, SlideTransitionPane, LoopSoundBoxChecked ));

    maRB_ADVANCE_ON_MOUSE.SetToggleHdl( LINK( this, SlideTransitionPane, AdvanceSlideRadioButtonToggled ));
    maRB_ADVANCE_AUTO.SetToggleHdl( LINK( this, SlideTransitionPane, AdvanceSlideRadioButtonToggled ));
    maMF_ADVANCE_AUTO_AFTER.SetModifyHdl( LINK( this, SlideTransitionPane, AdvanceTimeModified ));
    maCB_AUTO_PREVIEW.SetClickHdl( LINK( this, SlideTransitionPane, AutoPreviewClicked ));
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
    updateLayout();
}




void SlideTransitionPane::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;
    UpdateLook();
}




void SlideTransitionPane::UpdateLook (void)
{
    SetBackground(::sfx2::sidebar::Theme::GetWallpaper(::sfx2::sidebar::Theme::Paint_PanelBackground));
    maFL_APPLY_TRANSITION.SetBackground(Wallpaper());
    maFL_MODIFY_TRANSITION.SetBackground(Wallpaper());;
    maFT_SPEED.SetBackground(Wallpaper());
    maFT_SOUND.SetBackground(Wallpaper());
    maFL_ADVANCE_SLIDE.SetBackground(Wallpaper());
    maFL_EMPTY1.SetBackground(Wallpaper());
    maFL_EMPTY2.SetBackground(Wallpaper());
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

void SlideTransitionPane::updateLayout()
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
    ::Size aCtrlSize = maCB_AUTO_PREVIEW.GetSizePixel();
    aCtrlSize.setWidth( maCB_AUTO_PREVIEW.CalcMinimumSize( nMaxWidth ).getWidth());
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    maCB_AUTO_PREVIEW.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line above check-box
    aCtrlSize = maFL_EMPTY2.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    maFL_EMPTY2.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // buttons "Play" and "Slide Show"
    long nPlayButtonWidth = maPB_PLAY.CalcMinimumSize().getWidth() + 2 * nOffsetBtnX;
    long nSlideShowButtonWidth = maPB_SLIDE_SHOW.CalcMinimumSize().getWidth() + 2 * nOffsetBtnX;

    if( (nPlayButtonWidth + nSlideShowButtonWidth + nOffsetX) <= nMaxWidth )
    {
        // place buttons side by side
        aCtrlSize = maPB_PLAY.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nPlayButtonWidth );
        maPB_PLAY.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aUpperLeft.setX( aUpperLeft.getX() + nPlayButtonWidth + nOffsetX );
        aCtrlSize.setWidth( nSlideShowButtonWidth );
        maPB_SLIDE_SHOW.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }
    else
    {
        // place buttons on top of each other
        aCtrlSize = maPB_SLIDE_SHOW.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nSlideShowButtonWidth );
        maPB_SLIDE_SHOW.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = maPB_PLAY.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
        aCtrlSize.setWidth( nPlayButtonWidth );
        maPB_PLAY.SetPosSizePixel( aUpperLeft, aCtrlSize );
    }

    // "Apply to All Slides" button
    aCtrlSize = maPB_APPLY_TO_ALL.GetSizePixel();
    aCtrlSize.setWidth( maPB_APPLY_TO_ALL.CalcMinimumSize( nMaxWidth ).getWidth() + 2 * nOffsetBtnX );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maPB_APPLY_TO_ALL.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line above "Apply to All Slides" button
    aCtrlSize = maFL_EMPTY1.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    maFL_EMPTY1.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // advance automatically after ... seconds
    long nItemWidth = maRB_ADVANCE_AUTO.CalcMinimumSize().getWidth();

    if( (nItemWidth + nMinCtrlWidth + nOffsetX) <= nMaxWidth )
    {
        long nBase = aUpperLeft.getY();

        // place controls side by side
        aCtrlSize = maRB_ADVANCE_AUTO.GetSizePixel();
        aUpperLeft.setY( nBase - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nItemWidth );
        maRB_ADVANCE_AUTO.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = maMF_ADVANCE_AUTO_AFTER.GetSizePixel();
        aUpperLeft.setY( nBase - aCtrlSize.getHeight() );
        aUpperLeft.setX( aUpperLeft.getX() + nItemWidth + nOffsetX );
        aCtrlSize.setWidth( nMinCtrlWidth );
        maMF_ADVANCE_AUTO_AFTER.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }
    else
    {
        // place controls on top of each other
        aCtrlSize = maMF_ADVANCE_AUTO_AFTER.GetSizePixel();
        aUpperLeft.setX( nOffsetX + nTextIndent );
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
        aCtrlSize.setWidth( nMinCtrlWidth );
        maMF_ADVANCE_AUTO_AFTER.SetPosSizePixel( aUpperLeft, aCtrlSize );

        aCtrlSize = maRB_ADVANCE_AUTO.GetSizePixel();
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
        aUpperLeft.setX( nOffsetX );
        aCtrlSize.setWidth( nItemWidth );
        maRB_ADVANCE_AUTO.SetPosSizePixel( aUpperLeft, aCtrlSize );
        aUpperLeft.setX( nOffsetX );
    }

    // check box "On mouse click"
    aCtrlSize = maRB_ADVANCE_ON_MOUSE.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maRB_ADVANCE_ON_MOUSE.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // fixed line "Advance slide"
    aCtrlSize = maFL_ADVANCE_SLIDE.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maFL_ADVANCE_SLIDE.SetPosSizePixel( aUpperLeft, aCtrlSize );

    // check box "Loop until next sound"
    long nFTSpeedWidth = maFT_SPEED.CalcMinimumSize().getWidth() + 2 * nOffsetX;
    long nFTSoundWidth = maFT_SOUND.CalcMinimumSize().getWidth() + 2 * nOffsetX;
    long nIndent = ::std::max( nFTSoundWidth, nFTSpeedWidth );

    bool bStack = ( (nIndent + nMinCtrlWidth + nOffsetX) > nMaxWidth );

    if( bStack )
        nIndent = nTextIndent;

    aCtrlSize = maCB_LOOP_SOUND.GetSizePixel();
    aCtrlSize.setWidth( nMaxWidth - nIndent );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    aUpperLeft.setX( nIndent );
    maCB_LOOP_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );

    aCtrlSize = maLB_SOUND.GetSizePixel();
    aCtrlSize.setWidth( ::std::max( nMaxWidth - nIndent, nMinCtrlWidth ) );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maLB_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );
    maLB_SOUND.SetDropDownLineCount( 8 );
    aUpperLeft.setX( nOffsetX );

    aCtrlSize = maFT_SOUND.GetSizePixel();
    if( bStack )
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    aCtrlSize.setWidth( nFTSoundWidth - 2 * nOffsetX );
    maFT_SOUND.SetPosSizePixel( aUpperLeft, aCtrlSize );

    aUpperLeft.setX( nIndent );
    aCtrlSize = maLB_SPEED.GetSizePixel();
    aCtrlSize.setWidth( ::std::max( nMaxWidth - nIndent, nMinCtrlWidth ) );
    aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight() - nOffsetY );
    maLB_SPEED.SetPosSizePixel( aUpperLeft, aCtrlSize );
    maLB_SPEED.SetDropDownLineCount( 3 );
    aUpperLeft.setX( nOffsetX );

    aCtrlSize = maFT_SPEED.GetSizePixel();
    if( bStack )
        aUpperLeft.setY( aUpperLeft.getY() - aCtrlSize.getHeight());
    aCtrlSize.setWidth( nFTSpeedWidth - 2 * nOffsetX );
    maFT_SPEED.SetPosSizePixel( aUpperLeft, aCtrlSize );

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
    maLB_SLIDE_TRANSITIONS.SetPosSizePixel( aUpperLeftCorner, aCtrlSize );
}

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
        maLB_SLIDE_TRANSITIONS.SetNoSelection();
    else
    {
        // ToDo: That 0 is "no transition" is documented nowhere except in the
        // CTOR of sdpage
        if( aEffect.mnType == 0 )
            maLB_SLIDE_TRANSITIONS.SelectEntryPos( 0 );
        else
        {
            sal_uInt16 nEntry = lcl_getTransitionEffectIndex( mpDrawDoc, aEffect );
            if( nEntry == LISTBOX_ENTRY_NOTFOUND )
                maLB_SLIDE_TRANSITIONS.SetNoSelection();
            else
            {
                // first entry in list is "none", so add 1 after translation
                if( m_aPresetIndexes.find( nEntry ) != m_aPresetIndexes.end())
                    maLB_SLIDE_TRANSITIONS.SelectEntryPos( m_aPresetIndexes[ nEntry ] + 1 );
                else
                    maLB_SLIDE_TRANSITIONS.SetNoSelection();
            }
        }
    }

    if( aEffect.mbDurationAmbiguous )
        maLB_SPEED.SetNoSelection();
    else
        maLB_SPEED.SelectEntryPos(
            (aEffect.mfDuration > 2.0 )
            ? 0 : (aEffect.mfDuration < 2.0)
            ? 2 : 1 );       // else FADE_SPEED_FAST

    if( aEffect.mbSoundAmbiguous )
    {
        maLB_SOUND.SetNoSelection();
        maCurrentSoundFile.Erase();
    }
    else
    {
        maCurrentSoundFile.Erase();
        if( aEffect.mbStopSound )
        {
            maLB_SOUND.SelectEntryPos( 1 );
        }
        else if( aEffect.mbSoundOn && aEffect.maSound.Len() > 0 )
        {
            tSoundListType::size_type nPos = 0;
            if( lcl_findSoundInList( maSoundList, aEffect.maSound, nPos ))
            {
                // skip first three entries
                maLB_SOUND.SelectEntryPos( (sal_uInt16)nPos + 3 );
                maCurrentSoundFile = aEffect.maSound;
            }
        }
        else
        {
            maLB_SOUND.SelectEntryPos( 0 );
        }
    }

    if( aEffect.mbLoopSoundAmbiguous )
    {
        maCB_LOOP_SOUND.SetState( STATE_DONTKNOW );
    }
    else
    {
        maCB_LOOP_SOUND.Check( aEffect.mbLoopSound );
    }

    if( aEffect.mbPresChangeAmbiguous )
    {
        maRB_ADVANCE_ON_MOUSE.Check( sal_False );
        maRB_ADVANCE_AUTO.Check( sal_False );
    }
    else
    {
        maRB_ADVANCE_ON_MOUSE.Check( aEffect.mePresChange == PRESCHANGE_MANUAL );
        maRB_ADVANCE_AUTO.Check( aEffect.mePresChange == PRESCHANGE_AUTO );
        maMF_ADVANCE_AUTO_AFTER.SetValue( aEffect.mfTime * 100.0);
    }

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    maCB_AUTO_PREVIEW.Check( pOptions->IsPreviewTransitions() == sal_True );

    mbUpdatingControls = false;

    updateControlState();
}

void SlideTransitionPane::updateControlState()
{
    maLB_SLIDE_TRANSITIONS.Enable( mbHasSelection );
    maLB_SPEED.Enable( mbHasSelection );
    maLB_SOUND.Enable( mbHasSelection );
    maCB_LOOP_SOUND.Enable( mbHasSelection && (maLB_SOUND.GetSelectEntryPos() > 2));
    maRB_ADVANCE_ON_MOUSE.Enable( mbHasSelection );
    maRB_ADVANCE_AUTO.Enable( mbHasSelection );
    maMF_ADVANCE_AUTO_AFTER.Enable( mbHasSelection && maRB_ADVANCE_AUTO.IsChecked());

    maPB_APPLY_TO_ALL.Enable( mbHasSelection );
    maPB_PLAY.Enable( mbHasSelection );
    maCB_AUTO_PREVIEW.Enable( mbHasSelection );
}

void SlideTransitionPane::updateSoundList()
{
    ::std::vector< String > aSoundList;

    GalleryExplorer::FillObjList( GALLERY_THEME_SOUNDS, aSoundList );
    GalleryExplorer::FillObjList( GALLERY_THEME_USERSOUNDS, aSoundList );

    size_t nCount = aSoundList.size();
    maSoundList.clear();
    maSoundList.reserve( nCount );
    for( size_t i =0 ; i < nCount; ++i )
    {
        // store copy of string in member list
        maSoundList.push_back( aSoundList[ i ] );
    }

    lcl_FillSoundListBox( maSoundList, maLB_SOUND );
}

void SlideTransitionPane::openSoundFileDialog()
{
    if( ! maLB_SOUND.IsEnabled())
        return;

    SdOpenSoundFileDialog aFileDialog;

    String aFile;
    DBG_ASSERT( maLB_SOUND.GetSelectEntryPos() == 2,
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
                String aStrWarning(SdResId(STR_WARNING_NOSOUNDFILE));
                OUString aStr('%');
                aStrWarning.SearchAndReplace( aStr , aFile );
                WarningBox aWarningBox( NULL, WB_3DLOOK | WB_RETRY_CANCEL, aStrWarning );
                aWarningBox.SetModalInputMode (sal_True);
                bQuitLoop = (aWarningBox.Execute() != RET_RETRY);

                bValidSoundFile = false;
            }
        }

        if( bValidSoundFile )
            // skip first three entries in list
            maLB_SOUND.SelectEntryPos( (sal_uInt16)nPos + 3 );
    }

    if( ! bValidSoundFile )
    {
        if( maCurrentSoundFile.Len() > 0 )
        {
            tSoundListType::size_type nPos = 0;
            if( lcl_findSoundInList( maSoundList, maCurrentSoundFile, nPos ))
                maLB_SOUND.SelectEntryPos( (sal_uInt16)nPos + 3 );
            else
                maLB_SOUND.SelectEntryPos( 0 );  // NONE
        }
        else
            maLB_SOUND.SelectEntryPos( 0 );  // NONE
    }
}

impl::TransitionEffect SlideTransitionPane::getTransitionEffectFromControls() const
{
    impl::TransitionEffect aResult;
    aResult.setAllAmbiguous();

    // check first (aResult might be overwritten)
    if( maLB_SLIDE_TRANSITIONS.IsEnabled() &&
        maLB_SLIDE_TRANSITIONS.GetSelectEntryCount() > 0 )
    {
        TransitionPresetPtr pPreset = lcl_getTransitionPresetByUIName(
            mpDrawDoc, OUString( maLB_SLIDE_TRANSITIONS.GetSelectEntry()));

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
    if( maLB_SPEED.IsEnabled() &&
        maLB_SPEED.GetSelectEntryCount() > 0 )
    {
        sal_uInt16 nPos = maLB_SPEED.GetSelectEntryPos();
        aResult.mfDuration = (nPos == 0)
            ? 3.0
            : (nPos == 1)
            ? 2.0
            : 1.0;   // nPos == 2
        DBG_ASSERT( aResult.mfDuration != 1.0 || nPos == 2, "Invalid Listbox Entry" );

        aResult.mbDurationAmbiguous = false;
    }

    // slide-advance mode
    if( maRB_ADVANCE_ON_MOUSE.IsEnabled() && maRB_ADVANCE_AUTO.IsEnabled() &&
        (maRB_ADVANCE_ON_MOUSE.IsChecked() || maRB_ADVANCE_AUTO.IsChecked()))
    {
        if( maRB_ADVANCE_ON_MOUSE.IsChecked())
            aResult.mePresChange = PRESCHANGE_MANUAL;
        else
        {
            aResult.mePresChange = PRESCHANGE_AUTO;
            if( maMF_ADVANCE_AUTO_AFTER.IsEnabled())
            {
                aResult.mfTime = static_cast<double>(maMF_ADVANCE_AUTO_AFTER.GetValue() ) / 100.0 ;
                aResult.mbTimeAmbiguous = false;
            }
        }

        aResult.mbPresChangeAmbiguous = false;
    }

    // sound
    if( maLB_SOUND.IsEnabled())
    {
        maCurrentSoundFile.Erase();
        if( maLB_SOUND.GetSelectEntryCount() > 0 )
        {
            sal_uInt16 nPos = maLB_SOUND.GetSelectEntryPos();
            aResult.mbStopSound = nPos == 1;
            aResult.mbSoundOn = nPos > 1;
            if( aResult.mbStopSound )
            {
                aResult.maSound = OUString();
                aResult.mbSoundAmbiguous = false;
            }
            else
            {
                aResult.maSound = lcl_getSoundFileURL( maSoundList, maLB_SOUND );
                aResult.mbSoundAmbiguous = false;
                maCurrentSoundFile = aResult.maSound;
            }
        }
    }

    // sound loop
    if( maCB_LOOP_SOUND.IsEnabled() )
    {
        aResult.mbLoopSound = maCB_LOOP_SOUND.IsChecked();
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
        if( maCB_AUTO_PREVIEW.IsEnabled() &&
            maCB_AUTO_PREVIEW.IsChecked())
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
    if( maLB_SOUND.GetSelectEntryCount() )
    {
        sal_uInt16 nPos = maLB_SOUND.GetSelectEntryPos();
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
    pOptions->SetPreviewTransitions( maCB_AUTO_PREVIEW.IsChecked() ? sal_True : sal_False );
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
            maLB_SLIDE_TRANSITIONS.InsertEntry( aUIName );
            m_aPresetIndexes[ nIndex ] = (sal_uInt16)nUIIndex;
            ++nUIIndex;
        }
        ++nIndex;
    }

    updateSoundList();
    updateControls();

    return 0;
}

::Window * createSlideTransitionPanel( ::Window* pParent, ViewShellBase& rBase )
{
    DialogListBox* pWindow = 0;

    DrawDocShell* pDocSh = rBase.GetDocShell();
    if( pDocSh )
    {
        pWindow = new DialogListBox( pParent, WB_CLIPCHILDREN|WB_TABSTOP|WB_AUTOHSCROLL );

        Size aMinSize( pWindow->LogicToPixel( Size( 72, 216 ), MAP_APPFONT ) );
        ::Window* pPaneWindow = new SlideTransitionPane( pWindow, rBase, aMinSize, pDocSh->GetDoc() );
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
