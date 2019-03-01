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

#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/ParallelTimeContainer.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/dispatch.hxx>
#include "STLPropertySet.hxx"
#include "CustomAnimationPane.hxx"
#include "CustomAnimationDialog.hxx"
#include "CustomAnimationList.hxx"
#include "motionpathtag.hxx"
#include <CustomAnimationPreset.hxx>
#include <createcustomanimationpanel.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#include <vcl/button.hxx>

#include <comphelper/sequence.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sidebar/Theme.hxx>

#include <svx/unoapi.hxx>
#include <svx/svxids.hrc>
#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include <DrawViewShell.hxx>
#include <DrawController.hxx>
#include <sdresid.hxx>
#include <drawview.hxx>
#include <slideshow.hxx>
#include <undoanim.hxx>
#include <optsitem.hxx>
#include <sdmod.hxx>
#include <framework/FrameworkHelper.hxx>

#include <EventMultiplexer.hxx>

#include <strings.hrc>
#include <sdpage.hxx>
#include <app.hrc>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <algorithm>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::text;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using ::com::sun::star::view::XSelectionSupplier;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::text::XText;
using ::sd::framework::FrameworkHelper;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;

namespace sd {

void fillRepeatComboBox(weld::ComboBox& rBox)
{
    OUString aNone( SdResId( STR_CUSTOMANIMATION_REPEAT_NONE ) );
    rBox.append_text(aNone);
    rBox.append_text(OUString::number(2));
    rBox.append_text(OUString::number(3));
    rBox.append_text(OUString::number(4));
    rBox.append_text(OUString::number(5));
    rBox.append_text(OUString::number(10));

    OUString aUntilClick( SdResId( STR_CUSTOMANIMATION_REPEAT_UNTIL_NEXT_CLICK ) );
    rBox.append_text(aUntilClick);

    OUString aEndOfSlide( SdResId( STR_CUSTOMANIMATION_REPEAT_UNTIL_END_OF_SLIDE ) );
    rBox.append_text(aEndOfSlide);
}

CustomAnimationPane::CustomAnimationPane( Window* pParent, ViewShellBase& rBase,
                                          const css::uno::Reference<css::frame::XFrame>& rxFrame )
:   PanelLayout( pParent, "CustomAnimationsPanel", "modules/simpress/ui/customanimationspanel.ui", rxFrame ),
    mrBase( rBase ),
    mpCustomAnimationPresets(nullptr),
    mnPropertyType( nPropertyTypeNone ),
    mnCurvePathPos( LISTBOX_ENTRY_NOTFOUND ),
    mnPolygonPathPos( LISTBOX_ENTRY_NOTFOUND ),
    mnFreeformPathPos( LISTBOX_ENTRY_NOTFOUND ),
    mbHorizontal( false ),
    maLateInitTimer()
{
    initialize();
}

CustomAnimationPane::CustomAnimationPane( Window* pParent, ViewShellBase& rBase,
                                          const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                          bool )
:   PanelLayout( pParent, "CustomAnimationsPanel", "modules/simpress/ui/customanimationspanelhorizontal.ui", rxFrame ),
    mrBase( rBase ),
    mpCustomAnimationPresets(nullptr),
    mnPropertyType( nPropertyTypeNone ),
    mnCurvePathPos( LISTBOX_ENTRY_NOTFOUND ),
    mnPolygonPathPos( LISTBOX_ENTRY_NOTFOUND ),
    mnFreeformPathPos( LISTBOX_ENTRY_NOTFOUND ),
    mbHorizontal( true ),
    maLateInitTimer()
{
    initialize();
}

void CustomAnimationPane::initialize()
{
    // load resources
    get(mpPBAddEffect, "add_effect");
    get(mpPBRemoveEffect, "remove_effect");

    if(!mbHorizontal)
      get(mpFTEffect, "effect_label");

    get(mpFTStart, "start_effect");
    get(mpLBStart, "start_effect_list");
    get(mpFTProperty, "effect_property");
    get(mpPlaceholderBox, "placeholder");
    get(mpLBProperty, "effect_property_list");
    get(mpPBPropertyMore, "more_properties");

    get(mpFTDuration, "effect_duration");
    get(mpCBXDuration, "anim_duration");
    get(mpFTCategory, "categorylabel");
    get(mpLBCategory, "categorylb");
    get(mpFTAnimation, "effectlabel");
    get(mpLBAnimation, "effect_list");
    get(mpFTStartDelay, "delay_label");
    get(mpMFStartDelay, "delay_value");

    mpLBAnimation->SetSelectHdl(LINK(this, CustomAnimationPane, AnimationSelectHdl));
    get(mpCustomAnimationList, "custom_animation_list");
    mpCustomAnimationList->setController( dynamic_cast<ICustomAnimationListController*> ( this ) );
    mpCustomAnimationList->set_width_request(mpCustomAnimationList->approximate_char_width() * 16);
    mpCustomAnimationList->set_height_request(mpCustomAnimationList->GetTextHeight() * 8);

    mpLBAnimation->set_width_request(mpLBAnimation->approximate_char_width() * 16);
    mpLBAnimation->set_height_request(mpLBAnimation->GetTextHeight() * 8);

    get(mpPBMoveUp, "move_up");
    get(mpPBMoveDown, "move_down");
    get(mpPBPlay, "play");
    get(mpCBAutoPreview,"auto_preview");

    maStrProperty = mpFTProperty->GetText();

    //fillDurationMetricComboBox
    mpCBXDuration->InsertValue(50, FieldUnit::CUSTOM);
    mpCBXDuration->InsertValue(100, FieldUnit::CUSTOM);
    mpCBXDuration->InsertValue(200, FieldUnit::CUSTOM);
    mpCBXDuration->InsertValue(300, FieldUnit::CUSTOM);
    mpCBXDuration->InsertValue(500, FieldUnit::CUSTOM);
    mpCBXDuration->AdaptDropDownLineCountToMaximum();


    mpPBAddEffect->SetClickHdl( LINK( this, CustomAnimationPane, implClickHdl ) );
    mpPBRemoveEffect->SetClickHdl( LINK( this, CustomAnimationPane, implClickHdl ) );
    mpLBStart->SetSelectHdl( LINK( this, CustomAnimationPane, implControlListBoxHdl ) );
    mpCBXDuration->SetModifyHdl(LINK( this, CustomAnimationPane, DurationModifiedHdl));
    mpPBPropertyMore->SetClickHdl( LINK( this, CustomAnimationPane, implClickHdl ) );
    mpPBMoveUp->SetClickHdl( LINK( this, CustomAnimationPane, implClickHdl ) );
    mpPBMoveDown->SetClickHdl( LINK( this, CustomAnimationPane, implClickHdl ) );
    mpPBPlay->SetClickHdl( LINK( this, CustomAnimationPane, implClickHdl ) );
    mpCBAutoPreview->SetClickHdl( LINK( this, CustomAnimationPane, implClickHdl ) );
    mpLBCategory->SetSelectHdl( LINK(this, CustomAnimationPane, UpdateAnimationLB) );
    mpMFStartDelay->SetModifyHdl( LINK(this, CustomAnimationPane, DelayModifiedHdl) );
    mpMFStartDelay->SetLoseFocusHdl(LINK( this, CustomAnimationPane, DelayLoseFocusHdl));


    if(!mbHorizontal)
      maStrModify = mpFTEffect->GetText();

    // get current controller and initialize listeners
    try
    {
        mxView.set(mrBase.GetController(), UNO_QUERY);
        addListener();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationPane::CustomAnimationPane(), Exception caught!" );
    }

    // get current page and update custom animation list
    onChangeCurrentPage();

    // Wait a short time before the presets list is created.  This gives the
    // system time to paint the control.
    maLateInitTimer.SetTimeout(100);
    maLateInitTimer.SetInvokeHandler(LINK(this, CustomAnimationPane, lateInitCallback));
    maLateInitTimer.Start();
    UpdateLook();
}

CustomAnimationPane::~CustomAnimationPane()
{
    disposeOnce();
}

void CustomAnimationPane::dispose()
{
    maLateInitTimer.Stop();

    removeListener();

    MotionPathTagVector aTags;
    aTags.swap( maMotionPathTags );
    for (auto const& tag : aTags)
        tag->Dispose();

    mpPBAddEffect.clear();
    mpPBRemoveEffect.clear();
    if(!mbHorizontal)
      mpFTEffect.clear();
    mpFTStart.clear();
    mpLBStart.clear();
    mpFTProperty.clear();
    mpPlaceholderBox.clear();
    mpLBProperty.clear();
    mpPBPropertyMore.clear();
    mpFTDuration.clear();
    mpCBXDuration.clear();
    mpFTStartDelay.clear();
    mpMFStartDelay.clear();
    mpCustomAnimationList.clear();
    mpPBMoveUp.clear();
    mpPBMoveDown.clear();
    mpPBPlay.clear();
    mpCBAutoPreview.clear();
    mpFTCategory.clear();
    mpLBCategory.clear();
    mpFTAnimation.clear();
    mpLBAnimation.clear();

    PanelLayout::dispose();
}

void CustomAnimationPane::addUndo()
{
    SfxUndoManager* pManager = mrBase.GetDocShell()->GetUndoManager();
    if( pManager )
    {
        SdPage* pPage = SdPage::getImplementation( mxCurrentPage );
        if( pPage )
            pManager->AddUndoAction( std::make_unique<UndoAnimation>( mrBase.GetDocShell()->GetDoc(), pPage ) );
    }
}

void CustomAnimationPane::StateChanged( StateChangedType nStateChange )
{
    Control::StateChanged( nStateChange );

    if( nStateChange == StateChangedType::Visible )
        updateMotionPathTags();
}

void CustomAnimationPane::KeyInput( const KeyEvent& rKEvt )
{
    if( mpCustomAnimationList )
        mpCustomAnimationList->KeyInput( rKEvt );
}

void CustomAnimationPane::addListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,CustomAnimationPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener(aLink);
}

void CustomAnimationPane::removeListener()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,CustomAnimationPane,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );
}

