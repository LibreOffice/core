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

#include <o3tl/any.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include "epptooxml.hxx"
#include <sax/fshelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/AnimationValueType.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/text/XSimpleText.hpp>

#include <oox/export/utils.hxx>
#include <oox/ppt/pptfilterhelpers.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include "pptexanimations.hxx"
#include "../ppt/pptanimations.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::ppt;
using ::sax_fastparser::FSHelperPtr;
using namespace oox::drawingml;
using namespace oox::core;

void PowerPointExport::WriteAnimationProperty(const FSHelperPtr& pFS, const Any& rAny,
                                              sal_Int32 nToken)
{
    if (!rAny.hasValue())
        return;

    ValuePair aPair;

    if (rAny >>= aPair)
    {
        double x, y;
        if ((aPair.First >>= x) && (aPair.Second >>= y))
        {
            if (nToken == XML_by)
            {
                // MS needs ending values but we have offset values.
                x += 1.0;
                y += 1.0;
            }
            pFS->singleElementNS(XML_p, nToken, XML_x, OString::number(x * 100000).getStr(), XML_y,
                                 OString::number(y * 100000).getStr(), FSEND);
        }
        return;
    }

    sal_uInt32 nRgb;
    double fDouble;

    uno::TypeClass aClass = rAny.getValueType().getTypeClass();
    bool bWriteToken
        = nToken
          && (aClass == TypeClass_LONG || aClass == TypeClass_DOUBLE || aClass == TypeClass_STRING);

    if (bWriteToken)
        pFS->startElementNS(XML_p, nToken, FSEND);

    switch (rAny.getValueType().getTypeClass())
    {
        case TypeClass_LONG:
            rAny >>= nRgb;
            pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(nRgb), FSEND);
            break;
        case TypeClass_DOUBLE:
            rAny >>= fDouble;
            pFS->singleElementNS(XML_p, XML_fltVal, XML_val, DS(fDouble), FSEND);
            break;
        case TypeClass_STRING:
            pFS->singleElementNS(XML_p, XML_strVal, XML_val, USS(*o3tl::doAccess<OUString>(rAny)),
                                 FSEND);
            break;
        default:
            break;
    }

    if (bWriteToken)
        pFS->endElementNS(XML_p, nToken);
}

void PowerPointExport::WriteAnimateValues(const FSHelperPtr& pFS,
                                          const Reference<XAnimate>& rXAnimate)
{
    const Sequence<double> aKeyTimes = rXAnimate->getKeyTimes();
    if (aKeyTimes.getLength() <= 0)
        return;
    const Sequence<Any> aValues = rXAnimate->getValues();
    const OUString& sFormula = rXAnimate->getFormula();
    const OUString& rAttributeName = rXAnimate->getAttributeName();

    SAL_INFO("sd.eppt", "animate values, formula: " << USS(sFormula));

    pFS->startElementNS(XML_p, XML_tavLst, FSEND);

    for (int i = 0; i < aKeyTimes.getLength(); i++)
    {
        SAL_INFO("sd.eppt", "animate value " << i << ": " << aKeyTimes[i]);
        if (aValues[i].hasValue())
        {
            pFS->startElementNS(XML_p, XML_tav, XML_fmla,
                                sFormula.isEmpty() ? nullptr : USS(sFormula), XML_tm,
                                I32S(static_cast<sal_Int32>(aKeyTimes[i] * 100000.0)), FSEND);
            pFS->startElementNS(XML_p, XML_val, FSEND);
            ValuePair aPair;
            if (aValues[i] >>= aPair)
            {
                WriteAnimationProperty(
                    pFS, AnimationExporter::convertAnimateValue(aPair.First, rAttributeName));
                WriteAnimationProperty(
                    pFS, AnimationExporter::convertAnimateValue(aPair.Second, rAttributeName));
            }
            else
                WriteAnimationProperty(
                    pFS, AnimationExporter::convertAnimateValue(aValues[i], rAttributeName));

            pFS->endElementNS(XML_p, XML_val);
            pFS->endElementNS(XML_p, XML_tav);
        }
    }

    pFS->endElementNS(XML_p, XML_tavLst);
}

void PowerPointExport::WriteAnimateTo(const FSHelperPtr& pFS, const Any& rValue,
                                      const OUString& rAttributeName)
{
    if (!rValue.hasValue())
        return;

    SAL_INFO("sd.eppt", "to attribute name: " << USS(rAttributeName));

    WriteAnimationProperty(pFS, AnimationExporter::convertAnimateValue(rValue, rAttributeName),
                           XML_to);
}

