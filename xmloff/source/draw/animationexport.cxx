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


#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationEndSync.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <sax/tools/converter.hxx>

#include <tools/debug.hxx>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include "sdxmlexp_impl.hxx"
#include "sdpropls.hxx"
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/shapeexport.hxx>

#include "animations.hxx"
#include <xmloff/animationexport.hxx>


using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;

namespace xmloff
{

const SvXMLEnumMapEntry* getAnimationsEnumMap( sal_uInt16 nMap )
{
    switch( nMap )
    {
    case Animations_EnumMap_Fill:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_Fill[] =
            {
                { XML_DEFAULT,      AnimationFill::DEFAULT },
                { XML_REMOVE,       AnimationFill::REMOVE },
                { XML_FREEZE,       AnimationFill::FREEZE },
                { XML_HOLD,         AnimationFill::HOLD },
                { XML_TRANSITION,   AnimationFill::TRANSITION },
                { XML_AUTO,         AnimationFill::AUTO },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_Fill;
        }
    case Animations_EnumMap_FillDefault:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_Fill[] =
            {
                { XML_INHERIT,      AnimationFill::INHERIT },
                { XML_REMOVE,       AnimationFill::REMOVE },
                { XML_FREEZE,       AnimationFill::FREEZE },
                { XML_HOLD,         AnimationFill::HOLD },
                { XML_TRANSITION,   AnimationFill::TRANSITION },
                { XML_AUTO,         AnimationFill::AUTO },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_Fill;
        }
    case Animations_EnumMap_Restart:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_Restart[] =
            {
                { XML_DEFAULT,      AnimationRestart::DEFAULT },
                { XML_ALWAYS,       AnimationRestart::ALWAYS },
                { XML_WHENNOTACTIVE,AnimationRestart::WHEN_NOT_ACTIVE },
                { XML_NEVER,        AnimationRestart::NEVER },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_Restart;
        }
    case Animations_EnumMap_RestartDefault:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_RestartDefault[] =
            {
                { XML_INHERIT,      AnimationRestart::INHERIT },
                { XML_ALWAYS,       AnimationRestart::ALWAYS },
                { XML_WHENNOTACTIVE,AnimationRestart::WHEN_NOT_ACTIVE },
                { XML_NEVER,        AnimationRestart::NEVER },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_RestartDefault;
        }
    case Animations_EnumMap_Endsync:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_Endsync[] =
            {
                { XML_FIRST,        AnimationEndSync::FIRST },
                { XML_LAST,         AnimationEndSync::LAST },
                { XML_ALL,          AnimationEndSync::ALL },
                { XML_MEDIA,        AnimationEndSync::MEDIA },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_Endsync;
        }
    case Animations_EnumMap_CalcMode:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_CalcMode[] =
            {
                { XML_DISCRETE,     AnimationCalcMode::DISCRETE },
                { XML_LINEAR,       AnimationCalcMode::LINEAR },
                { XML_PACED,        AnimationCalcMode::PACED },
                { XML_SPLINE,       AnimationCalcMode::SPLINE },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_CalcMode;
        }
    case Animations_EnumMap_AdditiveMode:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_AdditiveMode[] =
            {
                { XML_BASE,         AnimationAdditiveMode::BASE },
                { XML_SUM,          AnimationAdditiveMode::SUM },
                { XML_REPLACE,      AnimationAdditiveMode::REPLACE },
                { XML_MULTIPLY,     AnimationAdditiveMode::MULTIPLY },
                { XML_NONE,         AnimationAdditiveMode::NONE },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_AdditiveMode;
        }
    case Animations_EnumMap_TransformType:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_TransformType[] =
            {
                { XML_TRANSLATE,    AnimationTransformType::TRANSLATE },
                { XML_SCALE,        AnimationTransformType::SCALE },
                { XML_ROTATE,       AnimationTransformType::ROTATE },
                { XML_SKEWX,        AnimationTransformType::SKEWX },
                { XML_SKEWY,        AnimationTransformType::SKEWY },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_TransformType;
        }
    case Animations_EnumMap_TransitionType:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_TransitionType[] =
            {
                { XML_BARWIPE,          TransitionType::BARWIPE },
                { XML_BOXWIPE,          TransitionType::BOXWIPE },
                { XML_FOURBOXWIPE,      TransitionType::FOURBOXWIPE },
                { XML_BARNDOORWIPE,     TransitionType::BARNDOORWIPE },
                { XML_DIAGONALWIPE,     TransitionType::DIAGONALWIPE },
                { XML_BOWTIEWIPE,       TransitionType::BOWTIEWIPE },
                { XML_MISCDIAGONALWIPE, TransitionType::MISCDIAGONALWIPE },
                { XML_VEEWIPE,          TransitionType::VEEWIPE },
                { XML_BARNVEEWIPE,      TransitionType::BARNVEEWIPE },
                { XML_ZIGZAGWIPE,       TransitionType::ZIGZAGWIPE },
                { XML_BARNZIGZAGWIPE,   TransitionType::BARNZIGZAGWIPE },
                { XML_IRISWIPE,         TransitionType::IRISWIPE },
                { XML_TRIANGLEWIPE,     TransitionType::TRIANGLEWIPE },
                { XML_ARROWHEADWIPE,    TransitionType::ARROWHEADWIPE },
                { XML_PENTAGONWIPE,     TransitionType::PENTAGONWIPE },
                { XML_HEXAGONWIPE,      TransitionType::HEXAGONWIPE },
                { XML_ELLIPSEWIPE,      TransitionType::ELLIPSEWIPE },
                { XML_EYEWIPE,          TransitionType::EYEWIPE },
                { XML_ROUNDRECTWIPE,    TransitionType::ROUNDRECTWIPE },
                { XML_STARWIPE,         TransitionType::STARWIPE },
                { XML_MISCSHAPEWIPE,    TransitionType::MISCSHAPEWIPE },
                { XML_CLOCKWIPE,        TransitionType::CLOCKWIPE },
                { XML_PINWHEELWIPE,     TransitionType::PINWHEELWIPE },
                { XML_SINGLESWEEPWIPE,  TransitionType::SINGLESWEEPWIPE },
                { XML_FANWIPE,          TransitionType::FANWIPE },
                { XML_DOUBLEFANWIPE,    TransitionType::DOUBLEFANWIPE },
                { XML_DOUBLESWEEPWIPE,  TransitionType::DOUBLESWEEPWIPE },
                { XML_SALOONDOORWIPE,   TransitionType::SALOONDOORWIPE },
                { XML_WINDSHIELDWIPE,   TransitionType::WINDSHIELDWIPE },
                { XML_SNAKEWIPE,        TransitionType::SNAKEWIPE },
                { XML_SPIRALWIPE,       TransitionType::SPIRALWIPE },
                { XML_PARALLELSNAKESWIPE,TransitionType::PARALLELSNAKESWIPE },
                { XML_BOXSNAKESWIPE,    TransitionType::BOXSNAKESWIPE },
                { XML_WATERFALLWIPE,    TransitionType::WATERFALLWIPE },
                { XML_PUSHWIPE,         TransitionType::PUSHWIPE },
                { XML_SLIDEWIPE,        TransitionType::SLIDEWIPE },
                { XML_FADE,             TransitionType::FADE },
                { XML_RANDOMBARWIPE,    TransitionType::RANDOMBARWIPE },
                { XML_CHECKERBOARDWIPE, TransitionType::CHECKERBOARDWIPE },
                { XML_DISSOLVE,         TransitionType::DISSOLVE },
                { XML_BLINDSWIPE,       TransitionType::BLINDSWIPE },
                { XML_RANDOM,           TransitionType::RANDOM },
                { XML_ZOOM,             TransitionType::ZOOM },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_TransitionType;
        }
    case Animations_EnumMap_TransitionSubType:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_TransitionSubType[] =
            {
                { XML_DEFAULT,              TransitionSubType::DEFAULT },
                { XML_LEFTTORIGHT,          TransitionSubType::LEFTTORIGHT },
                { XML_TOPTOBOTTOM,          TransitionSubType::TOPTOBOTTOM },
                { XML_TOPLEFT,              TransitionSubType::TOPLEFT },
                { XML_TOPRIGHT,             TransitionSubType::TOPRIGHT },
                { XML_BOTTOMRIGHT,          TransitionSubType::BOTTOMRIGHT },
                { XML_BOTTOMLEFT,           TransitionSubType::BOTTOMLEFT },
                { XML_TOPCENTER,            TransitionSubType::TOPCENTER },
                { XML_RIGHTCENTER,          TransitionSubType::RIGHTCENTER },
                { XML_BOTTOMCENTER,         TransitionSubType::BOTTOMCENTER },
                { XML_LEFTCENTER,           TransitionSubType::LEFTCENTER },
                { XML_CORNERSIN,            TransitionSubType::CORNERSIN },
                { XML_CORNERSOUT,           TransitionSubType::CORNERSOUT },
                { XML_VERTICAL,             TransitionSubType::VERTICAL },
                { XML_HORIZONTAL,           TransitionSubType::HORIZONTAL },
                { XML_DIAGONALBOTTOMLEFT,   TransitionSubType::DIAGONALBOTTOMLEFT },
                { XML_DIAGONALTOPLEFT,      TransitionSubType::DIAGONALTOPLEFT },
                { XML_DOUBLEBARNDOOR,       TransitionSubType::DOUBLEBARNDOOR },
                { XML_DOUBLEDIAMOND,        TransitionSubType::DOUBLEDIAMOND },
                { XML_DOWN,                 TransitionSubType::DOWN },
                { XML_LEFT,                 TransitionSubType::LEFT },
                { XML_UP,                   TransitionSubType::UP },
                { XML_RIGHT,                TransitionSubType::RIGHT },
                { XML_RECTANGLE,            TransitionSubType::RECTANGLE },
                { XML_DIAMOND,              TransitionSubType::DIAMOND },
                { XML_CIRCLE,               TransitionSubType::CIRCLE },
                { XML_FOURPOINT,            TransitionSubType::FOURPOINT },
                { XML_FIVEPOINT,            TransitionSubType::FIVEPOINT },
                { XML_SIXPOINT,             TransitionSubType::SIXPOINT },
                { XML_HEART,                TransitionSubType::HEART },
                { XML_KEYHOLE,              TransitionSubType::KEYHOLE },
                { XML_CLOCKWISETWELVE,      TransitionSubType::CLOCKWISETWELVE },
                { XML_CLOCKWISETHREE,       TransitionSubType::CLOCKWISETHREE },
                { XML_CLOCKWISESIX,         TransitionSubType::CLOCKWISESIX },
                { XML_CLOCKWISENINE,        TransitionSubType::CLOCKWISENINE },
                { XML_TWOBLADEVERTICAL,     TransitionSubType::TWOBLADEVERTICAL },
                { XML_TWOBLADEHORIZONTAL,   TransitionSubType::TWOBLADEHORIZONTAL },
                { XML_FOURBLADE,            TransitionSubType::FOURBLADE },
                { XML_CLOCKWISETOP,         TransitionSubType::CLOCKWISETOP },
                { XML_CLOCKWISERIGHT,       TransitionSubType::CLOCKWISERIGHT },
                { XML_CLOCKWISEBOTTOM,      TransitionSubType::CLOCKWISEBOTTOM },
                { XML_CLOCKWISELEFT,        TransitionSubType::CLOCKWISELEFT },
                { XML_CLOCKWISETOPLEFT,     TransitionSubType::CLOCKWISETOPLEFT },
                { XML_COUNTERCLOCKWISEBOTTOMLEFT,TransitionSubType::COUNTERCLOCKWISEBOTTOMLEFT },
                { XML_CLOCKWISEBOTTOMRIGHT, TransitionSubType::CLOCKWISEBOTTOMRIGHT },
                { XML_COUNTERCLOCKWISETOPRIGHT,TransitionSubType::COUNTERCLOCKWISETOPRIGHT },
                { XML_CENTERTOP,            TransitionSubType::CENTERTOP },
                { XML_CENTERRIGHT,          TransitionSubType::CENTERRIGHT },
                { XML_TOP,                  TransitionSubType::TOP },
                { XML_BOTTOM,               TransitionSubType::BOTTOM },
                { XML_FANOUTVERTICAL,       TransitionSubType::FANOUTVERTICAL },
                { XML_FANOUTHORIZONTAL,     TransitionSubType::FANOUTHORIZONTAL },
                { XML_FANINVERTICAL,        TransitionSubType::FANINVERTICAL },
                { XML_FANINHORIZONTAL,      TransitionSubType::FANINHORIZONTAL },
                { XML_PARALLELVERTICAL,     TransitionSubType::PARALLELVERTICAL },
                { XML_PARALLELDIAGONAL,     TransitionSubType::PARALLELDIAGONAL },
                { XML_OPPOSITEVERTICAL,     TransitionSubType::OPPOSITEVERTICAL },
                { XML_OPPOSITEHORIZONTAL,   TransitionSubType::OPPOSITEHORIZONTAL },
                { XML_PARALLELDIAGONALTOPLEFT,TransitionSubType::PARALLELDIAGONALTOPLEFT },
                { XML_PARALLELDIAGONALBOTTOMLEFT,TransitionSubType::PARALLELDIAGONALBOTTOMLEFT },
                { XML_TOPLEFTHORIZONTAL,    TransitionSubType::TOPLEFTHORIZONTAL },
                { XML_TOPLEFTDIAGONAL,      TransitionSubType::TOPLEFTDIAGONAL },
                { XML_TOPRIGHTDIAGONAL,     TransitionSubType::TOPRIGHTDIAGONAL },
                { XML_BOTTOMRIGHTDIAGONAL,  TransitionSubType::BOTTOMRIGHTDIAGONAL },
                { XML_BOTTOMLEFTDIAGONAL,   TransitionSubType::BOTTOMLEFTDIAGONAL },
                { XML_TOPLEFTCLOCKWISE,     TransitionSubType::TOPLEFTCLOCKWISE },
                { XML_TOPRIGHTCLOCKWISE,    TransitionSubType::TOPRIGHTCLOCKWISE },
                { XML_BOTTOMRIGHTCLOCKWISE, TransitionSubType::BOTTOMRIGHTCLOCKWISE },
                { XML_BOTTOMLEFTCLOCKWISE,  TransitionSubType::BOTTOMLEFTCLOCKWISE },
                { XML_TOPLEFTCOUNTERCLOCKWISE,TransitionSubType::TOPLEFTCOUNTERCLOCKWISE },
                { XML_TOPRIGHTCOUNTERCLOCKWISE,TransitionSubType::TOPRIGHTCOUNTERCLOCKWISE },
                { XML_BOTTOMRIGHTCOUNTERCLOCKWISE,TransitionSubType::BOTTOMRIGHTCOUNTERCLOCKWISE },
                { XML_BOTTOMLEFTCOUNTERCLOCKWISE,TransitionSubType::BOTTOMLEFTCOUNTERCLOCKWISE },
                { XML_VERTICALTOPSAME,      TransitionSubType::VERTICALTOPSAME },
                { XML_VERTICALBOTTOMSAME,   TransitionSubType::VERTICALBOTTOMSAME },
                { XML_VERTICALTOPLEFTOPPOSITE,TransitionSubType::VERTICALTOPLEFTOPPOSITE },
                { XML_VERTICALBOTTOMLEFTOPPOSITE,TransitionSubType::VERTICALBOTTOMLEFTOPPOSITE },
                { XML_HORIZONTALLEFTSAME,   TransitionSubType::HORIZONTALLEFTSAME },
                { XML_HORIZONTALRIGHTSAME,  TransitionSubType::HORIZONTALRIGHTSAME },
                { XML_HORIZONTALTOPLEFTOPPOSITE,TransitionSubType::HORIZONTALTOPLEFTOPPOSITE },
                { XML_HORIZONTALTOPRIGHTOPPOSITE,TransitionSubType::HORIZONTALTOPRIGHTOPPOSITE },
                { XML_DIAGONALBOTTOMLEFTOPPOSITE,TransitionSubType::DIAGONALBOTTOMLEFTOPPOSITE },
                { XML_DIAGONALTOPLEFTOPPOSITE,TransitionSubType::DIAGONALTOPLEFTOPPOSITE },
                { XML_TWOBOXTOP,            TransitionSubType::TWOBOXTOP },
                { XML_TWOBOXBOTTOM,         TransitionSubType::TWOBOXBOTTOM },
                { XML_TWOBOXLEFT,           TransitionSubType::TWOBOXLEFT },
                { XML_TWOBOXRIGHT,          TransitionSubType::TWOBOXRIGHT },
                { XML_FOURBOXVERTICAL,      TransitionSubType::FOURBOXVERTICAL },
                { XML_FOURBOXHORIZONTAL,    TransitionSubType::FOURBOXHORIZONTAL },
                { XML_VERTICALLEFT,         TransitionSubType::VERTICALLEFT },
                { XML_VERTICALRIGHT,        TransitionSubType::VERTICALRIGHT },
                { XML_HORIZONTALLEFT,       TransitionSubType::HORIZONTALLEFT },
                { XML_HORIZONTALRIGHT,      TransitionSubType::HORIZONTALRIGHT },
                { XML_FROMLEFT,             TransitionSubType::FROMLEFT },
                { XML_FROMTOP,              TransitionSubType::FROMTOP },
                { XML_FROMRIGHT,            TransitionSubType::FROMRIGHT },
                { XML_FROMBOTTOM,           TransitionSubType::FROMBOTTOM },
                { XML_CROSSFADE,            TransitionSubType::CROSSFADE },
                { XML_FADETOCOLOR,          TransitionSubType::FADETOCOLOR },
                { XML_FADEFROMCOLOR,        TransitionSubType::FADEFROMCOLOR },
                { XML_FADEOVERCOLOR,        TransitionSubType::FADEOVERCOLOR },
                { XML_THREEBLADE,           TransitionSubType::THREEBLADE },
                { XML_EIGHTBLADE,           TransitionSubType::EIGHTBLADE },
                { XML_ONEBLADE,             TransitionSubType::ONEBLADE },
                { XML_ACROSS,               TransitionSubType::ACROSS },
                { XML_TOPLEFTVERTICAL,      TransitionSubType::TOPLEFTVERTICAL },
                { XML_COMBHORIZONTAL,       TransitionSubType::COMBHORIZONTAL },
                { XML_COMBVERTICAL,         TransitionSubType::COMBVERTICAL },
                { XML_IN,                   TransitionSubType::IN },
                { XML_OUT,                  TransitionSubType::OUT },
                { XML_ROTATEIN,             TransitionSubType::ROTATEIN },
                { XML_ROTATEOUT,            TransitionSubType::ROTATEOUT },
                { XML_FROMTOPLEFT,          TransitionSubType::FROMTOPLEFT },
                { XML_FROMTOPRIGHT,         TransitionSubType::FROMTOPRIGHT },
                { XML_FROMBOTTOMLEFT,       TransitionSubType::FROMBOTTOMLEFT },
                { XML_FROMBOTTOMRIGHT,      TransitionSubType::FROMBOTTOMRIGHT },

                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_TransitionSubType;
        }
    case Animations_EnumMap_EventTrigger:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_EventTrigger[] =
            {
                { XML_ONBEGIN,          EventTrigger::ON_BEGIN },
                { XML_ONEND,            EventTrigger::ON_END },
                { XML_BEGIN,            EventTrigger::BEGIN_EVENT },
                { XML_END,              EventTrigger::END_EVENT },
                { XML_CLICK,            EventTrigger::ON_CLICK },
                { XML_DOUBLECLICK,      EventTrigger::ON_DBL_CLICK },
                { XML_MOUSEOVER,        EventTrigger::ON_MOUSE_ENTER },
                { XML_MOUSEOUT,         EventTrigger::ON_MOUSE_LEAVE },
                { XML_NEXT,             EventTrigger::ON_NEXT },
                { XML_PREVIOUS,         EventTrigger::ON_PREV },
                { XML_STOP_AUDIO,       EventTrigger::ON_STOP_AUDIO },
                { XML_REPEAT,           EventTrigger::REPEAT },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_EventTrigger;
        }

    case Animations_EnumMap_EffectPresetClass:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_EffectPresetClass[] =
            {
                { XML_CUSTOM,       EffectPresetClass::CUSTOM },
                { XML_ENTRANCE,     EffectPresetClass::ENTRANCE },
                { XML_EXIT,         EffectPresetClass::EXIT },
                { XML_EMPHASIS,     EffectPresetClass::EMPHASIS },
                { XML_MOTION_PATH,  EffectPresetClass::MOTIONPATH },
                { XML_OLE_ACTION,   EffectPresetClass::OLEACTION },
                { XML_MEDIA_CALL,   EffectPresetClass::MEDIACALL },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_EffectPresetClass;
        }

    case Animations_EnumMap_EffectNodeType:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_EffectNodeType[] =
            {
                { XML_DEFAULT,                  EffectNodeType::DEFAULT },
                { XML_ON_CLICK,                 EffectNodeType::ON_CLICK },
                { XML_WITH_PREVIOUS,            EffectNodeType::WITH_PREVIOUS },
                { XML_AFTER_PREVIOUS,           EffectNodeType::AFTER_PREVIOUS },
                { XML_MAIN_SEQUENCE,            EffectNodeType::MAIN_SEQUENCE },
                { XML_TIMING_ROOT,              EffectNodeType::TIMING_ROOT },
                { XML_INTERACTIVE_SEQUENCE,     EffectNodeType::INTERACTIVE_SEQUENCE },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_EffectNodeType;
        }
    case Animations_EnumMap_SubItem:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_SubItem[] =
            {
                { XML_WHOLE,                    ShapeAnimationSubType::AS_WHOLE },
                { XML_BACKGROUND,               ShapeAnimationSubType::ONLY_BACKGROUND },
                { XML_TEXT,                     ShapeAnimationSubType::ONLY_TEXT },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_SubItem;
        }
    case Animations_EnumMap_IterateType:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_IterateType[] =
            {
                { XML_BY_PARAGRAPH,             TextAnimationType::BY_PARAGRAPH },
                { XML_BY_WORD,                  TextAnimationType::BY_WORD },
                { XML_BY_LETTER,                TextAnimationType::BY_LETTER },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_IterateType;
        }
    case Animations_EnumMap_Command:
        {
            static const SvXMLEnumMapEntry aAnimations_EnumMap_Command[] =
            {
                { XML_CUSTOM,                   EffectCommands::CUSTOM },
                { XML_VERB,                     EffectCommands::VERB },
                { XML_PLAY,                     EffectCommands::PLAY },
                { XML_TOGGLE_PAUSE,             EffectCommands::TOGGLEPAUSE },
                { XML_STOP,                     EffectCommands::STOP },
                { XML_STOP_AUDIO,               EffectCommands::STOPAUDIO },
                { XML_TOKEN_INVALID, 0 }
            };
            return aAnimations_EnumMap_Command;
        }
    }

    OSL_FAIL( "xmloff::getAnimationsEnumMap(), invalid map!" );
    return NULL;
}

const struct ImplAttributeNameConversion* getAnimationAttributeNamesConversionList()
{
    static const struct ImplAttributeNameConversion gImplConversionList[] =
    {
        { XML_X,                        "X" },
        { XML_Y,                        "Y" },
        { XML_WIDTH,                    "Width" },
        { XML_HEIGHT,                   "Height" },
        { XML_ROTATE,                   "Rotate" },
        { XML_SKEWX,                    "SkewX" },
        { XML_FILL_COLOR,               "FillColor" },
        { XML_FILL,                     "FillStyle" },
        { XML_STROKE_COLOR,             "LineColor" },
        { XML_STROKE,                   "LineStyle" },
        { XML_COLOR,                    "CharColor" },
        { XML_TEXT_ROTATION_ANGLE,      "CharRotation" },
        { XML_FONT_WEIGHT,              "CharWeight" },
        { XML_TEXT_UNDERLINE,           "CharUnderline" },
        { XML_FONT_FAMILY,              "CharFontName" },
        { XML_FONT_SIZE,                "CharHeight" },
        { XML_FONT_STYLE,               "CharPosture" },
        { XML_VISIBILITY,               "Visibility" },
        { XML_OPACITY,                  "Opacity" },
        { XML_DIM,                      "DimColor" },
        { XML_TOKEN_INVALID,            NULL }
    };

    return gImplConversionList;
}


class AnimationsExporterImpl
{
public:
    AnimationsExporterImpl( SvXMLExport& rExport, const Reference< XPropertySet >& xPageProps );
    virtual ~AnimationsExporterImpl();