IMPL_LINK(CustomAnimationPane,EventMultiplexerListener,
    tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::EditViewSelection:
            onSelectionChanged();
            break;

        case EventMultiplexerEventId::CurrentPageChanged:
            onChangeCurrentPage();
            break;

        case EventMultiplexerEventId::MainViewAdded:
            // At this moment the controller may not yet been set at model
            // or ViewShellBase.  Take it from the view shell passed with
            // the event.
            if (mrBase.GetMainViewShell() != nullptr)
            {
                if( mrBase.GetMainViewShell()->GetShellType() == ViewShell::ST_IMPRESS )
                {
                    mxView.set(mrBase.GetDrawController(), UNO_QUERY);
                    onSelectionChanged();
                    onChangeCurrentPage();
                    break;
                }
            }
            [[fallthrough]];
        case EventMultiplexerEventId::MainViewRemoved:
            mxView = nullptr;
            mxCurrentPage = nullptr;
            updateControls();
            break;

        case EventMultiplexerEventId::Disposing:
            mxView.clear();
            onSelectionChanged();
            onChangeCurrentPage();
            break;
        case EventMultiplexerEventId::EndTextEdit:
            if( mpMainSequence.get() && rEvent.mpUserData )
                mpCustomAnimationList->update( mpMainSequence );
            break;
        default: break;
    }
}

static sal_Int32 getPropertyType( const OUString& rProperty )
{
    if ( rProperty == "Direction" )
        return nPropertyTypeDirection;

    if ( rProperty == "Spokes" )
        return nPropertyTypeSpokes;

    if ( rProperty == "Zoom" )
        return nPropertyTypeZoom;

    if ( rProperty == "Accelerate" )
        return nPropertyTypeAccelerate;

    if ( rProperty == "Decelerate" )
        return nPropertyTypeDecelerate;

    if ( rProperty == "Color1" )
        return nPropertyTypeFirstColor;

    if ( rProperty == "Color2" )
        return nPropertyTypeSecondColor;

    if ( rProperty == "FillColor" )
        return nPropertyTypeFillColor;

    if ( rProperty == "ColorStyle" )
        return nPropertyTypeColorStyle;

    if ( rProperty == "AutoReverse" )
        return nPropertyTypeAutoReverse;

    if ( rProperty == "FontStyle" )
        return nPropertyTypeFont;

    if ( rProperty == "CharColor" )
        return nPropertyTypeCharColor;

    if ( rProperty == "CharHeight" )
        return nPropertyTypeCharHeight;

    if ( rProperty == "CharDecoration" )
        return nPropertyTypeCharDecoration;

    if ( rProperty == "LineColor" )
        return nPropertyTypeLineColor;

    if ( rProperty == "Rotate" )
        return nPropertyTypeRotate;

    if ( rProperty == "Transparency" )
        return nPropertyTypeTransparency;

    if ( rProperty == "Color" )
        return nPropertyTypeColor;

    if ( rProperty == "Scale" )
        return nPropertyTypeScale;

    return nPropertyTypeNone;
}

OUString getPropertyName( sal_Int32 nPropertyType )
{
    switch( nPropertyType )
    {
    case nPropertyTypeDirection:
        return SdResId(STR_CUSTOMANIMATION_DIRECTION_PROPERTY);

    case nPropertyTypeSpokes:
        return SdResId(STR_CUSTOMANIMATION_SPOKES_PROPERTY);

    case nPropertyTypeFirstColor:
        return SdResId(STR_CUSTOMANIMATION_FIRST_COLOR_PROPERTY);

    case nPropertyTypeSecondColor:
        return SdResId(STR_CUSTOMANIMATION_SECOND_COLOR_PROPERTY);

    case nPropertyTypeZoom:
        return SdResId(STR_CUSTOMANIMATION_ZOOM_PROPERTY);

    case nPropertyTypeFillColor:
        return SdResId(STR_CUSTOMANIMATION_FILL_COLOR_PROPERTY);

    case nPropertyTypeColorStyle:
        return SdResId(STR_CUSTOMANIMATION_STYLE_PROPERTY);

    case nPropertyTypeFont:
        return SdResId(STR_CUSTOMANIMATION_FONT_PROPERTY);

    case nPropertyTypeCharHeight:
        return SdResId(STR_CUSTOMANIMATION_SIZE_PROPERTY);

    case nPropertyTypeCharColor:
        return SdResId(STR_CUSTOMANIMATION_FONT_COLOR_PROPERTY);

    case nPropertyTypeCharHeightStyle:
        return SdResId(STR_CUSTOMANIMATION_FONT_SIZE_STYLE_PROPERTY);

    case nPropertyTypeCharDecoration:
        return SdResId(STR_CUSTOMANIMATION_FONT_STYLE_PROPERTY);

    case nPropertyTypeLineColor:
        return SdResId(STR_CUSTOMANIMATION_LINE_COLOR_PROPERTY);

    case nPropertyTypeRotate:
        return SdResId(STR_CUSTOMANIMATION_AMOUNT_PROPERTY);

    case nPropertyTypeColor:
        return SdResId(STR_CUSTOMANIMATION_COLOR_PROPERTY);

    case nPropertyTypeTransparency:
        return SdResId(STR_CUSTOMANIMATION_AMOUNT_PROPERTY);

    case nPropertyTypeScale:
        return SdResId(STR_CUSTOMANIMATION_SCALE_PROPERTY);
    }

    return OUString();
}

void CustomAnimationPane::updateControls()
{
    mpFTDuration->Enable( mxView.is() );
    mpCBXDuration->Enable( mxView.is() );
    mpCustomAnimationList->Enable( mxView.is() );
    mpPBPlay->Enable( mxView.is() );
    mpCBAutoPreview->Enable( mxView.is() );

    if( !mxView.is() )
    {
        mpPBAddEffect->Enable( false );
        mpPBRemoveEffect->Enable( false );
        mpFTStart->Enable( false );
        mpLBStart->Enable( false );
        mpPBPropertyMore->Enable( false );
        mpLBProperty->Enable( false );
        mpFTProperty->Enable( false );
        mpFTCategory->Disable();
        mpLBCategory->Disable();
        mpFTAnimation->Disable();
        mpLBAnimation->Disable();
        mpFTStartDelay->Disable();
        mpMFStartDelay->Disable();
        mpLBAnimation->Clear();
        mpCustomAnimationList->clear();
        return;
    }

    const int nSelectionCount = maListSelection.size();

    mpPBAddEffect->Enable( maViewSelection.hasValue() );
    mpPBRemoveEffect->Enable(nSelectionCount != 0);
    bool bIsSelected = (nSelectionCount == 1);

    if(bIsSelected)
    {
        mpFTAnimation->Enable();
        mpLBAnimation->Enable();
    }
    else
    {
        mpFTAnimation->Disable();
        mpLBAnimation->Disable();
        mpLBAnimation->Clear();
    }

    mpLBCategory->Enable(bIsSelected);
    mpFTCategory->Enable(bIsSelected);

    mpFTStart->Enable(nSelectionCount > 0);
    mpLBStart->Enable(nSelectionCount > 0);
    mpLBProperty->Enable(nSelectionCount > 0);
    mpPBPropertyMore->Enable(nSelectionCount > 0);
    mpFTStartDelay->Enable(nSelectionCount > 0);
    mpMFStartDelay->Enable(nSelectionCount > 0);

    mpFTProperty->SetText( maStrProperty );

    mnPropertyType = nPropertyTypeNone;

    if(bIsSelected)
    {
        CustomAnimationEffectPtr pEffect = maListSelection.front();

        OUString aUIName( getPresets().getUINameForPresetId( pEffect->getPresetId() ) );

        OUString aTemp( maStrModify );

        if( !mbHorizontal && !aUIName.isEmpty() )
        {
            aTemp += " " + aUIName;
            mpFTEffect->SetText( aTemp );
        }

        CustomAnimationPresetPtr pDescriptor = getPresets().getEffectDescriptor( pEffect->getPresetId() );
        if( pDescriptor.get() )
        {
            PropertySubControl* pSubControl = nullptr;

            Any aValue;

            std::vector<OUString> aProperties( pDescriptor->getProperties() );
            if( !aProperties.empty() )
            {
                mnPropertyType = getPropertyType( aProperties.front() );

                mpFTProperty->SetText( getPropertyName( mnPropertyType )  );

                aValue = getProperty1Value( mnPropertyType, pEffect );
            }

            if( aValue.hasValue() )
            {
                pSubControl = mpLBProperty->getSubControl();
                if( !pSubControl || (pSubControl->getControlType() != mnPropertyType) )
                {
                    auto pNewControl = PropertySubControl::create( mnPropertyType, mpPlaceholderBox, aValue, pEffect->getPresetId(), LINK( this, CustomAnimationPane, implPropertyHdl ) );
                    pSubControl = pNewControl.get();
                    mpLBProperty->setSubControl( std::move(pNewControl) );
                }
                else
                {
                    pSubControl->setValue( aValue, pEffect->getPresetId() );
                }
            }
            else
            {
                mpLBProperty->setSubControl( nullptr );
            }

            bool bEnable = (pSubControl != nullptr) && (pSubControl->getControl()->IsEnabled());
            mpLBProperty->Enable( bEnable );
            mpFTProperty->Enable( bEnable );
        }
        else
        {
            mpLBProperty->setSubControl( nullptr );
            mpFTProperty->Enable( false );
            mpLBProperty->Enable( false );
            mpPBPropertyMore->Enable( false );
            mpFTStartDelay->Enable( false );
            mpMFStartDelay->Enable( false );
        }
        sal_uInt32 nCategoryPos = LISTBOX_ENTRY_NOTFOUND;
        switch(pEffect->getPresetClass())
        {
            case EffectPresetClass::ENTRANCE: nCategoryPos = 0; break;
            case EffectPresetClass::EMPHASIS: nCategoryPos = 1; break;
            case EffectPresetClass::EXIT: nCategoryPos = 2; break;
            case EffectPresetClass::MOTIONPATH: nCategoryPos = 3; break;
            default:
                break;
        }
        switch(pEffect->getCommand())
        {
            case EffectCommands::TOGGLEPAUSE:
            case EffectCommands::STOP:
            case EffectCommands::PLAY:
                nCategoryPos = 4; break;
            default:
            break;
        }
        mpLBCategory->SelectEntryPos(nCategoryPos);
        fillAnimationLB( pEffect->hasText() );
        OUString rsPresetId = pEffect->getPresetId();
        sal_Int32 nAnimationPos = mpLBAnimation->GetEntryCount();
        while( nAnimationPos-- )
        {
            void* pEntryData = mpLBAnimation->GetEntryData( nAnimationPos );
            if( pEntryData )
            {
                CustomAnimationPresetPtr& pPtr = *static_cast< CustomAnimationPresetPtr* >(pEntryData);
                if( pPtr.get() && pPtr->getPresetId() == rsPresetId )
                {
                    mpLBAnimation->SelectEntryPos( nAnimationPos );
                    break;
                }
            }
        }

        sal_uInt16 nPos = 0xffff;

        sal_Int16 nNodeType = pEffect->getNodeType();
        switch( nNodeType )
        {
        case EffectNodeType::ON_CLICK:          nPos = 0; break;
        case EffectNodeType::WITH_PREVIOUS:     nPos = 1; break;
        case EffectNodeType::AFTER_PREVIOUS:    nPos = 2; break;
        }

        mpLBStart->SelectEntryPos( nPos );

        double fDuration = pEffect->getDuration();
        const bool bHasSpeed = fDuration > 0.001;

        mpFTDuration->Enable(bHasSpeed);
        mpCBXDuration->Enable(bHasSpeed);

        if( bHasSpeed )
        {
            mpCBXDuration->SetValue( fDuration*100.0 );
        }

        mpPBPropertyMore->Enable();

        mpFTStartDelay->Enable();
        mpMFStartDelay->Enable();
        double fBegin = pEffect->getBegin();
        mpMFStartDelay->SetValue(fBegin*10.0);
    }
    else
    {
        mpLBProperty->setSubControl( nullptr );
        mpFTProperty->Enable( false );
        mpLBProperty->Enable( false );
        mpFTStartDelay->Enable( false );
        mpMFStartDelay->Enable( false );
        mpPBPropertyMore->Enable( false );
        mpFTDuration->Enable(false);
        mpCBXDuration->Enable(false);
        mpCBXDuration->SetNoSelection();
        if(!mbHorizontal)
            mpFTEffect->SetText( maStrModify );
    }

    bool bEnableUp = true;
    bool bEnableDown = true;
    if( nSelectionCount == 0 )
    {
        bEnableUp = false;
        bEnableDown = false;
    }
    else
    {
        if( mpMainSequence->find( maListSelection.front() ) == mpMainSequence->getBegin() )
            bEnableUp = false;

        EffectSequence::iterator aIter( mpMainSequence->find( maListSelection.back() ) );
        if( aIter == mpMainSequence->getEnd() )
        {
            bEnableDown = false;
        }
        else
        {
            do
            {
                ++aIter;
            }
            while( (aIter != mpMainSequence->getEnd()) && !(mpCustomAnimationList->isExpanded(*aIter) ) );

            if( aIter == mpMainSequence->getEnd() )
                bEnableDown = false;
        }

        if( bEnableUp || bEnableDown )
        {
            MainSequenceRebuildGuard aGuard( mpMainSequence );

            EffectSequenceHelper* pSequence = nullptr;
            for( CustomAnimationEffectPtr& pEffect : maListSelection )
            {
                if( pEffect.get() )
                {
                    if( pSequence == nullptr )
                    {
                        pSequence = pEffect->getEffectSequence();
                    }
                    else
                    {
                        if( pSequence != pEffect->getEffectSequence() )
                        {
                            bEnableUp = false;
                            bEnableDown = false;
                            break;
                        }
                    }
                }
            }
        }
    }

    mpPBMoveUp->Enable(mxView.is() &&  bEnableUp);
    mpPBMoveDown->Enable(mxView.is() && bEnableDown);

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
    mpCBAutoPreview->Check( pOptions->IsPreviewChangedEffects() );

    updateMotionPathTags();
}