void PowerPointExport::WriteAnimationAttributeName(const FSHelperPtr& pFS,
                                                   const OUString& rAttributeName)
{
    if (rAttributeName.isEmpty())
        return;

    pFS->startElementNS(XML_p, XML_attrNameLst, FSEND);

    SAL_INFO("sd.eppt", "write attribute name: " << USS(rAttributeName));

    if (rAttributeName == "X;Y")
    {
        pFS->startElementNS(XML_p, XML_attrName, FSEND);
        pFS->writeEscaped("ppt_x");
        pFS->endElementNS(XML_p, XML_attrName);

        pFS->startElementNS(XML_p, XML_attrName, FSEND);
        pFS->writeEscaped("ppt_y");
        pFS->endElementNS(XML_p, XML_attrName);
    }
    else
    {
        const oox::ppt::ImplAttributeNameConversion* attrConv
            = oox::ppt::getAttributeConversionList();
        const char* pAttribute = nullptr;

        while (attrConv->mpAPIName != nullptr)
        {
            if (rAttributeName.equalsAscii(attrConv->mpAPIName))
            {
                pAttribute = attrConv->mpMSName;
                break;
            }
            attrConv++;
        }

        if (pAttribute)
        {
            pFS->startElementNS(XML_p, XML_attrName, FSEND);
            pFS->writeEscaped(pAttribute);
            pFS->endElementNS(XML_p, XML_attrName);
        }
        else
        {
            SAL_WARN("sd.eppt", "unhandled animation attribute name: " << rAttributeName);
        }
    }

    pFS->endElementNS(XML_p, XML_attrNameLst);
}

void PowerPointExport::WriteAnimationTarget(const FSHelperPtr& pFS, const Any& rTarget)
{
    sal_Int32 nParagraph = -1;
    bool bParagraphTarget = false;

    Reference<XShape> rXShape;
    rTarget >>= rXShape;

    if (!rXShape.is())
    {
        ParagraphTarget aParagraphTarget;
        if (rTarget >>= aParagraphTarget)
            rXShape = aParagraphTarget.Shape;
        if (rXShape.is())
        {
            nParagraph = static_cast<sal_Int32>(aParagraphTarget.Paragraph);
            Reference<XSimpleText> xText(rXShape, UNO_QUERY);
            if (xText.is())
            {
                bParagraphTarget = true;
            }
        }
    }

    if (rXShape.is())
    {
        pFS->startElementNS(XML_p, XML_tgtEl, FSEND);
        pFS->startElementNS(XML_p, XML_spTgt, XML_spid,
                            I32S(ShapeExport::GetShapeID(rXShape, &maShapeMap)), FSEND);
        if (bParagraphTarget)
        {
            pFS->startElementNS(XML_p, XML_txEl, FSEND);
            pFS->singleElementNS(XML_p, XML_pRg, XML_st, I32S(nParagraph), XML_end,
                                 I32S(nParagraph), FSEND);
            pFS->endElementNS(XML_p, XML_txEl);
        }
        pFS->endElementNS(XML_p, XML_spTgt);
        pFS->endElementNS(XML_p, XML_tgtEl);
    }
}