    void prepareNode( const Reference< XAnimationNode >& xNode );
    void exportNode( const Reference< XAnimationNode >& xNode );

    void exportContainer( const Reference< XTimeContainer >& xNode, sal_Int16 nContainerNodeType );
    void exportAnimate( const Reference< XAnimate >& xNode );
    void exportAudio( const Reference< XAudio >& xAudio );
    void exportCommand( const Reference< XCommand >& xCommand );

    Reference< XInterface > getParagraphTarget( const ParagraphTarget* pTarget ) const;

    void convertPath( OUStringBuffer& sTmp, const Any& rPath ) const;
    void convertValue( XMLTokenEnum eAttributeName, OUStringBuffer& sTmp, const Any& rValue ) const;
    void convertTiming( OUStringBuffer& sTmp, const Any& rTiming ) const;
    void convertSource( OUStringBuffer& sTmp, const Any& rSource ) const;
    void convertTarget( OUStringBuffer& sTmp, const Any& rTarget ) const;

    void prepareValue( const Any& rValue );

    void exportTransitionNode();
    void prepareTransitionNode();

    bool mbHasTransition;
private:
    SvXMLExport& mrExport;
    Reference< XInterface > mxExport;
    Reference< XPropertySet > mxPageProps;
    XMLSdPropHdlFactory* mpSdPropHdlFactory;
};

AnimationsExporterImpl::AnimationsExporterImpl( SvXMLExport& rExport, const Reference< XPropertySet >& xPageProps )
: mbHasTransition(false)
, mrExport( rExport )
, mxPageProps( xPageProps )
{
    try
    {
        mxExport = static_cast< ::com::sun::star::document::XFilter *>(&rExport);
    }
    catch (const RuntimeException&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::AnimationsExporterImpl(), RuntimeException caught!" );
    }

    mpSdPropHdlFactory = new XMLSdPropHdlFactory( mrExport.GetModel(), mrExport );
    if( mpSdPropHdlFactory )
    {
        // set lock to avoid deletion
        mpSdPropHdlFactory->acquire();
    }

}

AnimationsExporterImpl::~AnimationsExporterImpl()
{
    // cleanup factory, decrease refcount. Should lead to destruction.
    if(mpSdPropHdlFactory)
    {
        mpSdPropHdlFactory->release();
        mpSdPropHdlFactory = 0L;
    }
}

void AnimationsExporterImpl::exportTransitionNode()
{
    if( mbHasTransition && mxPageProps.is() )
    {
        sal_Int16 nTransition = 0;
        mxPageProps->getPropertyValue("TransitionType") >>= nTransition;

        Any aSound( mxPageProps->getPropertyValue("Sound") );
        OUString sSoundURL;
        aSound >>= sSoundURL;
        sal_Bool bStopSound = sal_False;
        if( !(aSound >>= bStopSound) )
            bStopSound = sal_False;


        OUStringBuffer sTmp;
        if( (nTransition != 0) || !sSoundURL.isEmpty() || bStopSound )
        {
            Reference< XInterface > xSource( mxPageProps.get() );
            Event aEvent;
            aEvent.Source <<= xSource;
            aEvent.Trigger = EventTrigger::BEGIN_EVENT;
            aEvent.Repeat = 0;

            convertTiming( sTmp, Any( aEvent ) );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_BEGIN, sTmp.makeStringAndClear() );

            SvXMLElementExport aElement( mrExport, XML_NAMESPACE_ANIMATION, XML_PAR, sal_True, sal_True );

            if( nTransition != 0 )
            {
                sal_Int16 nSubtype = 0;
                sal_Bool bDirection = sal_False;
                sal_Int32 nFadeColor = 0;
                double fDuration = 0.0;
                mxPageProps->getPropertyValue("TransitionSubtype") >>= nSubtype;
                mxPageProps->getPropertyValue("TransitionDirection") >>= bDirection;
                mxPageProps->getPropertyValue("TransitionFadeColor") >>= nFadeColor;
                mxPageProps->getPropertyValue("TransitionDuration") >>= fDuration;

                ::sax::Converter::convertDouble( sTmp, fDuration );
                sTmp.append( sal_Unicode('s'));
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_DUR, sTmp.makeStringAndClear() );

                SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTransition, getAnimationsEnumMap(Animations_EnumMap_TransitionType) );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_TYPE, sTmp.makeStringAndClear() );