static bool updateMotionPathImpl( CustomAnimationPane& rPane, ::sd::View& rView,  EffectSequence::iterator aIter, const EffectSequence::iterator& aEnd, MotionPathTagVector& rOldTags, MotionPathTagVector& rNewTags )
{
    bool bChanges = false;
    while( aIter != aEnd )
    {
        CustomAnimationEffectPtr pEffect( *aIter++ );
        if( pEffect.get() && pEffect->getPresetClass() == css::presentation::EffectPresetClass::MOTIONPATH )
        {
            rtl::Reference< MotionPathTag > xMotionPathTag;
            // first try to find if there is already a tag for this
            auto aMIter = std::find_if(rOldTags.begin(), rOldTags.end(),
                [&pEffect](const rtl::Reference<MotionPathTag>& xTag) { return xTag->getEffect() == pEffect; });
            if (aMIter != rOldTags.end())
            {
                rtl::Reference< MotionPathTag > xTag( *aMIter );
                if( !xTag->isDisposed() )
                {
                    xMotionPathTag = xTag;
                    rOldTags.erase( aMIter );
                }
            }

            // if not found, create new one
            if( !xMotionPathTag.is() )
            {
                xMotionPathTag.set( new MotionPathTag( rPane, rView, pEffect ) );
                bChanges = true;
            }

            if( xMotionPathTag.is() )
                rNewTags.push_back( xMotionPathTag );
        }
    }

    return bChanges;
}

void CustomAnimationPane::updateMotionPathTags()
{
    bool bChanges = false;

    MotionPathTagVector aTags;
    aTags.swap( maMotionPathTags );

    ::sd::View* pView = nullptr;

    if( mxView.is() )
    {
        std::shared_ptr<ViewShell> xViewShell( mrBase.GetMainViewShell() );
        if( xViewShell.get() )
            pView = xViewShell->GetView();
    }

    if( IsVisible() && mpMainSequence.get() && pView )
    {
        bChanges = updateMotionPathImpl( *this, *pView, mpMainSequence->getBegin(), mpMainSequence->getEnd(), aTags, maMotionPathTags );

        auto rInteractiveSequenceVector = mpMainSequence->getInteractiveSequenceVector();
        for (InteractiveSequencePtr const& pIS : rInteractiveSequenceVector)
        {
            bChanges |= updateMotionPathImpl( *this, *pView, pIS->getBegin(), pIS->getEnd(), aTags, maMotionPathTags );
        }
    }

    if( !aTags.empty() )
    {
        bChanges = true;
        for( rtl::Reference< MotionPathTag >& xTag : aTags )
        {
            xTag->Dispose();
        }
    }

    if( bChanges && pView )
        pView->updateHandles();
}

void CustomAnimationPane::onSelectionChanged()
{
    if( maSelectionLock.isLocked() )
        return;

    ScopeLockGuard aGuard( maSelectionLock );

    if( mxView.is() ) try
    {
        Reference< XSelectionSupplier >  xSel( mxView, UNO_QUERY_THROW );
        maViewSelection = xSel->getSelection();
        mpCustomAnimationList->onSelectionChanged( maViewSelection );
        updateControls();
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationPane::onSelectionChanged(), Exception caught!" );
    }
}

void CustomAnimationPane::onDoubleClick()
{
    showOptions();
}

void CustomAnimationPane::onContextMenu(const OString &rIdent)
{
    if (rIdent == "onclick")
        onChangeStart( EffectNodeType::ON_CLICK );
    else if (rIdent == "withprev")
        onChangeStart( EffectNodeType::WITH_PREVIOUS  );
    else if (rIdent == "afterprev")
        onChangeStart( EffectNodeType::AFTER_PREVIOUS );
    else if (rIdent == "options")
        showOptions();
    else if (rIdent == "timing")
        showOptions("timing");
    else if (rIdent == "remove")
        onRemove();
    else if (rIdent == "create" && maViewSelection.hasValue())
        onAdd();
    updateControls();
}

void CustomAnimationPane::DataChanged (const DataChangedEvent&)
{
    UpdateLook();
}

void CustomAnimationPane::UpdateLook()
{
    if( mbHorizontal )
        return;

    Wallpaper aBackground (
        ::sfx2::sidebar::Theme::GetWallpaper(
            ::sfx2::sidebar::Theme::Paint_PanelBackground));
    SetBackground(aBackground);
    if (mpFTStart != nullptr)
        mpFTStart->SetBackground(aBackground);
    if (mpFTProperty != nullptr)
        mpFTProperty->SetBackground(aBackground);
    if (mpFTDuration != nullptr)
        mpFTDuration->SetBackground(aBackground);
}

static void addValue( const std::unique_ptr<STLPropertySet>& pSet, sal_Int32 nHandle, const Any& rValue )
{
    switch( pSet->getPropertyState( nHandle ) )
    {
    case STLPropertyState::Ambiguous:
        // value is already ambiguous, do nothing
        break;
    case STLPropertyState::Direct:
        // set to ambiguous if existing value is different
        if( rValue != pSet->getPropertyValue( nHandle ) )
            pSet->setPropertyState( nHandle, STLPropertyState::Ambiguous );
        break;
    case STLPropertyState::Default:
        // just set new value
        pSet->setPropertyValue( nHandle, rValue );
        break;
    }
}

static sal_Int32 calcMaxParaDepth( const Reference< XShape >& xTargetShape )
{
    sal_Int32 nMaxParaDepth = -1;

    if( xTargetShape.is() )
    {
        Reference< XEnumerationAccess > xText( xTargetShape, UNO_QUERY );
        if( xText.is() )
        {
            Reference< XPropertySet > xParaSet;

            Reference< XEnumeration > xEnumeration( xText->createEnumeration(), UNO_QUERY_THROW );
            while( xEnumeration->hasMoreElements() )
            {
                xEnumeration->nextElement() >>= xParaSet;
                if( xParaSet.is() )
                {
                    sal_Int32 nParaDepth = 0;
                    xParaSet->getPropertyValue( "NumberingLevel" ) >>= nParaDepth;

                    if( nParaDepth > nMaxParaDepth )
                        nMaxParaDepth = nParaDepth;
                }
            }
        }
    }

    return nMaxParaDepth + 1;
}

Any CustomAnimationPane::getProperty1Value( sal_Int32 nType, const CustomAnimationEffectPtr& pEffect )
{
    switch( nType )
    {
    case nPropertyTypeDirection:
    case nPropertyTypeSpokes:
    case nPropertyTypeZoom:
        return makeAny( pEffect->getPresetSubType() );

    case nPropertyTypeColor:
    case nPropertyTypeFillColor:
    case nPropertyTypeFirstColor:
    case nPropertyTypeSecondColor:
    case nPropertyTypeCharColor:
    case nPropertyTypeLineColor:
        {
            const sal_Int32 nIndex = (nPropertyTypeFirstColor == nType) ? 0 : 1;
            return pEffect->getColor( nIndex );
        }

    case nPropertyTypeFont:
        return pEffect->getProperty( AnimationNodeType::SET, "CharFontName" , EValue::To );

    case nPropertyTypeCharHeight:
        {
            const OUString aAttributeName( "CharHeight" );
            Any aValue( pEffect->getProperty( AnimationNodeType::SET, aAttributeName, EValue::To ) );
            if( !aValue.hasValue() )
                aValue = pEffect->getProperty( AnimationNodeType::ANIMATE, aAttributeName, EValue::To );
            return aValue;
        }

    case nPropertyTypeRotate:
        return pEffect->getTransformationProperty( AnimationTransformType::ROTATE, EValue::By);

    case nPropertyTypeTransparency:
        return pEffect->getProperty( AnimationNodeType::SET, "Opacity" , EValue::To );

    case nPropertyTypeScale:
        return pEffect->getTransformationProperty( AnimationTransformType::SCALE, EValue::By );

    case nPropertyTypeCharDecoration:
        {
            Sequence< Any > aValues(3);
            aValues[0] = pEffect->getProperty( AnimationNodeType::SET, "CharWeight" , EValue::To );
            aValues[1] = pEffect->getProperty( AnimationNodeType::SET, "CharPosture" , EValue::To );
            aValues[2] = pEffect->getProperty( AnimationNodeType::SET, "CharUnderline" , EValue::To );
            return makeAny( aValues );
        }
    }

    Any aAny;
    return aAny;
}

