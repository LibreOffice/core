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

#include <memory>

#include <com/sun/star/presentation/XPresentation2.hpp>

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

#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Impress.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/indexedpropertyvalues.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/profilezone.hxx>

#include <sal/log.hxx>
#include <editeng/unofield.hxx>
#include <notifydocumentevent.hxx>
#include <tpaction.hxx>
#include <unomodel.hxx>
#include "unopool.hxx"
#include <sfx2/lokhelper.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <editeng/UnoForbiddenCharsTable.hxx>
#include <svx/svdoutl.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/test_info.hxx>
#include <o3tl/unit_conversion.hxx>
#include <svx/UnoNamespaceMap.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdomedia.hxx>
#include <svx/unoapi.hxx>
#include <svx/unofill.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <editeng/fontitem.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdpagv.hxx>
#include <svtools/unoimap.hxx>
#include <svx/unoshape.hxx>
#include <editeng/unonrule.hxx>
#include <editeng/eeitem.hxx>
#include <unotools/datetime.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/autolayout.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/math.hxx>
#include <tools/helpers.hxx>
#include <tools/json_writer.hxx>

// Support creation of GraphicStorageHandler and EmbeddedObjectResolver
#include <svx/xmleohlp.hxx>
#include <svx/xmlgrhlp.hxx>
#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include "UnoDocumentSettings.hxx"

#include <Annotation.hxx>
#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <sdresid.hxx>
#include <sdpage.hxx>

#include <strings.hrc>
#include <strings.hxx>
#include "unolayer.hxx"
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
#include <Window.hxx>
#include <optsitem.hxx>
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

#include <sfx2/lokcomponenthelpers.hxx>
#include <sfx2/LokControlHandler.hxx>
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

#include <app.hrc>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::sd;

const TranslateId aTypeResIds[SdLinkTargetType::Count] =
{
    STR_SD_PAGE,            // SdLinkTargetType::Page
    STR_NOTES_MODE,         // SdLinkTargetType::Notes
    STR_HANDOUT,            // SdLinkTargetType::Handout
    STR_MASTERPAGE_NAME,    // SdLinkTargetType::MasterPage
};

TranslateId SdTPAction::GetClickActionSdResId( presentation::ClickAction eCA )
{
    switch( eCA )
    {
        case presentation::ClickAction_NONE:             return STR_CLICK_ACTION_NONE;
        case presentation::ClickAction_PREVPAGE:         return STR_CLICK_ACTION_PREVPAGE;
        case presentation::ClickAction_NEXTPAGE:         return STR_CLICK_ACTION_NEXTPAGE;
        case presentation::ClickAction_FIRSTPAGE:        return STR_CLICK_ACTION_FIRSTPAGE;
        case presentation::ClickAction_LASTPAGE:         return STR_CLICK_ACTION_LASTPAGE;
        case presentation::ClickAction_BOOKMARK:         return STR_CLICK_ACTION_BOOKMARK;
        case presentation::ClickAction_DOCUMENT:         return STR_CLICK_ACTION_DOCUMENT;
        case presentation::ClickAction_PROGRAM:          return STR_CLICK_ACTION_PROGRAM;
        case presentation::ClickAction_MACRO:            return STR_CLICK_ACTION_MACRO;
        case presentation::ClickAction_SOUND:            return STR_CLICK_ACTION_SOUND;
        case presentation::ClickAction_VERB:             return STR_CLICK_ACTION_VERB;
        case presentation::ClickAction_STOPPRESENTATION: return STR_CLICK_ACTION_STOPPRESENTATION;
        default: OSL_FAIL( "No StringResource for ClickAction available!" );
    }
    return {};
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
            mxBackground->getPropertyValue("FillTransparenceGradientName") >>= sGradientName;
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
    mxBackground->getPropertyValue("FillTransparence") >>= nFillTransparency;
    return nFillTransparency;
}

::Color SlideBackgroundInfo::getFillColor() const
{
    if (!mxBackground.is())
        return {};
    if (sal_Int32 nFillColor; mxBackground->getPropertyValue("FillColor") >>= nFillColor)
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
            if (xPropSet->getPropertySetInfo()->hasPropertyByName("Background"))
                xPropSet->getPropertyValue( "Background" ) >>= xBackground;
            if( xBackground.is() )
            {
                drawing::FillStyle aFillStyle;
                if( xBackground->getPropertyValue( "FillStyle" ) >>= aFillStyle )
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
using namespace ::css::uno;
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
    { animations::TransitionSubType::ROTATEIN, "RotateIn"}
});

constexpr auto constAnimationNodeTypeToString = mapEnumToString<sal_Int16>({
    { AnimationNodeType::ANIMATE, "Animate" },
    { AnimationNodeType::ANIMATECOLOR, "AnimateColor" },
    { AnimationNodeType::ANIMATEMOTION, "Animate" },
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
                       const Reference<drawing::XDrawPage>& xDrawPage);
    void exportAnimations();
    [[nodiscard]] bool hasEffects() const { return mbHasEffects; }

private:
    void exportNode(const Reference<XAnimationNode>& xNode);
    void exportNodeImpl(const Reference<XAnimationNode>& xNode);
    void exportContainer(const Reference<XTimeContainer>& xContainer);

    void exportAnimate(const Reference<XAnimate>& xAnimate);

    void convertValue(XMLTokenEnum eAttributeName, OStringBuffer& sTmp, const Any& rValue) const;
    static void convertTarget(OStringBuffer& sTmp, const Any& rTarget);
    static Reference<XInterface> getParagraphTarget( const ParagraphTarget& pTarget );
    void convertTiming(OStringBuffer& sTmp, const Any& rValue) const;

private:
    ::tools::JsonWriter& mrWriter;
    Reference<drawing::XDrawPage> mxDrawPage;
    Reference<XPropertySet> mxPageProps;
    Reference<XAnimationNode> mxRootNode;
    bool mbHasEffects;
};

AnimationsExporter::AnimationsExporter(::tools::JsonWriter& rWriter,
                                       const Reference<drawing::XDrawPage>& xDrawPage)
    : mrWriter(rWriter)
    , mxDrawPage(xDrawPage)
    , mbHasEffects(false)
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
                                                 css::uno::UNO_SET_THROW);
            if (xEnumeration->hasMoreElements())
            {
                // first child node may be an empty main sequence, check this
                Reference<XAnimationNode> xMainNode(xEnumeration->nextElement(), UNO_QUERY_THROW);
                Reference<XEnumerationAccess> xMainEnumerationAccess(xMainNode, UNO_QUERY_THROW);
                Reference<XEnumeration> xMainEnumeration(
                    xMainEnumerationAccess->createEnumeration(), css::uno::UNO_SET_THROW);

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
     if (!isValidNode(xNode))
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
                Reference<XInterface> xMaster;
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

Reference<XInterface> AnimationsExporter::getParagraphTarget(const ParagraphTarget& pTarget)
{
    try
    {
        Reference<XEnumerationAccess> xParaEnumAccess(pTarget.Shape, UNO_QUERY_THROW);

        Reference<XEnumeration> xEnumeration(xParaEnumAccess->createEnumeration(),
                                             css::uno::UNO_SET_THROW);
        sal_Int32 nParagraph = pTarget.Paragraph;

        while (xEnumeration->hasMoreElements())
        {
            Reference<XInterface> xRef(xEnumeration->nextElement(), UNO_QUERY);
            if (nParagraph-- == 0)
                return xRef;
        }
    }
    catch (const RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("sd", "AnimationsExporter::getParagraphTarget");
    }

    Reference<XInterface> xRef;
    return xRef;
}

void AnimationsExporter::convertTarget(OStringBuffer& sTmp, const Any& rTarget)
{
    if (!rTarget.hasValue())
        return;

    Reference<XInterface> xRef;
    if (!(rTarget >>= xRef))
    {
        if (auto pt = o3tl::tryAccess<ParagraphTarget>(rTarget))
        {
            xRef = getParagraphTarget(*pt);
        }
    }

    SAL_WARN_IF(!xRef.is(), "sd", "AnimationsExporter::convertTarget(), invalid target type!");
    if (xRef.is())
    {
        const std::string aIdentifier(GetInterfaceHash(xRef));
        if (!aIdentifier.empty())
            sTmp.append(aIdentifier);
    }
}

void AnimationsExporter::convertTiming(OStringBuffer& sTmp, const Any& rValue) const
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
                convertTarget(sTmp, pEvent->Source);
                sTmp.append('.');
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
                convertTarget(sTmp, aTemp);
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
                                             css::uno::UNO_SET_THROW);
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
            convertTarget(sTmp, aTemp);
            mrWriter.put("targetElement", sTmp.makeStringAndClear());
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
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("sd", "unomodel: AnimationsExporter");
    }
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
        { sUNO_Prop_HasValidSignatures,   WID_MODEL_HASVALIDSIGNATURES, ::cppu::UnoType<sal_Bool>::get(),                      beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_AllowLinkUpdate,      WID_MODEL_ALLOWLINKUPDATE,    ::cppu::UnoType<sal_Bool>::get(),                      beans::PropertyAttribute::READONLY, 0},
        { u"Fonts"_ustr,                  WID_MODEL_FONTS,              cppu::UnoType<uno::Sequence<uno::Any>>::get(),                     beans::PropertyAttribute::READONLY, 0},
        { sUNO_Prop_InteropGrabBag,       WID_MODEL_INTEROPGRABBAG,     cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get(),       0, 0},
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
uno::Any SAL_CALL SdXImpressDocument::queryInterface( const uno::Type & rType )
{
    uno::Any aAny;

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

void SAL_CALL SdXImpressDocument::acquire() noexcept
{
    SfxBaseModel::acquire();
}

void SAL_CALL SdXImpressDocument::release() noexcept
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
        catch (const uno::RuntimeException&)
        {
            // don't break throw ()
            TOOLS_WARN_EXCEPTION( "sd", "" );
        }
    }
    SfxBaseModel::release();
}

