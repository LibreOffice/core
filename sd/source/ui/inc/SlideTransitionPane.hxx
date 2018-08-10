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

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svtools/valueset.hxx>
#include <svx/sidebar/PanelLayout.hxx>

#include <vector>
#include <map>

class SdDrawDocument;

namespace com { namespace sun { namespace star { namespace drawing { class XDrawView; } } } }
namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace sd { namespace tools { class EventMultiplexerEvent; } }

namespace sd
{

class ViewShellBase;

namespace impl
{
    struct TransitionEffect;
}

class SlideTransitionPane : public PanelLayout
{
public:
    explicit SlideTransitionPane(
        Window * pParent,
        ViewShellBase & rBase,
        SdDrawDocument* pDoc,
        const css::uno::Reference<css::frame::XFrame>& rxFrame );
    explicit SlideTransitionPane(
        Window * pParent,
        ViewShellBase & rBase,
        SdDrawDocument* pDoc,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        bool bHorizontalLayout );
    virtual ~SlideTransitionPane() override;
    virtual void dispose() override;

    // Window
    virtual void DataChanged (const DataChangedEvent& rEvent) override;

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

    void UpdateLook();

    void Initialize(SdDrawDocument* pDoc);

    DECL_LINK( ApplyToAllButtonClicked, Button*, void );
    DECL_LINK( PlayButtonClicked, Button*, void );
    DECL_LINK( AutoPreviewClicked, Button*, void );

    DECL_LINK( TransitionSelected, ValueSet *, void );
    DECL_LINK( AdvanceSlideRadioButtonToggled, RadioButton&, void );
    DECL_LINK( AdvanceTimeModified, Edit&, void );
    DECL_LINK( VariantListBoxSelected, ListBox&, void );
    DECL_LINK( DurationModifiedHdl, Edit&, void );
    DECL_LINK( DurationLoseFocusHdl, Control&, void );
    DECL_LINK( SoundListBoxSelected, ListBox&, void );
    DECL_LINK( LoopSoundBoxChecked, Button*, void );
    DECL_LINK( EventMultiplexerListener, tools::EventMultiplexerEvent&, void );
    DECL_LINK(LateInitCallback, Timer *, void);

    ViewShellBase &   mrBase;
    SdDrawDocument *  mpDrawDoc;

    VclPtr<ValueSet>     mpVS_TRANSITION_ICONS;
    VclPtr<FixedText>    mpFT_VARIANT;
    VclPtr<ListBox>      mpLB_VARIANT;
    VclPtr<FixedText>    mpFT_duration;
    VclPtr<MetricBox>    mpCBX_duration;
    VclPtr<FixedText>    mpFT_SOUND;
    VclPtr<ListBox>      mpLB_SOUND;
    VclPtr<CheckBox>     mpCB_LOOP_SOUND;
    VclPtr<RadioButton>  mpRB_ADVANCE_ON_MOUSE;
    VclPtr<RadioButton>  mpRB_ADVANCE_AUTO;
    VclPtr<MetricField>  mpMF_ADVANCE_AUTO_AFTER;
    VclPtr<PushButton>   mpPB_APPLY_TO_ALL;
    VclPtr<PushButton>   mpPB_PLAY;
    VclPtr<CheckBox>     mpCB_AUTO_PREVIEW;

    css::uno::Reference< css::drawing::XDrawView >             mxView;
    css::uno::Reference< css::frame::XModel >                  mxModel;

    bool         mbHasSelection;
    bool         mbUpdatingControls;
    bool         mbIsMainViewChangePending;
    bool         mbHorizontalLayout;

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
