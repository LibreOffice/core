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

#include <com/sun/star/drawing/XDrawView.hpp>
#include <SlideTransitionPane.hxx>
#include <createslidetransitionpanel.hxx>

#include <TransitionPreset.hxx>
#include <sdresid.hxx>
#include <ViewShellBase.hxx>
#include <DrawDocShell.hxx>
#include <SlideSorterViewShell.hxx>
#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <sdpage.hxx>
#include <filedlg.hxx>
#include <strings.hrc>
#include <EventMultiplexer.hxx>

#include <sal/log.hxx>
#include <svx/gallery.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <tools/urlobj.hxx>
#include <slideshow.hxx>
#include <sdundogr.hxx>
#include <undoanim.hxx>
#include <optsitem.hxx>

#include <sfx2/sidebar/Theme.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;


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
    for( const auto& rpPage : *rpPages )
    {
        rEffect.applyTo( *rpPage );
    }
}

void lcl_CreateUndoForPages(
    const ::sd::slidesorter::SharedPageSelection& rpPages,
    ::sd::ViewShellBase const & rBase )
{
    ::sd::DrawDocShell* pDocSh      = rBase.GetDocShell();
    if (!pDocSh)
        return;
    SfxUndoManager* pManager   = pDocSh->GetUndoManager();
    if (!pManager)
        return;
    SdDrawDocument* pDoc            = pDocSh->GetDoc();
    if (!pDoc)
        return;

    OUString aComment( SdResId(STR_UNDO_SLIDE_PARAMS) );
    pManager->EnterListAction(aComment, aComment, 0, rBase.GetViewShellId());
    std::unique_ptr<SdUndoGroup> pUndoGroup(new SdUndoGroup( pDoc ));
    pUndoGroup->SetComment( aComment );

    for( const auto& rpPage : *rpPages )
    {
        pUndoGroup->AddAction( new sd::UndoTransition( pDoc, rpPage ) );
    }

    pManager->AddUndoAction( std::move(pUndoGroup) );
    pManager->LeaveListAction();
}

struct lcl_EqualsSoundFileName
{
    explicit lcl_EqualsSoundFileName( const OUString & rStr ) :
            maStr( rStr )
    {}

    bool operator() ( const OUString & rStr ) const
    {
        // note: formerly this was a case insensitive search for all
        // platforms. It seems more sensible to do this platform-dependent
        INetURLObject aURL(rStr);
#if defined(_WIN32)
        return maStr.equalsIgnoreAsciiCase( aURL.GetBase() );
#else
        return maStr == aURL.GetBase();
#endif
    }

private:
    OUString const maStr;
};

// returns -1 if no object was found
bool lcl_findSoundInList( const ::std::vector< OUString > & rSoundList,
                          const OUString & rFileName,
                          ::std::vector< OUString >::size_type & rOutPosition )
{
    INetURLObject aURL(rFileName);
    ::std::vector< OUString >::const_iterator aIt =
          ::std::find_if( rSoundList.begin(), rSoundList.end(),
                          lcl_EqualsSoundFileName( aURL.GetBase()));
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
    if( rListBox->GetSelectedEntryCount() > 0 )
    {
        sal_Int32 nPos = rListBox->GetSelectedEntryPos();
        // the first three entries are no actual sounds
        if( nPos >= 3 )
        {
            DBG_ASSERT( static_cast<sal_uInt32>(rListBox->GetEntryCount() - 3) == rSoundList.size(),
                        "Sound list-box is not synchronized to sound list" );
            nPos -= 3;
            if( rSoundList.size() > static_cast<size_t>(nPos) )
                return rSoundList[ nPos ];
        }
    }

    return OUString();
}

struct lcl_AppendSoundToListBox
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

/// Returns an offset into the list of transition presets
size_t getPresetOffset( const sd::impl::TransitionEffect &rEffect )
{
    const sd::TransitionPresetList& rPresetList =
        sd::TransitionPreset::getTransitionPresetList();
    sd::TransitionPresetPtr pFound;

    size_t nIdx = 0;
    for( const auto& aIt: rPresetList )
    {
        if( rEffect.operator==( *aIt ))
            break;
        nIdx++;
    }
    return nIdx;
}

} // anonymous namespace