// XUnoTunnel
const css::uno::Sequence< sal_Int8 > & SdXImpressDocument::getUnoTunnelId() noexcept
{
    static const comphelper::UnoIdInit theSdXImpressDocumentUnoTunnelId;
    return theSdXImpressDocumentUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL SdXImpressDocument::getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier )
{
    if (comphelper::isUnoTunnelId<SdrModel>(rIdentifier))
        return comphelper::getSomething_cast(mpDoc);

    return comphelper::getSomethingImpl(rIdentifier, this,
                                        comphelper::FallbackToGetSomethingOf<SfxBaseModel>{});
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SdXImpressDocument::getTypes(  )
{
    ::SolarMutexGuard aGuard;

    if( !maTypeSequence.hasElements() )
    {
        uno::Sequence< uno::Type > aTypes( SfxBaseModel::getTypes() );
        aTypes = comphelper::concatSequences(aTypes,
            uno::Sequence {
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
                uno::Sequence {
                    cppu::UnoType<presentation::XPresentationSupplier>::get(),
                    cppu::UnoType<presentation::XCustomPresentationSupplier>::get(),
                    cppu::UnoType<presentation::XHandoutMasterSupplier>::get() });
        }
        maTypeSequence = std::move(aTypes);
    }

    return maTypeSequence;
}

uno::Sequence< sal_Int8 > SAL_CALL SdXImpressDocument::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

/***********************************************************************
*                                                                      *
***********************************************************************/
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
void SAL_CALL SdXImpressDocument::lockControllers(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    mpDoc->setLock(true);
}

void SAL_CALL SdXImpressDocument::unlockControllers(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    if( mpDoc->isLocked() )
    {
        mpDoc->setLock(false);
    }
}

sal_Bool SAL_CALL SdXImpressDocument::hasControllersLocked(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    return mpDoc->isLocked();
}

uno::Reference < container::XIndexAccess > SAL_CALL SdXImpressDocument::getViewData()
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

                    uno::Sequence< beans::PropertyValue > aSeq;
                    pFrameView->WriteUserDataSequence( aSeq );
                    xCont->insertByIndex( i, uno::Any( aSeq ) );
                }
            }
        }
    }

    return xRet;
}

void SAL_CALL SdXImpressDocument::setViewData( const uno::Reference < container::XIndexAccess >& xData )
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

    uno::Sequence< beans::PropertyValue > aSeq;
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
uno::Reference< drawing::XDrawPage > SAL_CALL SdXImpressDocument::duplicate( const uno::Reference< drawing::XDrawPage >& xPage )
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
            uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
            return xDrawPage;
        }
    }

    uno::Reference< drawing::XDrawPage > xDrawPage;
    return xDrawPage;
}

// XDrawPagesSupplier
uno::Reference< drawing::XDrawPages > SAL_CALL SdXImpressDocument::getDrawPages()
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
uno::Reference< drawing::XDrawPages > SAL_CALL SdXImpressDocument::getMasterPages()
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
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getLayerManager(  )
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
uno::Reference< container::XNameContainer > SAL_CALL SdXImpressDocument::getCustomPresentations()
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
uno::Reference< presentation::XPresentation > SAL_CALL SdXImpressDocument::getPresentation()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    return mpDoc->getPresentation();
}

// XHandoutMasterSupplier
uno::Reference< drawing::XDrawPage > SAL_CALL SdXImpressDocument::getHandoutMasterPage()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPage > xPage;

    initializeDocument();
    SdPage* pPage = mpDoc->GetMasterSdPage(0, PageKind::Handout);
    if (pPage)
        xPage.set(pPage->getUnoPage(), uno::UNO_QUERY);
    return xPage;
}

// XMultiServiceFactory ( SvxFmMSFactory )