                if( nSubtype != TransitionSubType::DEFAULT )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nSubtype, getAnimationsEnumMap(Animations_EnumMap_TransitionSubType) );
                    mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_SUBTYPE, sTmp.makeStringAndClear() );
                }

                if( !bDirection )
                    mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_DIRECTION, XML_REVERSE );

                if( (nTransition == TransitionType::FADE) && ((nSubtype == TransitionSubType::FADETOCOLOR) || (nSubtype == TransitionSubType::FADEFROMCOLOR) ))
                {
                    ::sax::Converter::convertColor( sTmp, nFadeColor );
                    mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_FADECOLOR, sTmp.makeStringAndClear() );
                }
                SvXMLElementExport aElement2( mrExport, XML_NAMESPACE_ANIMATION, XML_TRANSITIONFILTER, sal_True, sal_True );
            }

            if( bStopSound )
            {
                mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_COMMAND, XML_STOP_AUDIO );
                SvXMLElementExport aElement2( mrExport, XML_NAMESPACE_ANIMATION, XML_COMMAND, sal_True, sal_True );
            }
            else if( !sSoundURL.isEmpty())
            {
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, mrExport.GetRelativeReference( sSoundURL ) );

                sal_Bool bLoopSound = sal_False;
                mxPageProps->getPropertyValue("LoopSound") >>= bLoopSound;

                if( bLoopSound )
                    mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_REPEATCOUNT, XML_INDEFINITE );
                SvXMLElementExport aElement2( mrExport, XML_NAMESPACE_ANIMATION, XML_AUDIO, sal_True, sal_True );
            }
        }
    }
}