namespace sd
{

class TransitionPane : public ValueSet
{
public:
    explicit TransitionPane( vcl::Window *pParent )
        : ValueSet( pParent, WB_TABSTOP | WB_3DLOOK | WB_BORDER |
                  WB_ITEMBORDER | WB_FLATVALUESET | WB_VSCROLL )
    {
        EnableFullItemMode( false );
        set_hexpand( true );
        set_vexpand( true );
        SetExtraSpacing( 2 );
    }
    virtual ~TransitionPane() override { disposeOnce(); }

    virtual Size GetOptimalSize() const override
    {
        return LogicToPixel(Size(70, 88), MapMode(MapUnit::MapAppFont));
    }
};

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
        mbHorizontalLayout( false ),
        maLateInitTimer()
{
    Initialize(pDoc);
}

SlideTransitionPane::SlideTransitionPane(
    Window * pParent,
    ViewShellBase & rBase,
    SdDrawDocument* pDoc,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    bool /*bHorizontalLayout*/ ) :
        PanelLayout( pParent, "SlideTransitionsPanel", "modules/simpress/ui/slidetransitionspanelhorizontal.ui", rxFrame ),

        mrBase( rBase ),
        mpDrawDoc( pDoc ),
        mbHasSelection( false ),
        mbUpdatingControls( false ),
        mbIsMainViewChangePending( false ),
        mbHorizontalLayout( true ),
        maLateInitTimer()
{
    Initialize(pDoc);
}

void SlideTransitionPane::Initialize(SdDrawDocument* pDoc)
{
    get(mpFT_VARIANT, "variant_label");
    get(mpLB_VARIANT, "variant_list");
    get(mpFT_duration, "duration_label");
    get(mpCBX_duration, "transition_duration");
    get(mpFT_SOUND, "sound_label");
    get(mpLB_SOUND, "sound_list");
    get(mpCB_LOOP_SOUND, "loop_sound" );
    get(mpRB_ADVANCE_ON_MOUSE, "rb_mouse_click");
    get(mpRB_ADVANCE_AUTO, "rb_auto_after");
    get(mpMF_ADVANCE_AUTO_AFTER, "auto_after_value");
    auto nMax = mpMF_ADVANCE_AUTO_AFTER->GetMax();
    mpMF_ADVANCE_AUTO_AFTER->SetMax(1000);
    Size aOptimalSize(mpMF_ADVANCE_AUTO_AFTER->CalcMinimumSize());
    mpMF_ADVANCE_AUTO_AFTER->set_width_request(aOptimalSize.Width());
    mpMF_ADVANCE_AUTO_AFTER->SetMax(nMax);
    get(mpPB_APPLY_TO_ALL, "apply_to_all");
    get(mpPB_PLAY, "play");
    get(mpCB_AUTO_PREVIEW, "auto_preview");

    VclPtr<VclAlignment> xAlign;
    get(xAlign, "transitions_icons");

    mpVS_TRANSITION_ICONS = VclPtr<TransitionPane>::Create(xAlign);
    mpVS_TRANSITION_ICONS->Show();

    mpLB_VARIANT->SetDropDownLineCount(4);

    if( pDoc )
        mxModel.set( pDoc->getUnoModel(), uno::UNO_QUERY );
    // TODO: get correct view
    if( mxModel.is())
        mxView.set( mxModel->getCurrentController(), uno::UNO_QUERY );

    // dummy list box of slide transitions for startup.
    mpVS_TRANSITION_ICONS->InsertItem(
        0, Image( StockImage::Yes, "sd/cmd/transition-none.png" ),
        SdResId( STR_SLIDETRANSITION_NONE ),
        VALUESET_APPEND, /* show legend */ true );
    mpVS_TRANSITION_ICONS->RecalculateItemSizes();

    mpCBX_duration->InsertValue(100, FieldUnit::CUSTOM);
    mpCBX_duration->InsertValue(200, FieldUnit::CUSTOM);
    mpCBX_duration->InsertValue(300, FieldUnit::CUSTOM);
    mpCBX_duration->AdaptDropDownLineCountToMaximum();

    // set defaults
    mpCB_AUTO_PREVIEW->Check();      // automatic preview on

    // update control states before adding handlers
    updateControls();

    // set handlers
    mpPB_APPLY_TO_ALL->SetClickHdl( LINK( this, SlideTransitionPane, ApplyToAllButtonClicked ));
    mpPB_PLAY->SetClickHdl( LINK( this, SlideTransitionPane, PlayButtonClicked ));

    mpVS_TRANSITION_ICONS->SetSelectHdl( LINK( this, SlideTransitionPane, TransitionSelected ));

    mpLB_VARIANT->SetSelectHdl( LINK( this, SlideTransitionPane, VariantListBoxSelected ));
    mpCBX_duration->SetModifyHdl(LINK( this, SlideTransitionPane, DurationModifiedHdl));
    mpCBX_duration->SetLoseFocusHdl(LINK( this, SlideTransitionPane, DurationLoseFocusHdl));
    mpLB_SOUND->SetSelectHdl( LINK( this, SlideTransitionPane, SoundListBoxSelected ));
    mpCB_LOOP_SOUND->SetClickHdl( LINK( this, SlideTransitionPane, LoopSoundBoxChecked ));

    mpRB_ADVANCE_ON_MOUSE->SetToggleHdl( LINK( this, SlideTransitionPane, AdvanceSlideRadioButtonToggled ));
    mpRB_ADVANCE_AUTO->SetToggleHdl( LINK( this, SlideTransitionPane, AdvanceSlideRadioButtonToggled ));
    mpMF_ADVANCE_AUTO_AFTER->SetModifyHdl( LINK( this, SlideTransitionPane, AdvanceTimeModified ));
    mpCB_AUTO_PREVIEW->SetClickHdl( LINK( this, SlideTransitionPane, AutoPreviewClicked ));
    addListener();

    maLateInitTimer.SetTimeout(200);
    maLateInitTimer.SetInvokeHandler(LINK(this, SlideTransitionPane, LateInitCallback));
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
    mpVS_TRANSITION_ICONS.disposeAndClear();
    mpFT_VARIANT.clear();
    mpLB_VARIANT.clear();
    mpFT_duration.clear();
    mpCBX_duration.clear();
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

void SlideTransitionPane::DataChanged (const DataChangedEvent&)
{
    UpdateLook();
}

void SlideTransitionPane::UpdateLook()
{
    if( mbHorizontalLayout )
    {
        SetBackground(Wallpaper());
    }
    else
    {
        SetBackground(::sfx2::sidebar::Theme::GetWallpaper(::sfx2::sidebar::Theme::Paint_PanelBackground));
        mpFT_duration->SetBackground(Wallpaper());
        mpFT_SOUND->SetBackground(Wallpaper());
    }
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
        pSelection.reset(new sd::slidesorter::SlideSorterViewShell::PageSelection);
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

    // start with second page (note aIt != aEndIt, because ! aSelectedPages.empty())
    for( const auto& rpPage : *pSelectedPages )
    {
        if( rpPage )
            aEffect.compareWith( *rpPage );
    }

    // detect current slide effect
    if( aEffect.mbEffectAmbiguous )
    {
        SAL_WARN( "sd.transitions", "Unusual, ambiguous transition effect" );
        mpVS_TRANSITION_ICONS->SetNoSelection();
    }
    else
    {
        // ToDo: That 0 is "no transition" is documented nowhere except in the
        // CTOR of sdpage
        if( aEffect.mnType == 0 )
            mpVS_TRANSITION_ICONS->SetNoSelection();
        else
            updateVariants( getPresetOffset( aEffect ) );
    }

    if( aEffect.mbDurationAmbiguous )
    {
        mpCBX_duration->SetText("");
        mpCBX_duration->SetNoSelection();
    }
    else
    {
        mpCBX_duration->SetValue( (aEffect.mfDuration)*100.0 );
    }

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
            std::vector<OUString>::size_type nPos = 0;
            if( lcl_findSoundInList( maSoundList, aEffect.maSound, nPos ))
            {
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

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
    mpCB_AUTO_PREVIEW->Check( pOptions->IsPreviewTransitions() );

    mbUpdatingControls = false;

    updateControlState();
}

void SlideTransitionPane::updateControlState()
{
    mpVS_TRANSITION_ICONS->Enable( mbHasSelection );
    mpLB_VARIANT->Enable( mbHasSelection && mpLB_VARIANT->GetEntryCount() > 0 );
    mpCBX_duration->Enable( mbHasSelection );
    mpLB_SOUND->Enable( mbHasSelection );
    mpCB_LOOP_SOUND->Enable( mbHasSelection && (mpLB_SOUND->GetSelectedEntryPos() > 2));
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

    SdOpenSoundFileDialog aFileDialog(GetFrameWeld());

    DBG_ASSERT( mpLB_SOUND->GetSelectedEntryPos() == 2,
                "Dialog should only open when \"Other sound\" is selected" );

    bool bValidSoundFile( false );
    bool bQuitLoop( false );

    while( ! bQuitLoop &&
           aFileDialog.Execute() == ERRCODE_NONE )
    {
        OUString aFile = aFileDialog.GetPath();
        std::vector<OUString>::size_type nPos = 0;
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
                OUString aStrWarning(SdResId(STR_WARNING_NOSOUNDFILE));
                aStrWarning = aStrWarning.replaceFirst("%", aFile);
                std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(nullptr,
                                                           VclMessageType::Warning, VclButtonsType::NONE,
                                                           aStrWarning));
                xWarn->add_button(Button::GetStandardText(StandardButtonType::Retry), RET_RETRY);
                xWarn->add_button(Button::GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
                bQuitLoop = (xWarn->run() != RET_RETRY);

                bValidSoundFile = false;
            }
        }

        if( bValidSoundFile )
            // skip first three entries in list
            mpLB_SOUND->SelectEntryPos( nPos + 3 );
    }

    if(  bValidSoundFile )
        return;

    if( !maCurrentSoundFile.isEmpty() )
    {
        std::vector<OUString>::size_type nPos = 0;
        if( lcl_findSoundInList( maSoundList, maCurrentSoundFile, nPos ))
            mpLB_SOUND->SelectEntryPos( nPos + 3 );
        else
            mpLB_SOUND->SelectEntryPos( 0 );  // NONE
    }
    else
        mpLB_SOUND->SelectEntryPos( 0 );  // NONE
}

impl::TransitionEffect SlideTransitionPane::getTransitionEffectFromControls() const
{
    impl::TransitionEffect aResult;
    aResult.setAllAmbiguous();