bool CustomAnimationPane::setProperty1Value( sal_Int32 nType, const CustomAnimationEffectPtr& pEffect, const Any& rValue )
{
    bool bEffectChanged = false;
    switch( nType )
    {
    case nPropertyTypeDirection:
    case nPropertyTypeSpokes:
    case nPropertyTypeZoom:
        {
            OUString aPresetSubType;
            rValue >>= aPresetSubType;
            if( aPresetSubType != pEffect->getPresetSubType() )
            {
                getPresets().changePresetSubType( pEffect, aPresetSubType );
                bEffectChanged = true;
            }
        }
        break;

    case nPropertyTypeFillColor:
    case nPropertyTypeColor:
    case nPropertyTypeFirstColor:
    case nPropertyTypeSecondColor:
    case nPropertyTypeCharColor:
    case nPropertyTypeLineColor:
        {
            const sal_Int32 nIndex = (nPropertyTypeFirstColor == nType) ? 0 : 1;
            Any aOldColor( pEffect->getColor( nIndex ) );
            if( aOldColor != rValue )
            {
                pEffect->setColor( nIndex, rValue );
                bEffectChanged = true;
            }
        }
        break;

    case nPropertyTypeFont:
        bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, "CharFontName" , EValue::To, rValue );
        break;

    case nPropertyTypeCharHeight:
        {
            const OUString aAttributeName( "CharHeight" );
            bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, aAttributeName, EValue::To, rValue );
            if( !bEffectChanged )
                bEffectChanged = pEffect->setProperty( AnimationNodeType::ANIMATE, aAttributeName, EValue::To, rValue );
        }
        break;
    case nPropertyTypeRotate:
        bEffectChanged = pEffect->setTransformationProperty( AnimationTransformType::ROTATE, EValue::By , rValue );
        break;

    case nPropertyTypeTransparency:
        bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, "Opacity" , EValue::To, rValue );
        break;

    case nPropertyTypeScale:
        bEffectChanged = pEffect->setTransformationProperty( AnimationTransformType::SCALE, EValue::By, rValue );
        break;

    case nPropertyTypeCharDecoration:
        {
            Sequence< Any > aValues(3);
            rValue >>= aValues;
            bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, "CharWeight" , EValue::To, aValues[0] );
            bEffectChanged |= pEffect->setProperty( AnimationNodeType::SET, "CharPosture" , EValue::To, aValues[1] );
            bEffectChanged |= pEffect->setProperty( AnimationNodeType::SET, "CharUnderline" , EValue::To, aValues[2] );
        }
        break;

    }

    return bEffectChanged;
}

static bool hasVisibleShape( const Reference< XShape >& xShape )
{
    try
    {
        const OUString sShapeType( xShape->getShapeType() );

        if( sShapeType == "com.sun.star.presentation.TitleTextShape" || sShapeType == "com.sun.star.presentation.OutlinerShape" ||
            sShapeType == "com.sun.star.presentation.SubtitleShape" || sShapeType == "com.sun.star.drawing.TextShape" )
        {
            Reference< XPropertySet > xSet( xShape, UNO_QUERY_THROW );

            FillStyle eFillStyle;
            xSet->getPropertyValue( "FillStyle" ) >>= eFillStyle;

            css::drawing::LineStyle eLineStyle;
            xSet->getPropertyValue( "LineStyle" ) >>= eLineStyle;

            return eFillStyle != FillStyle_NONE || eLineStyle != css::drawing::LineStyle_NONE;
        }
    }
    catch( Exception& )
    {
    }
    return true;
}

std::unique_ptr<STLPropertySet> CustomAnimationPane::createSelectionSet()
{
    std::unique_ptr<STLPropertySet> pSet = CustomAnimationDialog::createDefaultSet();

    pSet->setPropertyValue( nHandleCurrentPage, makeAny( mxCurrentPage ) );

    sal_Int32 nMaxParaDepth = 0;

    // get options from selected effects
    const CustomAnimationPresets& rPresets (getPresets());
    for( CustomAnimationEffectPtr& pEffect : maListSelection )
    {
        EffectSequenceHelper* pEffectSequence = pEffect->getEffectSequence();
        if( !pEffectSequence )
            pEffectSequence = mpMainSequence.get();

        if( pEffect->hasText() )
        {
            sal_Int32 n = calcMaxParaDepth(pEffect->getTargetShape());
            if( n > nMaxParaDepth )
                nMaxParaDepth = n;
        }

        addValue( pSet, nHandleHasAfterEffect, makeAny( pEffect->hasAfterEffect() ) );
        addValue( pSet, nHandleAfterEffectOnNextEffect, makeAny( pEffect->IsAfterEffectOnNext() ) );
        addValue( pSet, nHandleDimColor, pEffect->getDimColor() );
        addValue( pSet, nHandleIterateType, makeAny( pEffect->getIterateType() ) );

        // convert absolute time to percentage value
        // This calculation is done in float to avoid some rounding artifacts.
        float fIterateInterval = static_cast<float>(pEffect->getIterateInterval());
        if( pEffect->getDuration() )
            fIterateInterval = static_cast<float>(fIterateInterval / pEffect->getDuration() );
        fIterateInterval *= 100.0;
        addValue( pSet, nHandleIterateInterval, makeAny( static_cast<double>(fIterateInterval) ) );

        addValue( pSet, nHandleBegin, makeAny( pEffect->getBegin() ) );
        addValue( pSet, nHandleDuration, makeAny( pEffect->getDuration() ) );
        addValue( pSet, nHandleStart, makeAny( pEffect->getNodeType() ) );
        addValue( pSet, nHandleRepeat, pEffect->getRepeatCount() );
        addValue( pSet, nHandleEnd, pEffect->getEnd() );
        addValue( pSet, nHandleRewind, makeAny( pEffect->getFill() ) );

        addValue( pSet, nHandlePresetId, makeAny( pEffect->getPresetId() ) );

        addValue( pSet, nHandleHasText, makeAny( pEffect->hasText() ) );

        addValue( pSet, nHandleHasVisibleShape, Any( hasVisibleShape( pEffect->getTargetShape() ) ) );

        Any aSoundSource;
        if( pEffect->getAudio().is() )
        {
            aSoundSource = pEffect->getAudio()->getSource();
            addValue( pSet, nHandleSoundVolumne, makeAny( pEffect->getAudio()->getVolume() ) );
// todo     addValue( pSet, nHandleSoundEndAfterSlide, makeAny( pEffect->getAudio()->getEndAfterSlide() ) );
// this is now stored at the XCommand parameter sequence
        }
        else if( pEffect->getCommand() == EffectCommands::STOPAUDIO )
        {
            aSoundSource <<= true;
        }
        addValue( pSet, nHandleSoundURL, aSoundSource );

        sal_Int32 nGroupId = pEffect->getGroupId();
        CustomAnimationTextGroupPtr pTextGroup;
        if( nGroupId != -1 )
            pTextGroup = pEffectSequence->findGroup( nGroupId );

        addValue( pSet, nHandleTextGrouping, makeAny( pTextGroup.get() ? pTextGroup->getTextGrouping() : sal_Int32(-1) ) );
        addValue( pSet, nHandleAnimateForm, makeAny( pTextGroup.get() == nullptr || pTextGroup->getAnimateForm() ) );
        addValue( pSet, nHandleTextGroupingAuto, makeAny( pTextGroup.get() ? pTextGroup->getTextGroupingAuto() : -1.0 ) );
        addValue( pSet, nHandleTextReverse, makeAny( pTextGroup.get() && pTextGroup->getTextReverse() ) );

        if( pEffectSequence->getSequenceType() == EffectNodeType::INTERACTIVE_SEQUENCE  )
        {
            InteractiveSequence* pIS = static_cast< InteractiveSequence* >( pEffectSequence );
            addValue( pSet, nHandleTrigger, makeAny( pIS->getTriggerShape() ) );
        }

        CustomAnimationPresetPtr pDescriptor = rPresets.getEffectDescriptor( pEffect->getPresetId() );
        if( pDescriptor.get() )
        {
            sal_Int32 nType = nPropertyTypeNone;

            std::vector<OUString> aProperties( pDescriptor->getProperties() );
            if( !aProperties.empty() )
                nType = getPropertyType( aProperties.front() );

            if( nType != nPropertyTypeNone )
            {
                addValue( pSet, nHandleProperty1Type, makeAny( nType ) );
                addValue( pSet, nHandleProperty1Value, getProperty1Value( nType, pEffect ) );
            }

            if( pDescriptor->hasProperty( "Accelerate" ) )
            {
                addValue( pSet, nHandleAccelerate, makeAny( pEffect->getAcceleration() ) );
            }

            if( pDescriptor->hasProperty( "Decelerate" ) )
            {
                addValue( pSet, nHandleDecelerate, makeAny( pEffect->getDecelerate() ) );
            }

            if( pDescriptor->hasProperty( "AutoReverse" ) )
            {
                addValue( pSet, nHandleAutoReverse, makeAny( pEffect->getAutoReverse() ) );
            }
        }
    }

    addValue( pSet, nHandleMaxParaDepth, makeAny( nMaxParaDepth ) );

    return pSet;
}