void AnimationsExporterImpl::prepareTransitionNode()
{
    if( mxPageProps.is() ) try
    {
        sal_Int16 nTransition = 0;
        mxPageProps->getPropertyValue("TransitionType") >>= nTransition;

        sal_Bool bStopSound = sal_False;
        OUString sSoundURL;

        if( nTransition == 0 )
        {
            Any aSound( mxPageProps->getPropertyValue("Sound") );
            aSound >>= sSoundURL;

            if( !(aSound >>= bStopSound) )
                bStopSound = sal_False;
        }

        if( (nTransition != 0) || !sSoundURL.isEmpty() || bStopSound )
        {
            mbHasTransition = true;
            Reference< XInterface > xInt( mxPageProps.get() );
            mrExport.getInterfaceToIdentifierMapper().registerReference( xInt );
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::prepareNode(), Exception caught!" );
    }

}

void AnimationsExporterImpl::prepareNode( const Reference< XAnimationNode >& xNode )
{
    try
    {
        prepareValue( xNode->getBegin() );
        prepareValue( xNode->getEnd() );

        sal_Int16 nNodeType = xNode->getType();
        switch( nNodeType )
        {
        case AnimationNodeType::ITERATE:
        {
            Reference< XIterateContainer > xIter( xNode, UNO_QUERY_THROW );
            prepareValue( xIter->getTarget() );
        }
        // its intended that here is no break!
        case AnimationNodeType::PAR:
        case AnimationNodeType::SEQ:
        {
            Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY_THROW );
            Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
            while( xEnumeration->hasMoreElements() )
            {
                Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
                prepareNode( xChildNode );
            }
        }
        break;

        case AnimationNodeType::ANIMATE:
        case AnimationNodeType::SET:
        case AnimationNodeType::ANIMATEMOTION:
        case AnimationNodeType::ANIMATECOLOR:
        case AnimationNodeType::ANIMATETRANSFORM:
        case AnimationNodeType::TRANSITIONFILTER:
        {
            Reference< XAnimate > xAnimate( xNode, UNO_QUERY_THROW );
            prepareValue( xAnimate->getTarget() );
        }
        break;

        case AnimationNodeType::COMMAND:
        {
            Reference< XCommand > xCommand( xNode, UNO_QUERY_THROW );
            prepareValue( xCommand->getTarget() );
        }
        break;

        case AnimationNodeType::AUDIO:
        {
            Reference< XAudio > xAudio( xNode, UNO_QUERY_THROW );
            prepareValue( xAudio->getSource() );
        }
        break;
        }

        Sequence< NamedValue > aUserData( xNode->getUserData() );
        if( aUserData.hasElements() )
        {
            const NamedValue* pValue = aUserData.getConstArray();
            const sal_Int32 nLength = aUserData.getLength();
            sal_Int32 nElement;
            for( nElement = 0; nElement < nLength; nElement++, pValue++ )
            {
                if( IsXMLToken( pValue->Name, XML_MASTER_ELEMENT ) )
                {
                    Reference< XInterface > xMaster;
                    pValue->Value >>= xMaster;
                    if( xMaster.is() )
                        mrExport.getInterfaceToIdentifierMapper().registerReference( xMaster );
                }
            }
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::prepareNode(), RuntimeException caught!" );
    }
}

void AnimationsExporterImpl::exportNode( const Reference< XAnimationNode >& xNode )
{
    try
    {
        OUStringBuffer sTmp;

        const OUString& rExportIdentifier = mrExport.getInterfaceToIdentifierMapper().getIdentifier( xNode );
        if( !rExportIdentifier.isEmpty() )
        {
            mrExport.AddAttributeIdLegacy(
                XML_NAMESPACE_ANIMATION, rExportIdentifier);
        }

        Any aTemp( xNode->getBegin() );
        if( aTemp.hasValue() )
        {
            convertTiming( sTmp, aTemp );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_BEGIN, sTmp.makeStringAndClear() );
        }

        double fTemp = 0;
        sal_Int32 nTemp;

        aTemp = xNode->getDuration();
        if( aTemp.hasValue() )
        {
            if( aTemp >>= fTemp )
            {
                ::sax::Converter::convertDouble( sTmp, fTemp );
                sTmp.append( sal_Unicode('s'));
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_DUR, sTmp.makeStringAndClear() );
            }
            else
            {
                Timing eTiming;
                if( aTemp >>= eTiming )
                    mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_DUR, eTiming == Timing_INDEFINITE ? XML_INDEFINITE : XML_MEDIA );
            }
        }

        aTemp = xNode->getEnd();
        if( aTemp.hasValue() )
        {
            convertTiming( sTmp, aTemp );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_END, sTmp.makeStringAndClear() );
        }

        nTemp = xNode->getFill();
        if( nTemp != AnimationFill::DEFAULT )
        {
            SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_Fill) );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_FILL, sTmp.makeStringAndClear() );
        }

        nTemp = xNode->getFillDefault();
        if( nTemp != AnimationFill::INHERIT )
        {
            SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_FillDefault) );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_FILLDEFAULT, sTmp.makeStringAndClear() );
        }

        nTemp = xNode->getRestart();
        if( nTemp != AnimationRestart::DEFAULT )
        {
            SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_Restart) );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_RESTART, sTmp.makeStringAndClear() );
        }

        nTemp = xNode->getRestartDefault();
        if( nTemp != AnimationRestart::INHERIT )
        {
            SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_RestartDefault) );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_RESTARTDEFAULT, sTmp.makeStringAndClear() );
        }

        fTemp = xNode->getAcceleration();
        if( fTemp != 0.0 )
        {
            ::sax::Converter::convertDouble( sTmp, fTemp );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_ACCELERATE, sTmp.makeStringAndClear() );
        }

        fTemp = xNode->getDecelerate();
        if( fTemp != 0.0 )
        {
            ::sax::Converter::convertDouble( sTmp, fTemp );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_DECELERATE, sTmp.makeStringAndClear() );
        }

        sal_Bool bTemp = xNode->getAutoReverse();
        if( bTemp )
        {
            ::sax::Converter::convertBool( sTmp, bTemp );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_AUTOREVERSE, sTmp.makeStringAndClear() );
        }

        aTemp = xNode->getRepeatCount();
        if( aTemp.hasValue() )
        {
            Timing eTiming;
            if( (aTemp >>= eTiming ) && (eTiming == Timing_INDEFINITE ) )
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_REPEATCOUNT, XML_INDEFINITE );
            else if( aTemp >>= fTemp )
            {
                ::sax::Converter::convertDouble( sTmp, fTemp );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_REPEATCOUNT, sTmp.makeStringAndClear() );
            }
        }

        aTemp = xNode->getRepeatDuration();
        if( aTemp.hasValue() )
        {
            Timing eTiming;
            if( ( aTemp >>= eTiming ) && (eTiming == Timing_INDEFINITE) )
            {
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_REPEATDUR, XML_INDEFINITE );
            }
            else if( aTemp >>= fTemp )
            {
                ::sax::Converter::convertDouble( sTmp, fTemp );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_REPEATDUR, sTmp.makeStringAndClear() );
            }
        }

        aTemp = xNode->getEndSync();
        if( aTemp.hasValue() )
        {
            if( aTemp >>= nTemp )
            {
                SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_Endsync) );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_ENDSYNC, sTmp.makeStringAndClear() );
            }
        }

        sal_Int16 nContainerNodeType = EffectNodeType::DEFAULT;
        OUString aPresetId;
        Sequence< NamedValue > aUserData( xNode->getUserData() );
        if( aUserData.hasElements() )
        {
            const NamedValue* pValue = aUserData.getConstArray();
            const sal_Int32 nLength = aUserData.getLength();
            sal_Int32 nElement;
            for( nElement = 0; nElement < nLength; nElement++, pValue++ )
            {
                if( IsXMLToken( pValue->Name, XML_NODE_TYPE ) )
                {
                    if( (pValue->Value >>= nContainerNodeType) && (nContainerNodeType != EffectNodeType::DEFAULT) )
                    {
                        SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nContainerNodeType, getAnimationsEnumMap(Animations_EnumMap_EffectNodeType) );
                        mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_NODE_TYPE, sTmp.makeStringAndClear() );
                    }
                }
                else if( IsXMLToken( pValue->Name, XML_PRESET_ID ) )
                {
                    if( pValue->Value >>= aPresetId )
                    {
                        mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_PRESET_ID, aPresetId );
                    }
                }
                else if( IsXMLToken( pValue->Name, XML_PRESET_SUB_TYPE ) )
                {
                    OUString aPresetSubType;
                    if( pValue->Value >>= aPresetSubType )
                    {
                        mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_PRESET_SUB_TYPE, aPresetSubType );
                    }
                }
                else if( IsXMLToken( pValue->Name, XML_PRESET_CLASS ) )
                {
                    sal_Int16 nEffectPresetClass = sal_Int16();
                    if( pValue->Value >>= nEffectPresetClass )
                    {
                        SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nEffectPresetClass, getAnimationsEnumMap(Animations_EnumMap_EffectPresetClass) );
                        mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_PRESET_CLASS, sTmp.makeStringAndClear() );
                    }
                }
                else if( IsXMLToken( pValue->Name, XML_MASTER_ELEMENT ) )
                {
                    Reference< XInterface > xMaster;
                    pValue->Value >>= xMaster;
                    if( xMaster.is() )
                    {
                        const OUString& rIdentifier = mrExport.getInterfaceToIdentifierMapper().getIdentifier(xMaster);
                        if( !rIdentifier.isEmpty() )
                            mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_MASTER_ELEMENT, rIdentifier );
                    }
                }
                else if( IsXMLToken( pValue->Name, XML_GROUP_ID ) )
                {
                    sal_Int32 nGroupId = 0;
                    if( pValue->Value >>= nGroupId )
                        mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_GROUP_ID, OUString::valueOf( nGroupId ) );
                }
                else
                {
                    OUString aTmp;
                    if( pValue->Value >>= aTmp )
                        mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, pValue->Name, aTmp );
                }
            }
        }

        nTemp = xNode->getType();
        switch( nTemp )
        {
        case AnimationNodeType::PAR:
        case AnimationNodeType::SEQ:
        case AnimationNodeType::ITERATE:
        {
            Reference< XTimeContainer > xContainer( xNode, UNO_QUERY_THROW );
            exportContainer( xContainer, nContainerNodeType );
        }
        break;

        case AnimationNodeType::ANIMATE:
        case AnimationNodeType::SET:
        case AnimationNodeType::ANIMATEMOTION:
        case AnimationNodeType::ANIMATECOLOR:
        case AnimationNodeType::ANIMATETRANSFORM:
        case AnimationNodeType::TRANSITIONFILTER:
        {
            Reference< XAnimate > xAnimate( xNode, UNO_QUERY_THROW );
            exportAnimate( xAnimate );
        }
        break;
        case AnimationNodeType::AUDIO:
        {
            Reference< XAudio > xAudio( xNode, UNO_QUERY_THROW );
            exportAudio( xAudio );
        }
        break;
        case AnimationNodeType::COMMAND:
        {
            Reference< XCommand > xCommand( xNode, UNO_QUERY_THROW );
            exportCommand( xCommand );
        }
        break;
        default:
            OSL_FAIL( "xmloff::AnimationsExporterImpl::exportNode(), invalid AnimationNodeType!" );
        }
    }
    catch (const RuntimeException&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::exportNode(), RuntimeException caught!" );
    }

    // if something goes wrong, its always a good idea to clear the attribute list
    mrExport.ClearAttrList();
}