css::uno::Reference<css::uno::XInterface> SdXImpressDocument::create(
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
        return uno::Reference< uno::XInterface >( SvxCreateNumRule( mpDoc ), uno::UNO_QUERY );
    }
    if( aServiceSpecifier == "com.sun.star.drawing.Background" )
    {
        return uno::Reference< uno::XInterface >(
            static_cast<uno::XWeak*>(new SdUnoPageBackground( mpDoc )));
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
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::DATE ));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.Header" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.Header" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::PRESENTATION_HEADER ));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.Footer" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.Footer" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::PRESENTATION_FOOTER ));
    }

    if( aServiceSpecifier == "com.sun.star.presentation.TextField.DateTime" ||
        aServiceSpecifier == "com.sun.star.presentation.textfield.DateTime" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::PRESENTATION_DATE_TIME ));
    }

    if( aServiceSpecifier == "com.sun.star.text.TextField.PageName" ||
        aServiceSpecifier == "com.sun.star.text.textfield.PageName" )
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField( text::textfield::Type::PAGE_NAME ));
    }

    if (aServiceSpecifier == "com.sun.star.text.TextField.DocInfo.Custom" ||
        aServiceSpecifier == "com.sun.star.text.textfield.DocInfo.Custom")
    {
        return static_cast<cppu::OWeakObject *>(new SvxUnoTextField(text::textfield::Type::DOCINFO_CUSTOM));
    }

    if( aServiceSpecifier == "com.sun.star.xml.NamespaceMap" )
    {
        static sal_uInt16 aWhichIds[] = { SDRATTR_XMLATTRIBUTES, EE_CHAR_XMLATTRIBS, EE_PARA_XMLATTRIBS, 0 };

        return svx::NamespaceMap_createInstance( aWhichIds, &mpDoc->GetItemPool() );
    }

    // Support creation of GraphicStorageHandler and EmbeddedObjectResolver
    if (aServiceSpecifier == "com.sun.star.document.ExportGraphicStorageHandler")
    {
        return static_cast<cppu::OWeakObject *>(new SvXMLGraphicHelper( SvXMLGraphicHelperMode::Write ));
    }

    if (aServiceSpecifier == "com.sun.star.document.ImportGraphicStorageHandler")
    {
        return static_cast<cppu::OWeakObject *>(new SvXMLGraphicHelper( SvXMLGraphicHelperMode::Read ));
    }

    if( aServiceSpecifier == "com.sun.star.document.ExportEmbeddedObjectResolver" )
    {
        comphelper::IEmbeddedHelper* pPersist = mpDoc->GetPersist();
        if( nullptr == pPersist )
            throw lang::DisposedException();

        return static_cast<cppu::OWeakObject *>(new SvXMLEmbeddedObjectHelper( *pPersist, SvXMLEmbeddedObjectHelperMode::Write ));
    }

    if( aServiceSpecifier == "com.sun.star.document.ImportEmbeddedObjectResolver" )
    {
        comphelper::IEmbeddedHelper* pPersist = mpDoc->GetPersist();
        if( nullptr == pPersist )
            throw lang::DisposedException();

        return static_cast<cppu::OWeakObject *>(new SvXMLEmbeddedObjectHelper( *pPersist, SvXMLEmbeddedObjectHelperMode::Read ));
    }

    uno::Reference< uno::XInterface > xRet;

    if( aServiceSpecifier.startsWith( "com.sun.star.presentation.") )
    {
        const std::u16string_view aType( aServiceSpecifier.subView(26) );
        rtl::Reference<SvxShape> pShape;

        SdrObjKind nType = SdrObjKind::Text;
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
        pShape = CreateSvxShapeByTypeAndInventor( nType, SdrInventor::Default, referer );

        // set shape type
        if( pShape && !mbClipBoard )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = static_cast<uno::XWeak*>(pShape.get());
    }
    else if ( aServiceSpecifier == "com.sun.star.drawing.TableShape" )
    {
        rtl::Reference<SvxShape> pShape = CreateSvxShapeByTypeAndInventor( SdrObjKind::Table, SdrInventor::Default, referer );
        if( pShape && !mbClipBoard )
            pShape->SetShapeType(aServiceSpecifier);

        xRet = static_cast<uno::XWeak*>(pShape.get());
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

uno::Reference< uno::XInterface > SAL_CALL SdXImpressDocument::createInstance( const OUString& aServiceSpecifier )
{
    return create(aServiceSpecifier, u""_ustr);
}

css::uno::Reference<css::uno::XInterface>
SdXImpressDocument::createInstanceWithArguments(
    OUString const & ServiceSpecifier,
    css::uno::Sequence<css::uno::Any> const & Arguments)
{
    OUString arg;
    if ((ServiceSpecifier == "com.sun.star.drawing.GraphicObjectShape"
         || ServiceSpecifier == "com.sun.star.drawing.AppletShape"
         || ServiceSpecifier == "com.sun.star.drawing.FrameShape"
         || ServiceSpecifier == "com.sun.star.drawing.OLE2Shape"
         || ServiceSpecifier == "com.sun.star.drawing.MediaShape"
         || ServiceSpecifier == "com.sun.star.drawing.PluginShape"
         || ServiceSpecifier == "com.sun.star.presentation.MediaShape")
        && Arguments.getLength() == 1 && (Arguments[0] >>= arg))
    {
        return create(ServiceSpecifier, arg);
    }
    return SvxFmMSFactory::createInstanceWithArguments(
        ServiceSpecifier, Arguments);
}

uno::Sequence< OUString > SAL_CALL SdXImpressDocument::getAvailableServiceNames()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    const uno::Sequence< OUString > aSNS_ORG( SvxFmMSFactory::getAvailableServiceNames() );

    uno::Sequence< OUString > aSNS_Common{ u"com.sun.star.drawing.DashTable"_ustr,
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

    uno::Sequence< OUString > aSNS_Specific;

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
OUString SAL_CALL SdXImpressDocument::getImplementationName()
{
    return u"SdXImpressDocument"_ustr;
    /* // Matching the .component information:
       return mbImpressDoc
           ? OUString("com.sun.star.comp.Draw.PresentationDocument")
           : OUString("com.sun.star.comp.Draw.DrawingDocument");
    */
}

sal_Bool SAL_CALL SdXImpressDocument::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SdXImpressDocument::getSupportedServiceNames()
{
    ::SolarMutexGuard aGuard;

    return { u"com.sun.star.document.OfficeDocument"_ustr,
             u"com.sun.star.drawing.GenericDrawingDocument"_ustr,
             u"com.sun.star.drawing.DrawingDocumentFactory"_ustr,
             mbImpressDoc?u"com.sun.star.presentation.PresentationDocument"_ustr:u"com.sun.star.drawing.DrawingDocument"_ustr };
}

// XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL SdXImpressDocument::getPropertySetInfo(  )
{
    ::SolarMutexGuard aGuard;
    return mpPropSet->getPropertySetInfo();
}

void SAL_CALL SdXImpressDocument::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
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
        case WID_MODEL_THEME:
            {
                SdrModel& rModel = getSdrModelFromUnoModel();
                std::shared_ptr<model::Theme> pTheme = model::Theme::FromAny(aValue);
                rModel.setTheme(pTheme);
            }
            break;
        default:
            throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    SetModified();
}

uno::Any SAL_CALL SdXImpressDocument::getPropertyValue( const OUString& PropertyName )
{
    ::SolarMutexGuard aGuard;

    uno::Any aAny;
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
            return uno::Any( maBuildId );
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
                uno::Sequence<uno::Any> aSeq;
                int nSeqIndex = 0;

                sal_uInt16 const aWhichIds[] { EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK,
                                               EE_CHAR_FONTINFO_CTL };

                const SfxItemPool& rPool = mpDoc->GetPool();

                for(sal_uInt16 nWhichId : aWhichIds)
                {
                    ItemSurrogates aSurrogates;
                    rPool.GetItemSurrogates(aSurrogates, nWhichId);
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

                aSeq.realloc( nSeqIndex );
                aAny <<= aSeq;
                break;
            }
        case WID_MODEL_INTEROPGRABBAG:
            getGrabBagItem(aAny);
            break;
        case WID_MODEL_THEME:
            {
                SdrModel& rModel = getSdrModelFromUnoModel();
                auto const& pTheme = rModel.getTheme();
                if (pTheme)
                {
                    pTheme->ToAny(aAny);
                }
                else
                {
                    beans::PropertyValues aValues;
                    aAny <<= aValues;
                }
                break;
            }
        default:
            throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));
    }

    return aAny;
}

void SAL_CALL SdXImpressDocument::addPropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdXImpressDocument::removePropertyChangeListener( const OUString& , const uno::Reference< beans::XPropertyChangeListener >&  ) {}
void SAL_CALL SdXImpressDocument::addVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) {}
void SAL_CALL SdXImpressDocument::removeVetoableChangeListener( const OUString& , const uno::Reference< beans::XVetoableChangeListener >&  ) {}

// XLinkTargetSupplier
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getLinks()
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
uno::Reference< container::XNameAccess > SAL_CALL SdXImpressDocument::getStyleFamilies(  )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpDoc )
        throw lang::DisposedException();

    uno::Reference< container::XNameAccess > xStyles( static_cast< OWeakObject* >( mpDoc->GetStyleSheetPool() ), css::uno::UNO_QUERY );
    return xStyles;
}

// XAnyCompareFactory
uno::Reference< css::ucb::XAnyCompare > SAL_CALL SdXImpressDocument::createAnyCompareByName( const OUString& )
{
    return SvxCreateNumRuleCompare();
}

// XRenderable
sal_Int32 SAL_CALL SdXImpressDocument::getRendererCount( const uno::Any& rSelection,
                                                         const uno::Sequence< beans::PropertyValue >&  )
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