void CustomAnimationPane::changeSelection( STLPropertySet const * pResultSet, STLPropertySet const * pOldSet )
{
    // change selected effect
    bool bChanged = false;

    MainSequenceRebuildGuard aGuard( mpMainSequence );

    for( CustomAnimationEffectPtr& pEffect : maListSelection )
    {
        DBG_ASSERT( pEffect->getEffectSequence(), "sd::CustomAnimationPane::changeSelection(), dead effect in selection!" );
        if( !pEffect->getEffectSequence() )
            continue;

        double fDuration = 0.0; // we might need this for iterate-interval
        if( pResultSet->getPropertyState( nHandleDuration ) == STLPropertyState::Direct )
        {
            pResultSet->getPropertyValue( nHandleDuration ) >>= fDuration;
        }
        else
        {
            fDuration = pEffect->getDuration();
        }

        if( pResultSet->getPropertyState( nHandleIterateType ) == STLPropertyState::Direct )
        {
            sal_Int16 nIterateType = 0;
            pResultSet->getPropertyValue( nHandleIterateType ) >>= nIterateType;
            if( pEffect->getIterateType() != nIterateType )
            {
                pEffect->setIterateType( nIterateType );
                bChanged = true;
            }
        }

        if( pEffect->getIterateType() )
        {
            if( pResultSet->getPropertyState( nHandleIterateInterval ) == STLPropertyState::Direct )
            {
                double fIterateInterval = 0.0;
                pResultSet->getPropertyValue( nHandleIterateInterval ) >>= fIterateInterval;
                if( pEffect->getIterateInterval() != fIterateInterval )
                {
                    const double f = fIterateInterval * pEffect->getDuration() / 100;
                    pEffect->setIterateInterval( f );
                    bChanged = true;
                }
            }
        }

        double fBegin = 0.0;

        if( pResultSet->getPropertyState( nHandleBegin ) == STLPropertyState::Direct )
            pResultSet->getPropertyValue( nHandleBegin ) >>= fBegin;
        else
            fBegin = pEffect->getBegin();

        if( pEffect->getBegin() != fBegin && pResultSet->getPropertyState( nHandleBegin ) == STLPropertyState::Direct)
        {
            pEffect->setBegin( fBegin );
            bChanged = true;
        }

        if( pResultSet->getPropertyState( nHandleDuration ) == STLPropertyState::Direct )
        {
            if( pEffect->getDuration() != fDuration )
            {
                pEffect->setDuration( fDuration );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleStart ) == STLPropertyState::Direct )
        {
            sal_Int16 nNodeType = 0;
            pResultSet->getPropertyValue( nHandleStart ) >>= nNodeType;
            if( pEffect->getNodeType() != nNodeType )
            {
                pEffect->setNodeType( nNodeType );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleRepeat ) == STLPropertyState::Direct )
        {
            Any aRepeatCount( pResultSet->getPropertyValue( nHandleRepeat ) );
            if( aRepeatCount != pEffect->getRepeatCount() )
            {
                pEffect->setRepeatCount( aRepeatCount );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleEnd ) == STLPropertyState::Direct )
        {
            Any aEndValue( pResultSet->getPropertyValue( nHandleEnd ) );
            if( pEffect->getEnd() != aEndValue )
            {
                pEffect->setEnd( aEndValue );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleRewind ) == STLPropertyState::Direct )
        {
            sal_Int16 nFill = 0;
            pResultSet->getPropertyValue( nHandleRewind ) >>= nFill;
            if( pEffect->getFill() != nFill )
            {
                pEffect->setFill( nFill );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleHasAfterEffect ) == STLPropertyState::Direct )
        {
            bool bHasAfterEffect = false;
            if( pResultSet->getPropertyValue( nHandleHasAfterEffect )  >>= bHasAfterEffect )
            {
                if( pEffect->hasAfterEffect() != bHasAfterEffect )
                {
                    pEffect->setHasAfterEffect( bHasAfterEffect );
                    bChanged = true;
                }
            }
        }

        if( pResultSet->getPropertyState( nHandleAfterEffectOnNextEffect ) == STLPropertyState::Direct )
        {
            bool bAfterEffectOnNextEffect = false;
            if(   (pResultSet->getPropertyValue( nHandleAfterEffectOnNextEffect ) >>= bAfterEffectOnNextEffect)
               && (pEffect->IsAfterEffectOnNext() != bAfterEffectOnNextEffect) )
            {
                pEffect->setAfterEffectOnNext( bAfterEffectOnNextEffect );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleDimColor ) == STLPropertyState::Direct )
        {
            Any aDimColor( pResultSet->getPropertyValue( nHandleDimColor ) );
            if( pEffect->getDimColor() != aDimColor )
            {
                pEffect->setDimColor( aDimColor );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleAccelerate ) == STLPropertyState::Direct )
        {
            double fAccelerate = 0.0;
            pResultSet->getPropertyValue( nHandleAccelerate ) >>= fAccelerate;
            if( pEffect->getAcceleration() != fAccelerate )
            {
                pEffect->setAcceleration( fAccelerate );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleDecelerate ) == STLPropertyState::Direct )
        {
            double fDecelerate = 0.0;
            pResultSet->getPropertyValue( nHandleDecelerate ) >>= fDecelerate;
            if( pEffect->getDecelerate() != fDecelerate )
            {
                pEffect->setDecelerate( fDecelerate );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleAutoReverse ) == STLPropertyState::Direct )
        {
            bool bAutoReverse = false;
            pResultSet->getPropertyValue( nHandleAutoReverse ) >>= bAutoReverse;
            if( pEffect->getAutoReverse() != bAutoReverse )
            {
                pEffect->setAutoReverse( bAutoReverse );
                bChanged = true;
            }
        }

        if( pResultSet->getPropertyState( nHandleProperty1Value ) == STLPropertyState::Direct )
        {
            sal_Int32 nType = 0;
            pOldSet->getPropertyValue( nHandleProperty1Type ) >>= nType;

            bChanged |= setProperty1Value( nType, pEffect, pResultSet->getPropertyValue( nHandleProperty1Value ) );
        }

        if( pResultSet->getPropertyState( nHandleSoundURL ) == STLPropertyState::Direct )
        {
            const Any aSoundSource( pResultSet->getPropertyValue( nHandleSoundURL ) );

            if( aSoundSource.getValueType() == ::cppu::UnoType<sal_Bool>::get() )
            {
                pEffect->setStopAudio();
                bChanged = true;
            }
            else
            {
                OUString aSoundURL;
                aSoundSource >>= aSoundURL;

                if( !aSoundURL.isEmpty() )
                {
                    if( !pEffect->getAudio().is() )
                    {
                        pEffect->createAudio( aSoundSource );
                        bChanged = true;
                    }
                    else
                    {
                        if( pEffect->getAudio()->getSource() != aSoundSource )
                        {
                            pEffect->getAudio()->setSource( aSoundSource );
                            bChanged = true;
                        }
                    }
                }
                else
                {
                    if( pEffect->getAudio().is() || pEffect->getStopAudio() )
                    {
                        pEffect->removeAudio();
                        bChanged = true;
                    }
                }
            }
        }

        if( pResultSet->getPropertyState( nHandleTrigger ) == STLPropertyState::Direct )
        {
            Reference< XShape > xTriggerShape;
            pResultSet->getPropertyValue( nHandleTrigger ) >>= xTriggerShape;
            bChanged |= mpMainSequence->setTrigger( pEffect, xTriggerShape );
        }
    }

    const bool bHasTextGrouping = pResultSet->getPropertyState( nHandleTextGrouping ) == STLPropertyState::Direct;
    const bool bHasAnimateForm = pResultSet->getPropertyState( nHandleAnimateForm ) == STLPropertyState::Direct;
    const bool bHasTextGroupingAuto = pResultSet->getPropertyState( nHandleTextGroupingAuto ) == STLPropertyState::Direct;
    const bool bHasTextReverse = pResultSet->getPropertyState( nHandleTextReverse ) == STLPropertyState::Direct;

    if( bHasTextGrouping || bHasAnimateForm || bHasTextGroupingAuto || bHasTextReverse )
    {
        // we need to do a second pass for text grouping options
        // since changing them can cause effects to be removed
        // or replaced, we do this after we applied all other options
        // above

        sal_Int32 nTextGrouping = 0;
        bool bAnimateForm = true, bTextReverse = false;
        double fTextGroupingAuto = -1.0;

        if( bHasTextGrouping )
            pResultSet->getPropertyValue(nHandleTextGrouping) >>= nTextGrouping;
        else
            pOldSet->getPropertyValue(nHandleTextGrouping) >>= nTextGrouping;

        if( bHasAnimateForm )
            pResultSet->getPropertyValue(nHandleAnimateForm) >>= bAnimateForm;
        else
            pOldSet->getPropertyValue(nHandleAnimateForm) >>= bAnimateForm;

        if( bHasTextGroupingAuto )
            pResultSet->getPropertyValue(nHandleTextGroupingAuto) >>= fTextGroupingAuto;
        else
            pOldSet->getPropertyValue(nHandleTextGroupingAuto) >>= fTextGroupingAuto;

        if( bHasTextReverse )
            pResultSet->getPropertyValue(nHandleTextReverse) >>= bTextReverse;
        else
            pOldSet->getPropertyValue(nHandleTextReverse) >>= bTextReverse;

        EffectSequence const aSelectedEffects( maListSelection );
        for( CustomAnimationEffectPtr const& pEffect : aSelectedEffects )
        {
            EffectSequenceHelper* pEffectSequence = pEffect->getEffectSequence();
            if( !pEffectSequence )
                pEffectSequence = mpMainSequence.get();

            sal_Int32 nGroupId = pEffect->getGroupId();
            CustomAnimationTextGroupPtr pTextGroup;
            if( nGroupId != -1 )
            {
                // use existing group
                pTextGroup = pEffectSequence->findGroup( nGroupId );
            }
            else
            {
                // somethings changed so we need a group now
                pTextGroup = pEffectSequence->createTextGroup( pEffect, nTextGrouping, fTextGroupingAuto, bAnimateForm, bTextReverse );
                bChanged = true;
            }

            //#i119988#
            /************************************************************************/
            /*
            Note, the setAnimateForm means set the animation from TextGroup to Object's Shape
            And on the UI in means "Animate attached shape" in "Effect Option" dialog
            The setTextGrouping means set animation to Object's Text,
            the nTextGrouping is Text Animation Type
            nTextGrouping = -1 is "As one Object", means no text animation.

            The previous call order first do the setTextGrouping and then do the setAnimateForm,
            that will cause such defect: in the setTextGrouping, the effect has been removed,
            but in setAnimateForm still need this effect, then a NULL pointer of that effect will
            be gotten, and cause crash.

            []bHasAnimateForm means the UI has changed, bAnimateForm is it value

            So if create a new textgroup animation, the following animation will never be run!
            Since the \A1\B0Animate attached shape\A1\B1 is default checked.
            And the bHasAnimateForm default is false, and if user uncheck it the value bAnimateForm will be false,
            it same as the TextGroup\A1\AFs default value, also could not be run setAnimateForm.
            if( bHasAnimateForm )
            {
            if( pTextGroup->getAnimateForm() != bAnimateForm )
            {
            pEffectSequence->setAnimateForm( pTextGroup, bAnimateForm );
            bChanged = true;
            }
            }

            In setTextGrouping, there are three case:
            1.  Create new text effects for empty TextGroup
            2.  Remove all text effects of TextGroup (nTextGrouping == -1)
            3.  Change all the text effects\A1\AF start type

            So here is the right logic:
            If set the animation from text to shape and remove text animation,
            should do setAnimateForm first, then do setTextGrouping.
            Other case,do setTextGrouping first, then do setAnimateForm.

            */
            /************************************************************************/

            bool    bDoSetAnimateFormFirst = false;
            bool    bNeedDoSetAnimateForm = false;

            if( bHasAnimateForm )
            {
                if( pTextGroup.get() && pTextGroup->getAnimateForm() != bAnimateForm )
                {
                    if( (pTextGroup->getTextGrouping() >= 0) && (nTextGrouping == -1 ) )
                    {
                        bDoSetAnimateFormFirst = true;
                    }
                    bNeedDoSetAnimateForm = true;
                }
            }

            if (bDoSetAnimateFormFirst)
            {
                pEffectSequence->setAnimateForm( pTextGroup, bAnimateForm );
                bChanged = true;
            }

            if( bHasTextGrouping )
            {
                if( pTextGroup.get() && pTextGroup->getTextGrouping() != nTextGrouping )
                {
                    pEffectSequence->setTextGrouping( pTextGroup, nTextGrouping );

                    // All the effects of the outline object is removed so we need to
                    // put it back. OTOH, the shape object that still has effects
                    // in the text group is fine.
                    if (nTextGrouping == -1 && pTextGroup->getEffects().empty())
                    {
                        pEffect->setTarget(makeAny(pEffect->getTargetShape()));
                        pEffect->setGroupId(-1);
                        mpMainSequence->append(pEffect);
                    }

                    bChanged = true;
                }
            }

            if (!bDoSetAnimateFormFirst && bNeedDoSetAnimateForm)
            {
                if( pTextGroup.get() )
                {
                    pEffectSequence->setAnimateForm( pTextGroup, bAnimateForm );
                    bChanged = true;
                }
            }

            if( bHasTextGroupingAuto )
            {
                if( pTextGroup.get() && pTextGroup->getTextGroupingAuto() != fTextGroupingAuto )
                {
                    pEffectSequence->setTextGroupingAuto( pTextGroup, fTextGroupingAuto );
                    bChanged = true;
                }
            }

            if( bHasTextReverse )
            {
                if( pTextGroup.get() && pTextGroup->getTextReverse() != bTextReverse )
                {
                    pEffectSequence->setTextReverse( pTextGroup, bTextReverse );
                    bChanged = true;
                }
            }
        }
    }

    if( bChanged )
    {
        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();
    }
}