void AnimationsExporterImpl::exportContainer( const Reference< XTimeContainer >& xContainer, sal_Int16 nContainerNodeType )
{
    try
    {
        const sal_Int32 nNodeType = xContainer->getType();

        if( nNodeType == AnimationNodeType::ITERATE )
        {
            OUStringBuffer sTmp;
            Reference< XIterateContainer > xIter( xContainer, UNO_QUERY_THROW );

            Any aTemp( xIter->getTarget() );
            if( aTemp.hasValue() )
            {
                convertTarget( sTmp, aTemp );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_TARGETELEMENT, sTmp.makeStringAndClear() );
            }

            sal_Int16 nTemp = xIter->getSubItem();
            if( nTemp )
            {
                SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_SubItem) );
                mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_SUB_ITEM, sTmp.makeStringAndClear() );
            }

            nTemp = xIter->getIterateType();
            if( nTemp )
            {
                SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_IterateType) );
                mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_ITERATE_TYPE, sTmp.makeStringAndClear() );
            }

            double fTemp = xIter->getIterateInterval();
            if( fTemp )
            {
                if( 0 == ( mrExport.getExportFlags() & EXPORT_SAVEBACKWARDCOMPATIBLE ) )
                {
                    // issue 146582
                    OUStringBuffer buf;
                    ::sax::Converter::convertDuration(buf, fTemp / (24*60*60));
                    mrExport.AddAttribute( XML_NAMESPACE_ANIMATION,
                            XML_ITERATE_INTERVAL, buf.makeStringAndClear());
                }
                else
                {
                    sTmp.append( fTemp );
                    sTmp.append( (sal_Unicode)'s' );
                    mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_ITERATE_INTERVAL, sTmp.makeStringAndClear() );
                }
            }
        }

        XMLTokenEnum eElementToken;
        switch( nNodeType )
        {
        case AnimationNodeType::PAR:    eElementToken = XML_PAR; break;
        case AnimationNodeType::SEQ:    eElementToken = XML_SEQ; break;
        case AnimationNodeType::ITERATE:eElementToken = XML_ITERATE; break;
        default:
            OSL_FAIL( "xmloff::AnimationsExporterImpl::exportContainer(), invalid TimeContainerType!" );
            return;
        }
        SvXMLElementExport aElement( mrExport, XML_NAMESPACE_ANIMATION, eElementToken, sal_True, sal_True );

        if( nContainerNodeType == EffectNodeType::TIMING_ROOT )
            exportTransitionNode();

        Reference< XEnumerationAccess > xEnumerationAccess( xContainer, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
            exportNode( xChildNode );
        }
    }
    catch (const RuntimeException&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::exportContainer(), RuntimeException caught!" );
    }
}