void PowerPointExport::WriteAnimationNodeAnimate(const FSHelperPtr& pFS,
                                                 const Reference<XAnimationNode>& rXNode,
                                                 sal_Int32 nXmlNodeType, bool bMainSeqChild)
{
    Reference<XAnimate> rXAnimate(rXNode, UNO_QUERY);
    if (!rXAnimate.is())
        return;

    const char* pCalcMode = nullptr;
    const char* pValueType = nullptr;
    bool bSimple = (nXmlNodeType != XML_anim);
    bool bTo = true;

    if (!bSimple)
    {
        switch (rXAnimate->getCalcMode())
        {
            case AnimationCalcMode::DISCRETE:
                pCalcMode = "discrete";
                break;
            case AnimationCalcMode::LINEAR:
                pCalcMode = "lin";
                break;
        }

        switch (AnimationExporter::GetValueTypeForAttributeName(rXAnimate->getAttributeName()))
        {
            case AnimationValueType::STRING:
                pValueType = "str";
                break;
            case AnimationValueType::NUMBER:
                pValueType = "num";
                break;
            case AnimationValueType::COLOR:
                pValueType = "clr";
                break;
        }
    }

    if (nXmlNodeType == XML_animMotion)
    {
        OUString aPath;
        Reference<XAnimateMotion> xMotion(rXNode, UNO_QUERY);
        if (xMotion.is())
        {
            xMotion->getPath() >>= aPath;
            ::basegfx::B2DPolyPolygon aPolyPoly;
            if (::basegfx::utils::importFromSvgD(aPolyPoly, aPath, true, nullptr))
                aPath = ::basegfx::utils::exportToSvgD(aPolyPoly, false, false, true, true);
        }

        pFS->startElementNS(XML_p, nXmlNodeType, XML_origin, "layout", XML_path,
                            OUStringToOString(aPath, RTL_TEXTENCODING_UTF8), FSEND);
    }
    else if (nXmlNodeType == XML_animRot)
    {
        // when const char* is nullptr, the attribute is completely omitted in the output
        const char* pBy = nullptr;
        const char* pFrom = nullptr;
        const char* pTo = nullptr;
        OString aBy, aFrom, aTo;

        Reference<XAnimateTransform> xTransform(rXNode, UNO_QUERY);
        if (xTransform.is())
        {
            double value;
            if (xTransform->getBy() >>= value)
            {
                aBy = OString::number(static_cast<int>(value * PER_DEGREE));
                pBy = aBy.getStr();
            }

            if (xTransform->getFrom() >>= value)
            {
                aFrom = OString::number(static_cast<int>(value * PER_DEGREE));
                pFrom = aFrom.getStr();
            }

            if (xTransform->getTo() >>= value)
            {
                aTo = OString::number(static_cast<int>(value * PER_DEGREE));
                pTo = aTo.getStr();
            }
        }

        pFS->startElementNS(XML_p, nXmlNodeType, XML_by, pBy, XML_from, pFrom, XML_to, pTo, FSEND);
    }
    else if (nXmlNodeType == XML_animClr)
    {
        Reference<XAnimateColor> xColor(rXNode, UNO_QUERY);
        const char* pColorSpace = "rgb";
        const char* pDirection = nullptr;
        if (xColor.is() && xColor->getColorInterpolation() == AnimationColorSpace::HSL)
        {
            // Note: from, to, by can still be specified in any supported format.
            pColorSpace = "hsl";
            pDirection = xColor->getDirection() ? "cw" : "ccw";
        }
        pFS->startElementNS(XML_p, nXmlNodeType, XML_clrSpc, pColorSpace, XML_dir, pDirection,
                            XML_calcmode, pCalcMode, XML_valueType, pValueType, FSEND);
    }
    else
    {
        OUString sFrom, sTo, sBy;
        if (rXAnimate.is() && nXmlNodeType == XML_anim)
        {
            OUString sAttributeName = rXAnimate->getAttributeName();
            Any aFrom
                = AnimationExporter::convertAnimateValue(rXAnimate->getFrom(), sAttributeName);
            aFrom >>= sFrom;
            Any aTo = AnimationExporter::convertAnimateValue(rXAnimate->getTo(), sAttributeName);
            aTo >>= sTo;
            Any aBy = AnimationExporter::convertAnimateValue(rXAnimate->getBy(), sAttributeName);
            aBy >>= sBy;
        }

        pFS->startElementNS(XML_p, nXmlNodeType, XML_calcmode, pCalcMode, XML_valueType, pValueType,
                            XML_from, sFrom.getLength() ? USS(sFrom) : nullptr, XML_to,
                            sTo.getLength() ? USS(sTo) : nullptr, XML_by,
                            sBy.getLength() ? USS(sBy) : nullptr, FSEND);
        bTo = sTo.isEmpty() && sFrom.isEmpty() && sBy.isEmpty();
    }

    WriteAnimationNodeAnimateInside(pFS, rXNode, bMainSeqChild, bSimple, bTo);
    pFS->endElementNS(XML_p, nXmlNodeType);
}

void PowerPointExport::WriteAnimateColorColor(const FSHelperPtr& pFS, const Any& rAny,
                                              sal_Int32 nToken)
{
    if (!rAny.hasValue())
        return;

    sal_Int32 nColor = 0;
    if (rAny >>= nColor)
    {
        pFS->startElementNS(XML_p, nToken, FSEND);

        if (nToken == XML_by)
        {
            // CT_TLByRgbColorTransform
            SAL_WARN("sd.eppt", "Export p:rgb in p:by of animClr isn't implemented yet.");
        }
        else
        {
            // CT_Color
            pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(nColor), FSEND);
        }

        pFS->endElementNS(XML_p, nToken);
    }

    Sequence<double> aHSL(3);
    if (rAny >>= aHSL)
    {
        pFS->startElementNS(XML_p, nToken, FSEND);

        if (nToken == XML_by)
        {
            // CT_TLByHslColorTransform
            pFS->singleElementNS(XML_p, XML_hsl, XML_h, I32S(aHSL[0] * 60000), // ST_Angel
                                 XML_s, I32S(aHSL[1] * 100000), XML_l, I32S(aHSL[2] * 100000),
                                 FSEND);
        }
        else
        {
            // CT_Color
            SAL_WARN("sd.eppt", "Export p:hsl in p:from or p:to of animClr isn't implemented yet.");
        }

        pFS->endElementNS(XML_p, nToken);
    }
}

