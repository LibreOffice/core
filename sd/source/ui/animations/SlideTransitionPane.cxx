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
#include "CustomAnimation.hrc"
#include "createslidetransitionpanel.hxx"

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

#include <sfx2/sidebar/Theme.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;

using ::sd::framework::FrameworkHelper;

namespace sd
{
namespace impl
{
struct TransitionEffect
{
    TransitionEffect() :
            mnType( 0 ),
            mnSubType( 0 ),
            mbDirection( true ),
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
        mbSoundOn = false;
        mbLoopSound = false;
        mbStopSound = false;

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
                rOutPage.SetStopSound( true );
                rOutPage.SetSound( false );
            }
            else
            {
                rOutPage.SetStopSound( false );
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
    bool  mbDirection;
    sal_Int32 mnFadeColor;

    // other settings
    double      mfDuration;
    double       mfTime;
    PresChange  mePresChange;
    bool        mbSoundOn;
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

// Local Helper Functions
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
    if (!pDocSh)
        return;
    ::svl::IUndoManager* pManager   = pDocSh->GetUndoManager();
    if (!pManager)
        return;
    SdDrawDocument* pDoc            = pDocSh->GetDoc();
    if (!pDoc)
        return;

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
        sal_Int32 nPos = rListBox->GetSelectEntryPos();
        // the first three entries are no actual sounds
        if( nPos >= 3 )
        {
            DBG_ASSERT( (sal_uInt32)(rListBox->GetEntryCount() - 3) == rSoundList.size(),
                        "Sound list-box is not synchronized to sound list" );
            nPos -= 3;
            if( rSoundList.size() > static_cast<size_t>(nPos) )
                return rSoundList[ nPos ];
        }
    }

    return OUString();
}

struct lcl_AppendSoundToListBox : public ::std::unary_function< OUString, void >
{
    explicit lcl_AppendSoundToListBox( ListBox*  rListBox ) :
            mrListBox( rListBox )
    {}

    void operator() ( const OUString & rString ) const
    {
        INetURLObject aURL( rString );
        mrListBox->InsertEntry( aURL.GetBase() );
    }

private:
    VclPtr<ListBox>  mrListBox;
};

void lcl_FillSoundListBox(
    const ::std::vector< OUString > & rSoundList,
    ListBox*  rOutListBox )
{
    sal_Int32 nCount = rOutListBox->GetEntryCount();

    // keep first three entries
    for( sal_Int32 i=nCount - 1; i>=3; --i )
        rOutListBox->RemoveEntry( i );

    ::std::for_each( rSoundList.begin(), rSoundList.end(),
                     lcl_AppendSoundToListBox( rOutListBox ));
}

} // anonymous namespace

namespace sd
{

// SlideTransitionPane
SlideTransitionPane::SlideTransitionPane(
    Window * pParent,
    ViewShellBase & rBase,
    SdDrawDocument* pDoc,
    const css::uno::Reference<css::frame::XFrame>& rxFrame ) :
        PanelLayout( pParent, "SlideTransitionsPanel", "modules/simpress/ui/slidetransitionspanel.ui", rxFrame ),