void AnimationsExporterImpl::exportAnimate( const Reference< XAnimate >& xAnimate )
{
    try
    {
        const sal_Int16 nNodeType = xAnimate->getType();

        OUStringBuffer sTmp;
        sal_Int32 nTemp;
        sal_Bool bTemp;

        Any aTemp( xAnimate->getTarget() );
        if( aTemp.hasValue() )
        {
            convertTarget( sTmp, aTemp );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_TARGETELEMENT, sTmp.makeStringAndClear() );
        }

        nTemp = xAnimate->getSubItem();
        if( nTemp )
        {
            SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_SubItem) );
            mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_SUB_ITEM, sTmp.makeStringAndClear() );
        }

        XMLTokenEnum eAttributeName = XML_TOKEN_INVALID;

        if( nNodeType == AnimationNodeType::TRANSITIONFILTER )
        {
            eAttributeName = XML_TRANSITIONFILTER;
        }
        else if( nNodeType == AnimationNodeType::ANIMATETRANSFORM )
        {
            eAttributeName = XML_ANIMATETRANSFORM;
        }
        else if( nNodeType == AnimationNodeType::ANIMATEMOTION )
        {
            eAttributeName = XML_ANIMATEMOTION;
        }
        else
        {
            OUString sTemp( xAnimate->getAttributeName() );
            if( !sTemp.isEmpty() )
            {
                const struct ImplAttributeNameConversion* p = getAnimationAttributeNamesConversionList();
                while( p->mpAPIName )
                {
                    if( sTemp.compareToAscii( p->mpAPIName ) == 0 )
                    {
                        sTemp = GetXMLToken( p->meXMLToken );
                        eAttributeName = p->meXMLToken;
                        break;
                    }

                    p++;
                }

                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_ATTRIBUTENAME, sTemp );
            }
            else
            {
                OUString aStr( "invalid" );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_ATTRIBUTENAME, aStr );
            }
        }

        Sequence< Any > aValues( xAnimate->getValues() );
        if( aValues.getLength() )
        {
            aTemp <<= aValues;
            convertValue( eAttributeName, sTmp, aTemp );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_VALUES, sTmp.makeStringAndClear() );
        }
        else
        {
            aTemp = xAnimate->getFrom();
            if( aTemp.hasValue() )
            {
                convertValue( eAttributeName, sTmp, aTemp );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_FROM, sTmp.makeStringAndClear() );
            }

            aTemp = xAnimate->getBy();
            if( aTemp.hasValue() )
            {
                convertValue( eAttributeName, sTmp, aTemp );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_BY, sTmp.makeStringAndClear() );
            }

            aTemp = xAnimate->getTo();
            if( aTemp.hasValue() )
            {
                convertValue( eAttributeName, sTmp, aTemp );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_TO, sTmp.makeStringAndClear() );
            }
        }

        if(nNodeType != AnimationNodeType::SET)
        {
            Sequence< double > aKeyTimes( xAnimate->getKeyTimes() );
            if( aKeyTimes.getLength() )
            {
                sal_Int32 nLength = aKeyTimes.getLength();
                const double* p = aKeyTimes.getConstArray();

                while( nLength-- )
                {
                    if( !sTmp.isEmpty() )
                        sTmp.append( (sal_Unicode)';' );

                    sTmp.append( *p++ );
                }
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_KEYTIMES, sTmp.makeStringAndClear() );
            }

            OUString sTemp( xAnimate->getFormula() );
            if( !sTemp.isEmpty() )
                mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_FORMULA, sTemp );

            if( (nNodeType != AnimationNodeType::TRANSITIONFILTER) &&
                (nNodeType != AnimationNodeType::AUDIO ) )
            {
                // calcMode  = "discrete | linear | paced | spline"
                nTemp = xAnimate->getCalcMode();
                if( ((nNodeType == AnimationNodeType::ANIMATEMOTION ) && (nTemp != AnimationCalcMode::PACED)) ||
                    ((nNodeType != AnimationNodeType::ANIMATEMOTION ) && (nTemp != AnimationCalcMode::LINEAR)) )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_CalcMode) );
                    mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_CALCMODE, sTmp.makeStringAndClear() );
                }

                bTemp = xAnimate->getAccumulate();
                if( bTemp )
                    mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_ACCUMULATE, XML_SUM );

                nTemp = xAnimate->getAdditive();
                if( nTemp != AnimationAdditiveMode::REPLACE )
                {
                    SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_AdditiveMode) );
                    mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_ADDITIVE, sTmp.makeStringAndClear() );
                }
            }

            Sequence< TimeFilterPair > aTimeFilter( xAnimate->getTimeFilter() );
            if( aTimeFilter.getLength() )
            {
                sal_Int32 nLength = aTimeFilter.getLength();
                const TimeFilterPair* p = aTimeFilter.getConstArray();

                while( nLength-- )
                {
                    if( !sTmp.isEmpty() )
                        sTmp.append( (sal_Unicode)';' );

                    sTmp.append( OUString::number(p->Time) + "," + OUString::number(p->Progress) );

                    p++;
                }

                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_KEYSPLINES, sTmp.makeStringAndClear() );
            }
        }

        XMLTokenEnum eElementToken = XML_ANIMATE;

        switch( nNodeType )
        {
        case AnimationNodeType::ANIMATE:
            eElementToken = XML_ANIMATE;
            break;

        case AnimationNodeType::SET:
            eElementToken = XML_SET;
            break;

        case AnimationNodeType::ANIMATEMOTION:
        {
            eElementToken = XML_ANIMATEMOTION;

            Reference< XAnimateMotion > xAnimateMotion( xAnimate, UNO_QUERY_THROW );

            aTemp = xAnimateMotion->getPath();
            if( aTemp.hasValue() )
            {
                convertPath( sTmp, aTemp );
                mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_PATH, sTmp.makeStringAndClear() );
            }

            // TODO: origin = ( parent | layout )
            aTemp = xAnimateMotion->getOrigin();
        }
        break;

        case AnimationNodeType::ANIMATECOLOR:
        {
            eElementToken = XML_ANIMATECOLOR;

            Reference< XAnimateColor > xAnimateColor( xAnimate, UNO_QUERY_THROW );

            nTemp = xAnimateColor->getColorInterpolation();
            mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_COLOR_INTERPOLATION, (nTemp == AnimationColorSpace::RGB) ? XML_RGB : XML_HSL );

            bTemp = xAnimateColor->getDirection();
            mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_COLOR_INTERPOLATION_DIRECTION, bTemp ? XML_CLOCKWISE : XML_COUNTER_CLOCKWISE );
        }
        break;

        case AnimationNodeType::ANIMATETRANSFORM:
        {
            eElementToken = XML_ANIMATETRANSFORM;

            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_ATTRIBUTENAME, XML_TRANSFORM );

            Reference< XAnimateTransform > xTransform( xAnimate, UNO_QUERY_THROW );
            nTemp = xTransform->getTransformType();
            SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTemp, getAnimationsEnumMap(Animations_EnumMap_TransformType) );
            mrExport.AddAttribute( XML_NAMESPACE_SVG, XML_TYPE, sTmp.makeStringAndClear() );
        }
        break;

        case AnimationNodeType::TRANSITIONFILTER:
        {
            Reference< XTransitionFilter > xTransitionFilter( xAnimate, UNO_QUERY );
            eElementToken = XML_TRANSITIONFILTER;

            sal_Int16 nTransition = xTransitionFilter->getTransition();
            SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nTransition, getAnimationsEnumMap(Animations_EnumMap_TransitionType) );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_TYPE, sTmp.makeStringAndClear() );

            sal_Int16 nSubtype = xTransitionFilter->getSubtype();
            if( nSubtype != TransitionSubType::DEFAULT )
            {
                SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nSubtype, getAnimationsEnumMap(Animations_EnumMap_TransitionSubType) );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_SUBTYPE, sTmp.makeStringAndClear() );
            }

            bTemp = xTransitionFilter->getMode();
            if( !bTemp )
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_MODE, XML_OUT );

            bTemp = xTransitionFilter->getDirection();
            if( !bTemp )
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_DIRECTION, XML_REVERSE );

            if( (nTransition == TransitionType::FADE) && ((nSubtype == TransitionSubType::FADETOCOLOR) || (nSubtype == TransitionSubType::FADEFROMCOLOR) ))
            {
                nTemp = xTransitionFilter->getFadeColor();
                ::sax::Converter::convertColor( sTmp, nTemp );
                mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_FADECOLOR, sTmp.makeStringAndClear() );
            }
        }
        break;
        }

        SvXMLElementExport aElement( mrExport, XML_NAMESPACE_ANIMATION, eElementToken, sal_True, sal_True );

    }
    catch (const Exception&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::exportAnimate(), Exception cought!" );
    }
}

