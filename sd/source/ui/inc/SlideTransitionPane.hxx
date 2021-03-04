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
#ifndef INCLUDED_SD_SOURCE_UI_ANIMATIONS_SLIDETRANSITIONPANE_HXX
#define INCLUDED_SD_SOURCE_UI_ANIMATIONS_SLIDETRANSITIONPANE_HXX

#include "SlideSorterViewShell.hxx"

#include <svtools/valueset.hxx>
#include <sfx2/sidebar/ILayoutableWindow.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/weld.hxx>

#include <vector>
#include <map>

class SdDrawDocument;

namespace com::sun::star::drawing { class XDrawView; }
namespace com::sun::star::frame { class XModel; }
namespace sd::tools { class EventMultiplexerEvent; }

namespace sd
{

class TransitionPane;
class ViewShellBase;

namespace impl
{
    struct TransitionEffect;
}

class SlideTransitionPane : public PanelLayout
                          , public sfx2::sidebar::ILayoutableWindow
{
public:
    explicit SlideTransitionPane(
        weld::Widget* pParent,
        ViewShellBase & rBase);
    virtual ~SlideTransitionPane() override;

    // ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth) override;

    void onSelectionChanged();
    void onChangeCurrentPage();

private:
    void updateControls();
    void updateControlState();
    void updateVariants(size_t nPresetOffset);

    void updateSoundList();
    void openSoundFileDialog();

    impl::TransitionEffect getTransitionEffectFromControls() const;

    void applyToSelectedPages(bool bPreview);
    void playCurrentEffect();

    void addListener();
    void removeListener();

    ::sd::slidesorter::SharedPageSelection getSelectedPages() const;

    void Initialize(SdDrawDocument* pDoc);

    DECL_LINK( ApplyToAllButtonClicked, weld::Button&, void );
    DECL_LINK( PlayButtonClicked, weld::Button&, void );
    DECL_LINK( AutoPreviewClicked, weld::ToggleButton&, void );

    DECL_LINK( TransitionSelected, ValueSet*, void );
    DECL_LINK( AdvanceSlideRadioButtonToggled, weld::ToggleButton&, void );
    DECL_LINK( AdvanceTimeModified, weld::MetricSpinButton&, void );
    DECL_LINK( VariantListBoxSelected, weld::ComboBox&, void );
    DECL_LINK( DurationModifiedHdl, weld::MetricSpinButton&, void );
    DECL_LINK( DurationLoseFocusHdl, weld::Widget&, void );
    DECL_LINK( SoundListBoxSelected, weld::ComboBox&, void );
    DECL_LINK( LoopSoundBoxChecked, weld::ToggleButton&, void );
    DECL_LINK( EventMultiplexerListener, tools::EventMultiplexerEvent&, void );
    DECL_LINK(LateInitCallback, Timer *, void);

    ViewShellBase &   mrBase;
    SdDrawDocument *  mpDrawDoc;

    std::unique_ptr<TransitionPane> mxVS_TRANSITION_ICONS;
    std::unique_ptr<weld::CustomWeld> mxVS_TRANSITION_ICONSWin;
    std::unique_ptr<weld::Label> mxFT_VARIANT;
    std::unique_ptr<weld::ComboBox> mxLB_VARIANT;
    std::unique_ptr<weld::Label> mxFT_duration;
    std::unique_ptr<weld::MetricSpinButton> mxCBX_duration;
    std::unique_ptr<weld::Label> mxFT_SOUND;
    std::unique_ptr<weld::ComboBox> mxLB_SOUND;
    std::unique_ptr<weld::CheckButton> mxCB_LOOP_SOUND;
    std::unique_ptr<weld::RadioButton> mxRB_ADVANCE_ON_MOUSE;
    std::unique_ptr<weld::RadioButton> mxRB_ADVANCE_AUTO;
    std::unique_ptr<weld::MetricSpinButton> mxMF_ADVANCE_AUTO_AFTER;
    std::unique_ptr<weld::Button> mxPB_APPLY_TO_ALL;
    std::unique_ptr<weld::Button> mxPB_PLAY;
    std::unique_ptr<weld::CheckButton> mxCB_AUTO_PREVIEW;

    css::uno::Reference< css::drawing::XDrawView >             mxView;
    css::uno::Reference< css::frame::XModel >                  mxModel;

    bool         mbHasSelection;
    bool         mbUpdatingControls;
    bool         mbIsMainViewChangePending;

    std::vector<OUString>  maSoundList;
    mutable OUString maCurrentSoundFile;

    // How many variants each transition set has
    std::map< OUString, int > m_aNumVariants;

    Timer maLateInitTimer;
};

} //  namespace sd

// INCLUDED_SD_SOURCE_UI_ANIMATIONS_SLIDETRANSITIONPANE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
