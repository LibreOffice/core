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
#include <sfx2/viewfrm.hxx>
#include <tools/debug.hxx>
#include "STLPropertySet.hxx"
#include <CustomAnimationPane.hxx>
#include "CustomAnimationDialog.hxx"
#include <CustomAnimationList.hxx>
#include "motionpathtag.hxx"
#include <CustomAnimationPreset.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/sequence.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <tools/diagnose_ex.h>

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
    : PanelLayout(pParent, "CustomAnimationsPanel", "modules/simpress/ui/customanimationspanel.ui", rxFrame)
    , mrBase(rBase)
    // load resources
    , mxFTAnimation(m_xBuilder->weld_label("effectlabel"))
    , mxCustomAnimationList(new CustomAnimationList(m_xBuilder->weld_tree_view("custom_animation_list"),
                                                    m_xBuilder->weld_label("custom_animation_label"),
                                                    m_xBuilder->weld_widget("custom_animation_label_parent")))
    , mxPBAddEffect(m_xBuilder->weld_button("add_effect"))
    , mxPBRemoveEffect(m_xBuilder->weld_button("remove_effect"))
    , mxPBMoveUp(m_xBuilder->weld_button("move_up"))
    , mxPBMoveDown(m_xBuilder->weld_button("move_down"))
    , mxFTCategory(m_xBuilder->weld_label("categorylabel"))
    , mxLBCategory(m_xBuilder->weld_combo_box("categorylb"))
    , mxFTEffect(m_xBuilder->weld_label("effect_label"))
    , mxLBAnimation(m_xBuilder->weld_tree_view("effect_list"))
    , mxFTStart(m_xBuilder->weld_label("start_effect"))
    , mxLBStart(m_xBuilder->weld_combo_box("start_effect_list"))
    , mxFTProperty(m_xBuilder->weld_label("effect_property"))
    , mxPlaceholderBox(m_xBuilder->weld_container("placeholder"))
    , mxPBPropertyMore(m_xBuilder->weld_button("more_properties"))
    , mxFTDuration(m_xBuilder->weld_label("effect_duration"))
    , mxCBXDuration(m_xBuilder->weld_metric_spin_button("anim_duration", FieldUnit::SECOND))
    , mxFTStartDelay(m_xBuilder->weld_label("delay_label"))
    , mxMFStartDelay(m_xBuilder->weld_metric_spin_button("delay_value", FieldUnit::SECOND))
    , mxCBAutoPreview(m_xBuilder->weld_check_button("auto_preview"))
    , mxPBPlay(m_xBuilder->weld_button("play"))
    , maIdle("sd idle treeview select")
    , mnLastSelectedAnimation(-1)
    , mnPropertyType(nPropertyTypeNone)
    , mnCurvePathPos(-1)
    , mnPolygonPathPos(-1)
    , mnFreeformPathPos(-1)
    , maLateInitTimer()
{
    initialize();
    m_pInitialFocusWidget = &mxCustomAnimationList->get_widget();
}

css::ui::LayoutSize CustomAnimationPane::GetHeightForWidth(const sal_Int32 /*nWidth*/)
{
    sal_Int32 nMinimumHeight = get_preferred_size().Height();
    return css::ui::LayoutSize(nMinimumHeight, -1, nMinimumHeight);
}

