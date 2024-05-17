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
#include <o3tl/string_view.hxx>
#include <oox/token/tokens.hxx>
#include "epptooxml.hxx"
#include <sax/fshelper.hxx>
#include <sal/log.hxx>
#include <rtl/math.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/AnimationValueType.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <oox/export/utils.hxx>
#include <oox/ppt/pptfilterhelpers.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include "pptexanimations.hxx"
#include "pptx-animations.hxx"
#include "pptx-animations-cond.hxx"
#include "pptx-animations-nodectx.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::uno;
using namespace ::ppt;
using namespace oox::drawingml;
using namespace oox::core;
using namespace oox;

using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::text::XSimpleText;
using ::sax_fastparser::FSHelperPtr;

namespace
{
void WriteAnimationProperty(const FSHelperPtr& pFS, const Any& rAny, sal_Int32 nToken = 0)
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
            pFS->singleElementNS(XML_p, nToken, XML_x, OString::number(x * 100000), XML_y,
                                 OString::number(y * 100000));
        }
        return;
    }

    sal_Int32 nRgb = {}; // spurious -Werror=maybe-uninitialized
    double fDouble = {}; // spurious -Werror=maybe-uninitialized

    TypeClass aClass = rAny.getValueType().getTypeClass();
    bool bWriteToken
        = nToken
          && (aClass == TypeClass_LONG || aClass == TypeClass_DOUBLE || aClass == TypeClass_STRING);

    if (bWriteToken)
        pFS->startElementNS(XML_p, nToken);

    switch (rAny.getValueType().getTypeClass())
    {
        case TypeClass_LONG:
            if (!(rAny >>= nRgb))
            {
                assert(false);
            }
            pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(nRgb));
            break;
        case TypeClass_DOUBLE:
            if (!(rAny >>= fDouble))
            {
                assert(false);
            }
            pFS->singleElementNS(XML_p, XML_fltVal, XML_val, OString::number(fDouble));
            break;
        case TypeClass_STRING:
            pFS->singleElementNS(XML_p, XML_strVal, XML_val, *o3tl::doAccess<OUString>(rAny));
            break;
        default:
            break;
    }

    if (bWriteToken)
        pFS->endElementNS(XML_p, nToken);
}

void WriteAnimateColorColor(const FSHelperPtr& pFS, const Any& rAny, sal_Int32 nToken)
{
    if (!rAny.hasValue())
        return;

    sal_Int32 nColor = 0;
    if (rAny >>= nColor)
    {
        pFS->startElementNS(XML_p, nToken);

        if (nToken == XML_by)
        {
            // CT_TLByRgbColorTransform
            SAL_WARN("sd.eppt", "Export p:rgb in p:by of animClr isn't implemented yet.");
        }
        else
        {
            // CT_Color
            pFS->singleElementNS(XML_a, XML_srgbClr, XML_val, I32SHEX(nColor));
        }

        pFS->endElementNS(XML_p, nToken);
    }

    Sequence<double> aHSL(3);
    if (!(rAny >>= aHSL))
        return;

    pFS->startElementNS(XML_p, nToken);

    if (nToken == XML_by)
    {
        // CT_TLByHslColorTransform
        pFS->singleElementNS(XML_p, XML_hsl, XML_h, OString::number(aHSL[0] * 60000), // ST_Angel
                             XML_s, OString::number(aHSL[1] * 100000), XML_l,
                             OString::number(aHSL[2] * 100000));
    }
    else
    {
        // CT_Color
        SAL_WARN("sd.eppt", "Export p:hsl in p:from or p:to of animClr isn't implemented yet.");
    }

    pFS->endElementNS(XML_p, nToken);
}

void WriteAnimateTo(const FSHelperPtr& pFS, const Any& rValue, const OUString& rAttributeName)
{
    if (!rValue.hasValue())
        return;

    SAL_INFO("sd.eppt", "to attribute name: " << rAttributeName.toUtf8());

    WriteAnimationProperty(pFS, AnimationExporter::convertAnimateValue(rValue, rAttributeName),
                           XML_to);
}