uno::Sequence< beans::PropertyValue > SAL_CALL SdXImpressDocument::getRenderer( sal_Int32 , const uno::Any& ,
                                                                                const uno::Sequence< beans::PropertyValue >& rxOptions )
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
    uno::Sequence< beans::PropertyValue > aRenderer;
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
    catch (const uno::Exception&)
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
            uno::Any aAny( xShapePropSet->getPropertyValue( u"OnClick"_ustr ) );
            if ( aAny >>= eCa )
            {
                OUString const actionName(SdResId(SdTPAction::GetClickActionSdResId(eCa)));
                switch ( eCa )
                {
                    case presentation::ClickAction_LASTPAGE :
                    {
                        sal_Int32 nCount = rDoc.GetSdPageCount( PageKind::Standard );
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nCount - 1, vcl::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink(aLinkRect, actionName);
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;
                    case presentation::ClickAction_FIRSTPAGE :
                    {
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, 0, vcl::PDFWriter::DestAreaType::FitRectangle );
                        sal_Int32 nLinkId = rPDFExtOutDevData.CreateLink(aLinkRect, actionName);
                        rPDFExtOutDevData.SetLinkDest( nLinkId, nDestId );
                    }
                    break;
                    case presentation::ClickAction_PREVPAGE :
                    {
                        sal_Int32 nDestPage = rPDFExtOutDevData.GetCurrentPageNumber();
                        if ( nDestPage )
                            nDestPage--;
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nDestPage, vcl::PDFWriter::DestAreaType::FitRectangle );
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
                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nDestPage, vcl::PDFWriter::DestAreaType::FitRectangle );
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
                                        sal_Int32 nDestId = rPDFExtOutDevData.CreateDest( aPageRect, nPage, vcl::PDFWriter::DestAreaType::FitRectangle );
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

void SAL_CALL SdXImpressDocument::render( sal_Int32 nRenderer, const uno::Any& rSelection,
                                          const uno::Sequence< beans::PropertyValue >& rxOptions )
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
                uno::Any aAny;
                uno::Reference< drawing::XDrawPage > xPage( uno::Reference< drawing::XDrawPage >::query( pPage->getUnoPage() ) );
                if ( xPage.is() )
                {
                    if ( pPDFExtOutDevData->GetIsExportNotes() )
                        ImplPDFExportComments( xPage, *pPDFExtOutDevData );
                    uno::Reference< beans::XPropertySet > xPagePropSet( xPage, uno::UNO_QUERY );
                    if( xPagePropSet.is() )
                    {
                        // exporting object interactions to pdf

                        // if necessary, the master page interactions will be exported first
                        bool bIsBackgroundObjectsVisible = false;   // #i39428# IsBackgroundObjectsVisible not available for Draw
                        if ( mbImpressDoc && xPagePropSet->getPropertySetInfo()->hasPropertyByName( u"IsBackgroundObjectsVisible"_ustr ) )
                            xPagePropSet->getPropertyValue( u"IsBackgroundObjectsVisible"_ustr ) >>= bIsBackgroundObjectsVisible;
                        if ( bIsBackgroundObjectsVisible && !pPDFExtOutDevData->GetIsExportNotesPages() )
                        {
                            uno::Reference< drawing::XMasterPageTarget > xMasterPageTarget( xPage, uno::UNO_QUERY );
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
                            vcl::PDFWriter::PageTransition eType = vcl::PDFWriter::PageTransition::Regular;
                            if ( xPagePropSet->getPropertySetInfo( )->hasPropertyByName( sEffect ) )
                            {
                                aAny = xPagePropSet->getPropertyValue( sEffect );
                                if ( aAny >>= eFe )
                                {
                                    switch( eFe )
                                    {
                                        case presentation::FadeEffect_HORIZONTAL_LINES :
                                        case presentation::FadeEffect_HORIZONTAL_CHECKERBOARD :
                                        case presentation::FadeEffect_HORIZONTAL_STRIPES : eType = vcl::PDFWriter::PageTransition::BlindsHorizontal; break;

                                        case presentation::FadeEffect_VERTICAL_LINES :
                                        case presentation::FadeEffect_VERTICAL_CHECKERBOARD :
                                        case presentation::FadeEffect_VERTICAL_STRIPES : eType = vcl::PDFWriter::PageTransition::BlindsVertical; break;

                                        case presentation::FadeEffect_UNCOVER_TO_RIGHT :
                                        case presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT :
                                        case presentation::FadeEffect_ROLL_FROM_LEFT :
                                        case presentation::FadeEffect_FADE_FROM_UPPERLEFT :
                                        case presentation::FadeEffect_MOVE_FROM_UPPERLEFT :
                                        case presentation::FadeEffect_FADE_FROM_LEFT :
                                        case presentation::FadeEffect_MOVE_FROM_LEFT : eType = vcl::PDFWriter::PageTransition::WipeLeftToRight; break;

                                        case presentation::FadeEffect_UNCOVER_TO_BOTTOM :
                                        case presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT :
                                        case presentation::FadeEffect_ROLL_FROM_TOP :
                                        case presentation::FadeEffect_FADE_FROM_UPPERRIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_UPPERRIGHT :
                                        case presentation::FadeEffect_FADE_FROM_TOP :
                                        case presentation::FadeEffect_MOVE_FROM_TOP : eType = vcl::PDFWriter::PageTransition::WipeTopToBottom; break;

                                        case presentation::FadeEffect_UNCOVER_TO_LEFT :
                                        case presentation::FadeEffect_UNCOVER_TO_LOWERLEFT :
                                        case presentation::FadeEffect_ROLL_FROM_RIGHT :

                                        case presentation::FadeEffect_FADE_FROM_LOWERRIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_LOWERRIGHT :
                                        case presentation::FadeEffect_FADE_FROM_RIGHT :
                                        case presentation::FadeEffect_MOVE_FROM_RIGHT : eType = vcl::PDFWriter::PageTransition::WipeRightToLeft; break;

                                        case presentation::FadeEffect_UNCOVER_TO_TOP :
                                        case presentation::FadeEffect_UNCOVER_TO_UPPERLEFT :
                                        case presentation::FadeEffect_ROLL_FROM_BOTTOM :
                                        case presentation::FadeEffect_FADE_FROM_LOWERLEFT :
                                        case presentation::FadeEffect_MOVE_FROM_LOWERLEFT :
                                        case presentation::FadeEffect_FADE_FROM_BOTTOM :
                                        case presentation::FadeEffect_MOVE_FROM_BOTTOM : eType = vcl::PDFWriter::PageTransition::WipeBottomToTop; break;

                                        case presentation::FadeEffect_OPEN_VERTICAL : eType = vcl::PDFWriter::PageTransition::SplitHorizontalInward; break;
                                        case presentation::FadeEffect_CLOSE_HORIZONTAL : eType = vcl::PDFWriter::PageTransition::SplitHorizontalOutward; break;

                                        case presentation::FadeEffect_OPEN_HORIZONTAL : eType = vcl::PDFWriter::PageTransition::SplitVerticalInward; break;
                                        case presentation::FadeEffect_CLOSE_VERTICAL : eType = vcl::PDFWriter::PageTransition::SplitVerticalOutward; break;

                                        case presentation::FadeEffect_FADE_TO_CENTER : eType = vcl::PDFWriter::PageTransition::BoxInward; break;
                                        case presentation::FadeEffect_FADE_FROM_CENTER : eType = vcl::PDFWriter::PageTransition::BoxOutward; break;

                                        case presentation::FadeEffect_NONE : eType = vcl::PDFWriter::PageTransition::Regular; break;

                                        case presentation::FadeEffect_RANDOM :
                                        case presentation::FadeEffect_DISSOLVE :
                                        default: eType = vcl::PDFWriter::PageTransition::Dissolve; break;
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
            catch (const uno::Exception&)
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
                            pPDFExtOutDevData->SetLinkDest( rBookmark.nLinkId, pPDFExtOutDevData->CreateDest( aPageRect, nPage, vcl::PDFWriter::DestAreaType::FitRectangle ) );
                        else
                            pPDFExtOutDevData->DescribeRegisteredDest( rBookmark.nDestId, aPageRect, nPage, vcl::PDFWriter::DestAreaType::FitRectangle );
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
            catch (const uno::Exception&)
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
    comphelper::LibreOfficeKit::setTiledPainting(true);

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
    const Fraction scale = conversionFract(o3tl::Length::px, o3tl::Length::twip);
    Fraction scaleX = Fraction(nOutputWidth, nTileWidth) * scale;
    Fraction scaleY = Fraction(nOutputHeight, nTileHeight) * scale;

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

    rDevice.SetOutputSizePixel( Size(nOutputWidth, nOutputHeight) );

    Point aPoint(nTilePosXHMM, nTilePosYHMM);
    Size aSize(nTileWidthHMM, nTileHeightHMM);
    ::tools::Rectangle aRect(aPoint, aSize);

    SdrView* pView = pViewSh->GetDrawView();
    if (comphelper::LibreOfficeKit::isActive())
        pView->SetPaintTextEdit(mbPaintTextEdit);

    pViewSh->GetView()->CompleteRedraw(&rDevice, vcl::Region(aRect));

    if (comphelper::LibreOfficeKit::isActive())
        pView->SetPaintTextEdit(true);

    LokChartHelper::PaintAllChartsOnTile(rDevice, nOutputWidth, nOutputHeight,
                                         nTilePosX, nTilePosY, nTileWidth, nTileHeight);
    LokStarMathHelper::PaintAllInPlaceOnTile(rDevice, nOutputWidth, nOutputHeight, nTilePosX,
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
        LokControlHandler::paintControlTile(pPage, pDrawView, *pActiveWin, rDevice, aOutputSize, aTileRect);
    }

    comphelper::LibreOfficeKit::setTiledPainting(false);
}

OString SdXImpressDocument::getViewRenderState(SfxViewShell* pViewShell)
{
    OStringBuffer aState;
    DrawViewShell* pView = nullptr;

    if (ViewShellBase* pShellBase = dynamic_cast<ViewShellBase*>(pViewShell))
        pView = dynamic_cast<DrawViewShell*>(pShellBase->GetMainViewShell().get());
    else
        pView = GetViewShell();

    if (pView)
    {
        const SdViewOptions& pVOpt = pView->GetViewOptions();
        if (mpDoc->GetOnlineSpell())
            aState.append('S');
        if (pVOpt.mnDocBackgroundColor == svtools::ColorConfig::GetDefaultColor(svtools::DOCCOLOR, 1))
            aState.append('D');
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

    pViewSh->SelectPage(nPart, nSelect);
}

void SdXImpressDocument::moveSelectedParts(int nPosition, bool bDuplicate)
{
    // Duplicating is currently unsupported.
    if (!bDuplicate)
        mpDoc->MovePages(nPosition);
}

OUString SdXImpressDocument::getPartInfo(int nPart)
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return OUString();

    const SdPage* pSdPage = mpDoc->GetSdPage(nPart, pViewSh->GetPageKind());
    const bool bIsVisible = pSdPage && !pSdPage->IsExcluded();
    const bool bIsSelected = pViewSh->IsSelected(nPart);
    const sal_Int16 nMasterPageCount= pViewSh->GetDoc()->GetMasterSdPageCount(pViewSh->GetPageKind());

    OUString aPartInfo = "{ \"visible\": \"" +
        OUString::number(static_cast<unsigned int>(bIsVisible)) +
        "\", \"selected\": \"" +
        OUString::number(static_cast<unsigned int>(bIsSelected)) +
        "\", \"masterPageCount\": \"" +
        OUString::number(nMasterPageCount) +
        "\", \"mode\": \"" +
        OUString::number(getEditMode()) +
        "\" }";

    return aPartInfo;
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

    uno::Reference<drawing::XDrawPage> xDrawPage(pPage->getUnoPage(), uno::UNO_QUERY);
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

    if (VclPtr<vcl::Window> pWindow = SfxLokHelper::getInPlaceDocWindow(pViewShell->GetViewShell()))
        return pWindow;

    return pViewShell->GetActiveWindow();
}

void SdXImpressDocument::setPartMode( int nPartMode )
{
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    PageKind aPageKind( PageKind::Standard );
    switch ( nPartMode )
    {
    case LOK_PARTMODE_SLIDES:
        break;
    case LOK_PARTMODE_NOTES:
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
            uno::Reference<text::XText> xText(xAnnotation->getTextRange());
            rJsonWriter.put("text", xText->getString());
            rJsonWriter.put("parthash", pPage->GetUniqueID());
            geometry::RealPoint2D const aPoint = xAnnotation->getPosition();
            geometry::RealSize2D const aSize = xAnnotation->getSize();
            ::tools::Rectangle aRectangle(Point(aPoint.X * 100.0, aPoint.Y * 100.0), Size(aSize.Width * 100.0, aSize.Height * 100.0));
            aRectangle = o3tl::toTwips(aRectangle, o3tl::Length::mm100);
            OString sRectangle = aRectangle.toString();
            rJsonWriter.put("rectangle", sRectangle.getStr());
        }
    }
}

void SdXImpressDocument::initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
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
                pDrawView->SetAuthor(rValue.Value.get<OUString>());
            else if (rValue.Name == ".uno:SpellOnline" && rValue.Value.has<bool>())
                mpDoc->SetOnlineSpell(rValue.Value.get<bool>());
            else if (rValue.Name == ".uno:ChangeTheme" && rValue.Value.has<OUString>())
                sThemeName = rValue.Value.get<OUString>();
            else if (rValue.Name == ".uno:InvertBackground" && rValue.Value.has<OUString>())
                sBackgroundThemeName = rValue.Value.get<OUString>();
        }

        // Disable comments if requested
        SdOptions* pOptions = SdModule::get()->GetSdOptions(mpDoc->GetDocumentType());
        pOptions->SetShowComments(comphelper::LibreOfficeKit::isTiledAnnotations());

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

    if (!o3tl::IsRunningUnitTest() || !comphelper::LibreOfficeKit::isActive())
        officecfg::Office::Impress::MultiPaneGUI::SlideSorterBar::Visible::ImpressView::set(true,xChanges);
    xChanges->commit();

    // if we know what theme the user wants, then we can dispatch that now early
    if (!sThemeName.isEmpty())
    {
        css::uno::Sequence<css::beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            { "NewTheme", uno::Any(sThemeName) }
        }));
        comphelper::dispatchCommand(u".uno:ChangeTheme"_ustr, aPropertyValues);
    }
    if (!sBackgroundThemeName.isEmpty())
    {
        css::uno::Sequence<css::beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            { "NewTheme", uno::Any(sBackgroundThemeName) }
        }));
        comphelper::dispatchCommand(".uno:InvertBackground", aPropertyValues);
    }
}