void PowerPointExport::WriteAnimationNodeAnimateInside(const FSHelperPtr& pFS,
                                                       const Reference<XAnimationNode>& rXNode,
                                                       bool bMainSeqChild, bool bSimple,
                                                       bool bWriteTo)
{
    Reference<XAnimate> rXAnimate(rXNode, UNO_QUERY);
    if (!rXAnimate.is())
        return;

    const char* pAdditive = nullptr;

    if (!bSimple)
    {
        switch (rXAnimate->getAdditive())
        {
            case AnimationAdditiveMode::BASE:
                pAdditive = "base";
                break;
            case AnimationAdditiveMode::SUM:
                pAdditive = "sum";
                break;
            case AnimationAdditiveMode::REPLACE:
                pAdditive = "repl";
                break;
            case AnimationAdditiveMode::MULTIPLY:
                pAdditive = "mult";
                break;
            case AnimationAdditiveMode::NONE:
                pAdditive = "none";
                break;
        }
    }

    pFS->startElementNS(XML_p, XML_cBhvr, XML_additive, pAdditive, FSEND);
    WriteAnimationNodeCommonPropsStart(pFS, rXNode, true, bMainSeqChild);

    Reference<XIterateContainer> xIterate(rXNode->getParent(), UNO_QUERY);
    WriteAnimationTarget(pFS, xIterate.is() ? xIterate->getTarget() : rXAnimate->getTarget());

    Reference<XAnimateTransform> xTransform(rXNode, UNO_QUERY);

    // The attribute name of AnimateTransform is "Transform", we have to fix it.
    OUString sNewAttr;
    if (xTransform.is() && xTransform->getTransformType() == AnimationTransformType::ROTATE)
        sNewAttr = "Rotate";

    WriteAnimationAttributeName(pFS, xTransform.is() ? sNewAttr : rXAnimate->getAttributeName());

    pFS->endElementNS(XML_p, XML_cBhvr);
    WriteAnimateValues(pFS, rXAnimate);

    Reference<XAnimateColor> xColor(rXNode, UNO_QUERY);

    if (xColor.is())
    {
        WriteAnimateColorColor(pFS, xColor->getBy(), XML_by);
        WriteAnimateColorColor(pFS, xColor->getFrom(), XML_from);
        WriteAnimateColorColor(pFS, xColor->getTo(), XML_to);
    }
    else if (xTransform.is() && xTransform->getTransformType() == AnimationTransformType::SCALE)
    {
        WriteAnimationProperty(pFS, rXAnimate->getBy(), XML_by);
        WriteAnimationProperty(pFS, rXAnimate->getFrom(), XML_from);
        WriteAnimationProperty(pFS, rXAnimate->getTo(), XML_to);
    }
    else if (bWriteTo)
        WriteAnimateTo(pFS, rXAnimate->getTo(), rXAnimate->getAttributeName());
}

void PowerPointExport::WriteAnimationCondition(const FSHelperPtr& pFS, const char* pDelay,
                                               const char* pEvent, double fDelay, bool bHasFDelay,
                                               sal_Int32 nToken)
{
    if (bHasFDelay || pDelay || pEvent)
    {
        pFS->startElementNS(XML_p, nToken, FSEND);

        if (!pEvent)
            pFS->singleElementNS(
                XML_p, XML_cond, XML_delay,
                bHasFDelay ? I64S(static_cast<sal_uInt32>(fDelay * 1000.0)) : pDelay, FSEND);
        else
        {
            pFS->startElementNS(XML_p, XML_cond, XML_delay,
                                bHasFDelay ? I64S(static_cast<sal_uInt32>(fDelay * 1000.0))
                                           : pDelay,
                                XML_evt, pEvent, FSEND);

            pFS->startElementNS(XML_p, XML_tgtEl, FSEND);
            pFS->singleElementNS(XML_p, XML_sldTgt, FSEND);
            pFS->endElementNS(XML_p, XML_tgtEl);

            pFS->endElementNS(XML_p, XML_cond);
        }

        pFS->endElementNS(XML_p, nToken);
    }
}

