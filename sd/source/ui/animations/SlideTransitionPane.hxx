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

#include "EventMultiplexer.hxx"

#include "SlideSorterViewShell.hxx"
#include "TransitionPreset.hxx"

#include <vcl/ctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svtools/valueset.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vector>
#include <map>

class SdDrawDocument;

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
    virtual ~SlideTransitionPane();
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
    void stopEffects();

    void addListener();
    void removeListener();

    ::sd::slidesorter::SharedPageSelection getSelectedPages() const;

    void UpdateLook();

    DECL_LINK_TYPED( ApplyToAllButtonClicked, Button*, void );
    DECL_LINK_TYPED( PlayButtonClicked, Button*, void );
    DECL_LINK_TYPED( AutoPreviewClicked, Button*, void );

    DECL_LINK_TYPED( TransitionSelected, ValueSet *, void );
    DECL_LINK_TYPED( AdvanceSlideRadioButtonToggled, RadioButton&, void );
    DECL_LINK_TYPED( AdvanceTimeModified, Edit&, void );
    DECL_LINK_TYPED( VariantListBoxSelected, ListBox&, void );
    DECL_LINK_TYPED( DurationModifiedHdl, Edit&, void );
    DECL_LINK_TYPED( DurationLoseFocusHdl, Control&, void );
    DECL_LINK_TYPED( SoundListBoxSelected, ListBox&, void );
    DECL_LINK_TYPED( LoopSoundBoxChecked, Button*, void );
    DECL_LINK_TYPED( EventMultiplexerListener, tools::EventMultiplexerEvent&, void );
    DECL_LINK_TYPED(LateInitCallback, Timer *, void);

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

    typedef ::std::vector< OUString > tSoundListType;
    tSoundListType  maSoundList;
    mutable OUString maCurrentSoundFile;

    // How many variants each transition set has
    std::map< OUString, int > m_aNumVariants;

    Timer maLateInitTimer;
};

} //  namespace sd

// INCLUDED_SD_SOURCE_UI_ANIMATIONS_SLIDETRANSITIONPANE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