void SdXImpressDocument::postKeyEvent(int nType, int nCharCode, int nKeyCode)
{
    SolarMutexGuard aGuard;
    SfxLokHelper::postKeyEventAsync(getDocWindow(), nType, nCharCode, nKeyCode);
}

void SdXImpressDocument::postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    constexpr double fScale = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::px);

    if (SfxLokHelper::testInPlaceComponentMouseEventHit(
            pViewShell->GetViewShell(), nType, nX, nY, nCount, nButtons, nModifier, fScale, fScale))
        return;

    // try to forward mouse event to control
    const Point aPointTwip(nX, nY);
    const Point aPointHMM = o3tl::convert(aPointTwip, o3tl::Length::twip, o3tl::Length::mm100);
    SdrView* pDrawView = pViewShell->GetDrawView();
    SdrPageView* pPageView = pDrawView->GetSdrPageView();
    SdrPage* pPage = pPageView->GetPage();
    ::sd::Window* pActiveWin = pViewShell->GetActiveWindow();
    if (!pActiveWin)
    {
        return;
    }

    if (LokControlHandler::postMouseEvent(pPage, pDrawView, *pActiveWin, nType, aPointHMM, nCount, nButtons, nModifier))
            return;

    LokMouseEventData aMouseEventData(nType, aPointHMM, nCount, MouseEventModifiers::SIMPLECLICK,
                                      nButtons, nModifier);
    SfxLokHelper::postMouseEventAsync(pViewShell->GetActiveWindow(), aMouseEventData);
}

