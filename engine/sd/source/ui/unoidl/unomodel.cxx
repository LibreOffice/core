/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <chrono>
#include <memory>
#include <set>

#include <com/sun/star/presentation/XPresentation.hpp>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/XTheme.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/AnimationEndSync.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>


#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <cpo/uno/Sequence.hxx>

#include <animations/animationnodehelper.hxx>

#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Impress.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/indexedpropertyvalues.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/random.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/profilezone.hxx>
#include <comphelper/seqstream.hxx>
#include <avmedia/mediaitem.hxx>
#include <com/sun/star/io/XInputStream.hpp>

#include <sal/log.hxx>
#include <editeng/unofield.hxx>
#include <notifydocumentevent.hxx>
#include <tpaction.hxx>
#include <unomodel.hxx>
#include <sdhtmlfilter.hxx>
#include "unopool.hxx"
#include <sfx2/kit/helper.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/cryptosign.hxx>
#include <vcl/svapp.hxx>
#include <Outliner.hxx>
#include <COKit/COKit.hxx>

#include <editeng/UnoForbiddenCharsTable.hxx>
#include <svx/svdoutl.hxx>
#include <o3tl/any.hxx>
#include <o3tl/hash_combine.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/test_info.hxx>
#include <o3tl/unit_conversion.hxx>
#include <svx/UnoNamespaceMap.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoedge.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxenditm.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/compatflags.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/svditer.hxx>
#include <svx/seclabel/SecLabelStore.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdtext.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdhdl.hxx>
#include <svtools/colorcfg.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <svx/unofill.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <editeng/fontitem.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdedxv.hxx>
#include <svtools/unoimap.hxx>
#include <svx/unoshape.hxx>
#include <editeng/unonrule.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <PlaceholderDecoration.hxx>
#include <editeng/eeitem.hxx>
#include <unotools/datetime.hxx>
#include <tools/datetimeutils.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/autolayout.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/math.hxx>
#include <tools/helpers.hxx>
#include <iomanip>
#include <tools/json_writer.hxx>
#include <TransitionPreset.hxx>

// Support creation of GraphicStorageHandler and EmbeddedObjectResolver
#include <svx/xmleohlp.hxx>
#include <xmloff/xmlgrhlp.hxx>
#include <DesignTemplates.hxx>
#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include "UnoDocumentSettings.hxx"

#include <A11yObjectName.hxx>
#include <Annotation.hxx>
#include <CustomAnimationPreset.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <drawdoc.hxx>
#include <SlideSectionManager.hxx>
#include <UndoSlideSection.hxx>
#include <sdmod.hxx>
#include <sdresid.hxx>
#include <sdpage.hxx>

#include <strings.hrc>
#include <strings.hxx>
#include <unolayer.hxx>
#include <unopage.hxx>
#include "unocpres.hxx"
#include "unoobj.hxx"
#include <stlpool.hxx>
#include "unopback.hxx"
#include <unokywds.hxx>

#include <FrameView.hxx>
#include <ClientView.hxx>
#include <DrawViewShell.hxx>
#include <ViewShell.hxx>
#include <View.hxx>
#include <Window.hxx>
#include <optsitem.hxx>
#include <SlideLink.hxx>
#include <SlideshowLayerRenderer.hxx>

#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/pdf/PDFNote.hxx>

#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>
#include <com/sun/star/office/XAnnotationEnumeration.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <drawinglayer/primitive2d/structuretagprimitive2d.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/vector/b2enums.hxx>
#include <vcl/graph.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/GraphicAttributes.hxx>
#include <unordered_map>
#include <unordered_set>

#include <sfx2/kit/componenthelpers.hxx>
#include <sfx2/kit/ControlHandler.hxx>
#include <tools/gen.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/UnitConversion.hxx>
#include <svx/ColorSets.hxx>
#include <docmodel/theme/Theme.hxx>

#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>
#include <SlideSorter.hxx>
#include <SlideSorterViewShell.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>

#include <app.hrc>
#include <slideshow.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::sd;
using namespace ::cpo;
using namespace ::cpo::uno;

namespace
{
const TranslateId aTypeResIds[SdLinkTargetType::Count] =
{
    STR_SD_PAGE,            // SdLinkTargetType::Page
    STR_NOTES_MODE,         // SdLinkTargetType::Notes
    STR_HANDOUT,            // SdLinkTargetType::Handout
    STR_MASTERPAGE_NAME,    // SdLinkTargetType::MasterPage
};

const TranslateId aClickActionResIds[] =
{
    STR_CLICK_ACTION_NONE,             // ClickAction_NONE
    STR_CLICK_ACTION_PREVPAGE,         // ClickAction_PREVPAGE
    STR_CLICK_ACTION_NEXTPAGE,         // ClickAction_NEXTPAGE
    STR_CLICK_ACTION_FIRSTPAGE,        // ClickAction_FIRSTPAGE
    STR_CLICK_ACTION_LASTPAGE,         // ClickAction_LASTPAGE
    STR_CLICK_ACTION_BOOKMARK,         // ClickAction_BOOKMARK
    STR_CLICK_ACTION_DOCUMENT,         // ClickAction_DOCUMENT
    STR_CLICK_ACTION_INVISIBLE,        // ClickAction_INVISIBLE
    STR_CLICK_ACTION_SOUND,            // ClickAction_SOUND
    STR_CLICK_ACTION_VERB,             // ClickAction_VERB
    STR_CLICK_ACTION_VANISH,           // ClickAction_VANISH
    STR_CLICK_ACTION_PROGRAM,          // ClickAction_PROGRAM
    STR_CLICK_ACTION_MACRO,            // ClickAction_MACRO
    STR_CLICK_ACTION_STOPPRESENTATION, // ClickAction_STOPPRESENTATION
};

static_assert(SAL_N_ELEMENTS(aClickActionResIds)
              == static_cast<size_t>(presentation::ClickAction_STOPPRESENTATION) + 1,
              "aClickActionResIds is missing an entry for a presentation::ClickAction value");
}

TranslateId SdTPAction::GetClickActionSdResId( presentation::ClickAction eCA )
{
    auto nIndex = static_cast<size_t>(eCA);
    assert(nIndex < SAL_N_ELEMENTS(aClickActionResIds) && "ClickAction out of range");
    return aClickActionResIds[nIndex];
}

class SdUnoForbiddenCharsTable : public SvxUnoForbiddenCharsTable,
                                 public SfxListener
{
public:
    explicit SdUnoForbiddenCharsTable(SdrModel* pModel);
    virtual ~SdUnoForbiddenCharsTable() override;

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) noexcept override;
protected:
    virtual void onChange() override;

private:
    SdrModel*   mpModel;
};

namespace {

class SlideBackgroundInfo
{
public:
    SlideBackgroundInfo(const uno::Reference<drawing::XDrawPage>& xDrawPage,
                        const uno::Reference<drawing::XDrawPage>& xMasterPage);
    bool slideHasOwnBackground() const { return mbIsCustom; }
    bool hasBackground() const { return bHasBackground; }
    bool isSolidColor() const { return mbIsSolidColor; }
    ::Color getFillColor() const;
    sal_Int32 getFillTransparency() const;
    OString getFillColorAsRGBA() const;
private:
    bool getFillStyleImpl(const uno::Reference<drawing::XDrawPage>& xDrawPage);
private:
    uno::Reference<beans::XPropertySet> mxBackground;
    bool mbIsCustom;
    bool bHasBackground;
    bool mbIsSolidColor;
    drawing::FillStyle maFillStyle;
};

SlideBackgroundInfo::SlideBackgroundInfo(
        const uno::Reference<drawing::XDrawPage>& xDrawPage,
        const uno::Reference<drawing::XDrawPage>& xMasterPage)
    : mbIsCustom(false)
    , bHasBackground(false)
    , mbIsSolidColor(false)
    , maFillStyle(drawing::FillStyle_NONE)
{
    mbIsCustom = getFillStyleImpl(xDrawPage);
    bHasBackground = mbIsCustom;
    if (!bHasBackground)
    {
        bHasBackground = getFillStyleImpl(xMasterPage);
    }
    if (bHasBackground)
    {
        if (maFillStyle == drawing::FillStyle_SOLID)
        {
            OUString sGradientName;
            mxBackground->getPropertyValue(u"FillTransparenceGradientName"_ustr) >>= sGradientName;
            if (sGradientName.isEmpty())
            {
                mbIsSolidColor = true;
            }
        }
    }
}

sal_Int32 SlideBackgroundInfo::getFillTransparency() const
{
    if (!mxBackground.is())
        return 0;
    sal_Int32 nFillTransparency = 0;
    mxBackground->getPropertyValue(u"FillTransparence"_ustr) >>= nFillTransparency;
    return nFillTransparency;
}

::Color SlideBackgroundInfo::getFillColor() const
{
    if (!mxBackground.is())
        return {};
    if (sal_Int32 nFillColor; mxBackground->getPropertyValue(u"FillColor"_ustr) >>= nFillColor)
    {
        return ::Color(ColorTransparency, nFillColor & 0xffffff);
    }
    return {};
}

OString SlideBackgroundInfo::getFillColorAsRGBA() const
{
    ::Color aColor = getFillColor();
    OString sColor = aColor.AsRGBHEXString().toUtf8();
    sal_uInt32 nAlpha = std::round((100 - getFillTransparency()) * 255 / 100.0);
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill ('0') << std::setw(2) << nAlpha;
    sColor += ss.str().c_str();
    return sColor;
}

bool SlideBackgroundInfo::getFillStyleImpl(const uno::Reference<drawing::XDrawPage>& xDrawPage)
{
    if( xDrawPage.is() )
    {
        uno::Reference< beans::XPropertySet > xPropSet( xDrawPage, uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            uno::Reference< beans::XPropertySet > xBackground;
            if (xPropSet->getPropertySetInfo()->hasPropertyByName(u"Background"_ustr))
                xPropSet->getPropertyValue( u"Background"_ustr ) >>= xBackground;
            if( xBackground.is() )
            {
                drawing::FillStyle aFillStyle;
                if( xBackground->getPropertyValue( u"FillStyle"_ustr ) >>= aFillStyle )
                {
                    maFillStyle = aFillStyle;
                    if (aFillStyle != drawing::FillStyle_NONE)
                    {
                        mxBackground = std::move(xBackground);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

using namespace ::css::animations;
using namespace ::css::beans;
using namespace ::css::container;
using namespace ::xmloff::token;
using namespace ::css::presentation;

template <typename T, std::size_t N>
constexpr auto mapEnumToString(std::pair<T, std::string_view> const (&items)[N])
{
    return frozen::make_unordered_map<T, std::string_view, N>(items);
}

constexpr auto constTransitionTypeToString = mapEnumToString<sal_Int16>({
    { animations::TransitionType::BARWIPE, "BarWipe" }, // Wipe
    { animations::TransitionType::PINWHEELWIPE, "PineWheelWipe" }, // Wheel
    { animations::TransitionType::SLIDEWIPE, "SlideWipe" }, // Cover, Uncover
    { animations::TransitionType::RANDOMBARWIPE, "RandomBarWipe" }, // Bars
    { animations::TransitionType::CHECKERBOARDWIPE, "CheckerBoardWipe" }, // Checkers
    { animations::TransitionType::FOURBOXWIPE, "FourBoxWipe" }, // Shape
    { animations::TransitionType::IRISWIPE, "IrisWipe" }, // Box
    { animations::TransitionType::FANWIPE, "FanWipe" }, // Wedge
    { animations::TransitionType::BLINDSWIPE, "BlindWipe"}, // Venetian
    { animations::TransitionType::FADE, "Fade"},
    { animations::TransitionType::DISSOLVE, "Dissolve"},
    { animations::TransitionType::PUSHWIPE, "PushWipe"}, // Comb
    { animations::TransitionType::ELLIPSEWIPE, "EllipseWipe"}, // Shape
    { animations::TransitionType::BARNDOORWIPE, "BarnDoorWipe"}, // Split
    { animations::TransitionType::WATERFALLWIPE, "WaterfallWipe"}, // Diagonal
    { animations::TransitionType::MISCSHAPEWIPE, "MiscShapeWipe"},
    { animations::TransitionType::ZOOM, "Zoom"}
});

constexpr auto constTransitionSubTypeToString = mapEnumToString<sal_Int16>({
    { animations::TransitionSubType::LEFTTORIGHT, "LeftToRight" },
    { animations::TransitionSubType::TOPTOBOTTOM, "TopToBottom" },
    { animations::TransitionSubType::EIGHTBLADE, "8Blade" },
    { animations::TransitionSubType::FOURBLADE, "4Blade" },
    { animations::TransitionSubType::THREEBLADE, "3Blade" },
    { animations::TransitionSubType::TWOBLADEVERTICAL, "2BladeVertical" },
    { animations::TransitionSubType::ONEBLADE, "1Blade" },
    { animations::TransitionSubType::FROMTOPLEFT, "FromTopLeft" },
    { animations::TransitionSubType::FROMTOPRIGHT, "FromTopRight"},
    { animations::TransitionSubType::FROMBOTTOMLEFT, "FromBottomLeft"},
    { animations::TransitionSubType::FROMBOTTOMRIGHT, "FromBottomRight"},
    { animations::TransitionSubType::VERTICAL, "Vertical"},
    { animations::TransitionSubType::HORIZONTAL, "Horizontal"},
    { animations::TransitionSubType::DOWN, "Down"},
    { animations::TransitionSubType::ACROSS, "Across"},
    { animations::TransitionSubType::CORNERSOUT, "CornersOut"},
    { animations::TransitionSubType::DIAMOND, "Diamond"},
    { animations::TransitionSubType::CIRCLE, "Circle"},
    { animations::TransitionSubType::RECTANGLE, "Rectangle"},
    { animations::TransitionSubType::CENTERTOP, "CenterTop"},
    { animations::TransitionSubType::CROSSFADE, "CrossFade"},
    { animations::TransitionSubType::FADEOVERCOLOR, "FadeOverColor"},
    { animations::TransitionSubType::FROMLEFT, "FromLeft"},
    { animations::TransitionSubType::FROMRIGHT, "FromRight"},
    { animations::TransitionSubType::FROMTOP, "FromTop"},
    { animations::TransitionSubType::HORIZONTALLEFT, "HorizontalLeft"},
    { animations::TransitionSubType::HORIZONTALRIGHT, "HorizontalRight"},
    { animations::TransitionSubType::COMBVERTICAL, "CombVertical"},
    { animations::TransitionSubType::COMBHORIZONTAL, "CombHorizontal"},
    { animations::TransitionSubType::TOPLEFT, "TopLeft"},
    { animations::TransitionSubType::TOPRIGHT, "TopRight"},
    { animations::TransitionSubType::BOTTOMRIGHT, "BottomRight"},
    { animations::TransitionSubType::BOTTOMLEFT, "BottomLeft"},
    { animations::TransitionSubType::TOPCENTER, "TopCenter"},
    { animations::TransitionSubType::RIGHTCENTER, "RightCenter"},
    { animations::TransitionSubType::BOTTOMCENTER, "BottomCenter"},
    { animations::TransitionSubType::FANOUTHORIZONTAL, "FanOutHorizontal"},
    { animations::TransitionSubType::CORNERSIN, "CornersIn"},
    { animations::TransitionSubType::HEART, "Heart"},
    { animations::TransitionSubType::ROTATEIN, "RotateIn"},
    { animations::TransitionSubType::AIRPLANE, "Airplane"}
});

constexpr auto constAnimationNodeTypeToString = mapEnumToString<sal_Int16>({
    { AnimationNodeType::ANIMATE, "Animate" },
    { AnimationNodeType::ANIMATECOLOR, "AnimateColor" },
    { AnimationNodeType::ANIMATEMOTION, "AnimateMotion" },
    { AnimationNodeType::ANIMATEPHYSICS, "Animate" },
    { AnimationNodeType::ANIMATETRANSFORM, "AnimateTransform" },
    { AnimationNodeType::AUDIO, "Audio" },
    { AnimationNodeType::COMMAND, "Command" },
    { AnimationNodeType::CUSTOM, "Custom" },
    { AnimationNodeType::ITERATE, "Iterate" },
    { AnimationNodeType::PAR, "Par" },
    { AnimationNodeType::SEQ, "Seq" },
    { AnimationNodeType::SET, "Set" },
    { AnimationNodeType::TRANSITIONFILTER, "TransitionFilter" },
});

constexpr auto constFillToString = mapEnumToString<sal_Int16>({
    { AnimationFill::DEFAULT, "Default" },
    { AnimationFill::REMOVE, "Remove" },
    { AnimationFill::FREEZE, "Freeze" },
    { AnimationFill::HOLD, "Hold" },
    { AnimationFill::TRANSITION, "Transition" },
    { AnimationFill::AUTO, "Auto" },
});

constexpr auto constRestartToString = mapEnumToString<sal_Int16>({
    { AnimationRestart::DEFAULT, "Default" },
    { AnimationRestart::ALWAYS, "Always" },
    { AnimationRestart::WHEN_NOT_ACTIVE, "WhenNotActive" },
    { AnimationRestart::NEVER, "Never" },
});

constexpr auto constEndSyncToString = mapEnumToString<sal_Int16>({
    { AnimationEndSync::FIRST, "First" },
    { AnimationEndSync::LAST, "Last" },
    { AnimationEndSync::ALL, "All" },
    { AnimationEndSync::MEDIA, "Media" },
});

constexpr auto constCalcModeToString = mapEnumToString<sal_Int16>({
    { AnimationCalcMode::DISCRETE, "Discrete" },
    { AnimationCalcMode::LINEAR, "Linear" },
    { AnimationCalcMode::PACED, "Paced" },
    { AnimationCalcMode::SPLINE, "Spline" },
});

constexpr auto constAdditiveModeToString = mapEnumToString<sal_Int16>({
    { AnimationAdditiveMode::BASE, "Base" },
    { AnimationAdditiveMode::SUM, "Sum" },
    { AnimationAdditiveMode::REPLACE, "Replace" },
    { AnimationAdditiveMode::MULTIPLY, "Multiply" },
    { AnimationAdditiveMode::NONE, "None" },
});

constexpr auto constEffectPresetClassToString = mapEnumToString<sal_Int16>({
    { EffectPresetClass::CUSTOM, "Custom" },
    { EffectPresetClass::ENTRANCE, "Entrance" },
    { EffectPresetClass::EXIT, "Exit" },
    { EffectPresetClass::EMPHASIS, "Emphasis" },
    { EffectPresetClass::MOTIONPATH, "MotionPath" },
    { EffectPresetClass::OLEACTION, "OleAction" },
    { EffectPresetClass::MEDIACALL, "MediaCall" },
});

constexpr auto constEffectNodeTypeToString = mapEnumToString<sal_Int16>({
    { EffectNodeType::DEFAULT, "Default" },
    { EffectNodeType::ON_CLICK, "OnClick" },
    { EffectNodeType::WITH_PREVIOUS, "WithPrevious" },
    { EffectNodeType::AFTER_PREVIOUS, "AfterPrevious" },
    { EffectNodeType::MAIN_SEQUENCE, "MainSequence" },
    { EffectNodeType::TIMING_ROOT, "TimingRoot" },
    { EffectNodeType::INTERACTIVE_SEQUENCE, "InteractiveSequence" },
});

constexpr auto constEventTriggerToString = mapEnumToString<sal_Int16>({
    { EventTrigger::BEGIN_EVENT, "BeginEvent" },
    { EventTrigger::END_EVENT, "EndEvent" },
    { EventTrigger::NONE, "None" },
    { EventTrigger::ON_BEGIN, "OnBegin" },
    { EventTrigger::ON_CLICK, "OnClick" },
    { EventTrigger::ON_DBL_CLICK, "OnDblClick" },
    { EventTrigger::ON_END, "OnEnd" },
    { EventTrigger::ON_MOUSE_ENTER, "OnMouseEnter" },
    { EventTrigger::ON_MOUSE_LEAVE, "OnMouseLeave" },
    { EventTrigger::ON_NEXT, "OnNext" },
    { EventTrigger::ON_PREV, "OnPrev" },
    { EventTrigger::ON_STOP_AUDIO, "OnStopAudio" },
    { EventTrigger::REPEAT, "Repeat" },
});

constexpr auto constTimingToString = mapEnumToString<Timing>({
    { Timing_INDEFINITE, "indefinite" },
    { Timing_MEDIA, "media" },
});

constexpr auto constTransformTypeToString = mapEnumToString<sal_Int16>({
    { AnimationTransformType::TRANSLATE, "Translate" },
    { AnimationTransformType::SCALE, "Scale" },
    { AnimationTransformType::ROTATE, "Rotate" },
    { AnimationTransformType::SKEWX, "SkewX" },
    { AnimationTransformType::SKEWY, "SkewY" },
});

constexpr auto constSubItemToString = mapEnumToString<sal_Int16>({
    { ShapeAnimationSubType::AS_WHOLE, "AsWhole" },
    { ShapeAnimationSubType::ONLY_BACKGROUND, "OnlyBackground" },
    { ShapeAnimationSubType::ONLY_TEXT, "OnlyText" },
});

constexpr auto constIterateTypeToString = mapEnumToString<sal_Int16>({
    { TextAnimationType::BY_PARAGRAPH, "ByParagraph" },
    { TextAnimationType::BY_WORD, "ByWord" },
    { TextAnimationType::BY_LETTER, "ByLetter" },
});

constexpr auto constFillStyleToString = mapEnumToString<drawing::FillStyle>({
    { drawing::FillStyle_NONE, "None" },
    { drawing::FillStyle_SOLID, "Solid" },
    { drawing::FillStyle_BITMAP, "Bitmap" },
    { drawing::FillStyle_GRADIENT, "Gradient" },
    { drawing::FillStyle_HATCH, "Hatch" },
});

constexpr auto constLineStyleToString = mapEnumToString<drawing::LineStyle>({
    { drawing::LineStyle_NONE, "None" },
    { drawing::LineStyle_SOLID, "Solid" },
    { drawing::LineStyle_DASH, "Dash" },
});


constexpr auto constAttributeNameToXMLEnum
    = frozen::make_unordered_map<std::string_view, XMLTokenEnum>({
        { "X", XML_X },
        { "Y", XML_Y },
        { "Width", XML_WIDTH },
        { "Height", XML_HEIGHT },
        { "Rotate", XML_ROTATE },
        { "SkewX", XML_SKEWX },
        { "FillColor", XML_FILL_COLOR },
        { "FillStyle", XML_FILL },
        { "LineColor", XML_STROKE_COLOR },
        { "LineStyle",XML_STROKE  },
        { "CharColor", XML_COLOR },
        { "CharRotation", XML_TEXT_ROTATION_ANGLE },
        { "CharWeight", XML_FONT_WEIGHT },
        { "CharUnderline", XML_TEXT_UNDERLINE },
        { "CharFontName", XML_FONT_FAMILY },
        { "CharHeight", XML_FONT_SIZE },
        { "CharPosture", XML_FONT_STYLE },
        { "Visibility", XML_VISIBILITY },
        { "Opacity", XML_OPACITY },
        { "DimColor", XML_DIM },
});

class AnimationsExporter
{
public:
    AnimationsExporter(::tools::JsonWriter& rWriter,
                       const Reference<drawing::XDrawPage>& xDrawPage, bool bAllyState);
    void exportAnimations();
    void exportTriggers() const;
    [[nodiscard]] bool hasEffects() const { return mbHasEffects; }

private:
    void exportNode(const Reference<XAnimationNode>& xNode);
    void exportNodeImpl(const Reference<XAnimationNode>& xNode);
    void exportContainer(const Reference<XTimeContainer>& xContainer);

    void exportAnimate(const Reference<XAnimate>& xAnimate);

    void convertValue(XMLTokenEnum eAttributeName, OStringBuffer& sTmp, const Any& rValue) const;
    void convertTiming(OStringBuffer& sTmp, const Any& rValue);

    void appendTrigger(const cpo::uno::Any& rTarget, const OString& rTriggerHash);
    void exportTriggersImpl(const uno::Reference<drawing::XShapes>& xShapes) const;

private:
    ::tools::JsonWriter& mrWriter;
    Reference<drawing::XDrawPage> mxDrawPage;
    Reference<XPropertySet> mxPageProps;
    Reference<XAnimationNode> mxRootNode;
    bool mbHasEffects;
    bool mbAllyState;
    std::unordered_map<SdrObject*, OString> maEventTriggerSet;
};

AnimationsExporter::AnimationsExporter(::tools::JsonWriter& rWriter,
                                       const Reference<drawing::XDrawPage>& xDrawPage,
                                       bool bAllyState)
    : mrWriter(rWriter)
    , mxDrawPage(xDrawPage)
    , mbHasEffects(false)
    , mbAllyState(bAllyState)
{
    if (!mxDrawPage.is())
        return;

    try
    {
        mxPageProps = Reference<XPropertySet>(xDrawPage, UNO_QUERY);
        if (!mxPageProps.is())
            return;

        Reference<XAnimationNodeSupplier> xAnimNodeSupplier(mxDrawPage, UNO_QUERY);
        if (!xAnimNodeSupplier.is())
            return;

        Reference<XAnimationNode> xRootNode = xAnimNodeSupplier->getAnimationNode();
        if (xRootNode.is())
        {
            // first check if there are no animations
            Reference<XEnumerationAccess> xEnumerationAccess(xRootNode, UNO_QUERY_THROW);
            Reference<XEnumeration> xEnumeration(xEnumerationAccess->createEnumeration(),
                                                 cpo::uno::UNO_SET_THROW);
            if (xEnumeration->hasMoreElements())
            {
                // first child node may be an empty main sequence, check this
                Reference<XAnimationNode> xMainNode(xEnumeration->nextElement(), UNO_QUERY_THROW);
                Reference<XEnumerationAccess> xMainEnumerationAccess(xMainNode, UNO_QUERY_THROW);
                Reference<XEnumeration> xMainEnumeration(
                    xMainEnumerationAccess->createEnumeration(), cpo::uno::UNO_SET_THROW);

                // only export if the main sequence is not empty or if there are additional
                // trigger sequences
                mbHasEffects
                    = xMainEnumeration->hasMoreElements() || xEnumeration->hasMoreElements();
            }
        }
        if (mbHasEffects)
            mxRootNode = std::move(xRootNode);
    }
    catch (const RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("sd", "unomodel: AnimationsExporter");
    }
}

template <typename EnumT, size_t N>
constexpr bool convertEnum(OStringBuffer& rBuffer, EnumT nValue,
                           const frozen::unordered_map<EnumT, std::string_view, N>& rMap)
{
    auto iterator = rMap.find(nValue);
    if (iterator == rMap.end())
        return false;
    rBuffer.append(iterator->second);
    return true;
}

void convertDouble(OStringBuffer& rBuffer, double fValue)
{
        ::rtl::math::doubleToStringBuffer(rBuffer, fValue, rtl_math_StringFormat_Automatic,
                                          rtl_math_DecimalPlaces_Max, '.', true);
}

void convertBool(OStringBuffer& rBuffer, bool bValue)
{
    rBuffer.append( bValue );
}

void convertPath(OStringBuffer& sTmp, const Any& rPath)
{
    OUString aStr;
    rPath >>= aStr;
    sTmp = aStr.toUtf8();
}

void convertColor(OStringBuffer& rBuffer, sal_Int32 nColor)
{
    OUStringBuffer aUBuffer;
    ::sax::Converter::convertColor(aUBuffer, nColor);
    rBuffer.append(aUBuffer.makeStringAndClear().toUtf8());
}

void convertColor(OStringBuffer& rBuffer, const Any& rValue)
{
    sal_Int32 nColor = 0;
    if (rValue >>= nColor)
    {
        convertColor(rBuffer, nColor);
    }
    else
    {
        Sequence<double> aHSL;
        if ((rValue >>= aHSL) && (aHSL.getLength() == 3))
        {
            rBuffer.append("hsl(" + OString::number(aHSL[0]) + ","
                           + OString::number(aHSL[1] * 100.0) + "%,"
                           + OString::number(aHSL[2] * 100.0) + "%)");
        }
    }
}

bool isValidNode(const Reference<XAnimationNode>& xNode)
{
    if (xNode.is())
    {
        sal_Int16 nNodeType = xNode->getType();
        auto iterator = constAnimationNodeTypeToString.find(nNodeType);
        return iterator != constAnimationNodeTypeToString.end();
    }
    return false;
}

SdrObject* getObjectForShape(uno::Reference<drawing::XShape> const& xShape)
{
    if (!xShape.is())
        return nullptr;
    SvxShape* pShape = comphelper::getFromUnoTunnel<SvxShape>(xShape);
    if (pShape)
        return pShape->GetSdrObject();
    return nullptr;
}

SdrObject* getTargetObject(const cpo::uno::Any& aTargetAny)
{
    SdrObject* pObject = nullptr;
    uno::Reference<drawing::XShape> xShape;

    if ((aTargetAny >>= xShape) && xShape.is())
    {
        pObject = getObjectForShape(xShape);
    }
    else // if target is not a shape - could be paragraph target containing a shape
    {
        presentation::ParagraphTarget aParagraphTarget;
        if ((aTargetAny >>= aParagraphTarget) && aParagraphTarget.Shape.is())
        {
            pObject = getObjectForShape(aParagraphTarget.Shape);
        }
    }

    return pObject;
}

bool isNodeTargetInShapeGroup(const Reference<XAnimationNode>& xNode)
{
    Reference<XAnimate> xAnimate(xNode, UNO_QUERY);
    if (xAnimate.is())
    {
        SdrObject* pObject = getTargetObject(xAnimate->getTarget());
        if (pObject)
            return pObject->getParentSdrObjectFromSdrObject() != nullptr;
    }
    return false;
}

bool isNodeTargetAGroup(const Reference<XAnimationNode>& xNode)
{
    Reference<XAnimate> xAnimate(xNode, UNO_QUERY);
    if (xAnimate.is())
    {
        SdrObject* pObject = getTargetObject(xAnimate->getTarget());
        if (pObject)
            return pObject->getChildrenOfSdrObject() != nullptr;
    }
    return false;
}

bool isEffectValidForTarget(const Reference<XAnimationNode>& xNode)
{
    const Sequence<NamedValue> aUserData(xNode->getUserData());
    for (const auto& rValue : aUserData)
    {
        if (!IsXMLToken(rValue.Name, XML_PRESET_ID))
            continue;

        OUString aPresetId;
        if (rValue.Value >>= aPresetId)
        {
            if (constNonValidEffectsForGroupSet.find(aPresetId.toUtf8())
                != constNonValidEffectsForGroupSet.end())
            {
                // it's in the list, so we need to check if the effect target is a group or not
                Reference<XTimeContainer> xContainer(xNode, UNO_QUERY);
                if (xContainer.is())
                {
                    Reference<XEnumerationAccess> xEnumerationAccess(xContainer, UNO_QUERY);
                    Reference<XEnumeration> xEnumeration = xEnumerationAccess->createEnumeration();

                    // target is the same for all children, check the first one
                    if (xEnumeration.is() && xEnumeration->hasMoreElements())
                    {
                        Reference<XAnimationNode> xChildNode(xEnumeration->nextElement(),
                                                             UNO_QUERY);
                        if (isNodeTargetAGroup(xChildNode))
                            return false;
                    }
                }
            }
        }
        // preset id found and checked, we can exit
        break;
    }
    return true;
}

void AnimationsExporter::exportAnimations()
{
    if (!mxDrawPage.is() || !mxPageProps.is() || !mxRootNode.is() || !hasEffects())
        return;

    if (isValidNode(mxRootNode))
    {
        auto aNode = mrWriter.startNode("root");
        exportNodeImpl(mxRootNode);
    }
}

void AnimationsExporter::exportNode(const Reference<XAnimationNode>& xNode)
{
    // afaics, when a shape is part of a group any applied effect is ignored
    // moreover, some kind of effect, like the ones based on color animations,
    // is ignored when applied to a group
    if (!isValidNode(xNode) || isNodeTargetInShapeGroup(xNode) || !isEffectValidForTarget(xNode))
        return;
    auto aStruct = mrWriter.startStruct();
    exportNodeImpl(xNode);
}

void AnimationsExporter::exportNodeImpl(const Reference<XAnimationNode>& xNode)
{
    try
    {
        std::string sId = GetInterfaceHash(xNode);
        mrWriter.put("id", sId);
        sal_Int16 nNodeType = xNode->getType();
        auto iterator = constAnimationNodeTypeToString.find(nNodeType);
        assert(iterator != constAnimationNodeTypeToString.end() && "must be previously checked with isValidNode");
        mrWriter.put("nodeName", iterator->second);

        // common properties
        OStringBuffer sTmp;
        Any aTemp;
        double fTemp = 0;
        sal_Int16 nTemp;

        aTemp = xNode->getBegin();
        if (aTemp.hasValue())
        {
            convertTiming(sTmp, aTemp);
            mrWriter.put("begin", sTmp.makeStringAndClear());
        }
        aTemp = xNode->getDuration();
        if (aTemp.hasValue())
        {
            if (aTemp >>= fTemp)
            {
                convertDouble(sTmp, fTemp);
                sTmp.append('s');
                mrWriter.put("dur", sTmp.makeStringAndClear());
            }
            else
            {
                Timing eTiming;
                if (aTemp >>= eTiming)
                {
                    mrWriter.put("dur", eTiming == Timing_INDEFINITE ? "indefinite" : "media");
                }
            }
        }
        aTemp = xNode->getEnd();
        if (aTemp.hasValue())
        {
            convertTiming(sTmp, aTemp);
            mrWriter.put("end", sTmp.makeStringAndClear());
        }
        nTemp = xNode->getFill();
        if (nTemp != AnimationFill::DEFAULT)
        {
            convertEnum(sTmp, nTemp, constFillToString);
            mrWriter.put("fill", sTmp.makeStringAndClear());
        }
        nTemp = xNode->getFillDefault();
        if (nTemp != AnimationFill::INHERIT)
        {
            convertEnum(sTmp, nTemp, constFillToString);
            mrWriter.put("fillDefault", sTmp.makeStringAndClear());
        }
        nTemp = xNode->getRestart();
        if (nTemp != AnimationRestart::DEFAULT)
        {
            convertEnum(sTmp, nTemp, constRestartToString);
            mrWriter.put("restart", sTmp.makeStringAndClear());
        }
        nTemp = xNode->getRestartDefault();
        if (nTemp != AnimationRestart::INHERIT)
        {
            convertEnum(sTmp, nTemp, constRestartToString);
            mrWriter.put("restartDefault", sTmp.makeStringAndClear());
        }
        fTemp = xNode->getAcceleration();
        if (fTemp != 0.0)
        {
            convertDouble(sTmp, fTemp);
            mrWriter.put("accelerate", sTmp.makeStringAndClear());
        }
        fTemp = xNode->getDecelerate();
        if (fTemp != 0.0)
        {
            convertDouble(sTmp, fTemp);
            mrWriter.put("decelerate", sTmp.makeStringAndClear());
        }
        bool bTemp = xNode->getAutoReverse();
        if (bTemp)
        {
            convertBool(sTmp, bTemp);
            mrWriter.put("autoreverse", sTmp.makeStringAndClear());
        }
        aTemp = xNode->getRepeatCount();
        if (aTemp.hasValue())
        {
            Timing eTiming;
            if ((aTemp >>= eTiming) && (eTiming == Timing_INDEFINITE))
            {
                mrWriter.put("repeatCount", "indefinite");
            }
            else if (aTemp >>= fTemp)
            {
                convertDouble(sTmp, fTemp);
                mrWriter.put("repeatCount", sTmp.makeStringAndClear());
            }
        }
        aTemp = xNode->getRepeatDuration();
        if (aTemp.hasValue())
        {
            Timing eTiming;
            if ((aTemp >>= eTiming) && (eTiming == Timing_INDEFINITE))
            {
                mrWriter.put("repeatDur", "indefinite");
            }
            else if (aTemp >>= fTemp)
            {
                convertDouble(sTmp, fTemp);
                mrWriter.put("repeatDur", sTmp.makeStringAndClear());
            }
        }
        aTemp = xNode->getEndSync();
        if (aTemp.hasValue() && (aTemp >>= nTemp))
        {
            convertEnum(sTmp, nTemp, constEndSyncToString);
            mrWriter.put("endSync", sTmp.makeStringAndClear());
        }

        sal_Int16 nContainerNodeType = EffectNodeType::DEFAULT;
        const Sequence<NamedValue> aUserData(xNode->getUserData());
        for (const auto& rValue : aUserData)
        {
            if (IsXMLToken(rValue.Name, XML_NODE_TYPE))
            {
                if ((rValue.Value >>= nContainerNodeType)
                    && (nContainerNodeType != EffectNodeType::DEFAULT))
                {
                    convertEnum(sTmp, nContainerNodeType, constEffectNodeTypeToString);
                    mrWriter.put("nodeType", sTmp.makeStringAndClear());
                }
            }
            else if (IsXMLToken(rValue.Name, XML_PRESET_ID))
            {
                OUString aPresetId;
                if (rValue.Value >>= aPresetId)
                {
                    mrWriter.put("presetId", aPresetId);

                    if (mbAllyState)
                    {
                        CustomAnimationPresetPtr pPreset
                            = CustomAnimationPresets::getCustomAnimationPresets()
                                  .getEffectDescriptor(aPresetId);
                        if (pPreset)
                            mrWriter.put("presetLabel", pPreset->getLabel());
                    }
                }
            }
            else if (IsXMLToken(rValue.Name, XML_PRESET_SUB_TYPE))
            {
                OUString aPresetSubType;
                if (rValue.Value >>= aPresetSubType)
                {
                    mrWriter.put("presetSubType", aPresetSubType);
                }
            }
            else if (IsXMLToken(rValue.Name, XML_PRESET_CLASS))
            {
                sal_Int16 nEffectPresetClass = sal_uInt16(0);
                if (rValue.Value >>= nEffectPresetClass)
                {
                    convertEnum(sTmp, nEffectPresetClass, constEffectPresetClassToString);
                    mrWriter.put("presetClass", sTmp.makeStringAndClear());
                }
            }
            else if (IsXMLToken(rValue.Name, XML_MASTER_ELEMENT))
            {
                Reference<cpo::uno::XInterface> xMaster;
                rValue.Value >>= xMaster;
                if (xMaster.is())
                {
                    const std::string aIdentifier(GetInterfaceHash(xMaster));
                    if (!aIdentifier.empty())
                        mrWriter.put("masterElement", aIdentifier);
                }
            }
            else if (IsXMLToken(rValue.Name, XML_GROUP_ID))
            {
                sal_Int32 nGroupId = 0;
                if (rValue.Value >>= nGroupId)
                    mrWriter.put("groupId", nGroupId);
            }
            else
            {
                OUString aTmp;
                if (rValue.Value >>= aTmp)
                    mrWriter.put(rValue.Name, aTmp);
            }
        }

        switch (nNodeType)
        {
            case AnimationNodeType::PAR:
            case AnimationNodeType::SEQ:
            case AnimationNodeType::ITERATE:
            {
                Reference<XTimeContainer> xContainer(xNode, UNO_QUERY_THROW);
                exportContainer(xContainer);
            }
            break;

            case AnimationNodeType::ANIMATE:
            case AnimationNodeType::SET:
            case AnimationNodeType::ANIMATEMOTION:
            case AnimationNodeType::ANIMATEPHYSICS:
            case AnimationNodeType::ANIMATECOLOR:
            case AnimationNodeType::ANIMATETRANSFORM:
            case AnimationNodeType::TRANSITIONFILTER:
            {
                Reference<XAnimate> xAnimate(xNode, UNO_QUERY_THROW);
                exportAnimate(xAnimate);
            }
            break;
            case AnimationNodeType::AUDIO:
            {
                SAL_WARN("sd", "AnimationsExporter::exportNode(): Audio Node not supported.");
            }
            break;
            case AnimationNodeType::COMMAND:
            {
                SAL_WARN("sd", "AnimationsExporter::exportNode(): Command Node not supported.");
            }
            break;
            default:
            {
                OSL_FAIL(
                    "sd unomodel: AnimationsExporter::exportNode(), invalid AnimationNodeType!");
            }
        }
    }
    catch (const RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("sd", "unomodel: AnimationsExporter");
    }
}

void AnimationsExporter::convertTiming(OStringBuffer& sTmp, const Any& rValue)
{
    if (!rValue.hasValue())
        return;

    if (auto pSequence = o3tl::tryAccess<Sequence<Any>>(rValue))
    {
        const sal_Int32 nLength = pSequence->getLength();
        sal_Int32 nElement;
        const Any* pAny = pSequence->getConstArray();

        OStringBuffer sTmp2;

        for (nElement = 0; nElement < nLength; nElement++, pAny++)
        {
            if (!sTmp.isEmpty())
                sTmp.append(';');
            convertTiming(sTmp2, *pAny);
            sTmp.append(sTmp2);
            sTmp2.setLength(0);
        }
    }
    else if (auto x = o3tl::tryAccess<double>(rValue))
    {
        sTmp.append(*x);
        sTmp.append('s');
    }
    else if (auto pTiming = o3tl::tryAccess<Timing>(rValue))
    {
        const auto svTiming = (*pTiming == Timing_MEDIA)
                                  ? constTimingToString.at(Timing_MEDIA)
                                  : constTimingToString.at(Timing_INDEFINITE);
        sTmp.append(svTiming);
    }
    else if (auto pEvent = o3tl::tryAccess<Event>(rValue))
    {
        OStringBuffer sTmp2;

        if (pEvent->Trigger != EventTrigger::NONE)
        {
            if (pEvent->Source.hasValue())
            {
                OStringBuffer aTriggerBuffer;
                // hash must not start with a digit or on client it is parsed as a time in seconds
                aTriggerBuffer.append("id");
                anim::convertTarget(aTriggerBuffer, pEvent->Source);
                OString sTriggerHash(aTriggerBuffer.makeStringAndClear());
                sTmp.append(sTriggerHash);
                sTmp.append('.');
                appendTrigger(pEvent->Source, sTriggerHash);
            }

            convertEnum(sTmp2, pEvent->Trigger, constEventTriggerToString);

            sTmp.append(sTmp2);
            sTmp2.setLength(0);
        }

        if (pEvent->Offset.hasValue())
        {
            convertTiming(sTmp2, pEvent->Offset);

            if (!sTmp.isEmpty())
                sTmp.append('+');

            sTmp.append(sTmp2);
            sTmp2.setLength(0);
        }
    }
    else
    {
        OSL_FAIL("sd.unomodel: AnimationsExporter::convertTiming, invalid value type!");
    }
}

void AnimationsExporter::appendTrigger(const cpo::uno::Any& rTarget, const OString& rTriggerHash)
{
    cpo::uno::Reference<cpo::uno::XInterface> xRef;
    rTarget >>= xRef;

    uno::Reference<drawing::XShape> xShape(xRef, uno::UNO_QUERY);
    if (!xShape.is())
    {
        if (auto xParagraphTarget = o3tl::tryAccess<css::presentation::ParagraphTarget>(rTarget))
        {
            xShape = xParagraphTarget->Shape;
        }
    }
    if (xShape.is())
    {
        auto* pObject = SdrObject::getSdrObjectFromXShape(xShape);
        maEventTriggerSet[pObject] = rTriggerHash;
    }
}

void AnimationsExporter::exportTriggersImpl(const uno::Reference<drawing::XShapes>& xShapes) const
{
    if (!xShapes.is())
        return;

    sal_Int32 nCount = xShapes->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        auto xObject = xShapes->getByIndex(i);
        uno::Reference<drawing::XShape> xShape(xObject, uno::UNO_QUERY);
        if (!xShape.is())
            continue;

        auto* pObject = SdrObject::getSdrObjectFromXShape(xShape);
        if (maEventTriggerSet.find(pObject) == maEventTriggerSet.end())
            continue;
        {
            auto aShape = mrWriter.startStruct();
            mrWriter.put("hash", maEventTriggerSet.at(pObject));
            {
                auto const& rRectangle = pObject->GetLogicRect();
                auto aRectangle
                    = o3tl::convert(rRectangle, o3tl::Length::mm100, o3tl::Length::twip);
                auto aRect = mrWriter.startNode("bounds");
                mrWriter.put("x", aRectangle.Left());
                mrWriter.put("y", aRectangle.Top());
                mrWriter.put("width", aRectangle.GetWidth());
                mrWriter.put("height", aRectangle.GetHeight());
            }
        }
    }
}

void AnimationsExporter::exportTriggers() const
{
    uno::Reference<drawing::XShapes> const xShapes(mxDrawPage, uno::UNO_QUERY_THROW);
    if (!xShapes.is())
        return;

    auto aTriggerList = mrWriter.startArray("triggers");
    exportTriggersImpl(xShapes);
}

void AnimationsExporter::convertValue(XMLTokenEnum eAttributeName, OStringBuffer& sTmp,
                                      const Any& rValue) const
{
    if (!rValue.hasValue())
        return;

    if (auto pValuePair = o3tl::tryAccess<ValuePair>(rValue))
    {
        OStringBuffer sTmp2;
        convertValue(eAttributeName, sTmp, pValuePair->First);
        sTmp.append(',');
        convertValue(eAttributeName, sTmp2, pValuePair->Second);
        sTmp.append(sTmp2);
    }
    else if (auto pSequence = o3tl::tryAccess<Sequence<Any>>(rValue))
    {
        const sal_Int32 nLength = pSequence->getLength();
        sal_Int32 nElement;
        const Any* pAny = pSequence->getConstArray();

        OStringBuffer sTmp2;

        for (nElement = 0; nElement < nLength; nElement++, pAny++)
        {
            if (!sTmp.isEmpty())
                sTmp.append(';');
            convertValue(eAttributeName, sTmp2, *pAny);
            sTmp.append(sTmp2);
            sTmp2.setLength(0);
        }
    }
    else
    {
        switch (eAttributeName)
        {
            case XML_X:
            case XML_Y:
            case XML_WIDTH:
            case XML_HEIGHT:
            case XML_ANIMATETRANSFORM:
            case XML_ANIMATEMOTION:
            case XML_ANIMATEPHYSICS:
            {
                if (auto sValue = o3tl::tryAccess<OUString>(rValue))
                {
                    sTmp.append(sValue->toUtf8());
                }
                else if (auto aValue = o3tl::tryAccess<double>(rValue))
                {
                    sTmp.append(*aValue);
                }
                else
                {
                    OSL_FAIL("sd::AnimationsExporter::convertValue(), invalid value type!");
                }
                return;
            }
            case XML_SKEWX:
            case XML_ROTATE:
            case XML_OPACITY:
            case XML_TRANSITIONFILTER:
                if (auto aValue = o3tl::tryAccess<double>(rValue))
                {
                    sTmp.append(*aValue);
                }
                break;
            case XML_TEXT_ROTATION_ANGLE:
                if (auto aValue = o3tl::tryAccess<sal_Int16>(rValue))
                {
                    // on win and armv7 platforms compiler complains
                    // that append(sal_Int16) is ambiguous
                    sTmp.append(static_cast<sal_Int32>(*aValue));
                }
                break;
            case XML_FILL_COLOR:
            case XML_STROKE_COLOR:
            case XML_DIM:
            case XML_COLOR:
            {
                convertColor(sTmp, rValue);
            }
            break;
            case XML_FILL:
                if (auto aValue = o3tl::tryAccess<drawing::FillStyle>(rValue))
                {
                    convertEnum(sTmp, *aValue, constFillStyleToString);
                }
                break;
            case XML_STROKE:
                if (auto aValue = o3tl::tryAccess<drawing::LineStyle>(rValue))
                {
                    convertEnum(sTmp, *aValue, constLineStyleToString);
                }
                break;
            case XML_FONTSIZE:
                if (auto aValue = o3tl::tryAccess<double>(rValue))
                {
                    double fValue = *aValue * 100;
                    fValue += fValue > 0 ? 0.5 : -0.5;
                    auto nValue = static_cast<sal_Int32>(fValue);
                    sTmp.append(nValue); // percent
                }
                break;
            case XML_FONT_WEIGHT:
            case XML_FONT_STYLE:
            case XML_TEXT_UNDERLINE:
                SAL_WARN("sd", "AnimationsExporter::convertValue(): value type "
                                   << GetXMLToken(eAttributeName) << " not supported");
                break;
            case XML_VISIBILITY:
                if (auto aValue = o3tl::tryAccess<bool>(rValue))
                {
                    OUString sValue = *aValue ? GetXMLToken(XML_VISIBLE) : GetXMLToken(XML_HIDDEN);
                    sTmp.append(sValue.toUtf8());
                }
                break;
            default:
                OSL_FAIL("unomodel: AnimationsExporter::convertValue(), invalid AttributeName!");
        }
    }
}

void AnimationsExporter::exportContainer(const Reference<XTimeContainer>& xContainer)
{
    try
    {
        const sal_Int32 nNodeType = xContainer->getType();

        if (nNodeType == AnimationNodeType::ITERATE)
        {
            OStringBuffer sTmp;
            Reference<XIterateContainer> xIter(xContainer, UNO_QUERY_THROW);

            Any aTemp(xIter->getTarget());
            if (aTemp.hasValue())
            {
                anim::convertTarget(sTmp, aTemp);
                mrWriter.put("targetElement", sTmp.makeStringAndClear());
            }
            sal_Int16 nTemp = xIter->getSubItem();
            if (nTemp)
            {
                convertEnum(sTmp, nTemp, constSubItemToString);
                mrWriter.put("subItem", sTmp.makeStringAndClear());
            }
            nTemp = xIter->getIterateType();
            if (nTemp)
            {
                convertEnum(sTmp, nTemp, constIterateTypeToString);
                mrWriter.put("iterateType", sTmp.makeStringAndClear());
            }
            double fTemp = xIter->getIterateInterval();
            if (fTemp != 0)
            {
                OUStringBuffer buf;
                ::sax::Converter::convertDuration(buf, fTemp / (24 * 60 * 60));
                mrWriter.put("iterateInterval", sTmp.makeStringAndClear());
            }
        }

        auto anArray = mrWriter.startArray("children");

        Reference<XEnumerationAccess> xEnumerationAccess(xContainer, UNO_QUERY_THROW);
        Reference<XEnumeration> xEnumeration(xEnumerationAccess->createEnumeration(),
                                             cpo::uno::UNO_SET_THROW);
        while (xEnumeration->hasMoreElements())
        {
            Reference<XAnimationNode> xChildNode(xEnumeration->nextElement(), UNO_QUERY_THROW);
            exportNode(xChildNode);
        }
    }
    catch (const RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("sd", "unomodel: AnimationsExporter");
    }
}

void AnimationsExporter::exportAnimate(const Reference<XAnimate>& xAnimate)
{
    try
    {
        const sal_Int16 nNodeType = xAnimate->getType();

        OStringBuffer sTmp;
        sal_Int16 nTemp;
        bool bTemp;

        Any aTemp(xAnimate->getTarget());
        if (aTemp.hasValue())
        {
            anim::convertTarget(sTmp, aTemp);
            mrWriter.put("targetElement", sTmp.makeStringAndClear());
            SdrObject* pObject = getTargetObject(aTemp);
            if (pObject)
            {
                mrWriter.put("title", pObject->GetTitle());

                if (mbAllyState)
                    mrWriter.put("objectName", sd::GetA11yObjectName(*pObject));
            }
        }
        nTemp = xAnimate->getSubItem();
        if (nTemp)
        {
            convertEnum(sTmp, nTemp, constSubItemToString);
            mrWriter.put("subItem", sTmp.makeStringAndClear());
        }

        XMLTokenEnum eAttributeName = XML_TOKEN_INVALID;
        if (nNodeType == AnimationNodeType::TRANSITIONFILTER)
        {
            eAttributeName = XML_TRANSITIONFILTER;
        }
        else if (nNodeType == AnimationNodeType::ANIMATETRANSFORM)
        {
            eAttributeName = XML_ANIMATETRANSFORM;
        }
        else if (nNodeType == AnimationNodeType::ANIMATEMOTION)
        {
            eAttributeName = XML_ANIMATEMOTION;
        }
        else if (nNodeType == AnimationNodeType::ANIMATEPHYSICS)
        {
            eAttributeName = XML_ANIMATEPHYSICS;
        }
        else
        {
            OString sTemp(xAnimate->getAttributeName().toUtf8());
            if (!sTemp.isEmpty())
            {
                auto iterator = constAttributeNameToXMLEnum.find(sTemp);
                if (iterator != constAttributeNameToXMLEnum.end())
                {
                    eAttributeName = iterator->second;
                    mrWriter.put("attributeName", sTemp);
                }
                else
                {
                    mrWriter.put("attributeName", "invalid");
                }
            }
        }

        Sequence<Any> aValues(xAnimate->getValues());
        if (aValues.hasElements())
        {
            aTemp <<= aValues;
            convertValue(eAttributeName, sTmp, aTemp);
            mrWriter.put("values", sTmp.makeStringAndClear());
        }
        else
        {
            aTemp = xAnimate->getFrom();
            if (aTemp.hasValue())
            {
                convertValue(eAttributeName, sTmp, aTemp);
                mrWriter.put("from", sTmp.makeStringAndClear());
            }

            aTemp = xAnimate->getBy();
            if (aTemp.hasValue())
            {
                convertValue(eAttributeName, sTmp, aTemp);
                mrWriter.put("by", sTmp.makeStringAndClear());
            }

            aTemp = xAnimate->getTo();
            if (aTemp.hasValue())
            {
                convertValue(eAttributeName, sTmp, aTemp);
                mrWriter.put("to", sTmp.makeStringAndClear());
            }
        }

        if (nNodeType != AnimationNodeType::SET)
        {
            const Sequence<double> aKeyTimes(xAnimate->getKeyTimes());
            if (aKeyTimes.hasElements())
            {
                for (const auto& rKeyTime : aKeyTimes)
                {
                    if (!sTmp.isEmpty())
                        sTmp.append(';');

                    sTmp.append(rKeyTime);
                }
                mrWriter.put("keyTimes", sTmp.makeStringAndClear());
            }

            OUString sTemp(xAnimate->getFormula());
            if (!sTemp.isEmpty())
            {
                mrWriter.put("formula", sTemp);
            }

            if ((nNodeType != AnimationNodeType::TRANSITIONFILTER)
                && (nNodeType != AnimationNodeType::AUDIO))
            {
                // calcMode  = "discrete | linear | paced | spline"
                nTemp = xAnimate->getCalcMode();
                if (((nNodeType == AnimationNodeType::ANIMATEMOTION)
                     && (nTemp != AnimationCalcMode::PACED))
                    || ((nNodeType != AnimationNodeType::ANIMATEMOTION)
                        && (nTemp != AnimationCalcMode::LINEAR)))
                {
                    convertEnum(sTmp, nTemp, constCalcModeToString);
                    mrWriter.put("calcMode", sTmp.makeStringAndClear());
                }

                bTemp = xAnimate->getAccumulate();
                if (bTemp)
                {
                    mrWriter.put("accumulate", "sum");
                }

                nTemp = xAnimate->getAdditive();
                if (nTemp != AnimationAdditiveMode::REPLACE)
                {
                    convertEnum(sTmp, nTemp, constAdditiveModeToString);
                    mrWriter.put("additive", sTmp.makeStringAndClear());
                }
            }

            const Sequence<TimeFilterPair> aTimeFilter(xAnimate->getTimeFilter());
            if (aTimeFilter.hasElements())
            {
                for (const auto& rPair : aTimeFilter)
                {
                    if (!sTmp.isEmpty())
                        sTmp.append(';');

                    sTmp.append(OString::number(rPair.Time) + ","
                                + OString::number(rPair.Progress));
                }
                mrWriter.put("keySplines", sTmp.makeStringAndClear());
            }
        }

        switch (nNodeType)
        {
            case AnimationNodeType::ANIMATE:
            case AnimationNodeType::SET:
                // All data is exported by the common code above. These node
                // types carry no type-specific properties to add here.
                break;
            case AnimationNodeType::ANIMATEMOTION:
            {
                Reference<XAnimateMotion> xAnimateMotion(xAnimate, UNO_QUERY_THROW);

                aTemp = xAnimateMotion->getPath();
                if (aTemp.hasValue())
                {
                    convertPath(sTmp, aTemp);
                    mrWriter.put("path", sTmp.makeStringAndClear());
                }
            }
            break;
            case AnimationNodeType::ANIMATEPHYSICS:
            {
                SAL_WARN(
                    "sd",
                    "unomodel: AnimationsExporter::exportAnimate(): AnimatePhysics not supported");
            }
            break;
            case AnimationNodeType::ANIMATECOLOR:
            {
                Reference<XAnimateColor> xAnimateColor(xAnimate, UNO_QUERY_THROW);

                nTemp = xAnimateColor->getColorInterpolation();
                mrWriter.put("colorInterpolation",
                             (nTemp == AnimationColorSpace::RGB) ? "rgb" : "hsl");

                bTemp = xAnimateColor->getDirection();
                mrWriter.put("colorInterpolationDirection",
                             bTemp ? "clockwise" : "counterClockwise");
            }
            break;
            case AnimationNodeType::ANIMATETRANSFORM:
            {
                mrWriter.put("attributeName", "transform");

                Reference<XAnimateTransform> xTransform(xAnimate, UNO_QUERY_THROW);
                nTemp = xTransform->getTransformType();
                convertEnum(sTmp, nTemp, constTransformTypeToString);
                mrWriter.put("transformType", sTmp.makeStringAndClear());
            }
            break;
            case AnimationNodeType::TRANSITIONFILTER:
            {
                Reference<XTransitionFilter> xTransitionFilter(xAnimate, UNO_QUERY);

                sal_Int16 nTransition = xTransitionFilter->getTransition();
                convertEnum(sTmp, nTransition, constTransitionTypeToString);
                mrWriter.put("transitionType", sTmp.makeStringAndClear());

                sal_Int16 nSubtype = xTransitionFilter->getSubtype();
                if (nSubtype != TransitionSubType::DEFAULT)
                {
                    convertEnum(sTmp, nSubtype, constTransitionSubTypeToString);
                    mrWriter.put("transitionSubType", sTmp.makeStringAndClear());
                }

                bTemp = xTransitionFilter->getMode();
                if (!bTemp)
                    mrWriter.put("transitionMode", "out");

                bTemp = xTransitionFilter->getDirection();
                if (!bTemp)
                    mrWriter.put("transitionDirection", "reverse");

                if ((nTransition == TransitionType::FADE)
                    && ((nSubtype == TransitionSubType::FADETOCOLOR)
                        || (nSubtype == TransitionSubType::FADEFROMCOLOR)))
                {
                    sal_Int32 nColor = xTransitionFilter->getFadeColor();
                    convertColor(sTmp, nColor);
                    mrWriter.put("transitionFadeColor", sTmp.makeStringAndClear());
                }
            }
            break;
            default:
            {
                SAL_WARN("sd",
                         "unomodel: AnimationsExporter::exportAnimate(): not supported node type: "
                             << nNodeType);
            }
        }
    }
    catch (const cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sd", "unomodel: AnimationsExporter");
    }
}

/// Returns a snapshot of one SdrText's paragraphs. Text that is still being
/// typed lives in the object's editing outliner rather than in the stored
/// paragraph object, so an active edit session is snapshotted from there.
/// An empty result means the text carries no paragraphs.
std::optional<OutlinerParaObject> getTextParagraphSnapshot(const SdrTextObj* pTextObj,
                                                           const SdrText* pText)
{
    if (!pText)
        return std::nullopt;
    if (pTextObj->IsInEditMode() && pText == pTextObj->getActiveText())
    {
        if (std::optional<OutlinerParaObject> oEdited = pTextObj->CreateEditOutlinerParaObject())
            return oEdited;
    }
    const OutlinerParaObject* pStored = pText->GetOutlinerParaObject();
    if (!pStored)
        return std::nullopt;
    return *pStored;
}

// The placeholder role of an object on a slide, as a stable name for the
// document-structure JSON. Empty for a plain object with no placeholder role.
OUString lcl_PresObjKindName(PresObjKind eKind)
{
    switch (eKind)
    {
        case PresObjKind::Title:
            return u"Title"_ustr;
        case PresObjKind::Outline:
            return u"Outline"_ustr;
        case PresObjKind::Text:
            return u"Text"_ustr;
        case PresObjKind::Graphic:
            return u"Graphic"_ustr;
        case PresObjKind::Object:
            return u"Object"_ustr;
        case PresObjKind::Chart:
            return u"Chart"_ustr;
        case PresObjKind::OrgChart:
            return u"OrgChart"_ustr;
        case PresObjKind::Table:
            return u"Table"_ustr;
        case PresObjKind::Page:
            return u"Page"_ustr;
        case PresObjKind::Handout:
            return u"Handout"_ustr;
        case PresObjKind::Notes:
            return u"Notes"_ustr;
        case PresObjKind::PagePreview:
            return u"PagePreview"_ustr;
        case PresObjKind::Header:
            return u"Header"_ustr;
        case PresObjKind::Footer:
            return u"Footer"_ustr;
        case PresObjKind::DateTime:
            return u"DateTime"_ustr;
        case PresObjKind::SlideNumber:
            return u"SlideNumber"_ustr;
        case PresObjKind::Calc:
            return u"Calc"_ustr;
        case PresObjKind::Media:
            return u"Media"_ustr;
        case PresObjKind::NONE:
            break;
    }
    return OUString();
}

void GetDocStructureSlides(::tools::JsonWriter& rJsonWriter, const SdXImpressDocument* pDoc,
                           const std::map<OUString, OUString>& rArguments)
{
    auto it = rArguments.find(u"filter"_ustr);
    if (it != rArguments.end())
    {
        // If filter is present but we are filtering not to slide information
        if (!it->second.equals(u"slides"_ustr))
            return;
    }

    sal_uInt16 nPageCount = pDoc->GetDoc()->GetSdPageCount(PageKind::Standard);
    sal_uInt16 nMasterPageCount = pDoc->GetDoc()->GetMasterSdPageCount(PageKind::Standard);

    rJsonWriter.put("SlideCount", nPageCount);
    rJsonWriter.put("MasterSlideCount", nMasterPageCount);

    // write data of every master slide
    if (nMasterPageCount > 0)
    {
        auto aMasterPagesNode = rJsonWriter.startNode("MasterSlides");
        for (int nMPId = 0; nMPId < nMasterPageCount; nMPId++)
        {
            auto aMasterPageNode = rJsonWriter.startNode("MasterSlide " + std::to_string(nMPId));
            SdPage* pMaster = pDoc->GetDoc()->GetMasterSdPage(nMPId, PageKind::Standard);
            rJsonWriter.put("Name", pMaster->GetName());
            // The color set and font scheme the master carries, when it has
            // one. A deck's look follows its masters, so the theme names tell
            // a reader of this structure about the deck's design.
            if (auto const& pTheme = pMaster->getSdrPageProperties().getTheme())
                rJsonWriter.put("Theme", pTheme->GetName());
        }
    }

    // write data of every slide
    if (nPageCount > 0)
    {
        auto aPagesNode = rJsonWriter.startNode("Slides");
        for (int nPId = 0; nPId < nPageCount; nPId++)
        {
            auto aPageNode = rJsonWriter.startNode("Slide " + std::to_string(nPId));
            SdPage* pPageStandard = pDoc->GetDoc()->GetSdPage(nPId, PageKind::Standard);

            // Slide Name
            rJsonWriter.put("SlideName", pPageStandard->GetName());

            // MasterSlide Name
            const FmFormPage* pMasterPage
                = dynamic_cast<const FmFormPage*>(&pPageStandard->TRG_GetMasterPage());

            if (pMasterPage)
            {
                rJsonWriter.put("MasterSlideName", pMasterPage->GetName());
            }

            // Layout id, and name.
            AutoLayout nLayout = pPageStandard->GetAutoLayout();
            rJsonWriter.put("LayoutId", static_cast<int>(nLayout));
            rJsonWriter.put("LayoutName", SdPage::autoLayoutToString(nLayout));

            // Every Objects in the page
            int nObjCount = pPageStandard->GetObjCount();
            rJsonWriter.put("ObjectCount", nObjCount);

            if (nObjCount > 0)
            {
                auto aObjectsNode = rJsonWriter.startNode("Objects");
                for (int nOId = 0; nOId < nObjCount; nOId++)
                {
                    auto aObjectNode = rJsonWriter.startNode("Objects " + std::to_string(nOId));
                    SdrObject* pSdrObj = pPageStandard->GetObj(nOId);
                    // The object's placeholder role, and whether the
                    // placeholder still shows only its prompt text - together
                    // they say which layout slots hold content and which are
                    // still empty.
                    const OUString aKindName
                        = lcl_PresObjKindName(pPageStandard->GetPresObjKind(pSdrObj));
                    if (!aKindName.isEmpty())
                        rJsonWriter.put("Kind", aKindName);
                    if (pSdrObj->IsEmptyPresObj())
                        rJsonWriter.put("Empty", true);
                    SdrTextObj* pSdrTxtObj = DynCastSdrTextObj(pSdrObj);
                    if (pSdrTxtObj && pSdrTxtObj->HasText())
                    {
                        sal_Int32 nTextCount = pSdrTxtObj->getTextCount();
                        rJsonWriter.put("TextCount", nTextCount);
                        if (nTextCount > 0)
                        {
                            auto aTextsNode = rJsonWriter.startNode("Texts");
                            for (int nTId = 0; nTId < nTextCount; nTId++)
                            {
                                auto aTextNode
                                    = rJsonWriter.startNode("Text " + std::to_string(nTId));
                                std::optional<OutlinerParaObject> oParagraphs
                                    = getTextParagraphSnapshot(pSdrTxtObj,
                                                               pSdrTxtObj->getText(nTId));
                                if (!oParagraphs)
                                    continue;

                                const EditTextObject& rTextObject = oParagraphs->GetTextObject();
                                sal_Int32 nParaCount = rTextObject.GetParagraphCount();

                                rJsonWriter.put("ParaCount", nParaCount);
                                auto aParasNode = rJsonWriter.startArray("Paragraphs");
                                for (int nParaId = 0; nParaId < nParaCount; nParaId++)
                                {
                                    OUString aParaStr(rTextObject.GetText(nParaId));

                                    rJsonWriter.putSimpleValue(aParaStr);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/// Writes the text of every slide as markdown into a "BodyText" node, one
/// "## Slide N: <name>" section per slide. Reads the live document, so
/// unsaved edits and text still being typed are reflected. Whole slides are
/// appended until the size cap is reached, so a truncated result ends at a
/// slide boundary.
void GetDocStructureSlidesText(::tools::JsonWriter& rJsonWriter, const SdXImpressDocument* pDoc)
{
    auto aBodyNode = rJsonWriter.startNode("BodyText");
    rJsonWriter.put("format", "markdown");

    SdDrawDocument* pDrawDoc = pDoc->GetDoc();
    const sal_uInt16 nPageCount = pDrawDoc ? pDrawDoc->GetSdPageCount(PageKind::Standard) : 0;

    OUStringBuffer aBody;
    bool bTruncated = false;
    sal_uInt16 nIncludedSlides = 0;
    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        SdPage* pPageStandard = pDrawDoc->GetSdPage(nPage, PageKind::Standard);

        OUStringBuffer aSlideBuffer("## Slide " + OUString::number(nPage + 1) + ": "
                                    + pPageStandard->GetName() + "\n");
        const int nObjCount = pPageStandard->GetObjCount();
        for (int nObj = 0; nObj < nObjCount; ++nObj)
        {
            SdrTextObj* pTextObj = DynCastSdrTextObj(pPageStandard->GetObj(nObj));
            if (!pTextObj || !pTextObj->HasText())
                continue;
            // An empty presentation object shows only its placeholder prompt
            // (like "Click to add Title"), not document content. One with an
            // active edit session is kept: the flag is cleared only when the
            // edit ends, while the typed text already lives in the editing
            // outliner.
            if (pTextObj->IsEmptyPresObj() && !pTextObj->IsInEditMode())
                continue;

            const sal_Int32 nTextCount = pTextObj->getTextCount();
            for (sal_Int32 nText = 0; nText < nTextCount; ++nText)
            {
                std::optional<OutlinerParaObject> oParagraphs
                    = getTextParagraphSnapshot(pTextObj, pTextObj->getText(nText));
                if (!oParagraphs)
                    continue;

                const EditTextObject& rTextObject = oParagraphs->GetTextObject();
                const sal_Int32 nParaCount = rTextObject.GetParagraphCount();
                aSlideBuffer.append("\n");
                for (sal_Int32 nPara = 0; nPara < nParaCount; ++nPara)
                    aSlideBuffer.append(rTextObject.GetText(nPara) + "\n");
            }
        }

        const OUString aSlideText = aSlideBuffer.makeStringAndClear();
        if (aBody.getLength() + aSlideText.getLength() > KitHelper::AIBodyTextMaxChars)
        {
            bTruncated = true;
            // A single slide can exceed the cap on its own; a hard-cut head
            // of that slide is more useful than an empty result.
            if (nIncludedSlides == 0)
            {
                aBody.append(aSlideText.subView(0, KitHelper::AIBodyTextMaxChars));
                ++nIncludedSlides;
            }
            break;
        }
        aBody.append(aSlideText + "\n");
        ++nIncludedSlides;
    }

    // Emit the truncation flag and an explicit instruction before the text,
    // so the model acts on it rather than overlooking a flag buried after a
    // large body of markdown.
    rJsonWriter.put("truncated", bTruncated);
    if (bTruncated)
        rJsonWriter.put("note",
                        "The presentation is large, so only the first "
                            + OUString::number(nIncludedSlides) + " of "
                            + OUString::number(nPageCount)
                            + " slides are included here. Tell the user the summary covers only "
                              "those slides.");
    rJsonWriter.put("text", aBody.makeStringAndClear());
}

} // end anonymous namespace

SdUnoForbiddenCharsTable::SdUnoForbiddenCharsTable( SdrModel* pModel )
: SvxUnoForbiddenCharsTable( pModel->GetForbiddenCharsTable() ), mpModel( pModel )
{
    StartListening( *pModel );
}

void SdUnoForbiddenCharsTable::onChange()
{
    if( mpModel )
    {
        mpModel->ReformatAllTextObjects();
    }
}

SdUnoForbiddenCharsTable::~SdUnoForbiddenCharsTable()
{
    SolarMutexGuard g;

    if( mpModel )
        EndListening( *mpModel );
}

void SdUnoForbiddenCharsTable::Notify( SfxBroadcaster&, const SfxHint& rHint ) noexcept
{
    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;
    const SdrHint* pSdrHint = static_cast<const SdrHint*>( &rHint );
    if( SdrHintKind::ModelCleared == pSdrHint->GetKind() )
    {
        mpModel = nullptr;
    }
}

const sal_uInt16 WID_MODEL_LANGUAGE           =  1;
const sal_uInt16 WID_MODEL_TABSTOP            =  2;
const sal_uInt16 WID_MODEL_VISAREA            =  3;
const sal_uInt16 WID_MODEL_MAPUNIT            =  4;
const sal_uInt16 WID_MODEL_FORBCHARS          =  5;
const sal_uInt16 WID_MODEL_CONTFOCUS          =  6;
const sal_uInt16 WID_MODEL_DSGNMODE           =  7;
const sal_uInt16 WID_MODEL_BASICLIBS          =  8;
const sal_uInt16 WID_MODEL_RUNTIMEUID         =  9;
const sal_uInt16 WID_MODEL_BUILDID            = 10;
const sal_uInt16 WID_MODEL_HASVALIDSIGNATURES = 11;
const sal_uInt16 WID_MODEL_DIALOGLIBS         = 12;
const sal_uInt16 WID_MODEL_FONTS              = 13;
const sal_uInt16 WID_MODEL_INTEROPGRABBAG     = 14;
const sal_uInt16 WID_MODEL_THEME = 15;
const sal_uInt16 WID_MODEL_ALLOWLINKUPDATE    = 16;
const sal_uInt16 WID_MODEL_SLIDESECTIONS      = 17;
const sal_uInt16 WID_MODEL_ANIMATIONSOUNDLINK = 18;

static const SvxItemPropertySet* ImplGetDrawModelPropertySet()
{
    // Attention: the first parameter HAS TO BE sorted!!!
    const static SfxItemPropertyMapEntry aDrawModelPropertyMap_Impl[] =
    {
        { u"BuildId"_ustr,                WID_MODEL_BUILDID,            ::cppu::UnoType<OUString>::get(),                      0, 0},
        { sUNO_Prop_CharLocale,           WID_MODEL_LANGUAGE,           ::cppu::UnoType<lang::Locale>::get(),                                  0, 0},
        { sUNO_Prop_TabStop,              WID_MODEL_TABSTOP,            ::cppu::UnoType<sal_Int32>::get(),                                     0, 0},
        { sUNO_Prop_VisibleArea,          WID_MODEL_VISAREA,            ::cppu::UnoType<awt::Rectangle>::get(),                                0, 0},
        { sUNO_Prop_MapUnit,              WID_MODEL_MAPUNIT,            ::cppu::UnoType<sal_Int16>::get(),                                     beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_ForbiddenCharacters,  WID_MODEL_FORBCHARS,          cppu::UnoType<i18n::XForbiddenCharacters>::get(), beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_AutomContFocus,       WID_MODEL_CONTFOCUS,          cppu::UnoType<bool>::get(),                                                 0, 0},
        { sUNO_Prop_ApplyFrmDsgnMode,     WID_MODEL_DSGNMODE,           cppu::UnoType<bool>::get(),                                                 0, 0},
        { u"BasicLibraries"_ustr,         WID_MODEL_BASICLIBS,          cppu::UnoType<script::XLibraryContainer>::get(),  beans::PropertyAttribute::READONLY, 0},
        { u"DialogLibraries"_ustr,        WID_MODEL_DIALOGLIBS,         cppu::UnoType<script::XLibraryContainer>::get(),  beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_RuntimeUID,           WID_MODEL_RUNTIMEUID,         ::cppu::UnoType<OUString>::get(),                      beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_HasValidSignatures,   WID_MODEL_HASVALIDSIGNATURES, ::cppu::UnoType<bool>::get(),                      beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_AllowLinkUpdate,      WID_MODEL_ALLOWLINKUPDATE,    ::cppu::UnoType<bool>::get(),                      beans::PropertyAttribute::READONLY, 0},
        { u"Fonts"_ustr,                  WID_MODEL_FONTS,              cppu::UnoType<cpo::uno::Sequence<cpo::uno::Any>>::get(),                     beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_InteropGrabBag,       WID_MODEL_INTEROPGRABBAG,     cppu::UnoType<cpo::uno::Sequence< beans::PropertyValue >>::get(),       0, 0},
        { u"SlideSections"_ustr,          WID_MODEL_SLIDESECTIONS,      cppu::UnoType<cpo::uno::Sequence< beans::PropertyValue >>::get(),       0, 0},
        { u"HasExternalAnimationSoundLink"_ustr, WID_MODEL_ANIMATIONSOUNDLINK, cppu::UnoType<bool>::get(),       0, 0},
        { sUNO_Prop_Theme,                WID_MODEL_THEME,              cppu::UnoType<util::XTheme>::get(),       0, 0},
    };
    static SvxItemPropertySet aDrawModelPropertySet_Impl( aDrawModelPropertyMap_Impl, SdrObject::GetGlobalDrawObjectItemPool() );
    return &aDrawModelPropertySet_Impl;
}

// this ctor is used from the DocShell
SdXImpressDocument::SdXImpressDocument(::sd::DrawDocShell* pShell, bool bClipBoard)
:   SfxBaseModel( pShell ),
    mpDocShell( pShell ),
    mpDoc( pShell ? pShell->GetDoc() : nullptr ),
    mbDisposed(false),
    mbImpressDoc( pShell && pShell->GetDoc() && pShell->GetDoc()->GetDocumentType() == DocumentType::Impress ),
    mbClipBoard( bClipBoard ),
    mpPropSet( ImplGetDrawModelPropertySet() ),
    mbPaintTextEdit( true )
{
    if( mpDoc )
    {
        StartListening( *mpDoc );
    }
    else
    {
        OSL_FAIL("DocShell is invalid");
    }
}

SdXImpressDocument::SdXImpressDocument(SdDrawDocument* pDoc, bool bClipBoard)
:   SfxBaseModel( nullptr ),
    mpDocShell( nullptr ),
    mpDoc( pDoc ),
    mbDisposed(false),
    mbImpressDoc( pDoc && pDoc->GetDocumentType() == DocumentType::Impress ),
    mbClipBoard( bClipBoard ),
    mpPropSet( ImplGetDrawModelPropertySet() ),
    mbPaintTextEdit( true )
{
    if( mpDoc )
    {
        StartListening( *mpDoc );
    }
    else
    {
        OSL_FAIL("SdDrawDocument is invalid");
    }
}

/***********************************************************************
*                                                                      *
***********************************************************************/
SdXImpressDocument::~SdXImpressDocument() noexcept
{
}

// XInterface
cpo::uno::Any SdXImpressDocument::queryInterface( const cpo::uno::Type & rType )
{
    cpo::uno::Any aAny;

    if (rType == cppu::UnoType<lang::XServiceInfo>::get())
        aAny <<= uno::Reference<lang::XServiceInfo>(this);
    else if (rType == cppu::UnoType<beans::XPropertySet>::get())
        aAny <<= uno::Reference<beans::XPropertySet>(this);
    else if (rType == cppu::UnoType<lang::XMultiServiceFactory>::get())
        aAny <<= uno::Reference<lang::XMultiServiceFactory>(this);
    else if (rType == cppu::UnoType<drawing::XDrawPageDuplicator>::get())
        aAny <<= uno::Reference<drawing::XDrawPageDuplicator>(this);
    else if (rType == cppu::UnoType<drawing::XLayerSupplier>::get())
        aAny <<= uno::Reference<drawing::XLayerSupplier>(this);
    else if (rType == cppu::UnoType<drawing::XMasterPagesSupplier>::get())
        aAny <<= uno::Reference<drawing::XMasterPagesSupplier>(this);
    else if (rType == cppu::UnoType<drawing::XDrawPagesSupplier>::get())
        aAny <<= uno::Reference<drawing::XDrawPagesSupplier>(this);
    else if (rType == cppu::UnoType<presentation::XHandoutMasterSupplier>::get())
        aAny <<= uno::Reference<presentation::XHandoutMasterSupplier>(this);
    else if (rType == cppu::UnoType<document::XLinkTargetSupplier>::get())
        aAny <<= uno::Reference<document::XLinkTargetSupplier>(this);
    else if (rType == cppu::UnoType<style::XStyleFamiliesSupplier>::get())
        aAny <<= uno::Reference<style::XStyleFamiliesSupplier>(this);
    else if (rType == cppu::UnoType<css::ucb::XAnyCompareFactory>::get())
        aAny <<= uno::Reference<css::ucb::XAnyCompareFactory>(this);
    else if (rType == cppu::UnoType<view::XRenderable>::get())
        aAny <<= uno::Reference<view::XRenderable>(this);
    else if (mbImpressDoc && rType == cppu::UnoType<presentation::XPresentationSupplier>::get())
        aAny <<= uno::Reference< presentation::XPresentationSupplier >(this);
    else if (mbImpressDoc && rType == cppu::UnoType<presentation::XCustomPresentationSupplier>::get())
        aAny <<= uno::Reference< presentation::XCustomPresentationSupplier >(this);
    else
        return SfxBaseModel::queryInterface(rType);

    return aAny;
}

void SdXImpressDocument::acquire() noexcept
{
    SfxBaseModel::acquire();
}

void SdXImpressDocument::release() noexcept
{
    if (osl_atomic_decrement( &m_refCount ) != 0)
        return;

    // restore reference count:
    osl_atomic_increment( &m_refCount );
    if(!mbDisposed)
    {
        try
        {
            dispose();
        }
        catch (const cpo::uno::RuntimeException&)
        {
            // don't break throw ()
            TOOLS_WARN_EXCEPTION( "sd", "" );
        }
    }
    SfxBaseModel::release();
}

// XUnoTunnel
const cpo::uno::Sequence< sal_Int8 > & SdXImpressDocument::getUnoTunnelId() noexcept
{
    static const comphelper::UnoIdInit theSdXImpressDocumentUnoTunnelId;
    return theSdXImpressDocumentUnoTunnelId.getSeq();
}

sal_Int64 SdXImpressDocument::getSomething( const cpo::uno::Sequence< sal_Int8 >& rIdentifier )
{
    if (comphelper::isUnoTunnelId<SdrModel>(rIdentifier))
        return comphelper::getSomething_cast(mpDoc);

    if (comphelper::isUnoTunnelId<SfxObjectShell>(rIdentifier))
        return comphelper::getSomething_cast(mpDocShell);

    return comphelper::getSomethingImpl(rIdentifier, this,
                                        comphelper::FallbackToGetSomethingOf<SfxBaseModel>{});
}

// XTypeProvider
cpo::uno::Sequence< cpo::uno::Type > SdXImpressDocument::getTypes(  )
{
    ::SolarMutexGuard aGuard;

    if( !maTypeSequence.hasElements() )
    {
        cpo::uno::Sequence< cpo::uno::Type > aTypes( SfxBaseModel::getTypes() );
        aTypes = comphelper::concatSequences(aTypes,
            cpo::uno::Sequence {
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XMultiServiceFactory>::get(),
                cppu::UnoType<drawing::XDrawPageDuplicator>::get(),
                cppu::UnoType<drawing::XLayerSupplier>::get(),
                cppu::UnoType<drawing::XMasterPagesSupplier>::get(),
                cppu::UnoType<drawing::XDrawPagesSupplier>::get(),
                cppu::UnoType<document::XLinkTargetSupplier>::get(),
                cppu::UnoType<style::XStyleFamiliesSupplier>::get(),
                cppu::UnoType<css::ucb::XAnyCompareFactory>::get(),
                cppu::UnoType<view::XRenderable>::get() });
        if( mbImpressDoc )
        {
            aTypes = comphelper::concatSequences(aTypes,
                cpo::uno::Sequence {
                    cppu::UnoType<presentation::XPresentationSupplier>::get(),
                    cppu::UnoType<presentation::XCustomPresentationSupplier>::get(),
                    cppu::UnoType<presentation::XHandoutMasterSupplier>::get() });
        }
        maTypeSequence = std::move(aTypes);
    }

    return maTypeSequence;
}

cpo::uno::Sequence< sal_Int8 > SdXImpressDocument::getImplementationId(  )
{
    return cpo::uno::Sequence<sal_Int8>();
}

sal_Int32 SdXImpressDocument::newVectorEpoch()
{
    // Every model counts its versions up from 0, so each draws its own epoch. The clock is mixed
    // in because a fixed random seed makes a restarted kit draw the same number again.
    sal_uInt32 nEpoch = sal_uInt32(comphelper::rng::uniform_int_distribution(1, SAL_MAX_INT32));
    o3tl::hash_combine(nEpoch, std::chrono::steady_clock::now().time_since_epoch().count());
    nEpoch &= SAL_MAX_INT32;
    return nEpoch == 0 ? 1 : sal_Int32(nEpoch);
}

sal_Int32 SdXImpressDocument::getVectorEpoch() const
{
    // The epoch is drawn on first use, so only a model that serves vector content draws from the
    // shared random sequence.
    if (mnVectorEpoch == 0)
        mnVectorEpoch = newVectorEpoch();
    return mnVectorEpoch;
}

/// The key the vector content state of a page is filed under: the page's GUID, which the part id
/// spells out. It names the same page however the page list around it changes.
static const tools::Guid& vectorPartKeyOf(const SdrPage& rPage) { return rPage.GetGuid(); }

sal_uInt64 SdXImpressDocument::getVectorPartVersion(const SdrPage& rPage) const
{
    auto aIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    return aIterator != maVectorParts.end() ? aIterator->second.mnVersion : 0;
}

bool SdXImpressDocument::isVectorObjectChangedSince(const SdrPage& rPage, sal_uInt64 nObjectId,
                                                    sal_uInt64 nSince) const
{
    auto aPartIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    if (aPartIterator == maVectorParts.end())
        return false;
    const auto& rObjectVersions = aPartIterator->second.maObjectChangeVersions;
    auto aObjectIterator = rObjectVersions.find(nObjectId);
    return aObjectIterator != rObjectVersions.end() && aObjectIterator->second > nSince;
}

bool SdXImpressDocument::isVectorOrderChangedSince(const SdrPage& rPage, sal_uInt64 nSince) const
{
    auto aIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    return aIterator != maVectorParts.end()
           && aIterator->second.mnOrderChangeVersion > nSince;
}

std::unordered_set<sal_uInt64> SdXImpressDocument::takeVectorDirtyObjects(const SdrPage& rPage)
{
    auto aIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    if (aIterator == maVectorParts.end())
        return {};

    std::unordered_set<sal_uInt64> aDirty;
    aDirty.swap(aIterator->second.maDirtyObjects);
    return aDirty;
}

bool SdXImpressDocument::recordVectorObjectContent(const SdrPage& rPage, sal_uInt64 nObjectId,
                                                   const VectorObjectContent& rContent)
{
    VectorPartState& rState = maVectorParts[vectorPartKeyOf(rPage)];
    auto aIterator = rState.maObjectContent.find(nObjectId);
    if (aIterator != rState.maObjectContent.end() && aIterator->second == rContent)
        return false;

    rState.maObjectContent[nObjectId] = rContent;
    rState.maObjectChangeVersions[nObjectId] = ++rState.mnVersion;
    return true;
}

const SdXImpressDocument::VectorObjectContent*
SdXImpressDocument::findVectorObjectContent(const SdrPage& rPage, sal_uInt64 nObjectId) const
{
    auto aIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    if (aIterator == maVectorParts.end())
        return nullptr;
    auto aContent = aIterator->second.maObjectContent.find(nObjectId);
    return aContent != aIterator->second.maObjectContent.end() ? &aContent->second : nullptr;
}

void SdXImpressDocument::noteVectorObjectWritten(const SdrPage& rPage, sal_uInt64 nObjectId,
                                                 const VectorObjectContent& rContent)
{
    maVectorParts[vectorPartKeyOf(rPage)].maObjectContent[nObjectId] = rContent;
}

void SdXImpressDocument::forgetVectorObject(const SdrPage& rPage, sal_uInt64 nObjectId)
{
    auto aIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    if (aIterator == maVectorParts.end())
        return;

    aIterator->second.maObjectContent.erase(nObjectId);
    aIterator->second.maObjectChangeVersions.erase(nObjectId);
}

std::vector<sal_uInt64> SdXImpressDocument::getVectorRecordedIds(const SdrPage& rPage) const
{
    auto aIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    if (aIterator == maVectorParts.end())
        return {};

    std::vector<sal_uInt64> aIds;
    aIds.reserve(aIterator->second.maObjectContent.size());
    for (const auto& rRecorded : aIterator->second.maObjectContent)
        aIds.push_back(rRecorded.first);
    return aIds;
}

bool SdXImpressDocument::recordVectorPaintOrder(const SdrPage& rPage,
                                                const std::vector<sal_uInt64>& rOrder)
{
    VectorPartState& rState = maVectorParts[vectorPartKeyOf(rPage)];
    const bool bMoved = rState.maPaintOrder.has_value() && *rState.maPaintOrder != rOrder;
    rState.maPaintOrder = rOrder;
    if (bMoved)
    {
        ++rState.mnVersion;
        rState.mnOrderChangeVersion = rState.mnVersion;
    }
    return bMoved;
}

namespace
{
/// The id of the entry that stands for the page itself. The unique id of an object counts up
/// from 1, so zero names no object, and an object that sits directly on the page already gives
/// zero as its parent.
constexpr sal_Int64 constPageEntryId = 0;

/// The parent an entry names when it sits under nothing. No entry carries this id, so a walk
/// up the parents ends here.
constexpr sal_Int64 constNoParentId = -1;

/// The id of the entry carrying the text of the edit running in the view with the given id.
/// An object's unique id counts up from 1, the page is zero and minus one stands for no parent,
/// so these start below that and collide with none of them. Several views can edit at once,
/// even the same object, so the id comes from the view.
constexpr sal_Int64 textEditEntryId(sal_Int32 nViewId) { return -2 - sal_Int64(nViewId); }
constexpr sal_uInt64 textEditEntryKey(sal_Int32 nViewId)
{
    return sal_uInt64(textEditEntryId(nViewId));
}

/// True for the key of an entry that carries a running text edit. An object's unique id counts
/// up from 1 and the page is zero, so such an entry is the only one with a negative id.
constexpr bool isTextEditEntryKey(sal_uInt64 nKey) { return sal_Int64(nKey) < 0; }

/// The page list a vector-rendering part index addresses.
constexpr sal_Int32 constVectorModeSlides = 0;
constexpr sal_Int32 constVectorModeMasterPages = 1;
constexpr sal_Int32 constVectorModeNotesPages = 2;

/// The views render the changed part from vector primitives only if
/// they asked for them, so each view's callback handler decides itself
/// whether to push a delta. The part is named by its id, the page GUID.
void notifyViewsVectorPartChanged(const SfxObjectShell* pDocShell, const SdrPage& rPage,
                                  sal_Int32 nMode)
{
    const OString aPartId = rPage.GetGuid().getString();
    SfxViewShell* pShell = SfxViewShell::GetFirst(false);
    while (pShell)
    {
        if (pShell->GetObjectShell() == pDocShell)
            pShell->viewVectorPartChanged(aPartId, nMode);
        pShell = SfxViewShell::GetNext(*pShell, false);
    }
}

/// Position of a master page in the master-page list of the standard
/// pages, or -1 when the document does not hold it.
sal_Int32 findMasterPageIndex(SdDrawDocument& rDocument, const SdPage* pMasterPage)
{
    const sal_uInt16 nCount = rDocument.GetMasterSdPageCount(PageKind::Standard);
    for (sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex)
    {
        if (rDocument.GetMasterSdPage(nIndex, PageKind::Standard) == pMasterPage)
            return nIndex;
    }
    return -1;
}

/// True when the master object is not painted behind a slide: a layout prototype such as the
/// title or outline placeholder, or an empty presentation object with neither fill nor line.
/// Both hold for every slide that uses the master.
bool isHiddenBehindSlide(const SdrObject& rObject)
{
    if (rObject.IsNotVisibleAsMaster())
        return true;
    // A page object on a master shows a page in the master's own view only, which is what the
    // office does with it as well.
    if (dynamic_cast<const SdrPageObj*>(&rObject))
        return true;
    if (rObject.IsEmptyPresObj() && !(rObject.HasFillStyle() || rObject.HasLineStyle()))
        return true;
    // A member of a group that is hidden is hidden with it.
    const SdrObject* pParent = rObject.getParentSdrObjectFromSdrObject();
    return pParent && isHiddenBehindSlide(*pParent);
}

/// True when the object, or a member of it when it is a group, holds text with a field. A field
/// resolves against the page being drawn, so the object looks different behind each slide.
bool hasTextField(const SdrObject& rObject)
{
    if (const SdrTextObj* pTextObject = DynCastSdrTextObj(&rObject))
    {
        // A table holds one text per cell, so every text of the object is asked.
        for (sal_Int32 nText = 0; nText < pTextObject->getTextCount(); ++nText)
        {
            const SdrText* pText = pTextObject->getText(nText);
            const OutlinerParaObject* pParaObject
                = pText ? pText->GetOutlinerParaObject() : nullptr;
            if (pParaObject && pParaObject->GetTextObject().HasField())
                return true;
        }
    }
    if (const SdrObjList* pChildren = rObject.GetSubList())
    {
        for (size_t i = 0; i < pChildren->GetObjCount(); ++i)
        {
            const SdrObject* pChild = pChildren->GetObj(i);
            if (pChild && hasTextField(*pChild))
                return true;
        }
    }
    return false;
}

/// True when the master object looks different behind each slide, so each slide carries its
/// own copy of it: the header, footer, date and slide number placeholders, and any object whose
/// text holds a field.
bool isSlideDependent(const SdPage& rMasterPage, SdrObject& rObject)
{
    switch (rMasterPage.GetPresObjKind(&rObject))
    {
        case PresObjKind::Header:
        case PresObjKind::Footer:
        case PresObjKind::DateTime:
        case PresObjKind::SlideNumber:
            return true;
        default:
            return hasTextField(rObject);
    }
}

/// True when the slide shows the master placeholder of that kind. The header, footer, date and
/// slide number are turned on and off per slide.
bool slideShowsPlaceholder(const SdPage& rPage, PresObjKind eKind)
{
    const sd::HeaderFooterSettings& rSettings = rPage.getHeaderFooterSettings();
    switch (eKind)
    {
        case PresObjKind::Header:
            return rSettings.mbHeaderVisible;
        case PresObjKind::Footer:
            return rSettings.mbFooterVisible;
        case PresObjKind::DateTime:
            return rSettings.mbDateTimeVisible;
        case PresObjKind::SlideNumber:
            return rSettings.mbSlideNumberVisible;
        default:
            return true;
    }
}

/// A part index paired with the mode that says which page list the index addresses.
struct VectorPartAddress
{
    sal_Int32 mnPart = 0;
    sal_Int32 mnMode = 0;
};

/// The part and mode a request names the page by, or nothing for a page vector rendering does
/// not serve. A slide and its notes page sit next to each other in the document's own list, so
/// one index names either in its own list. A master page is named by its place in the master
/// list.
std::optional<VectorPartAddress> partAndModeOfPage(const SdPage* pPage)
{
    if (!pPage || pPage->GetPageNum() == 0)
        return {};

    if (pPage->GetPageKind() == PageKind::Notes && !pPage->IsMasterPage())
        return VectorPartAddress{ sal_Int32((pPage->GetPageNum() - 1) / 2),
                                  constVectorModeNotesPages };

    if (pPage->GetPageKind() != PageKind::Standard)
        return {};

    if (!pPage->IsMasterPage())
        return VectorPartAddress{ sal_Int32((pPage->GetPageNum() - 1) / 2),
                                  constVectorModeSlides };

    auto& rDocument = static_cast<SdDrawDocument&>(pPage->getSdrModelFromSdrPage());
    const sal_Int32 nMasterPart = findMasterPageIndex(rDocument, pPage);
    if (nMasterPart >= 0)
        return VectorPartAddress{ nMasterPart, constVectorModeMasterPages };

    return {};
}

/// Count the part's version up and remember it as the version the object set, or the order it
/// paints in, last changed at.
void recordOrderChange(SdXImpressDocument::VectorPartState& rState)
{
    ++rState.mnVersion;
    rState.mnOrderChangeVersion = rState.mnVersion;
}

/// Marks the object and, when it is a group, everything inside it.
void markSubtreeDirty(SdXImpressDocument::VectorPartState& rState, const SdrObject* pObject)
{
    rState.maDirtyObjects.insert(pObject->GetUniqueID());

    if (const SdrObjList* pChildren = pObject->GetSubList())
    {
        for (size_t i = 0; i < pChildren->GetObjCount(); ++i)
        {
            if (const SdrObject* pChild = pChildren->GetObj(i))
                markSubtreeDirty(rState, pChild);
        }
    }
}

/// Forgets what was recorded for the object and everything inside it.
void forgetSubtree(SdXImpressDocument::VectorPartState& rState, const SdrObject* pObject)
{
    rState.maObjectContent.erase(pObject->GetUniqueID());
    rState.maObjectChangeVersions.erase(pObject->GetUniqueID());
    rState.maDirtyObjects.erase(pObject->GetUniqueID());

    if (const SdrObjList* pChildren = pObject->GetSubList())
    {
        for (size_t i = 0; i < pChildren->GetObjCount(); ++i)
        {
            if (const SdrObject* pChild = pChildren->GetObj(i))
                forgetSubtree(rState, pChild);
        }
    }
}

/// Marks every object of the page, and of the master it draws under itself, for a fresh look.
/// Text and lines in the automatic color resolve against the page background, so a change to
/// the page changes how they draw with no hint naming them, and the comparison finds them.
void markPageObjectsDirty(SdXImpressDocument::VectorPartState& rState, const SdrPage& rPage)
{
    for (size_t i = 0; i < rPage.GetObjCount(); ++i)
    {
        if (const SdrObject* pObject = rPage.GetObj(i))
            markSubtreeDirty(rState, pObject);
    }

    if (!rPage.TRG_HasMasterPage())
        return;

    const SdrPage& rMasterPage = rPage.TRG_GetMasterPage();
    for (size_t i = 0; i < rMasterPage.GetObjCount(); ++i)
    {
        if (const SdrObject* pObject = rMasterPage.GetObj(i))
            markSubtreeDirty(rState, pObject);
    }
}

/// Marks the objects a change touched, so the next write looks at them again and counts the
/// part's version up for those that differ. A hint for a group covers the objects inside it,
/// and an object that moves marks the groups above it, whose boxes move with it. An insertion
/// or a removal shows in the paint order, which the next write compares as a whole, so it is
/// the write that counts the version up for it.
void recordObjectChange(SdXImpressDocument::VectorPartState& rState, const SdrObject* pObject,
                        SdrHintKind eKind)
{
    if (eKind == SdrHintKind::ObjectRemoved)
    {
        forgetSubtree(rState, pObject);
        return;
    }

    markSubtreeDirty(rState, pObject);

    for (const SdrObject* pParent = pObject->getParentSdrObjectFromSdrObject(); pParent;
         pParent = pParent->getParentSdrObjectFromSdrObject())
        rState.maDirtyObjects.insert(pParent->GetUniqueID());
}

/// Marks the page entry for comparison. The next write compares it against what was written
/// last and counts the version up only when it differs, so a property set to its current value
/// is not sent.
void recordMasterChange(SdXImpressDocument::VectorPartState& rState)
{
    rState.mbPageDirty = true;
}

// A slide's presentation info changed, so tell every view of the document.
// The payload names the reason and the changed part.
void notifyViewsPresentationInfoChanged(const SfxObjectShell* pDocShell, sal_Int32 nPart)
{
    if (!comphelper::COKit::isActive() || !pDocShell)
        return;

    ::tools::JsonWriter aWriter;
    aWriter.put("reason", "gifupdate");
    aWriter.put("part", nPart);
    const OString aPayload = aWriter.finishAndGetAsOString();

    SfxViewShell* pShell = SfxViewShell::GetFirst(false);
    while (pShell)
    {
        if (pShell->GetObjectShell() == pDocShell)
            pShell->viewCallback(COKitCallbackType::PRESENTATION_INFO, aPayload);
        pShell = SfxViewShell::GetNext(*pShell, false);
    }
}

/// The notes page of a slide shows the slide in its page object, so a change on the slide is a
/// change of that object, recorded on the part of the notes page. The write that follows compares
/// the object and counts the version up only when what it shows really differs.
void recordSlidePreviewChange(
    SdDrawDocument& rDocument, const SfxObjectShell* pDocShell,
    std::unordered_map<tools::Guid, SdXImpressDocument::VectorPartState>& rVectorParts,
    sal_Int32 nSlide)
{
    if (nSlide < 0 || nSlide >= rDocument.GetSdPageCount(PageKind::Notes))
        return;

    SdPage* pNotesPage = rDocument.GetSdPage(sal_uInt16(nSlide), PageKind::Notes);
    const SdPage* pSlide = rDocument.GetSdPage(sal_uInt16(nSlide), PageKind::Standard);
    if (!pNotesPage || !pSlide)
        return;

    // Every page object on the notes page that shows the slide, whether or not it is the one
    // the layout placed there.
    bool bShown = false;
    for (size_t i = 0; i < pNotesPage->GetObjCount(); ++i)
    {
        const auto* pPageObject = dynamic_cast<const SdrPageObj*>(pNotesPage->GetObj(i));
        if (!pPageObject || pPageObject->GetReferencedPage() != pSlide)
            continue;
        rVectorParts[vectorPartKeyOf(*pNotesPage)].maDirtyObjects.insert(
            pPageObject->GetUniqueID());
        bShown = true;
    }
    if (bShown)
        notifyViewsVectorPartChanged(pDocShell, *pNotesPage, constVectorModeNotesPages);
}

/// A master change shows on every page that uses the master, so raise those pages' versions,
/// remember it and tell the views. A slide master serves the slides and a notes master the
/// notes pages. A master's page number is a place in the master list, so it names no page.
void bumpMasterChangeForUsers(
    SdDrawDocument& rDocument, const SfxObjectShell* pDocShell,
    std::unordered_map<tools::Guid, SdXImpressDocument::VectorPartState>& rVectorParts,
    const SdPage* pMasterPage, const SdrObject* pChangedObject = nullptr)
{
    const PageKind ePageKind = pMasterPage->GetPageKind();
    if (ePageKind != PageKind::Standard && ePageKind != PageKind::Notes)
        return;
    const sal_Int32 nMode
        = ePageKind == PageKind::Standard ? constVectorModeSlides : constVectorModeNotesPages;

    const sal_uInt16 nPageCount = rDocument.GetSdPageCount(ePageKind);
    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        const SdPage* pUserPage = rDocument.GetSdPage(nPage, ePageKind);
        if (pUserPage && pUserPage->TRG_HasMasterPage()
            && &pUserPage->TRG_GetMasterPage() == pMasterPage)
        {
            SdXImpressDocument::VectorPartState& rState
                = rVectorParts[vectorPartKeyOf(*pUserPage)];
            if (pChangedObject)
            {
                // A slide draws its master from the master part, so a changed master object
                // reaches it through that part. What the slide carries of the master is its
                // own copy of each placeholder that reads differently on each slide, so the
                // changed object is looked at again on the slide and the version moves only
                // when such a copy changed. A notes page still carries its master inline, so
                // its version moves outright.
                if (nMode != constVectorModeSlides)
                    recordMasterChange(rState);
                markSubtreeDirty(rState, pChangedObject);
            }
            else
            {
                // The master's own properties changed. Its background travels with the page
                // entry of every page that uses it, and the automatic color resolves against
                // that background.
                recordMasterChange(rState);
                markPageObjectsDirty(rState, *pUserPage);
            }
            notifyViewsVectorPartChanged(pDocShell, *pUserPage, nMode);
            if (nMode == constVectorModeSlides)
                recordSlidePreviewChange(rDocument, pDocShell, rVectorParts, nPage);
        }
    }
}
}

void SdXImpressDocument::notifyTextEditChanged(const SdrObject& rEdited)
{
    if (!mpDoc || !mpDocShell)
        return;

    const SdPage* pPage = dynamic_cast<const SdPage*>(rEdited.getSdrPageFromSdrObject());
    const auto oPart = partAndModeOfPage(pPage);
    if (!oPart)
        return;

    // The write that follows looks at the text and counts the version up when it moved, which
    // is what makes the client take the delta rather than drop it as one it already holds.
    notifyViewsVectorPartChanged(mpDocShell, *pPage, oPart->mnMode);
}

bool SdXImpressDocument::takeVectorPageDirty(const SdrPage& rPage)
{
    auto aIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    if (aIterator == maVectorParts.end())
        return true;
    VectorPartState& rState = aIterator->second;
    const bool bDirty = rState.mbPageDirty || !rState.moPageContent;
    rState.mbPageDirty = false;
    return bDirty;
}

bool SdXImpressDocument::recordVectorPageContent(const SdrPage& rPage,
                                                 const VectorObjectContent& rContent,
                                                 const OString& rMasterPartId,
                                                 const std::vector<sal_Int32>& rMasterHiddenLayers)
{
    VectorPartState& rState = maVectorParts[vectorPartKeyOf(rPage)];
    const bool bMoved = rState.moPageContent
                        && (!(*rState.moPageContent == rContent)
                            || rState.maPageMasterPartId != rMasterPartId
                            || rState.maPageMasterHiddenLayers != rMasterHiddenLayers);
    rState.moPageContent = rContent;
    rState.maPageMasterPartId = rMasterPartId;
    rState.maPageMasterHiddenLayers = rMasterHiddenLayers;
    if (bMoved)
    {
        ++rState.mnVersion;
        rState.mnMasterChangeVersion = rState.mnVersion;
    }
    return bMoved;
}

bool SdXImpressDocument::isVectorMasterChangedSince(const SdrPage& rPage, sal_uInt64 nSince) const
{
    auto aIterator = maVectorParts.find(vectorPartKeyOf(rPage));
    return aIterator != maVectorParts.end() && aIterator->second.mnMasterChangeVersion > nSince;
}

void SdXImpressDocument::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( mpDoc )
    {
        if (rHint.GetId() == SfxHintId::ThisIsAnSdrHint)
        {
            const SdrHint* pSdrHint = static_cast<const SdrHint*>( &rHint );
            if( hasEventListeners() )
            {
                document::EventObject aEvent;
                if( SvxUnoDrawMSFactory::createEvent( mpDoc, pSdrHint, aEvent ) )
                    notifyEvent( aEvent );
            }

            // A changed, inserted or removed object changes the vector
            // content of its slide, so count that slide's version up.
            const SdrHintKind eKind = pSdrHint->GetKind();
            if (eKind == SdrHintKind::ObjectChange
                || eKind == SdrHintKind::ObjectInserted
                || eKind == SdrHintKind::ObjectRemoved)
            {
                if (const SdrObject* pObject = pSdrHint->GetObject())
                {
                    const SdPage* pPage
                        = dynamic_cast<const SdPage*>(pObject->getSdrPageFromSdrObject());
                    if (pPage && pPage->IsMasterPage())
                    {
                        // The master shows on every page that uses it, and a
                        // slide master is a part of its own in master view.
                        bumpMasterChangeForUsers(*mpDoc, mpDocShell, maVectorParts, pPage,
                                                 pObject);

                        if (pPage->GetPageKind() == PageKind::Standard)
                        {
                            recordObjectChange(maVectorParts[vectorPartKeyOf(*pPage)], pObject,
                                               eKind);
                            notifyViewsVectorPartChanged(mpDocShell, *pPage,
                                                         constVectorModeMasterPages);
                        }
                    }
                    else if (const auto oPart = partAndModeOfPage(pPage))
                    {
                        // A page keeps the content its source gave it until an edit, and
                        // this change is one, so the slide becomes the document's own.
                        if (oPart->mnMode == constVectorModeSlides)
                            sd::SlideLink::BreakOnEdit(*mpDoc, oPart->mnPart);

                        recordObjectChange(maVectorParts[vectorPartKeyOf(*pPage)], pObject, eKind);
                        notifyViewsVectorPartChanged(mpDocShell, *pPage, oPart->mnMode);

                        // An animated image came, went or moved, so
                        // re-send the presentation info. A move arrives
                        // coalesced, and only the slides have such info.
                        if (oPart->mnMode == constVectorModeSlides)
                        {
                            recordSlidePreviewChange(*mpDoc, mpDocShell, maVectorParts,
                                                     oPart->mnPart);
                            if (const SdrGrafObj* pGraphicObject
                                    = dynamic_cast<const SdrGrafObj*>(pObject))
                            {
                                if (pGraphicObject->GetGraphic().IsAnimated())
                                    notifyViewsPresentationInfoChanged(mpDocShell, oPart->mnPart);
                            }
                        }
                    }
                }
            }
            else if (eKind == SdrHintKind::BeginEdit || eKind == SdrHintKind::EndEdit)
            {
                // The entry carrying the text comes and goes with the edit, so the set of
                // entries moved, and the object it runs on starts and stops hiding its own
                // text. An edit that changes no text says nothing else, so both are noted here.
                if (const SdrObject* pObject = pSdrHint->GetObject())
                {
                    const SdPage* pPage
                        = dynamic_cast<const SdPage*>(pObject->getSdrPageFromSdrObject());
                    if (const auto oPart = partAndModeOfPage(pPage))
                    {
                        SdXImpressDocument::VectorPartState& rState
                            = maVectorParts[vectorPartKeyOf(*pPage)];
                        recordOrderChange(rState);
                        recordObjectChange(rState, pObject, SdrHintKind::ObjectChange);
                        notifyViewsVectorPartChanged(mpDocShell, *pPage, oPart->mnMode);
                    }
                }
            }
            else if (eKind == SdrHintKind::PageOrderChange)
            {
                // This hint also announces a change to the page's own
                // properties, such as its background fill. The background
                // travels with the master page content, so it is
                // remembered as a master change.
                const SdPage* pPage = dynamic_cast<const SdPage*>(pSdrHint->GetPage());
                if (pPage && !pPage->IsInserted())
                {
                    // The page left the document, so its recorded content is dropped. The
                    // version and the push mark stay, because an undo or a redo brings the same
                    // page back under the same id and its versions must keep counting from
                    // where they were.
                    const auto aIterator = maVectorParts.find(vectorPartKeyOf(*pPage));
                    if (aIterator != maVectorParts.end())
                    {
                        SdXImpressDocument::VectorPartState aKept;
                        aKept.mnVersion = aIterator->second.mnVersion;
                        aKept.mnLastSentVersion = aIterator->second.mnLastSentVersion;
                        aKept.mbServed = aIterator->second.mbServed;
                        aIterator->second = std::move(aKept);
                    }
                }
                else if (pPage && pPage->IsMasterPage())
                {
                    bumpMasterChangeForUsers(*mpDoc, mpDocShell, maVectorParts, pPage);

                    // In master view the master is the page itself, so its
                    // background is part of its own master-page content.
                    if (pPage->GetPageKind() == PageKind::Standard)
                    {
                        SdXImpressDocument::VectorPartState& rState
                            = maVectorParts[vectorPartKeyOf(*pPage)];
                        recordMasterChange(rState);
                        markPageObjectsDirty(rState, *pPage);
                        notifyViewsVectorPartChanged(mpDocShell, *pPage,
                                                     constVectorModeMasterPages);
                    }
                }
                else if (const auto oPart = partAndModeOfPage(pPage))
                {
                    // The background changed, and with it what the automatic color of the
                    // objects resolves against.
                    SdXImpressDocument::VectorPartState& rState
                        = maVectorParts[vectorPartKeyOf(*pPage)];
                    recordMasterChange(rState);
                    markPageObjectsDirty(rState, *pPage);
                    notifyViewsVectorPartChanged(mpDocShell, *pPage, oPart->mnMode);
                    if (oPart->mnMode == constVectorModeSlides)
                        recordSlidePreviewChange(*mpDoc, mpDocShell, maVectorParts, oPart->mnPart);
                }
            }

            if( pSdrHint->GetKind() == SdrHintKind::ModelCleared )
            {
                if( mpDoc )
                    EndListening( *mpDoc );
                mpDoc = nullptr;
                mpDocShell = nullptr;
            }
        }
        else
        {
            // did our SdDrawDocument just died?
            if(rHint.GetId() == SfxHintId::Dying)
            {
                // yes, so we ask for a new one
                if( mpDocShell )
                {
                    SdDrawDocument *pNewDoc = mpDocShell->GetDoc();

                    // is there a new one?
                    if( pNewDoc != mpDoc )
                    {
                        mpDoc = pNewDoc;
                        if(mpDoc)
                            StartListening( *mpDoc );
                    }
                }
            }
        }
    }
    SfxBaseModel::Notify( rBC, rHint );
}

namespace
{
/// Serializes a Draw or Impress page to JSON primitives. The page is named by its part id, the
/// page GUID, in the page list nMode names.
class VectorContentWriter
{
    static constexpr double constTwipConversionFactor
        = o3tl::convert(1.0, o3tl::Length::mm100, o3tl::Length::twip);

public:
    VectorContentWriter(SdDrawDocument* pDocument, SdXImpressDocument* pModel, OString aPartId,
                        sal_Int32 nMode = constVectorModeSlides)
        : mpDocument(pDocument)
        , mpModel(pModel)
        , maPartId(std::move(aPartId))
        , mnMode(nMode)
    {
    }

    /// A non-negative since-version asks for a delta against that version rather than the full
    /// page. Set before writing.
    void setSinceVersion(sal_Int64 nSinceVersion) { mnSinceVersion = nSinceVersion; }

    bool isDelta() const { return mnSinceVersion >= 0; }

    /// True when the delta is pushed rather than pulled. A push with nothing changed writes no
    /// document. A pull always writes the header.
    void setPush(bool bPush) { mbPush = bPush; }

    /// The page the request names, or nullptr when the document holds no such page or the mode
    /// is one this writer does not serve. Resolved once, and later calls return the same page.
    SdPage* resolvePage()
    {
        if (!mbResolved)
        {
            mpPage = resolveCurrentPage();
            mbResolved = true;
        }
        return mpPage;
    }

    void write(tools::JsonWriter& rWriter)
    {
        SdPage* pPage = resolvePage();
        if (!pPage)
        {
            // A push writes nothing for a page that is not there. A pull gets the header
            // alone.
            if (!isDelta() || !mbPush)
                writeMissingPage(rWriter);
            return;
        }

        setupProcessor(rWriter, pPage);

        // A change only asked for a fresh look at the objects it touched. Comparing them against
        // what was written last is what decides whether the part's version moves at all, so it
        // happens before the version is reported.
        resolveDirtyObjects(pPage);
        resolvePageContent(pPage);
        resolveTextEditEntry(pPage);

        // A trigger only says that something may have changed. When the comparison finds
        // nothing changed, a push writes nothing at all and a pull is answered with the header
        // alone.
        if (isDelta() && mbPush && !hasContentToSend(pPage))
            return;

        writeHeader(rWriter, pPage);

        // The order names every live object on the part, so it travels only when that set or the
        // order it paints in moved. A client that already holds the order keeps it.
        if (isDelta() && mpModel->isVectorOrderChangedSince(*pPage, sal_uInt64(mnSinceVersion)))
            writeObjectOrder(rWriter, pPage);

        // The page is the first painted object: its entry carries the background, the page
        // fill and the master page content, and the objects on the page follow it. A delta
        // carries the page entry whenever that content changed after the client's version.
        auto aObjectsArray = rWriter.startArray("objects");
        if (!isDelta() || mpModel->isVectorMasterChangedSince(*pPage, sal_uInt64(mnSinceVersion)))
            writePageEntry(rWriter, pPage);
        writePageObjects(rWriter, pPage);

        // The text of a running edit is the last entry, so it draws over the object it runs on.
        // It changes with every keystroke, so a delta always carries it while an edit runs.
        writeTextEditEntry(rWriter, pPage);
    }

private:
    /// Answer a request for a page the document does not hold with the header alone. A client
    /// whose page list has not caught up with a mode switch asks for a part that is not there,
    /// and an answer it can read lets it drop the request and ask again.
    void writeMissingPage(tools::JsonWriter& rWriter)
    {
        writeCommonHeader(rWriter, "vectorprimitives", maPartId);
    }

    /// The fields every payload opens with: its type, the mode and the id of the page it
    /// describes, and the epoch its versions count in.
    void writeCommonHeader(tools::JsonWriter& rWriter, std::string_view aType,
                           std::string_view aPartId)
    {
        rWriter.put("type", aType);
        rWriter.put("mode", mnMode);
        if (!aPartId.empty())
            rWriter.put("partId", aPartId);
        rWriter.put("epoch", mpModel->getVectorEpoch());
    }

    /// The page the part id names, in the list the mode names. A slide is served in the slides
    /// mode, a notes page in the notes mode and a slide master in the master mode. A page id
    /// that names any other page, or a page of another kind than the mode asks for, names
    /// nothing.
    SdPage* resolvePageById()
    {
        const tools::Guid aGuid(std::string_view(maPartId.getStr(), maPartId.getLength()));
        if (aGuid.isEmpty())
            return nullptr;
        SdPage* pPage = mpDocument->GetPageByGuid(aGuid);
        const auto oAddress = partAndModeOfPage(pPage);
        if (!oAddress || oAddress->mnMode != mnMode)
            return nullptr;
        return pPage;
    }

    /// The page the request names, or nullptr for a mode this writer does not serve or a page id
    /// that names no page of that list.
    SdPage* resolveCurrentPage()
    {
        if (mnMode != constVectorModeSlides && mnMode != constVectorModeMasterPages
            && mnMode != constVectorModeNotesPages)
            return nullptr;
        return resolvePageById();
    }

    void writeHeader(tools::JsonWriter& rWriter, SdPage* pPage)
    {
        writeCommonHeader(rWriter, isDelta() ? "vectorprimitivesdelta" : "vectorprimitives",
                          pPage->GetGuid().getString());

        // A delta names the version it was compared against and the version it brings the part
        // to. Only the objects that moved between those two versions are in it.
        if (isDelta())
            rWriter.put("from", mnSinceVersion);

        rWriter.put("version", sal_Int64(mpModel->getVectorPartVersion(*pPage)));
    }

    void setupProcessor(tools::JsonWriter& rWriter, SdPage* pPage)
    {
        maProcessor.emplace(rWriter);
        maProcessor->setScaleFactor(constTwipConversionFactor);
        maProcessor->setBitmapCache(mpModel->getBitmapCache());
        maProcessor->setFontCache(mpModel->getVectorFontCache(), mpModel->getVectorFontFaceByKey());

        // Set up ViewInformation2D with visualized page
        drawinglayer::geometry::ViewInformation2D aViewInfo;
        cpo::uno::Reference<css::drawing::XDrawPage> xDrawPage(pPage->getUnoPage());
        if (xDrawPage.is())
            aViewInfo.setVisualizedPage(xDrawPage);

        // Text and lines with the automatic color resolve against what lies behind them. On a
        // page that is the page background, the master page's when the page defines none.
        aViewInfo.setAutoColor(pPage->GetPageBackgroundColor());

        // Some content is kept for an editing view alone: the prompt of an empty placeholder
        // and the stand-in graphic of an empty picture. The drawing layer empties the wrapper
        // holding it unless the output says it is such a view, and the payload serves one, so
        // the content travels.
        aViewInfo.setEditViewActive(true);

        // While a text edit runs, the object it runs on hides its own text and the entry for the
        // edit carries what has been typed so far. Saying the edit is active is what makes the
        // object hide it, so the text is not drawn twice.
        aViewInfo.setTextEditActive(anyViewIsEditing(pPage));

        maViewInformation = aViewInfo;
        maProcessor->setViewInformation2D(aViewInfo);
    }

    /// The entry that stands for the page itself. It is drawn first and holds what lies
    /// behind the objects. Its id is zero, which no object takes, and its box is the page.
    ///
    /// A slide carries its own background and page fill and names the master part it draws
    /// under itself, so the master travels once rather than as a copy on every slide. The
    /// master objects that look different behind each slide, the header, footer, date and slide
    /// number and anything holding a field, are objects of the slide, rendered for it. Any other
    /// page carries the master content inline, since its master is not served as a part.
    void writePageEntry(tools::JsonWriter& rWriter, SdPage* pPage)
    {
        auto pPageNode = rWriter.startStruct();
        rWriter.put("id", constPageEntryId);
        rWriter.put("name", pPage->GetName());
        // An object on the page gives zero as its parent, which is this entry, so it reads as
        // a child of the page. The page sits under nothing and names no parent of its own, so
        // a walk up the parents of an entry ends at it.
        rWriter.put("parent", constNoParentId);
        rWriter.put("kind", "page");
        const sal_Int64 nWidth = sal_Int64(pPage->GetWidth() * constTwipConversionFactor);
        const sal_Int64 nHeight = sal_Int64(pPage->GetHeight() * constTwipConversionFactor);
        rWriter.put("x", sal_Int64(0));
        rWriter.put("y", sal_Int64(0));
        rWriter.put("width", nWidth);
        rWriter.put("height", nHeight);
        {
            auto aTransformArray = rWriter.startArray("transform");
            rWriter.putSimpleValue(double(nWidth));
            rWriter.putSimpleValue(0.0);
            rWriter.putSimpleValue(0.0);
            rWriter.putSimpleValue(double(nHeight));
            rWriter.putSimpleValue(0.0);
            rWriter.putSimpleValue(0.0);
        }

        const drawinglayer::primitive2d::Primitive2DContainer& rContent
            = pageContent(pPage).maPrimitives;
        if (!maPageMasterPartId.isEmpty())
        {
            rWriter.put("masterPartId", maPageMasterPartId);
            // The master layers this page hides. The document keeps them as the set of master
            // layers the page shows.
            if (!maPageMasterHiddenLayers.empty())
            {
                auto aLayerArray = rWriter.startArray("masterHiddenLayers");
                for (const sal_Int32 nLayer : maPageMasterHiddenLayers)
                    rWriter.putSimpleValue(nLayer);
            }
        }
        auto aPrimArray = rWriter.startArray("primitives");
        if (!rContent.empty())
            maProcessor->decomposeAndWrite(rContent);
    }

    /// What the page entry carries behind the objects, built once per write: the page's own
    /// background and fill, and the master content inline for a page whose master is not
    /// served as a part. The master a slide draws under is remembered by its part id.
    const SdXImpressDocument::VectorObjectContent& pageContent(SdPage* pPage)
    {
        if (!moPageContent)
        {
            SdXImpressDocument::VectorObjectContent aContent;
            if (const SdPage* pMasterPage = servedMasterOf(pPage))
            {
                pageOwnPrimitives(pPage, aContent.maPrimitives);
                maPageMasterPartId = pMasterPage->GetGuid().getString();
                maPageMasterHiddenLayers = hiddenMasterLayersOf(pPage);
            }
            else
                pageContentPrimitives(pPage, aContent.maPrimitives);

            decomposeForComparison(aContent);
            aContent.maPaintedBox
                = tools::Rectangle(Point(0, 0), Size(pPage->GetWidth(), pPage->GetHeight()));
            moPageContent = std::move(aContent);
        }
        return *moPageContent;
    }

    /// Fills in what the primitives of the content decompose to, which is what is compared. A
    /// primitive that decomposes to nothing is a leaf, a fill or a background color, and is
    /// drawn itself, so it is compared itself.
    void decomposeForComparison(SdXImpressDocument::VectorObjectContent& rContent) const
    {
        for (const auto& rPrimitive : rContent.maPrimitives)
        {
            // A page preview shows another page, and a field inside it reads against that page
            // rather than against the one being drawn, so the preview names the page it shows
            // as the visualized one.
            drawinglayer::geometry::ViewInformation2D aViewInformation(maViewInformation);
            if (const auto* pPreview
                = dynamic_cast<const drawinglayer::primitive2d::PagePreviewPrimitive2D*>(
                    rPrimitive.get()))
                aViewInformation.setVisualizedPage(pPreview->getXDrawPage());

            drawinglayer::primitive2d::Primitive2DContainer aDecomposed;
            rPrimitive->get2DDecomposition(aDecomposed, aViewInformation);
            if (aDecomposed.empty())
                rContent.maDrawn.push_back(rPrimitive);
            else
                rContent.maDrawn.append(std::move(aDecomposed));
        }
    }

    /// Compares the page entry when a change marked it, or for a full response, and counts the
    /// part's version up only when it differs from what was written last.
    void resolvePageContent(SdPage* pPage)
    {
        // A full response also repairs a page the client holds, so the entry is compared again.
        const bool bDirty = mpModel->takeVectorPageDirty(*pPage);
        if (!bDirty && isDelta())
            return;
        const SdXImpressDocument::VectorObjectContent& rContent = pageContent(pPage);
        mpModel->recordVectorPageContent(*pPage, rContent, maPageMasterPartId,
                                         maPageMasterHiddenLayers);
    }

    /// The ids of the master layers the page does not show. Turning the master background or
    /// the master objects off for a page removes their layer from the set the page shows.
    std::vector<sal_Int32> hiddenMasterLayersOf(const SdPage* pPage) const
    {
        std::vector<sal_Int32> aHidden;
        if (!pPage->TRG_HasMasterPage())
            return aHidden;
        const SdrLayerIDSet& rVisible = pPage->TRG_GetMasterPageVisibleLayers();
        const SdrLayerAdmin& rLayerAdmin = mpDocument->GetLayerAdmin();
        for (sal_uInt16 nLayerPosition = 0; nLayerPosition < rLayerAdmin.GetLayerCount();
             ++nLayerPosition)
        {
            const SdrLayerID nId = rLayerAdmin.GetLayer(nLayerPosition)->GetID();
            if (!rVisible.IsSet(nId))
                aHidden.push_back(sal_Int32(nId.get()));
        }
        return aHidden;
    }

    /// True when the object is drawn behind the page the part stands for rather than being an
    /// object of that page. A slide's copy of a master placeholder is such an object: it lives
    /// on the master and the slide fills it in.
    bool isBehindThePage(const SdrObject& rObject) const
    {
        if (mnMode == constVectorModeMasterPages)
            return false;
        const SdPage* pObjectPage
            = dynamic_cast<const SdPage*>(rObject.getSdrPageFromSdrObject());
        return pObjectPage && pObjectPage->IsMasterPage();
    }

    /// The master a slide draws under itself, served as a part of its own, or nullptr for a
    /// page whose master is not served as a part: a master page, a notes page, or a page
    /// without a master.
    static const SdPage* servedMasterOf(SdPage* pPage)
    {
        if (pPage->IsMasterPage() || pPage->GetPageKind() != PageKind::Standard
            || !pPage->TRG_HasMasterPage())
            return nullptr;
        return dynamic_cast<const SdPage*>(&pPage->TRG_GetMasterPage());
    }

    /// The parts of the page that belong to it and lie behind its objects, in paint order: the
    /// background, the page fill and the background the master defines.
    static void pageOwnPrimitives(SdPage* pPage,
                                  drawinglayer::primitive2d::Primitive2DContainer& rContent)
    {
        // ViewContactOfSdrPage fixed child order:
        //   0=Background, 1=Shadow, 2=Fill, 3=MasterPage

        // PageBackground: emits a BackgroundColorPrimitive2D with the
        // configured DOCCOLOR. Drawn first so the slide fill paints
        // over it.
        pPage->GetViewContact().GetViewContact(0).getViewIndependentPrimitive2DContainer(rContent);

        // PageFill: always produces a solid fill for the slide background
        pPage->GetViewContact().GetViewContact(2).getViewIndependentPrimitive2DContainer(rContent);

        // MasterPageDescriptor: adds a background fill if the master page defines one. The
        // master background is drawn only while the page shows the background layer of its
        // master, which "Display Master Background" toggles.
        const SdrLayerID nBackgroundLayer
            = pPage->getSdrModelFromSdrPage().GetLayerAdmin().GetLayerID(sUNO_LayerName_background);
        if (pPage->TRG_HasMasterPage()
            && pPage->TRG_GetMasterPageVisibleLayers().IsSet(nBackgroundLayer))
            pPage->GetViewContact().GetViewContact(3).getViewIndependentPrimitive2DContainer(
                rContent);
    }

    /// What lies behind the objects of a page, in paint order: the background, the page fill
    /// and the master page content, with the master placeholders the page does not show left
    /// out.
    static void pageContentPrimitives(SdPage* pPage,
                                      drawinglayer::primitive2d::Primitive2DContainer& rContent)
    {
        pageOwnPrimitives(pPage, rContent);
        if (!pPage->TRG_HasMasterPage())
            return;

        SdPage* pMasterPage = dynamic_cast<SdPage*>(&pPage->TRG_GetMasterPage());
        if (!pMasterPage)
            return;

        // A page shows the master objects on the layers it keeps of its master, which
        // "Display Master Objects" toggles.
        const SdrLayerIDSet& rVisible = pPage->TRG_GetMasterPageVisibleLayers();
        for (size_t i = 0; i < pMasterPage->GetObjCount(); ++i)
        {
            SdrObject* pObject = pMasterPage->GetObj(i);
            if (!pObject || isHiddenBehindSlide(*pObject) || !rVisible.IsSet(pObject->GetLayer())
                || !slideShowsPlaceholder(*pPage, pMasterPage->GetPresObjKind(pObject)))
                continue;

            pObject->GetViewContact().getViewIndependentPrimitive2DContainer(rContent);
        }
    }

    /// What a page object shows: the page it stands for, drawn as it presents and fitted into
    /// the object's box. The drawing layer decomposes such an object to a yellow outline that
    /// stands in for the page, so the content is built from the page here.
    static drawinglayer::primitive2d::Primitive2DContainer
    pagePreviewContent(const SdrPageObj& rPageObject)
    {
        drawinglayer::primitive2d::Primitive2DContainer aPreview;
        SdPage* pShown = dynamic_cast<SdPage*>(rPageObject.GetReferencedPage());
        if (!pShown)
            return aPreview;

        drawinglayer::primitive2d::Primitive2DContainer aPageContent;
        pageContentPrimitives(pShown, aPageContent);
        for (size_t i = 0; i < pShown->GetObjCount(); ++i)
        {
            if (SdrObject* pObject = pShown->GetObj(i))
                pObject->GetViewContact().getViewIndependentPrimitive2DContainer(aPageContent);
        }
        if (aPageContent.empty())
            return aPreview;

        const basegfx::B2DRange aBox
            = vcl::unotools::b2DRectangleFromRectangle(rPageObject.GetCurrentBoundRect());
        basegfx::B2DHomMatrix aTransform;
        aTransform.set(0, 0, aBox.getWidth());
        aTransform.set(1, 1, aBox.getHeight());
        aTransform.set(0, 2, aBox.getMinX());
        aTransform.set(1, 2, aBox.getMinY());

        aPreview.push_back(new drawinglayer::primitive2d::PagePreviewPrimitive2D(
            GetXDrawPageForSdrPage(pShown), aTransform, double(pShown->GetWidth()),
            double(pShown->GetHeight()), std::move(aPageContent)));

        // The office frames a page object in the color it draws the boundaries of a document
        // in, so the preview reads as a page.
        const Color aFrameColor(
            svtools::ColorConfig().GetColorValue(svtools::DOCBOUNDARIES).nColor);
        basegfx::B2DPolygon aOutline(basegfx::utils::createUnitPolygon());
        aOutline.transform(aTransform);
        aPreview.push_back(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
            std::move(aOutline), aFrameColor.getBColor()));
        return aPreview;
    }

    /// Every object the list paints, in paint order: each object followed by the objects
    /// inside it when it is a group, depth first.
    static void collectPaintedObjects(const SdrObjList& rList, std::vector<SdrObject*>& rObjects)
    {
        for (size_t i = 0; i < rList.GetObjCount(); ++i)
        {
            SdrObject* pObject = rList.GetObj(i);
            if (!pObject)
                continue;
            rObjects.push_back(pObject);
            if (const SdrObjList* pChildren = pObject->GetSubList())
                collectPaintedObjects(*pChildren, rObjects);
        }
    }

    /// Every object the part paints, in paint order. A slide that names a master part paints
    /// the master's placeholders it fills in, the header, the footer, the date and the slide
    /// number, so those come first as objects of the slide, rendered for it. Any other page
    /// paints its own objects alone.
    static void collectPartObjects(SdPage* pPage, std::vector<SdrObject*>& rObjects)
    {
        if (servedMasterOf(pPage))
        {
            SdPage* pMasterPage = dynamic_cast<SdPage*>(&pPage->TRG_GetMasterPage());
            for (size_t i = 0; pMasterPage && i < pMasterPage->GetObjCount(); ++i)
            {
                SdrObject* pObject = pMasterPage->GetObj(i);
                if (!pObject || isHiddenBehindSlide(*pObject)
                    || !isSlideDependent(*pMasterPage, *pObject)
                    || !slideShowsPlaceholder(*pPage, pMasterPage->GetPresObjKind(pObject)))
                    continue;
                // A group is slide dependent through a member that holds a field, and the
                // master names every member of such a group as the slide's to supply, so the
                // slide carries the members along with the group.
                rObjects.push_back(pObject);
                if (const SdrObjList* pChildren = pObject->GetSubList())
                    collectPaintedObjects(*pChildren, rObjects);
            }
        }

        collectPaintedObjects(*pPage, rObjects);
    }

    /// One entry per view of the document, saying which of them is running a text edit on the
    /// page. A view that is not editing it is named too, so what it last had recorded can be
    /// forgotten.
    struct EditingView
    {
        sal_Int32 mnViewId = -1;
        SdrObjEditView* mpView = nullptr;
    };

    /// The painted objects of the part, collected once per write.
    const std::vector<SdrObject*>& partObjects(SdPage* pPage)
    {
        if (!moPartObjects)
        {
            moPartObjects.emplace();
            collectPartObjects(pPage, *moPartObjects);
        }
        return *moPartObjects;
    }

    /// The views of the document and the edit each runs on this page, collected once per write.
    const std::vector<EditingView>& views(SdPage* pPage)
    {
        if (!moViews)
            moViews = viewsOfDocument(pPage);
        return *moViews;
    }

    /// The order array lists every live object id on the page in paint order: the page entry
    /// first, then each object with the objects inside a group right after the group. It is
    /// the authoritative object set and ordering for the part.
    void writeObjectOrder(tools::JsonWriter& rWriter, SdPage* pPage)
    {
        const std::vector<SdrObject*>& rObjects = partObjects(pPage);

        auto aOrderArray = rWriter.startArray("order");
        rWriter.putSimpleValue(constPageEntryId);
        for (const SdrObject* pObject : rObjects)
            rWriter.putSimpleValue(sal_Int64(pObject->GetUniqueID()));

        // The text of a running edit is last, so it draws over the object it runs on. Several
        // views can be editing at once, so there is one entry for each of them.
        for (const EditingView& rView : views(pPage))
        {
            if (rView.mpView)
                rWriter.putSimpleValue(textEditEntryId(rView.mnViewId));
        }
    }

    std::vector<EditingView> viewsOfDocument(SdPage* pPage) const
    {
        std::vector<EditingView> aViews;
        if (!mpModel->GetDocShell())
            return aViews;

        SfxViewShell* pShell = SfxViewShell::GetFirst(false);
        while (pShell)
        {
            if (pShell->GetObjectShell() == mpModel->GetDocShell())
            {
                EditingView aView;
                aView.mnViewId = sal_Int32(pShell->GetViewShellId().get());

                // The main view shell is the one that edits the slide. An edit in the outline
                // or the notes pane of the same base is not mirrored here.
                ::sd::ViewShellBase* pBase = dynamic_cast<::sd::ViewShellBase*>(pShell);
                std::shared_ptr<::sd::ViewShell> pViewShell
                    = pBase ? pBase->GetMainViewShell() : nullptr;
                SdrObjEditView* pView = pViewShell ? pViewShell->GetView() : nullptr;

                // Only an edit running on this page belongs in this page's payload.
                if (pView && pView->IsTextEdit())
                {
                    const SdrObject* pEdited = pView->GetTextEditObject();
                    if (pEdited && pEdited->getSdrPageFromSdrObject() == pPage)
                        aView.mpView = pView;
                }

                aViews.push_back(aView);
            }
            pShell = SfxViewShell::GetNext(*pShell, false);
        }

        return aViews;
    }

    /// True when any view is running a text edit on the page.
    bool anyViewIsEditing(SdPage* pPage)
    {
        const std::vector<EditingView>& rViews = views(pPage);
        return std::any_of(rViews.begin(), rViews.end(),
                           [](const EditingView& rView) { return rView.mpView != nullptr; });
    }

    /*
        True while the document is being signed and a view of it has chosen the certificate.
        The document is read-only then, and what is selected may still be moved and resized,
        since that is how the signature line is placed on the page.
    */
    bool isBeingSigned() const
    {
        SfxObjectShell* pShell = mpModel->GetDocShell();
        if (!pShell || !pShell->IsSignPDF())
            return false;

        for (SfxViewShell* pView = SfxViewShell::GetFirst(false); pView;
             pView = SfxViewShell::GetNext(*pView, false))
        {
            if (pView->GetObjectShell() == pShell && pView->GetSignPDFCertificate().Is())
                return true;
        }

        return false;
    }

    /*
        What a picture says about the graphic it shows: the size the graphic has by its own
        resolution, brought into the unit the document keeps its items in the way the crop drag
        does, and the four amounts cropped away from it, all as twips. A picture that shows no
        graphic yet says nothing.
    */
    /// How one end of the connector holds on to its object, read from the connection record,
    /// or where the end lies when it holds on to none, read from the run.
    static SdXImpressDocument::VectorObjectContent::ConnectorTie
    tieOf(const SdrEdgeObj& rConnector, bool bStart)
    {
        SdXImpressDocument::VectorObjectContent::ConnectorTie aTie;
        const SdrObjConnection& rConnection = rConnector.GetConnection(bStart);

        if (!rConnector.GetConnectedNode(bStart))
        {
            const basegfx::B2DPolyPolygon aTrack(rConnector.GetEdgeTrackPath());
            if (aTrack.count() && aTrack.getB2DPolygon(0).count())
            {
                const basegfx::B2DPolygon& rRun = aTrack.getB2DPolygon(0);
                const basegfx::B2DPoint aAt(
                    rRun.getB2DPoint(bStart ? 0 : rRun.count() - 1));
                aTie.moAt = basegfx::B2DPoint(aAt.getX() * constTwipConversionFactor,
                                              aAt.getY() * constTwipConversionFactor);
            }
            return aTie;
        }

        if (!rConnection.IsBestConnection())
            aTie.moPoint = rConnection.GetConnectorId();
        aTie.mbBestVertex = rConnection.IsBestVertex();
        aTie.mbAutoVertex = rConnection.IsAutoVertex();
        aTie.mbAutoCorner = rConnection.IsAutoCorner();
        return aTie;
    }

    /// The numbers the connector's run is worked out from, read from its items and its ties.
    static SdXImpressDocument::VectorObjectContent::ConnectorRouting
    routingOf(const SdrEdgeObj& rConnector)
    {
        SdXImpressDocument::VectorObjectContent::ConnectorRouting aRouting;
        const SfxItemSet& rSet = rConnector.GetMergedItemSet();

        switch (rSet.Get(SDRATTR_EDGEKIND).GetValue())
        {
            case SdrEdgeKind::OrthoLines: aRouting.maKind = "ortho"_ostr; break;
            case SdrEdgeKind::ThreeLines: aRouting.maKind = "three"_ostr; break;
            case SdrEdgeKind::OneLine: aRouting.maKind = "one"_ostr; break;
            case SdrEdgeKind::Bezier: aRouting.maKind = "bezier"_ostr; break;
            case SdrEdgeKind::Arc: aRouting.maKind = "arc"_ostr; break;
        }

        if (rSet.HasItem(SDRATTR_EDGEOOXMLCURVE))
            aRouting.mbCurveFollowsOOXML = rSet.Get(SDRATTR_EDGEOOXMLCURVE).GetValue();

        aRouting.mfNode1Across
            = rSet.Get(SDRATTR_EDGENODE1HORZDIST).GetValue() * constTwipConversionFactor;
        aRouting.mfNode1Down
            = rSet.Get(SDRATTR_EDGENODE1VERTDIST).GetValue() * constTwipConversionFactor;
        aRouting.mfNode2Across
            = rSet.Get(SDRATTR_EDGENODE2HORZDIST).GetValue() * constTwipConversionFactor;
        aRouting.mfNode2Down
            = rSet.Get(SDRATTR_EDGENODE2VERTDIST).GetValue() * constTwipConversionFactor;

        aRouting.mnLineDeltaCount = rSet.Get(SDRATTR_EDGELINEDELTACOUNT).GetValue();
        aRouting.mfLineDelta1
            = rSet.Get(SDRATTR_EDGELINE1DELTA).GetValue() * constTwipConversionFactor;
        aRouting.mfLineDelta2
            = rSet.Get(SDRATTR_EDGELINE2DELTA).GetValue() * constTwipConversionFactor;
        aRouting.mfLineDelta3
            = rSet.Get(SDRATTR_EDGELINE3DELTA).GetValue() * constTwipConversionFactor;

        aRouting.mbAgainstSnapRectangles
            = rConnector.getSdrModelFromSdrObject().GetCompatibilityFlag(
                SdrCompatibilityFlag::ConnectorUseSnapRect);

        aRouting.mbTrackUserDefined = rConnector.IsEdgeTrackUserDefined();
        aRouting.maTieAtStart = tieOf(rConnector, true);
        aRouting.maTieAtEnd = tieOf(rConnector, false);
        return aRouting;
    }

    static void takeInThePicture(const SdrGrafObj& rPicture,
                                 SdXImpressDocument::VectorObjectContent& rContent)
    {
        if (rPicture.GetGraphicType() == GraphicType::NONE
            || rPicture.GetGraphicType() == GraphicType::Default)
            return;

        const GraphicObject& rGraphic(rPicture.GetGraphicObject());
        const MapMode aPoolMapMode(rPicture.getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
        Size aGraphicSize(rGraphic.GetPrefSize());

        if (MapUnit::MapPixel == rGraphic.GetPrefMapMode().GetMapUnit())
            aGraphicSize
                = Application::GetDefaultDevice()->PixelToLogic(aGraphicSize, aPoolMapMode);
        else
            aGraphicSize = OutputDevice::LogicToLogic(aGraphicSize, rGraphic.GetPrefMapMode(),
                                                      aPoolMapMode);

        if (0 == aGraphicSize.Width() || 0 == aGraphicSize.Height())
            return;

        const SdrGrafCropItem& rCrop = rPicture.GetMergedItem(SDRATTR_GRAFCROP);

        rContent.mbIsPicture = true;
        // The checksum the graphic is named by on the wire, so the picture can be drawn from
        // the same fetched image the primitives draw it from.
        rContent.mnGraphicChecksum = static_cast<sal_Int64>(rGraphic.GetGraphic().GetChecksum());
        // The graphic is turned over as the picture draws it: across where the picture is
        // mirrored, inside the picture's own mapping, which carries any turn. A picture turned
        // upside down is a mirrored one turned by half a circle.
        rContent.mnGraphicMirror = static_cast<sal_Int32>(
            rPicture.IsMirrored() ? BmpMirrorFlags::Horizontal : BmpMirrorFlags::NONE);
        rContent.mfGraphicWidth = aGraphicSize.Width() * constTwipConversionFactor;
        rContent.mfGraphicHeight = aGraphicSize.Height() * constTwipConversionFactor;
        rContent.mfCropLeft = rCrop.GetLeft() * constTwipConversionFactor;
        rContent.mfCropTop = rCrop.GetTop() * constTwipConversionFactor;
        rContent.mfCropRight = rCrop.GetRight() * constTwipConversionFactor;
        rContent.mfCropBottom = rCrop.GetBottom() * constTwipConversionFactor;
    }

    /// True when the object, or any object inside it when it is a group, has a text edit
    /// running on it.
    static bool hasActiveTextEdit(SdrObject* pObject)
    {
        SdrTextObj* pTextObject = DynCastSdrTextObj(pObject);
        if (pTextObject && pTextObject->IsInEditMode())
            return true;

        if (SdrObjList* pChildren = pObject->GetSubList())
        {
            for (size_t i = 0; i < pChildren->GetObjCount(); ++i)
            {
                SdrObject* pChild = pChildren->GetObj(i);
                if (pChild && hasActiveTextEdit(pChild))
                    return true;
            }
        }

        return false;
    }

    /// What is written for one object: its primitives, the box it paints and the mapping of the
    /// unit rectangle onto it. The objects inside a group draw the group's content, so a group
    /// with members has no primitives of its own.
    SdXImpressDocument::VectorObjectContent contentOf(SdrObject& rObject)
    {
        SdXImpressDocument::VectorObjectContent aContent;

        const SdrObjList* pChildren = rObject.GetSubList();
        if (!pChildren || pChildren->GetObjCount() == 0)
        {
            // A page object shows the page it stands for. One that stands for no page keeps
            // the outline the drawing layer gives it.
            if (const auto* pPageObject = dynamic_cast<const SdrPageObj*>(&rObject))
                aContent.maPrimitives = pagePreviewContent(*pPageObject);
            if (aContent.maPrimitives.empty())
                rObject.GetViewContact().getViewIndependentPrimitive2DContainer(
                    aContent.maPrimitives);

            // The aids that mark out a placeholder, its dashed boundary and the name of the
            // area, travel in an array of their own. A master page is only ever looked at as
            // a page being worked on, so its thumbnail draws the prompt text of an empty
            // placeholder while leaving the aids to the view that edits the page. An object
            // drawn behind the page rather than on it is marked out where it lives, on the
            // master, so it brings none of them here.
            aContent.maAids
                = sd::createPlaceholderDecoration(rObject, isBehindThePage(rObject));

            aContent.maHandles = shapingHandlesOf(rObject);
            aContent.maPath = pathOf(rObject);
            aContent.meKind = rObject.GetObjIdentifier();

            if (const SdrEdgeObj* pConnector = dynamic_cast<const SdrEdgeObj*>(&rObject))
            {
                const SdrObject* pAtStart = pConnector->GetConnectedNode(true);
                const SdrObject* pAtEnd = pConnector->GetConnectedNode(false);
                aContent.mbTiedAtStart = nullptr != pAtStart;
                aContent.mbTiedAtEnd = nullptr != pAtEnd;
                aContent.mnTiedToAtStart = pAtStart ? pAtStart->GetUniqueID() : 0;
                aContent.mnTiedToAtEnd = pAtEnd ? pAtEnd->GetUniqueID() : 0;
                aContent.moRouting = routingOf(*pConnector);
            }

            /*
                The place of a glue point is given on the object, so that the client can move it,
                scale it and turn it with the object without asking anything. The way back from
                the page to the object is the transformation the entry carries, undone.
            */
            basegfx::B2DHomMatrix aFromTheObject(transformationInTwips(rObject));
            const bool bCanBeUndone(aFromTheObject.invert());

            /*
                The four at the middles of the sides are the object's own business: a rectangle
                or a picture turns them and shears them with itself and puts them out by half its
                line, a custom shape has its own, and anything else takes the middles of its box.
                So they are asked for, and given on the object like the others.
            */
            if (bCanBeUndone && !dynamic_cast<const SdrEdgeObj*>(&rObject))
            {
                for (sal_uInt16 nSide = 0; nSide < 4; ++nSide)
                {
                    const Point aAt(rObject.GetVertexGluePoint(nSide).GetAbsolutePos(rObject));
                    aContent.maSideGluePoints.push_back(
                        aFromTheObject
                        * basegfx::B2DPoint(aAt.X() * constTwipConversionFactor,
                                            aAt.Y() * constTwipConversionFactor));
                }
            }

            if (const SdrGluePointList* pGluePoints = rObject.GetGluePointList())
            {

                for (sal_uInt16 nGluePoint = 0; nGluePoint < pGluePoints->GetCount();
                     ++nGluePoint)
                {
                    const SdrGluePoint& rGluePoint((*pGluePoints)[nGluePoint]);
                    const Point aAt(rGluePoint.GetAbsolutePos(rObject));
                    basegfx::B2DPoint aOnThePage(aAt.X() * constTwipConversionFactor,
                                                 aAt.Y() * constTwipConversionFactor);

                    if (bCanBeUndone)
                        aOnThePage = aFromTheObject * aOnThePage;

                    SdXImpressDocument::VectorObjectContent::GluePoint aOne;
                    aOne.mfX = aOnThePage.getX();
                    aOne.mfY = aOnThePage.getY();
                    aOne.mbShareOfTheSize = rGluePoint.IsPercent();
                    aOne.mnId = rGluePoint.GetId();

                    /*
                        The six a point can hold, which are the six the interface a document is
                        read through knows. A mix of sides the model could hold besides them
                        turns into none of them on the way in, so it never reaches here.
                    */
                    switch (rGluePoint.GetEscDir())
                    {
                        case SdrEscapeDirection::LEFT:
                            aOne.maWayOut = "left"_ostr;
                            break;
                        case SdrEscapeDirection::RIGHT:
                            aOne.maWayOut = "right"_ostr;
                            break;
                        case SdrEscapeDirection::TOP:
                            aOne.maWayOut = "top"_ostr;
                            break;
                        case SdrEscapeDirection::BOTTOM:
                            aOne.maWayOut = "bottom"_ostr;
                            break;
                        case SdrEscapeDirection::HORZ:
                            aOne.maWayOut = "horizontal"_ostr;
                            break;
                        case SdrEscapeDirection::VERT:
                            aOne.maWayOut = "vertical"_ostr;
                            break;
                        default:
                            break;
                    }

                    const SdrAlign eAcross(rGluePoint.GetHorzAlign());
                    if (SdrAlign::HORZ_LEFT == eAcross)
                        aOne.maFromAcross = "left"_ostr;
                    else if (SdrAlign::HORZ_RIGHT == eAcross)
                        aOne.maFromAcross = "right"_ostr;

                    const SdrAlign eDown(rGluePoint.GetVertAlign());
                    if (SdrAlign::VERT_TOP == eDown)
                        aOne.maFromDown = "top"_ostr;
                    else if (SdrAlign::VERT_BOTTOM == eDown)
                        aOne.maFromDown = "bottom"_ostr;

                    aContent.maOwnGluePoints.push_back(aOne);
                }
            }

            decomposeForComparison(aContent);
        }

        /*
            What may be done to the object. The place and the size are each held against being
            changed on their own, while whether it may be turned is one of the things the object
            answers about itself - a media object, for one, will not be. A group answers about
            these as much as anything else does, so they are taken outside the part that only an
            object drawing something of its own goes through.
        */
        aContent.mbIsDiagram = rObject.isDiagram();
        aContent.maGraphicExtension = rObject.getGraphicExtension();
        aContent.mbSigning = isBeingSigned();

        if (const auto* pPicture = dynamic_cast<const SdrGrafObj*>(&rObject))
            takeInThePicture(*pPicture, aContent);

        SdrObjTransformInfoRec aWhatItAllows;
        rObject.TakeObjInfo(aWhatItAllows);
        aContent.mbCanBeMoved = !rObject.IsMoveProtect();
        aContent.mbCanBeResized = !rObject.IsResizeProtect();
        aContent.mbCanBeTurned = aWhatItAllows.bRotateFreeAllowed;
        aContent.mbCanBeResizedFreely = aWhatItAllows.bResizeFreeAllowed;
        aContent.mbCanBeMirrored = aWhatItAllows.bMirrorFreeAllowed;
        aContent.mbCanBeSheared = aWhatItAllows.bShearAllowed;

        aContent.maPaintedBox = paintedRectangleInTwips(rObject, aContent.maPrimitives);
        aContent.maTransformation = transformationInTwips(rObject);
        aContent.maName = rObject.GetName();
        aContent.moLayer = sal_Int32(rObject.GetLayer().get());
        aContent.mbEmptyPlaceholder = rObject.IsEmptyPresObj();
        aContent.mbTextEdit = hasActiveTextEdit(&rObject);
        const SdrObject* pParent = rObject.getParentSdrObjectFromSdrObject();
        aContent.mnParentId = pParent ? pParent->GetUniqueID() : 0;
        // The text primitive resolves the automatic color when it is drawn, so the
        // decomposition above reads the same on a light and on a dark page. An object with
        // text is compared against the background it was drawn on, and travels when it moves.
        if (rObject.HasText())
            aContent.maAutoColor = maViewInformation.getAutoColor();
        return aContent;
    }

    /// Looks again at the objects a change marked and counts the part's version up only for
    /// those whose content really differs from what was written last. An object the page no
    /// longer holds is forgotten.
    void resolveDirtyObjects(SdPage* pPage)
    {
        std::unordered_set<sal_uInt64> aDirty
            = mpModel->takeVectorDirtyObjects(*pPage);

        const std::vector<SdrObject*>& rObjects = partObjects(pPage);

        // A full response also repairs a page the client holds, so every recorded object is
        // compared again. That also catches a change that came without a hint.
        if (!isDelta())
        {
            for (const SdrObject* pObject : rObjects)
            {
                if (mpModel->findVectorObjectContent(*pPage, pObject->GetUniqueID()))
                    aDirty.insert(pObject->GetUniqueID());
            }
        }

        // The paint order is compared as a whole. Raising an object above another announces a
        // change on that object alone and its own content stands still, so the order is the
        // only place the move shows. A group that lost or gained a member is looked at with it.
        std::vector<sal_uInt64> aOrder;
        aOrder.reserve(rObjects.size());
        for (const SdrObject* pObject : rObjects)
            aOrder.push_back(pObject->GetUniqueID());
        if (mpModel->recordVectorPaintOrder(*pPage, aOrder))
        {
            for (const SdrObject* pObject : rObjects)
            {
                const SdrObjList* pChildren = pObject->GetSubList();
                if (pChildren && pChildren->GetObjCount() > 0)
                    aDirty.insert(pObject->GetUniqueID());
            }
        }

        std::unordered_map<sal_uInt64, SdrObject*> aObjectById;
        for (SdrObject* pObject : rObjects)
            aObjectById.emplace(pObject->GetUniqueID(), pObject);

        // An object under edit is compared like any other, because what is typed rides on the
        // entry for the edit and the object's own look stands still until something moves it.
        for (const sal_uInt64 nObjectId : aDirty)
        {
            const auto aFound = aObjectById.find(nObjectId);
            if (aFound == aObjectById.end())
            {
                mpModel->forgetVectorObject(*pPage, nObjectId);
                continue;
            }

            const auto aInserted
                = maResolvedContent.emplace(nObjectId, contentOf(*aFound->second));
            mpModel->recordVectorObjectContent(*pPage, nObjectId, aInserted.first->second);
        }
    }

    /// True when a delta would carry something: the page's own content changed, the object set
    /// or its order moved, or at least one object differs from what the client holds.
    bool hasContentToSend(SdPage* pPage)
    {
        if (mpModel->isVectorMasterChangedSince(*pPage, sal_uInt64(mnSinceVersion))
            || mpModel->isVectorOrderChangedSince(*pPage, sal_uInt64(mnSinceVersion)))
        {
            return true;
        }

        // The text of a running edit, when a keystroke moved it.
        for (const EditingView& rView : views(pPage))
        {
            if (mpModel->isVectorObjectChangedSince(*pPage, textEditEntryKey(rView.mnViewId),
                                                    sal_uInt64(mnSinceVersion)))
            {
                return true;
            }
        }

        const std::vector<SdrObject*>& rObjects = partObjects(pPage);

        for (const SdrObject* pObject : rObjects)
        {
            if (mpModel->isVectorObjectChangedSince(*pPage, pObject->GetUniqueID(),
                                                    sal_uInt64(mnSinceVersion)))
            {
                return true;
            }
        }

        return false;
    }

    /// The entry that carries the text of a running text edit, into the open objects array.
    /// The object it runs on hides its own text, so this is what shows what has been typed.
    void writeTextEditEntry(tools::JsonWriter& rWriter, SdPage* pPage)
    {
        for (const EditingView& rView : views(pPage))
        {
            if (!rView.mpView)
                continue;

            // A delta carries an entry only when a keystroke moved its text, so the model
            // change that follows a pause in the typing adds nothing to it.
            if (isDelta()
                && !mpModel->isVectorObjectChangedSince(*pPage, textEditEntryKey(rView.mnViewId),
                                                        sal_uInt64(mnSinceVersion)))
            {
                continue;
            }

            // An edit whose text is empty still gets its entry, with an empty primitive list.
            // The order names the entry for as long as the edit runs, and a client that holds
            // the text from before the last deletion replaces it with nothing.
            const SdrObject* pEdited = rView.mpView->GetTextEditObject();
            writeEntry(rWriter, textEditEntryId(rView.mnViewId),
                       pEdited ? pEdited->GetUniqueID() : 0, "texteditoverlay",
                       textEditContentOf(rView), rView.mnViewId);
        }
    }

    /// The content of the entry carrying the view's running edit, laid out once per write. The
    /// comparison that decides whether the entry travels and the write that follows both read
    /// it, and laying out a long edit is what a keystroke costs.
    const SdXImpressDocument::VectorObjectContent& textEditContentOf(const EditingView& rView)
    {
        auto aFound = maTextEditContent.find(rView.mnViewId);
        if (aFound != maTextEditContent.end())
            return aFound->second;

        auto aAdded = maTextEditContent.emplace(rView.mnViewId, textEditContent(*rView.mpView));
        return aAdded.first->second;
    }

    SdXImpressDocument::VectorObjectContent textEditContent(SdrObjEditView& rView)
    {
        SdXImpressDocument::VectorObjectContent aContent;
        aContent.maPrimitives = rView.getTextEditPrimitives();
        aContent.maDrawn = aContent.maPrimitives;
        aContent.maPaintedBox = rangeInTwips(aContent.maPrimitives.getB2DRange(maViewInformation));
        aContent.maTransformation = boxTransformation(aContent.maPaintedBox);
        // The entry names the object the edit runs on as its parent and draws on that object's
        // layer, so moving the edit to another object, or that object to another layer, is a
        // change of the entry.
        const SdrObject* pEdited = rView.GetTextEditObject();
        aContent.mnParentId = pEdited ? pEdited->GetUniqueID() : 0;
        if (pEdited)
            aContent.moLayer = sal_Int32(pEdited->GetLayer().get());
        return aContent;
    }

    /// Looks at the text of a running edit and counts the part's version up when it moved, the
    /// way a changed object does. An edit that has ended leaves nothing recorded behind it.
    void resolveTextEditEntry(SdPage* pPage)
    {
        std::unordered_set<sal_uInt64> aLiveKeys;
        for (const EditingView& rView : views(pPage))
        {
            const sal_uInt64 nKey = textEditEntryKey(rView.mnViewId);
            aLiveKeys.insert(nKey);

            // A view that stopped editing, or moved to another page, leaves nothing behind.
            if (!rView.mpView)
                mpModel->forgetVectorObject(*pPage, nKey);
            else
                mpModel->recordVectorObjectContent(*pPage, nKey, textEditContentOf(rView));
        }

        // A view that closed while its edit ran is gone from the list of views, so its entry
        // is dropped by the key rather than by the view.
        for (const sal_uInt64 nKey : mpModel->getVectorRecordedIds(*pPage))
        {
            if (isTextEditEntryKey(nKey) && !aLiveKeys.contains(nKey))
                mpModel->forgetVectorObject(*pPage, nKey);
        }
    }

    /// The mapping of the unit rectangle onto an upright box, for an entry that has no
    /// transformation of its own.
    static basegfx::B2DHomMatrix boxTransformation(const tools::Rectangle& rBox)
    {
        basegfx::B2DHomMatrix aTransformation;
        aTransformation.set(0, 0, double(rBox.GetWidth()));
        aTransformation.set(1, 1, double(rBox.GetHeight()));
        aTransformation.set(0, 2, double(rBox.Left()));
        aTransformation.set(1, 2, double(rBox.Top()));
        return aTransformation;
    }

    /// One entry per painted object on the page, into the open objects array, in the order the
    /// paint visits them.
    void writePageObjects(tools::JsonWriter& rWriter, SdPage* pPage)
    {
        const std::vector<SdrObject*>& rObjects = partObjects(pPage);

        for (SdrObject* pObject : rObjects)
        {
            // A delta carries full content only for the objects that changed after the client's
            // version. The rest stay in the order list. An object a text edit runs on says so in
            // the content compared, so beginning and ending an edit carries it.
            if (isDelta()
                && !mpModel->isVectorObjectChangedSince(*pPage, pObject->GetUniqueID(),
                                                        sal_uInt64(mnSinceVersion)))
            {
                continue;
            }

            // The content compared in this write is written. An object not compared is
            // unchanged since its record, so the record is written. Only an object with no
            // record is built here.
            const sal_uInt64 nObjectId = pObject->GetUniqueID();
            const SdXImpressDocument::VectorObjectContent* pContent = nullptr;
            const auto aResolved = maResolvedContent.find(nObjectId);
            if (aResolved != maResolvedContent.end())
                pContent = &aResolved->second;
            else
                pContent = mpModel->findVectorObjectContent(*pPage, nObjectId);
            if (!pContent)
            {
                pContent
                    = &maResolvedContent.emplace(nObjectId, contentOf(*pObject)).first->second;
                // What was written is what the client holds from here on, so a later change to
                // the object is compared against this.
                mpModel->noteVectorObjectWritten(*pPage, nObjectId, *pContent);
            }

            const SdrObject* pParent = pObject->getParentSdrObjectFromSdrObject();
            writeObjectEntry(rWriter, *pObject, pParent ? pParent->GetUniqueID() : 0, *pContent);
        }
    }

    /** Takes the places a handle may be moved between onto the object's own square, where 0 and
        0 is its upper left corner and 1 and 1 its lower right one.

        A place written that way travels with the object: a reader that moves the object itself,
        without waiting to be told what became of it, still has the places right. It also says
        the same thing however the object is turned, sheared or mirrored, since all of that is
        in the mapping the reader already holds.
     */
    static void onTheObjectsSquare(SdrHandleRails& rRails, const SdrObject& rObject)
    {
        basegfx::B2DHomMatrix aOntoTheSquare(transformationInTwips(rObject));
        if (!aOntoTheSquare.invert())
            return;

        auto aOne = [&aOntoTheSquare](basegfx::B2DPoint& rPlace) {
            rPlace = aOntoTheSquare * basegfx::B2DPoint(rPlace.getX() * constTwipConversionFactor,
                                                        rPlace.getY() * constTwipConversionFactor);
        };

        aOne(rRails.maLeastAcross);
        aOne(rRails.maMostAcross);
        aOne(rRails.maLeastDown);
        aOne(rRails.maMostDown);
        aOne(rRails.maAround);
        aOne(rRails.maNearest);
        aOne(rRails.maFurthest);
        aOne(rRails.maFrom);
        aOne(rRails.maTo);
    }

    /** The handles that shape an object, in twips: the corner radius of a rectangle and the
        points a custom shape is shaped by.

        A reader works the rest out from what it draws - the points of a polygon and the weights
        of a curve are in the geometry it already has - while these two say nothing about the
        drawing and cannot be found in it. Each handle is named by what it means, the kind with
        the polygon and the point it belongs to, so that a reader can say which handle it moves
        without counting the handles it was never sent.
     */
    static std::vector<SdXImpressDocument::VectorObjectContent::Handle>
    shapingHandlesOf(const SdrObject& rObject)
    {
        SdrHdlList aHandleList(nullptr);
        rObject.AddToHdlList(aHandleList);

        std::vector<SdXImpressDocument::VectorObjectContent::Handle> aHandles;
        for (size_t nHandle = 0; nHandle < aHandleList.GetHdlCount(); ++nHandle)
        {
            const SdrHdl* pHandle = aHandleList.GetHdl(nHandle);
            if (!pHandle)
                continue;

            /*
                The handles a reader cannot work out for itself. The corner radius of a
                rectangle and the points a custom shape is shaped by say nothing about what is
                drawn. The points of an object that is shaped by them - the ends of a connector
                and the lines between them, the tail of a caption - are another such case,
                unless the object's path travels as well, and then they follow from it.
            */
            const SdrHdlKind eKind = pHandle->GetKind();
            const bool bWanted(eKind == SdrHdlKind::Circle || eKind == SdrHdlKind::CustomShape1
                               || (eKind == SdrHdlKind::Poly
                                   && nullptr == dynamic_cast<const SdrPathObj*>(&rObject)));
            if (!bWanted)
                continue;

            const Point aPosition(pHandle->GetPos());
            SdXImpressDocument::VectorObjectContent::Handle aOne;
            aOne.mnKind = static_cast<sal_Int32>(eKind);
            aOne.mnPolygon = pHandle->GetPolyNum();
            aOne.mnPoint = pHandle->GetPointNum();
            aOne.mnObjHandle = pHandle->GetObjHdlNum();

            /*
                A point a custom shape is shaped by may only go where the shape lets it, and what
                it lets it do is worked out from the shape as it stands. It travels so that a
                client can hold a drag to it without knowing anything about the shape itself.
            */
            // How far the handle may be moved is the object's own business, and it answers in
            // its own coordinates, so the places are taken onto its square here.
            aOne.mbHasRails = rObject.GetHandleRails(*pHandle, aOne.maRails);
            if (aOne.mbHasRails)
                onTheObjectsSquare(aOne.maRails, rObject);
            aOne.maPosition = Point(
                basegfx::fround<tools::Long>(aPosition.X() * constTwipConversionFactor),
                basegfx::fround<tools::Long>(aPosition.Y() * constTwipConversionFactor));
            aHandles.push_back(aOne);
        }

        return aHandles;
    }

    /** The path an object is drawn from, in twips, taken from the model rather than from what
        the object draws.

        What is drawn is the path with everything the drawing does to it: a stroke becomes the
        outline of a fat line, a dash becomes many polygons, a shadow is another copy of it all,
        and a curve may have been cut into straight pieces. An editor needs the points the model
        holds, in the order it holds them, because that order is what names a point when it is
        moved. An object that has no path of its own answers with an empty one.
     */
    static basegfx::B2DPolyPolygon pathOf(const SdrObject& rObject)
    {
        const auto* pPathObject = dynamic_cast<const SdrPathObj*>(&rObject);
        if (!pPathObject)
            return basegfx::B2DPolyPolygon();

        basegfx::B2DPolyPolygon aPath(pPathObject->GetPathPoly());
        aPath.transform(basegfx::utils::createScaleB2DHomMatrix(constTwipConversionFactor,
                                                                constTwipConversionFactor));
        return aPath;
    }

    /// The range as an upright box in twips, empty for an empty range.
    static tools::Rectangle rangeInTwips(const basegfx::B2DRange& rRange)
    {
        if (rRange.isEmpty())
            return tools::Rectangle();

        return tools::Rectangle(
            basegfx::fround<tools::Long>(rRange.getMinX() * constTwipConversionFactor),
            basegfx::fround<tools::Long>(rRange.getMinY() * constTwipConversionFactor),
            basegfx::fround<tools::Long>(rRange.getMaxX() * constTwipConversionFactor),
            basegfx::fround<tools::Long>(rRange.getMaxY() * constTwipConversionFactor));
    }

    /// The rectangle the object paints, in twips: the range of its primitives, which takes in
    /// the line width and a shadow, or the object's bound rectangle when it paints nothing.
    tools::Rectangle
    paintedRectangleInTwips(const SdrObject& rObject,
                            const drawinglayer::primitive2d::Primitive2DContainer& rPrimitives)
    {
        basegfx::B2DRange aRange(rPrimitives.getB2DRange(maViewInformation));
        if (aRange.isEmpty())
            aRange = vcl::unotools::b2DRectangleFromRectangle(rObject.GetCurrentBoundRect());

        return rangeInTwips(aRange);
    }

    /*
        The mapping an object is drawn by. What the object answers describes its base geometry,
        which holds the shear the other way round: the object lays itself out with the shear
        turned over, and the answer keeps the angle as the model holds it, so that the two agree
        when the answer is handed back. A reader that wants to know where the object is drawn
        needs the first of those, so the shear is turned over here.

        An object with no shear is the same either way, and a scale of a turned object is what
        gives an object shear, which is why this only shows there.
    */
    static basegfx::B2DHomMatrix asItIsDrawn(const basegfx::B2DHomMatrix& rBaseGeometry)
    {
        basegfx::B2DTuple aScale;
        basegfx::B2DTuple aTranslate;
        double fRotate(0.0);
        double fShearX(0.0);
        rBaseGeometry.decompose(aScale, aTranslate, fRotate, fShearX);

        if (basegfx::fTools::equalZero(fShearX))
            return rBaseGeometry;

        return basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(aScale, -fShearX,
                                                                            fRotate, aTranslate);
    }

    /** Mirrors the mapping back about the line the drawing layer un-mirrored it about: an
        upright line through the middle of the box around it, or a level one.

        A shape that says it faces the other way is answered for with the mapping it would have
        if it did not, and that is worked out by mirroring it about a line of the page, not about
        a line of its own. For a shape standing straight the two are the same line and nothing
        moves. For a turned one they are not, and the answer lies somewhere else than the shape
        is drawn, which is what this takes back. The box around a shape is not changed by
        mirroring it about its own middle, so the line can be worked out here just as well.
     */
    static basegfx::B2DHomMatrix mirroredBack(const basegfx::B2DHomMatrix& rTransformation,
                                              const bool bAcross)
    {
        basegfx::B2DRange aBox;
        for (const auto& rCorner : { basegfx::B2DPoint(0.0, 0.0), basegfx::B2DPoint(1.0, 0.0),
                                     basegfx::B2DPoint(1.0, 1.0), basegfx::B2DPoint(0.0, 1.0) })
            aBox.expand(rTransformation * rCorner);

        const double fMiddle(bAcross ? aBox.getCenter().getX() : aBox.getCenter().getY());

        basegfx::B2DHomMatrix aMirror;
        if (bAcross)
        {
            aMirror.set(0, 0, -1.0);
            aMirror.set(0, 2, 2.0 * fMiddle);
        }
        else
        {
            aMirror.set(1, 1, -1.0);
            aMirror.set(1, 2, 2.0 * fMiddle);
        }

        return aMirror * rTransformation;
    }

    /** The mapping that takes the object's own square onto the page, in twips, saying the whole
        truth about where the object is and which way round it is drawn.

        The drawing layer holds none of its own mirroring in a mapping: a shape keeps it apart,
        as the way it says it faces, and answers with a mapping that has positive lengths
        whichever way it is drawn. A reader that only has the mapping would then have no way of
        telling the two apart, so the mirroring is laid back into it here. The shear is turned
        over here for the same reason, and both belong at this one place: what leaves the engine
        should be a mapping anybody can work with, and the drawing layer's own way of keeping
        these should not have to be known on the other side.
     */
    static basegfx::B2DHomMatrix transformationInTwips(const SdrObject& rObject)
    {
        basegfx::B2DHomMatrix aTransformation;
        basegfx::B2DPolyPolygon aPolyPolygon;
        rObject.TRGetBaseGeometry(aTransformation, aPolyPolygon);
        aTransformation = asItIsDrawn(aTransformation);

        if (const auto* pShape = dynamic_cast<const SdrObjCustomShape*>(&rObject))
        {
            if (pShape->IsMirroredX())
                aTransformation = mirroredBack(aTransformation, true);
            if (pShape->IsMirroredY())
                aTransformation = mirroredBack(aTransformation, false);
        }

        return basegfx::utils::createScaleB2DHomMatrix(constTwipConversionFactor,
                                                       constTwipConversionFactor)
               * aTransformation;
    }

    /// An object with an empty decomposition still gets an entry, with an empty primitive
    /// list. The object set then always matches the ids the order array carries, and content
    /// that became empty replaces what a client has cached.
    void writeObjectEntry(tools::JsonWriter& rWriter, const SdrObject& rObject,
                          sal_uInt64 nParentId,
                          const SdXImpressDocument::VectorObjectContent& rContent)
    {
        auto pObjectNode = rWriter.startStruct();
        rWriter.put("id", sal_Int64(rObject.GetUniqueID()));
        rWriter.put("name", rContent.maName);
        // The group the object sits in, 0 for an object directly on the page.
        rWriter.put("parent", sal_Int64(nParentId));
        if (rContent.moLayer)
            rWriter.put("layer", *rContent.moLayer);
        // A placeholder that holds no content of its own yet.
        if (rContent.mbEmptyPlaceholder)
            rWriter.put("emptyPlaceholder", true);
        // A text edit is running on the object, so it is showing none of its own text and the
        // entry for the edit carries what has been typed.
        if (rContent.mbTextEdit)
            rWriter.put("textEdit", true);

        const SdPage* pObjectPage = dynamic_cast<const SdPage*>(rObject.getSdrPageFromSdrObject());
        if (mnMode == constVectorModeMasterPages && pObjectPage)
        {
            // In master view the master is the page itself, so its entries say what a slide
            // does with them: left out, taken from the slide's own copy, or drawn as the
            // master carries it when neither is set.
            if (isHiddenBehindSlide(rObject))
                rWriter.put("hiddenBehindSlide", true);
            else if (isSlideDependent(*pObjectPage, const_cast<SdrObject&>(rObject)))
                rWriter.put("slideDependent", true);
        }
        else if (isBehindThePage(rObject))
        {
            // The slide's copy of a master placeholder.
            rWriter.put("masterContent", true);
        }

        writeEntryGeometry(rWriter, rContent);
    }

    /// An entry that stands for something other than an object on the page, named by a kind
    /// rather than by a name of its own.
    void writeEntry(tools::JsonWriter& rWriter, sal_Int64 nId, sal_uInt64 nParentId,
                    const char* pKind, const SdXImpressDocument::VectorObjectContent& rContent,
                    std::optional<sal_Int32> oViewId = std::nullopt)
    {
        auto pEntryNode = rWriter.startStruct();
        rWriter.put("id", nId);
        rWriter.put("parent", sal_Int64(nParentId));
        rWriter.put("kind", pKind);
        // Whose edit it is, so a reader can tell its own from another user's.
        if (oViewId)
            rWriter.put("viewId", sal_Int32(*oViewId));
        // The layer of the object the entry belongs to, so the entry is hidden with it.
        if (rContent.moLayer)
            rWriter.put("layer", *rContent.moLayer);

        writeEntryGeometry(rWriter, rContent);
    }

    /// How one end of a connector holds on to its object, as a node of the given name.
    static void writeTie(tools::JsonWriter& rWriter, std::string_view aName,
                         const SdXImpressDocument::VectorObjectContent::ConnectorTie& rTie)
    {
        auto aNode = rWriter.startNode(aName);
        if (rTie.moAt)
        {
            rWriter.put("x", rTie.moAt->getX());
            rWriter.put("y", rTie.moAt->getY());
        }
        if (rTie.moPoint)
            rWriter.put("point", sal_Int32(*rTie.moPoint));
        if (rTie.mbBestVertex)
            rWriter.put("bestVertex", true);
        if (rTie.mbAutoVertex)
            rWriter.put("autoVertex", true);
        if (rTie.mbAutoCorner)
            rWriter.put("autoCorner", true);
    }

    /// The numbers a connector's run is worked out from, as the "routing" node of its entry.
    static void
    writeRouting(tools::JsonWriter& rWriter,
                 const SdXImpressDocument::VectorObjectContent::ConnectorRouting& rRouting)
    {
        auto aNode = rWriter.startNode("routing");
        rWriter.put("kind", rRouting.maKind);
        if (rRouting.mbCurveFollowsOOXML)
            rWriter.put("curveFollowsOOXML", true);
        {
            auto aDistances = rWriter.startArray("nodeDistances");
            rWriter.putSimpleValue(rRouting.mfNode1Across);
            rWriter.putSimpleValue(rRouting.mfNode1Down);
            rWriter.putSimpleValue(rRouting.mfNode2Across);
            rWriter.putSimpleValue(rRouting.mfNode2Down);
        }
        rWriter.put("lineDeltaCount", rRouting.mnLineDeltaCount);
        {
            auto aDeltas = rWriter.startArray("lineDeltas");
            rWriter.putSimpleValue(rRouting.mfLineDelta1);
            rWriter.putSimpleValue(rRouting.mfLineDelta2);
            rWriter.putSimpleValue(rRouting.mfLineDelta3);
        }
        if (rRouting.mbAgainstSnapRectangles)
            rWriter.put("againstSnapRectangles", true);
        if (rRouting.mbTrackUserDefined)
            rWriter.put("trackUserDefined", true);
        writeTie(rWriter, "tieAtStart", rRouting.maTieAtStart);
        writeTie(rWriter, "tieAtEnd", rRouting.maTieAtEnd);
    }

    /// Where an entry paints, how the unit rectangle maps onto it, and how it looks.
    void writeEntryGeometry(tools::JsonWriter& rWriter,
                            const SdXImpressDocument::VectorObjectContent& rContent)
    {
        const tools::Rectangle& rPainted = rContent.maPaintedBox;
        rWriter.put("x", sal_Int64(rPainted.Left()));
        rWriter.put("y", sal_Int64(rPainted.Top()));
        rWriter.put("width", sal_Int64(rPainted.GetWidth()));
        rWriter.put("height", sal_Int64(rPainted.GetHeight()));

        // In the order a canvas takes it: x' = a * x + c * y + e and y' = b * x + d * y + f.
        {
            const basegfx::B2DHomMatrix& aTransformation = rContent.maTransformation;
            auto aTransformArray = rWriter.startArray("transform");
            rWriter.putSimpleValue(aTransformation.get(0, 0));
            rWriter.putSimpleValue(aTransformation.get(1, 0));
            rWriter.putSimpleValue(aTransformation.get(0, 1));
            rWriter.putSimpleValue(aTransformation.get(1, 1));
            rWriter.putSimpleValue(aTransformation.get(0, 2));
            rWriter.putSimpleValue(aTransformation.get(1, 2));
        }

        {
            auto aPrimitiveArray = rWriter.startArray("primitives");
            maProcessor->decomposeAndWrite(rContent.maPrimitives);
        }
        if (!rContent.maAids.empty())
        {
            auto aAidArray = rWriter.startArray("aids");
            maProcessor->decomposeAndWrite(rContent.maAids);
        }
        if (rContent.meKind != SdrObjKind::NONE)
            rWriter.put("objectKind", static_cast<sal_Int32>(rContent.meKind));
        if (rContent.mbIsDiagram)
            rWriter.put("isDiagram", true);
        if (!rContent.maGraphicExtension.isEmpty())
            rWriter.put("graphicExtension", rContent.maGraphicExtension);
        if (rContent.mbSigning)
            rWriter.put("signing", true);
        if (rContent.mbIsPicture)
        {
            rWriter.put("graphicChecksum", rContent.mnGraphicChecksum);
            if (rContent.mnGraphicMirror)
                rWriter.put("graphicMirror", rContent.mnGraphicMirror);
            rWriter.put("graphicWidth", rContent.mfGraphicWidth);
            rWriter.put("graphicHeight", rContent.mfGraphicHeight);
            rWriter.put("cropLeft", rContent.mfCropLeft);
            rWriter.put("cropTop", rContent.mfCropTop);
            rWriter.put("cropRight", rContent.mfCropRight);
            rWriter.put("cropBottom", rContent.mfCropBottom);
        }

        // Only what it may not have done travels, since most objects allow everything.
        if (!rContent.mbCanBeMoved)
            rWriter.put("cannotBeMoved", true);
        if (!rContent.mbCanBeResized)
            rWriter.put("cannotBeResized", true);
        if (!rContent.mbCanBeTurned)
            rWriter.put("cannotBeTurned", true);
        if (!rContent.mbCanBeResizedFreely)
            rWriter.put("cannotBeResizedFreely", true);
        if (!rContent.mbCanBeMirrored)
            rWriter.put("cannotBeMirrored", true);
        if (!rContent.mbCanBeSheared)
            rWriter.put("cannotBeSheared", true);

        if (rContent.mbTiedAtStart)
            rWriter.put("tiedAtStart", true);
        if (rContent.mbTiedAtEnd)
            rWriter.put("tiedAtEnd", true);
        if (rContent.mnTiedToAtStart)
            rWriter.put("tiedToAtStart", sal_Int64(rContent.mnTiedToAtStart));
        if (rContent.mnTiedToAtEnd)
            rWriter.put("tiedToAtEnd", sal_Int64(rContent.mnTiedToAtEnd));

        if (rContent.moRouting)
            writeRouting(rWriter, *rContent.moRouting);

        if (!rContent.maOwnGluePoints.empty())
        {
            auto aGlueNode = rWriter.startArray("gluePoints");
            for (const auto& rGluePoint : rContent.maOwnGluePoints)
            {
                auto aOneNode = rWriter.startStruct();
                rWriter.put("x", rGluePoint.mfX);
                rWriter.put("y", rGluePoint.mfY);
                rWriter.put("id", sal_Int32(rGluePoint.mnId));
                if (!rGluePoint.mbShareOfTheSize)
                    rWriter.put("keepsItsDistance", true);
                if (!rGluePoint.maWayOut.isEmpty())
                    rWriter.put("wayOut", rGluePoint.maWayOut);
                if (!rGluePoint.maFromAcross.isEmpty())
                    rWriter.put("fromAcross", rGluePoint.maFromAcross);
                if (!rGluePoint.maFromDown.isEmpty())
                    rWriter.put("fromDown", rGluePoint.maFromDown);
            }
        }

        if (!rContent.maSideGluePoints.empty())
        {
            auto aSideNode = rWriter.startArray("sideGluePoints");
            for (const auto& rPoint : rContent.maSideGluePoints)
            {
                auto aOneNode = rWriter.startStruct();
                rWriter.put("x", rPoint.getX());
                rWriter.put("y", rPoint.getY());
            }
        }

        if (rContent.maPath.count())
        {
            auto aPathArray = rWriter.startArray("path");
            for (const auto& rPolygon : rContent.maPath)
            {
                auto aPolygonNode = rWriter.startStruct();
                if (rPolygon.isClosed())
                    rWriter.put("closed", true);

                auto aPointArray = rWriter.startArray("points");
                for (sal_uInt32 nPoint = 0; nPoint < rPolygon.count(); ++nPoint)
                {
                    /*
                        The points travel in whole twips. A client that edits the path says which
                        point moved and where to, so nothing of what it was given comes back, and
                        a path of many thousand points stays short on the way out.
                    */
                    auto aPointNode = rWriter.startStruct();
                    const basegfx::B2DPoint aPoint(rPolygon.getB2DPoint(nPoint));
                    rWriter.put("x", basegfx::fround<sal_Int64>(aPoint.getX()));
                    rWriter.put("y", basegfx::fround<sal_Int64>(aPoint.getY()));

                    // The two weights of a curve, each written only where it is in use. They are
                    // named as a handle names them: the one behind the point and the one ahead.
                    if (rPolygon.isPrevControlPointUsed(nPoint))
                    {
                        const basegfx::B2DPoint aBehind(rPolygon.getPrevControlPoint(nPoint));
                        rWriter.put("behindX", basegfx::fround<sal_Int64>(aBehind.getX()));
                        rWriter.put("behindY", basegfx::fround<sal_Int64>(aBehind.getY()));
                    }
                    if (rPolygon.isNextControlPointUsed(nPoint))
                    {
                        const basegfx::B2DPoint aAhead(rPolygon.getNextControlPoint(nPoint));
                        rWriter.put("aheadX", basegfx::fround<sal_Int64>(aAhead.getX()));
                        rWriter.put("aheadY", basegfx::fround<sal_Int64>(aAhead.getY()));
                    }

                    /*
                        How the curve carries on through the point: 1 where the two weights lie on
                        one line through it, 2 where they lie on one line and are the same length
                        as well. It is worked out here from the points as the model holds them,
                        which are of a finer grain than the twips that travel, and rounding them
                        first can turn either answer into the other.
                    */
                    const basegfx::B2VectorContinuity eContinuity(
                        rPolygon.getContinuityInPoint(nPoint));
                    if (eContinuity != basegfx::B2VectorContinuity::NONE)
                        rWriter.put("continuity",
                                    eContinuity == basegfx::B2VectorContinuity::C2 ? 2 : 1);
                }
            }
        }
        if (!rContent.maHandles.empty())
        {
            auto aHandleArray = rWriter.startArray("handles");
            for (const auto& rHandle : rContent.maHandles)
            {
                auto aHandleNode = rWriter.startStruct();
                rWriter.put("kind", rHandle.mnKind);
                if (rHandle.mnPolygon)
                    rWriter.put("polygon", sal_Int64(rHandle.mnPolygon));
                if (rHandle.mnPoint)
                    rWriter.put("point", sal_Int64(rHandle.mnPoint));
                if (rHandle.mnObjHandle)
                    rWriter.put("at", sal_Int64(rHandle.mnObjHandle));
                rWriter.put("x", sal_Int64(rHandle.maPosition.X()));
                rWriter.put("y", sal_Int64(rHandle.maPosition.Y()));

                if (rHandle.mbHasRails)
                {
                    const auto& rRails = rHandle.maRails;
                    auto aRailNode = rWriter.startNode("rails");

                    // A place is written on the object's own square, where 0 and 0 is its
                    // upper left corner and 1 and 1 its lower right one.
                    auto aPlace = [&rWriter](const char* pName, const bool bHas,
                                             const basegfx::B2DPoint& rPlace) {
                        if (!bHas)
                            return;

                        auto aNode = rWriter.startNode(pName);
                        rWriter.put("x", rPlace.getX());
                        rWriter.put("y", rPlace.getY());
                    };

                    if (SdrHandleRailKind::Around == rRails.meKind)
                    {
                        rWriter.put("movesAround", true);
                        aPlace("around", true, rRails.maAround);
                        aPlace("nearest", rRails.mbHasNearest, rRails.maNearest);
                        aPlace("furthest", rRails.mbHasFurthest, rRails.maFurthest);
                        aPlace("from", rRails.mbHasFrom, rRails.maFrom);
                        aPlace("to", rRails.mbHasTo, rRails.maTo);
                    }
                    else
                    {
                        if (rRails.mbMovesAcross)
                            rWriter.put("movesAcross", true);
                        if (rRails.mbMovesDown)
                            rWriter.put("movesDown", true);
                        aPlace("leastAcross", rRails.mbHasLeastAcross, rRails.maLeastAcross);
                        aPlace("mostAcross", rRails.mbHasMostAcross, rRails.maMostAcross);
                        aPlace("leastDown", rRails.mbHasLeastDown, rRails.maLeastDown);
                        aPlace("mostDown", rRails.mbHasMostDown, rRails.maMostDown);
                    }

                    if (rRails.mbKeepsItsLength)
                        rWriter.put("keepsItsLength", true);
                }
            }
        }
    }

    SdDrawDocument* mpDocument;
    SdXImpressDocument* mpModel;
    OString maPartId;
    sal_Int32 mnMode;
    sal_Int64 mnSinceVersion = -1;
    bool mbPush = false;
    bool mbResolved = false;
    SdPage* mpPage = nullptr;
    drawinglayer::geometry::ViewInformation2D maViewInformation;
    std::optional<drawinglayer::Primitive2dJsonProcessor> maProcessor;
    /// The content of the text edit entry of each editing view, by view id, for this write.
    std::unordered_map<sal_Int32, SdXImpressDocument::VectorObjectContent> maTextEditContent;
    /// The content built for an object in this write, by object id.
    std::unordered_map<sal_uInt64, SdXImpressDocument::VectorObjectContent> maResolvedContent;
    /// The page entry's content, the master part it names and the layers of that master it
    /// leaves out, built once per write.
    std::optional<SdXImpressDocument::VectorObjectContent> moPageContent;
    OString maPageMasterPartId;
    std::vector<sal_Int32> maPageMasterHiddenLayers;
    /// The painted objects of the part and the views of the document, collected once per write.
    std::optional<std::vector<SdrObject*>> moPartObjects;
    std::optional<std::vector<EditingView>> moViews;
};

} // anonymous namespace

bool SdXImpressDocument::supportsCommand(std::u16string_view rCommand)
{
    if (rCommand == u"VectorPrimitives" || rCommand == u"VectorRenderingGraphics"
        || rCommand == u"VectorRenderingFont" || rCommand == u"ExtractDocumentStructure"
        || rCommand == u"GetDesignTemplates" || rCommand == u"GetDesignTemplateDesigns"
        || rCommand == u"SecurityLabel")
        return true;
    return false;
}

namespace
{
// Reads the thumbnail image embedded in a template document and returns it as a
// base64 PNG data URL. Returns an empty string when the document has no
// thumbnail or cannot be opened.
OUString lcl_ReadTemplateThumbnailDataUrl(const OUString& rUrl)
{
    try
    {
        uno::Reference<lang::XSingleServiceFactory> xFactory
            = embed::StorageFactory::create(comphelper::getProcessComponentContext());
        cpo::uno::Sequence<beans::PropertyValue> aDescriptor{ comphelper::makePropertyValue(
            u"StorageFormat"_ustr, sal_Int32(embed::StorageFormats::PACKAGE)) };
        cpo::uno::Sequence<cpo::uno::Any> aArgs{ cpo::uno::Any(rUrl),
                                                 cpo::uno::Any(embed::ElementModes::READ),
                                                 cpo::uno::Any(aDescriptor) };
        uno::Reference<embed::XHierarchicalStorageAccess> xStorage
            = xFactory->createInstanceWithArguments(aArgs)
                  .queryThrow<embed::XHierarchicalStorageAccess>();

        uno::Reference<io::XStream> xStream = xStorage->openStreamElementByHierarchicalName(
            u"Thumbnails/thumbnail.png"_ustr, embed::ElementModes::READ);
        uno::Reference<io::XInputStream> xInput = xStream.is() ? xStream->getInputStream()
                                                               : uno::Reference<io::XInputStream>();
        if (!xInput.is())
            return OUString();

        std::vector<sal_Int8> aBytes;
        cpo::uno::Sequence<sal_Int8> aChunk;
        sal_Int32 nRead = 0;
        do
        {
            nRead = xInput->readBytes(aChunk, 16384);
            aBytes.insert(aBytes.end(), aChunk.getConstArray(), aChunk.getConstArray() + nRead);
        } while (nRead > 0);
        xInput->closeInput();

        if (aBytes.empty())
            return OUString();

        OUStringBuffer aBase64;
        comphelper::Base64::encode(
            aBase64, cpo::uno::Sequence<sal_Int8>(aBytes.data(),
                                                  static_cast<sal_Int32>(aBytes.size())));
        return u"data:image/png;base64,"_ustr + aBase64.makeStringAndClear();
    }
    catch (const cpo::uno::Exception&)
    {
        return OUString();
    }
}
}

void SdXImpressDocument::getCommandValues(::tools::JsonWriter& rJsonWriter,
                                          std::string_view rCommand)
{
    std::map<OUString, OUString> aMap
        = KitHelper::parseCommandParameters(OUString::fromUtf8(rCommand));

    if (o3tl::starts_with(rCommand, ".uno:SecurityLabel"))
    {
        // The document's STANAG marking (empty when unlabelled), for the browser's
        // read-only classification banner. Rendered with the label's provisioned policy.
        rJsonWriter.put("commandName", ".uno:SecurityLabel");
        const cpo::uno::Reference<css::frame::XModel> xModel
            = mpDocShell ? mpDocShell->GetModel() : nullptr;
        const OUString aMarking = xModel.is() ? svx::seclabel::readMarking(xModel) : OUString();
        auto aValues = rJsonWriter.startNode("commandValues");
        rJsonWriter.put("marking", aMarking);
        // Whether this document's format can carry a label; the browser hides the
        // command and its button when false (only OOXML supports the customXml part).
        rJsonWriter.put("supported", xModel.is() && svx::seclabel::modelSupportsLabel(xModel));
        return;
    }

    if (o3tl::starts_with(rCommand, ".uno:ExtractDocumentStructure"))
    {
        OUString aFilter;
        if (auto it = aMap.find(u"filter"_ustr); it != aMap.end())
            aFilter = it->second;

        auto aStructureNode = rJsonWriter.startNode("DocStructure");
        if (aFilter == "text" || aFilter.startsWith("text,"))
            GetDocStructureSlidesText(rJsonWriter, this);
        else
            GetDocStructureSlides(rJsonWriter, this, aMap);
    }
    else if (o3tl::starts_with(rCommand, ".uno:GetDesignTemplateDesigns"))
    {
        // One template's slide layouts and design masters, read from the
        // template document. Deriving them loads the document, so they are
        // queried per template on demand instead of for the whole list at once.
        // This is a separate command from the template list so the two replies
        // are told apart by their command name, not by parsing the query. The
        // layouts are the distinct layouts the example slides cover. The masters
        // are the designs the deck can be themed with, each with the part it
        // plays, so the model can put each slide on the design that fits its
        // role.
        auto itName = aMap.find(u"name"_ustr);
        if (itName != aMap.end())
        {
            // The command name echoes the query so the reply matches the request
            // that asked for this particular template.
            rJsonWriter.put("commandName", rCommand);
            auto aValues = rJsonWriter.startNode("commandValues");

            OUString aTemplateUrl;
            for (const auto& [rName, rUrl] : sd::CollectDesignTemplates())
            {
                if (rName.equalsIgnoreAsciiCase(itName->second))
                {
                    aTemplateUrl = rUrl;
                    break;
                }
            }

            SdDrawDocument* pTemplate
                = (!aTemplateUrl.isEmpty() && mpDoc) ? mpDoc->OpenBookmarkDoc(aTemplateUrl)
                                                     : nullptr;
            if (pTemplate)
            {
                {
                    auto aLayoutsNode = rJsonWriter.startArray("layouts");
                    std::set<AutoLayout> aSeenLayouts;
                    const sal_uInt16 nSlides = pTemplate->GetSdPageCount(PageKind::Standard);
                    for (sal_uInt16 i = 0; i < nSlides; ++i)
                    {
                        SdPage* pSlide = pTemplate->GetSdPage(i, PageKind::Standard);
                        if (!pSlide || !aSeenLayouts.insert(pSlide->GetAutoLayout()).second)
                            continue;
                        rJsonWriter.putSimpleValue(
                            SdPage::autoLayoutToString(pSlide->GetAutoLayout()));
                    }
                }
                {
                    auto aMastersNode = rJsonWriter.startArray("masters");
                    for (const auto& rMaster : sd::CollectDesignTemplateMasters(*pTemplate))
                    {
                        auto aEntry = rJsonWriter.startStruct();
                        rJsonWriter.put("name", rMaster.maName);
                        // A master that plays no part reports as "other" and is
                        // not offered to the model as a part.
                        rJsonWriter.put("role", sd::DesignRoleToWireName(rMaster.meRole));
                        // Whether the part came from the template's declared
                        // manifest or from the name-and-example heuristic. This
                        // tells a template author whether their manifest was
                        // read.
                        rJsonWriter.put("source",
                                        rMaster.mbDeclared ? "declared" : "inferred");
                    }
                }
                // A declared manifest may also carry an image art-direction
                // sentence and content budgets. They are reported only when the
                // manifest states them, so a template with no manifest leaves
                // them out and the server keeps its own defaults.
                if (std::optional<sd::AIDesignManifest> oManifest
                    = sd::ReadAIDesignManifest(*pTemplate))
                {
                    if (!oManifest->maArtDirection.isEmpty())
                        rJsonWriter.put("artDirection", oManifest->maArtDirection);
                    if (oManifest->moMaxSlides || oManifest->moMaxItemsPerBullets
                        || oManifest->moMaxItemLength || oManifest->moMaxTitleLength)
                    {
                        auto aBudgets = rJsonWriter.startNode("budgets");
                        if (oManifest->moMaxSlides)
                            rJsonWriter.put("maxSlides", *oManifest->moMaxSlides);
                        if (oManifest->moMaxItemsPerBullets)
                            rJsonWriter.put("maxItemsPerBullets",
                                            *oManifest->moMaxItemsPerBullets);
                        if (oManifest->moMaxItemLength)
                            rJsonWriter.put("maxItemLength", *oManifest->moMaxItemLength);
                        if (oManifest->moMaxTitleLength)
                            rJsonWriter.put("maxTitleLength", *oManifest->moMaxTitleLength);
                    }
                }
                mpDoc->CloseBookmarkDoc();
            }
        }
    }
    else if (o3tl::starts_with(rCommand, ".uno:GetDesignTemplates"))
    {
        // The design templates the user can pick from to style a generated
        // deck: the bundled set plus any preset templates, each with its
        // thumbnail as a PNG data URL. Wrapped as commandName plus
        // commandValues, the shape every commandvalues response carries.
        rJsonWriter.put("commandName", ".uno:GetDesignTemplates");
        auto aValues = rJsonWriter.startNode("commandValues");
        auto aTemplatesNode = rJsonWriter.startArray("templates");
        for (const auto& [rName, rTemplateUrl] : sd::CollectDesignTemplates())
        {
            auto aEntry = rJsonWriter.startStruct();
            rJsonWriter.put("name", rName);
            const OUString aThumbnail = lcl_ReadTemplateThumbnailDataUrl(rTemplateUrl);
            if (!aThumbnail.isEmpty())
                rJsonWriter.put("thumbnail", aThumbnail);
        }
    }
    else if (o3tl::starts_with(rCommand, ".uno:VectorRenderingGraphics"))
    {
        auto it = aMap.find(u"checksum"_ustr);
        if (it != aMap.end())
        {
            sal_Int64 nChecksum = it->second.toInt64();
            // The response always carries the type and the requested
            // checksum, and then either the image data or an error
            // attribute naming why there is none.
            rJsonWriter.put("type", "vectorrenderinggraphics");
            rJsonWriter.put("checksum", nChecksum);
            auto itGraphic = maBitmapCache.find(nChecksum);
            if (itGraphic != maBitmapCache.end())
            {
                drawinglayer::Primitive2dJsonProcessor::writeGraphicBase64(
                    rJsonWriter, itGraphic->second);
            }
            else
            {
                rJsonWriter.put("error", "notfound");
            }
        }
    }
    else if (o3tl::starts_with(rCommand, ".uno:VectorRenderingFont"))
    {
        auto aIdIterator = aMap.find(u"id"_ustr);
        if (aIdIterator != aMap.end())
        {
            sal_uInt64 nFontId = aIdIterator->second.toUInt64(16);
            rJsonWriter.put("type", "vectorrenderingfont");
            rJsonWriter.put("fontId", OString::number(nFontId, 16));

            auto aFontIterator = maVectorFontCache.find(nFontId);
            if (aFontIterator != maVectorFontCache.end())
            {
                const BinaryDataContainer& rBytes = aFontIterator->second;
                const cpo::uno::Sequence<sal_Int8> aSequence(
                    reinterpret_cast<const sal_Int8*>(rBytes.getData()),
                    static_cast<sal_Int32>(rBytes.getSize()));
                OStringBuffer aBase64;
                comphelper::Base64::encode(aBase64, aSequence);
                rJsonWriter.put("data", aBase64.makeStringAndClear());
            }
        }
    }
    else if (o3tl::starts_with(rCommand, ".uno:VectorPrimitives"))
    {
        // A page is named by its part id, the page GUID.
        OString aPartId;
        auto aPartIdIterator = aMap.find(u"partid"_ustr);
        if (aPartIdIterator != aMap.end())
            aPartId = OUStringToOString(aPartIdIterator->second, RTL_TEXTENCODING_ASCII_US);

        // No mode named means the slides.
        sal_Int32 nMode = constVectorModeSlides;
        auto aModeIterator = aMap.find(u"mode"_ustr);
        if (aModeIterator != aMap.end())
            nMode = aModeIterator->second.toInt32();

        // A "since" version asks for a delta: only the objects that
        // changed after that version, plus the current object order.
        sal_Int64 nSinceVersion = -1;
        auto aSinceIterator = aMap.find(u"since"_ustr);
        if (aSinceIterator != aMap.end())
            nSinceVersion = aSinceIterator->second.toInt64();

        // The epoch names the version space the client's version counts in. A version from
        // another epoch, or one that names no epoch, says nothing about this model, so the client
        // is served the page whole.
        auto aEpochIterator = aMap.find(u"epoch"_ustr);
        if (aEpochIterator == aMap.end() || aEpochIterator->second.toInt32() != getVectorEpoch())
            nSinceVersion = -1;

        writeVectorPrimitives(rJsonWriter, aPartId, nMode, nSinceVersion, /*bPush*/ false);
    }
}

void SdXImpressDocument::pushVectorPrimitivesDelta(::tools::JsonWriter& rJsonWriter,
                                                   std::string_view rPartId, int nMode)
{
    writeVectorPrimitives(rJsonWriter, OString(rPartId), nMode, /*nSinceVersion*/ -1,
                          /*bPush*/ true);
}

void SdXImpressDocument::writeVectorPrimitives(::tools::JsonWriter& rJsonWriter,
                                               const OString& rPartId, sal_Int32 nMode,
                                               sal_Int64 nSinceVersion, bool bPush)
{
    if (!mpDoc)
        return;

    // This reader draws from the model, so an open edit has to broadcast.
    mpDoc->SetDrawnFromModel(true);

    // This render is not the view's own. The text edit measures a selection while it is
    // decomposed, and the flag keeps that measurement from reaching the client as the view's
    // selection.
    comphelper::COKit::setVectorRendering(true);
    comphelper::ScopeGuard aVectorRenderingGuard(
        [] { comphelper::COKit::setVectorRendering(false); });

    VectorContentWriter aContentWriter(mpDoc, this, rPartId, nMode);
    SdPage* pPage = aContentWriter.resolvePage();

    // A version above the part's own comes from content this model never served, so the page
    // is sent whole.
    if (pPage && nSinceVersion > sal_Int64(getVectorPartVersion(*pPage)))
        nSinceVersion = -1;

    // A push steps from the version the part was last pushed at, then advances that mark. One
    // delta is written for the part and every client that holds the part reads that same one. A
    // page the document does not hold has no mark to move.
    if (bPush)
    {
        if (!pPage)
            return;
        nSinceVersion = sal_Int64(maVectorParts[vectorPartKeyOf(*pPage)].mnLastSentVersion);
    }

    aContentWriter.setSinceVersion(nSinceVersion);
    aContentWriter.setPush(bPush);
    aContentWriter.write(rJsonWriter);

    if (!pPage)
        return;

    VectorPartState& rState = maVectorParts[vectorPartKeyOf(*pPage)];
    if (bPush)
    {
        rState.mnLastSentVersion = getVectorPartVersion(*pPage);
    }
    else
    {
        // A pull moves the mark only when nothing has served the part yet. Nothing holds the
        // part at that point, so no reader has a step to make, and starting the mark at what
        // this response carries keeps the delta after it down to what moved after it.
        //
        // A later pull leaves the mark where it is. It serves the one reader that asked, while
        // the mark stands for where all of them are, and the readers behind it still need the
        // step from there written for them. That holds when the pull itself counted the version
        // up: the readers were not told, so the step from where they stand is still to be
        // written.
        if (!rState.mbServed)
        {
            rState.mbServed = true;
            rState.mnLastSentVersion = getVectorPartVersion(*pPage);
        }
    }
}

/******************************************************************************
*                                                                             *
******************************************************************************/
SdPage* SdXImpressDocument::InsertSdPage( sal_uInt16 nPage, bool bDuplicate )
{
    sal_uInt16 nPageCount = mpDoc->GetSdPageCount( PageKind::Standard );
    SdrLayerAdmin& rLayerAdmin = mpDoc->GetLayerAdmin();
    SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
    SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);

    rtl::Reference<SdPage> pStandardPage;

    if( 0 == nPageCount )
    {
        // this is only used for clipboard where we only have one page
        pStandardPage = mpDoc->AllocSdPage(false);

        Size aDefSize(21000, 29700);   // A4 portrait orientation
        pStandardPage->SetSize( aDefSize );
        mpDoc->InsertPage(pStandardPage.get(), 0);
    }
    else
    {
        // here we determine the page after which we should insert
        SdPage* pPreviousStandardPage = mpDoc->GetSdPage( std::min( static_cast<sal_uInt16>(nPageCount - 1), nPage ), PageKind::Standard );
        SdrLayerIDSet aVisibleLayers = pPreviousStandardPage->TRG_GetMasterPageVisibleLayers();
        bool bIsPageBack = aVisibleLayers.IsSet( aBckgrnd );
        bool bIsPageObj = aVisibleLayers.IsSet( aBckgrndObj );

        // AutoLayouts must be ready
        mpDoc->StopWorkStartupDelay();

        /* First we create a standard page and then a notes page. It is
           guaranteed, that after a standard page the corresponding notes page
           follows. */

        sal_uInt16 nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        SdPage* pPreviousNotesPage = static_cast<SdPage*>( mpDoc->GetPage( nStandardPageNum - 1 ) );
        sal_uInt16 nNotesPageNum = nStandardPageNum + 1;

        /**************************************************************
        * standard page
        **************************************************************/
        if( bDuplicate )
            pStandardPage = static_cast<SdPage*>( pPreviousStandardPage->CloneSdrPage(*mpDoc).get() );
        else
            pStandardPage = mpDoc->AllocSdPage(false);

        pStandardPage->SetSize( pPreviousStandardPage->GetSize() );
        pStandardPage->SetBorder( pPreviousStandardPage->GetLeftBorder(),
                                    pPreviousStandardPage->GetUpperBorder(),
                                    pPreviousStandardPage->GetRightBorder(),
                                    pPreviousStandardPage->GetLowerBorder() );
        pStandardPage->SetOrientation( pPreviousStandardPage->GetOrientation() );
        pStandardPage->SetName(OUString());

        // insert page after current page
        mpDoc->InsertPage(pStandardPage.get(), nStandardPageNum);

        if( !bDuplicate )
        {
            // use MasterPage of the current page
            pStandardPage->TRG_SetMasterPage(pPreviousStandardPage->TRG_GetMasterPage());
            pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
            pStandardPage->SetAutoLayout(AUTOLAYOUT_NONE, true );
        }

        aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
        aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
        aVisibleLayers.Set(aBckgrnd, bIsPageBack);
        aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
        pStandardPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);

        /**************************************************************
        * notes page
        **************************************************************/
        rtl::Reference<SdPage> pNotesPage;

        if( bDuplicate )
            pNotesPage = static_cast<SdPage*>( pPreviousNotesPage->CloneSdrPage(*mpDoc).get() );
        else
            pNotesPage = mpDoc->AllocSdPage(false);

        pNotesPage->SetSize( pPreviousNotesPage->GetSize() );
        pNotesPage->SetBorder( pPreviousNotesPage->GetLeftBorder(),
                                pPreviousNotesPage->GetUpperBorder(),
                                pPreviousNotesPage->GetRightBorder(),
                                pPreviousNotesPage->GetLowerBorder() );
        pNotesPage->SetOrientation( pPreviousNotesPage->GetOrientation() );
        pNotesPage->SetName(OUString());
        pNotesPage->SetPageKind(PageKind::Notes);

        // insert page after current page
        mpDoc->InsertPage(pNotesPage.get(), nNotesPageNum);

        if( !bDuplicate )
        {
            // use MasterPage of the current page
            pNotesPage->TRG_SetMasterPage(pPreviousNotesPage->TRG_GetMasterPage());
            pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
            pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true );
        }
    }

    SetModified();

    return pStandardPage.get();
}

void SdXImpressDocument::SetModified() noexcept
{
    if( mpDoc )
        mpDoc->SetChanged();
}

// XModel
void SdXImpressDocument::lockControllers(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    mpDoc->setLock(true);
}

void SdXImpressDocument::unlockControllers(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if( mpDoc->isLocked() )
    {
        mpDoc->setLock(false);
    }
}

bool SdXImpressDocument::hasControllersLocked(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    return mpDoc->isLocked();
}

uno::Reference < container::XIndexAccess > SdXImpressDocument::getViewData()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference < container::XIndexAccess > xRet( SfxBaseModel::getViewData() );

    if( !xRet.is() )
    {
        const std::vector<std::unique_ptr<sd::FrameView>> &rList = mpDoc->GetFrameViewList();

        if( !rList.empty() )
        {
            xRet = new comphelper::IndexedPropertyValuesContainer();

            uno::Reference < container::XIndexContainer > xCont( xRet, uno::UNO_QUERY );
            DBG_ASSERT( xCont.is(), "SdXImpressDocument::getViewData() failed for OLE object" );
            if( xCont.is() )
            {
                for( sal_uInt32 i = 0, n = rList.size(); i < n; i++ )
                {
                    ::sd::FrameView* pFrameView = rList[ i ].get();

                    cpo::uno::Sequence< beans::PropertyValue > aSeq;
                    pFrameView->WriteUserDataSequence( aSeq );
                    xCont->insertByIndex( i, cpo::uno::Any( aSeq ) );
                }
            }
        }
    }

    return xRet;
}

void SdXImpressDocument::setViewData( const uno::Reference < container::XIndexAccess >& xData )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    SfxBaseModel::setViewData( xData );
    if( !(mpDocShell && (mpDocShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED) && xData.is()) )
        return;

    const sal_Int32 nCount = xData->getCount();

    std::vector<std::unique_ptr<sd::FrameView>> &rViews = mpDoc->GetFrameViewList();

    rViews.clear();

    cpo::uno::Sequence< beans::PropertyValue > aSeq;
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        if( xData->getByIndex( nIndex ) >>= aSeq )
        {
            std::unique_ptr<::sd::FrameView> pFrameView(new ::sd::FrameView( mpDoc ));
            pFrameView->ReadUserDataSequence( aSeq );
            rViews.push_back( std::move(pFrameView) );
        }
    }
}

// XDrawPageDuplicator
uno::Reference< drawing::XDrawPage > SdXImpressDocument::duplicate( const uno::Reference< drawing::XDrawPage >& xPage )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    // get pPage from xPage and determine the Id (nPos ) afterwards
    SvxDrawPage* pSvxPage = comphelper::getFromUnoTunnel<SvxDrawPage>( xPage );
    if( pSvxPage )
    {
        SdPage* pPage = static_cast<SdPage*>( pSvxPage->GetSdrPage() );
        sal_uInt16 nPos = pPage->GetPageNum();
        nPos = ( nPos - 1 ) / 2;
        pPage = InsertSdPage( nPos, true );
        if( pPage )
        {
            uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage() );
            return xDrawPage;
        }
    }

    uno::Reference< drawing::XDrawPage > xDrawPage;
    return xDrawPage;
}

// XDrawPagesSupplier
uno::Reference< drawing::XDrawPages > SdXImpressDocument::getDrawPages()
{
    ::SolarMutexGuard aGuard;

    return getSdDrawPages();
}

rtl::Reference< SdDrawPagesAccess > SdXImpressDocument::getSdDrawPages()
{
    if( nullptr == mpDoc )
        throw lang::DisposedException();

    rtl::Reference< SdDrawPagesAccess > xDrawPages( mxDrawPagesAccess );

    if( !xDrawPages.is() )
    {
        initializeDocument();
        xDrawPages = new SdDrawPagesAccess(*this);
        mxDrawPagesAccess = xDrawPages.get();
    }

    return xDrawPages;
}

// XMasterPagesSupplier
uno::Reference< drawing::XDrawPages > SdXImpressDocument::getMasterPages()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    rtl::Reference< SdMasterPagesAccess > xMasterPages( mxMasterPagesAccess );

    if( !xMasterPages.is() )
    {
        if ( !hasControllersLocked() )
            initializeDocument();
        xMasterPages = new SdMasterPagesAccess(*this);
        mxMasterPagesAccess = xMasterPages.get();
    }

    return xMasterPages;
}

// XLayerManagerSupplier
uno::Reference< container::XNameAccess > SdXImpressDocument::getLayerManager(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    rtl::Reference< SdLayerManager >  xLayerManager( mxLayerManager );

    if( !xLayerManager.is() )
    {
        xLayerManager = new SdLayerManager(*this);
        mxLayerManager = xLayerManager.get();
    }

    return xLayerManager;
}

// XCustomPresentationSupplier
uno::Reference< container::XNameContainer > SdXImpressDocument::getCustomPresentations()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    rtl::Reference< SdXCustomPresentationAccess >  xCustomPres( mxCustomPresentationAccess );

    if( !xCustomPres.is() )
    {
        xCustomPres = new SdXCustomPresentationAccess(*this);
        mxCustomPresentationAccess = xCustomPres.get();
    }

    return xCustomPres;
}

// XPresentationSupplier
uno::Reference< presentation::XPresentation > SdXImpressDocument::getPresentation()
{
    return getSlideShow();
}

rtl::Reference< sd::SlideShow > SdXImpressDocument::getSlideShow()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    return mpDoc->getSlideShow();
}

// XHandoutMasterSupplier
uno::Reference< drawing::XDrawPage > SdXImpressDocument::getHandoutMasterPage()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPage > xPage;

    initializeDocument();
    SdPage* pPage = mpDoc->GetMasterSdPage(0, PageKind::Handout);
    if (pPage)
        xPage.set(pPage->getUnoPage());
    return xPage;
}

// XMultiServiceFactory ( SvxFmMSFactory )

cpo::uno::Reference<cpo::uno::XInterface> SdXImpressDocument::create(
    OUString const & aServiceSpecifier, OUString const & referer)
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if( aServiceSpecifier == "com.sun.star.drawing.DashTable" )
    {
        if( !mxDashTable.is() )
            mxDashTable = SvxUnoDashTable_createInstance( mpDoc );

        return mxDashTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.GradientTable" )
    {
        if( !mxGradientTable.is() )
            mxGradientTable = SvxUnoGradientTable_createInstance( mpDoc );

        return mxGradientTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.HatchTable" )
    {
        if( !mxHatchTable.is() )
            mxHatchTable = SvxUnoHatchTable_createInstance( mpDoc );

        return mxHatchTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.BitmapTable" )
    {
        if( !mxBitmapTable.is() )
            mxBitmapTable = SvxUnoBitmapTable_createInstance( mpDoc );

        return mxBitmapTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.TransparencyGradientTable" )
    {
        if( !mxTransGradientTable.is() )
            mxTransGradientTable = SvxUnoTransGradientTable_createInstance( mpDoc );

        return mxTransGradientTable;
    }
    if( aServiceSpecifier == "com.sun.star.drawing.MarkerTable" )
    {
        if( !mxMarkerTable.is() )
            mxMarkerTable = SvxUnoMarkerTable_createInstance( mpDoc );

        return mxMarkerTable;
    }
    if( aServiceSpecifier == "com.sun.star.text.NumberingRules" )
    {
        return uno::Reference< cpo::uno::XInterface >( SvxCreateNumRule( mpDoc ), uno::UNO_QUERY );
    }
    if( aServiceSpecifier == "com.sun.star.drawing.Background" )
    {
        return uno::Reference<cpo::uno::XInterface>(cppu::getXWeak(new SdUnoPageBackground(mpDoc)));
    }

    if( aServiceSpecifier == "com.sun.star.drawing.Defaults" )
    {
        if( !mxDrawingPool.is() )
            mxDrawingPool = SdUnoCreatePool( mpDoc );

        return mxDrawingPool;

    }

    if ( aServiceSpecifier == sUNO_Service_ImageMapRectangleObject )
    {
        return SvUnoImageMapRectangleObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if ( aServiceSpecifier == sUNO_Service_ImageMapCircleObject )
    {
        return SvUnoImageMapCircleObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if ( aServiceSpecifier == sUNO_Service_ImageMapPolygonObject )
    {
        return SvUnoImageMapPolygonObject_createInstance( ImplGetSupportedMacroItems() );
    }

    if( aServiceSpecifier == "com.sun.star.document.Settings" ||
        ( !mbImpressDoc && ( aServiceSpecifier == "com.sun.star.drawing.DocumentSettings" ) ) ||
        (  mbImpressDoc && ( aServiceSpecifier == "com.sun.star.presentation.DocumentSettings" ) ) )
    {
        return sd::DocumentSettings_createInstance( this );
    }

    if( aServiceSpecifier == "com.sun.star.text.TextField.DateTime" ||
        aServiceSpecifier == "com.sun.star.text.textfield.DateTime" )
    {
        return cppu::getXWeak(new SvxUnoTextField(text::textfield::Type::DATE));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.Header" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.Header" )
    {
        return cppu::getXWeak(new SvxUnoTextField(text::textfield::Type::PRESENTATION_HEADER));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.Footer" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.Footer" )
    {
        return cppu::getXWeak(new SvxUnoTextField(text::textfield::Type::PRESENTATION_FOOTER));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.DateTime" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.DateTime" )
    {
        return cppu::getXWeak(new SvxUnoTextField(text::textfield::Type::PRESENTATION_DATE_TIME));
    }

    if( aServiceSpecifier == "com.sun.star.text.TextField.PageName" ||
        aServiceSpecifier == "com.sun.star.text.textfield.PageName" )
    {
        return cppu::getXWeak(new SvxUnoTextField(text::textfield::Type::PAGE_NAME));
    }

    if (aServiceSpecifier == "com.sun.star.text.TextField.DocInfo.Custom" ||
        aServiceSpecifier == "com.sun.star.text.textfield.DocInfo.Custom")
    {
        return cppu::getXWeak(new SvxUnoTextField(text::textfield::Type::DOCINFO_CUSTOM));
    }

    if( aServiceSpecifier == "com.sun.star.xml.NamespaceMap" )
    {
        static const sal_uInt16 aWhichIds[] = { SDRATTR_XMLATTRIBUTES, EE_CHAR_XMLATTRIBS, EE_PARA_XMLATTRIBS, 0 };

        return svx::NamespaceMap_createInstance( aWhichIds, &mpDoc->GetItemPool() );
    }

    // Support creation of GraphicStorageHandler and EmbeddedObjectResolver
    if (aServiceSpecifier == "com.sun.star.document.ExportGraphicStorageHandler")
    {
        return cppu::getXWeak(new SvXMLGraphicHelper(SvXMLGraphicHelperMode::Write));
    }

    if (aServiceSpecifier == "com.sun.star.document.ImportGraphicStorageHandler")
    {
        return cppu::getXWeak(new SvXMLGraphicHelper(SvXMLGraphicHelperMode::Read));
    }

    if( aServiceSpecifier == "com.sun.star.document.ExportEmbeddedObjectResolver" )
    {
        comphelper::IEmbeddedHelper* pPersist = mpDoc->GetPersist();
        if( nullptr == pPersist )
            throw lang::DisposedException();

        return cppu::getXWeak(new SvXMLEmbeddedObjectHelper( *pPersist, SvXMLEmbeddedObjectHelperMode::Write ));
    }

    if( aServiceSpecifier == "com.sun.star.document.ImportEmbeddedObjectResolver" )
    {
        comphelper::IEmbeddedHelper* pPersist = mpDoc->GetPersist();
        if( nullptr == pPersist )
            throw lang::DisposedException();

        return cppu::getXWeak(new SvXMLEmbeddedObjectHelper( *pPersist, SvXMLEmbeddedObjectHelperMode::Read ));
    }

    uno::Reference< cpo::uno::XInterface > xRet;

    if (std::u16string_view aType; aServiceSpecifier.startsWith("com.sun.star.presentation.", &aType))
    {
        SdrObjKind nType;
        // create a shape wrapper
        if( o3tl::starts_with(aType, u"TitleTextShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aType, u"OutlinerShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aType, u"SubtitleShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aType, u"GraphicObjectShape" ) )
        {
            nType = SdrObjKind::Graphic;
        }
        else if( o3tl::starts_with(aType, u"PageShape" ) )
        {
            nType = SdrObjKind::Page;
        }
        else if( o3tl::starts_with(aType, u"OLE2Shape" ) )
        {
            nType = SdrObjKind::OLE2;
        }
        else if( o3tl::starts_with(aType, u"ChartShape" ) )
        {
            nType = SdrObjKind::OLE2;
        }
        else if( o3tl::starts_with(aType, u"CalcShape" ) )
        {
            nType = SdrObjKind::OLE2;
        }
        else if( o3tl::starts_with(aType, u"TableShape" ) )
        {
            nType = SdrObjKind::Table;
        }
        else if( o3tl::starts_with(aType, u"OrgChartShape" ) )
        {
            nType = SdrObjKind::OLE2;
        }
        else if( o3tl::starts_with(aType, u"NotesShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aType, u"HandoutShape" ) )
        {
            nType = SdrObjKind::Page;
        }
        else if( o3tl::starts_with(aType, u"FooterShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aType, u"HeaderShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aType, u"SlideNumberShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aType, u"DateTimeShape" ) )
        {
            nType = SdrObjKind::Text;
        }
        else if( o3tl::starts_with(aType, u"MediaShape" ) )
        {
            nType = SdrObjKind::Media;
        }
        else
        {
            throw lang::ServiceNotRegisteredException();
        }

        // create the API wrapper
        rtl::Reference<SvxShape> pShape = CreateSvxShapeByTypeAndInventor(nType, SdrInventor::Default, referer);

        // set shape type
        if( pShape && !mbClipBoard )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = cppu::getXWeak(pShape.get());
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.TableShape" )
    {
        rtl::Reference<SvxShape> pShape = CreateSvxShapeByTypeAndInventor( SdrObjKind::Table, SdrInventor::Default, referer );
        if( pShape && !mbClipBoard )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = cppu::getXWeak(pShape.get());
    }
    else
    {
        xRet = SvxFmMSFactory::createInstance( aServiceSpecifier );
    }

    uno::Reference< drawing::XShape > xShape( xRet, uno::UNO_QUERY );
    SvxShape* pShape = xShape.is() ? comphelper::getFromUnoTunnel<SvxShape>(xShape) : nullptr;
    if (pShape)
    {
        xRet.clear();
        new SdXShape( pShape, this );
        xRet = xShape;
        xShape.clear();
    }

    return xRet;
}

uno::Reference< cpo::uno::XInterface > SdXImpressDocument::createInstance( const OUString& aServiceSpecifier )
{
    return create(aServiceSpecifier, u""_ustr);
}

cpo::uno::Reference<cpo::uno::XInterface>
SdXImpressDocument::createInstanceWithArguments(
    OUString const & ServiceSpecifier,
    cpo::uno::Sequence<cpo::uno::Any> const & Arguments)
{
    OUString arg;
    if ((ServiceSpecifier == "com.sun.star.drawing.GraphicObjectShape"
         || ServiceSpecifier == "com.sun.star.drawing.FrameShape"
         || ServiceSpecifier == "com.sun.star.drawing.OLE2Shape"
         || ServiceSpecifier == "com.sun.star.drawing.MediaShape"
         || ServiceSpecifier == "com.sun.star.presentation.MediaShape")
        && Arguments.getLength() == 1 && (Arguments[0] >>= arg))
    {
        return create(ServiceSpecifier, arg);
    }
    return SvxFmMSFactory::createInstanceWithArguments(
        ServiceSpecifier, Arguments);
}

cpo::uno::Sequence< OUString > SdXImpressDocument::getAvailableServiceNames()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    const cpo::uno::Sequence< OUString > aSNS_ORG( SvxFmMSFactory::getAvailableServiceNames() );

    cpo::uno::Sequence< OUString > aSNS_Common{ u"com.sun.star.drawing.DashTable"_ustr,
                                           u"com.sun.star.drawing.GradientTable"_ustr,
                                           u"com.sun.star.drawing.HatchTable"_ustr,
                                           u"com.sun.star.drawing.BitmapTable"_ustr,
                                           u"com.sun.star.drawing.TransparencyGradientTable"_ustr,
                                           u"com.sun.star.drawing.MarkerTable"_ustr,
                                           u"com.sun.star.text.NumberingRules"_ustr,
                                           u"com.sun.star.drawing.Background"_ustr,
                                           u"com.sun.star.document.Settings"_ustr,
                                           sUNO_Service_ImageMapRectangleObject,
                                           sUNO_Service_ImageMapCircleObject,
                                           sUNO_Service_ImageMapPolygonObject,
                                           u"com.sun.star.xml.NamespaceMap"_ustr,

                                           // Support creation of GraphicStorageHandler and EmbeddedObjectResolver
                                           u"com.sun.star.document.ExportGraphicStorageHandler"_ustr,
                                           u"com.sun.star.document.ImportGraphicStorageHandler"_ustr,
                                           u"com.sun.star.document.ExportEmbeddedObjectResolver"_ustr,
                                           u"com.sun.star.document.ImportEmbeddedObjectResolver"_ustr,
                                           u"com.sun.star.drawing.TableShape"_ustr };

    cpo::uno::Sequence< OUString > aSNS_Specific;

    if(mbImpressDoc)
        aSNS_Specific = { u"com.sun.star.presentation.TitleTextShape"_ustr,
                          u"com.sun.star.presentation.OutlinerShape"_ustr,
                          u"com.sun.star.presentation.SubtitleShape"_ustr,
                          u"com.sun.star.presentation.GraphicObjectShape"_ustr,
                          u"com.sun.star.presentation.ChartShape"_ustr,
                          u"com.sun.star.presentation.PageShape"_ustr,
                          u"com.sun.star.presentation.OLE2Shape"_ustr,
                          u"com.sun.star.presentation.TableShape"_ustr,
                          u"com.sun.star.presentation.OrgChartShape"_ustr,
                          u"com.sun.star.presentation.NotesShape"_ustr,
                          u"com.sun.star.presentation.HandoutShape"_ustr,
                          u"com.sun.star.presentation.DocumentSettings"_ustr,
                          u"com.sun.star.presentation.FooterShape"_ustr,
                          u"com.sun.star.presentation.HeaderShape"_ustr,
                          u"com.sun.star.presentation.SlideNumberShape"_ustr,
                          u"com.sun.star.presentation.DateTimeShape"_ustr,
                          u"com.sun.star.presentation.CalcShape"_ustr,
                          u"com.sun.star.presentation.MediaShape"_ustr };
    else
        aSNS_Specific = { u"com.sun.star.drawing.DocumentSettings"_ustr };

    return comphelper::concatSequences( aSNS_ORG, aSNS_Common, aSNS_Specific );
}

// lang::XServiceInfo
OUString SdXImpressDocument::getImplementationName()
{
    return u"SdXImpressDocument"_ustr;
    /* // Matching the .component information:
       return mbImpressDoc
           ? OUString("com.sun.star.comp.Draw.PresentationDocument")
           : OUString("com.sun.star.comp.Draw.DrawingDocument");
    */
}

bool SdXImpressDocument::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

cpo::uno::Sequence< OUString > SdXImpressDocument::getSupportedServiceNames()
{
    ::SolarMutexGuard aGuard;

    return { u"com.sun.star.document.OfficeDocument"_ustr,
             u"com.sun.star.drawing.GenericDrawingDocument"_ustr,
             u"com.sun.star.drawing.DrawingDocumentFactory"_ustr,
             mbImpressDoc?u"com.sun.star.presentation.PresentationDocument"_ustr:u"com.sun.star.drawing.DrawingDocument"_ustr };
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SdXImpressDocument::getPropertySetInfo(  )
{
    ::SolarMutexGuard aGuard;
    return mpPropSet->getPropertySetInfo();
}

void SdXImpressDocument::setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    const SfxItemPropertyMapEntry* pEntry = mpPropSet->getPropertyMapEntry(aPropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
        case WID_MODEL_LANGUAGE:
        {
            lang::Locale aLocale;
            if(!(aValue >>= aLocale))
                throw lang::IllegalArgumentException();

            mpDoc->SetLanguage( LanguageTag::convertToLanguageType(aLocale), EE_CHAR_LANGUAGE );
            break;
        }
        case WID_MODEL_TABSTOP:
        {
            sal_Int32 nValue = 0;
            if(!(aValue >>= nValue) || nValue < 0 )
                throw lang::IllegalArgumentException();

            mpDoc->SetDefaultTabulator(static_cast<sal_uInt16>(nValue));
            break;
        }
        case WID_MODEL_VISAREA:
            {
                SfxObjectShell* pEmbeddedObj = mpDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                awt::Rectangle aVisArea;
                if( !(aValue >>= aVisArea) || (aVisArea.Width < 0) || (aVisArea.Height < 0) )
                    throw lang::IllegalArgumentException();

                sal_Int32 nRight, nTop;
                if (o3tl::checked_add(aVisArea.X, aVisArea.Width, nRight) || o3tl::checked_add(aVisArea.Y, aVisArea.Height, nTop))
                    throw lang::IllegalArgumentException();

                pEmbeddedObj->SetVisArea(::tools::Rectangle(aVisArea.X, aVisArea.Y, nRight, nTop));
            }
            break;
        case WID_MODEL_CONTFOCUS:
            {
                bool bFocus = false;
                if( !(aValue >>= bFocus ) )
                    throw lang::IllegalArgumentException();
                mpDoc->SetAutoControlFocus( bFocus );
            }
            break;
        case WID_MODEL_DSGNMODE:
            {
                bool bMode = false;
                if( !(aValue >>= bMode ) )
                    throw lang::IllegalArgumentException();
                mpDoc->SetOpenInDesignMode( bMode );
            }
            break;
        case WID_MODEL_BUILDID:
            aValue >>= maBuildId;
            return;
        case WID_MODEL_MAPUNIT:
        case WID_MODEL_BASICLIBS:
        case WID_MODEL_RUNTIMEUID: // is read-only
        case WID_MODEL_DIALOGLIBS:
        case WID_MODEL_FONTS:
            throw beans::PropertyVetoException();
        case WID_MODEL_INTEROPGRABBAG:
            setGrabBagItem(aValue);
            break;
        case WID_MODEL_SLIDESECTIONS:
        {
            cpo::uno::Sequence<beans::PropertyValue> aSections;
            if (aValue >>= aSections)
                mpDoc->GetSectionManager().SetSectionsFromPropertyValues(aSections);
            break;
        }
        case WID_MODEL_ANIMATIONSOUNDLINK:
        {
            bool bHasLink = false;
            if (aValue >>= bHasLink)
                mpDoc->SetMaybeHasAnimationSoundLinks(bHasLink);
            break;
        }
        case WID_MODEL_THEME:
            getSdrModelFromUnoModel().setTheme(model::Theme::FromAny(aValue));
            break;
        default:
            throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    SetModified();
}

cpo::uno::Any SdXImpressDocument::getPropertyValue( const OUString& PropertyName )
{
    ::SolarMutexGuard aGuard;

    cpo::uno::Any aAny;
    if( nullptr == mpDoc )
        throw lang::DisposedException();

    const SfxItemPropertyMapEntry* pEntry = mpPropSet->getPropertyMapEntry(PropertyName);

    switch( pEntry ? pEntry->nWID : -1 )
    {
        case WID_MODEL_LANGUAGE:
        {
            LanguageType eLang = mpDoc->GetLanguage( EE_CHAR_LANGUAGE );
            aAny <<= LanguageTag::convertToLocale( eLang);
            break;
        }
        case WID_MODEL_TABSTOP:
            aAny <<= static_cast<sal_Int32>(mpDoc->GetDefaultTabulator());
            break;
        case WID_MODEL_VISAREA:
            {
                SfxObjectShell* pEmbeddedObj = mpDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                const ::tools::Rectangle& aRect = pEmbeddedObj->GetVisArea();
                awt::Rectangle aVisArea( aRect.Left(), aRect.Top(), aRect.getOpenWidth(), aRect.getOpenHeight() );
                aAny <<= aVisArea;
            }
            break;
        case WID_MODEL_MAPUNIT:
            {
                SfxObjectShell* pEmbeddedObj = mpDoc->GetDocSh();
                if( !pEmbeddedObj )
                    break;

                sal_Int16 nMeasureUnit = 0;
                SvxMapUnitToMeasureUnit( pEmbeddedObj->GetMapUnit(), nMeasureUnit );
                aAny <<= nMeasureUnit;
        }
        break;
        case WID_MODEL_FORBCHARS:
        {
            aAny <<= getForbiddenCharsTable();
        }
        break;
        case WID_MODEL_CONTFOCUS:
            aAny <<= mpDoc->GetAutoControlFocus();
            break;
        case WID_MODEL_DSGNMODE:
            aAny <<= mpDoc->GetOpenInDesignMode();
            break;
        case WID_MODEL_BASICLIBS:
            aAny <<= mpDocShell->GetBasicContainer();
            break;
        case WID_MODEL_DIALOGLIBS:
            aAny <<= mpDocShell->GetDialogContainer();
            break;
        case WID_MODEL_RUNTIMEUID:
            aAny <<= getRuntimeUID();
            break;
        case WID_MODEL_BUILDID:
            return cpo::uno::Any( maBuildId );
        case WID_MODEL_HASVALIDSIGNATURES:
            aAny <<= hasValidSignatures();
            break;
        case WID_MODEL_ALLOWLINKUPDATE:
        {
            comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = mpDocShell->getEmbeddedObjectContainer();
            aAny <<= rEmbeddedObjectContainer.getUserAllowsLinkUpdate();
            break;
        }
        case WID_MODEL_FONTS:
            {
                cpo::uno::Sequence<cpo::uno::Any> aSeq;
                int nSeqIndex = 0;

                sal_uInt16 const aWhichIds[] { EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK,
                                               EE_CHAR_FONTINFO_CTL };

                const SfxItemPool& rPool = mpDoc->GetPool();

                for(sal_uInt16 nWhichId : aWhichIds)
                {
                    ItemSurrogates aSurrogates = rPool.GetItemSurrogates(nWhichId);
                    const sal_uInt32 nItems(aSurrogates.size());

                    aSeq.realloc( aSeq.getLength() + nItems*5 + 5 );
                    auto pSeq = aSeq.getArray();

                    for (const SfxPoolItem* pItem : aSurrogates)
                    {
                        const SvxFontItem *pFont = static_cast<const SvxFontItem *>(pItem);

                        pSeq[nSeqIndex++] <<= pFont->GetFamilyName();
                        pSeq[nSeqIndex++] <<= pFont->GetStyleName();
                        pSeq[nSeqIndex++] <<= sal_Int16(pFont->GetFamily());
                        pSeq[nSeqIndex++] <<= sal_Int16(pFont->GetPitch());
                        pSeq[nSeqIndex++] <<= sal_Int16(pFont->GetCharSet());
                    }

                    const SvxFontItem& rFont = static_cast<const SvxFontItem&>(rPool.GetUserOrPoolDefaultItem( nWhichId ));

                    pSeq[nSeqIndex++] <<= rFont.GetFamilyName();
                    pSeq[nSeqIndex++] <<= rFont.GetStyleName();
                    pSeq[nSeqIndex++] <<= sal_Int16(rFont.GetFamily());
                    pSeq[nSeqIndex++] <<= sal_Int16(rFont.GetPitch());
                    pSeq[nSeqIndex++] <<= sal_Int16(rFont.GetCharSet());

                }

                aAny <<= aSeq;
                break;
            }
        case WID_MODEL_INTEROPGRABBAG:
            getGrabBagItem(aAny);
            break;
        case WID_MODEL_SLIDESECTIONS:
            aAny <<= mpDoc->GetSectionManager().GetSectionsAsPropertyValues();
            break;
        case WID_MODEL_ANIMATIONSOUNDLINK:
            aAny <<= mpDoc->MaybeHasAnimationSoundLinks();
            break;
        case WID_MODEL_THEME:
            if (auto const& pTheme = getSdrModelFromUnoModel().getTheme())
                pTheme->ToAny(aAny);
            break;
        default:
            throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    return aAny;
}

void SdXImpressDocument::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) {}
void SdXImpressDocument::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) {}
void SdXImpressDocument::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) {}
void SdXImpressDocument::removeVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) {}

// XLinkTargetSupplier
uno::Reference< container::XNameAccess > SdXImpressDocument::getLinks()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    rtl::Reference< SdDocLinkTargets > xLinks( mxLinks );
    if( !xLinks.is() )
    {
        xLinks = new SdDocLinkTargets( *this );
        mxLinks = xLinks.get();
    }
    return xLinks;
}

// XStyleFamiliesSupplier
uno::Reference< container::XNameAccess > SdXImpressDocument::getStyleFamilies(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess > xStyles( static_cast< OWeakObject* >( mpDoc->GetStyleSheetPool() ), cpo::uno::UNO_QUERY );
    return xStyles;
}

// XAnyCompareFactory
uno::Reference< css::ucb::XAnyCompare > SdXImpressDocument::createAnyCompareByName( const OUString& )
{
    return SvxCreateNumRuleCompare();
}

// XRenderable
sal_Int32 SdXImpressDocument::getRendererCount( const cpo::uno::Any& rSelection,
                                                         const cpo::uno::Sequence< beans::PropertyValue >&  )
{
    ::SolarMutexGuard aGuard;
    sal_Int32   nRet = 0;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if (mpDocShell)
    {
        uno::Reference< frame::XModel > xModel;

        rSelection >>= xModel;

        if( xModel == mpDocShell->GetModel() )
            nRet = mpDoc->GetSdPageCount( PageKind::Standard );
        else
        {
            uno::Reference< drawing::XShapes > xShapes;

            rSelection >>= xShapes;

            if( xShapes.is() && xShapes->getCount() )
                nRet = 1;
        }
    }
    return nRet;
}

cpo::uno::Sequence< beans::PropertyValue > SdXImpressDocument::getRenderer( sal_Int32 , const cpo::uno::Any& ,
                                                                                const cpo::uno::Sequence< beans::PropertyValue >& rxOptions )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    bool bExportNotesPages = false;
    for( const auto& rOption : rxOptions )
    {
        if ( rOption.Name == "ExportNotesPages" )
            rOption.Value >>= bExportNotesPages;
    }
    cpo::uno::Sequence< beans::PropertyValue > aRenderer;
    if (mpDocShell)
    {
        awt::Size aPageSize;
        if ( bExportNotesPages )
        {
            Size aNotesPageSize = mpDoc->GetSdPage( 0, PageKind::Notes )->GetSize();
            aPageSize = awt::Size( aNotesPageSize.Width(), aNotesPageSize.Height() );
        }
        else
        {
            const ::tools::Rectangle aVisArea( mpDocShell->GetVisArea( embed::Aspects::MSOLE_DOCPRINT ) );
            aPageSize = awt::Size( aVisArea.GetWidth(), aVisArea.GetHeight() );
        }
        aRenderer = { comphelper::makePropertyValue(u"PageSize"_ustr, aPageSize) };
    }
    return aRenderer;
}

namespace {

class ImplRenderPaintProc : public sdr::contact::ViewObjectContactRedirector
{
    const SdrLayerAdmin&    rLayerAdmin;
    SdrPageView*            pSdrPageView;

public:
    bool IsVisible  ( const SdrObject* pObj ) const;
    bool IsPrintable( const SdrObject* pObj ) const;

    ImplRenderPaintProc(const SdrLayerAdmin& rLA, SdrPageView* pView);

    // all default implementations just call the same methods at the original. To do something
    // different, override the method and at least do what the method does.
    virtual void createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) override;
};

}

ImplRenderPaintProc::ImplRenderPaintProc(const SdrLayerAdmin& rLA, SdrPageView *const pView)
    : rLayerAdmin(rLA)
    , pSdrPageView(pView)
{
}

static sal_Int32 ImplPDFGetBookmarkPage( const OUString& rBookmark, SdDrawDocument const & rDoc )
{
    sal_Int32 nPage = -1;

    OUString aBookmark( rBookmark );

    if( rBookmark.startsWith("#") )
        aBookmark = rBookmark.copy( 1 );

    // is the bookmark a page ?
    bool        bIsMasterPage;
    sal_uInt16  nPgNum = rDoc.GetPageByName( aBookmark, bIsMasterPage );

    if ( nPgNum == SDRPAGE_NOTFOUND )
    {
        // is the bookmark an object ?
        SdrObject* pObj = rDoc.GetObj( aBookmark );
        if (pObj)
            nPgNum = pObj->getSdrPageFromSdrObject()->GetPageNum();
    }
    if ( nPgNum != SDRPAGE_NOTFOUND )
        nPage = ( nPgNum - 1 ) / 2;
    return nPage;
}

static void ImplPDFExportComments( const uno::Reference< drawing::XDrawPage >& xPage, vcl::PDFExtOutDevData& rPDFExtOutDevData )
{
    try
    {
        uno::Reference< office::XAnnotationAccess > xAnnotationAccess( xPage, uno::UNO_QUERY_THROW );
        uno::Reference< office::XAnnotationEnumeration > xAnnotationEnumeration( xAnnotationAccess->createAnnotationEnumeration() );

        while( xAnnotationEnumeration->hasMoreElements() )
        {
            uno::Reference<office::XAnnotation> xAnnotation(xAnnotationEnumeration->nextElement());

            geometry::RealPoint2D aRealPoint2D(xAnnotation->getPosition());
            geometry::RealSize2D aRealSize2D(xAnnotation->getSize());

            Point aPoint(aRealPoint2D.X * 100.0, aRealPoint2D.Y * 100.0);
            Size aSize(aRealSize2D.Width * 100.0, aRealSize2D.Height * 100.0);

            Point aPopupPoint(aPoint.X(), aPoint.Y());
            Size aPopupSize(aSize.Width() * 10.0, aSize.Height() * 10.0);

            uno::Reference<text::XText> xText(xAnnotation->getTextRange());

            vcl::pdf::PDFNote aNote;
            aNote.maTitle = xAnnotation->getAuthor();
            aNote.maContents = xText->getString();
            aNote.maModificationDate = xAnnotation->getDateTime();
            auto* pAnnotation = dynamic_cast<sd::Annotation*>(xAnnotation.get());

            if (pAnnotation && pAnnotation->IsThreaded())
            {
                aNote.mnAnnotationId = pAnnotation->GetId();
                aNote.mnParentId = pAnnotation->GetParentId();
                aNote.mbResolved = pAnnotation->IsResolved();
            }

            if (pAnnotation && pAnnotation->getCreationInfo().meType != sdr::annotation::AnnotationType::None)
            {
                sdr::annotation::CreationInfo const& rCreation = pAnnotation->getCreationInfo();
                aNote.maPolygons = rCreation.maPolygons;
                aNote.maAnnotationColor = rCreation.maColor;
                aNote.maInteriorColor = rCreation.maFillColor;
                aNote.mfWidth = rCreation.mnWidth;
                switch (rCreation.meType)
                {
                    case sdr::annotation::AnnotationType::Square:
                        aNote.meType = vcl::pdf::PDFAnnotationSubType::Square; break;
                    case sdr::annotation::AnnotationType::Circle:
                        aNote.meType = vcl::pdf::PDFAnnotationSubType::Circle; break;
                    case sdr::annotation::AnnotationType::Polygon:
                        aNote.meType = vcl::pdf::PDFAnnotationSubType::Polygon; break;
                    case sdr::annotation::AnnotationType::Ink:
                        aNote.meType = vcl::pdf::PDFAnnotationSubType::Ink; break;
                    case sdr::annotation::AnnotationType::Highlight:
                        aNote.meType = vcl::pdf::PDFAnnotationSubType::Highlight; break;
                    case sdr::annotation::AnnotationType::Line:
                        aNote.meType = vcl::pdf::PDFAnnotationSubType::Line; break;
                    case sdr::annotation::AnnotationType::FreeText:
                        aNote.meType = vcl::pdf::PDFAnnotationSubType::FreeText; break;
                    default:
                        aNote.meType = vcl::pdf::PDFAnnotationSubType::Text;
                        break;
                }
            }

            rPDFExtOutDevData.CreateNote(::tools::Rectangle(aPoint, aSize), aNote,
                                         ::tools::Rectangle(aPopupPoint, aPopupSize));
        }
    }
    catch (const cpo::uno::Exception&)
    {
    }
}

static void ImplPDFExportShapeInteraction( const uno::Reference< drawing::XShape >& xShape, SdDrawDocument& rDoc, vcl::PDFExtOutDevData& rPDFExtOutDevData )
{
    if ( xShape->getShapeType() == "com.sun.star.drawing.GroupShape" )
    {
        uno::Reference< container::XIndexAccess > xIndexAccess( xShape, uno::UNO_QUERY );
        if ( xIndexAccess.is() )
        {
            sal_Int32 i, nCount = xIndexAccess->getCount();
            for ( i = 0; i < nCount; i++ )
            {
                uno::Reference< drawing::XShape > xSubShape( xIndexAccess->getByIndex( i ), uno::UNO_QUERY );
                if ( xSubShape.is() )
                    ImplPDFExportShapeInteraction( xSubShape, rDoc, rPDFExtOutDevData );
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet > xShapePropSet( xShape, uno::UNO_QUERY );
        if( xShapePropSet.is() )
        {
            Size        aPageSize( rDoc.GetSdPage( 0, PageKind::Standard )->GetSize() );
            Point aPoint( 0, 0 );
            ::tools::Rectangle   aPageRect( aPoint, aPageSize );

            awt::Point  aShapePos( xShape->getPosition() );
            awt::Size   aShapeSize( xShape->getSize() );
            ::tools::Rectangle   aLinkRect( Point( aShapePos.X, aShapePos.Y ), Size( aShapeSize.Width, aShapeSize.Height ) );

            // Handle linked videos.
            if (xShape->getShapeType() == "com.sun.star.drawing.MediaShape" || xShape->getShapeType() == "com.sun.star.presentation.MediaShape")
            {
                OUString title;
                xShapePropSet->getPropertyValue(u"Title"_ustr) >>= title;
                OUString description;
                xShapePropSet->getPropertyValue(u"Description"_ustr) >>= description;
                OUString const altText(title.isEmpty()
                    ? description
                    : description.isEmpty()
                        ? title
                        : OUString::Concat(title) + OUString::Concat("\n") + OUString::Concat(description));

                OUString aMediaURL;
                xShapePropSet->getPropertyValue(u"MediaURL"_ustr) >>= aMediaURL;
                if (!aMediaURL.isEmpty())
                {
                    SdrObject const*const pSdrObj(SdrObject::getSdrObjectFromXShape(xShape));
                    OUString const mimeType(xShapePropSet->getPropertyValue(u"MediaMimeType"_ustr).get<OUString>());
                    sal_Int32 nScreenId = rPDFExtOutDevData.CreateScreen(aLinkRect, altText, mimeType, rPDFExtOutDevData.GetCurrentPageNumber(), pSdrObj);
                    if (aMediaURL.startsWith("vnd.sun.star.Package:"))
                    {
                        OUString aTempFileURL;
                        xShapePropSet->getPropertyValue(u"PrivateTempFileURL"_ustr) >>= aTempFileURL;
                        rPDFExtOutDevData.SetScreenStream(nScreenId, aTempFileURL);
                    }
                    else
                        rPDFExtOutDevData.SetScreenURL(nScreenId, aMediaURL);
                }
            }

            presentation::ClickAction eCa;
            cpo::uno::Any aAny( xShapePropSet->getPropertyValue( u"OnClick"_ustr ) );
            if ( aAny >>= eCa )
            {
                OUString const actionName(SdResId(SdTPAction::GetClickActionSdResId(eCa)));
                switch ( eCa )
                {
                    case presentation::ClickAction_LASTPAGE :
                    {
                        sal_Int32 nCount = rDoc.GetSdPageCount( PageKind::Standard );
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nCount - 1, vcl::pdf::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink(aLinkRect, actionName);
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;
                    case presentation::ClickAction_FIRSTPAGE :
                    {
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, 0, vcl::pdf::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink(aLinkRect, actionName);
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;
                    case presentation::ClickAction_PREVPAGE :
                    {
                        sal_Int32 nDestPage = rPDFExtOutDevData.GetCurrentPageNumber();
                        if ( nDestPage )
                            nDestPage--;
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nDestPage, vcl::pdf::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink(aLinkRect, actionName);
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;
                    case presentation::ClickAction_NEXTPAGE :
                    {
                        sal_Int32 nDestPage = rPDFExtOutDevData.GetCurrentPageNumber() + 1;
                        sal_Int32 nLastPage = rDoc.GetSdPageCount( PageKind::Standard ) - 1;
                        if ( nDestPage > nLastPage )
                            nDestPage = nLastPage;
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nDestPage, vcl::pdf::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink(aLinkRect, actionName);
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;

                    case presentation::ClickAction_PROGRAM :
                    case presentation::ClickAction_BOOKMARK :
                    case presentation::ClickAction_DOCUMENT :
                    {
                        OUString aBookmark;
                        xShapePropSet->getPropertyValue( u"Bookmark"_ustr ) >>= aBookmark;
                        if( !aBookmark.isEmpty() )
                        {
                            switch( eCa )
                            {
                                case presentation::ClickAction_DOCUMENT :
                                case presentation::ClickAction_PROGRAM :
                                {
                                    sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink(aLinkRect, actionName);
                                    rPDFExtOutDevData.SetLinkURL( nLinkId, aBookmark );
                                }
                                break;
                                case presentation::ClickAction_BOOKMARK :
                                {
                                    sal_Int32 nPage = ImplPDFGetBookmarkPage( aBookmark, rDoc );
                                    if ( nPage != -1 )
                                    {
                                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nPage, vcl::pdf::PDFWriter::DestAreaType::FitRectangle );
                                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink(aLinkRect, actionName);
                                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                                    }
                                }
                                break;
                                default:
                                    break;
                            }
                        }
                    }
                    break;

                    case presentation::ClickAction_STOPPRESENTATION :
                    case presentation::ClickAction_SOUND :
                    case presentation::ClickAction_INVISIBLE :
                    case presentation::ClickAction_VERB :
                    case presentation::ClickAction_VANISH :
                    case presentation::ClickAction_MACRO :
                    default :
                    break;
                }
            }
        }
    }
}

void ImplRenderPaintProc::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo,
    drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();
    if(!pObject)
    {
        // not an object, maybe a page
        sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo, rVisitor);
        return;
    }
    SdrPage* pSdrPage(pObject->getSdrPageFromSdrObject());
    if(!pSdrPage)
        return;
    if(!pSdrPage->checkVisibility(rOriginal, rDisplayInfo, false))
        return;
    if(!IsVisible(pObject) || !IsPrintable(pObject))
        return;

    sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo, rVisitor);
}

bool ImplRenderPaintProc::IsVisible( const SdrObject* pObj ) const
{
    bool bVisible = true;
    SdrLayerID nLayerId = pObj->GetLayer();
    if( pSdrPageView )
    {
        const SdrLayer* pSdrLayer = rLayerAdmin.GetLayerPerID( nLayerId );
        if ( pSdrLayer )
        {
            const OUString& aLayerName = pSdrLayer->GetName();
            bVisible = pSdrPageView->IsLayerVisible( aLayerName );
        }
    }
    return bVisible;
}
bool ImplRenderPaintProc::IsPrintable( const SdrObject* pObj ) const
{
    bool bPrintable = true;
    SdrLayerID nLayerId = pObj->GetLayer();
    if( pSdrPageView )
    {
        const SdrLayer* pSdrLayer = rLayerAdmin.GetLayerPerID( nLayerId );
        if ( pSdrLayer )
        {
            const OUString& aLayerName = pSdrLayer->GetName();
            bPrintable = pSdrPageView->IsLayerPrintable( aLayerName );
        }
    }
    return bPrintable;

}

namespace
{
    sal_Int16 CalcOutputPageNum(vcl::PDFExtOutDevData const * pPDFExtOutDevData, SdDrawDocument const *pDoc, sal_Int16 nPageNumber)
    {
        //export all pages, simple one to one case
        if (pPDFExtOutDevData && pPDFExtOutDevData->GetIsExportHiddenSlides())
            return nPageNumber-1;
        //check all preceding pages, and only count non-hidden ones
        sal_Int16 nRet = 0;
        for (sal_Int16 i = 0; i < nPageNumber-1; ++i)
        {
           if (!pDoc->GetSdPage(i, PageKind::Standard)->IsExcluded())
                ++nRet;
        }
        return nRet;
    }
}

void SdXImpressDocument::render( sal_Int32 nRenderer, const cpo::uno::Any& rSelection,
                                          const cpo::uno::Sequence< beans::PropertyValue >& rxOptions )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if (!mpDocShell)
        return;

    uno::Reference< awt::XDevice >  xRenderDevice;
    const sal_Int32                 nPageNumber = nRenderer + 1;
    PageKind                        ePageKind = PageKind::Standard;
    bool                        bExportNotesPages = false;

    for( const auto& rOption : rxOptions )
    {
        if ( rOption.Name == "RenderDevice" )
            rOption.Value >>= xRenderDevice;
        else if ( rOption.Name == "ExportNotesPages" )
        {
            rOption.Value >>= bExportNotesPages;
            if ( bExportNotesPages )
                ePageKind = PageKind::Notes;
        }
    }

    if( !(xRenderDevice.is() && nPageNumber && ( nPageNumber <= mpDoc->GetSdPageCount( ePageKind ) )) )
        return;

    VCLXDevice* pDevice = dynamic_cast<VCLXDevice*>( xRenderDevice.get() );
    VclPtr< OutputDevice> pOut = pDevice ? pDevice->GetOutputDevice() : VclPtr< OutputDevice >();

    if( !pOut )
        return;

    vcl::PDFExtOutDevData* pPDFExtOutDevData = dynamic_cast<vcl::PDFExtOutDevData* >( pOut->GetExtOutDevData() );

    if ( mpDoc->GetSdPage(static_cast<sal_Int16>(nPageNumber)-1, PageKind::Standard)->IsExcluded() &&
        !(pPDFExtOutDevData && pPDFExtOutDevData->GetIsExportHiddenSlides()) )
        return;

    if (pPDFExtOutDevData)
    {
        css::lang::Locale const docLocale(Application::GetSettings().GetLanguageTag().getLocale());
        pPDFExtOutDevData->SetDocumentLocale(docLocale);
    }

    ::sd::ClientView aView( mpDocShell, pOut );
    ::tools::Rectangle aVisArea( Point(), mpDoc->GetSdPage( static_cast<sal_uInt16>(nPageNumber) - 1, ePageKind )->GetSize() );
    vcl::Region                       aRegion( aVisArea );

    ::sd::ViewShell* pOldViewSh = mpDocShell->GetViewShell();
    ::sd::View* pOldSdView = pOldViewSh ? pOldViewSh->GetView() : nullptr;

    if  ( pOldSdView )
        pOldSdView->SdrEndTextEdit();

    aView.SetHlplVisible( false );
    aView.SetGridVisible( false );
    aView.SetBordVisible( false );
    aView.SetPageVisible( false );
    aView.SetGlueVisible( false );

    pOut->SetMapMode(MapMode(MapUnit::Map100thMM));
    pOut->IntersectClipRegion( aVisArea );

    uno::Reference< frame::XModel > xModel;
    rSelection >>= xModel;

    if( xModel == mpDocShell->GetModel() )
    {
        aView.ShowSdrPage( mpDoc->GetSdPage( static_cast<sal_uInt16>(nPageNumber) - 1, ePageKind ));
        SdrPageView* pPV = aView.GetSdrPageView();

        if( pOldSdView )
        {
            SdrPageView* pOldPV = pOldSdView->GetSdrPageView();
            if( pPV && pOldPV )
            {
                pPV->SetVisibleLayers( pOldPV->GetVisibleLayers() );
                pPV->SetPrintableLayers( pOldPV->GetPrintableLayers() );
            }
        }

        ImplRenderPaintProc aImplRenderPaintProc( mpDoc->GetLayerAdmin(),
            pPV);

        // background color for outliner :o
        SdPage* pPage = pPV ? static_cast<SdPage*>(pPV->GetPage()) : nullptr;
        if( pPage )
        {
            SdrOutliner& rOutl = mpDoc->GetDrawOutliner();
            bool bScreenDisplay(true);

            // #i75566# printing; suppress AutoColor BackgroundColor generation
            // for visibility reasons by giving GetPageBackgroundColor()
            // the needed hint
            // #i75566# PDF export; suppress AutoColor BackgroundColor generation (see printing)
            if (pOut && ((OUTDEV_PRINTER == pOut->GetOutDevType())
                    || (OUTDEV_PDF == pOut->GetOutDevType())))
                bScreenDisplay = false;

            // #i75566# Name change GetBackgroundColor -> GetPageBackgroundColor and
            // hint value if screen display. Only then the AutoColor mechanisms shall be applied
            rOutl.SetBackgroundColor( pPage->GetPageBackgroundColor( pPV, bScreenDisplay ) );
        }

        // produce link annots for media shapes before painting them
        if ( pPDFExtOutDevData && pPage )
        {
            try
            {
                cpo::uno::Any aAny;
                rtl::Reference< SvxDrawPage > xPage( pPage->getUnoPage() );
                if ( xPage.is() )
                {
                    if ( pPDFExtOutDevData->GetIsExportNotes() )
                        ImplPDFExportComments( xPage, *pPDFExtOutDevData );
                    uno::Reference< beans::XPropertySet > xPagePropSet( cppu::getXWeak(xPage.get()), uno::UNO_QUERY );
                    if( xPagePropSet.is() )
                    {
                        // exporting object interactions to pdf

                        // if necessary, the master page interactions will be exported first
                        bool bIsBackgroundObjectsVisible = false;   // #i39428# IsBackgroundObjectsVisible not available for Draw
                        if ( mbImpressDoc && xPagePropSet->getPropertySetInfo()->hasPropertyByName( u"IsBackgroundObjectsVisible"_ustr ) )
                            xPagePropSet->getPropertyValue( u"IsBackgroundObjectsVisible"_ustr ) >>= bIsBackgroundObjectsVisible;
                        if ( bIsBackgroundObjectsVisible && !pPDFExtOutDevData->GetIsExportNotesPages() )
                        {
                            uno::Reference< drawing::XMasterPageTarget > xMasterPageTarget( cppu::getXWeak(xPage.get()), uno::UNO_QUERY );
                            if ( xMasterPageTarget.is() )
                            {
                                uno::Reference< drawing::XDrawPage > xMasterPage = xMasterPageTarget->getMasterPage();
                                if ( xMasterPage.is() )
                                {
                                    sal_Int32 i, nCount = xMasterPage->getCount();
                                    for ( i = 0; i < nCount; i++ )
                                    {
                                        aAny = xMasterPage->getByIndex( i );
                                        uno::Reference< drawing::XShape > xShape;
                                        if ( aAny >>= xShape )
                                            ImplPDFExportShapeInteraction( xShape, *mpDoc, *pPDFExtOutDevData );
                                    }
                                }
                            }
                        }

                        // exporting slide page object interactions
                        sal_Int32 i, nCount = xPage->getCount();
                        for ( i = 0; i < nCount; i++ )
                        {
                            aAny = xPage->getByIndex( i );
                            uno::Reference< drawing::XShape > xShape;
                            if ( aAny >>= xShape )
                                ImplPDFExportShapeInteraction( xShape, *mpDoc, *pPDFExtOutDevData );
                        }

                        // exporting transition effects to pdf
                        if ( mbImpressDoc && !pPDFExtOutDevData->GetIsExportNotesPages() && pPDFExtOutDevData->GetIsExportTransitionEffects() )
                        {
                            static constexpr OUString sEffect( u"Effect"_ustr );
                            static constexpr OUString sSpeed ( u"Speed"_ustr );
                            sal_Int32 nTime = 800;
                            presentation::AnimationSpeed aAs;
                            if ( xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sSpeed ) )
                            {
                                aAny = xPagePropSet->getPropertyValue( sSpeed );
                                if ( aAny >>= aAs )
                                {
                                    switch( aAs )
                                    {
                                        case presentation::AnimationSpeed_SLOW : nTime = 1500; break;
                                        case presentation::AnimationSpeed_FAST : nTime = 300; break;
                                        default:
                                        case presentation::AnimationSpeed_MEDIUM : nTime = 800;
                                    }
                                }
                            }
                            presentation::FadeEffect eFe;
                            vcl::pdf::PDFWriter::PageTransition eType = vcl::pdf::PDFWriter::PageTransition::Regular;
                            if ( xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sEffect ) )
                            {
                                aAny = xPagePropSet->getPropertyValue( sEffect );
                                if ( aAny >>= eFe )
                                {
                                    switch( eFe )
                                    {
                                        case presentation::FadeEffect_HORIZONTAL_LINES :
                                        case presentation::FadeEffect_HORIZONTAL_CHECKERBOARD :
                                        case presentation::FadeEffect_HORIZONTAL_STRIPES : eType = vcl::pdf::PDFWriter::PageTransition::BlindsHorizontal; break;

                                        case presentation::FadeEffect_VERTICAL_LINES :
                                        case presentation::FadeEffect_VERTICAL_CHECKERBOARD :
                                        case presentation::FadeEffect_VERTICAL_STRIPES : eType = vcl::pdf::PDFWriter::PageTransition::BlindsVertical; break;

                                        case presentation::FadeEffect_UNCOVER_TO_RIGHT :
                                        case presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT :
                                        case presentation::FadeEffect_ROLL_FROM_LEFT :
                                        case presentation::FadeEffect_FADE_FROM_UPPERLEFT :
                                        case presentation::FadeEffect_MOVE_FROM_UPPERLEFT :
                                        case presentation::FadeEffect_FADE_FROM_LEFT :
                                        case presentation::FadeEffect_MOVE_FROM_LEFT : eType = vcl::pdf::PDFWriter::PageTransition::WipeLeftToRight; break;

                                        case presentation::FadeEffect_UNCOVER_TO_BOTTOM :
                                        case presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT :
                                        case presentation::FadeEffect_ROLL_FROM_TOP :
                                        case presentation::FadeEffect_FADE_FROM_UPPERRIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_UPPERRIGHT :
                                        case presentation::FadeEffect_FADE_FROM_TOP :
                                        case presentation::FadeEffect_MOVE_FROM_TOP : eType = vcl::pdf::PDFWriter::PageTransition::WipeTopToBottom; break;

                                        case presentation::FadeEffect_UNCOVER_TO_LEFT :
                                        case presentation::FadeEffect_UNCOVER_TO_LOWERLEFT :
                                        case presentation::FadeEffect_ROLL_FROM_RIGHT :

                                        case presentation::FadeEffect_FADE_FROM_LOWERRIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_LOWERRIGHT :
                                        case presentation::FadeEffect_FADE_FROM_RIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_RIGHT : eType = vcl::pdf::PDFWriter::PageTransition::WipeRightToLeft; break;

                                        case presentation::FadeEffect_UNCOVER_TO_TOP :
                                        case presentation::FadeEffect_UNCOVER_TO_UPPERLEFT :
                                        case presentation::FadeEffect_ROLL_FROM_BOTTOM :
                                        case presentation::FadeEffect_FADE_FROM_LOWERLEFT :
                                        case presentation::FadeEffect_MOVE_FROM_LOWERLEFT :
                                        case presentation::FadeEffect_FADE_FROM_BOTTOM :
                                        case presentation::FadeEffect_MOVE_FROM_BOTTOM : eType = vcl::pdf::PDFWriter::PageTransition::WipeBottomToTop; break;

                                        case presentation::FadeEffect_OPEN_VERTICAL : eType = vcl::pdf::PDFWriter::PageTransition::SplitHorizontalInward; break;
                                        case presentation::FadeEffect_CLOSE_HORIZONTAL : eType = vcl::pdf::PDFWriter::PageTransition::SplitHorizontalOutward; break;

                                        case presentation::FadeEffect_OPEN_HORIZONTAL : eType = vcl::pdf::PDFWriter::PageTransition::SplitVerticalInward; break;
                                        case presentation::FadeEffect_CLOSE_VERTICAL : eType = vcl::pdf::PDFWriter::PageTransition::SplitVerticalOutward; break;

                                        case presentation::FadeEffect_FADE_TO_CENTER : eType = vcl::pdf::PDFWriter::PageTransition::BoxInward; break;
                                        case presentation::FadeEffect_FADE_FROM_CENTER : eType = vcl::pdf::PDFWriter::PageTransition::BoxOutward; break;

                                        case presentation::FadeEffect_NONE : eType = vcl::pdf::PDFWriter::PageTransition::Regular; break;

                                        case presentation::FadeEffect_RANDOM :
                                        case presentation::FadeEffect_DISSOLVE :
                                        default: eType = vcl::pdf::PDFWriter::PageTransition::Dissolve; break;
                                    }
                                }
                            }

                            if ( xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sEffect ) ||
                                xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sSpeed ) )
                            {
                                pPDFExtOutDevData->SetPageTransition( eType, nTime );
                            }
                        }
                    }
                }
            }
            catch (const cpo::uno::Exception&)
            {
            }
        }

        aView.SdrPaintView::CompleteRedraw(pOut, aRegion, &aImplRenderPaintProc);

        if (pPDFExtOutDevData && pPage)
        {
            try
            {
                Size        aPageSize( mpDoc->GetSdPage( 0, PageKind::Standard )->GetSize() );
                Point aPoint( 0, 0 );
                ::tools::Rectangle   aPageRect( aPoint, aPageSize );

                // resolving links found in this page by the method ImpEditEngine::Paint
                std::vector< vcl::PDFExtOutDevBookmarkEntry >& rBookmarks = pPDFExtOutDevData->GetBookmarks();
                for ( const auto& rBookmark : rBookmarks )
                {
                    sal_Int32 nPage = ImplPDFGetBookmarkPage( rBookmark.aBookmark, *mpDoc );
                    if ( nPage != -1 )
                    {
                        if ( rBookmark.nLinkId != -1 )
                            pPDFExtOutDevData->SetLinkDest( rBookmark.nLinkId, pPDFExtOutDevData->CreateDest( aPageRect, nPage, vcl::pdf::PDFWriter::DestAreaType::FitRectangle ) );
                        else
                            pPDFExtOutDevData->DescribeRegisteredDest( rBookmark.nDestId, aPageRect, nPage, vcl::pdf::PDFWriter::DestAreaType::FitRectangle );
                    }
                    else
                        pPDFExtOutDevData->SetLinkURL( rBookmark.nLinkId, rBookmark.aBookmark );
                }
                rBookmarks.clear();
                //---> #i56629, #i40318
                //get the page name, will be used as outline element in PDF bookmark pane
                OUString aPageName = mpDoc->GetSdPage( static_cast<sal_uInt16>(nPageNumber) - 1 , PageKind::Standard )->GetName();
                if( !aPageName.isEmpty() )
                {
                    // Destination PageNum
                    const sal_Int32 nDestPageNum = CalcOutputPageNum(pPDFExtOutDevData, mpDoc, nPageNumber);

                    // insert the bookmark to this page into the NamedDestinations
                    if( pPDFExtOutDevData->GetIsExportNamedDestinations() )
                        pPDFExtOutDevData->CreateNamedDest(aPageName, aPageRect, nDestPageNum);

                    // add the name to the outline, (almost) same code as in sc/source/ui/unoobj/docuno.cxx
                    // issue #i40318.

                    if( pPDFExtOutDevData->GetIsExportBookmarks() )
                    {
                        // Destination Export
                        const sal_Int32 nDestId =
                            pPDFExtOutDevData->CreateDest(aPageRect , nDestPageNum);

                        // Create a new outline item:
                        pPDFExtOutDevData->CreateOutlineItem( -1 , aPageName, nDestId );
                    }
                }
                //<--- #i56629, #i40318
            }
            catch (const cpo::uno::Exception&)
            {
            }

        }
    }
    else
    {
        uno::Reference< drawing::XShapes > xShapes;
        rSelection >>= xShapes;

        if( xShapes.is() && xShapes->getCount() )
        {
            SdrPageView* pPV = nullptr;

            ImplRenderPaintProc  aImplRenderPaintProc( mpDoc->GetLayerAdmin(),
                            pOldSdView ? pOldSdView->GetSdrPageView() : nullptr);

            for( sal_uInt32 i = 0, nCount = xShapes->getCount(); i < nCount; i++ )
            {
                uno::Reference< drawing::XShape > xShape;
                xShapes->getByIndex( i ) >>= xShape;

                if( xShape.is() )
                {
                    SdrObject* pObj = SdrObject::getSdrObjectFromXShape( xShape );
                    if( pObj && pObj->getSdrPageFromSdrObject()
                        && aImplRenderPaintProc.IsVisible( pObj )
                            && aImplRenderPaintProc.IsPrintable( pObj ) )
                    {
                        if( !pPV )
                            pPV = aView.ShowSdrPage( pObj->getSdrPageFromSdrObject() );

                        if( pPV )
                            aView.MarkObj( pObj, pPV );
                    }
                }
            }
            aView.DrawMarkedObj(*pOut);
        }
    }
}

DrawViewShell* SdXImpressDocument::GetViewShell()
{
    if (!mpDocShell)
    {
        return nullptr;
    }

    DrawViewShell* pViewSh = dynamic_cast<DrawViewShell*>(mpDocShell->GetViewShell());
    if (!pViewSh)
    {
        SAL_WARN("sd", "DrawViewShell not available!");
        return nullptr;
    }
    return pViewSh;
}

void SdXImpressDocument::paintTile( VirtualDevice& rDevice,
                            int nOutputWidth, int nOutputHeight,
                            int nTilePosX, int nTilePosY,
                            ::tools::Long nTileWidth, ::tools::Long nTileHeight )
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    // we need to skip tile invalidation for controls on rendering
    comphelper::COKit::TiledPaintingGuard aTiledPaintingGuard(
        pViewSh->GetViewShellBase().GetDocId());

    // Setup drawing layer to work properly. Since we use a custom VirtualDevice
    // for the drawing, SdrPaintView::BeginCompleteRedraw() will call FindPaintWindow()
    // unsuccessfully and use a temporary window that doesn't keep state. So patch
    // the existing SdrPageWindow to use a temporary, and this way the state will be kept.
    // Well, at least that's how I understand it based on Writer's RenderContextGuard,
    // as the drawing layer classes lack documentation.
    SdrPageWindow* patchedPageWindow = nullptr;
    SdrPaintWindow* previousPaintWindow = nullptr;
    std::unique_ptr<SdrPaintWindow> temporaryPaintWindow;
    if(SdrView* pDrawView = pViewSh->GetDrawView())
    {
        if(SdrPageView* pSdrPageView = pDrawView->GetSdrPageView())
        {
            pSdrPageView->SetApplicationDocumentColor(pViewSh->GetViewOptions().mnDocBackgroundColor);
            patchedPageWindow = pSdrPageView->FindPageWindow(*getDocWindow()->GetOutDev());
            temporaryPaintWindow.reset(new SdrPaintWindow(*pDrawView, rDevice));
            if (patchedPageWindow)
                previousPaintWindow = patchedPageWindow->patchPaintWindow(*temporaryPaintWindow);
        }
    }

    // Scaling. Must convert from pixels to twips. We know
    // that VirtualDevices use a DPI of 96.
    // We specifically calculate these scales first as we're still
    // in TWIPs, and might as well minimize the number of conversions.
    const double scale = conversionFract(o3tl::Length::px, o3tl::Length::twip);
    double scaleX = double(nOutputWidth) / nTileWidth * scale;
    double scaleY = double(nOutputHeight) / nTileHeight * scale;

    // svx seems to be the only component that works natively in
    // 100th mm rather than TWIP. It makes most sense just to
    // convert here and in getDocumentSize, and leave the tiled
    // rendering API working in TWIPs.
    ::tools::Long nTileWidthHMM = convertTwipToMm100( nTileWidth );
    ::tools::Long nTileHeightHMM = convertTwipToMm100( nTileHeight );
    int nTilePosXHMM = convertTwipToMm100( nTilePosX );
    int nTilePosYHMM = convertTwipToMm100( nTilePosY );

    MapMode aMapMode = rDevice.GetMapMode();
    aMapMode.SetMapUnit( MapUnit::Map100thMM );
    aMapMode.SetOrigin( Point( -nTilePosXHMM,
                               -nTilePosYHMM) );
    aMapMode.SetScaleX( scaleX );
    aMapMode.SetScaleY( scaleY );

    rDevice.SetMapMode( aMapMode );

    rDevice.SetOutputSizePixel( Size(nOutputWidth, nOutputHeight), /*bErase*/false );

    Point aPoint(nTilePosXHMM, nTilePosYHMM);
    Size aSize(nTileWidthHMM, nTileHeightHMM);
    ::tools::Rectangle aRect(aPoint, aSize);

    SdrView* pView = pViewSh->GetDrawView();
    if (comphelper::COKit::isActive())
        pView->SetPaintTextEdit(mbPaintTextEdit);

    pViewSh->GetView()->CompleteRedraw(&rDevice, vcl::Region(aRect));

    if (comphelper::COKit::isActive())
        pView->SetPaintTextEdit(true);

    KitChartHelper::PaintAllChartsOnTile(rDevice, nOutputWidth, nOutputHeight,
                                         nTilePosX, nTilePosY, nTileWidth, nTileHeight);
    KitStarMathHelper::PaintAllInPlaceOnTile(rDevice, nOutputWidth, nOutputHeight, nTilePosX,
                                             nTilePosY, nTileWidth, nTileHeight);

    if(patchedPageWindow != nullptr)
        patchedPageWindow->unpatchPaintWindow(previousPaintWindow);

    // Draw Form controls
    SdrView* pDrawView = pViewSh->GetDrawView();
    SdrPageView* pPageView = pDrawView->GetSdrPageView();
    if (pPageView != nullptr)
    {
        SdrPage* pPage = pPageView->GetPage();
        ::sd::Window* pActiveWin = pViewSh->GetActiveWindow();
        ::tools::Rectangle aTileRect(Point(nTilePosX, nTilePosY), Size(nTileWidth, nTileHeight));
        Size aOutputSize(nOutputWidth, nOutputHeight);
        KitControlHandler::paintControlTile(pPage, pDrawView, *pActiveWin, rDevice, aOutputSize, aTileRect);
    }
}

OString SdXImpressDocument::getViewRenderState(const SfxViewShell* pViewShell)
{
    OStringBuffer aState;
    DrawViewShell* pView = nullptr;

    if (const ViewShellBase* pShellBase = dynamic_cast<const ViewShellBase*>(pViewShell))
        pView = dynamic_cast<DrawViewShell*>(pShellBase->GetMainViewShell().get());
    else
        pView = GetViewShell();

    if (pView)
    {
        const SdViewOptions& pVOpt = pView->GetViewOptions();
        if (mpDoc && mpDoc->GetOnlineSpell())
            aState.append('S');
        if (!ThemeColors::UseOnlyWhiteDocBackground())
        {
            // The dark background is asked for by name. Without the index the answer follows the
            // appearance mode of the process, which is one setting for every view, so a light view
            // would be the one marked here.
            if (pVOpt.mnDocBackgroundColor
                == svtools::ColorConfig::GetDefaultColor(svtools::DOCCOLOR, 1))
                aState.append('D');
        }
        aState.append(';');

        OString aThemeName = OUStringToOString(pVOpt.msColorSchemeName, RTL_TEXTENCODING_UTF8);
        aState.append(aThemeName);
    }
    return aState.makeStringAndClear();
}

void SdXImpressDocument::selectPart(int nPart, int nSelect)
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    // Extending the selection must leave the current part alone
    pViewSh->SelectPage(nPart, nSelect, /*bUpdateCurrentPage=*/false);
}

void SdXImpressDocument::moveSelectedParts(int nPosition, bool bDuplicate, int nIntoSection)
{
    // Duplicating is currently unsupported.
    if (bDuplicate)
        return;

    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    auto pSlideSorter
        = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewSh->GetViewShellBase());
    sd::slidesorter::SharedPageSelection pSelectedPage
        = pSlideSorter ? pSlideSorter->GetPageSelection() : nullptr;
    if (!pSelectedPage)
        return;

    sd::SlideSectionManager& rSectionMgr = mpDoc->GetSectionManager();
    const sal_Int32 nSectionCount = rSectionMgr.GetSectionCount();
    const sal_uInt16 nPageCount = mpDoc->GetSdPageCount(PageKind::Standard);

    // Sections are positional boundaries - they don't follow dragged slides.
    // When a section's start slide is moved away, the section anchors to
    // the first non-moved slide remaining in its range.  If all slides in
    // the section were moved, the section is removed.
    //
    // For sections whose start slide was NOT moved, we still need to
    // update the startIndex to account for the positional shift caused
    // by the insertion/removal.
    //
    // If nIntoSection is set, the user explicitly wants the first moved
    // slide to become the new start of that section, overriding the
    // default anchor for that one section.

    std::set<SdPage*> aMovedPages(pSelectedPage->begin(), pSelectedPage->end());

    struct SectionAnchor
    {
        SdPage* pPage;
        OUString aName;
        OUString aId;
    };
    std::vector<SectionAnchor> aAnchors;

    if (nSectionCount > 0)
    {
        for (sal_Int32 s = 0; s < nSectionCount; ++s)
        {
            const sd::SlideSection& rSec = rSectionMgr.GetSection(s);
            sal_Int32 nStart = rSec.mnStartIndex;
            sal_Int32 nEnd = (s + 1 < nSectionCount)
                ? rSectionMgr.GetSection(s + 1).mnStartIndex
                : static_cast<sal_Int32>(nPageCount);

            SdPage* pAnchor = nullptr;
            for (sal_Int32 p = nStart; p < nEnd; ++p)
            {
                SdPage* pPage = mpDoc->GetSdPage(
                    static_cast<sal_uInt16>(p), PageKind::Standard);
                if (aMovedPages.find(pPage) == aMovedPages.end())
                {
                    pAnchor = pPage;
                    break;
                }
            }

            aAnchors.push_back({ pAnchor, rSec.maName, rSec.maId });
        }

        // Override one section's anchor with the first moved slide so it
        // becomes the new section start (drop onto section header UX).
        if (nIntoSection >= 0 && nIntoSection < nSectionCount
            && !pSelectedPage->empty())
        {
            aAnchors[nIntoSection].pPage = (*pSelectedPage)[0];
        }
    }

    const bool bUndo = mpDoc->IsUndoEnabled();
    if (bUndo && nSectionCount > 0)
        mpDoc->BegUndo(SdResId(STR_UNDO_MOVEPAGES));

    auto pSectionUndo = (bUndo && nSectionCount > 0)
        ? std::make_unique<sd::UndoSlideSection>(*mpDoc, SdResId(STR_UNDO_MOVEPAGES))
        : nullptr;

    mpDoc->MovePages(nPosition, *pSelectedPage);

    // The new page order leaves only the current page selected in the document, and that is
    // the selection clients show. The slide sorter takes the same selection, so the next move
    // takes the pages the clients show as selected.
    pSlideSorter->GetSlideSorter().GetController().GetPageSelector().GetCoreSelection();

    if (nSectionCount > 0)
    {
        sal_uInt16 nNewPageCount = mpDoc->GetSdPageCount(PageKind::Standard);

        std::vector<sd::SlideSection> aNewSections;
        for (const auto& rAnchor : aAnchors)
        {
            if (!rAnchor.pPage)
                continue;

            for (sal_uInt16 p = 0; p < nNewPageCount; ++p)
            {
                if (mpDoc->GetSdPage(p, PageKind::Standard) == rAnchor.pPage)
                {
                    sd::SlideSection aSec;
                    aSec.maName = rAnchor.aName;
                    aSec.maId = rAnchor.aId;
                    aSec.mnStartIndex = static_cast<sal_Int32>(p);
                    aNewSections.push_back(aSec);
                    break;
                }
            }
        }

        std::sort(aNewSections.begin(), aNewSections.end(),
                  [](const sd::SlideSection& a, const sd::SlideSection& b) {
                      return a.mnStartIndex < b.mnStartIndex;
                  });

        auto aOldSections = rSectionMgr.GetSectionsSnapshot();
        bool bChanged = (aNewSections.size() != aOldSections.size());
        if (!bChanged)
        {
            for (size_t i = 0; i < aNewSections.size(); ++i)
            {
                if (aNewSections[i].mnStartIndex != aOldSections[i].mnStartIndex)
                {
                    bChanged = true;
                    break;
                }
            }
        }

        if (bChanged)
            rSectionMgr.RestoreSectionsSnapshot(aNewSections);
    }

    if (pSectionUndo)
    {
        mpDoc->AddUndo(std::move(pSectionUndo));
        mpDoc->EndUndo();
    }
}

std::string SdXImpressDocument::getPartInfo(int nPart)
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return {};

    // In master view the parts are the master pages, so describe the master page at
    // that index, the same list getPartName and getPartHash read in that mode.
    SdPage* pSdPage = isMasterViewMode() ? mpDoc->GetMasterSdPage(nPart, pViewSh->GetPageKind())
                                         : mpDoc->GetSdPage(nPart, pViewSh->GetPageKind());
    const sal_Int16 nMasterPageCount= pViewSh->GetDoc()->GetMasterSdPageCount(pViewSh->GetPageKind());

    ::tools::JsonWriter jsonWriter;

    jsonWriter.put("masterPageCount", nMasterPageCount);
    jsonWriter.put("mode", getEditMode());
    jsonWriter.put("gridSnapEnabled", pViewSh->GetDrawView()->IsGridSnap());
    jsonWriter.put("gridVisible", pViewSh->GetDrawView()->IsGridVisible());

    // Below information is useful when grid snapping is enabled. It let's to calculate the points we can snap to.
    const Size gridCoarse = pViewSh->GetDrawView()->GetGridCoarse();
    const Size innerDots = pViewSh->GetDrawView()->GetGridFine();
    jsonWriter.put("gridCoarseWidth", gridCoarse.getWidth());
    jsonWriter.put("gridCoarseHeight", gridCoarse.getHeight());
    jsonWriter.put("innerSpacesX", innerDots.getWidth() ? gridCoarse.getWidth() / innerDots.getWidth() : 0);
    jsonWriter.put("innerSpacesY", innerDots.getHeight() ? gridCoarse.getHeight() / innerDots.getHeight() : 0);

    if (pSdPage)
        pSdPage->GetPageInfo(jsonWriter);
    else
        SAL_WARN("sd", "getPartInfo request for SdPage " << nPart << " that does not exist!");

    return jsonWriter.finishAndGetAsStdString();
}

namespace
{
// The page at the given index of the page list a mode addresses, independently of any view's
// current edit mode: 0 the standard slides, 1 the master pages, 2 the notes pages, 3 the
// notes master pages, 4 the handout master page. Null past the end of that list and for an
// unknown mode.
SdPage* getPageByIndexAndMode(SdDrawDocument& rDoc, int nPart, int nMode)
{
    if (nPart < 0)
        return nullptr;

    switch (nMode)
    {
        case 0:
            if (nPart < rDoc.GetSdPageCount(PageKind::Standard))
                return rDoc.GetSdPage(nPart, PageKind::Standard);
            break;
        case 1:
            if (nPart < rDoc.GetMasterSdPageCount(PageKind::Standard))
                return rDoc.GetMasterSdPage(nPart, PageKind::Standard);
            break;
        case 2:
            if (nPart < rDoc.GetSdPageCount(PageKind::Notes))
                return rDoc.GetSdPage(nPart, PageKind::Notes);
            break;
        case 3:
            if (nPart < rDoc.GetMasterSdPageCount(PageKind::Notes))
                return rDoc.GetMasterSdPage(nPart, PageKind::Notes);
            break;
        case 4:
            if (nPart < rDoc.GetMasterSdPageCount(PageKind::Handout))
                return rDoc.GetMasterSdPage(nPart, PageKind::Handout);
            break;
        default:
            break;
    }

    return nullptr;
}
}

OString SdXImpressDocument::getPartId(int nPart, int nMode)
{
    if (!mpDoc)
        return OString();

    SdPage* pPage = getPageByIndexAndMode(*mpDoc, nPart, nMode);
    return pPage ? pPage->GetGuid().getString() : OString();
}

int SdXImpressDocument::getPartIndex(std::string_view rPartId, int nMode)
{
    if (!mpDoc)
        return -1;

    // The identifier is the page's GUID; a value that is not one names no page.
    const tools::Guid aGuid(rPartId);
    if (aGuid.isEmpty())
        return -1;

    for (int nIndex = 0;; ++nIndex)
    {
        SdPage* pPage = getPageByIndexAndMode(*mpDoc, nIndex, nMode);
        if (!pPage)
            return -1;
        if (pPage->GetGuid() == aGuid)
            return nIndex;
    }
}

void SdXImpressDocument::setPart( int nPart, bool bAllowChangeFocus )
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    pViewSh->SwitchPage( nPart, bAllowChangeFocus );
}

int SdXImpressDocument::getParts()
{
    if (!mpDoc)
        return 0;

    if (isMasterViewMode())
        return mpDoc->GetMasterSdPageCount(PageKind::Standard);

    return mpDoc->GetSdPageCount(PageKind::Standard);
}

int SdXImpressDocument::getPart()
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return 0;

    return pViewSh->GetViewShellBase().getPart();
}

OUString SdXImpressDocument::getPartName(int nPart)
{
    SdPage* pPage;
    if (isMasterViewMode())
        pPage = mpDoc->GetMasterSdPage(nPart, PageKind::Standard);
    else
        pPage = mpDoc->GetSdPage(nPart, PageKind::Standard);

    if (!pPage)
    {
        SAL_WARN("sd", "DrawViewShell not available!");
        return OUString();
    }

    return pPage->GetName();
}

OUString SdXImpressDocument::getPartHash(int nPart)
{
    SdPage* pPage;
    if (isMasterViewMode())
        pPage = mpDoc->GetMasterSdPage(nPart, PageKind::Standard);
    else
        pPage = mpDoc->GetSdPage(nPart, PageKind::Standard);

    if (!pPage)
    {
        SAL_WARN("sd", "DrawViewShell not available!");
        return OUString();
    }

    uno::Reference<drawing::XDrawPage> xDrawPage(pPage->getUnoPage());
    return OUString::fromUtf8(GetInterfaceHash(xDrawPage));
}

bool SdXImpressDocument::isMasterViewMode()
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return false;

    if (pViewSh->GetDispatcher())
    {
        SfxPoolItemHolder aResult;
        pViewSh->GetDispatcher()->QueryState(SID_SLIDE_MASTER_MODE, aResult);
        const SfxBoolItem* isMasterViewMode(static_cast<const SfxBoolItem*>(aResult.getItem()));
        if (isMasterViewMode && isMasterViewMode->GetValue())
            return true;
    }
    return false;
}

VclPtr<vcl::Window> SdXImpressDocument::getDocWindow()
{
    SolarMutexGuard aGuard;
    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return {};

    if (VclPtr<vcl::Window> pWindow = KitHelper::getInPlaceDocWindow(pViewShell->GetViewShell()))
        return pWindow;

    return pViewShell->GetActiveWindow();
}

void SdXImpressDocument::setPartMode( COKitPartMode ePartMode )
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    PageKind aPageKind( PageKind::Standard );
    switch ( ePartMode )
    {
    case COKitPartMode::SLIDES:
        break;
    case COKitPartMode::NOTES:
        aPageKind = PageKind::Notes;
        break;
    }
    pViewSh->SetPageKind( aPageKind );
    //TODO do the same as setEditMode and then can probably remove the TODOs
    //from doc_setPartMode
}

int SdXImpressDocument::getEditMode()
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return 0;

    return pViewSh->GetViewShellBase().getEditMode();
}

void SdXImpressDocument::setDrawnFromModel(bool bDrawnFromModel)
{
    if (mpDoc)
        mpDoc->SetDrawnFromModel(bDrawnFromModel);
}

void SdXImpressDocument::setEditMode(int nMode)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    pViewSh->GetViewShellBase().setEditMode(nMode);
}

Size SdXImpressDocument::getDocumentSize()
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return Size();

    SdrView *pSdrView = pViewSh->GetView();
    if (!pSdrView)
        return Size();

    SdrPageView* pCurPageView = pSdrView->GetSdrPageView();
    if (!pCurPageView)
        return Size();

    Size aSize = pCurPageView->GetPageRect().GetSize();
    // Convert the size in 100th mm to TWIP
    // See paintTile above for further info.
    return o3tl::convert(aSize, o3tl::Length::mm100, o3tl::Length::twip);
}

Size SdXImpressDocument::getPartSize(int part)
{
    if (part < 0 || part > 0xFFFF)
        return Size(0,0);

    const sal_uInt16 nSlideIndex = static_cast<sal_uInt16>(part);
    SdPage* pPage = nullptr;
    if (mpDoc)
    {
        if (isMasterViewMode())
            pPage = mpDoc->GetMasterSdPage(nSlideIndex, PageKind::Standard);
        else
        {
            DrawViewShell* pViewSh = GetViewShell();
            PageKind ePageKind = pViewSh ? pViewSh->GetPageKind() : PageKind::Standard;
            pPage = mpDoc->GetSdPage(nSlideIndex, ePageKind);
        }
    }

    if (pPage == nullptr)
        return Size(0,0);

    Size aRectSize(pPage->GetWidth() + 1, pPage->GetHeight() + 1);
    return o3tl::convert(aRectSize, o3tl::Length::mm100, o3tl::Length::twip);
}

void SdXImpressDocument::getAllPartSize(::tools::JsonWriter& rJsonWriter)
{
    auto aArray = rJsonWriter.startArray("parts");
    const int nParts = getParts();
    for (int i = 0; i < nParts; ++i)
    {
        const Size aSize = getPartSize(i);
        auto aItem = rJsonWriter.startStruct();
        rJsonWriter.put("width", aSize.getWidth());
        rJsonWriter.put("height", aSize.getHeight());
    }
}

void SdXImpressDocument::getPostIts(::tools::JsonWriter& rJsonWriter)
{
    auto commentsNode = rJsonWriter.startNode("comments");
    if (!mpDoc)
        return;
    // Return annotations on master pages too ?
    const sal_uInt16 nMaxPages = mpDoc->GetPageCount();
    for (sal_uInt16 nPage = 0; nPage < nMaxPages; ++nPage)
    {
        SdrPage* pPage = mpDoc->GetPage(nPage);

        for (auto const& xAnnotation : pPage->getAnnotations())
        {
            sal_uInt32 nID = xAnnotation->GetId();
            OString nodeName = "comment" + OString::number(nID);
            auto commentNode = rJsonWriter.startNode(nodeName);
            rJsonWriter.put("id", nID);
            rJsonWriter.put("author", xAnnotation->getAuthor());
            rJsonWriter.put("dateTime", utl::toISO8601(xAnnotation->getDateTime()));
            // The moment the annotation was written in UTC, when it is known. dateTime beside it
            // is the author's wall clock with no zone.
            const util::DateTime aDateTimeUTC = xAnnotation->getDateTimeUTC();
            if (aDateTimeUTC.Year != 0)
                rJsonWriter.put("dateTimeUtc", DateTimeToOString(DateTime(aDateTimeUTC)));
            uno::Reference<text::XText> xText(xAnnotation->getTextRange());
            rJsonWriter.put("text", xText->getString());
            rJsonWriter.put("part", pPage->GetGuid().getString());
            geometry::RealPoint2D const aPoint = xAnnotation->getPosition();
            geometry::RealSize2D const aSize = xAnnotation->getSize();
            ::tools::Rectangle aRectangle(Point(aPoint.X * 100.0, aPoint.Y * 100.0), Size(aSize.Width * 100.0, aSize.Height * 100.0));
            aRectangle = o3tl::toTwips(aRectangle, o3tl::Length::mm100);
            OString sRectangle = aRectangle.toString();
            rJsonWriter.put("rectangle", sRectangle.getStr());
            if (xAnnotation->IsThreaded())
            {
                rJsonWriter.put("threaded", "true");
                rJsonWriter.put("resolved", xAnnotation->IsResolved() ? "true" : "false");
                rJsonWriter.put("parentId", xAnnotation->GetParentId());
            }
        }
    }
}

void SdXImpressDocument::initializeForTiledRendering(const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    SolarMutexGuard aGuard;

    OUString sThemeName;
    OUString sBackgroundThemeName;

    if (DrawViewShell* pViewShell = GetViewShell())
    {
        DrawView* pDrawView = pViewShell->GetDrawView();
        for (const beans::PropertyValue& rValue : rArguments)
        {
            if (rValue.Name == ".uno:ShowBorderShadow" && rValue.Value.has<bool>())
                pDrawView->SetPageShadowVisible(rValue.Value.get<bool>());
            else if (rValue.Name == ".uno:Author" && rValue.Value.has<OUString>())
                pViewShell->GetViewShell()->SetKitAuthor(rValue.Value.get<OUString>());
            else if (rValue.Name == ".uno:FocusRingColor" && rValue.Value.has<OUString>())
                pViewShell->GetViewShell()->SetKitFocusRingColor(
                    ::Color::STRtoRGB(rValue.Value.get<OUString>()));
            else if (rValue.Name == ".uno:SpellOnline" && rValue.Value.has<bool>())
                mpDoc->SetOnlineSpell(rValue.Value.get<bool>());
            else if (rValue.Name == ".uno:ChangeTheme" && rValue.Value.has<OUString>())
                sThemeName = rValue.Value.get<OUString>();
            else if (rValue.Name == ".uno:InvertBackground" && rValue.Value.has<OUString>())
                sBackgroundThemeName = rValue.Value.get<OUString>();
        }

        // Disable comments
        SdOptions* pOptions = SdModule::get()->GetSdOptions(mpDoc->GetDocumentType());
        pOptions->SetShowComments(false);

        pViewShell->SetRuler(false);
        pViewShell->SetScrollBarsVisible(false);

        if (sd::Window* pWindow = pViewShell->GetActiveWindow())
        {
            // get the full page size in pixels
            pWindow->EnableMapMode();
            Size aSize(pWindow->LogicToPixel(pDrawView->GetSdrPageView()->GetPage()->GetSize()));
            // Disable map mode, so that it's possible to send mouse event
            // coordinates in logic units
            pWindow->EnableMapMode(false);

            // arrange UI elements again with new view size
            pViewShell->GetParentWindow()->SetSizePixel(aSize);
            pViewShell->Resize();
        }

        // Forces all images to be swapped in synchronously, this
        // ensures that images are available when paintTile is called
        // (whereas with async loading images start being loaded after
        //  we have painted the tile, resulting in an invalidate, followed
        //  by the tile being rerendered - which is wasteful and ugly).
        pDrawView->SetSwapAsynchron(false);
    }

    // when the "This document may contain formatting or content that cannot
    // be saved..." dialog appears, it is auto-cancelled with tiled rendering,
    // causing 'Save' being disabled; so let's always save to the original
    // format
    auto xChanges = comphelper::ConfigurationChanges::create();
    officecfg::Office::Common::Save::Document::WarnAlienFormat::set(false, xChanges);

    if (!o3tl::IsRunningUnitTest() || !comphelper::COKit::isActive())
        officecfg::Office::Impress::MultiPaneGUI::SlideSorterBar::Visible::ImpressView::set(true,xChanges);
    xChanges->commit();

    // if we know what theme the user wants, then we can dispatch that now early
    if (!sThemeName.isEmpty())
    {
        cpo::uno::Sequence<css::beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            { u"NewTheme"_ustr, cpo::uno::Any(sThemeName) }
        }));
        comphelper::dispatchCommand(u".uno:ChangeTheme"_ustr, aPropertyValues);
    }
    if (!sBackgroundThemeName.isEmpty())
    {
        cpo::uno::Sequence<css::beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            { u"NewTheme"_ustr, cpo::uno::Any(sBackgroundThemeName) }
        }));
        comphelper::dispatchCommand(u".uno:InvertBackground"_ustr, aPropertyValues);
    }
}

void SdXImpressDocument::postKeyEvent(COKitKeyEventType eType, int nCharCode, int nKeyCode)
{
    SolarMutexGuard aGuard;
    KitHelper::postKeyEventAsync(getDocWindow(), eType, nCharCode, nKeyCode);
}

void SdXImpressDocument::postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount,
                                        int nButtons, int nModifier)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    constexpr double fScale = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::px);

    if (KitHelper::testInPlaceComponentMouseEventHit(
            pViewShell->GetViewShell(), eType, nX, nY, nCount, nButtons, nModifier, fScale, fScale))
        return;

    // try to forward mouse event to control
    const Point aPointTwip(nX, nY);
    const Point aPointHMM = o3tl::convert(aPointTwip, o3tl::Length::twip, o3tl::Length::mm100);
    SdrView* pDrawView = pViewShell->GetDrawView();
    SdrPageView* pPageView = pDrawView->GetSdrPageView();
    if (!pPageView)
    {
        return;
    }

    SdrPage* pPage = pPageView->GetPage();
    ::sd::Window* pActiveWin = pViewShell->GetActiveWindow();
    if (!pActiveWin)
    {
        return;
    }

    if (KitControlHandler::postMouseEvent(pPage, pDrawView, *pActiveWin, eType, aPointHMM, nCount, nButtons, nModifier))
            return;

    KitMouseEventData aMouseEventData(eType, aPointHMM, nCount, MouseEventModifiers::SIMPLECLICK,
                                      nButtons, nModifier);
    KitHelper::postMouseEventAsync(pViewShell->GetActiveWindow(), aMouseEventData);
}

void SdXImpressDocument::setTextSelection(COKitSetTextSelectionType eType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    KitChartHelper aChartHelper(pViewShell->GetViewShell());
    if (aChartHelper.setTextSelection(eType, nX, nY))
        return;

    Point aPoint(convertTwipToMm100(nX), convertTwipToMm100(nY));
    switch (eType)
    {
    case COKitSetTextSelectionType::START:
        pViewShell->SetCursorMm100Position(aPoint, /*bPoint=*/false, /*bClearMark=*/false);
        break;
    case COKitSetTextSelectionType::END:
        pViewShell->SetCursorMm100Position(aPoint, /*bPoint=*/true, /*bClearMark=*/false);
        break;
    case COKitSetTextSelectionType::RESET:
        pViewShell->SetCursorMm100Position(aPoint, /*bPoint=*/true, /*bClearMark=*/true);
        break;
    default:
        assert(false);
        break;
    }
}

uno::Reference<datatransfer::XTransferable> SdXImpressDocument::getSelection()
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return uno::Reference<datatransfer::XTransferable>();

    return pViewShell->GetSelectionTransferable();
}

void SdXImpressDocument::setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    constexpr double fScale = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::px);

    KitChartHelper aChartHelper(pViewShell->GetViewShell());
    if (aChartHelper.setGraphicSelection(eType, nX, nY, fScale, fScale))
        return;

    Point aPoint(convertTwipToMm100(nX), convertTwipToMm100(nY));
    switch (eType)
    {
    case COKitSetGraphicSelectionType::START:
        pViewShell->SetGraphicMm100Position(/*bStart=*/true, aPoint);
        break;
    case COKitSetGraphicSelectionType::END:
        pViewShell->SetGraphicMm100Position(/*bStart=*/false, aPoint);
        break;
    default:
        assert(false);
        break;
    }
}

void SdXImpressDocument::resetSelection()
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    SdrView* pSdrView = pViewShell->GetView();
    if (!pSdrView)
        return;

    if (pSdrView->IsTextEdit())
    {
        // Reset the editeng selection.
        pSdrView->UnmarkAll();
        // Finish editing.
        pSdrView->SdrEndTextEdit();
    }
    // Reset graphic selection.
    pSdrView->UnmarkAll();
}

void SdXImpressDocument::setPageZoom(int nPageZoom)
{
    SolarMutexGuard aGuard;
    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;
    pViewShell->RememberPageZoom(nPageZoom);
}

void SdXImpressDocument::setClientVisibleArea(const ::tools::Rectangle& rRectangle)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    pViewShell->GetViewShellBase().setKitVisibleArea(rRectangle);

    if (pViewShell->getCurrentPage()->IsCanvasPage())
        pViewShell->RememberCanvasPageVisArea(rRectangle);
}

void SdXImpressDocument::setClipboard(const uno::Reference<datatransfer::clipboard::XClipboard>& xClipboard)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    pViewShell->GetActiveWindow()->SetClipboard(xClipboard);
}

bool SdXImpressDocument::isMimeTypeSupported()
{
    SolarMutexGuard aGuard;
    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return false;

    TransferableDataHelper aDataHelper(TransferableDataHelper::CreateFromSystemClipboard(pViewShell->GetActiveWindow()));
    return EditEngine::HasValidData(aDataHelper.GetTransferable());
}

PointerStyle SdXImpressDocument::getPointer()
{
    SolarMutexGuard aGuard;
    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return PointerStyle::Arrow;

    Window* pWindow = pViewShell->GetActiveWindow();
    if (!pWindow)
        return PointerStyle::Arrow;

    return pWindow->GetPointer();
}

uno::Reference< i18n::XForbiddenCharacters > SdXImpressDocument::getForbiddenCharsTable()
{
    rtl::Reference<SdUnoForbiddenCharsTable> xRef = mxForbiddenCharacters.get();
    if( !xRef )
    {
        xRef = new SdUnoForbiddenCharsTable( mpDoc );
        mxForbiddenCharacters = xRef.get();
    }
    return xRef;
}

void SdXImpressDocument::initializeDocument()
{
    if( mbClipBoard )
        return;

    switch( mpDoc->GetPageCount() )
    {
    case 1:
    {
        // nasty hack to detect clipboard document
        mbClipBoard = true;
        break;
    }
    case 0:
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        break;
    }
    }
}

static
void getShapeClickAction(const uno::Reference<drawing::XShape> &xShape, ::tools::JsonWriter& rJsonWriter)
{
    bool bIsShapeVisible = true;
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    if (!xShapeProps)
        return;

    if (!xShapeProps->getPropertySetInfo()->hasPropertyByName( u"Visible"_ustr ))
        return;
    xShapeProps->getPropertyValue(u"Visible"_ustr) >>= bIsShapeVisible;

    if (!bIsShapeVisible)
        return;

    if (!xShapeProps->getPropertySetInfo()->hasPropertyByName( u"OnClick"_ustr ))
        return;

    presentation::ClickAction eClickAction = presentation::ClickAction_NONE;
    xShapeProps->getPropertyValue(u"OnClick"_ustr) >>= eClickAction;

    if (eClickAction != presentation::ClickAction_NONE)
    {
        auto aShape = rJsonWriter.startStruct();

        sal_Int32 nVerb = 0;
        OUString sBookmark;

        if (xShapeProps->getPropertySetInfo()->hasPropertyByName( u"Bookmark"_ustr ))
            xShapeProps->getPropertyValue(u"Bookmark"_ustr) >>= sBookmark;

        {
            auto* pObject = SdrObject::getSdrObjectFromXShape(xShape);
            auto const& rRectangle = pObject->GetLogicRect();
            auto aRectangle = o3tl::convert(rRectangle, o3tl::Length::mm100, o3tl::Length::twip);
            auto aRect = rJsonWriter.startNode("bounds");
            rJsonWriter.put("x", aRectangle.Left());
            rJsonWriter.put("y", aRectangle.Top());
            rJsonWriter.put("width", aRectangle.GetWidth());
            rJsonWriter.put("height", aRectangle.GetHeight());
        }

        {
            auto aInteraction = rJsonWriter.startNode("clickAction");
            switch (eClickAction)
            {
            case presentation::ClickAction_BOOKMARK:
                rJsonWriter.put("action", "bookmark");
                rJsonWriter.put("bookmark", sBookmark);
                break;
            case presentation::ClickAction_DOCUMENT:
                rJsonWriter.put("action", "document");
                rJsonWriter.put("document", sBookmark);
                break;

            case presentation::ClickAction_PREVPAGE:
                rJsonWriter.put("action", "prevpage");
                break;
            case presentation::ClickAction_NEXTPAGE:
                rJsonWriter.put("action", "nextpage");
                break;

            case presentation::ClickAction_FIRSTPAGE:
                rJsonWriter.put("action", "firstpage");
                break;
            case presentation::ClickAction_LASTPAGE:
                rJsonWriter.put("action", "lastpage");
                break;

            case presentation::ClickAction_SOUND:
                rJsonWriter.put("action", "sound");
                rJsonWriter.put("sound", sBookmark);
                break;

            case presentation::ClickAction_VERB:
                rJsonWriter.put("action", "verb");
                xShapeProps->getPropertyValue(u"Verb"_ustr) >>= nVerb;
                rJsonWriter.put("verb", nVerb);
                break;

            case presentation::ClickAction_PROGRAM:
                rJsonWriter.put("action", "program");
                rJsonWriter.put("program", sBookmark);
                break;

            case presentation::ClickAction_MACRO:
                rJsonWriter.put("action", "macro");
                rJsonWriter.put("macro", sBookmark);
                break;

            case presentation::ClickAction_STOPPRESENTATION:
                rJsonWriter.put("action", "stoppresentation");
                break;

            default:
                break;
            }
        }
    }
}

static void getTextHyperlinkInteractions(const uno::Reference<drawing::XShape>& xShape,
                                         ::tools::JsonWriter& rJsonWriter)
{
    SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
    SdrTextObj* pTextObj = DynCastSdrTextObj(pObject);
    if (!pTextObj)
        return;

    OutlinerParaObject* pOutlinerParaObject = pTextObj->GetOutlinerParaObject();
    if (!pOutlinerParaObject
        || !pOutlinerParaObject->GetTextObject().HasField(css::text::textfield::Type::URL))
        return;

    // Lay the text out in the model's shared draw outliner so we can query the
    // bounding rectangle of each field.
    SdrOutliner& rOutliner = pTextObj->getSdrModelFromSdrObject().GetDrawOutliner(pTextObj);
    tools::Rectangle aTextRect;
    pTextObj->TakeTextRect(rOutliner, aTextRect, false, nullptr);

    const EditEngine& rEditEngine = rOutliner.GetEditEngine();
    for (sal_Int32 nPara = 0; nPara < rEditEngine.GetParagraphCount(); ++nPara)
    {
        for (const EFieldInfo& rFieldInfo : rEditEngine.GetFieldInfo(nPara))
        {
            const SvxFieldData* pField
                = rFieldInfo.pFieldItem ? rFieldInfo.pFieldItem->GetField() : nullptr;
            const auto* pURLField = dynamic_cast<const SvxURLField*>(pField);
            if (!pURLField)
                continue;

            // The field occupies a single (dummy) character in the text, map its
            // bounds from text-relative to page coordinates and convert to twips.
            tools::Rectangle aCharRect = rEditEngine.GetCharacterBounds(rFieldInfo.aPosition);
            aCharRect.Move(aTextRect.Left(), aTextRect.Top());
            auto aRect = o3tl::convert(aCharRect, o3tl::Length::mm100, o3tl::Length::twip);

            auto aShape = rJsonWriter.startStruct();
            {
                auto aBounds = rJsonWriter.startNode("bounds");
                rJsonWriter.put("x", aRect.Left());
                rJsonWriter.put("y", aRect.Top());
                rJsonWriter.put("width", aRect.GetWidth());
                rJsonWriter.put("height", aRect.GetHeight());
            }
            auto aInteraction = rJsonWriter.startNode("clickAction");
            rJsonWriter.put("action", "document");
            rJsonWriter.put("document", pURLField->GetURL());
        }
    }

    rOutliner.Clear();
}

OUString SdXImpressDocument::getOrCreateAnimatedGifUrl(const SdrGrafObj& rGraphicObject) const
{
    const Graphic& rGraphic = rGraphicObject.GetGraphic();
    if (!rGraphic.IsAnimated() || !rGraphic.IsGfxLink())
        return OUString();

    const GfxLink aGfxLink = rGraphic.GetGfxLink();
    if (aGfxLink.GetType() != GfxLinkType::NativeGif || aGfxLink.GetDataSize() == 0)
        return OUString();

    const sal_uInt64 nId = rGraphicObject.GetUniqueID();
    const sal_uInt64 nChecksum = rGraphic.GetChecksum();

    // Reuse the file already written for this object while its image is
    // unchanged. A replaced image has a different checksum and gets a fresh
    // file below.
    auto it = maAnimatedGifCache.find(nId);
    if (it != maAnimatedGifCache.end() && it->second.mnChecksum == nChecksum
        && it->second.mpTempFile)
        return it->second.maUrl;

    const cpo::uno::Sequence<sal_Int8> aBytes(
        reinterpret_cast<const sal_Int8*>(aGfxLink.GetData()),
        static_cast<sal_Int32>(aGfxLink.GetDataSize()));
    uno::Reference<io::XInputStream> xStream(new comphelper::SequenceInputStream(aBytes));

    OUString aTempFileUrl;
    if (!avmedia::CreateMediaTempFile(xStream, aTempFileUrl, u".gif"))
        return OUString();

    AnimatedGifTempFile aEntry;
    aEntry.mnChecksum = nChecksum;
    aEntry.maUrl = aTempFileUrl;
    aEntry.mpTempFile = std::make_shared<avmedia::MediaTempFile>(aTempFileUrl);
    maAnimatedGifCache[nId] = std::move(aEntry);
    return aTempFileUrl;
}

std::string SdXImpressDocument::getPresentationInfo(bool bAllyState) const
{
    ::tools::JsonWriter aJsonWriter;

    // The largest page this document shows, in 1/100 mm while the slides below fill it in. A
    // hidden page stays out of it, which keeps out the canvas page, a large square board
    // carrying a preview of every other page.
    Size aDocSize;

    try
    {
        rtl::Reference<SdDrawPagesAccess> xDrawPages = const_cast<SdXImpressDocument*>(this)->getSdDrawPages();

        sd::PresentationSettings const& rSettings = mpDoc->getPresentationSettings();

        const bool bIsEndless = rSettings.mbEndless;
        aJsonWriter.put("isEndless", bIsEndless);

        if (bIsEndless) {
            const sal_Int32 nPauseTimeout = rSettings.mnPauseTimeout;
            aJsonWriter.put("loopAndRepeatDuration", nPauseTimeout);
        }

        // Section data for COOL slide panel
        {
            sd::SlideSectionManager& rSectionMgr = mpDoc->GetSectionManager();
            if (rSectionMgr.GetSectionCount() > 0)
                rSectionMgr.WriteSectionsJson(aJsonWriter, "sections");
        }

        auto aSlideList = aJsonWriter.startArray("slides");
        sal_Int32 nSlideCount = xDrawPages->getCount();
        for (sal_Int32 i = 0; i < nSlideCount; ++i)
        {
            SdGenericDrawPage* pSlide(xDrawPages->getDrawPageByIndex(i));
            SdPage* pPage = SdPage::getImplementation(pSlide);
            if (!pPage)
            {
                SAL_WARN("sd", "slide: " << i << " is disposed, skipping");
                continue;
            }

            bool bIsVisible = true; // default visible
            pSlide->getPropertyValue(u"Visible"_ustr) >>= bIsVisible;

            if (!bIsVisible)
            {
                auto aSlideNode = aJsonWriter.startStruct();
                std::string sSlideHash = GetInterfaceHash(cppu::getXWeak(pSlide));
                aJsonWriter.put("hash", sSlideHash);
                aJsonWriter.put("index", i);
                aJsonWriter.put("hidden", true);
                aJsonWriter.put("part", pPage->GetGuid().getString());
                aJsonWriter.put("guid", pPage->GetGuid().getOUString());
            }
            else
            {
                auto aSlideNode = aJsonWriter.startStruct();
                std::string sSlideHash = GetInterfaceHash(cppu::getXWeak(pSlide));
                aJsonWriter.put("hash", sSlideHash);
                aJsonWriter.put("index", i);
                aJsonWriter.put("part", pPage->GetGuid().getString());
                aJsonWriter.put("guid", pPage->GetGuid().getOUString());

                auto aName = SdDrawPage::getPageApiNameFromUiName(pPage->GetName());
                aJsonWriter.put("name", aName);

                const Size aSlizeSize = const_cast<SdXImpressDocument*>(this)->getPartSize(i);
                aJsonWriter.put("slideWidth", aSlizeSize.getWidth());
                aJsonWriter.put("slideHeight", aSlizeSize.getHeight());

                // Each side counts one unit more than the page bounds, the same way the size
                // reported for this slide does.
                aDocSize.setWidth(std::max(aDocSize.getWidth(), pPage->GetWidth() + 1));
                aDocSize.setHeight(std::max(aDocSize.getHeight(), pPage->GetHeight() + 1));

                if (bAllyState)
                {
                    OUStringBuffer aHtml;
                    SdrOutliner* pOutliner = mpDoc->GetInternalOutliner();
                    if (pOutliner)
                    {
                        SdHTMLFilter::ExportPage(pOutliner, pPage, aHtml);
                        aJsonWriter.put("a11y", aHtml.makeStringAndClear());
                        pOutliner->Clear();
                    }
                }

                bool bIsDrawPageEmpty = pSlide->getCount() == 0;
                aJsonWriter.put("empty", bIsDrawPageEmpty);

                // Notes
                SdPage* pNotesPage = mpDoc->GetSdPage((pPage->GetPageNum() - 1) >> 1, PageKind::Notes);
                if (pNotesPage)
                {
                    SdrObject* pItObj = nullptr;
                    SdrObject* pNotes = pNotesPage->GetPresObj(PresObjKind::Notes);

                    if (!pNotes)
                    {
                        for (size_t nNote = 0; nNote < pNotesPage->GetObjCount(); nNote++)
                        {
                            pItObj = pNotesPage->GetObj(nNote);

                            if (pItObj && pItObj->HasText() &&
                                (pItObj->GetObjIdentifier() == SdrObjKind::Text
                                 || pItObj->GetObjIdentifier() == SdrObjKind::TitleText
                                 || pItObj->GetObjIdentifier() == SdrObjKind::OutlineText))
                            {
                                pNotes = pItObj;
                                break;
                            }
                        }
                    }

                    if (pNotes)
                    {
                        OutlinerParaObject* pPara = pNotes->GetOutlinerParaObject();
                        if (pPara)
                        {
                            const EditTextObject& rText = pPara->GetTextObject();
                            aJsonWriter.put("notes", rText.GetText(LINEEND_LF));

                            OUStringBuffer aNotesHtml;
                            SdrOutliner* pNotesOutliner = mpDoc->GetInternalOutliner();
                            if (pNotesOutliner)
                            {
                                SdHTMLFilter::ExportTextObject(pNotesOutliner, pPara, aNotesHtml);
                                aJsonWriter.put("notesHtml", aNotesHtml.makeStringAndClear());
                                pNotesOutliner->Clear();
                            }
                        }
                    }
                }

                SdMasterPage* pMasterPage = nullptr;
                SdDrawPage* pMasterPageTarget(dynamic_cast<SdDrawPage*>(pSlide));
                if (pMasterPageTarget)
                {
                    pMasterPage = pMasterPageTarget->getSdMasterPage();
                    if (pMasterPage)
                    {
                        std::string sMPHash = GetInterfaceHash(cppu::getXWeak(pMasterPage));
                        aJsonWriter.put("masterPage", sMPHash);

                        bool bBackgroundObjectsVisibility = true; // default visible
                        pSlide->getPropertyValue(u"IsBackgroundObjectsVisible"_ustr) >>= bBackgroundObjectsVisibility;
                        aJsonWriter.put("masterPageObjectsVisibility", bBackgroundObjectsVisibility);
                    }
                }

                bool bBackgroundVisibility = true; // default visible
                pSlide->getPropertyValue(u"IsBackgroundVisible"_ustr)  >>= bBackgroundVisibility;
                if (bBackgroundVisibility)
                {
                    SlideBackgroundInfo aSlideBackgroundInfo(pSlide, static_cast<SvxDrawPage*>(pMasterPage));
                    if (aSlideBackgroundInfo.hasBackground())
                    {
                        auto aBackgroundNode = aJsonWriter.startNode("background");
                        aJsonWriter.put("isCustom", aSlideBackgroundInfo.slideHasOwnBackground());
                        if (aSlideBackgroundInfo.isSolidColor())
                        {
                            aJsonWriter.put("fillColor", aSlideBackgroundInfo.getFillColorAsRGBA());
                        }
                    }
                }

                {
                    auto aVideoList = aJsonWriter.startArray("videos");
                    SdrObjListIter aIterator(pPage, SdrIterMode::DeepWithGroups);
                    while (aIterator.IsMore())
                    {
                        auto* pObject = aIterator.Next();
                        if (pObject->GetObjIdentifier() == SdrObjKind::Media)
                        {
                            auto aVideosNode = aJsonWriter.startStruct();
                            auto* pMediaObject = static_cast<SdrMediaObj*>(pObject);
                            auto const& rRectangle = pMediaObject->GetLogicRect();
                            auto aRectangle = o3tl::convert(rRectangle, o3tl::Length::mm100, o3tl::Length::twip);
                            aJsonWriter.put("id", reinterpret_cast<sal_uInt64>(pMediaObject));
                            aJsonWriter.put("url", pMediaObject->getTempURL());
                            aJsonWriter.put("x", aRectangle.Left());
                            aJsonWriter.put("y", aRectangle.Top());
                            aJsonWriter.put("width", aRectangle.GetWidth());
                            aJsonWriter.put("height", aRectangle.GetHeight());
                        }
                    }
                }

                {
                    auto aGifList = aJsonWriter.startArray("gifs");
                    SdrObjListIter aIterator(pPage, SdrIterMode::DeepWithGroups);
                    while (aIterator.IsMore())
                    {
                        auto* pObject = aIterator.Next();
                        auto* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
                        if (!pGraphicObject)
                            continue;

                        const OUString aUrl = getOrCreateAnimatedGifUrl(*pGraphicObject);
                        if (aUrl.isEmpty())
                            continue;

                        auto aGifNode = aJsonWriter.startStruct();
                        auto const& rRectangle = pGraphicObject->GetSnapRect();
                        auto aRectangle = o3tl::convert(rRectangle, o3tl::Length::mm100, o3tl::Length::twip);
                        aJsonWriter.put("id", static_cast<sal_Int64>(pGraphicObject->GetUniqueID()));
                        aJsonWriter.put("url", aUrl);
                        // The checksum changes when the image behind the shape is
                        // replaced, so the client can tell a new image from a move.
                        aJsonWriter.put(
                            "checksum",
                            static_cast<sal_Int64>(pGraphicObject->GetGraphic().GetChecksum()));
                        aJsonWriter.put("mimeType", "image/gif");
                        aJsonWriter.put("x", aRectangle.Left());
                        aJsonWriter.put("y", aRectangle.Top());
                        aJsonWriter.put("width", aRectangle.GetWidth());
                        aJsonWriter.put("height", aRectangle.GetHeight());

                        const GraphicAttr aGraphicAttr
                            = pGraphicObject->GetGraphicAttr(SdrGrafObjTransformsAttrs::MIRROR);
                        if (aGraphicAttr.IsMirrored())
                            aJsonWriter.put("mirror",
                                            static_cast<sal_Int32>(aGraphicAttr.GetMirrorFlags()));
                    }
                }

                uno::Reference<drawing::XShapes> const xShapes(cppu::getXWeak(pSlide), uno::UNO_QUERY_THROW);
                if (xShapes.is())
                {
                    auto aInteractions = aJsonWriter.startArray("interactions");
                    auto count = xShapes->getCount();
                    for (auto j = 0; j < count; j++)
                    {
                        auto xObject = xShapes->getByIndex(j);
                        uno::Reference<drawing::XShape> xShape(xObject, uno::UNO_QUERY);
                        if (!xShape.is())
                        {
                            continue;
                        }

                        getShapeClickAction(xShape, aJsonWriter);
                        getTextHyperlinkInteractions(xShape, aJsonWriter);
                    }
                }

                sal_Int32 nTransitionType = 0;
                pSlide->getPropertyValue(u"TransitionType"_ustr) >>= nTransitionType;

                if (nTransitionType != 0)
                {
                    auto iterator = constTransitionTypeToString.find(nTransitionType);

                    if (iterator != constTransitionTypeToString.end())
                    {
                        aJsonWriter.put("transitionType", iterator->second);

                        sal_Int32 nTransitionSubtype = 0;
                        pSlide->getPropertyValue(u"TransitionSubtype"_ustr) >>= nTransitionSubtype;

                        auto iteratorSubType = constTransitionSubTypeToString.find(nTransitionSubtype);
                        if (iteratorSubType != constTransitionSubTypeToString.end())
                        {
                            aJsonWriter.put("transitionSubtype", iteratorSubType->second);
                        }
                        else
                        {
                            SAL_WARN("sd", "Transition sub-type unknown: " << nTransitionSubtype);
                        }

                        if (bAllyState)
                        {
                            const TransitionPresetList& rList = TransitionPreset::getTransitionPresetList();
                            auto aTransition = std::find_if(rList.begin(), rList.end(),
                                                            [nTransitionType,
                                                             nTransitionSubtype](const TransitionPresetPtr& rxPreset) {
                                                                return (rxPreset->getTransition() == nTransitionType)
                                                                    && (rxPreset->getSubtype() == nTransitionSubtype);
                                                            });

                            if (aTransition != rList.end())
                            {
                                aJsonWriter.put("transitionLabel", (*aTransition)->getSetLabel());
                            }
                        }

                        bool nTransitionDirection = false;
                        pSlide->getPropertyValue(u"TransitionDirection"_ustr) >>= nTransitionDirection;
                        aJsonWriter.put("transitionDirection", nTransitionDirection);

                        // fade color
                        if ((nTransitionType == TransitionType::FADE)
                                && ((nTransitionSubtype == TransitionSubType::FADETOCOLOR)
                                    || (nTransitionSubtype == TransitionSubType::FADEFROMCOLOR)
                                    || (nTransitionSubtype == TransitionSubType::FADEOVERCOLOR)))
                        {
                            sal_Int32 nFadeColor = 0;
                            pSlide->getPropertyValue(u"TransitionFadeColor"_ustr) >>= nFadeColor;
                            OUStringBuffer sTmpBuf;
                            ::sax::Converter::convertColor(sTmpBuf, nFadeColor);
                            aJsonWriter.put("transitionFadeColor", sTmpBuf.makeStringAndClear());
                        }
                    }

                    double nTransitionDuration(0.0);
                    if( pSlide->getPropertySetInfo()->hasPropertyByName( u"TransitionDuration"_ustr ) &&
                        (pSlide->getPropertyValue( u"TransitionDuration"_ustr ) >>= nTransitionDuration ) && nTransitionDuration != 0.0 )
                    {
                        // convert transitionDuration time to ms
                        aJsonWriter.put("transitionDuration", nTransitionDuration * 1000);
                    }
                }

                sal_Int32 nChange(0);
                if( pSlide->getPropertySetInfo()->hasPropertyByName( u"Change"_ustr ) &&
                        (pSlide->getPropertyValue( u"Change"_ustr ) >>= nChange ) && nChange == 1 )
                {
                    double fSlideDuration(0);
                    if( pSlide->getPropertySetInfo()->hasPropertyByName( u"HighResDuration"_ustr ) &&
                            (pSlide->getPropertyValue( u"HighResDuration"_ustr ) >>= fSlideDuration) )
                    {
                        // convert slide duration time to ms
                        aJsonWriter.put("nextSlideDuration", fSlideDuration * 1000);
                    }
                }


                AnimationsExporter aAnimationExporter(aJsonWriter, pSlide, bAllyState);
                if (aAnimationExporter.hasEffects())
                {
                    {
                        auto aAnimationsNode = aJsonWriter.startNode("animations");
                        aAnimationExporter.exportAnimations();
                    }
                    aAnimationExporter.exportTriggers();
                }
            }
        }
    }
    catch (cpo::uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("sd", "SdXImpressDocument::getSlideShowInfo ... maybe some property can't be retrieved");
    }

    // The slides array closes with the block above, so the size goes in here, in twips. A
    // document whose pages are all hidden leaves the size empty, and then the page the view
    // has on screen answers for the document.
    if (aDocSize.IsEmpty())
        aDocSize = const_cast<SdXImpressDocument*>(this)->getDocumentSize();
    else
        aDocSize = o3tl::convert(aDocSize, o3tl::Length::mm100, o3tl::Length::twip);
    aJsonWriter.put("docWidth", aDocSize.getWidth());
    aJsonWriter.put("docHeight", aDocSize.getHeight());

    return aJsonWriter.finishAndGetAsStdString();
}

namespace
{
bool isRequestedSlideValid(SdDrawDocument* mpDoc, sal_Int32 nSlideNumber, const std::string& slideHash)
{
    try
    {
        SdXImpressDocument* pDoc = mpDoc->getUnoModel();
        rtl::Reference<SdDrawPagesAccess> xDrawPages = pDoc->getSdDrawPages();
        SdGenericDrawPage* pSlide(xDrawPages->getDrawPageByIndex(nSlideNumber));
        if (pSlide)
        {
            return slideHash == GetInterfaceHash(cppu::getXWeak(pSlide));
        }
    }
    catch (cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "SdXImpressDocument::createKitSlideRenderer: failed" );
    }
    return false;
}
}

bool SdXImpressDocument::createSlideRenderer(
    const OString& rSlideHash,
    sal_Int32 nSlideNumber, sal_Int32& nViewWidth, sal_Int32& nViewHeight,
    bool bRenderBackground, bool bRenderMasterPage)
{
    std::string sSlideHash(rSlideHash);
    if (!isRequestedSlideValid(mpDoc, nSlideNumber, sSlideHash))
        return false;

    SdPage* pPage = mpDoc->GetSdPage(sal_uInt16(nSlideNumber), PageKind::Standard);
    if (!pPage)
        return false;

    mpSlideshowLayerRenderer.reset(new SlideshowLayerRenderer(*pPage, rSlideHash, bRenderBackground, bRenderMasterPage));
    Size aDesiredSize(nViewWidth, nViewHeight);
    Size aCalculatedSize = mpSlideshowLayerRenderer->calculateAndSetSizePixel(aDesiredSize);
    nViewWidth = aCalculatedSize.Width();
    nViewHeight = aCalculatedSize.Height();
    return true;
}

bool SdXImpressDocument::renderNextSlideLayer(unsigned char* pBuffer, bool& bIsBitmapLayer, double& rScale, std::string& rJsonMsg)
{
    bool bDone = true;

    if (!mpSlideshowLayerRenderer)
        return bDone;

    bool bOK = mpSlideshowLayerRenderer->render(pBuffer, bIsBitmapLayer, rScale, rJsonMsg);

    if (bOK)
    {
        bDone = false;
    }

    return bDone;
}

bool SdXImpressDocument::getSlideLinks(tools::JsonWriter& rJsonWriter)
{
    if (!mpDoc)
        return false;

    sd::SlideLink::WriteLinks(*mpDoc, rJsonWriter);
    return true;
}

sal_Int32 SdXImpressDocument::refreshSlideLinks(const OUString& rSourceName,
                                                const OUString& rFileUrl,
                                                const OUString& rLastModifiedTime,
                                                std::vector<OString>* pNotUpdated,
                                                sal_Int32 nPageIndex)
{
    if (!mpDoc)
        return -1;

    return sd::SlideLink::Refresh(*mpDoc, rSourceName, rFileUrl, rLastModifiedTime, pNotUpdated,
                                  nPageIndex);
}

bool SdXImpressDocument::breakSlideLink(sal_Int32 nIndex)
{
    if (!mpDoc)
        return false;

    return sd::SlideLink::Break(*mpDoc, nIndex);
}

namespace
{
/// Names the standard pages the given indices ask for, by the name each carries in the
/// document. An empty list names every page, which is what an insert of a whole deck and a
/// write of a whole document both ask for. False when an index names no page of it, or when
/// the document holds no page at all.
bool collectPageNames(SdDrawDocument& rDoc, const std::vector<sal_Int32>& rPages,
                      std::vector<OUString>& rNames)
{
    const sal_uInt16 nPageCount = rDoc.GetSdPageCount(PageKind::Standard);
    if (rPages.empty())
    {
        for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
            rNames.push_back(rDoc.GetSdPage(nPage, PageKind::Standard)->GetName());
    }
    else
    {
        for (const sal_Int32 nPage : rPages)
        {
            if (nPage < 0 || nPage >= static_cast<sal_Int32>(nPageCount))
                return false;

            rNames.push_back(
                rDoc.GetSdPage(static_cast<sal_uInt16>(nPage), PageKind::Standard)->GetName());
        }
    }

    return !rNames.empty();
}

/// Takes the source document off the pages of a run, which a plain insert leaves as pages of
/// this document alone.
void clearInsertedPageLinks(SdDrawDocument& rDoc, sal_uInt16 nFirstSlide, sal_uInt16 nSlideCount)
{
    for (sal_uInt16 nSlide = nFirstSlide; nSlide < nFirstSlide + nSlideCount; ++nSlide)
    {
        SdPage* pPage = rDoc.GetSdPage(nSlide, PageKind::Standard);
        if (!pPage)
            continue;

        pPage->DisconnectLink();
        pPage->SetFileName(OUString());
        pPage->SetBookmarkName(OUString());
    }
}

/// Marks on the pages the slide of the source document each one came from: the position that slide
/// holds in the source when the run names the positions, and the identifier of the slide otherwise.
void recordInsertedPageSources(SdDrawDocument& rDoc, SdDrawDocument& rSource,
                               const std::vector<OUString>& rReadPages, sal_uInt16 nFirstSlide,
                               sal_uInt16 nSlideCount,
                               const std::vector<sal_Int32>& rSourcePositions)
{
    if (rReadPages.size() != static_cast<size_t>(nSlideCount))
        return;

    // The run names the position of every page it inserts or of none of them.
    if (!rSourcePositions.empty() && rSourcePositions.size() != static_cast<size_t>(nSlideCount))
        return;

    for (sal_uInt16 nSlide = nFirstSlide; nSlide < nFirstSlide + nSlideCount; ++nSlide)
    {
        SdPage* pPage = rDoc.GetSdPage(nSlide, PageKind::Standard);
        if (!pPage)
            continue;

        if (!rSourcePositions.empty())
        {
            // The page is linked to the position its slide holds in the source and to no
            // identifier, so an update reads whichever slide stands there.
            pPage->SetBookmarkName(
                sd::SlideLink::MakePositionName(rSourcePositions[nSlide - nFirstSlide]));
            pPage->SetSourcePageGuid(OUString());
            continue;
        }

        bool bIsMasterPage = false;
        const OUString& rReadName = rReadPages[nSlide - nFirstSlide];
        const sal_uInt16 nRead = rSource.GetPageByName(rReadName, bIsMasterPage);
        if (nRead == SDRPAGE_NOTFOUND || bIsMasterPage)
            continue;

        const SdPage* pReadPage = dynamic_cast<const SdPage*>(rSource.GetPage(nRead));
        if (!pReadPage)
            continue;

        const OUString aOriginPage = sd::SlideLink::GetOriginPage(*pReadPage);
        const OUString aSourceName = aOriginPage.isEmpty() ? rReadName : aOriginPage;

        // empty name is intentional here
        const bool bOwnName = sd::SlideLink::GetNamedPosition(aSourceName) == 0;
        pPage->SetBookmarkName(bOwnName ? aSourceName : OUString());
        pPage->SetSourcePageGuid(pReadPage->GetGuid().getOUString());
    }
}

/// Records on the pages of a run the time the source document was last modified, so each linked
/// page carries when its source was last seen.
void stampInsertedPageSource(SdDrawDocument& rDoc, sal_uInt16 nFirstSlide, sal_uInt16 nSlideCount,
                             const OUString& rModifiedTime)
{
    for (sal_uInt16 nSlide = nFirstSlide; nSlide < nFirstSlide + nSlideCount; ++nSlide)
    {
        SdPage* pPage = rDoc.GetSdPage(nSlide, PageKind::Standard);
        if (pPage)
            pPage->SetSourceModifiedTime(rModifiedTime);
    }
}
} // namespace

bool SdXImpressDocument::insertPagesFromFile(const OUString& rFileUrl, const OString& rJsonOptions)
{
    if (!mpDoc || rFileUrl.isEmpty())
        return false;

    sal_Int32 nAt = -1;
    bool bKeepDesign = false;
    bool bLink = false;
    OUString aSourceName;
    OUString aLastModifiedTime;
    std::vector<sal_Int32> aPages;
    std::vector<sal_Int32> aSourcePositions;
    try
    {
        std::stringstream aStream((std::string(rJsonOptions)));
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        nAt = aTree.get<sal_Int32>("at", -1);
        bKeepDesign = aTree.get<bool>("keepDesign", false);
        bLink = aTree.get<bool>("link", false);
        aSourceName = OStringToOUString(aTree.get<std::string>("source", ""),
                                        RTL_TEXTENCODING_UTF8);
        aLastModifiedTime = OStringToOUString(aTree.get<std::string>("lastModifiedTime", ""),
                                              RTL_TEXTENCODING_UTF8);
        if (auto oPages = aTree.get_child_optional("slides"))
        {
            for (const auto& rPage : *oPages)
                aPages.push_back(rPage.second.get_value<sal_Int32>());
        }
        if (auto oPositions = aTree.get_child_optional("sourcePositions"))
        {
            for (const auto& rPosition : *oPositions)
            {
                const sal_Int32 nPosition = rPosition.second.get_value<sal_Int32>();
                if (nPosition < 1)
                    return false;

                aSourcePositions.push_back(nPosition);
            }
        }
    }
    catch (const boost::property_tree::ptree_error&)
    {
        return false;
    }

    // A page keeps its link by the persistent link of the source document, any text the storage
    // resolves to that document, so an insert that asks for links needs one to record.
    if (bLink && aSourceName.isEmpty())
        return false;

    SdDrawDocument* pSource = mpDoc->OpenBookmarkDoc(rFileUrl);
    if (!pSource)
        return false;

    comphelper::ScopeGuard aCloseSource([this] { mpDoc->CloseBookmarkDoc(); });

    std::vector<OUString> aBookmarkList;
    if (!collectPageNames(*pSource, aPages, aBookmarkList))
        return false;

    // The standard page of slide S sits at page position 2*S+1; a position past the last page
    // appends at the end.
    sal_uInt16 nInsertPos = 0xFFFF;
    if (nAt >= 0 && nAt < mpDoc->GetSdPageCount(PageKind::Standard))
        nInsertPos = static_cast<sal_uInt16>(2 * nAt + 1);

    InsertBookmarkOptions aOptions = InsertBookmarkOptions::ForSlideImport(bKeepDesign);
    if (bLink)
    {
        aOptions.bLink = true;
        aOptions.aLinkSourceUrl = sd::SlideLink::MakeSourceReference(aSourceName);
    }

    const sal_uInt16 nSlidesBefore = mpDoc->GetSdPageCount(PageKind::Standard);
    if (!mpDoc->InsertFileAsPage(aBookmarkList, /*pExchangeList=*/nullptr, aOptions, nInsertPos,
                                 pSource->GetDocSh(), /*oScaleObjects=*/true))
        return false;

    const sal_uInt16 nFirstSlide
        = nInsertPos == 0xFFFF ? nSlidesBefore : static_cast<sal_uInt16>(nAt);
    const sal_uInt16 nInsertedCount = mpDoc->GetSdPageCount(PageKind::Standard) - nSlidesBefore;
    if (!bLink)
        clearInsertedPageLinks(*mpDoc, nFirstSlide, nInsertedCount);
    else
    {
        // A linked page records the slide of the source document it came from, which is the slide
        // the file was written from rather than the page it holds in the file.
        recordInsertedPageSources(*mpDoc, *pSource, aBookmarkList, nFirstSlide, nInsertedCount,
                                  aSourcePositions);
        if (!aLastModifiedTime.isEmpty())
            stampInsertedPageSource(*mpDoc, nFirstSlide, nInsertedCount, aLastModifiedTime);
    }

    return true;
}

sal_Int32 SdXImpressDocument::getSlideIndexOfGuid(const OUString& rGuid)
{
    if (!mpDoc || rGuid.isEmpty())
        return -1;

    for (sal_uInt16 nIndex = 0, nCount = mpDoc->GetSdPageCount(PageKind::Standard); nIndex < nCount;
         ++nIndex)
    {
        const SdPage* pPage = mpDoc->GetSdPage(nIndex, PageKind::Standard);
        if (pPage && pPage->GetGuid().getOUString() == rGuid)
            return nIndex;
    }

    return -1;
}

bool SdXImpressDocument::exportPages(const std::vector<sal_Int32>& rPages, const OUString& rFileUrl)
{
    if (!mpDoc || !mpDocShell || rFileUrl.isEmpty())
        return false;

    // The pages are named to the copy below the way an insert from a file names them, by the
    // name each page carries in this document.
    std::vector<OUString> aPageNames;
    if (!collectPageNames(*mpDoc, rPages, aPageNames))
        return false;

    // The pages go into a presentation of their own, which starts out with the one page a new
    // document holds and loses it once the pages of this one are in.
    rtl::Reference<::sd::DrawDocShell> xShell(
        new ::sd::DrawDocShell(SfxObjectCreateMode::STANDARD, false, mpDoc->GetDocumentType()));
    SfxObjectShellLock xLock(xShell.get());
    xShell->DoInitNew();
    // The shell serves this one write and is closed again whichever way the function ends.
    comphelper::ScopeGuard aCloseShell([&xShell] { xShell->DoClose(); });

    SdDrawDocument* pTarget = xShell->GetDoc();
    if (!pTarget)
        return false;

    pTarget->CreateFirstPages();
    pTarget->StopWorkStartupDelay();

    // The pages travel with the design they carry here, and they keep the name that design
    // holds in this presentation. The master page the new document starts with is named
    // "Default", as most designs are, so it is moved to a name that neither presentation uses
    // and leaves the name to the design that arrives.
    if (SdPage* pStockMaster = pTarget->GetMasterSdPage(0, PageKind::Standard))
    {
        const OUString aStockName
            = SdDrawDocument::GetBaseLayoutName(pStockMaster->GetLayoutName());
        pTarget->RenameLayoutTemplate(
            pStockMaster->GetLayoutName(),
            SdDrawDocument::GetUnusedLayoutName(
                *pTarget, mpDoc, {}, SdDrawDocument::GenerateNewLayoutName(aStockName)));
    }

    const sal_uInt16 nPagesBefore = pTarget->GetSdPageCount(PageKind::Standard);
    InsertBookmarkOptions aOptions = InsertBookmarkOptions::ForSlideImport(/*bKeepDesign=*/true);
    if (!pTarget->InsertFileAsPage(aPageNames, /*pExchangeList=*/nullptr, aOptions,
                                   /*nInsertPos=*/0xFFFF, mpDocShell, /*oScaleObjects=*/true))
        return false;

    // The pages a new document starts with sit before the ones that were inserted, and the
    // written presentation holds the pages that were asked for and no others. A slide is a
    // standard page and the notes page that follows it, so both go: the notes page moves into
    // the place the standard page left.
    for (sal_uInt16 nPage = nPagesBefore; nPage > 0; --nPage)
    {
        const sal_uInt16 nPageNum
            = pTarget->GetSdPage(nPage - 1, PageKind::Standard)->GetPageNum();
        pTarget->RemovePage(nPageNum);
        if (nPageNum < pTarget->GetPageCount())
            pTarget->RemovePage(nPageNum);
    }

    // The master page the new document started with has lost the page that used it.
    pTarget->RemoveUnnecessaryMasterPages(nullptr, false, /*bUndo=*/false);

    const sal_uInt16 nWritten = pTarget->GetSdPageCount(PageKind::Standard);
    if (aPageNames.size() == static_cast<size_t>(nWritten))
    {
        for (sal_uInt16 nPage = 0; nPage < nWritten; ++nPage)
        {
            SdPage* pPage = pTarget->GetSdPage(nPage, PageKind::Standard);
            if (!pPage)
                continue;

            pPage->SetFileName(sd::SlideLink::OriginReference());
            pPage->SetBookmarkName(aPageNames[nPage]);
        }
    }

    try
    {
        uno::Reference<frame::XStorable> xStorable(xShell->GetModel(), uno::UNO_QUERY_THROW);
        xStorable->storeToURL(rFileUrl, {});
    }
    catch (const cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sd", "writing the pages of a presentation out failed");
        return false;
    }

    return true;
}

SdrModel& SdXImpressDocument::getSdrModelFromUnoModel() const
{
    OSL_ENSURE(GetDoc(), "No SdrModel in draw/Impress, should not happen");
    return *GetDoc(); // TTTT should be reference
}

void SdXImpressDocument::dispose()
{
    if( mbDisposed )
        return;

    ::SolarMutexGuard aGuard;

    if( mpDoc )
    {
        EndListening( *mpDoc );
        mpDoc = nullptr;
    }

    // Drop the extracted animated GIF files so they are removed from the jail.
    maAnimatedGifCache.clear();

    // Call the base class dispose() before setting the mbDisposed flag
    // to true.  The reason for this is that if close() has not yet been
    // called this is done in SfxBaseModel::dispose().  At the end of
    // that dispose() is called again.  It is important to forward this
    // second dispose() to the base class, too.
    // As a consequence the following code has to be able to be run twice.
    SfxBaseModel::dispose();
    mbDisposed = true;

    rtl::Reference< SdDocLinkTargets > xLinks( mxLinks );
    if( xLinks.is() )
    {
        xLinks->dispose();
        xLinks = nullptr;
    }

    rtl::Reference< SdDrawPagesAccess > xDrawPagesAccess( mxDrawPagesAccess );
    if( xDrawPagesAccess.is() )
    {
        xDrawPagesAccess->dispose();
        xDrawPagesAccess = nullptr;
    }

    rtl::Reference< SdMasterPagesAccess > xMasterPagesAccess( mxMasterPagesAccess );
    if( xDrawPagesAccess.is() )
    {
        xMasterPagesAccess->dispose();
        xMasterPagesAccess = nullptr;
    }

    rtl::Reference< SdLayerManager > xLayerManager( mxLayerManager );
    if( xLayerManager.is() )
    {
        xLayerManager->dispose();
        xLayerManager = nullptr;
    }

    mxDashTable = nullptr;
    mxGradientTable = nullptr;
    mxHatchTable = nullptr;
    mxBitmapTable = nullptr;
    mxTransGradientTable = nullptr;
    mxMarkerTable = nullptr;
    mxDrawingPool = nullptr;
    mpDocShell = nullptr;
}

SdDrawPagesAccess::SdDrawPagesAccess( SdXImpressDocument& rMyModel )  noexcept
:   mpModel( &rMyModel)
{
}

SdDrawPagesAccess::~SdDrawPagesAccess() noexcept
{
}

// XIndexAccess
sal_Int32 SdDrawPagesAccess::getCount()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
}

cpo::uno::Any SdDrawPagesAccess::getByIndex( sal_Int32 Index )
{
    uno::Reference< drawing::XDrawPage > xDrawPage( getDrawPageByIndex(Index) );
    return cpo::uno::Any(xDrawPage);
}

SdGenericDrawPage* SdDrawPagesAccess::getDrawPageByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    if( (Index < 0) || (Index >= mpModel->mpDoc->GetSdPageCount( PageKind::Standard ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = mpModel->mpDoc->GetSdPage( static_cast<sal_uInt16>(Index), PageKind::Standard );
    if( pPage )
        return dynamic_cast<SdGenericDrawPage*>( pPage->getUnoPage().get() );

    return nullptr;
}

// XNameAccess
cpo::uno::Any SdDrawPagesAccess::getByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    if( !aName.isEmpty() )
    {
        const sal_uInt16 nCount = mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
        sal_uInt16 nPage;
        for( nPage = 0; nPage < nCount; nPage++ )
        {
            SdPage* pPage = mpModel->mpDoc->GetSdPage( nPage, PageKind::Standard );
            if(nullptr == pPage)
                continue;

            if( aName == SdDrawPage::getPageApiName( pPage ) )
            {
                cpo::uno::Any aAny;
                uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage() );
                aAny <<= xDrawPage;
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
}

cpo::uno::Sequence< OUString > SdDrawPagesAccess::getElementNames()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    const sal_uInt16 nCount = mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
    cpo::uno::Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();

    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = mpModel->mpDoc->GetSdPage( nPage, PageKind::Standard );
        *pNames++ = SdDrawPage::getPageApiName( pPage );
    }

    return aNames;
}

bool SdDrawPagesAccess::hasByName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    const sal_uInt16 nCount = mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = mpModel->mpDoc->GetSdPage( nPage, PageKind::Standard );
        if(nullptr == pPage)
            continue;

        if( aName == SdDrawPage::getPageApiName( pPage ) )
            return true;
    }

    return false;
}

// XElementAccess
cpo::uno::Type SdDrawPagesAccess::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

bool SdDrawPagesAccess::hasElements()
{
    return getCount() > 0;
}

// XDrawPages

/**
 * Creates a new page with model at the specified position.
 * @returns corresponding SdDrawPage
 */
uno::Reference< drawing::XDrawPage > SdDrawPagesAccess::insertNewByIndex( sal_Int32 nIndex )
{
    ::SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("insertNewByIndex");

    if( nullptr == mpModel )
        throw lang::DisposedException();

    if( mpModel->mpDoc )
    {
        SdPage* pPage = mpModel->InsertSdPage( static_cast<sal_uInt16>(nIndex), false );
        if( pPage )
        {
            uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage() );
            return xDrawPage;
        }
    }
    uno::Reference< drawing::XDrawPage > xDrawPage;
    return xDrawPage;
}

/**
 * Removes the specified SdDrawPage from the model and the internal list. It
 * only works, if there is at least one *normal* page in the model after
 * removing this page.
 */
void SdDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel || mpModel->mpDoc == nullptr )
        throw lang::DisposedException();

    SdDrawDocument& rDoc = *mpModel->mpDoc;

    sal_uInt16 nPageCount = rDoc.GetSdPageCount( PageKind::Standard );
    if( nPageCount > 1 )
    {
        // get pPage from xPage and determine the Id (nPos ) afterwards
        SdDrawPage* pSvxPage = comphelper::getFromUnoTunnel<SdDrawPage>( xPage );
        if( pSvxPage )
        {
            SdPage* pPage = static_cast<SdPage*>(pSvxPage->GetSdrPage());
            if(pPage && ( pPage->GetPageKind() == PageKind::Standard ) )
            {
                sal_uInt16 nPage = pPage->GetPageNum();

                SdPage* pNotesPage = static_cast< SdPage* >( rDoc.GetPage( nPage+1 ) );

                bool bUndo = rDoc.IsUndoEnabled();
                if( bUndo )
                {
                    // Add undo actions and delete the pages.  The order of adding
                    // the undo actions is important.
                    rDoc.BegUndo( SdResId( STR_UNDO_DELETEPAGES ) );
                    if (pNotesPage)
                    {
                        rDoc.AddUndo(SdrUndoFactory::CreateUndoDeletePage(*pNotesPage));
                    }
                    rDoc.AddUndo(SdrUndoFactory::CreateUndoDeletePage(*pPage));
                }

                rDoc.RemovePage( nPage ); // the page
                if (pNotesPage)
                {
                    rDoc.RemovePage( nPage ); // the notes page
                }

                if( bUndo )
                {
                    rDoc.EndUndo();
                }
            }
        }
    }

    mpModel->SetModified();
}

// XServiceInfo

OUString SdDrawPagesAccess::getImplementationName(  )
{
    return u"SdDrawPagesAccess"_ustr;
}

bool SdDrawPagesAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

cpo::uno::Sequence< OUString > SdDrawPagesAccess::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.DrawPages"_ustr };
}

// XComponent
void SdDrawPagesAccess::dispose(  )
{
    mpModel = nullptr;
}

void SdDrawPagesAccess::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SdDrawPagesAccess::removeEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}


SdMasterPagesAccess::SdMasterPagesAccess( SdXImpressDocument& rMyModel ) noexcept
:   mpModel(&rMyModel)
{
}

SdMasterPagesAccess::~SdMasterPagesAccess() noexcept
{
}

// XComponent
void SdMasterPagesAccess::dispose(  )
{
    mpModel = nullptr;
}

void SdMasterPagesAccess::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SdMasterPagesAccess::removeEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

// XIndexAccess
sal_Int32 SdMasterPagesAccess::getCount()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel->mpDoc )
        throw lang::DisposedException();

    return mpModel->mpDoc->GetMasterSdPageCount(PageKind::Standard);
}

/**
 * Provides a drawing::XDrawPage interface for accessing the Masterpage at the
 * specified position in the model.
 */
cpo::uno::Any SdMasterPagesAccess::getByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SdMasterPagesAccess::getByIndex");

    if( nullptr == mpModel )
        throw lang::DisposedException();

    cpo::uno::Any aAny;

    if( (Index < 0) || (Index >= mpModel->mpDoc->GetMasterSdPageCount( PageKind::Standard ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = mpModel->mpDoc->GetMasterSdPage( static_cast<sal_uInt16>(Index), PageKind::Standard );
    if( pPage )
    {
        uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage() );
        aAny <<= xDrawPage;
    }

    return aAny;
}

// XElementAccess
cpo::uno::Type SdMasterPagesAccess::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

bool SdMasterPagesAccess::hasElements()
{
    return getCount() > 0;
}

// XDrawPages
uno::Reference< drawing::XDrawPage > SdMasterPagesAccess::insertNewByIndex( sal_Int32 nInsertPos )
{
    return insertNewImpl(nInsertPos, std::nullopt);
}

// XDrawPages2
uno::Reference< drawing::XDrawPage > SdMasterPagesAccess::insertNamedNewByIndex( sal_Int32 nInsertPos, const OUString& sName )
{
    return insertNewImpl(nInsertPos, sName);
}

rtl::Reference< SvxDrawPage > SdMasterPagesAccess::insertNewImpl( sal_Int32 nInsertPos, std::optional<OUString> oPageName )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    rtl::Reference< SvxDrawPage > xDrawPage;

    SdDrawDocument* pDoc = mpModel->mpDoc;
    if( pDoc )
    {
        // calculate internal index and check for range errors
        const sal_Int32 nMPageCount = pDoc->GetMasterPageCount();
        nInsertPos = nInsertPos * 2 + 1;
        if( nInsertPos < 0 || nInsertPos > nMPageCount )
            nInsertPos = nMPageCount;

        // now generate a unique name for the new masterpage
        OUString aPrefix;
        if (oPageName)
            aPrefix = *oPageName;
        else
        {
            const OUString aStdPrefix( SdResId(STR_LAYOUT_DEFAULT_NAME) );
            aPrefix = aStdPrefix;

            bool bUnique = true;

            std::vector<OUString> aPageNames;
            for (sal_Int32 nMaster = 1; nMaster < nMPageCount; ++nMaster)
            {
                const SdPage* pPage = static_cast<const SdPage*>(pDoc->GetMasterPage(static_cast<sal_uInt16>(nMaster)));
                if (!pPage)
                    continue;
                aPageNames.push_back(pPage->GetName());
                if (aPageNames.back() == aPrefix)
                    bUnique = false;
            }

            sal_Int32 i = 0;
            while (!bUnique)
            {
                aPrefix = aStdPrefix + " " + OUString::number(++i);
                bUnique = std::find(aPageNames.begin(), aPageNames.end(), aPrefix) == aPageNames.end();
            }
        }
        OUString aLayoutName = aPrefix + SD_LT_SEPARATOR + STR_LAYOUT_OUTLINE;

        // create styles
        static_cast<SdStyleSheetPool*>(pDoc->GetStyleSheetPool())->CreateLayoutStyleSheets( aPrefix );

        // get the first page for initial size and border settings
        SdPage* pPage = mpModel->mpDoc->GetSdPage( sal_uInt16(0), PageKind::Standard );
        SdPage* pRefNotesPage = mpModel->mpDoc->GetSdPage( sal_uInt16(0), PageKind::Notes);

        // create and insert new draw masterpage
        rtl::Reference<SdPage> pMPage = mpModel->mpDoc->AllocSdPage(true);
        pMPage->SetSize( pPage->GetSize() );
        pMPage->SetBorder( pPage->GetLeftBorder(),
                           pPage->GetUpperBorder(),
                           pPage->GetRightBorder(),
                           pPage->GetLowerBorder() );
        if (oPageName)
            // no need to update the page URLs on a brand new page
            pMPage->SetName(*oPageName, /*bUpdatePageRelativeURLs*/false);
        pMPage->SetLayoutName( aLayoutName );
        pDoc->InsertMasterPage(pMPage.get(),  static_cast<sal_uInt16>(nInsertPos));

        {
            // ensure default MasterPage fill
            pMPage->EnsureMasterPageDefaultBackground();
        }

        xDrawPage = pMPage->getUnoPage();

        // create and insert new notes masterpage
        rtl::Reference<SdPage> pMNotesPage = mpModel->mpDoc->AllocSdPage(true);
        pMNotesPage->SetSize( pRefNotesPage->GetSize() );
        pMNotesPage->SetPageKind(PageKind::Notes);
        pMNotesPage->SetBorder( pRefNotesPage->GetLeftBorder(),
                                pRefNotesPage->GetUpperBorder(),
                                pRefNotesPage->GetRightBorder(),
                                pRefNotesPage->GetLowerBorder() );
        pMNotesPage->SetLayoutName( aLayoutName );
        pDoc->InsertMasterPage(pMNotesPage.get(),  static_cast<sal_uInt16>(nInsertPos) + 1);
        pMNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true, true);
        mpModel->SetModified();
    }

    return xDrawPage;
}

/**
 * Removes the specified SdMasterPage from the model and the internal list. It
 * only works, if there is no *normal* page using this page as MasterPage in
 * the model.
 */
void SdMasterPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel || mpModel->mpDoc == nullptr )
        throw lang::DisposedException();

    SdMasterPage* pSdPage = comphelper::getFromUnoTunnel<SdMasterPage>( xPage );
    if(pSdPage == nullptr)
        return;

    SdPage* pPage = dynamic_cast< SdPage* > (pSdPage->GetSdrPage());

    DBG_ASSERT( pPage && pPage->IsMasterPage(), "SdMasterPage is not masterpage?");

    if( !pPage || !pPage->IsMasterPage() || (mpModel->mpDoc->GetMasterPageUserCount(pPage) > 0))
        return; //Todo: this should be excepted

    // only standard pages can be removed directly
    if( pPage->GetPageKind() != PageKind::Standard )
        return;

    sal_uInt16 nPage = pPage->GetPageNum();

    SdDrawDocument& rDoc = *mpModel->mpDoc;

    SdPage* pNotesPage = static_cast< SdPage* >( rDoc.GetMasterPage( nPage+1 ) );

    bool bUndo = rDoc.IsUndoEnabled();
    if( bUndo )
    {
        // Add undo actions and delete the pages.  The order of adding
        // the undo actions is important.
        rDoc.BegUndo( SdResId( STR_UNDO_DELETEPAGES ) );
        rDoc.AddUndo(SdrUndoFactory::CreateUndoDeletePage(*pNotesPage));
        rDoc.AddUndo(SdrUndoFactory::CreateUndoDeletePage(*pPage));
    }

    // remove both pages
    rDoc.RemoveMasterPage( nPage );
    rDoc.RemoveMasterPage( nPage );

    if( bUndo )
    {
        rDoc.EndUndo();
    }
}

// XServiceInfo

OUString SdMasterPagesAccess::getImplementationName(  )
{
    return u"SdMasterPagesAccess"_ustr;
}

bool SdMasterPagesAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

cpo::uno::Sequence< OUString > SdMasterPagesAccess::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.MasterPages"_ustr };
}

SdDocLinkTargets::SdDocLinkTargets(SdXImpressDocument& rMyModel)
    : mpModel(&rMyModel)
{
    for (sal_uInt16 i=0; i < SdLinkTargetType::Count; i++)
        aNames[i] = SdResId(aTypeResIds[i]);
}

SdDocLinkTargets::~SdDocLinkTargets() noexcept
{
}

// XComponent
void SdDocLinkTargets::dispose(  )
{
    mpModel = nullptr;
}

void SdDocLinkTargets::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SdDocLinkTargets::removeEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

// XNameAccess
cpo::uno::Any SdDocLinkTargets::getByName( const OUString& aName )
{
    if (mpModel)
    {
        for (sal_uInt16 i=0; i < SdLinkTargetType::Count; i++)
            if ( aNames[i] == aName )
                return cpo::uno::Any(uno::Reference< beans::XPropertySet >(new SdDocLinkTargetType( mpModel, i )));
    }

    throw container::NoSuchElementException();
}

cpo::uno::Sequence< OUString > SdDocLinkTargets::getElementNames()
{
    cpo::uno::Sequence<OUString> aRet(SdLinkTargetType::Count);
    OUString* pArray = aRet.getArray();
    for (sal_uInt16 i=0; i < SdLinkTargetType::Count; i++)
        pArray[i] = aNames[i];
    return aRet;
}

bool SdDocLinkTargets::hasByName( const OUString& aName )
{
    for (const auto & i : aNames)
        if ( i == aName )
            return true;
    return false;
}

// container::XElementAccess
cpo::uno::Type SdDocLinkTargets::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

bool SdDocLinkTargets::hasElements()
{
    return true;
}

SdPage* SdDocLinkTarget::FindPage( std::u16string_view rName ) const
{
    SdDrawDocument* pDoc = mpModel->GetDoc();
    if( pDoc == nullptr )
        return nullptr;

    const sal_uInt16 nMaxPages = pDoc->GetPageCount();
    const sal_uInt16 nMaxMasterPages = pDoc->GetMasterPageCount();

    sal_uInt16 nPage;
    SdPage* pPage;

    const bool bDraw = pDoc->GetDocumentType() == DocumentType::Draw;

    // standard pages
    for( nPage = 0; nPage < nMaxPages; nPage++ )
    {
        pPage = static_cast<SdPage*>(pDoc->GetPage( nPage ));
        if( (pPage->GetName() == rName) && (!bDraw || (pPage->GetPageKind() == PageKind::Standard)) )
            return pPage;
    }

    // master pages
    for( nPage = 0; nPage < nMaxMasterPages; nPage++ )
    {
        pPage = static_cast<SdPage*>(pDoc->GetMasterPage( nPage ));
        if( (pPage->GetName() == rName) && (!bDraw || (pPage->GetPageKind() == PageKind::Standard)) )
            return pPage;
    }

    return nullptr;
}

// XServiceInfo
OUString SdDocLinkTargets::getImplementationName()
{
    return u"SdDocLinkTargets"_ustr;
}

bool SdDocLinkTargets::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

cpo::uno::Sequence< OUString > SdDocLinkTargets::getSupportedServiceNames()
{
    return { u"com.sun.star.document.LinkTargets"_ustr };
}

SdDocLinkTargetType::SdDocLinkTargetType(SdXImpressDocument* pModel, sal_uInt16 nT)
    : mpModel(pModel)
    , mnType(nT)
{
    maName = SdResId(aTypeResIds[nT]);
}

// beans::XPropertySet

uno::Reference< beans::XPropertySetInfo > SdDocLinkTargetType::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > aRef;//(new SfxItemPropertySetInfo( lcl_GetLinkTargetMap() ));
    return aRef;
}

void SdDocLinkTargetType::setPropertyValue(const OUString& /* aPropertyName */,
            const cpo::uno::Any& /* aValue */)
{
    //  everything is read-only
}

cpo::uno::Any SdDocLinkTargetType::getPropertyValue(const OUString& PropertyName)
{
    cpo::uno::Any aRet;
    if ( PropertyName == "LinkDisplayName" )
        aRet <<= maName;

    return aRet;
}

void SdDocLinkTargetType::addPropertyChangeListener( const OUString&,
                            const uno::Reference<beans::XPropertyChangeListener>&)
{ OSL_FAIL("not implemented"); }

void SdDocLinkTargetType::removePropertyChangeListener( const OUString&,
                        const uno::Reference<beans::XPropertyChangeListener>&)
{ OSL_FAIL("not implemented"); }

void SdDocLinkTargetType::addVetoableChangeListener( const OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
{ OSL_FAIL("not implemented"); }

void SdDocLinkTargetType::removeVetoableChangeListener( const OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
{ OSL_FAIL("not implemented"); }

// document::XLinkTargetSupplier

uno::Reference< container::XNameAccess > SdDocLinkTargetType::getLinks()
{
    return new SdDocLinkTarget( mpModel, mnType );
}

// XServiceInfo
OUString SdDocLinkTargetType::getImplementationName()
{
    return u"SdDocLinkTargetType"_ustr;
}

bool SdDocLinkTargetType::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

cpo::uno::Sequence< OUString > SdDocLinkTargetType::getSupportedServiceNames()
{
    return { u"com.sun.star.document.LinkTargetSupplier"_ustr };
}

SdDocLinkTarget::SdDocLinkTarget( SdXImpressDocument* pModel, sal_uInt16 nT )
    : mpModel(pModel)
    , mnType(nT)
{
}

// container::XNameAccess

cpo::uno::Any SdDocLinkTarget::getByName(const OUString& aName)
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    SdPage* pPage = FindPage( aName );

    if( pPage == nullptr )
        throw container::NoSuchElementException();

    cpo::uno::Any aAny;

    uno::Reference< beans::XPropertySet > xProps( cppu::getXWeak(pPage->getUnoPage().get()), uno::UNO_QUERY );
    if( xProps.is() )
        aAny <<= xProps;

    return aAny;
}

cpo::uno::Sequence<OUString> SdDocLinkTarget::getElementNames()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    SdDrawDocument* pDoc = mpModel->GetDoc();
    if( pDoc == nullptr )
    {
        return { };
    }

    if( pDoc->GetDocumentType() == DocumentType::Draw )
    {
        const sal_uInt16 nMaxPages = pDoc->GetSdPageCount( PageKind::Standard );
        const sal_uInt16 nMaxMasterPages = pDoc->GetMasterSdPageCount( PageKind::Standard );

        cpo::uno::Sequence< OUString > aSeq( mnType == SdLinkTargetType::Page ? nMaxPages : nMaxMasterPages );
        OUString* pStr = aSeq.getArray();

        sal_uInt16 nPage;
        if (mnType == SdLinkTargetType::Page)
        {
            // standard pages
            for( nPage = 0; nPage < nMaxPages; nPage++ )
                *pStr++ = pDoc->GetSdPage( nPage, PageKind::Standard )->GetName();
        }
        else
        {
            // master pages
            for( nPage = 0; nPage < nMaxMasterPages; nPage++ )
                *pStr++ = pDoc->GetMasterSdPage( nPage, PageKind::Standard )->GetName();
        }
        return aSeq;
    }
    else
    {
        PageKind eKind;
        switch (mnType)
        {
            case SdLinkTargetType::Notes:
                eKind = PageKind::Notes;
                break;
            case SdLinkTargetType::Handout:
                eKind = PageKind::Handout;
                break;
            default:
                eKind = PageKind::Standard;
                break;
        }
        const sal_uInt16 nMaxPages = pDoc->GetSdPageCount( eKind );
        const sal_uInt16 nMaxMasterPages = pDoc->GetMasterPageCount();

        cpo::uno::Sequence< OUString > aSeq( mnType == SdLinkTargetType::MasterPage ? nMaxMasterPages : nMaxPages );
        OUString* pStr = aSeq.getArray();

        sal_uInt16 nPage;
        switch (mnType)
        {
            case SdLinkTargetType::Page:
            {
                for( nPage = 0; nPage < nMaxPages; nPage++ )
                    *pStr++ = pDoc->GetSdPage( nPage, PageKind::Standard )->GetName();
                break;
            }
            case SdLinkTargetType::Notes:
            {
                for( nPage = 0; nPage < nMaxPages; nPage++ )
                    *pStr++ = pDoc->GetSdPage( nPage, PageKind::Notes )->GetName();
                break;
            }
            case SdLinkTargetType::Handout:
            {
                for( nPage = 0; nPage < nMaxPages; nPage++ )
                    *pStr++ = pDoc->GetSdPage( nPage, PageKind::Handout )->GetName();
                break;
            }
            case SdLinkTargetType::MasterPage:
            {
                for( nPage = 0; nPage < nMaxMasterPages; nPage++ )
                    *pStr++ = static_cast<SdPage*>(pDoc->GetMasterPage( nPage ))->GetName();
                break;
            }
        }
        return aSeq;
    }
}

bool SdDocLinkTarget::hasByName(const OUString& aName)
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return FindPage( aName ) != nullptr;
}

// container::XElementAccess

cpo::uno::Type SdDocLinkTarget::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

bool SdDocLinkTarget::hasElements()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return mpModel->GetDoc() != nullptr;
}

// XServiceInfo
OUString SdDocLinkTarget::getImplementationName()
{
    return u"SdDocLinkTarget"_ustr;
}

bool SdDocLinkTarget::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

cpo::uno::Sequence< OUString > SdDocLinkTarget::getSupportedServiceNames()
{
    return { u"com.sun.star.document.LinkTargets"_ustr };
}

rtl::Reference< SdXImpressDocument > SdXImpressDocument::GetModel( SdDrawDocument const & rDocument )
{
    rtl::Reference< SdXImpressDocument > xRet;
    ::sd::DrawDocShell* pDocShell(rDocument.GetDocSh());
    if( pDocShell )
    {
        uno::Reference<frame::XModel> xModel(pDocShell->GetModel());

        xRet.set( dynamic_cast< SdXImpressDocument* >( xModel.get() ) );
    }

    return xRet;
}

void NotifyDocumentEvent(SdDrawDocument const & rDocument, const OUString& rEventName, const uno::Reference<cpo::uno::XInterface>& xSource)
{
    rtl::Reference<SdXImpressDocument> xModel(SdXImpressDocument::GetModel(rDocument));

    if (xModel.is())
    {
        css::document::EventObject aEvent(xSource, rEventName);
        xModel->notifyEvent(aEvent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