void PowerPointExport::WriteAnimationCondition(const FSHelperPtr& pFS, Any const& rAny,
                                               bool bWriteEvent, bool bMainSeqChild,
                                               sal_Int32 nToken)
{
    bool bHasFDelay = false;
    double fDelay = 0;
    Timing eTiming;
    Event aEvent;
    Reference<XShape> xShape;
    const char* pDelay = nullptr;
    const char* pEvent = nullptr;

    if (rAny >>= fDelay)
        bHasFDelay = true;
    else if (rAny >>= eTiming)
    {
        if (eTiming == Timing_INDEFINITE)
            pDelay = "indefinite";
    }
    else if (rAny >>= aEvent)
    {
        // TODO

        SAL_INFO("sd.eppt", "animation condition event: TODO");
        SAL_INFO("sd.eppt", "event offset has value: "
                                << aEvent.Offset.hasValue() << " trigger: " << aEvent.Trigger
                                << " source has value: " << aEvent.Source.hasValue());
        if (!bWriteEvent && aEvent.Trigger == EventTrigger::ON_NEXT && bMainSeqChild)
            pDelay = "indefinite";
        else if (bWriteEvent)
        {
            switch (aEvent.Trigger)
            {
                case EventTrigger::ON_NEXT:
                    pEvent = "onNext";
                    break;
                case EventTrigger::ON_PREV:
                    pEvent = "onPrev";
                    break;
                case EventTrigger::BEGIN_EVENT:
                    pEvent = "begin";
                    break;
                case EventTrigger::END_EVENT:
                    pEvent = "end";
                    break;
                case EventTrigger::ON_BEGIN:
                    pEvent = "onBegin";
                    break;
                case EventTrigger::ON_END:
                    pEvent = "onEnd";
                    break;
                case EventTrigger::ON_CLICK:
                    pEvent = "onClick";
                    break;
                case EventTrigger::ON_DBL_CLICK:
                    pEvent = "onDblClick";
                    break;
                case EventTrigger::ON_STOP_AUDIO:
                    pEvent = "onStopAudio";
                    break;
                case EventTrigger::ON_MOUSE_ENTER:
                    pEvent = "onMouseOver"; // not exact?
                    break;
                case EventTrigger::ON_MOUSE_LEAVE:
                    pEvent = "onMouseOut";
                    break;
            }
        }

        if (aEvent.Offset >>= fDelay)
        {
            bHasFDelay = true;
            SAL_INFO("sd.eppt", "event offset: " << fDelay);
        }
        else if (aEvent.Offset >>= eTiming)
        {
            if (eTiming == Timing_INDEFINITE)
                pDelay = "indefinite";
            SAL_INFO("sd.eppt", "event offset timing: " << static_cast<int>(eTiming));
        }
    }
    else if (rAny >>= xShape)
    {
        SAL_INFO("sd.eppt", "Got the xShape: " << xShape->getShapeType());
        if (xShape->getShapeType() == "com.sun.star.drawing.MediaShape"
            || xShape->getShapeType() == "com.sun.star.presentation.MediaShape")
        {
            // write the default
            bHasFDelay = true;
        }
    }

    WriteAnimationCondition(pFS, pDelay, pEvent, fDelay, bHasFDelay, nToken);
}