void SdXImpressDocument::setTextSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    LokChartHelper aChartHelper(pViewShell->GetViewShell());
    if (aChartHelper.setTextSelection(nType, nX, nY))
        return;

    Point aPoint(convertTwipToMm100(nX), convertTwipToMm100(nY));
    switch (nType)
    {
    case LOK_SETTEXTSELECTION_START:
        pViewShell->SetCursorMm100Position(aPoint, /*bPoint=*/false, /*bClearMark=*/false);
        break;
    case LOK_SETTEXTSELECTION_END:
        pViewShell->SetCursorMm100Position(aPoint, /*bPoint=*/true, /*bClearMark=*/false);
        break;
    case LOK_SETTEXTSELECTION_RESET:
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

void SdXImpressDocument::setGraphicSelection(int nType, int nX, int nY)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    constexpr double fScale = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::px);

    LokChartHelper aChartHelper(pViewShell->GetViewShell());
    if (aChartHelper.setGraphicSelection(nType, nX, nY, fScale, fScale))
        return;

    Point aPoint(convertTwipToMm100(nX), convertTwipToMm100(nY));
    switch (nType)
    {
    case LOK_SETGRAPHICSELECTION_START:
        pViewShell->SetGraphicMm100Position(/*bStart=*/true, aPoint);
        break;
    case LOK_SETGRAPHICSELECTION_END:
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

void SdXImpressDocument::setClientVisibleArea(const ::tools::Rectangle& rRectangle)
{
    SolarMutexGuard aGuard;

    DrawViewShell* pViewShell = GetViewShell();
    if (!pViewShell)
        return;

    pViewShell->GetViewShellBase().setLOKVisibleArea(rRectangle);
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
    xShapeProps->getPropertyValue("Visible") >>= bIsShapeVisible;

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

OString SdXImpressDocument::getPresentationInfo() const
{
    ::tools::JsonWriter aJsonWriter;

    try
    {
        rtl::Reference<SdDrawPagesAccess> xDrawPages = const_cast<SdXImpressDocument*>(this)->getSdDrawPages();
        // size in twips
        Size aDocSize = const_cast<SdXImpressDocument*>(this)->getDocumentSize();
        aJsonWriter.put("docWidth", aDocSize.getWidth());
        aJsonWriter.put("docHeight", aDocSize.getHeight());

        sd::PresentationSettings const& rSettings = mpDoc->getPresentationSettings();

        const bool bIsEndless = rSettings.mbEndless;
        aJsonWriter.put("isEndless", bIsEndless);

        if (bIsEndless) {
            const sal_Int32 nPauseTimeout = rSettings.mnPauseTimeout;
            aJsonWriter.put("loopAndRepeatDuration", nPauseTimeout);
        }

        auto aSlideList = aJsonWriter.startArray("slides");
        sal_Int32 nSlideCount = xDrawPages->getCount();
        for (sal_Int32 i = 0; i < nSlideCount; ++i)
        {
            SdGenericDrawPage* pSlide(xDrawPages->getDrawPageByIndex(i));
            bool bIsVisible = true; // default visible
            pSlide->getPropertyValue("Visible") >>= bIsVisible;
            if (!bIsVisible)
            {
                auto aSlideNode = aJsonWriter.startStruct();
                std::string sSlideHash = GetInterfaceHash(cppu::getXWeak(pSlide));
                aJsonWriter.put("hash", sSlideHash);
                aJsonWriter.put("index", i);
                aJsonWriter.put("hidden", true);
            }
            else
            {
                SdrPage* pPage = pSlide->GetSdrPage();

                auto aSlideNode = aJsonWriter.startStruct();
                std::string sSlideHash = GetInterfaceHash(cppu::getXWeak(pSlide));
                aJsonWriter.put("hash", sSlideHash);
                aJsonWriter.put("index", i);

                bool bIsDrawPageEmpty = pSlide->getCount() == 0;
                aJsonWriter.put("empty", bIsDrawPageEmpty);

                // Notes
                SdPage* pNotesPage = pPage ? mpDoc->GetSdPage((pPage->GetPageNum() - 1) >> 1, PageKind::Notes) : nullptr;
                if (pNotesPage)
                {
                    SdrObject* pNotes = pNotesPage->GetPresObj(PresObjKind::Notes);
                    if (pNotes)
                    {
                        OUStringBuffer strNotes;
                        OutlinerParaObject* pPara = pNotes->GetOutlinerParaObject();
                        if (pPara)
                        {
                            const EditTextObject& rText = pPara->GetTextObject();
                            for (sal_Int32 nNote = 0; nNote < rText.GetParagraphCount(); nNote++)
                            {
                                strNotes.append(rText.GetText(nNote));
                            }
                            aJsonWriter.put("notes", strNotes.makeStringAndClear());
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
                        pSlide->getPropertyValue("IsBackgroundObjectsVisible") >>= bBackgroundObjectsVisibility;
                        aJsonWriter.put("masterPageObjectsVisibility", bBackgroundObjectsVisibility);
                    }
                }

                bool bBackgroundVisibility = true; // default visible
                pSlide->getPropertyValue("IsBackgroundVisible")  >>= bBackgroundVisibility;
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

                uno::Reference<drawing::XShapes> const xShapes(cppu::getXWeak(pSlide), uno::UNO_QUERY_THROW);
                if (xShapes.is())
                {
                    auto aVideoList = aJsonWriter.startArray("interactions");
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
                    }
                }

                sal_Int32 nTransitionType = 0;
                pSlide->getPropertyValue("TransitionType") >>= nTransitionType;

                if (nTransitionType != 0)
                {
                    auto iterator = constTransitionTypeToString.find(nTransitionType);

                    if (iterator != constTransitionTypeToString.end())
                    {
                        aJsonWriter.put("transitionType", iterator->second);

                        sal_Int32 nTransitionSubtype = 0;
                        pSlide->getPropertyValue("TransitionSubtype") >>= nTransitionSubtype;

                        auto iteratorSubType = constTransitionSubTypeToString.find(nTransitionSubtype);
                        if (iteratorSubType != constTransitionSubTypeToString.end())
                        {
                            aJsonWriter.put("transitionSubtype", iteratorSubType->second);
                        }
                        else
                        {
                            SAL_WARN("sd", "Transition sub-type unknown: " << nTransitionSubtype);
                        }

                        bool nTransitionDirection = false;
                        pSlide->getPropertyValue("TransitionDirection") >>= nTransitionDirection;
                        aJsonWriter.put("transitionDirection", nTransitionDirection);

                        // fade color
                        if ((nTransitionType == TransitionType::FADE)
                                && ((nTransitionSubtype == TransitionSubType::FADETOCOLOR)
                                    || (nTransitionSubtype == TransitionSubType::FADEFROMCOLOR)
                                    || (nTransitionSubtype == TransitionSubType::FADEOVERCOLOR)))
                        {
                            sal_Int32 nFadeColor = 0;
                            pSlide->getPropertyValue("TransitionFadeColor") >>= nFadeColor;
                            OUStringBuffer sTmpBuf;
                            ::sax::Converter::convertColor(sTmpBuf, nFadeColor);
                            aJsonWriter.put("transitionFadeColor", sTmpBuf.makeStringAndClear());
                        }
                    }

                    double nTransitionDuration(0.0);
                    if( pSlide->getPropertySetInfo()->hasPropertyByName( "TransitionDuration" ) &&
                        (pSlide->getPropertyValue( "TransitionDuration" ) >>= nTransitionDuration ) && nTransitionDuration != 0.0 )
                    {
                        // convert transitionDuration time to ms
                        aJsonWriter.put("transitionDuration", nTransitionDuration * 1000);
                    }
                }

                sal_Int32 nChange(0);
                if( pSlide->getPropertySetInfo()->hasPropertyByName( "Change" ) &&
                        (pSlide->getPropertyValue( "Change" ) >>= nChange ) && nChange == 1 )
                {
                    double fSlideDuration(0);
                    if( pSlide->getPropertySetInfo()->hasPropertyByName( "HighResDuration" ) &&
                            (pSlide->getPropertyValue( "HighResDuration" ) >>= fSlideDuration) )
                    {
                        // convert slide duration time to ms
                        aJsonWriter.put("nextSlideDuration", fSlideDuration * 1000);
                    }
                }


                AnimationsExporter aAnimationExporter(aJsonWriter, pSlide);
                if (aAnimationExporter.hasEffects())
                {
                    auto aAnimationsNode = aJsonWriter.startNode("animations");
                    aAnimationExporter.exportAnimations();
                }
            }
        }
    }
    catch (uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("sd", "SdXImpressDocument::getSlideShowInfo ... maybe some property can't be retrieved");
    }
    return aJsonWriter.finishAndGetAsOString();
}

namespace
{
// use VCL slideshow renderer or not - leave the old one in for now, so it is possible to compare output
constexpr const bool bVCLSlideShowRenderer = true;
}

bool SdXImpressDocument::createSlideRenderer(
    sal_Int32 nSlideNumber, sal_Int32& nViewWidth, sal_Int32& nViewHeight,
    bool bRenderBackground, bool bRenderMasterPage)
{
    if (bVCLSlideShowRenderer)
    {
        SdPage* pPage = mpDoc->GetSdPage(sal_uInt16(nSlideNumber), PageKind::Standard);
        if (!pPage)
            return false;

        mpSlideshowLayerRenderer.reset(new SlideshowLayerRenderer(*pPage));
        Size aDesiredSize(nViewWidth, nViewHeight);
        Size aCalculatedSize = mpSlideshowLayerRenderer->calculateAndSetSizePixel(aDesiredSize);
        nViewWidth = aCalculatedSize.Width();
        nViewHeight = aCalculatedSize.Height();
        return true;
    }
    else
    {
        DrawViewShell* pViewSh = GetViewShell();
        if (!pViewSh)
            return false;

        uno::Reference<presentation::XSlideShow> xSlideShow = pViewSh->getXSlideShowInstance();
        if (!xSlideShow.is())
            return false;

        bool bSuccess = false;
        try
        {
            rtl::Reference<SdXImpressDocument> xDrawPages(mpDoc->getUnoModel());
            uno::Reference<container::XIndexAccess> xSlides(xDrawPages->getDrawPages(), uno::UNO_QUERY_THROW);
            uno::Reference<drawing::XDrawPage> xSlide(xSlides->getByIndex(nSlideNumber), uno::UNO_QUERY_THROW);
            uno::Reference<animations::XAnimationNodeSupplier> xAnimNodeSupplier(xSlide, uno::UNO_QUERY_THROW);
            uno::Reference<animations::XAnimationNode> xAnimNode = xAnimNodeSupplier->getAnimationNode();

            bSuccess = xSlideShow->createLOKSlideRenderer(nViewWidth, nViewHeight,
                    bRenderMasterPage, bRenderBackground,
                    xSlide, xDrawPages, xAnimNode);
        }
        catch (uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION( "sd", "SdXImpressDocument::createLOKSlideRenderer: failed" );
        }
        return bSuccess;
    }
}

void SdXImpressDocument::postSlideshowCleanup()
{
    if (bVCLSlideShowRenderer)
    {
        mpSlideshowLayerRenderer.reset();
    }
    else
    {
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return;

    pViewSh->destroyXSlideShowInstance();
}
}

bool SdXImpressDocument::renderNextSlideLayer(unsigned char* pBuffer, bool& bIsBitmapLayer, OUString& rJsonMsg)
{
    if (bVCLSlideShowRenderer)
    {
        bool bDone = true;

        if (!mpSlideshowLayerRenderer)
            return bDone;

        OString sMsg;
        bool bOK = mpSlideshowLayerRenderer->render(pBuffer, sMsg);

        if (bOK)
        {
            rJsonMsg = OUString::fromUtf8(sMsg);
            bIsBitmapLayer = true;
            bDone = false;
        }

        return bDone;
    }
    else
    {
    DrawViewShell* pViewSh = GetViewShell();
    if (!pViewSh)
        return true;

    uno::Reference<presentation::XSlideShow> xSlideShow = pViewSh->getXSlideShowInstance();
    if (!xSlideShow.is())
        return true;

    auto nBufferPointer = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(pBuffer));
    sal_Bool bBitmapLayer = false;
    bool bDone = xSlideShow->renderNextLOKSlideLayer(nBufferPointer, bBitmapLayer, rJsonMsg);
    bIsBitmapLayer = bBitmapLayer;

    return bDone;
}
}

SdrModel& SdXImpressDocument::getSdrModelFromUnoModel() const
{
    OSL_ENSURE(GetDoc(), "No SdrModel in draw/Impress, should not happen");
    return *GetDoc(); // TTTT should be reference
}

void SAL_CALL SdXImpressDocument::dispose()
{
    if( mbDisposed )
        return;

    ::SolarMutexGuard aGuard;

    if( mpDoc )
    {
        EndListening( *mpDoc );
        mpDoc = nullptr;
    }

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
}


SdDrawPagesAccess::SdDrawPagesAccess( SdXImpressDocument& rMyModel )  noexcept
:   mpModel( &rMyModel)
{
}

SdDrawPagesAccess::~SdDrawPagesAccess() noexcept
{
}

// XIndexAccess
sal_Int32 SAL_CALL SdDrawPagesAccess::getCount()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
}