void CustomAnimationPane::showOptions(const OString& rPage)
{
    std::unique_ptr<STLPropertySet> xSet = createSelectionSet();

    std::shared_ptr<CustomAnimationDialog> xDlg(new CustomAnimationDialog(GetFrameWeld(), std::move(xSet), rPage));

    weld::DialogController::runAsync(xDlg, [xDlg, this](sal_Int32 nResult){
        if (nResult )
        {
            addUndo();
            changeSelection(xDlg->getResultSet(), xDlg->getPropertySet());
            updateControls();
        }
    });
}

void CustomAnimationPane::onChangeCurrentPage()
{
    if( !mxView.is() )
        return;

    try
    {
        Reference< XDrawPage > xNewPage( mxView->getCurrentPage() );
        if( xNewPage != mxCurrentPage )
        {
            mxCurrentPage = xNewPage;
            SdPage* pPage = SdPage::getImplementation( mxCurrentPage );
            if( pPage )
            {
                mpMainSequence = pPage->getMainSequence();
                mpCustomAnimationList->update( mpMainSequence );
            }
            updateControls();
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationPane::onChangeCurrentPage(), exception caught!" );
    }
}

static bool getTextSelection( const Any& rSelection, Reference< XShape >& xShape, std::vector< sal_Int16 >& rParaList )
{
    Reference< XTextRange > xSelectedText;
    rSelection >>= xSelectedText;
    if( xSelectedText.is() ) try
    {
        xShape.set( xSelectedText->getText(), UNO_QUERY_THROW );

        Reference< XTextRangeCompare > xTextRangeCompare( xShape, UNO_QUERY_THROW );
        Reference< XEnumerationAccess > xParaEnumAccess( xShape, UNO_QUERY_THROW );
        Reference< XEnumeration > xParaEnum( xParaEnumAccess->createEnumeration(), UNO_QUERY_THROW );
        Reference< XTextRange > xRange;
        Reference< XTextRange > xStart( xSelectedText->getStart() );
        Reference< XTextRange > xEnd( xSelectedText->getEnd() );

        if( xTextRangeCompare->compareRegionEnds( xStart, xEnd ) < 0 )
        {
            Reference< XTextRange > xTemp( xStart );
            xStart = xEnd;
            xEnd = xTemp;
        }

        sal_Int16 nPara = 0;
        while( xParaEnum->hasMoreElements() )
        {
            xParaEnum->nextElement() >>= xRange;

            // break if start of selection is prior to end of current paragraph
            if( xRange.is() && (xTextRangeCompare->compareRegionEnds( xStart, xRange ) >= 0 ) )
                break;

            nPara++;
        }

        while( xRange.is() )
        {
            if( xRange.is() && !xRange->getString().isEmpty() )
                rParaList.push_back( nPara );

            // break if end of selection is before or at end of current paragraph
            if( xRange.is() && xTextRangeCompare->compareRegionEnds( xEnd, xRange ) >= 0 )
                break;

            nPara++;

            if( xParaEnum->hasMoreElements() )
                xParaEnum->nextElement() >>= xRange;
            else
                xRange.clear();
        }

        return true;
    }
    catch( Exception& )
    {
        OSL_FAIL( "sd::CustomAnimationPane::getTextSelection(), exception caught!" );
    }

    return false;
}

void CustomAnimationPane::onAdd()
{
    bool bHasText = true;

    // first create vector of targets for dialog preview
    std::vector< Any > aTargets;

    // gather shapes from the selection
    Reference< XSelectionSupplier >  xSel( mxView, UNO_QUERY_THROW );
    maViewSelection = xSel->getSelection();

    if( maViewSelection.getValueType() == cppu::UnoType<XShapes>::get())
    {
        Reference< XIndexAccess > xShapes;
        maViewSelection >>= xShapes;

        sal_Int32 nCount = xShapes->getCount();
        aTargets.reserve( nCount );
        for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            Any aTarget( xShapes->getByIndex( nIndex ) );
            aTargets.push_back( aTarget );
            if( bHasText )
            {
                Reference< XText > xText;
                aTarget >>= xText;
                if( !xText.is() || xText->getString().isEmpty() )
                    bHasText = false;
            }
        }
    }
    else if ( maViewSelection.getValueType() == cppu::UnoType<XShape>::get())
    {
        aTargets.push_back( maViewSelection );
        Reference< XText > xText;
        maViewSelection >>= xText;
        if( !xText.is() || xText->getString().isEmpty() )
            bHasText = false;
    }
    else if ( maViewSelection.getValueType() == cppu::UnoType<XTextCursor>::get())
    {
        Reference< XShape > xShape;
        std::vector< sal_Int16 > aParaList;
        if( getTextSelection( maViewSelection, xShape, aParaList ) )
        {
            ParagraphTarget aParaTarget;
            aParaTarget.Shape = xShape;

            for( const auto& rPara : aParaList )
            {
                aParaTarget.Paragraph = rPara;
                aTargets.push_back( makeAny( aParaTarget ) );
            }
        }
    }
    else
    {
        OSL_FAIL("sd::CustomAnimationPane::onAdd(), unknown view selection!" );
        return;
    }

    CustomAnimationPresetPtr pDescriptor;
    mpFTCategory->Enable();
    mpFTAnimation->Enable();

    bool bCategoryReset = false;

    if (!mpLBCategory->IsEnabled() ||
            mpLBCategory->GetSelectedEntryPos()  == LISTBOX_ENTRY_NOTFOUND)
    {
        mpLBCategory->Enable();
        mpLBCategory->SelectEntryPos(0);
        bCategoryReset = true;
    }

    if (bCategoryReset || !mpLBAnimation->IsEnabled() ||
            mpLBAnimation->GetSelectedEntryPos()  == LISTBOX_ENTRY_NOTFOUND)
    {
        mpLBAnimation->Enable();

        sal_uInt32 nFirstEffect = fillAnimationLB( bHasText );
        if(nFirstEffect == LISTBOX_ENTRY_NOTFOUND)
            return;

        mpLBAnimation->SelectEntryPos(nFirstEffect);
    }

    void* pEntryData = mpLBAnimation->GetSelectedEntryData();
    if( pEntryData )
        pDescriptor = *static_cast< CustomAnimationPresetPtr* >( pEntryData );

    const double fDuration = pDescriptor->getDuration();
    mpCBXDuration->SetValue( fDuration*100.0 );
    bool bHasSpeed = pDescriptor->getDuration() > 0.001;
    mpCBXDuration->Enable( bHasSpeed );
    mpFTDuration->Enable( bHasSpeed );

    if( pDescriptor.get() )
    {
        mpCustomAnimationList->SelectAll( false );

        // gather shapes from the selection
        bool bFirst = true;
        for( const auto& rTarget : aTargets )
        {
            CustomAnimationEffectPtr pCreated = mpMainSequence->append( pDescriptor, rTarget, fDuration );

            // if only one shape with text and no fill or outline is selected, animate only by first level paragraphs
            if( bHasText && (aTargets.size() == 1) )
            {
                Reference< XShape > xShape( rTarget, UNO_QUERY );
                if( xShape.is() && !hasVisibleShape( xShape ) )
                {
                    mpMainSequence->createTextGroup( pCreated, 1, -1.0, false, false );
                }
            }

            if( bFirst )
                bFirst = false;
            else
                pCreated->setNodeType( EffectNodeType::WITH_PREVIOUS );

            if( pCreated.get() )
                mpCustomAnimationList->select( pCreated );
        }
    }

    mrBase.GetDocShell()->SetModified();

    updateControls();

    SlideShow::Stop( mrBase );
}