void PowerPointExport::WriteAnimationNodeCommonPropsStart(const FSHelperPtr& pFS,
                                                          const Reference<XAnimationNode>& rXNode,
                                                          bool bSingle, bool bMainSeqChild)
{
    const char* pDuration = nullptr;
    const char* pRestart = nullptr;
    const char* pNodeType = nullptr;
    const char* pPresetClass = nullptr;
    const char* pFill = nullptr;
    double fDuration = 0;
    Any aAny;

    aAny = rXNode->getDuration();
    if (aAny.hasValue())
    {
        Timing eTiming;

        if (aAny >>= eTiming)
        {
            if (eTiming == Timing_INDEFINITE)
                pDuration = "indefinite";
        }
        else
            aAny >>= fDuration;
    }

    switch (rXNode->getRestart())
    {
        case AnimationRestart::ALWAYS:
            pRestart = "always";
            break;
        case AnimationRestart::WHEN_NOT_ACTIVE:
            pRestart = "whenNotActive";
            break;
        case AnimationRestart::NEVER:
            pRestart = "never";
            break;
    }

    const Sequence<NamedValue> aUserData = rXNode->getUserData();
    const Any* pAny[DFF_ANIM_PROPERTY_ID_COUNT];
    AnimationExporter::GetUserData(aUserData, pAny, sizeof(pAny));

    sal_Int16 nType = 0;
    if (pAny[DFF_ANIM_NODE_TYPE] && (*pAny[DFF_ANIM_NODE_TYPE] >>= nType))
    {
        switch (nType)
        {
            case EffectNodeType::TIMING_ROOT:
                pNodeType = "tmRoot";
                if (!pDuration)
                    pDuration = "indefinite";
                if (!pRestart)
                    pRestart = "never";
                break;
            case EffectNodeType::MAIN_SEQUENCE:
                pNodeType = "mainSeq";
                pDuration = "indefinite";
                break;
            case EffectNodeType::ON_CLICK:
                pNodeType = "clickEffect";
                break;
            case EffectNodeType::AFTER_PREVIOUS:
                pNodeType = "afterEffect";
                break;
            case EffectNodeType::WITH_PREVIOUS:
                pNodeType = "withEffect";
                break;
            case EffectNodeType::INTERACTIVE_SEQUENCE:
                pNodeType = "interactiveSeq";
                break;
        }
    }

    sal_uInt32 nPresetClass = DFF_ANIM_PRESS_CLASS_USER_DEFINED;
    if (pAny[DFF_ANIM_PRESET_CLASS])
    {
        if (*pAny[DFF_ANIM_PRESET_CLASS] >>= nPresetClass)
        {
            switch (nPresetClass)
            {
                case EffectPresetClass::ENTRANCE:
                    pPresetClass = "entr";
                    break;
                case EffectPresetClass::EXIT:
                    pPresetClass = "exit";
                    break;
                case EffectPresetClass::EMPHASIS:
                    pPresetClass = "emph";
                    break;
                case EffectPresetClass::MOTIONPATH:
                    pPresetClass = "path";
                    break;
                case EffectPresetClass::OLEACTION:
                    pPresetClass = "verb"; // ?
                    break;
                case EffectPresetClass::MEDIACALL:
                    pPresetClass = "mediacall";
                    break;
            }
        }
    }

    sal_uInt32 nPresetId = 0;
    bool bPresetId = false;
    if (pAny[DFF_ANIM_PRESET_ID])
    {
        OUString sPreset;
        if (*pAny[DFF_ANIM_PRESET_ID] >>= sPreset)
            nPresetId = AnimationExporter::GetPresetID(sPreset, nPresetClass, bPresetId);
    }

    sal_uInt32 nPresetSubType = 0;
    bool bPresetSubType = false;
    if (pAny[DFF_ANIM_PRESET_SUB_TYPE])
    {
        OUString sPresetSubType;
        if (*pAny[DFF_ANIM_PRESET_SUB_TYPE] >>= sPresetSubType)
        {
            nPresetSubType = AnimationExporter::TranslatePresetSubType(nPresetClass, nPresetId,
                                                                       sPresetSubType);
            bPresetSubType = true;
        }
    }

    if (nType != EffectNodeType::TIMING_ROOT && nType != EffectNodeType::MAIN_SEQUENCE)
    {
        // it doesn't seem to work right on root and mainseq nodes
        sal_Int16 nFill = AnimationExporter::GetFillMode(rXNode, AnimationFill::AUTO);
        switch (nFill)
        {
            case AnimationFill::FREEZE:
                pFill = "hold";
                break;
            case AnimationFill::HOLD:
                pFill = "hold";
                break;
            case AnimationFill::REMOVE:
                pFill = "remove";
                break;
            case AnimationFill::TRANSITION:
                pFill = "transition";
                break;
        }
    }

    bool bAutoReverse = rXNode->getAutoReverse();

    pFS->startElementNS(
        XML_p, XML_cTn, XML_id, I64S(mnAnimationNodeIdMax++), XML_dur,
        fDuration != 0 ? I32S(static_cast<sal_Int32>(fDuration * 1000.0)) : pDuration, XML_autoRev,
        bAutoReverse ? "1" : nullptr, XML_restart, pRestart, XML_nodeType, pNodeType, XML_fill,
        pFill, XML_presetClass, pPresetClass, XML_presetID, bPresetId ? I64S(nPresetId) : nullptr,
        XML_presetSubtype, bPresetSubType ? I64S(nPresetSubType) : nullptr, FSEND);

    aAny = rXNode->getBegin();
    if (aAny.hasValue())
    {
        Sequence<Any> aCondSeq;

        if (aAny >>= aCondSeq)
        {
            for (int i = 0; i < aCondSeq.getLength(); i++)
                WriteAnimationCondition(pFS, aCondSeq[i], false, bMainSeqChild, XML_stCondLst);
        }
        else
            WriteAnimationCondition(pFS, aAny, false, bMainSeqChild, XML_stCondLst);
    }

    aAny = rXNode->getEnd();
    if (aAny.hasValue())
    {
        Sequence<Any> aCondSeq;

        if (aAny >>= aCondSeq)
        {
            for (int i = 0; i < aCondSeq.getLength(); i++)
                WriteAnimationCondition(pFS, aCondSeq[i], false, bMainSeqChild, XML_endCondLst);
        }
        else
            WriteAnimationCondition(pFS, aAny, false, bMainSeqChild, XML_endCondLst);
    }

    if (rXNode->getType() == AnimationNodeType::ITERATE)
    {
        Reference<XIterateContainer> xIterate(rXNode, UNO_QUERY);
        if (xIterate.is())
        {
            const char* sType = nullptr;
            switch (xIterate->getIterateType())
            {
                case TextAnimationType::BY_PARAGRAPH:
                    sType = "el";
                    break;
                case TextAnimationType::BY_LETTER:
                    sType = "lt";
                    break;
                case TextAnimationType::BY_WORD:
                default:
                    sType = "wd";
                    break;
            }
            pFS->startElementNS(XML_p, XML_iterate, XML_type, sType, FSEND);
            pFS->singleElementNS(XML_p, XML_tmAbs, XML_val,
                                 I32S(xIterate->getIterateInterval() * 1000), FSEND);
            pFS->endElementNS(XML_p, XML_iterate);
        }
    }

    Reference<XEnumerationAccess> xEnumerationAccess(rXNode, UNO_QUERY);
    if (xEnumerationAccess.is())
    {
        Reference<XEnumeration> xEnumeration(xEnumerationAccess->createEnumeration(), UNO_QUERY);
        if (xEnumeration.is())
        {
            SAL_INFO("sd.eppt", "-----");

            if (xEnumeration->hasMoreElements())
            {
                pFS->startElementNS(XML_p, XML_childTnLst, FSEND);

                do
                {
                    Reference<XAnimationNode> xChildNode(xEnumeration->nextElement(), UNO_QUERY);
                    if (xChildNode.is())
                        WriteAnimationNode(pFS, xChildNode, nType == EffectNodeType::MAIN_SEQUENCE);
                } while (xEnumeration->hasMoreElements());

                pFS->endElementNS(XML_p, XML_childTnLst);
            }
            SAL_INFO("sd.eppt", "-----");
        }
    }

    if (bSingle)
        pFS->endElementNS(XML_p, XML_cTn);
}