uno::Any SAL_CALL SdDrawPagesAccess::getByIndex( sal_Int32 Index )
{
    uno::Reference< drawing::XDrawPage > xDrawPage( getDrawPageByIndex(Index) );
    return uno::Any(xDrawPage);
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
uno::Any SAL_CALL SdDrawPagesAccess::getByName( const OUString& aName )
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
                uno::Any aAny;
                uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
                aAny <<= xDrawPage;
                return aAny;
            }
        }
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SdDrawPagesAccess::getElementNames()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    const sal_uInt16 nCount = mpModel->mpDoc->GetSdPageCount( PageKind::Standard );
    uno::Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();

    sal_uInt16 nPage;
    for( nPage = 0; nPage < nCount; nPage++ )
    {
        SdPage* pPage = mpModel->mpDoc->GetSdPage( nPage, PageKind::Standard );
        *pNames++ = SdDrawPage::getPageApiName( pPage );
    }

    return aNames;
}

sal_Bool SAL_CALL SdDrawPagesAccess::hasByName( const OUString& aName )
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
uno::Type SAL_CALL SdDrawPagesAccess::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

sal_Bool SAL_CALL SdDrawPagesAccess::hasElements()
{
    return getCount() > 0;
}

// XDrawPages

/**
 * Creates a new page with model at the specified position.
 * @returns corresponding SdDrawPage
 */
uno::Reference< drawing::XDrawPage > SAL_CALL SdDrawPagesAccess::insertNewByIndex( sal_Int32 nIndex )
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
            uno::Reference< drawing::XDrawPage > xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
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
void SAL_CALL SdDrawPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
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
                    rDoc.AddUndo(rDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pNotesPage));
                    rDoc.AddUndo(rDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
                }

                rDoc.RemovePage( nPage ); // the page
                rDoc.RemovePage( nPage ); // the notes page

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

OUString SAL_CALL SdDrawPagesAccess::getImplementationName(  )
{
    return u"SdDrawPagesAccess"_ustr;
}

sal_Bool SAL_CALL SdDrawPagesAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SdDrawPagesAccess::getSupportedServiceNames(  )
{
    return { u"com.sun.star.drawing.DrawPages"_ustr };
}

// XComponent
void SAL_CALL SdDrawPagesAccess::dispose(  )
{
    mpModel = nullptr;
}

void SAL_CALL SdDrawPagesAccess::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SAL_CALL SdDrawPagesAccess::removeEventListener( const uno::Reference< lang::XEventListener >&  )
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
void SAL_CALL SdMasterPagesAccess::dispose(  )
{
    mpModel = nullptr;
}

void SAL_CALL SdMasterPagesAccess::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SAL_CALL SdMasterPagesAccess::removeEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

// XIndexAccess
sal_Int32 SAL_CALL SdMasterPagesAccess::getCount()
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
uno::Any SAL_CALL SdMasterPagesAccess::getByIndex( sal_Int32 Index )
{
    ::SolarMutexGuard aGuard;
    comphelper::ProfileZone aZone("SdMasterPagesAccess::getByIndex");

    if( nullptr == mpModel )
        throw lang::DisposedException();

    uno::Any aAny;

    if( (Index < 0) || (Index >= mpModel->mpDoc->GetMasterSdPageCount( PageKind::Standard ) ) )
        throw lang::IndexOutOfBoundsException();

    SdPage* pPage = mpModel->mpDoc->GetMasterSdPage( static_cast<sal_uInt16>(Index), PageKind::Standard );
    if( pPage )
    {
        uno::Reference< drawing::XDrawPage >  xDrawPage( pPage->getUnoPage(), uno::UNO_QUERY );
        aAny <<= xDrawPage;
    }

    return aAny;
}