void AnimationsExporterImpl::exportAudio( const Reference< XAudio >& xAudio )
{
    if( xAudio.is() ) try
    {
        OUString aSourceURL;
        xAudio->getSource() >>= aSourceURL;
        if( !aSourceURL.isEmpty() )
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, mrExport.GetRelativeReference( aSourceURL ) );

        const double fVolume = xAudio->getVolume();
        if( fVolume != 1.0 )
        {
            OUStringBuffer sTmp;
            ::sax::Converter::convertDouble( sTmp, fVolume );
            mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_AUDIO_LEVEL, sTmp.makeStringAndClear() );
        }

/* todo?
        sal_Int32 nEndAfterSlide = 0;
        xAudio->getEndAfterSlide() >>= nEndAfterSlide;
        if( nEndAfterSlide != 0 )
            mrExport.AddAttribute( );
*/
        SvXMLElementExport aElement( mrExport, XML_NAMESPACE_ANIMATION, XML_AUDIO, sal_True, sal_True );

    }
    catch (const Exception&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::exportAudio(), exception caught!" );
    }
}

void AnimationsExporterImpl::exportCommand( const Reference< XCommand >& xCommand )
{
    if( xCommand.is() ) try
    {
        OUStringBuffer sTmp;
        Any aTemp( xCommand->getTarget() );
        if( aTemp.hasValue() )
        {
            convertTarget( sTmp, aTemp );
            mrExport.AddAttribute( XML_NAMESPACE_SMIL, XML_TARGETELEMENT, sTmp.makeStringAndClear() );
        }

        sal_Int16 nCommand = xCommand->getCommand();
        SvXMLUnitConverter::convertEnum( sTmp, (sal_uInt16)nCommand, getAnimationsEnumMap(Animations_EnumMap_Command) );
        mrExport.AddAttribute( XML_NAMESPACE_ANIMATION, XML_COMMAND, sTmp.makeStringAndClear() );

// todo virtual ::com::sun::star::uno::Any SAL_CALL getParameter() throw (::com::sun::star::uno::RuntimeException) = 0;

        SvXMLElementExport aElement( mrExport, XML_NAMESPACE_ANIMATION, XML_COMMAND, sal_True, sal_True );

    }
    catch (const Exception&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::exportCommand(), exception caught!" );
    }
}

Reference< XInterface > AnimationsExporterImpl::getParagraphTarget( const ParagraphTarget* pTarget ) const
{
    if( pTarget ) try
    {
        Reference< XEnumerationAccess > xParaEnumAccess( pTarget->Shape, UNO_QUERY_THROW );

        Reference< XEnumeration > xEnumeration( xParaEnumAccess->createEnumeration(), UNO_QUERY_THROW );
        sal_Int32 nParagraph = pTarget->Paragraph;

        while( xEnumeration->hasMoreElements() )
        {
            Reference< XInterface > xRef( xEnumeration->nextElement(), UNO_QUERY );
            if( nParagraph-- == 0 )
                return xRef;
        }
    }
    catch (const RuntimeException&)
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::getParagraphTarget(), RuntimeException caught!" );
    }

    Reference< XInterface > xRef;
    return xRef;
}

void AnimationsExporterImpl::convertPath( OUStringBuffer& sTmp, const Any& rPath ) const
{
    OUString aStr;
    rPath >>= aStr;

    sTmp = aStr;
}

void AnimationsExporterImpl::convertValue( XMLTokenEnum eAttributeName, OUStringBuffer& sTmp, const Any& rValue ) const
{
    if( !rValue.hasValue() )
        return;

    if( rValue.getValueType() == ::getCppuType((const ValuePair*)0) )
    {
        const ValuePair* pValuePair = static_cast< const ValuePair* >( rValue.getValue() );
        OUStringBuffer sTmp2;
        convertValue( eAttributeName, sTmp, pValuePair->First );
        sTmp.append( (sal_Unicode)',' );
        convertValue( eAttributeName, sTmp2, pValuePair->Second );
        sTmp.append( sTmp2.makeStringAndClear() );
    }
    else if( rValue.getValueType() == ::getCppuType((Sequence<Any>*)0) )
    {
        const Sequence<Any>* pSequence = static_cast< const Sequence<Any>* >( rValue.getValue() );
        const sal_Int32 nLength = pSequence->getLength();
        sal_Int32 nElement;
        const Any* pAny = pSequence->getConstArray();

        OUStringBuffer sTmp2;

        for( nElement = 0; nElement < nLength; nElement++, pAny++ )
        {
            if( !sTmp.isEmpty() )
                sTmp.append( (sal_Unicode)';' );
            convertValue( eAttributeName, sTmp2, *pAny );
            sTmp.append( sTmp2.makeStringAndClear() );
        }
    }
    else
    {
        OUString aString;
        sal_Int32 nType;

        switch( eAttributeName )
        {
        case XML_X:
        case XML_Y:
        case XML_WIDTH:
        case XML_HEIGHT:
        case XML_ANIMATETRANSFORM:
        case XML_ANIMATEMOTION:
        {
            if( rValue >>= aString )
            {
                sTmp.append( aString );
            }
            else if( rValue.getValueType() == ::getCppuType((const double*)0) )
            {
                sTmp.append( *(static_cast< const double* >( rValue.getValue() )) );
            }
            else
            {
                OSL_FAIL( "xmloff::AnimationsExporterImpl::convertValue(), invalid value type!" );
            }
            return;
        }

        case XML_SKEWX:
        case XML_ROTATE:            nType = XML_TYPE_DOUBLE;                    break;
        case XML_TEXT_ROTATION_ANGLE: nType = XML_TYPE_NUMBER16;                break;
        case XML_FILL_COLOR:
        case XML_STROKE_COLOR:
        case XML_DIM:
        case XML_COLOR:             nType = XML_TYPE_COLOR;                     break;
        case XML_FILL:              nType = XML_SD_TYPE_FILLSTYLE;              break;
        case XML_STROKE:            nType = XML_SD_TYPE_STROKE;                 break;
        case XML_FONT_WEIGHT:       nType = XML_TYPE_TEXT_WEIGHT;               break;
        case XML_FONT_STYLE:        nType = XML_TYPE_TEXT_POSTURE;              break;
        case XML_TEXT_UNDERLINE:    nType = XML_TYPE_TEXT_UNDERLINE_STYLE;      break;
        case XML_FONT_SIZE:         nType = XML_TYPE_DOUBLE_PERCENT;            break;
        case XML_VISIBILITY:        nType = XML_SD_TYPE_PRESPAGE_VISIBILITY;    break;
        case XML_OPACITY:
        case XML_TRANSITIONFILTER:  nType = XML_TYPE_DOUBLE;                    break;
        default:
            OSL_FAIL( "xmloff::AnimationsExporterImpl::convertValue(), invalid AttributeName!" );
            nType = XML_TYPE_STRING;
        }

        //const XMLPropertyHandler* pHandler = static_cast<SdXMLExport*>(&mrExport)->GetSdPropHdlFactory()->GetPropertyHandler( nType );
        const XMLPropertyHandler* pHandler = mpSdPropHdlFactory->GetPropertyHandler( nType );
        if( pHandler )
        {
            pHandler->exportXML( aString, rValue, mrExport.GetMM100UnitConverter() );
            sTmp.append( aString );
        }
    }
}