void PowerPointExport::WriteAnimationNodeSeq(const FSHelperPtr& pFS,
                                             const Reference<XAnimationNode>& rXNode, sal_Int32,
                                             bool bMainSeqChild)
{
    SAL_INFO("sd.eppt", "write animation node SEQ");

    pFS->startElementNS(XML_p, XML_seq, FSEND);

    WriteAnimationNodeCommonPropsStart(pFS, rXNode, true, bMainSeqChild);

    WriteAnimationCondition(pFS, nullptr, "onPrev", 0, true, XML_prevCondLst);
    WriteAnimationCondition(pFS, nullptr, "onNext", 0, true, XML_nextCondLst);

    pFS->endElementNS(XML_p, XML_seq);
}

void PowerPointExport::WriteAnimationNodeEffect(const FSHelperPtr& pFS,
                                                const Reference<XAnimationNode>& rXNode, sal_Int32,
                                                bool bMainSeqChild)
{
    SAL_INFO("sd.eppt", "write animation node FILTER");
    Reference<XTransitionFilter> xFilter(rXNode, UNO_QUERY);
    if (xFilter.is())
    {
        const char* pFilter = ::ppt::AnimationExporter::FindTransitionName(
            xFilter->getTransition(), xFilter->getSubtype(), xFilter->getDirection());
        const char* pMode = xFilter->getMode() ? "in" : "out";
        pFS->startElementNS(XML_p, XML_animEffect, XML_filter, pFilter, XML_transition, pMode,
                            FSEND);

        WriteAnimationNodeAnimateInside(pFS, rXNode, bMainSeqChild, false);

        pFS->endElementNS(XML_p, XML_animEffect);
    }
}

void PowerPointExport::WriteAnimationNodeCommand(const FSHelperPtr& pFS,
                                                 const Reference<XAnimationNode>& rXNode, sal_Int32,
                                                 bool bMainSeqChild)
{
    SAL_INFO("sd.eppt", "write animation node COMMAND");
    Reference<XCommand> xCommand(rXNode, UNO_QUERY);
    if (xCommand.is())
    {
        const char* pType = "call";
        const char* pCommand = nullptr;
        switch (xCommand->getCommand())
        {
            case EffectCommands::VERB:
                pType = "verb";
                pCommand = "1"; /* FIXME hardcoded viewing */
                break;
            case EffectCommands::PLAY:
                pCommand = "play";
                break;
            case EffectCommands::TOGGLEPAUSE:
                pCommand = "togglePause";
                break;
            case EffectCommands::STOP:
                pCommand = "stop";
                break;
            default:
                SAL_WARN("sd.eppt", "unknown command: " << xCommand->getCommand());
                break;
        }

        pFS->startElementNS(XML_p, XML_cmd, XML_type, pType, XML_cmd, pCommand, FSEND);

        WriteAnimationNodeAnimateInside(pFS, rXNode, bMainSeqChild, false);
        pFS->startElementNS(XML_p, XML_cBhvr, FSEND);
        WriteAnimationNodeCommonPropsStart(pFS, rXNode, true, bMainSeqChild);
        WriteAnimationTarget(pFS, xCommand->getTarget());
        pFS->endElementNS(XML_p, XML_cBhvr);

        pFS->endElementNS(XML_p, XML_cmd);
    }
}

