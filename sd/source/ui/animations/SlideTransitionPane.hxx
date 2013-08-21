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
#ifndef SD_SLIDETRANSITIONPANE_HXX
#define SD_SLIDETRANSITIONPANE_HXX

#include "EventMultiplexer.hxx"

#include "SlideSorterViewShell.hxx"

#include <vcl/ctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
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
        ::Window * pParent,
        ViewShellBase & rBase,
        const Size& rMinSize,
        SdDrawDocument* pDoc,
        const cssu::Reference<css::frame::XFrame>& rxFrame );
    virtual ~SlideTransitionPane();

    // Window
    virtual void Resize();
    virtual void DataChanged (const DataChangedEvent& rEvent);

    void onSelectionChanged();
    void onChangeCurrentPage();

private:
    void updateLayout();
    void updateControls();
    void updateControlState();

    void updateSoundList();
    void openSoundFileDialog();

    impl::TransitionEffect getTransitionEffectFromControls() const;

    void applyToSelectedPages();
    void playCurrentEffect();

    void addListener();
    void removeListener();

    ::sd::slidesorter::SharedPageSelection getSelectedPages (void) const;

    void UpdateLook (void);

    DECL_LINK( ApplyToAllButtonClicked, void * );
    DECL_LINK( PlayButtonClicked, void * );
    DECL_LINK( SlideShowButtonClicked, void * );
    DECL_LINK( AutoPreviewClicked, void * );

    DECL_LINK( TransitionSelected, void * );
    DECL_LINK( AdvanceSlideRadioButtonToggled, void * );
    DECL_LINK( AdvanceTimeModified, void * );
    DECL_LINK( SpeedListBoxSelected, void * );
    DECL_LINK( SoundListBoxSelected, void * );
    DECL_LINK( LoopSoundBoxChecked, void * );
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
    DECL_LINK(LateInitCallback, void *);

    ViewShellBase &   mrBase;
    SdDrawDocument *  mpDrawDoc;
    Size              maMinSize;

    ListBox*      mpLB_SLIDE_TRANSITIONS;
    FixedText*    mpFT_SPEED;
    ListBox*      mpLB_SPEED;
    FixedText*    mpFT_SOUND;
    ListBox*      mpLB_SOUND;
    CheckBox*     mpCB_LOOP_SOUND;
    RadioButton*  mpRB_ADVANCE_ON_MOUSE;
    RadioButton*  mpRB_ADVANCE_AUTO;
    MetricField*  mpMF_ADVANCE_AUTO_AFTER;
    PushButton*   mpPB_APPLY_TO_ALL;
    PushButton*   mpPB_PLAY;
    PushButton*   mpPB_SLIDE_SHOW;
    CheckBox*     mpCB_AUTO_PREVIEW;

    OUString     maSTR_NO_TRANSITION;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView >             mxView;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                  mxModel;

    bool         mbHasSelection;
    bool         mbUpdatingControls;
    bool         mbIsMainViewChangePending;

    typedef ::std::vector< OUString > tSoundListType;
    tSoundListType  maSoundList;
    mutable OUString maCurrentSoundFile;

    typedef ::std::map< sal_uInt16, sal_uInt16 > tPresetIndexesType;
    tPresetIndexesType m_aPresetIndexes;

    Timer maLateInitTimer;
};

} //  namespace sd

// SD_SLIDETRANSITIONPANE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