        mrBase( rBase ),
        mpDrawDoc( pDoc ),
        mbHasSelection( false ),
        mbUpdatingControls( false ),
        mbIsMainViewChangePending( false ),
        maLateInitTimer()
{
    get(mpLB_SLIDE_TRANSITIONS, "transitions_list");
    get(mpFT_VARIANT, "variant_label");
    get(mpLB_VARIANT, "variant_list");
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
    get(mpCB_AUTO_PREVIEW, "auto_preview");

    mpLB_SLIDE_TRANSITIONS->set_width_request(mpLB_SLIDE_TRANSITIONS->approximate_char_width() * 16);
    mpLB_SLIDE_TRANSITIONS->SetDropDownLineCount(4);

    mpLB_VARIANT->SetDropDownLineCount(4);

    if( pDoc )
        mxModel.set( pDoc->getUnoModel(), uno::UNO_QUERY );
    // TODO: get correct view
    if( mxModel.is())
        mxView.set( mxModel->getCurrentController(), uno::UNO_QUERY );

    // fill list box of slide transitions
    mpLB_SLIDE_TRANSITIONS->InsertEntry( SD_RESSTR( STR_SLIDETRANSITION_NONE ), Image( BitmapEx( "sd/cmd/transition-none.png" ) ) );
    m_aTransitionLBToSet.push_back( "" );

    // set defaults
    mpCB_AUTO_PREVIEW->Check();      // automatic preview on

    // update control states before adding handlers
    updateControls();

    // set handlers
    mpPB_APPLY_TO_ALL->SetClickHdl( LINK( this, SlideTransitionPane, ApplyToAllButtonClicked ));
    mpPB_PLAY->SetClickHdl( LINK( this, SlideTransitionPane, PlayButtonClicked ));

    mpLB_SLIDE_TRANSITIONS->SetSelectHdl( LINK( this, SlideTransitionPane, TransitionSelected ));

    mpLB_VARIANT->SetSelectHdl( LINK( this, SlideTransitionPane, VariantListBoxSelected ));
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
    disposeOnce();
}

void SlideTransitionPane::dispose()
{
    maLateInitTimer.Stop();
    removeListener();
    mpLB_SLIDE_TRANSITIONS.clear();
    mpFT_VARIANT.clear();
    mpLB_VARIANT.clear();
    mpFT_SPEED.clear();
    mpLB_SPEED.clear();
    mpFT_SOUND.clear();
    mpLB_SOUND.clear();
    mpCB_LOOP_SOUND.clear();
    mpRB_ADVANCE_ON_MOUSE.clear();
    mpRB_ADVANCE_AUTO.clear();
    mpMF_ADVANCE_AUTO_AFTER.clear();
    mpPB_APPLY_TO_ALL.clear();
    mpPB_PLAY.clear();
    mpCB_AUTO_PREVIEW.clear();
    PanelLayout::dispose();
}

void SlideTransitionPane::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;
    UpdateLook();
}

void SlideTransitionPane::UpdateLook()
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