void AnimationsExporterImpl::convertTiming( OUStringBuffer& sTmp, const Any& rValue ) const
{
    if( !rValue.hasValue() )
        return;

    if( rValue.getValueType() == ::getCppuType((Sequence<Any>*)0) )
    {
        const Sequence<Any>* pSequence = static_cast< const Sequence<Any>* >( rValue.getValue() );
        const sal_Int32 nLength = pSequence->getLength();
        sal_Int32 nElement;
        const Any* pAny = pSequence->getConstArray();

        OUStringBuffer sTmp2;

        for( nElement = 0; nElement < nLength; nElement++, pAny++ )
        {
            if( !sTmp.isEmpty() )
                sTmp.append( (sal_Unicode)';' );
            convertTiming( sTmp2, *pAny );
            sTmp.append( sTmp2.makeStringAndClear() );
        }
    }
    else if( rValue.getValueType() == ::getCppuType((const double*)0) )
    {
        sTmp.append( *(static_cast< const double* >( rValue.getValue() )) );
        sTmp.append( sal_Unicode('s'));
    }
    else if( rValue.getValueType() == ::getCppuType((const Timing*)0) )
    {
        const Timing* pTiming = static_cast< const Timing* >( rValue.getValue() );
        sTmp.append( GetXMLToken( (*pTiming == Timing_MEDIA) ? XML_MEDIA : XML_INDEFINITE ) );
    }
    else if( rValue.getValueType() == ::getCppuType((const Event*)0) )
    {
        OUStringBuffer sTmp2;

        const Event* pEvent = static_cast< const Event* >( rValue.getValue() );

        if( pEvent->Trigger != EventTrigger::NONE )
        {
            if( pEvent->Source.hasValue() )
            {
                convertSource( sTmp, pEvent->Source );
                sTmp.append( (sal_Unicode)'.' );
            }

            SvXMLUnitConverter::convertEnum( sTmp2, (sal_uInt16)pEvent->Trigger, getAnimationsEnumMap(Animations_EnumMap_EventTrigger) );

            sTmp.append( sTmp2.makeStringAndClear() );
        }

        if( pEvent->Offset.hasValue() )
        {
            convertTiming( sTmp2, pEvent->Offset );

            if( !sTmp.isEmpty() )
                sTmp.append( (sal_Unicode)'+' );

            sTmp.append( sTmp2.makeStringAndClear() );
        }
    }
    else
    {
        OSL_FAIL( "xmloff::AnimationsExporterImpl::convertTiming(), invalid value type!" );
    }
}

void AnimationsExporterImpl::convertSource( OUStringBuffer& sTmp, const Any& rSource ) const
{
    convertTarget( sTmp, rSource );
}

void AnimationsExporterImpl::convertTarget( OUStringBuffer& sTmp, const Any& rTarget ) const
{
    if( !rTarget.hasValue() )
        return;

    Reference< XInterface > xRef;

    if( rTarget.getValueTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE )
    {
        rTarget >>= xRef;
    }
    else if( rTarget.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
    {
        xRef = getParagraphTarget( static_cast< const ParagraphTarget* >( rTarget.getValue() ) );
    }

    DBG_ASSERT( xRef.is(), "xmloff::AnimationsExporterImpl::convertTarget(), invalid target type!" );
    if( xRef.is() )
    {
        const OUString& rIdentifier = mrExport.getInterfaceToIdentifierMapper().getIdentifier(xRef);
        if( !rIdentifier.isEmpty() )
            sTmp.append( rIdentifier );
    }
}

void AnimationsExporterImpl::prepareValue( const Any& rValue )
{
    if( !rValue.hasValue() )
        return;

    if( rValue.getValueType() == ::getCppuType((const ValuePair*)0) )
    {
        const ValuePair* pValuePair = static_cast< const ValuePair* >( rValue.getValue() );
        prepareValue( pValuePair->First );
        prepareValue( pValuePair->Second );
    }
    else if( rValue.getValueType() == ::getCppuType((Sequence<Any>*)0) )
    {
        const Sequence<Any>* pSequence = static_cast< const Sequence<Any>* >( rValue.getValue() );
        const sal_Int32 nLength = pSequence->getLength();
        sal_Int32 nElement;
        const Any* pAny = pSequence->getConstArray();

        for( nElement = 0; nElement < nLength; nElement++, pAny++ )
            prepareValue( *pAny );
    }
    else if( rValue.getValueTypeClass() == ::com::sun::star::uno::TypeClass_INTERFACE )
    {
        Reference< XInterface> xRef( rValue, UNO_QUERY );
        if( xRef.is() )
            mrExport.getInterfaceToIdentifierMapper().registerReference( xRef );
    }
    else if( rValue.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
    {
        Reference< XInterface> xRef( getParagraphTarget( static_cast< const ParagraphTarget* >( rValue.getValue() ) ) );
        if( xRef.is() )
            mrExport.getInterfaceToIdentifierMapper().registerReference( xRef );
    }
    else if( rValue.getValueType() == ::getCppuType((const Event*)0) )
    {
        const Event* pEvent = static_cast< const Event* >( rValue.getValue() );
        prepareValue( pEvent->Source );
    }
}

AnimationsExporter::AnimationsExporter( SvXMLExport& rExport, const Reference< XPropertySet >& xPageProps )
{
    mpImpl = new AnimationsExporterImpl( rExport, xPageProps );
}

AnimationsExporter::~AnimationsExporter()
{
    delete mpImpl;
}

void AnimationsExporter::prepare( Reference< XAnimationNode > xRootNode )
{
    try
    {
        if( xRootNode.is() )
        {
            mpImpl->prepareTransitionNode();
            mpImpl->prepareNode( xRootNode );
        }
    }
    catch (const RuntimeException&)
    {
        OSL_FAIL( "xmloff::AnimationsExporter::prepare(), exception caught" );
    }
}

void AnimationsExporter::exportAnimations( Reference< XAnimationNode > xRootNode )
{
    try
    {
        if( xRootNode.is() )
        {
            bool bHasEffects = mpImpl->mbHasTransition;

            if( !bHasEffects )
            {
                // first check if there are no animations
                Reference< XEnumerationAccess > xEnumerationAccess( xRootNode, UNO_QUERY_THROW );
                Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
                if( xEnumeration->hasMoreElements() )
                {
                    // first child node may be an empty main sequence, check this
                    Reference< XAnimationNode > xMainNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
                    Reference< XEnumerationAccess > xMainEnumerationAccess( xMainNode, UNO_QUERY_THROW );
                    Reference< XEnumeration > xMainEnumeration( xMainEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );

                    // only export if the main sequence is not empty or if there are additional
                    // trigger sequences
                    bHasEffects = xMainEnumeration->hasMoreElements() || xEnumeration->hasMoreElements();
                }
            }

            if( bHasEffects )
                mpImpl->exportNode( xRootNode );
        }
    }
    catch (const RuntimeException&)
    {
        OSL_FAIL( "xmloff::AnimationsExporter::exportAnimations(), exception caught" );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