    // check first (aResult might be overwritten)
    if(  mpVS_TRANSITION_ICONS->IsEnabled() &&
        !mpVS_TRANSITION_ICONS->IsNoSelection() &&
         mpVS_TRANSITION_ICONS->GetSelectedItemId() > 0 )
    {
        const sd::TransitionPresetList& rPresetList = sd::TransitionPreset::getTransitionPresetList();
        auto aSelected = rPresetList.begin();
        std::advance( aSelected, mpVS_TRANSITION_ICONS->GetSelectedItemId() - 1);

        if( mpLB_VARIANT->GetSelectedEntryPos() == LISTBOX_ENTRY_NOTFOUND )
        {
            // Transition with just one effect.
            aResult = impl::TransitionEffect( **aSelected );
            aResult.setAllAmbiguous();
        }
        else
        {
            int nVariant = 0;
            bool bFound = false;
            for( const auto& aIter: rPresetList )
            {
                if( aIter->getSetId() == (*aSelected)->getSetId() )
                {
                    if( mpLB_VARIANT->GetSelectedEntryPos() == nVariant)
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
        }
        aResult.mbEffectAmbiguous = false;
    }
    else if (mpVS_TRANSITION_ICONS->IsNoSelection())
    {
        aResult.mbEffectAmbiguous = false;
    }

    //duration

    if( mpCBX_duration->IsEnabled() && (!(mpCBX_duration->GetText()).isEmpty()) )
    {
        aResult.mfDuration = static_cast<double>(mpCBX_duration->GetValue())/100.0;
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
        if( mpLB_SOUND->GetSelectedEntryCount() > 0 )
        {
            sal_Int32 nPos = mpLB_SOUND->GetSelectedEntryPos();
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

void SlideTransitionPane::applyToSelectedPages(bool bPreview = true)
{
    if(  mbUpdatingControls )
        return;

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
        mpCB_AUTO_PREVIEW->IsChecked() && bPreview)
    {
        if (aEffect.mnType) // mnType = 0 denotes no transition
            playCurrentEffect();
        else if( mxView.is() )
            SlideShow::Stop( mrBase );
    }

    if (pFocusWindow)
        pFocusWindow->GrabFocus();
}

void SlideTransitionPane::playCurrentEffect()
{
    if( mxView.is() )
    {

        Reference< css::animations::XAnimationNode > xNode;
        SlideShow::StartPreview( mrBase, mxView->getCurrentPage(), xNode );
    }
}

void SlideTransitionPane::addListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,SlideTransitionPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener( aLink );
}

void SlideTransitionPane::removeListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,SlideTransitionPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

IMPL_LINK(SlideTransitionPane,EventMultiplexerListener,
    tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::EditViewSelection:
            onSelectionChanged();
            break;

        case EventMultiplexerEventId::CurrentPageChanged:
        case EventMultiplexerEventId::SlideSortedSelection:
            onChangeCurrentPage();
            break;

        case EventMultiplexerEventId::MainViewRemoved:
            mxView.clear();
            onSelectionChanged();
            onChangeCurrentPage();
            break;

        case EventMultiplexerEventId::MainViewAdded:
            mbIsMainViewChangePending = true;
            break;

        case EventMultiplexerEventId::ConfigurationUpdated:
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
            if (rEvent.meEventId != EventMultiplexerEventId::Disposing)
            {
                onSelectionChanged();
                onChangeCurrentPage();
            }
            break;
    }
}

IMPL_LINK_NOARG(SlideTransitionPane, ApplyToAllButtonClicked, Button*, void)
{
    DBG_ASSERT( mpDrawDoc, "Invalid Draw Document!" );
    if( !mpDrawDoc )
        return;

    ::sd::slidesorter::SharedPageSelection pPages (
        new ::sd::slidesorter::SlideSorterViewShell::PageSelection);

    sal_uInt16 nPageCount = mpDrawDoc->GetSdPageCount( PageKind::Standard );
    pPages->reserve( nPageCount );
    for( sal_uInt16 i=0; i<nPageCount; ++i )
    {
        SdPage * pPage = mpDrawDoc->GetSdPage( i, PageKind::Standard );
        if( pPage )
            pPages->push_back( pPage );
    }

    if( ! pPages->empty())
    {
        lcl_CreateUndoForPages( pPages, mrBase );
        lcl_ApplyToPages( pPages, getTransitionEffectFromControls() );
    }
}

IMPL_LINK_NOARG(SlideTransitionPane, PlayButtonClicked, Button*, void)
{
    playCurrentEffect();
}

IMPL_LINK_NOARG(SlideTransitionPane, TransitionSelected, ValueSet *, void)
{
    updateVariants( mpVS_TRANSITION_ICONS->GetSelectedItemId() - 1 );
    applyToSelectedPages();
}

/// we use an integer offset into the list of transition presets
void SlideTransitionPane::updateVariants( size_t nPresetOffset )
{
    const sd::TransitionPresetList& rPresetList = sd::TransitionPreset::getTransitionPresetList();
    mpLB_VARIANT->Clear();
    mpVS_TRANSITION_ICONS->SetNoSelection();

    if( nPresetOffset >= rPresetList.size() )
    {
        mpLB_VARIANT->Enable( false );
    }
    else
    {
        auto pFound = rPresetList.begin();
        std::advance( pFound, nPresetOffset );

        // Fill in the variant listbox
        size_t nFirstItem = 0, nItem = 1;
        for( const auto& aIt: rPresetList )
        {
            if( aIt->getSetId() == (*pFound)->getSetId() )
            {
                if (!nFirstItem)
                    nFirstItem = nItem;
                if( !aIt->getVariantLabel().isEmpty() )
                {
                    mpLB_VARIANT->InsertEntry( aIt->getVariantLabel() );
                    if( *pFound == aIt )
                        mpLB_VARIANT->SelectEntryPos( mpLB_VARIANT->GetEntryCount()-1 );
                }
            }
            nItem++;
        }

        if( mpLB_VARIANT->GetEntryCount() == 0 )
            mpLB_VARIANT->Enable( false );
        else
            mpLB_VARIANT->Enable();

        // item has the id of the first transition from this set.
        mpVS_TRANSITION_ICONS->SelectItem( nFirstItem );
    }
}

IMPL_LINK_NOARG(SlideTransitionPane, AdvanceSlideRadioButtonToggled, RadioButton&, void)
{
    updateControlState();
    applyToSelectedPages(false);
}

IMPL_LINK_NOARG(SlideTransitionPane, AdvanceTimeModified, Edit&, void)
{
    applyToSelectedPages(false);
}

IMPL_LINK_NOARG(SlideTransitionPane, VariantListBoxSelected, ListBox&, void)
{
    applyToSelectedPages();
}

IMPL_LINK_NOARG(SlideTransitionPane, DurationModifiedHdl, Edit&, void)
{
    double duration_value = static_cast<double>(mpCBX_duration->GetValue());
    if(duration_value <= 0.0)
        mpCBX_duration->SetValue(0);
    else
        mpCBX_duration->SetValue(duration_value);
}

IMPL_LINK_NOARG(SlideTransitionPane, DurationLoseFocusHdl, Control&, void)
{
    applyToSelectedPages();
}

IMPL_LINK_NOARG(SlideTransitionPane, SoundListBoxSelected, ListBox&, void)
{
    if( mpLB_SOUND->GetSelectedEntryCount() )
    {
        sal_Int32 nPos = mpLB_SOUND->GetSelectedEntryPos();
        if( nPos == 2 )
        {
            // other sound...
            openSoundFileDialog();
        }
    }
    updateControlState();
    applyToSelectedPages();
}

IMPL_LINK_NOARG(SlideTransitionPane, LoopSoundBoxChecked, Button*, void)
{
    applyToSelectedPages();
}

IMPL_LINK_NOARG(SlideTransitionPane, AutoPreviewClicked, Button*, void)
{
    SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
    pOptions->SetPreviewTransitions( mpCB_AUTO_PREVIEW->IsChecked() );
}

IMPL_LINK_NOARG(SlideTransitionPane, LateInitCallback, Timer *, void)
{
    const TransitionPresetList& rPresetList = TransitionPreset::getTransitionPresetList();

    size_t nPresetOffset = 0;
    for( const TransitionPresetPtr& pPreset: rPresetList )
    {
        const OUString sLabel( pPreset->getSetLabel() );
        if( !sLabel.isEmpty() )
        {
            if( m_aNumVariants.find( pPreset->getSetId() ) == m_aNumVariants.end() )
            {
                OUString sImageName("sd/cmd/transition-" + pPreset->getSetId() + ".png");
                BitmapEx aIcon( sImageName );
                if ( aIcon.IsEmpty() ) // need a fallback
                    sImageName = "sd/cmd/transition-none.png";

                mpVS_TRANSITION_ICONS->InsertItem(
                    nPresetOffset + 1, Image(StockImage::Yes, sImageName), sLabel,
                    VALUESET_APPEND, /* show legend */ true );

                m_aNumVariants[ pPreset->getSetId() ] = 1;
            }
            else
            {
                m_aNumVariants[ pPreset->getSetId() ]++;
            }
        }
        nPresetOffset++;
    }
    mpVS_TRANSITION_ICONS->RecalculateItemSizes();

    SAL_INFO( "sd.transitions", "Item transition offsets in ValueSet:");
    for( size_t i = 0; i < mpVS_TRANSITION_ICONS->GetItemCount(); ++i )
        SAL_INFO( "sd.transitions", i << ":" << mpVS_TRANSITION_ICONS->GetItemId( i ) );

    nPresetOffset = 0;
    SAL_INFO( "sd.transitions", "Transition presets by offsets:");
    for( const auto& aIter: rPresetList )
    {
        SAL_INFO( "sd.transitions", nPresetOffset++ << " " <<
                  aIter->getPresetId() << ": " << aIter->getSetId() );
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