::sd::slidesorter::SharedPageSelection SlideTransitionPane::getSelectedPages() const
{
    ::sd::slidesorter::SlideSorterViewShell * pSlideSorterViewShell
        = ::sd::slidesorter::SlideSorterViewShell::GetSlideSorter(mrBase);
    std::shared_ptr<sd::slidesorter::SlideSorterViewShell::PageSelection> pSelection;

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
    ::sd::slidesorter::SlideSorterViewShell::PageSelection::const_iterator aPageIt(
        pSelectedPages->begin());
    ::sd::slidesorter::SlideSorterViewShell::PageSelection::const_iterator aPageEndIt(
        pSelectedPages->end());

    // start with second page (note aIt != aEndIt, because ! aSelectedPages.empty())
    for( ++aPageIt; aPageIt != aPageEndIt; ++aPageIt )
    {
        if( *aPageIt )
            aEffect.compareWith( *(*aPageIt) );
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
            int nEntry = LISTBOX_ENTRY_NOTFOUND;
            const sd::TransitionPresetList& rPresetList = sd::TransitionPreset::getTransitionPresetList();
            sd::TransitionPresetPtr pFound;

            for( auto aIt: rPresetList )
            {
                if( aEffect.operator==( *aIt ))
                {
                    pFound = aIt;
                    // This function can be called before LateInit, with m_aSetToTransitionLBIndex
                    // not set up properly.  In that case nEntry will stay as
                    // LISTBOX_ENTRY_NOTFOUND. But in that case we will be called soon again, I think.
                    if( m_aSetToTransitionLBIndex.find( aIt->getSetId() ) != m_aSetToTransitionLBIndex.end() )
                        nEntry = m_aSetToTransitionLBIndex[aIt->getSetId()];
                    break;
                }
            }

            mpLB_VARIANT->Clear();
            if( nEntry == LISTBOX_ENTRY_NOTFOUND )
            {
                mpLB_SLIDE_TRANSITIONS->SetNoSelection();
                mpLB_VARIANT->Enable( false );
            }
            else
            {
                // Fill in the variant listbox
                for( auto aIt: rPresetList )
                {
                    if( aIt->getSetId().equals( pFound->getSetId() ) )
                    {
                        if( !aIt->getVariantLabel().isEmpty() )
                        {
                            mpLB_VARIANT->InsertEntry( aIt->getVariantLabel() );
                            if( aEffect.operator==( *aIt ))
                                mpLB_VARIANT->SelectEntryPos( mpLB_VARIANT->GetEntryCount()-1 );
                        }
                    }
                }
                if( mpLB_VARIANT->GetEntryCount() == 0 )
                    mpLB_VARIANT->Enable( false );
                else
                    mpLB_VARIANT->Enable();
                mpLB_SLIDE_TRANSITIONS->SelectEntryPos( nEntry );
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
        maCurrentSoundFile.clear();
    }
    else
    {
        maCurrentSoundFile.clear();
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
                mpLB_SOUND->SelectEntryPos( nPos + 3 );
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
        mpCB_LOOP_SOUND->SetState( TRISTATE_INDET );
    }
    else
    {
        mpCB_LOOP_SOUND->Check( aEffect.mbLoopSound );
    }

    if( aEffect.mbPresChangeAmbiguous )
    {
        mpRB_ADVANCE_ON_MOUSE->Check( false );
        mpRB_ADVANCE_AUTO->Check( false );
    }
    else
    {
        mpRB_ADVANCE_ON_MOUSE->Check( aEffect.mePresChange == PRESCHANGE_MANUAL );
        mpRB_ADVANCE_AUTO->Check( aEffect.mePresChange == PRESCHANGE_AUTO );
        mpMF_ADVANCE_AUTO_AFTER->SetValue( aEffect.mfTime * 100.0);
    }

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    mpCB_AUTO_PREVIEW->Check( pOptions->IsPreviewTransitions() );

    mbUpdatingControls = false;

    updateControlState();
}

void SlideTransitionPane::updateControlState()
{
    mpLB_SLIDE_TRANSITIONS->Enable( mbHasSelection );
    mpLB_VARIANT->Enable( mbHasSelection && mpLB_VARIANT->GetEntryCount() > 0 );
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
                ScopedVclPtrInstance< WarningBox > aWarningBox( nullptr, WB_3DLOOK | WB_RETRY_CANCEL, aStrWarning );
                aWarningBox->SetModalInputMode (true);
                bQuitLoop = (aWarningBox->Execute() != RET_RETRY);

                bValidSoundFile = false;
            }
        }

        if( bValidSoundFile )
            // skip first three entries in list
            mpLB_SOUND->SelectEntryPos( nPos + 3 );
    }

    if( ! bValidSoundFile )
    {
        if( !maCurrentSoundFile.isEmpty() )
        {
            tSoundListType::size_type nPos = 0;
            if( lcl_findSoundInList( maSoundList, maCurrentSoundFile, nPos ))
                mpLB_SOUND->SelectEntryPos( nPos + 3 );
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
        const sd::TransitionPresetList& rPresetList = sd::TransitionPreset::getTransitionPresetList();

        int nVariant = 0;
        bool bFound = false;
        for( auto aIter: rPresetList )
        {
            if( aIter->getSetId().equals(m_aTransitionLBToSet[mpLB_SLIDE_TRANSITIONS->GetSelectEntryPos()]) )
            {
                if( mpLB_VARIANT->GetSelectEntryPos() == nVariant)
                {
                    aResult = impl::TransitionEffect( *aIter );
                    aResult.setAllAmbiguous();
                    bFound = true;
                    break;
                }
                else
                {
                    nVariant++;
                }
            }
        }
        if( !bFound )
        {
            aResult.mnType = 0;
        }
        aResult.mbEffectAmbiguous = false;
    }

    // speed
    if( mpLB_SPEED->IsEnabled() &&
        mpLB_SPEED->GetSelectEntryCount() > 0 )
    {
        sal_Int32 nPos = mpLB_SPEED->GetSelectEntryPos();
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
        maCurrentSoundFile.clear();
        if( mpLB_SOUND->GetSelectEntryCount() > 0 )
        {
            sal_Int32 nPos = mpLB_SOUND->GetSelectEntryPos();
            aResult.mbStopSound = nPos == 1;
            aResult.mbSoundOn = nPos > 1;
            if( aResult.mbStopSound )
            {
                aResult.maSound.clear();
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
        Window *pFocusWindow = Application::GetFocusWindow();

        ::sd::slidesorter::SharedPageSelection pSelectedPages( getSelectedPages());
        impl::TransitionEffect aEffect = getTransitionEffectFromControls();
        if( ! pSelectedPages->empty())
        {
            lcl_CreateUndoForPages( pSelectedPages, mrBase );
            lcl_ApplyToPages( pSelectedPages, aEffect );
            mrBase.GetDocShell()->SetModified();
        }
        if( mpCB_AUTO_PREVIEW->IsEnabled() &&
            mpCB_AUTO_PREVIEW->IsChecked())
        {
            if (aEffect.mnType) // mnType = 0 denotes no transition
                playCurrentEffect();
            else
                stopEffects();
        }

        if (pFocusWindow)
            pFocusWindow->GrabFocus();
    }
}

void SlideTransitionPane::playCurrentEffect()
{
    if( mxView.is() )
    {

        Reference< css::animations::XAnimationNode > xNode;
        SlideShow::StartPreview( mrBase, mxView->getCurrentPage(), xNode );
    }
}

void SlideTransitionPane::stopEffects()
{
    if( mxView.is() )
    {
        SlideShow::Stop( mrBase );
    }
}

void SlideTransitionPane::addListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,SlideTransitionPane,EventMultiplexerListener) );
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
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,SlideTransitionPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