// XElementAccess
uno::Type SAL_CALL SdMasterPagesAccess::getElementType()
{
    return cppu::UnoType<drawing::XDrawPage>::get();
}

sal_Bool SAL_CALL SdMasterPagesAccess::hasElements()
{
    return getCount() > 0;
}

// XDrawPages
uno::Reference< drawing::XDrawPage > SAL_CALL SdMasterPagesAccess::insertNewByIndex( sal_Int32 nInsertPos )
{
    return insertNewImpl(nInsertPos, std::nullopt);
}

// XDrawPages2
uno::Reference< drawing::XDrawPage > SAL_CALL SdMasterPagesAccess::insertNamedNewByIndex( sal_Int32 nInsertPos, const OUString& sName )
{
    return insertNewImpl(nInsertPos, sName);
}

uno::Reference< drawing::XDrawPage > SdMasterPagesAccess::insertNewImpl( sal_Int32 nInsertPos, std::optional<OUString> oPageName )
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    uno::Reference< drawing::XDrawPage > xDrawPage;

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

        xDrawPage.set( pMPage->getUnoPage(), uno::UNO_QUERY );

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
void SAL_CALL SdMasterPagesAccess::remove( const uno::Reference< drawing::XDrawPage >& xPage )
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
        rDoc.AddUndo(rDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pNotesPage));
        rDoc.AddUndo(rDoc.GetSdrUndoFactory().CreateUndoDeletePage(*pPage));
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

OUString SAL_CALL SdMasterPagesAccess::getImplementationName(  )
{
    return u"SdMasterPagesAccess"_ustr;
}

sal_Bool SAL_CALL SdMasterPagesAccess::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SdMasterPagesAccess::getSupportedServiceNames(  )
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
void SAL_CALL SdDocLinkTargets::dispose(  )
{
    mpModel = nullptr;
}

void SAL_CALL SdDocLinkTargets::addEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

void SAL_CALL SdDocLinkTargets::removeEventListener( const uno::Reference< lang::XEventListener >&  )
{
    OSL_FAIL( "not implemented!" );
}

// XNameAccess
uno::Any SAL_CALL SdDocLinkTargets::getByName( const OUString& aName )
{
    if (mpModel)
    {
        for (sal_uInt16 i=0; i < SdLinkTargetType::Count; i++)
            if ( aNames[i] == aName )
                return uno::Any(uno::Reference< beans::XPropertySet >(new SdDocLinkTargetType( mpModel, i )));
    }

    throw container::NoSuchElementException();
}

uno::Sequence< OUString > SAL_CALL SdDocLinkTargets::getElementNames()
{
    uno::Sequence<OUString> aRet(SdLinkTargetType::Count);
    OUString* pArray = aRet.getArray();
    for (sal_uInt16 i=0; i < SdLinkTargetType::Count; i++)
        pArray[i] = aNames[i];
    return aRet;
}

sal_Bool SAL_CALL SdDocLinkTargets::hasByName( const OUString& aName )
{
    for (const auto & i : aNames)
        if ( i == aName )
            return true;
    return false;
}

// container::XElementAccess
uno::Type SAL_CALL SdDocLinkTargets::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SAL_CALL SdDocLinkTargets::hasElements()
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
OUString SAL_CALL SdDocLinkTargets::getImplementationName()
{
    return u"SdDocLinkTargets"_ustr;
}

sal_Bool SAL_CALL SdDocLinkTargets::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdDocLinkTargets::getSupportedServiceNames()
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

uno::Reference< beans::XPropertySetInfo > SAL_CALL SdDocLinkTargetType::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > aRef;//(new SfxItemPropertySetInfo( lcl_GetLinkTargetMap() ));
    return aRef;
}

void SAL_CALL SdDocLinkTargetType::setPropertyValue(const OUString& /* aPropertyName */,
            const uno::Any& /* aValue */)
{
    //  everything is read-only
}

uno::Any SAL_CALL SdDocLinkTargetType::getPropertyValue(const OUString& PropertyName)
{
    uno::Any aRet;
    if ( PropertyName == "LinkDisplayName" )
        aRet <<= maName;

    return aRet;
}

void SAL_CALL SdDocLinkTargetType::addPropertyChangeListener( const OUString&,
                            const uno::Reference<beans::XPropertyChangeListener>&)
{ OSL_FAIL("not implemented"); }

void SAL_CALL SdDocLinkTargetType::removePropertyChangeListener( const OUString&,
                        const uno::Reference<beans::XPropertyChangeListener>&)
{ OSL_FAIL("not implemented"); }

void SAL_CALL SdDocLinkTargetType::addVetoableChangeListener( const OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
{ OSL_FAIL("not implemented"); }

void SAL_CALL SdDocLinkTargetType::removeVetoableChangeListener( const OUString&,
                        const uno::Reference<beans::XVetoableChangeListener>&)
{ OSL_FAIL("not implemented"); }

// document::XLinkTargetSupplier

uno::Reference< container::XNameAccess > SAL_CALL SdDocLinkTargetType::getLinks()
{
    return new SdDocLinkTarget( mpModel, mnType );
}

// XServiceInfo
OUString SAL_CALL SdDocLinkTargetType::getImplementationName()
{
    return u"SdDocLinkTargetType"_ustr;
}

sal_Bool SAL_CALL SdDocLinkTargetType::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdDocLinkTargetType::getSupportedServiceNames()
{
    return { u"com.sun.star.document.LinkTargetSupplier"_ustr };
}

SdDocLinkTarget::SdDocLinkTarget( SdXImpressDocument* pModel, sal_uInt16 nT )
    : mpModel(pModel)
    , mnType(nT)
{
}

// container::XNameAccess

uno::Any SAL_CALL SdDocLinkTarget::getByName(const OUString& aName)
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    SdPage* pPage = FindPage( aName );

    if( pPage == nullptr )
        throw container::NoSuchElementException();

    uno::Any aAny;

    uno::Reference< beans::XPropertySet > xProps( pPage->getUnoPage(), uno::UNO_QUERY );
    if( xProps.is() )
        aAny <<= xProps;

    return aAny;
}

uno::Sequence<OUString> SAL_CALL SdDocLinkTarget::getElementNames()
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

        uno::Sequence< OUString > aSeq( mnType == SdLinkTargetType::Page ? nMaxPages : nMaxMasterPages );
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

        uno::Sequence< OUString > aSeq( mnType == SdLinkTargetType::MasterPage ? nMaxMasterPages : nMaxPages );
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

sal_Bool SAL_CALL SdDocLinkTarget::hasByName(const OUString& aName)
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return FindPage( aName ) != nullptr;
}

// container::XElementAccess

uno::Type SAL_CALL SdDocLinkTarget::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SAL_CALL SdDocLinkTarget::hasElements()
{
    ::SolarMutexGuard aGuard;

    if( nullptr == mpModel )
        throw lang::DisposedException();

    return mpModel->GetDoc() != nullptr;
}

// XServiceInfo
OUString SAL_CALL SdDocLinkTarget::getImplementationName()
{
    return u"SdDocLinkTarget"_ustr;
}

sal_Bool SAL_CALL SdDocLinkTarget::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

uno::Sequence< OUString > SAL_CALL SdDocLinkTarget::getSupportedServiceNames()
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

void NotifyDocumentEvent(SdDrawDocument const & rDocument, const OUString& rEventName)
{
    rtl::Reference<SdXImpressDocument> xModel(SdXImpressDocument::GetModel(rDocument));

    if (xModel.is())
    {
        uno::Reference<uno::XInterface> xSource(static_cast<uno::XWeak*>(xModel.get()));
        NotifyDocumentEvent(rDocument, rEventName, xSource);
    }
}

void NotifyDocumentEvent(SdDrawDocument const & rDocument, const OUString& rEventName, const uno::Reference<uno::XInterface>& xSource)
{
    rtl::Reference<SdXImpressDocument> xModel(SdXImpressDocument::GetModel(rDocument));

    if (xModel.is())
    {
        css::document::EventObject aEvent(xSource, rEventName);
        xModel->notifyEvent(aEvent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