void WriteAnimateValues(const FSHelperPtr& pFS, const Reference<XAnimate>& rXAnimate)
{
    const Sequence<double> aKeyTimes = rXAnimate->getKeyTimes();
    if (!aKeyTimes.hasElements())
        return;
    const Sequence<Any> aValues = rXAnimate->getValues();
    const OUString& sFormula = rXAnimate->getFormula();
    const OUString& rAttributeName = rXAnimate->getAttributeName();

    SAL_INFO("sd.eppt", "animate values, formula: " << sFormula.toUtf8());

    assert(aValues.getLength() == aKeyTimes.getLength());

    pFS->startElementNS(XML_p, XML_tavLst);

    for (int i = 0; i < aKeyTimes.getLength(); i++)
    {
        SAL_INFO("sd.eppt", "animate value " << i << ": " << aKeyTimes[i]);
        if (aValues[i].hasValue())
        {
            pFS->startElementNS(XML_p, XML_tav, XML_fmla,
                                sax_fastparser::UseIf(sFormula, !sFormula.isEmpty()), XML_tm,
                                OString::number(static_cast<sal_Int32>(aKeyTimes[i] * 100000.0)));
            pFS->startElementNS(XML_p, XML_val);
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

// Write condition list ( either prevCondlst or nextCondlst ) of Seq.
void WriteAnimationCondListForSeq(const FSHelperPtr& pFS, sal_Int32 nToken)
{
    const char* pEvent = (nToken == XML_prevCondLst) ? "onPrev" : "onNext";

    pFS->startElementNS(XML_p, nToken);
    pFS->startElementNS(XML_p, XML_cond, XML_evt, pEvent);
    pFS->startElementNS(XML_p, XML_tgtEl);
    pFS->singleElementNS(XML_p, XML_sldTgt);
    pFS->endElementNS(XML_p, XML_tgtEl);
    pFS->endElementNS(XML_p, XML_cond);
    pFS->endElementNS(XML_p, nToken);
}

void WriteAnimationAttributeName(const FSHelperPtr& pFS, const OUString& rAttributeName)
{
    if (rAttributeName.isEmpty())
        return;

    pFS->startElementNS(XML_p, XML_attrNameLst);

    SAL_INFO("sd.eppt", "write attribute name: " << rAttributeName.toUtf8());

    if (rAttributeName == "X;Y")
    {
        pFS->startElementNS(XML_p, XML_attrName);
        pFS->writeEscaped("ppt_x");
        pFS->endElementNS(XML_p, XML_attrName);

        pFS->startElementNS(XML_p, XML_attrName);
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
            pFS->startElementNS(XML_p, XML_attrName);
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

/// extract ooxml node type from a XAnimationNode.
sal_Int32 extractNodeType(const Reference<XAnimationNode>& rXNode)
{
    sal_Int16 nType = rXNode->getType();
    sal_Int32 xmlNodeType = -1;
    switch (nType)
    {
        case AnimationNodeType::ITERATE:
        case AnimationNodeType::PAR:
            xmlNodeType = XML_par;
            break;
        case AnimationNodeType::SEQ:
            xmlNodeType = XML_seq;
            break;
        case AnimationNodeType::ANIMATE:
            xmlNodeType = XML_anim;
            break;
        case AnimationNodeType::ANIMATEMOTION:
            xmlNodeType = XML_animMotion;
            break;
        case AnimationNodeType::ANIMATETRANSFORM:
        {
            Reference<XAnimateTransform> xTransform(rXNode, UNO_QUERY);
            if (xTransform.is())
            {
                if (xTransform->getTransformType() == AnimationTransformType::SCALE)
                    xmlNodeType = XML_animScale;
                else if (xTransform->getTransformType() == AnimationTransformType::ROTATE)
                    xmlNodeType = XML_animRot;
            }
            break;
        }
        case AnimationNodeType::ANIMATECOLOR:
            xmlNodeType = XML_animClr;
            break;
        case AnimationNodeType::SET:
            xmlNodeType = XML_set;
            break;
        case AnimationNodeType::TRANSITIONFILTER:
            xmlNodeType = XML_animEffect;
            break;
        case AnimationNodeType::COMMAND:
            xmlNodeType = XML_cmd;
            break;
        case AnimationNodeType::AUDIO:
            xmlNodeType = XML_audio;
            break;
        default:
            SAL_WARN("sd.eppt", "unhandled animation node: " << nType);
            break;
    }
    return xmlNodeType;
}

/// Convert AnimationRestart to ST_TLTimeNodeRestartType value.
const char* convertAnimationRestart(sal_Int16 nRestart)
{
    const char* pRestart = nullptr;
    switch (nRestart)
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
    return pRestart;
}

/// Convert EffectNodeType to ST_TLTimeNodeType
const char* convertEffectNodeType(sal_Int16 nType)
{
    const char* pNodeType = nullptr;
    switch (nType)
    {
        case EffectNodeType::TIMING_ROOT:
            pNodeType = "tmRoot";
            break;
        case EffectNodeType::MAIN_SEQUENCE:
            pNodeType = "mainSeq";
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
    return pNodeType;
}

/// Convert EffectPresetClass to ST_TLTimeNodePresetClassType
const char* convertEffectPresetClass(sal_Int16 nPresetClass)
{
    const char* pPresetClass = nullptr;
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
    return pPresetClass;
}

/// convert AnimationFill to ST_TLTimeNodeFillType.
const char* convertAnimationFill(sal_Int16 nFill)
{
    const char* pFill = nullptr;
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
    return pFill;
}

/// Convert TextAnimationType to ST_IterateType.
const char* convertTextAnimationType(sal_Int16 nType)
{
    const char* sType = nullptr;
    switch (nType)
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
    return sType;
}

class PPTXAnimationExport
{
    void WriteAnimationNode(const NodeContextPtr& pContext);
    void WriteAnimationNodeAnimate(sal_Int32 nXmlNodeType);
    void WriteAnimationNodeAnimateInside(bool bSimple, bool bWriteTo = true);
    void WriteAnimationNodeSeq();
    void WriteAnimationNodeEffect();
    void WriteAnimationNodeCommand();
    /// Handles XAudio nodes, used for both video and audio.
    void WriteAnimationNodeMedia();
    void WriteAnimationNodeCommonPropsStart();
    void WriteAnimationTarget(const Any& rTarget);
    void WriteAnimationCondList(const std::vector<Cond>& rList, sal_Int32 nToken);
    void WriteAnimationCond(const Cond& rCond);
    const Reference<XAnimationNode>& getCurrentNode() const;

    PowerPointExport& mrPowerPointExport;
    const FSHelperPtr& mpFS;
    const NodeContext* mpContext;

    std::map<Reference<XAnimationNode>, sal_Int32> maAnimationNodeIdMap;
    sal_Int32 GetNextAnimationNodeId(const Reference<XAnimationNode>& rNode);
    sal_Int32 GetAnimationNodeId(const Reference<XAnimationNode>& rNode);

public:
    PPTXAnimationExport(PowerPointExport& rExport, const FSHelperPtr& pFS);
    void WriteAnimations(const Reference<XDrawPage>& rXDrawPage);
};

/// Returns if rURL has an extension which is an audio format.
bool IsAudioURL(std::u16string_view rURL)
{
    return o3tl::endsWithIgnoreAsciiCase(rURL, ".wav")
           || o3tl::endsWithIgnoreAsciiCase(rURL, ".m4a");
}

/// Returns if rURL has an extension which is a video format.
bool IsVideoURL(std::u16string_view rURL) { return o3tl::endsWithIgnoreAsciiCase(rURL, ".mp4"); }
}

namespace oox::core
{
void WriteAnimations(const FSHelperPtr& pFS, const Reference<XDrawPage>& rXDrawPage,
                     PowerPointExport& rExport)
{
    PPTXAnimationExport aAnimationExport(rExport, pFS);
    aAnimationExport.WriteAnimations(rXDrawPage);
}
}

PPTXAnimationExport::PPTXAnimationExport(PowerPointExport& rExport, const FSHelperPtr& pFS)
    : mrPowerPointExport(rExport)
    , mpFS(pFS)
    , mpContext(nullptr)
{
}

const Reference<XAnimationNode>& PPTXAnimationExport::getCurrentNode() const
{
    assert(mpContext);
    return mpContext->getNode();
}

void PPTXAnimationExport::WriteAnimationTarget(const Any& rTarget)
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

    if (!rXShape.is())
        return;

    sal_Int32 nShapeID = mrPowerPointExport.GetShapeID(rXShape);

    mpFS->startElementNS(XML_p, XML_tgtEl);
    mpFS->startElementNS(XML_p, XML_spTgt, XML_spid, OString::number(nShapeID));
    if (bParagraphTarget)
    {
        mpFS->startElementNS(XML_p, XML_txEl);
        mpFS->singleElementNS(XML_p, XML_pRg, XML_st, OString::number(nParagraph), XML_end,
                              OString::number(nParagraph));
        mpFS->endElementNS(XML_p, XML_txEl);
    }
    mpFS->endElementNS(XML_p, XML_spTgt);
    mpFS->endElementNS(XML_p, XML_tgtEl);
}

void PPTXAnimationExport::WriteAnimationCondList(const std::vector<Cond>& rList, sal_Int32 nToken)
{
    if (rList.size() > 0)
    {
        mpFS->startElementNS(XML_p, nToken);

        for (const Cond& rCond : rList)
            WriteAnimationCond(rCond);

        mpFS->endElementNS(XML_p, nToken);
    }
}

void PPTXAnimationExport::WriteAnimationCond(const Cond& rCond)
{
    if (rCond.mpEvent)
    {
        sal_Int32 nId = -1;
        if (rCond.mxShape.is())
        {
            mpFS->startElementNS(XML_p, XML_cond, XML_delay, rCond.getDelay(), XML_evt,
                                 rCond.mpEvent);
            WriteAnimationTarget(Any(rCond.mxShape));
            mpFS->endElementNS(XML_p, XML_cond);
        }
        else if (rCond.mxNode.is() && (nId = GetAnimationNodeId(rCond.mxNode)) != -1)
        {
            mpFS->startElementNS(XML_p, XML_cond, XML_delay, rCond.getDelay(), XML_evt,
                                 rCond.mpEvent);
            mpFS->singleElementNS(XML_p, XML_tn, XML_val, OString::number(nId));
            mpFS->endElementNS(XML_p, XML_cond);
        }
        else
        {
            mpFS->singleElementNS(XML_p, XML_cond, XML_delay, rCond.getDelay(), XML_evt,
                                  rCond.mpEvent);
        }
    }
    else
        mpFS->singleElementNS(XML_p, XML_cond, XML_delay, rCond.getDelay());
}

void PPTXAnimationExport::WriteAnimationNodeAnimate(sal_Int32 nXmlNodeType)
{
    const Reference<XAnimationNode>& rXNode = getCurrentNode();
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

        mpFS->startElementNS(XML_p, nXmlNodeType, XML_origin, "layout", XML_path, aPath);
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

        mpFS->startElementNS(XML_p, nXmlNodeType, XML_by, pBy, XML_from, pFrom, XML_to, pTo);
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
        mpFS->startElementNS(XML_p, nXmlNodeType, XML_clrSpc, pColorSpace, XML_dir, pDirection,
                             XML_calcmode, pCalcMode, XML_valueType, pValueType);
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

        mpFS->startElementNS(XML_p, nXmlNodeType, XML_calcmode, pCalcMode, XML_valueType,
                             pValueType, XML_from, sax_fastparser::UseIf(sFrom, !sFrom.isEmpty()),
                             XML_to, sax_fastparser::UseIf(sTo, !sTo.isEmpty()), XML_by,
                             sax_fastparser::UseIf(sBy, !sBy.isEmpty()));
        bTo = sTo.isEmpty() && sFrom.isEmpty() && sBy.isEmpty();
    }

    WriteAnimationNodeAnimateInside(bSimple, bTo);
    mpFS->endElementNS(XML_p, nXmlNodeType);
}

void PPTXAnimationExport::WriteAnimationNodeAnimateInside(bool bSimple, bool bWriteTo)
{
    const Reference<XAnimationNode>& rXNode = getCurrentNode();
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

    mpFS->startElementNS(XML_p, XML_cBhvr, XML_additive, pAdditive);
    WriteAnimationNodeCommonPropsStart();

    Reference<XIterateContainer> xIterate(rXNode->getParent(), UNO_QUERY);
    WriteAnimationTarget(xIterate.is() ? xIterate->getTarget() : rXAnimate->getTarget());

    Reference<XAnimateTransform> xTransform(rXNode, UNO_QUERY);

    // The attribute name of AnimateTransform is "Transform", we have to fix it.
    OUString sNewAttr;
    if (xTransform.is() && xTransform->getTransformType() == AnimationTransformType::ROTATE)
        sNewAttr = "Rotate";

    WriteAnimationAttributeName(mpFS, xTransform.is() ? sNewAttr : rXAnimate->getAttributeName());

    mpFS->endElementNS(XML_p, XML_cBhvr);
    WriteAnimateValues(mpFS, rXAnimate);

    Reference<XAnimateColor> xColor(rXNode, UNO_QUERY);

    if (xColor.is())
    {
        WriteAnimateColorColor(mpFS, xColor->getBy(), XML_by);
        WriteAnimateColorColor(mpFS, xColor->getFrom(), XML_from);
        WriteAnimateColorColor(mpFS, xColor->getTo(), XML_to);
    }
    else if (xTransform.is() && xTransform->getTransformType() == AnimationTransformType::SCALE)
    {
        WriteAnimationProperty(mpFS, rXAnimate->getBy(), XML_by);
        WriteAnimationProperty(mpFS, rXAnimate->getFrom(), XML_from);
        WriteAnimationProperty(mpFS, rXAnimate->getTo(), XML_to);
    }
    else if (bWriteTo)
        WriteAnimateTo(mpFS, rXAnimate->getTo(), rXAnimate->getAttributeName());
}

void PPTXAnimationExport::WriteAnimationNodeCommonPropsStart()
{
    const Reference<XAnimationNode>& rXNode = getCurrentNode();
    std::optional<OString> sDuration;
    std::optional<OString> sRepeatCount;
    const char* pRestart = nullptr;
    const char* pNodeType = nullptr;
    const char* pPresetClass = nullptr;
    const char* pFill = nullptr;
    double fDuration = 0;
    double fRepeatCount = 0;
    Any aAny;
    assert(mpContext);

    aAny = rXNode->getDuration();
    if (aAny.hasValue())
    {
        Timing eTiming;

        if (aAny >>= eTiming)
        {
            if (eTiming == Timing_INDEFINITE)
                sDuration = "indefinite";
        }
        else
            aAny >>= fDuration;
    }

    pRestart = convertAnimationRestart(rXNode->getRestart());

    sal_Int16 nType = mpContext->getEffectNodeType();
    if (nType != -1)
    {
        pNodeType = convertEffectNodeType(nType);
        if (nType == EffectNodeType::TIMING_ROOT)
        {
            if (!sDuration)
                sDuration = "indefinite";
            if (!pRestart)
                pRestart = "never";
        }
        else if (nType == EffectNodeType::MAIN_SEQUENCE)
        {
            sDuration = "indefinite";
        }
    }

    if (fDuration != 0)
        sDuration = OString::number(static_cast<sal_Int32>(fDuration * 1000.0));

    sal_uInt32 nPresetClass = mpContext->getEffectPresetClass();
    if (nPresetClass != EffectPresetClass::CUSTOM)
        pPresetClass = convertEffectPresetClass(nPresetClass);

    sal_uInt32 nPresetId = 0;
    bool bPresetId = false;
    const OUString& rPresetId = mpContext->getEffectPresetId();
    if (rPresetId.getLength() > 0)
    {
        nPresetId = AnimationExporter::GetPresetID(rPresetId, nPresetClass, bPresetId);
        bPresetId = true;
    }

    sal_uInt32 nPresetSubType = 0;
    bool bPresetSubType = false;
    const OUString& sPresetSubType = mpContext->getEffectPresetSubType();
    if (sPresetSubType.getLength() > 0)
    {
        nPresetSubType
            = AnimationExporter::TranslatePresetSubType(nPresetClass, nPresetId, sPresetSubType);
        bPresetSubType = true;
    }

    if (nType != EffectNodeType::TIMING_ROOT && nType != EffectNodeType::MAIN_SEQUENCE)
    {
        // it doesn't seem to work right on root and mainseq nodes
        sal_Int16 nFill = AnimationExporter::GetFillMode(rXNode, AnimationFill::AUTO);
        pFill = convertAnimationFill(nFill);
    }

    bool bAutoReverse = rXNode->getAutoReverse();

    aAny = rXNode->getRepeatCount();
    if (aAny.hasValue())
    {
        Timing eTiming;

        if (aAny >>= eTiming)
        {
            if (eTiming == Timing_INDEFINITE)
                sRepeatCount = "indefinite";
        }
        else
            aAny >>= fRepeatCount;
    }

    if (fRepeatCount != 0)
        sRepeatCount = OString::number(static_cast<sal_Int32>(fRepeatCount * 1000.0));

    mpFS->startElementNS(
        XML_p, XML_cTn, XML_id, OString::number(GetNextAnimationNodeId(rXNode)), XML_dur, sDuration,
        XML_autoRev, sax_fastparser::UseIf("1", bAutoReverse), XML_restart, pRestart, XML_nodeType,
        pNodeType, XML_fill, pFill, XML_presetClass, pPresetClass, XML_presetID,
        sax_fastparser::UseIf(OString::number(nPresetId), bPresetId), XML_presetSubtype,
        sax_fastparser::UseIf(OString::number(nPresetSubType), bPresetSubType), XML_repeatCount,
        sRepeatCount);

    WriteAnimationCondList(mpContext->getBeginCondList(), XML_stCondLst);
    WriteAnimationCondList(mpContext->getEndCondList(), XML_endCondLst);

    if (rXNode->getType() == AnimationNodeType::ITERATE)
    {
        Reference<XIterateContainer> xIterate(rXNode, UNO_QUERY);
        if (xIterate.is())
        {
            const char* sType = convertTextAnimationType(xIterate->getIterateType());

            mpFS->startElementNS(XML_p, XML_iterate, XML_type, sType);
            mpFS->singleElementNS(XML_p, XML_tmAbs, XML_val,
                                  OString::number(xIterate->getIterateInterval() * 1000));
            mpFS->endElementNS(XML_p, XML_iterate);
        }
    }

    const std::vector<NodeContextPtr>& aChildNodes = mpContext->getChildNodes();
    if (!aChildNodes.empty())
    {
        bool bSubTnLst = false;
        mpFS->startElementNS(XML_p, XML_childTnLst);
        for (const NodeContextPtr& pChildContext : aChildNodes)
        {
            if (pChildContext->isValid())
            {
                if (pChildContext->isOnSubTnLst())
                    bSubTnLst = true;
                else
                    WriteAnimationNode(pChildContext);
            }
        }
        mpFS->endElementNS(XML_p, XML_childTnLst);

        if (bSubTnLst)
        {
            mpFS->startElementNS(XML_p, XML_subTnLst);
            for (const NodeContextPtr& pChildContext : aChildNodes)
            {
                if (pChildContext->isValid() && pChildContext->isOnSubTnLst())
                    WriteAnimationNode(pChildContext);
            }
            mpFS->endElementNS(XML_p, XML_subTnLst);
        }
    }
    mpFS->endElementNS(XML_p, XML_cTn);
}

void PPTXAnimationExport::WriteAnimationNodeSeq()
{
    SAL_INFO("sd.eppt", "write animation node SEQ");

    mpFS->startElementNS(XML_p, XML_seq);

    WriteAnimationNodeCommonPropsStart();

    WriteAnimationCondListForSeq(mpFS, XML_prevCondLst);
    WriteAnimationCondListForSeq(mpFS, XML_nextCondLst);

    mpFS->endElementNS(XML_p, XML_seq);
}

void PPTXAnimationExport::WriteAnimationNodeEffect()
{
    SAL_INFO("sd.eppt", "write animation node FILTER");
    Reference<XTransitionFilter> xFilter(getCurrentNode(), UNO_QUERY);
    if (xFilter.is())
    {
        const char* pFilter = ::ppt::AnimationExporter::FindTransitionName(
            xFilter->getTransition(), xFilter->getSubtype(), xFilter->getDirection());
        const char* pMode = xFilter->getMode() ? "in" : "out";
        mpFS->startElementNS(XML_p, XML_animEffect, XML_filter, pFilter, XML_transition, pMode);

        WriteAnimationNodeAnimateInside(false);

        mpFS->endElementNS(XML_p, XML_animEffect);
    }
}

void PPTXAnimationExport::WriteAnimationNodeCommand()
{
    SAL_INFO("sd.eppt", "write animation node COMMAND");
    Reference<XCommand> xCommand(getCurrentNode(), UNO_QUERY);
    if (!xCommand.is())
        return;

    const char* pType = "call";
    OString aCommand;
    switch (xCommand->getCommand())
    {
        case EffectCommands::VERB:
            pType = "verb";
            aCommand = "1"_ostr; /* FIXME hardcoded viewing */
            break;
        case EffectCommands::PLAY:
        {
            aCommand = "play"_ostr;
            uno::Sequence<beans::NamedValue> aParamSeq;
            xCommand->getParameter() >>= aParamSeq;
            comphelper::SequenceAsHashMap aMap(aParamSeq);
            auto it = aMap.find(u"MediaTime"_ustr);
            if (it != aMap.end())
            {
                double fMediaTime = 0;
                it->second >>= fMediaTime;
                // PowerPoint represents 0 as 0.0, so just use a single decimal.
                OString aMediaTime
                    = rtl::math::doubleToString(fMediaTime, rtl_math_StringFormat_F, 1, '.');
                aCommand += "From(" + aMediaTime + ")";
            }
            break;
        }
        case EffectCommands::TOGGLEPAUSE:
            aCommand = "togglePause"_ostr;
            break;
        case EffectCommands::STOP:
            aCommand = "stop"_ostr;
            break;
        default:
            SAL_WARN("sd.eppt", "unknown command: " << xCommand->getCommand());
            break;
    }

    mpFS->startElementNS(XML_p, XML_cmd, XML_type, pType, XML_cmd, aCommand.getStr());

    WriteAnimationNodeAnimateInside(false);
    mpFS->startElementNS(XML_p, XML_cBhvr);
    WriteAnimationNodeCommonPropsStart();
    WriteAnimationTarget(xCommand->getTarget());
    mpFS->endElementNS(XML_p, XML_cBhvr);

    mpFS->endElementNS(XML_p, XML_cmd);
}

void PPTXAnimationExport::WriteAnimationNodeMedia()
{
    SAL_INFO("sd.eppt", "write animation node media");
    Reference<XAudio> xAudio(getCurrentNode(), UNO_QUERY);

    OUString sUrl;
    uno::Reference<drawing::XShape> xShape;
    OUString sRelId;
    OUString sName;

    if (!xAudio.is())
    {
        return;
    }

    bool bValid = false;
    if ((xAudio->getSource() >>= sUrl) && !sUrl.isEmpty() && IsAudioURL(sUrl))
    {
        bValid = true;
    }

    bool bVideo = false;
    if (!bValid)
    {
        if (xAudio->getSource() >>= xShape)
        {
            uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
            bool bHasMediaURL
                = xShapeProps->getPropertySetInfo()->hasPropertyByName(u"MediaURL"_ustr);
            if (bHasMediaURL && (xShapeProps->getPropertyValue(u"MediaURL"_ustr) >>= sUrl))
            {
                bVideo = IsVideoURL(sUrl);
                bValid = IsAudioURL(sUrl) || bVideo;
            }
        }
    }

    if (!bValid)
        return;

    if (!xShape.is())
    {
        mrPowerPointExport.embedEffectAudio(mpFS, sUrl, sRelId, sName);
    }

    if (bVideo)
    {
        mpFS->startElementNS(XML_p, XML_video);
        mpFS->startElementNS(XML_p, XML_cMediaNode);
    }
    else
    {
        bool bNarration = xAudio->getNarration();
        mpFS->startElementNS(XML_p, XML_audio, XML_isNarration, bNarration ? "1" : "0");
        bool bHideDuringShow = xAudio->getHideDuringShow();
        mpFS->startElementNS(XML_p, XML_cMediaNode, XML_showWhenStopped,
                             bHideDuringShow ? "0" : "1");
    }

    animations::Timing eTiming{};
    bool bLooping
        = (xAudio->getRepeatCount() >>= eTiming) && eTiming == animations::Timing_INDEFINITE;
    if (bVideo && bLooping)
    {
        mpFS->startElementNS(XML_p, XML_cTn, XML_repeatCount, "indefinite");
    }
    else
    {
        mpFS->startElementNS(XML_p, XML_cTn);
    }
    WriteAnimationCondList(mpContext->getBeginCondList(), XML_stCondLst);
    WriteAnimationCondList(mpContext->getEndCondList(), XML_endCondLst);
    mpFS->endElementNS(XML_p, XML_cTn);

    mpFS->startElementNS(XML_p, XML_tgtEl);
    if (xShape.is())
    {
        sal_Int32 nShapeID = mrPowerPointExport.GetShapeID(xShape);
        mpFS->singleElementNS(XML_p, XML_spTgt, XML_spid, OString::number(nShapeID));
    }
    else
    {
        mpFS->singleElementNS(XML_p, XML_sndTgt, FSNS(XML_r, XML_embed),
                              sax_fastparser::UseIf(sRelId, !sRelId.isEmpty()), XML_name,
                              sax_fastparser::UseIf(sName, !sUrl.isEmpty()));
    }
    mpFS->endElementNS(XML_p, XML_tgtEl);

    mpFS->endElementNS(XML_p, XML_cMediaNode);
    if (bVideo)
    {
        mpFS->endElementNS(XML_p, XML_video);
    }
    else
    {
        mpFS->endElementNS(XML_p, XML_audio);
    }
}

void PPTXAnimationExport::WriteAnimationNode(const NodeContextPtr& pContext)
{
    const NodeContext* pSavedContext = mpContext;
    mpContext = pContext.get();

    const Reference<XAnimationNode>& rXNode = getCurrentNode();

    SAL_INFO("sd.eppt", "export node type: " << rXNode->getType());
    sal_Int32 xmlNodeType = extractNodeType(rXNode);

    switch (xmlNodeType)
    {
        case XML_par:
            mpFS->startElementNS(XML_p, xmlNodeType);
            WriteAnimationNodeCommonPropsStart();
            mpFS->endElementNS(XML_p, xmlNodeType);
            break;
        case XML_seq:
            WriteAnimationNodeSeq();
            break;
        case XML_animScale:
        case XML_animRot:
        case XML_anim:
        case XML_animMotion:
        case XML_animClr:
        case XML_set:
            WriteAnimationNodeAnimate(xmlNodeType);
            break;
        case XML_animEffect:
            WriteAnimationNodeEffect();
            break;
        case XML_cmd:
            WriteAnimationNodeCommand();
            break;
        case XML_audio:
            WriteAnimationNodeMedia();
            break;
        default:
            SAL_WARN("sd.eppt", "export ooxml node type: " << xmlNodeType);
            break;
    }

    mpContext = pSavedContext;
}

void PPTXAnimationExport::WriteAnimations(const Reference<XDrawPage>& rXDrawPage)
{
    Reference<XAnimationNodeSupplier> xNodeSupplier(rXDrawPage, UNO_QUERY);
    if (!xNodeSupplier.is())
        return;

    const Reference<XAnimationNode> xNode(xNodeSupplier->getAnimationNode());
    if (!xNode.is())
        return;

    Reference<XEnumerationAccess> xEnumerationAccess(xNode, UNO_QUERY);
    if (!xEnumerationAccess.is())
        return;

    Reference<XEnumeration> xEnumeration = xEnumerationAccess->createEnumeration();
    if (!(xEnumeration.is() && xEnumeration->hasMoreElements()))
        return;

    auto pNodeContext = std::make_unique<NodeContext>(xNode, false, false);
    if (pNodeContext->isValid())
    {
        mpFS->startElementNS(XML_p, XML_timing);
        mpFS->startElementNS(XML_p, XML_tnLst);

        WriteAnimationNode(pNodeContext);

        mpFS->endElementNS(XML_p, XML_tnLst);
        mpFS->endElementNS(XML_p, XML_timing);
    }
}

sal_Int32 PPTXAnimationExport::GetNextAnimationNodeId(const Reference<XAnimationNode>& xNode)
{
    sal_Int32 nId = mrPowerPointExport.GetNextAnimationNodeID();
    maAnimationNodeIdMap[xNode] = nId;
    return nId;
}

sal_Int32 PPTXAnimationExport::GetAnimationNodeId(const Reference<XAnimationNode>& xNode)
{
    sal_Int32 nId = -1;
    const auto& aIter = maAnimationNodeIdMap.find(xNode);
    if (aIter != maAnimationNodeIdMap.end())
    {
        nId = aIter->second;
    }
    return nId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