void CustomAnimationPane::initialize()
{
    mxLBAnimation->connect_changed(LINK(this, CustomAnimationPane, AnimationSelectHdl));
    mxCustomAnimationList->setController( static_cast<ICustomAnimationListController*> ( this ) );
    mxCustomAnimationList->set_size_request(mxCustomAnimationList->get_approximate_digit_width() * 15,
                                            mxCustomAnimationList->get_height_rows(8));

    mxLBAnimation->set_size_request(mxLBAnimation->get_approximate_digit_width() * 15,
                                    mxLBAnimation->get_height_rows(8));

    maStrProperty = mxFTProperty->get_label();

    mxPBAddEffect->connect_clicked( LINK( this, CustomAnimationPane, implClickHdl ) );
    mxPBRemoveEffect->connect_clicked( LINK( this, CustomAnimationPane, implClickHdl ) );
    mxLBStart->connect_changed( LINK( this, CustomAnimationPane, implControlListBoxHdl ) );
    mxCBXDuration->connect_value_changed(LINK( this, CustomAnimationPane, DurationModifiedHdl));
    mxPBPropertyMore->connect_clicked( LINK( this, CustomAnimationPane, implClickHdl ) );
    mxPBMoveUp->connect_clicked( LINK( this, CustomAnimationPane, implClickHdl ) );
    mxPBMoveDown->connect_clicked( LINK( this, CustomAnimationPane, implClickHdl ) );
    mxPBPlay->connect_clicked( LINK( this, CustomAnimationPane, implClickHdl ) );
    mxCBAutoPreview->connect_clicked( LINK( this, CustomAnimationPane, implClickHdl ) );
    mxLBCategory->connect_changed( LINK(this, CustomAnimationPane, UpdateAnimationLB) );
    mxMFStartDelay->connect_value_changed( LINK(this, CustomAnimationPane, DelayModifiedHdl) );
    mxMFStartDelay->connect_focus_out(LINK( this, CustomAnimationPane, DelayLoseFocusHdl));

    maIdle.SetPriority(TaskPriority::DEFAULT);
    maIdle.SetInvokeHandler(LINK(this, CustomAnimationPane, SelectionHandler));

    maStrModify = mxFTEffect->get_label();

    // get current controller and initialize listeners
    try
    {
        mxView.set(mrBase.GetController(), UNO_QUERY);
        addListener();
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPane::CustomAnimationPane()" );
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

    mxPBAddEffect.reset();
    mxPBRemoveEffect.reset();
    mxFTEffect.reset();
    mxFTStart.reset();
    mxLBStart.reset();
    mxLBSubControl.reset();
    mxFTProperty.reset();
    mxPlaceholderBox.reset();
    mxPBPropertyMore.reset();
    mxFTDuration.reset();
    mxCBXDuration.reset();
    mxFTStartDelay.reset();
    mxMFStartDelay.reset();
    mxCustomAnimationList.reset();
    mxPBMoveUp.reset();
    mxPBMoveDown.reset();
    mxPBPlay.reset();
    mxCBAutoPreview.reset();
    mxFTCategory.reset();
    mxLBCategory.reset();
    mxFTAnimation.reset();
    mxLBAnimation.reset();

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

void CustomAnimationPane::KeyInput( const KeyEvent& rKEvt )
{
    if (mxCustomAnimationList)
        mxCustomAnimationList->KeyInputHdl(rKEvt);
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
            if (mpMainSequence && rEvent.mpUserData)
                mxCustomAnimationList->update( mpMainSequence );
            break;
        default: break;
    }
}

static sal_Int32 getPropertyType( std::u16string_view rProperty )
{
    if ( rProperty == u"Direction" )
        return nPropertyTypeDirection;

    if ( rProperty == u"Spokes" )
        return nPropertyTypeSpokes;

    if ( rProperty == u"Zoom" )
        return nPropertyTypeZoom;

    if ( rProperty == u"Accelerate" )
        return nPropertyTypeAccelerate;

    if ( rProperty == u"Decelerate" )
        return nPropertyTypeDecelerate;

    if ( rProperty == u"Color1" )
        return nPropertyTypeFirstColor;

    if ( rProperty == u"Color2" )
        return nPropertyTypeSecondColor;

    if ( rProperty == u"FillColor" )
        return nPropertyTypeFillColor;

    if ( rProperty == u"ColorStyle" )
        return nPropertyTypeColorStyle;

    if ( rProperty == u"AutoReverse" )
        return nPropertyTypeAutoReverse;

    if ( rProperty == u"FontStyle" )
        return nPropertyTypeFont;

    if ( rProperty == u"CharColor" )
        return nPropertyTypeCharColor;

    if ( rProperty == u"CharHeight" )
        return nPropertyTypeCharHeight;

    if ( rProperty == u"CharDecoration" )
        return nPropertyTypeCharDecoration;

    if ( rProperty == u"LineColor" )
        return nPropertyTypeLineColor;

    if ( rProperty == u"Rotate" )
        return nPropertyTypeRotate;

    if ( rProperty == u"Transparency" )
        return nPropertyTypeTransparency;

    if ( rProperty == u"Color" )
        return nPropertyTypeColor;

    if ( rProperty == u"Scale" )
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
    mxFTDuration->set_sensitive(mxView.is());
    mxCBXDuration->set_sensitive(mxView.is());
    mxCustomAnimationList->set_sensitive(mxView.is());
    if (comphelper::LibreOfficeKit::isActive())
    {
        mxPBPlay->hide();
        mxCBAutoPreview->set_active(false);
        mxCBAutoPreview->hide();
    }
    else
    {
        mxPBPlay->set_sensitive(mxView.is());
        mxCBAutoPreview->set_sensitive(mxView.is());
    }

    if (!mxView.is())
    {
        mxPBAddEffect->set_sensitive(false);
        mxPBRemoveEffect->set_sensitive(false);
        mxFTStart->set_sensitive(false);
        mxLBStart->set_sensitive(false);
        mxPBPropertyMore->set_sensitive(false);
        mxPlaceholderBox->set_sensitive(false);
        mxFTProperty->set_sensitive(false);
        mxFTCategory->set_sensitive(false);
        mxLBCategory->set_sensitive(false);
        mxFTAnimation->set_sensitive(false);
        mxLBAnimation->set_sensitive(false);
        mxFTStartDelay->set_sensitive(false);
        mxMFStartDelay->set_sensitive(false);
        mxLBAnimation->clear();
        mnLastSelectedAnimation = -1;
        mxCustomAnimationList->clear();
        return;
    }

    const int nSelectionCount = maListSelection.size();

    mxPBAddEffect->set_sensitive( maViewSelection.hasValue() );
    mxPBRemoveEffect->set_sensitive(nSelectionCount != 0);
    bool bIsSelected = (nSelectionCount > 0);

    if(bIsSelected)
    {
        mxFTAnimation->set_sensitive(true);
        mxLBAnimation->set_sensitive(true);
    }
    else
    {
        mxFTAnimation->set_sensitive(false);
        mxLBAnimation->set_sensitive(false);
        mxLBAnimation->clear();
        mnLastSelectedAnimation = -1;
    }

    mxLBCategory->set_sensitive(bIsSelected);
    mxFTCategory->set_sensitive(bIsSelected);

    mxFTStart->set_sensitive(nSelectionCount > 0);
    mxLBStart->set_sensitive(nSelectionCount > 0);
    mxPlaceholderBox->set_sensitive(nSelectionCount > 0);
    mxPBPropertyMore->set_sensitive(nSelectionCount > 0);
    mxFTStartDelay->set_sensitive(nSelectionCount > 0);
    mxMFStartDelay->set_sensitive(nSelectionCount > 0);

    mxFTProperty->set_label(maStrProperty);

    sal_Int32 nOldPropertyType = mnPropertyType;

    mnPropertyType = nPropertyTypeNone;

    if(bIsSelected)
    {
        CustomAnimationEffectPtr pEffect = maListSelection.front();

        OUString aUIName( CustomAnimationPresets::getCustomAnimationPresets().getUINameForPresetId( pEffect->getPresetId() ) );

        OUString aTemp( maStrModify );

        if( !aUIName.isEmpty() )
        {
            aTemp += " " + aUIName;
            mxFTEffect->set_label( aTemp );
        }

        Any aValue;
        CustomAnimationPresetPtr pDescriptor = CustomAnimationPresets::getCustomAnimationPresets().getEffectDescriptor( pEffect->getPresetId() );
        if (pDescriptor)
        {
            std::vector<OUString> aProperties( pDescriptor->getProperties() );
            if( !aProperties.empty() )
            {
                mnPropertyType = getPropertyType( aProperties.front() );

                mxFTProperty->set_label( getPropertyName( mnPropertyType )  );

                aValue = getProperty1Value( mnPropertyType, pEffect );
            }
        }

        sal_Int32 nNewPropertyType = mnPropertyType;
        // if there is no value, then the control will be disabled, just show a disabled Direction box in that
        // case to have something to fill the space
        if (!aValue.hasValue())
            nNewPropertyType = nPropertyTypeDirection;

        if (!mxLBSubControl || nOldPropertyType != nNewPropertyType)
        {
            mxLBSubControl = SdPropertySubControl::create(nNewPropertyType, mxFTProperty.get(), mxPlaceholderBox.get(), GetFrameWeld(), aValue, pEffect->getPresetId(), LINK(this, CustomAnimationPane, implPropertyHdl));
        }
        else
        {
            mxLBSubControl->setValue(aValue, pEffect->getPresetId());
        }

        bool bEnable = aValue.hasValue();
        mxPlaceholderBox->set_sensitive( bEnable );
        mxFTProperty->set_sensitive( bEnable );

        if (!pDescriptor)
        {
            mxPBPropertyMore->set_sensitive( false );
            mxFTStartDelay->set_sensitive( false );
            mxMFStartDelay->set_sensitive( false );
        }
        sal_Int32 nCategoryPos = -1;
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
        mxLBCategory->set_active(nCategoryPos);

        fillAnimationLB( pEffect->hasText() );

        OUString rsPresetId = pEffect->getPresetId();
        sal_Int32 nAnimationPos = mxLBAnimation->n_children();
        while( nAnimationPos-- )
        {
            auto nEntryData = mxLBAnimation->get_id(nAnimationPos).toInt64();
            if (nEntryData)
            {
                CustomAnimationPresetPtr& pPtr = *reinterpret_cast<CustomAnimationPresetPtr*>(nEntryData);
                if( pPtr && pPtr->getPresetId() == rsPresetId )
                {
                    mxLBAnimation->select( nAnimationPos );
                    mnLastSelectedAnimation = nAnimationPos;
                    break;
                }
            }
        }

        // If preset id is missing and category is motion path.
        if (nAnimationPos < 0 && nCategoryPos == 3)
        {
            if (rsPresetId == "libo-motionpath-curve")
            {
                mxLBAnimation->select(mnCurvePathPos);
                mnLastSelectedAnimation = mnCurvePathPos;
            }
            else if (rsPresetId == "libo-motionpath-polygon")
            {
                mxLBAnimation->select(mnPolygonPathPos);
                mnLastSelectedAnimation = mnPolygonPathPos;
            }
            else if (rsPresetId == "libo-motionpath-freeform-line")
            {
                mxLBAnimation->select(mnFreeformPathPos);
                mnLastSelectedAnimation = mnFreeformPathPos;
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

        mxLBStart->set_active( nPos );

        double fDuration = pEffect->getDuration();
        const bool bHasSpeed = fDuration > 0.001;

        mxFTDuration->set_sensitive(bHasSpeed);
        mxCBXDuration->set_sensitive(bHasSpeed);

        if( bHasSpeed )
        {
            mxCBXDuration->set_value(fDuration*100.0, FieldUnit::NONE);
        }

        mxPBPropertyMore->set_sensitive(true);

        mxFTStartDelay->set_sensitive(true);
        mxMFStartDelay->set_sensitive(true);
        double fBegin = pEffect->getBegin();
        mxMFStartDelay->set_value(fBegin*10.0, FieldUnit::NONE);
    }
    else
    {
        // use an empty direction box to fill the space
        mxLBSubControl = SdPropertySubControl::create(nPropertyTypeDirection, mxFTProperty.get(), mxPlaceholderBox.get(), GetFrameWeld(), uno::Any(), OUString(), LINK(this, CustomAnimationPane, implPropertyHdl));
        mxPlaceholderBox->set_sensitive(false);
        mxFTProperty->set_sensitive(false);
        mxFTStartDelay->set_sensitive(false);
        mxMFStartDelay->set_sensitive(false);
        mxPBPropertyMore->set_sensitive(false);
        mxFTDuration->set_sensitive(false);
        mxCBXDuration->set_sensitive(false);
        mxCBXDuration->set_text(OUString());
        mxFTEffect->set_label(maStrModify);
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
            while( (aIter != mpMainSequence->getEnd()) && !(mxCustomAnimationList->isExpanded(*aIter) ) );

            if( aIter == mpMainSequence->getEnd() )
                bEnableDown = false;
        }

        if( bEnableUp || bEnableDown )
        {
            MainSequenceRebuildGuard aGuard( mpMainSequence );

            EffectSequenceHelper* pSequence = nullptr;
            for( const CustomAnimationEffectPtr& pEffect : maListSelection )
            {
                if( pEffect )
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

    mxPBMoveUp->set_sensitive(mxView.is() &&  bEnableUp);
    mxPBMoveDown->set_sensitive(mxView.is() && bEnableDown);

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
    mxCBAutoPreview->set_active(pOptions->IsPreviewChangedEffects());

    updateMotionPathTags();
}

static bool updateMotionPathImpl( CustomAnimationPane& rPane, ::sd::View& rView,  EffectSequence::iterator aIter, const EffectSequence::iterator& aEnd, MotionPathTagVector& rOldTags, MotionPathTagVector& rNewTags )
{
    bool bChanges = false;
    while( aIter != aEnd )
    {
        CustomAnimationEffectPtr pEffect( *aIter++ );
        if( pEffect && pEffect->getPresetClass() == css::presentation::EffectPresetClass::MOTIONPATH )
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
        if( xViewShell )
            pView = xViewShell->GetView();
    }

    if( IsVisible() && mpMainSequence && pView )
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
        mxCustomAnimationList->onSelectionChanged( maViewSelection );
        updateControls();
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPane::onSelectionChanged()" );
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
    Color aBackground (
        ::sfx2::sidebar::Theme::GetColor(
            ::sfx2::sidebar::Theme::Color_PanelBackground));
    SetBackground(aBackground);
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

            Reference< XEnumeration > xEnumeration( xText->createEnumeration(), UNO_SET_THROW );
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
        return pEffect->getProperty( AnimationNodeType::SET, u"CharFontName" , EValue::To );

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
        return pEffect->getProperty( AnimationNodeType::SET, u"Opacity" , EValue::To );

    case nPropertyTypeScale:
        return pEffect->getTransformationProperty( AnimationTransformType::SCALE, EValue::By );

    case nPropertyTypeCharDecoration:
        {
            Sequence< Any > aValues(3);
            aValues[0] = pEffect->getProperty( AnimationNodeType::SET, u"CharWeight" , EValue::To );
            aValues[1] = pEffect->getProperty( AnimationNodeType::SET, u"CharPosture" , EValue::To );
            aValues[2] = pEffect->getProperty( AnimationNodeType::SET, u"CharUnderline" , EValue::To );
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
                CustomAnimationPresets::getCustomAnimationPresets().changePresetSubType( pEffect, aPresetSubType );
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
        bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, u"CharFontName" , EValue::To, rValue );
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
        bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, u"Opacity" , EValue::To, rValue );
        break;

    case nPropertyTypeScale:
        bEffectChanged = pEffect->setTransformationProperty( AnimationTransformType::SCALE, EValue::By, rValue );
        break;

    case nPropertyTypeCharDecoration:
        {
            Sequence< Any > aValues(3);
            rValue >>= aValues;
            bEffectChanged = pEffect->setProperty( AnimationNodeType::SET, u"CharWeight" , EValue::To, aValues[0] );
            bEffectChanged |= pEffect->setProperty( AnimationNodeType::SET, u"CharPosture" , EValue::To, aValues[1] );
            bEffectChanged |= pEffect->setProperty( AnimationNodeType::SET, u"CharUnderline" , EValue::To, aValues[2] );
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
    const CustomAnimationPresets& rPresets (CustomAnimationPresets::getCustomAnimationPresets());
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
            addValue( pSet, nHandleSoundVolume, makeAny( pEffect->getAudio()->getVolume() ) );
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

        addValue( pSet, nHandleTextGrouping, makeAny( pTextGroup ? pTextGroup->getTextGrouping() : sal_Int32(-1) ) );
        addValue( pSet, nHandleAnimateForm, makeAny( !pTextGroup || pTextGroup->getAnimateForm() ) );
        addValue( pSet, nHandleTextGroupingAuto, makeAny( pTextGroup ? pTextGroup->getTextGroupingAuto() : -1.0 ) );
        addValue( pSet, nHandleTextReverse, makeAny( pTextGroup && pTextGroup->getTextReverse() ) );

        if( pEffectSequence->getSequenceType() == EffectNodeType::INTERACTIVE_SEQUENCE  )
        {
            InteractiveSequence* pIS = static_cast< InteractiveSequence* >( pEffectSequence );
            addValue( pSet, nHandleTrigger, makeAny( pIS->getTriggerShape() ) );
        }

        CustomAnimationPresetPtr pDescriptor = rPresets.getEffectDescriptor( pEffect->getPresetId() );
        if( pDescriptor )
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

            if( pDescriptor->hasProperty( u"Accelerate" ) )
            {
                addValue( pSet, nHandleAccelerate, makeAny( pEffect->getAcceleration() ) );
            }

            if( pDescriptor->hasProperty( u"Decelerate" ) )
            {
                addValue( pSet, nHandleDecelerate, makeAny( pEffect->getDecelerate() ) );
            }

            if( pDescriptor->hasProperty( u"AutoReverse" ) )
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
                if( pTextGroup && pTextGroup->getAnimateForm() != bAnimateForm )
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
                if( pTextGroup && pTextGroup->getTextGrouping() != nTextGrouping )
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
                if( pTextGroup )
                {
                    pEffectSequence->setAnimateForm( pTextGroup, bAnimateForm );
                    bChanged = true;
                }
            }

            if( bHasTextGroupingAuto )
            {
                if( pTextGroup && pTextGroup->getTextGroupingAuto() != fTextGroupingAuto )
                {
                    pEffectSequence->setTextGroupingAuto( pTextGroup, fTextGroupingAuto );
                    bChanged = true;
                }
            }

            if( bHasTextReverse )
            {
                if( pTextGroup && pTextGroup->getTextReverse() != bTextReverse )
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

    auto xDlg = std::make_shared<CustomAnimationDialog>(GetFrameWeld(), std::move(xSet), rPage);

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
                mxCustomAnimationList->update( mpMainSequence );
            }
            updateControls();
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPane::onChangeCurrentPage()" );
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
        Reference< XEnumeration > xParaEnum( xParaEnumAccess->createEnumeration(), UNO_SET_THROW );
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
        TOOLS_WARN_EXCEPTION( "sd", "sd::CustomAnimationPane::getTextSelection()" );
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
    mxFTCategory->set_sensitive(true);
    mxFTAnimation->set_sensitive(true);

    bool bCategoryReset = false;

    if (!mxLBCategory->get_sensitive() || mxLBCategory->get_active() == -1)
    {
        mxLBCategory->set_sensitive(true);
        mxLBCategory->set_active(0);
        bCategoryReset = true;
    }

    if (bCategoryReset || !mxLBAnimation->get_sensitive() ||
        mxLBAnimation->get_selected_index() == -1)
    {
        mxLBAnimation->set_sensitive(true);

        sal_Int32 nFirstEffect = fillAnimationLB(bHasText);
        if (nFirstEffect == -1)
            return;

        mxLBAnimation->select(nFirstEffect);
        mnLastSelectedAnimation = nFirstEffect;
    }

    auto nEntryData = mxLBAnimation->get_selected_id().toInt64();
    if (nEntryData)
        pDescriptor = *reinterpret_cast<CustomAnimationPresetPtr*>(nEntryData);

    if( pDescriptor )
    {
        const double fDuration = pDescriptor->getDuration();
        mxCBXDuration->set_value(fDuration*100.0, FieldUnit::NONE);
        bool bHasSpeed = pDescriptor->getDuration() > 0.001;
        mxCBXDuration->set_sensitive( bHasSpeed );
        mxFTDuration->set_sensitive( bHasSpeed );

        mxCustomAnimationList->unselect_all();

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

            if( pCreated )
                mxCustomAnimationList->select( pCreated );
        }
    }

    PathKind ePathKind = getCreatePathKind();

    if (ePathKind != PathKind::NONE)
    {
        createPath( ePathKind, aTargets, 0.0 );
        updateMotionPathTags();
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
    sal_Int16 nNodeType;
    switch( mxLBStart->get_active() )
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

double CustomAnimationPane::getDuration() const
{
    double fDuration = 0;

    if (!mxCBXDuration->get_text().isEmpty())
        fDuration = mxCBXDuration->get_value(FieldUnit::NONE) / 100.0;

    return fDuration;
}

PathKind CustomAnimationPane::getCreatePathKind() const
{
    PathKind eKind = PathKind::NONE;

    if (mxLBAnimation->count_selected_rows() == 1 &&
        mxLBCategory->get_active() == gnMotionPathPos)
    {
        const sal_Int32 nPos = mxLBAnimation->get_selected_index();
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
    if (!mxLBSubControl)
        return;

    addUndo();

    MainSequenceRebuildGuard aGuard( mpMainSequence );

    const Any aValue(mxLBSubControl->getValue());

    bool bNeedUpdate = false;

    // change selected effect
    for( const CustomAnimationEffectPtr& pEffect : maListSelection )
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

IMPL_LINK_NOARG(CustomAnimationPane, DelayModifiedHdl, weld::MetricSpinButton&, void)
{
    addUndo();
}

IMPL_LINK_NOARG(CustomAnimationPane, DelayLoseFocusHdl, weld::Widget&, void)
{
    double fBegin = mxMFStartDelay->get_value(FieldUnit::NONE);

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

IMPL_LINK_NOARG(CustomAnimationPane, AnimationSelectHdl, weld::TreeView&, void)
{
    maIdle.Start();
}

IMPL_LINK_NOARG(CustomAnimationPane, SelectionHandler, Timer*, void)
{
    if (mxLBAnimation->has_grab()) // tdf#136474 try again later
    {
        maIdle.Start();
        return;
    }

    int nSelected = mxLBAnimation->get_selected_index();

    // tdf#99137, the selected entry may also be a subcategory title, so not an effect
    // just skip it and move to the next one in this case
    if (mxLBAnimation->get_text_emphasis(nSelected, 0))
    {
        if (nSelected == 0 || nSelected > mnLastSelectedAnimation)
            mxLBAnimation->select(++nSelected);
        else
            mxLBAnimation->select(--nSelected);
    }

    mnLastSelectedAnimation = nSelected;

    CustomAnimationPresetPtr* pPreset = reinterpret_cast<CustomAnimationPresetPtr*>(mxLBAnimation->get_id(nSelected).toInt64());
    PathKind ePathKind = getCreatePathKind();

    if ( ePathKind != PathKind::NONE )
    {
        std::vector< Any > aTargets;
        MainSequenceRebuildGuard aGuard( mpMainSequence );

        for( const CustomAnimationEffectPtr& pEffect : maListSelection )
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
    for( const CustomAnimationEffectPtr& pEffect : maListSelection )
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

IMPL_LINK_NOARG(CustomAnimationPane, UpdateAnimationLB, weld::ComboBox&, void)
{
    //FIXME: first effect only? what if there is more?
    CustomAnimationEffectPtr pEffect = maListSelection.front();
    fillAnimationLB( pEffect->hasText() );
}

IMPL_LINK_NOARG(CustomAnimationPane, DurationModifiedHdl, weld::MetricSpinButton&, void)
{
    if (!mxCBXDuration->get_text().isEmpty())
    {
        double duration_value = static_cast<double>(mxCBXDuration->get_value(FieldUnit::NONE));
        if(duration_value <= 0.0)
        {
            mxCBXDuration->set_value(1, FieldUnit::NONE);
        }
        onChangeSpeed();
    }
}

namespace
{
    void InsertCategory(weld::TreeView& rLBAnimation, const OUString& rMotionPathLabel)
    {
        int nRow = rLBAnimation.n_children();
        rLBAnimation.append_text(rMotionPathLabel);
        rLBAnimation.set_text_emphasis(nRow, true, 0);
        rLBAnimation.set_text_align(nRow, 0.5, 0);
    }
}

sal_Int32 CustomAnimationPane::fillAnimationLB( bool bHasText )
{
    PresetCategoryList rCategoryList;
    sal_uInt16 nPosition = mxLBCategory->get_active();
    const CustomAnimationPresets& rPresets (CustomAnimationPresets::getCustomAnimationPresets());
    switch(nPosition)
    {
        case 0:rCategoryList = rPresets.getEntrancePresets();break;
        case 1:rCategoryList = rPresets.getEmphasisPresets();break;
        case 2:rCategoryList = rPresets.getExitPresets();break;
        case 3:rCategoryList = rPresets.getMotionPathsPresets();break;
        case 4:rCategoryList = rPresets.getMiscPresets();break;
    }

    sal_Int32 nFirstEffect = -1;

    int nOldEntryCount = mxLBAnimation->n_children();
    int nOldScrollPos = mxLBAnimation->vadjustment_get_value();

    mxLBAnimation->freeze();
    mxLBAnimation->clear();
    mnLastSelectedAnimation = -1;

    if (nPosition == gnMotionPathPos)
    {
        OUString sMotionPathLabel( SdResId( STR_CUSTOMANIMATION_USERPATH ) );
        InsertCategory(*mxLBAnimation, sMotionPathLabel);
        mnCurvePathPos = mxLBAnimation->n_children();
        mxLBAnimation->append_text( SvxResId(STR_ObjNameSingulCOMBLINE) );
        mxLBAnimation->set_text_emphasis(mnCurvePathPos, false, 0);
        mnPolygonPathPos = mnCurvePathPos + 1;
        mxLBAnimation->append_text( SvxResId(STR_ObjNameSingulPOLY) );
        mxLBAnimation->set_text_emphasis(mnPolygonPathPos, false, 0);
        mnFreeformPathPos = mnPolygonPathPos + 1;
        mxLBAnimation->append_text( SvxResId(STR_ObjNameSingulFREELINE) );
        mxLBAnimation->set_text_emphasis(mnFreeformPathPos, false, 0);
    }

    for (PresetCategoryPtr& pCategory : rCategoryList)
    {
        if( pCategory )
        {
            InsertCategory(*mxLBAnimation, pCategory->maLabel);

            int nPos = mxLBAnimation->n_children();

            std::vector< CustomAnimationPresetPtr > aSortedVector =
                pCategory->maEffects;

            for( CustomAnimationPresetPtr& pDescriptor : aSortedVector )
            {
                // ( !isTextOnly || ( isTextOnly && bHasText ) ) <=> !isTextOnly || bHasText
                if( pDescriptor && ( !pDescriptor->isTextOnly() || bHasText ) )
                {
                    auto pCustomPtr = new CustomAnimationPresetPtr(pDescriptor);
                    OUString sId = OUString::number(reinterpret_cast<sal_Int64>(pCustomPtr));
                    mxLBAnimation->append(sId, pDescriptor->getLabel());
                    mxLBAnimation->set_text_emphasis(nPos, false, 0);

                    if (nFirstEffect == -1)
                        nFirstEffect = nPos;

                    ++nPos;
                }
            }
        }
    }

    mxLBAnimation->thaw();

    if (mxLBAnimation->n_children() == nOldEntryCount)
        mxLBAnimation->vadjustment_set_value(nOldScrollPos);

    return nFirstEffect;
}

IMPL_LINK(CustomAnimationPane, implClickHdl, weld::Button&, rBtn, void)
{
    implControlHdl(&rBtn);
}

IMPL_LINK( CustomAnimationPane, implControlListBoxHdl, weld::ComboBox&, rListBox, void )
{
    implControlHdl(&rListBox);
}

/// this link is called when one of the controls is modified
void CustomAnimationPane::implControlHdl(const weld::Widget* pControl)
{
    if (pControl == mxPBAddEffect.get())
        onAdd();
    else if (pControl == mxPBRemoveEffect.get())
        onRemove();
    else if (pControl == mxLBStart.get())
        onChangeStart();
    else if (pControl == mxPBPropertyMore.get())
        showOptions();
    else if (pControl == mxPBMoveUp.get())
        moveSelection( true );
    else if (pControl == mxPBMoveDown.get())
        moveSelection( false );
    else if (pControl == mxPBPlay.get())
        onPreview( true );
    else if (pControl == mxCBAutoPreview.get())
    {
        SdOptions* pOptions = SD_MOD()->GetSdOptions(DocumentType::Impress);
        pOptions->SetPreviewChangedEffects(mxCBAutoPreview->get_active());
    }
}

IMPL_LINK_NOARG(CustomAnimationPane, lateInitCallback, Timer *, void)
{
    // Call getPresets() to initiate the (expensive) construction of the
    // presets list.
    CustomAnimationPresets::getCustomAnimationPresets();

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
        for( const CustomAnimationEffectPtr& pEffect : maListSelection )
        {
            EffectSequence::iterator aUpEffectPos( pSequence->find( pEffect ) );
            // coverity[copy_paste_error : FALSE] - this is correct, checking if it exists
            if( aUpEffectPos != rEffectSequence.end() )
            {
                EffectSequence::iterator aInsertPos( rEffectSequence.erase( aUpEffectPos ) );

                if( aInsertPos != rEffectSequence.begin() )
                {
                    --aInsertPos;
                    while( (aInsertPos != rEffectSequence.begin()) && !mxCustomAnimationList->isExpanded(*aInsertPos))
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
                        && !mxCustomAnimationList->isExpanded(*aInsertPos)
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
    if (!bForcePreview && !mxCBAutoPreview->get_active())
        return;

    // No preview in LOK.
    if (comphelper::LibreOfficeKit::isActive())
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
            MainSequencePtr pSequence = std::make_shared<MainSequence>();
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
        MainSequencePtr pSequence = std::make_shared<MainSequence>();

        for( const CustomAnimationEffectPtr& pEffect : maListSelection )
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

// ICustomAnimationListController
void CustomAnimationPane::onSelect()
{
    maListSelection = mxCustomAnimationList->getSelection();
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
    if ( !mpMainSequence )
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
            if (aIter != aEnd && mxCustomAnimationList->isVisible(*aIter))
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
    if( (pPathObj != nullptr) && pEffect )
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