void CustomAnimationPane::onRemove()
{
    if( maListSelection.empty() )
        return;

    addUndo();

    MainSequenceRebuildGuard aGuard( mpMainSequence );

    EffectSequence aList( maListSelection );

    for( CustomAnimationEffectPtr& pEffect : aList )
    {
        if( pEffect->getEffectSequence() )
            pEffect->getEffectSequence()->remove( pEffect );
    }

    maListSelection.clear();
    mrBase.GetDocShell()->SetModified();
}

void CustomAnimationPane::remove( CustomAnimationEffectPtr const & pEffect )
{
    if( pEffect->getEffectSequence() )
    {
        addUndo();
        pEffect->getEffectSequence()->remove( pEffect );
        mrBase.GetDocShell()->SetModified();
    }
}

void CustomAnimationPane::onChangeStart()
{
    if( mpLBStart->GetSelectedEntryCount() != 1 )
        return;

    sal_Int16 nNodeType;
    switch( mpLBStart->GetSelectedEntryPos() )
    {
    case 0: nNodeType = EffectNodeType::ON_CLICK; break;
    case 1: nNodeType = EffectNodeType::WITH_PREVIOUS; break;
    case 2: nNodeType = EffectNodeType::AFTER_PREVIOUS; break;
    default:
        return;
    }

    onChangeStart( nNodeType );
}

void CustomAnimationPane::onChangeStart( sal_Int16 nNodeType )
{
    addUndo();

    MainSequenceRebuildGuard aGuard( mpMainSequence );

    bool bNeedRebuild = false;

    for( CustomAnimationEffectPtr& pEffect : maListSelection )
    {
        if( pEffect->getNodeType() != nNodeType )
        {
            pEffect->setNodeType( nNodeType );
            bNeedRebuild = true;
        }
    }

    if( bNeedRebuild )
    {
        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();
    }
}

void CustomAnimationPane::onChangeSpeed()
{
    double fDuration = getDuration();

    if(fDuration < 0)
        return;
    else
    {
        addUndo();

        MainSequenceRebuildGuard aGuard( mpMainSequence );

        // change selected effect
        for( CustomAnimationEffectPtr& pEffect : maListSelection )
        {
            pEffect->setDuration( fDuration );
        }

        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();
    }
}

double CustomAnimationPane::getDuration()
{
    double fDuration = 0;

    if(!(mpCBXDuration->GetText()).isEmpty())
    {
        fDuration = static_cast<double>(mpCBXDuration->GetValue())/100.0;
    }
    return fDuration;
}

PathKind CustomAnimationPane::getCreatePathKind() const
{
    PathKind eKind = PathKind::NONE;

    if( ( mpLBAnimation->GetSelectedEntryCount() == 1 ) &&
        ( mpLBCategory->GetSelectedEntryPos() == gnMotionPathPos ) )
    {
        const sal_Int32 nPos = mpLBAnimation->GetSelectedEntryPos();
        if( nPos == mnCurvePathPos )
        {
            eKind = PathKind::CURVE;
        }
        else if( nPos == mnPolygonPathPos )
        {
            eKind = PathKind::POLYGON;
        }
        else if( nPos == mnFreeformPathPos )
        {
            eKind = PathKind::FREEFORM;
        }
    }

    return eKind;
}

void CustomAnimationPane::createPath( PathKind eKind, std::vector< Any >& rTargets, double fDuration)
{
    sal_uInt16 nSID = 0;

    switch( eKind )
    {
    case PathKind::CURVE:     nSID = SID_DRAW_BEZIER_NOFILL; break;
    case PathKind::POLYGON:   nSID = SID_DRAW_POLYGON_NOFILL; break;
    case PathKind::FREEFORM:  nSID = SID_DRAW_FREELINE_NOFILL; break;
    default: break;
    }

    if( !nSID )
        return;

    DrawViewShell* pViewShell = dynamic_cast< DrawViewShell* >(
        FrameworkHelper::Instance(mrBase)->GetViewShell(FrameworkHelper::msCenterPaneURL).get());

    if( pViewShell )
    {
        DrawView* pView = pViewShell->GetDrawView();
        if( pView )
            pView->UnmarkAllObj();

        std::vector< Any > aTargets( 1, Any( fDuration ) );
        aTargets.insert( aTargets.end(), rTargets.begin(), rTargets.end() );
        Sequence< Any > aTargetSequence( comphelper::containerToSequence( aTargets ) );
        const SfxUnoAnyItem aItem( SID_ADD_MOTION_PATH, Any( aTargetSequence ) );
        pViewShell->GetViewFrame()->GetDispatcher()->ExecuteList( nSID, SfxCallMode::ASYNCHRON, {&aItem} );
    }
}


/// this link is called when the property box is modified by the user
IMPL_LINK_NOARG(CustomAnimationPane, implPropertyHdl, LinkParamNone*, void)
{
    if( !mpLBProperty->getSubControl() )
        return;

    addUndo();

    MainSequenceRebuildGuard aGuard( mpMainSequence );

    const Any aValue( mpLBProperty->getSubControl()->getValue() );

    bool bNeedUpdate = false;

    // change selected effect
    for( CustomAnimationEffectPtr& pEffect : maListSelection )
    {
        if( setProperty1Value( mnPropertyType, pEffect, aValue ) )
            bNeedUpdate = true;
    }

    if( bNeedUpdate )
    {
        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();
    }

    onPreview( false );
}

IMPL_LINK_NOARG(CustomAnimationPane, DelayModifiedHdl, Edit&, void)
{
    addUndo();
}

IMPL_LINK_NOARG(CustomAnimationPane, DelayLoseFocusHdl, Control&, void)
{
    double fBegin = mpMFStartDelay->GetValue();

    //sequence rebuild only when the control loses focus
    MainSequenceRebuildGuard aGuard( mpMainSequence );

    // change selected effect
    for( CustomAnimationEffectPtr& pEffect : maListSelection )
    {
        pEffect->setBegin( fBegin/10.0 );
    }

    mpMainSequence->rebuild();
    updateControls();
    mrBase.GetDocShell()->SetModified();
}

IMPL_LINK_NOARG(CustomAnimationPane, AnimationSelectHdl, ListBox&, void)
{
    if( maListSelection.size() != 1 )
        return;

    CustomAnimationPresetPtr* pPreset = static_cast< CustomAnimationPresetPtr* >(mpLBAnimation->GetSelectedEntryData());
    PathKind ePathKind = getCreatePathKind();

    // tdf#99137, the selected entry may also be a subcategory title, so not an effect
    // just leave in this case
    if ( !pPreset && ( ePathKind == PathKind::NONE ) )
        return;

    if ( ePathKind != PathKind::NONE )
    {
        std::vector< Any > aTargets;
        MainSequenceRebuildGuard aGuard( mpMainSequence );

        for( CustomAnimationEffectPtr& pEffect : maListSelection )
        {
            aTargets.push_back( pEffect->getTarget() );

            EffectSequenceHelper* pEffectSequence = pEffect->getEffectSequence();
            if( !pEffectSequence )
                pEffectSequence = mpMainSequence.get();

            // delete the old animation, new one will be appended
            // by createPath and SID_ADD_MOTION_PATH therein
            pEffectSequence->remove( pEffect );
        }

        createPath( ePathKind, aTargets, 0.0 );
        updateMotionPathTags();
        return;
    }

    CustomAnimationPresetPtr pDescriptor(*pPreset);
    const double fDuration = (*pPreset)->getDuration();
    MainSequenceRebuildGuard aGuard( mpMainSequence );

    // get selected effect
    for( CustomAnimationEffectPtr& pEffect : maListSelection )
    {
        // Dispose the deprecated motion path tag. It will be rebuilt later.
        if (pEffect->getPresetClass() == css::presentation::EffectPresetClass::MOTIONPATH)
        {
            for (auto const& xTag: maMotionPathTags)
            {
                if(xTag->getEffect() == pEffect && !xTag->isDisposed())
                    xTag->Dispose();
            }
        }

        EffectSequenceHelper* pEffectSequence = pEffect->getEffectSequence();
        if( !pEffectSequence )
            pEffectSequence = mpMainSequence.get();

        pEffectSequence->replace( pEffect, pDescriptor, fDuration );
    }

    onPreview(false);
}

IMPL_LINK_NOARG(CustomAnimationPane, UpdateAnimationLB, ListBox&, void)
{
    //FIXME: first effect only? what if there is more?
    CustomAnimationEffectPtr pEffect = maListSelection.front();
    fillAnimationLB( pEffect->hasText() );
}

IMPL_LINK_NOARG(CustomAnimationPane, DurationModifiedHdl, Edit&, void)
{
    if(!(mpCBXDuration->GetText()).isEmpty() )
    {
        double duration_value = static_cast<double>(mpCBXDuration->GetValue());
        if(duration_value <= 0.0)
        {
            mpCBXDuration->SetValue(1);
        }
        onChangeSpeed();
    }
}

sal_uInt32 CustomAnimationPane::fillAnimationLB( bool bHasText )
{
    PresetCategoryList rCategoryList;
    sal_uInt16 nPosition = mpLBCategory->GetSelectedEntryPos();
    const CustomAnimationPresets& rPresets (getPresets());
    switch(nPosition)
    {
        case 0:rCategoryList = rPresets.getEntrancePresets();break;
        case 1:rCategoryList = rPresets.getEmphasisPresets();break;
        case 2:rCategoryList = rPresets.getExitPresets();break;
        case 3:rCategoryList = rPresets.getMotionPathsPresets();break;
        case 4:rCategoryList = rPresets.getMiscPresets();break;
    }

    sal_uInt32 nFirstEffect = LISTBOX_ENTRY_NOTFOUND;

    mpLBAnimation->Clear();

    if(nPosition == gnMotionPathPos)
    {
        OUString sMotionPathLabel( SdResId( STR_CUSTOMANIMATION_USERPATH ) );
        mpLBAnimation->InsertCategory( sMotionPathLabel );
        mnCurvePathPos = mpLBAnimation->InsertEntry( SvxResId(STR_ObjNameSingulCOMBLINE) );
        mnPolygonPathPos = mpLBAnimation->InsertEntry( SvxResId(STR_ObjNameSingulPOLY) );
        mnFreeformPathPos = mpLBAnimation->InsertEntry( SvxResId(STR_ObjNameSingulFREELINE) );
    }

    for (PresetCategoryPtr& pCategory : rCategoryList)
    {
        if( pCategory.get() )
        {
            mpLBAnimation->InsertCategory( pCategory->maLabel );

            std::vector< CustomAnimationPresetPtr > aSortedVector(pCategory->maEffects.size());
            std::copy( pCategory->maEffects.begin(), pCategory->maEffects.end(), aSortedVector.begin() );

            for( CustomAnimationPresetPtr& pDescriptor : aSortedVector )
            {
                // ( !isTextOnly || ( isTextOnly && bHasText ) ) <=> !isTextOnly || bHasText
                if( pDescriptor.get() && ( !pDescriptor->isTextOnly() || bHasText ) )
                {
                    sal_Int32 nPos = mpLBAnimation->InsertEntry( pDescriptor->getLabel() );
                    mpLBAnimation->SetEntryData(nPos, new CustomAnimationPresetPtr(pDescriptor));

                    if( nFirstEffect == LISTBOX_ENTRY_NOTFOUND )
                        nFirstEffect = nPos;
                }
            }
        }
    }
    return nFirstEffect;
}