void PowerPointExport::WriteAnimationNode(const FSHelperPtr& pFS,
                                          const Reference<XAnimationNode>& rXNode,
                                          bool bMainSeqChild)
{
    SAL_INFO("sd.eppt", "export node type: " << rXNode->getType());
    sal_Int32 xmlNodeType = -1;
    typedef void (PowerPointExport::*AnimationNodeWriteMethod)(
        const FSHelperPtr&, const Reference<XAnimationNode>&, sal_Int32, bool);
    AnimationNodeWriteMethod pMethod = nullptr;

    switch (rXNode->getType())
    {
        case AnimationNodeType::ITERATE:
        case AnimationNodeType::PAR:
            xmlNodeType = XML_par;
            break;
        case AnimationNodeType::SEQ:
            pMethod = &PowerPointExport::WriteAnimationNodeSeq;
            break;
        case AnimationNodeType::ANIMATE:
            xmlNodeType = XML_anim;
            pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
            break;
        case AnimationNodeType::ANIMATEMOTION:
            xmlNodeType = XML_animMotion;
            pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
            break;
        case AnimationNodeType::ANIMATETRANSFORM:
        {
            Reference<XAnimateTransform> xTransform(rXNode, UNO_QUERY);
            if (xTransform.is())
            {
                if (xTransform->getTransformType() == AnimationTransformType::SCALE)
                {
                    xmlNodeType = XML_animScale;
                    pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
                }
                else if (xTransform->getTransformType() == AnimationTransformType::ROTATE)
                {
                    xmlNodeType = XML_animRot;
                    pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
                }
            }
        }
        break;
        case AnimationNodeType::ANIMATECOLOR:
            xmlNodeType = XML_animClr;
            pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
            break;
        case AnimationNodeType::SET:
            xmlNodeType = XML_set;
            pMethod = &PowerPointExport::WriteAnimationNodeAnimate;
            break;
        case AnimationNodeType::TRANSITIONFILTER:
            xmlNodeType = XML_animEffect;
            pMethod = &PowerPointExport::WriteAnimationNodeEffect;
            break;
        case AnimationNodeType::COMMAND:
            xmlNodeType = XML_cmd;
            pMethod = &PowerPointExport::WriteAnimationNodeCommand;
            break;
        default:
            SAL_WARN("sd.eppt", "unhandled animation node: " << rXNode->getType());
            break;
    }

    if (pMethod)
    {
        (this->*pMethod)(pFS, rXNode, xmlNodeType, bMainSeqChild);
        return;
    }

    if (xmlNodeType == -1)
        return;

    pFS->startElementNS(XML_p, xmlNodeType, FSEND);

    WriteAnimationNodeCommonPropsStart(pFS, rXNode, true, bMainSeqChild);

    pFS->endElementNS(XML_p, xmlNodeType);
}

void PowerPointExport::WriteAnimations(const FSHelperPtr& pFS)
{
    Reference<XAnimationNodeSupplier> xNodeSupplier(mXDrawPage, UNO_QUERY);
    if (xNodeSupplier.is())
    {
        const Reference<XAnimationNode> xNode(xNodeSupplier->getAnimationNode());
        if (xNode.is())
        {
            Reference<XEnumerationAccess> xEnumerationAccess(xNode, UNO_QUERY);
            if (xEnumerationAccess.is())
            {
                Reference<XEnumeration> xEnumeration(xEnumerationAccess->createEnumeration(),
                                                     UNO_QUERY);
                if (xEnumeration.is() && xEnumeration->hasMoreElements())
                {
                    pFS->startElementNS(XML_p, XML_timing, FSEND);
                    pFS->startElementNS(XML_p, XML_tnLst, FSEND);

                    WriteAnimationNode(pFS, xNode, false);

                    pFS->endElementNS(XML_p, XML_tnLst);
                    pFS->endElementNS(XML_p, XML_timing);
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