IMPL_LINK_TYPED(SlideTransitionPane,EventMultiplexerListener,
    tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case tools::EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION:
            onSelectionChanged();
            break;

        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
        case tools::EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION:
            onChangeCurrentPage();
            break;

        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
            mxView.clear();
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
                if (mrBase.GetMainViewShell() != nullptr)
                {
                    mxView.set(mrBase.GetController(), css::uno::UNO_QUERY);
                    onSelectionChanged();
                    onChangeCurrentPage();
                }
            }
            break;

        default:
            break;
    }
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, ApplyToAllButtonClicked, Button*, void)
{
    DBG_ASSERT( mpDrawDoc, "Invalid Draw Document!" );
    if( !mpDrawDoc )
        return;

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
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, PlayButtonClicked, Button*, void)
{
    playCurrentEffect();
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, TransitionSelected, ListBox&, void)
{
    mpLB_VARIANT->Clear();

    if( mpLB_SLIDE_TRANSITIONS->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
        return;

    if( mpLB_SLIDE_TRANSITIONS->GetSelectEntryPos() > 0)
    {
        const sd::TransitionPresetList& rPresetList = sd::TransitionPreset::getTransitionPresetList();

        for( auto aIt: rPresetList )
        {
            if( m_aSetToTransitionLBIndex[aIt->getSetId()] == mpLB_SLIDE_TRANSITIONS->GetSelectEntryPos() )
                mpLB_VARIANT->InsertEntry( aIt->getVariantLabel() );
        }
    }

    if( mpLB_VARIANT->GetEntryCount() == 0 )
    {
        mpLB_VARIANT->Enable( false );
    }
    else
    {
        mpLB_VARIANT->Enable();
        mpLB_VARIANT->SelectEntryPos( 0 );
    }

    applyToSelectedPages();
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, AdvanceSlideRadioButtonToggled, RadioButton&, void)
{
    updateControlState();
    applyToSelectedPages();
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, AdvanceTimeModified, Edit&, void)
{
    applyToSelectedPages();
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, VariantListBoxSelected, ListBox&, void)
{
    applyToSelectedPages();
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, SpeedListBoxSelected, ListBox&, void)
{
    applyToSelectedPages();
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, SoundListBoxSelected, ListBox&, void)
{
    if( mpLB_SOUND->GetSelectEntryCount() )
    {
        sal_Int32 nPos = mpLB_SOUND->GetSelectEntryPos();
        if( nPos == 2 )
        {
            // other sound...
            openSoundFileDialog();
        }
    }
    updateControlState();
    applyToSelectedPages();
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, LoopSoundBoxChecked, Button*, void)
{
    applyToSelectedPages();
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, AutoPreviewClicked, Button*, void)
{
    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    pOptions->SetPreviewTransitions( mpCB_AUTO_PREVIEW->IsChecked() );
}

IMPL_LINK_NOARG_TYPED(SlideTransitionPane, LateInitCallback, Timer *, void)
{
    const TransitionPresetList& rPresetList = TransitionPreset::getTransitionPresetList();

    for( auto aIter: rPresetList )
    {
        TransitionPresetPtr pPreset = aIter;
        const OUString sLabel( pPreset->getSetLabel() );
        if( !sLabel.isEmpty() )
        {
            if( m_aNumVariants.find( pPreset->getSetId() ) == m_aNumVariants.end() )
            {
                OUString sImageName("sd/cmd/transition-" + pPreset->getSetId() + ".png");

                mpLB_SLIDE_TRANSITIONS->InsertEntry( sLabel, Image( BitmapEx( sImageName) ) );

                m_aTransitionLBToSet.push_back( pPreset->getSetId() );

                assert( m_aTransitionLBToSet.size() == static_cast<size_t>( mpLB_SLIDE_TRANSITIONS->GetEntryCount() ) );

                m_aNumVariants[ pPreset->getSetId() ] = 1;
                m_aSetToTransitionLBIndex[ aIter->getSetId() ] = mpLB_SLIDE_TRANSITIONS->GetEntryCount() - 1;
            }
            else
            {
                m_aNumVariants[ pPreset->getSetId() ]++;
            }
        }
    }

    for( int i = 0; i < mpLB_SLIDE_TRANSITIONS->GetEntryCount(); ++i )
        SAL_INFO("sd.transitions", i << ":" << mpLB_SLIDE_TRANSITIONS->GetEntry( i ) << " (" << m_aTransitionLBToSet[i] << ")");

    for( auto aIter: rPresetList )
    {
        SAL_INFO("sd.transitions",
                 aIter->getPresetId() << ": " <<
                 m_aSetToTransitionLBIndex[ aIter->getSetId() ] <<
                 " (" << mpLB_SLIDE_TRANSITIONS->GetEntry( m_aSetToTransitionLBIndex[ aIter->getSetId() ] ) << ")" );
    }

    updateSoundList();
    updateControls();
}

vcl::Window * createSlideTransitionPanel( vcl::Window* pParent, ViewShellBase& rBase, const css::uno::Reference<css::frame::XFrame>& rxFrame )
{
    vcl::Window* pWindow = nullptr;

    DrawDocShell* pDocSh = rBase.GetDocShell();
    if( pDocSh )
    {
        pWindow = VclPtr<SlideTransitionPane>::Create( pParent, rBase, pDocSh->GetDoc(), rxFrame );
    }

    return pWindow;
}

} //  namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