IMPL_LINK( CustomAnimationPane, implClickHdl, Button*, pBtn, void )
{
    implControlHdl(pBtn);
}
IMPL_LINK( CustomAnimationPane, implControlListBoxHdl, ListBox&, rListBox, void )
{
    implControlHdl(&rListBox);
}

/// this link is called when one of the controls is modified
void CustomAnimationPane::implControlHdl(Control const * pControl )
{
    if( pControl == mpPBAddEffect )
        onAdd();
    else if( pControl == mpPBRemoveEffect )
        onRemove();
    else if( pControl == mpLBStart )
        onChangeStart();
    else if( pControl == mpPBPropertyMore )
        showOptions();
    else if( pControl == mpPBMoveUp )
        moveSelection( true );
    else if( pControl == mpPBMoveDown )
        moveSelection( false );
    else if( pControl == mpPBPlay )
        onPreview( true );
    else if( pControl == mpCBAutoPreview )
    {
        SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
        pOptions->SetPreviewChangedEffects( mpCBAutoPreview->IsChecked() );
    }
}

IMPL_LINK_NOARG(CustomAnimationPane, lateInitCallback, Timer *, void)
{
    // Call getPresets() to initiate the (expensive) construction of the
    // presets list.
    getPresets();

    // update selection and control states
    onSelectionChanged();
}

void CustomAnimationPane::moveSelection( bool bUp )
{
    if( maListSelection.empty() )
        return;

    EffectSequenceHelper* pSequence = maListSelection.front()->getEffectSequence();
    if( pSequence == nullptr )
        return;

    addUndo();

    bool bChanged = false;

    MainSequenceRebuildGuard aGuard( mpMainSequence );
    EffectSequence& rEffectSequence = pSequence->getSequence();

    if( bUp )
    {
        for( CustomAnimationEffectPtr& pEffect : maListSelection )
        {
            EffectSequence::iterator aUpEffectPos( pSequence->find( pEffect ) );
            // coverity[copy_paste_error : FALSE] - this is correct, checking if it exists
            if( aUpEffectPos != rEffectSequence.end() )
            {
                EffectSequence::iterator aInsertPos( rEffectSequence.erase( aUpEffectPos ) );

                if( aInsertPos != rEffectSequence.begin() )
                {
                    --aInsertPos;
                    while( (aInsertPos != rEffectSequence.begin()) && !mpCustomAnimationList->isExpanded(*aInsertPos))
                        --aInsertPos;

                    rEffectSequence.insert( aInsertPos, pEffect );
                }
                else
                {
                    rEffectSequence.push_front( pEffect );
                }
                bChanged = true;
            }
        }
    }
    else
    {
        EffectSequence::reverse_iterator aIter( maListSelection.rbegin() );
        const EffectSequence::reverse_iterator aEnd( maListSelection.rend() );

        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = *aIter++;

            EffectSequence::iterator aDownEffectPos( pSequence->find( pEffect ) );
            // coverity[copy_paste_error : FALSE] - this is correct, checking if it exists
            if( aDownEffectPos != rEffectSequence.end() )
            {
                EffectSequence::iterator aInsertPos( rEffectSequence.erase( aDownEffectPos ) );

                if( aInsertPos != rEffectSequence.end() )
                {
                    ++aInsertPos;
                    // Advance over rolled-up (un-expanded) items, unless we just moved it there.
                    while( (aInsertPos != rEffectSequence.end())
                        && !mpCustomAnimationList->isExpanded(*aInsertPos)
                        && (std::find(maListSelection.begin(), maListSelection.end(), *aInsertPos)
                                == maListSelection.end())
                    )
                        ++aInsertPos;

                    rEffectSequence.insert( aInsertPos, pEffect );
                }
                else
                {
                    rEffectSequence.push_back( pEffect );
                }
                bChanged = true;
            }
        }
    }

    if( bChanged )
    {
        mpMainSequence->rebuild();
        updateControls();
        mrBase.GetDocShell()->SetModified();
    }
}

void CustomAnimationPane::onPreview( bool bForcePreview )
{
    if( !bForcePreview && !mpCBAutoPreview->IsChecked() )
        return;

    if( maListSelection.empty() )
    {
        rtl::Reference< MotionPathTag > xMotionPathTag;
        auto aIter = std::find_if(maMotionPathTags.begin(), maMotionPathTags.end(),
            [](const MotionPathTagVector::value_type& rxMotionPathTag) { return rxMotionPathTag->isSelected(); });
        if (aIter != maMotionPathTags.end())
            xMotionPathTag = *aIter;

        if( xMotionPathTag.is() )
        {
            MainSequencePtr pSequence( new MainSequence() );
            pSequence->append( xMotionPathTag->getEffect()->clone() );
            preview( pSequence->getRootNode() );
        }
        else
        {
            Reference< XAnimationNodeSupplier > xNodeSupplier( mxCurrentPage, UNO_QUERY );
            if( !xNodeSupplier.is() )
                return;

            preview( xNodeSupplier->getAnimationNode() );
        }
    }
    else
    {
        MainSequencePtr pSequence( new MainSequence() );

        for( CustomAnimationEffectPtr& pEffect : maListSelection )
        {
            pSequence->append( pEffect->clone() );
        }

        preview( pSequence->getRootNode() );
    }
}

void CustomAnimationPane::preview( const Reference< XAnimationNode >& xAnimationNode )
{
    Reference< XParallelTimeContainer > xRoot = ParallelTimeContainer::create( ::comphelper::getProcessComponentContext() );
    Sequence< css::beans::NamedValue > aUserData
        { { "node-type", css::uno::makeAny(css::presentation::EffectNodeType::TIMING_ROOT) } };
    xRoot->setUserData( aUserData );
    xRoot->appendChild( xAnimationNode );

    SlideShow::StartPreview( mrBase, mxCurrentPage, xRoot );
}

const CustomAnimationPresets& CustomAnimationPane::getPresets()
{
    if (mpCustomAnimationPresets == nullptr)
        mpCustomAnimationPresets = &CustomAnimationPresets::getCustomAnimationPresets();
    return *mpCustomAnimationPresets;
}

// ICustomAnimationListController
void CustomAnimationPane::onSelect()
{
    maListSelection = mpCustomAnimationList->getSelection();
    updateControls();

    // mark shapes from selected effects
    if( maSelectionLock.isLocked() )
        return;

    ScopeLockGuard aGuard( maSelectionLock );
    DrawViewShell* pViewShell = dynamic_cast< DrawViewShell* >(
        FrameworkHelper::Instance(mrBase)->GetViewShell(FrameworkHelper::msCenterPaneURL).get());
    DrawView* pView = pViewShell ? pViewShell->GetDrawView() : nullptr;

    if( pView )
    {
        pView->UnmarkAllObj();
        for( const CustomAnimationEffectPtr& pEffect : maListSelection )
        {
            Reference< XShape > xShape( pEffect->getTargetShape() );
            SdrObject* pObj = GetSdrObjectFromXShape( xShape );
            if( pObj )
                pView->MarkObj(pObj, pView->GetSdrPageView());
        }
    }
}

// ICustomAnimationListController
// pEffectInsertBefore may be null if moving to end of list.
void CustomAnimationPane::onDragNDropComplete(std::vector< CustomAnimationEffectPtr > pEffectsDragged, CustomAnimationEffectPtr pEffectInsertBefore)
{
    if ( !mpMainSequence.get() )
        return;

    addUndo();

    MainSequenceRebuildGuard aGuard( mpMainSequence );

    // Move all selected effects
    for( auto const& pEffectDragged : pEffectsDragged )
    {
        // Move this dragged effect and any hidden sub-effects
        EffectSequence::iterator aIter = mpMainSequence->find( pEffectDragged );
        const EffectSequence::iterator aEnd( mpMainSequence->getEnd() );

        while( aIter != aEnd )
        {
            CustomAnimationEffectPtr pEffect = *aIter++;

            // Update model with new location (function triggers a rebuild)
            // target may be null, which will insert at the end.
            mpMainSequence->moveToBeforeEffect( pEffect, pEffectInsertBefore );

            // Done moving effect and its hidden sub-effects when *next* effect is visible.
            if ( mpCustomAnimationList->isVisible( *aIter ) )
                break;
        }

    }


    updateControls();
    mrBase.GetDocShell()->SetModified();
}


void CustomAnimationPane::updatePathFromMotionPathTag( const rtl::Reference< MotionPathTag >& xTag )
{
    MainSequenceRebuildGuard aGuard( mpMainSequence );
    if( !xTag.is() )
        return;

    SdrPathObj* pPathObj = xTag->getPathObj();
    CustomAnimationEffectPtr pEffect = xTag->getEffect();
    if( (pPathObj != nullptr) && pEffect.get() != nullptr )
    {
        SfxUndoManager* pManager = mrBase.GetDocShell()->GetUndoManager();
        if( pManager )
        {
            SdPage* pPage = SdPage::getImplementation( mxCurrentPage );
            if( pPage )
                pManager->AddUndoAction( std::make_unique<UndoAnimationPath>( mrBase.GetDocShell()->GetDoc(), pPage, pEffect->getNode() ) );
        }

        pEffect->updatePathFromSdrPathObj( *pPathObj );
    }
}

vcl::Window * createCustomAnimationPanel( vcl::Window* pParent, ViewShellBase& rBase, const css::uno::Reference<css::frame::XFrame>& rxFrame )
{
    vcl::Window* pWindow = nullptr;

    DrawDocShell* pDocSh = rBase.GetDocShell();
    if( pDocSh )
    {
        pWindow = VclPtr<CustomAnimationPane>::Create( pParent, rBase, rxFrame );
    }

    return pWindow;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
