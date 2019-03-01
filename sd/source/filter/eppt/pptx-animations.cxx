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
#include <oox/token/tokens.hxx>
#include "epptooxml.hxx"
#include <sax/fshelper.hxx>
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
#include <oox/export/utils.hxx>
#include <oox/ppt/pptfilterhelpers.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include "pptexanimations.hxx"
#include "pptx-animations.hxx"
#include "../ppt/pptanimations.hxx"

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
            pFS->singleElementNS(XML_p, nToken, XML_x, OString::number(x * 100000).getStr(), XML_y,
                                 OString::number(y * 100000).getStr(), FSEND);
        }
        return;
    }

    sal_uInt32 nRgb;
    double fDouble;

    TypeClass aClass = rAny.getValueType().getTypeClass();
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

void WriteAnimateColorColor(const FSHelperPtr& pFS, const Any& rAny, sal_Int32 nToken)
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
    if (!(rAny >>= aHSL))
        return;

    pFS->startElementNS(XML_p, nToken, FSEND);

    if (nToken == XML_by)
    {
        // CT_TLByHslColorTransform
        pFS->singleElementNS(XML_p, XML_hsl, XML_h, I32S(aHSL[0] * 60000), // ST_Angel
                             XML_s, I32S(aHSL[1] * 100000), XML_l, I32S(aHSL[2] * 100000), FSEND);
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

    SAL_INFO("sd.eppt", "to attribute name: " << USS(rAttributeName));

    WriteAnimationProperty(pFS, AnimationExporter::convertAnimateValue(rValue, rAttributeName),
                           XML_to);
}

void WriteAnimateValues(const FSHelperPtr& pFS, const Reference<XAnimate>& rXAnimate)
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

// Write condition list ( either prevCondlst or nextCondlst ) of Seq.
void WriteAnimationCondListForSeq(const FSHelperPtr& pFS, sal_Int32 nToken)
{
    const char* pEvent = (nToken == XML_prevCondLst) ? "onPrev" : "onNext";

    pFS->startElementNS(XML_p, nToken, FSEND);
    pFS->startElementNS(XML_p, XML_cond, XML_evt, pEvent, FSEND);
    pFS->startElementNS(XML_p, XML_tgtEl, FSEND);
    pFS->singleElementNS(XML_p, XML_sldTgt, FSEND);
    pFS->endElementNS(XML_p, XML_tgtEl);
    pFS->endElementNS(XML_p, XML_cond);
    pFS->endElementNS(XML_p, nToken);
}

const char* convertEventTrigger(sal_Int16 nTrigger)
{
    const char* pEvent = nullptr;
    switch (nTrigger)
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
    return pEvent;
}

void WriteAnimationAttributeName(const FSHelperPtr& pFS, const OUString& rAttributeName)
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

bool isValidTarget(const Any& rTarget)
{
    Reference<XShape> xShape;

    if ((rTarget >>= xShape) && xShape.is())
        return true;

    ParagraphTarget aParagraphTarget;

    return (rTarget >>= aParagraphTarget) && aParagraphTarget.Shape.is();
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

class NodeContext;

typedef std::unique_ptr<NodeContext> NodeContextPtr;

class NodeContext
{
    const Reference<XAnimationNode> mxNode;
    const bool mbMainSeqChild;

    std::vector<NodeContextPtr> maChildNodes;
    // if the node has valid target or contains at least one valid target.
    bool mbValid;

    // Attributes initialized from mxNode->getUserData().
    sal_Int16 mnEffectNodeType;
    sal_Int16 mnEffectPresetClass;
    OUString msEffectPresetId;
    OUString msEffectPresetSubType;

    /// constructor helper for initializing user datas.
    void initUserData();

    /// constructor helper to initialize maChildNodes.
    /// return true if at least one childnode is valid.
    bool initChildNodes();

    /// constructor helper to initialize mbValid
    void initValid(bool bHasValidChild, bool bIsIterateChild);

public:
    NodeContext(const Reference<XAnimationNode>& xNode, bool bMainSeqChild, bool bIsIterateChild);
    const Reference<XAnimationNode>& getNode() const { return mxNode; }
    bool isMainSeqChild() const { return mbMainSeqChild; }
    sal_Int16 getEffectNodeType() const { return mnEffectNodeType; }
    sal_Int16 getEffectPresetClass() const { return mnEffectPresetClass; }
    const OUString& getEffectPresetId() const { return msEffectPresetId; }
    const OUString& getEffectPresetSubType() const { return msEffectPresetSubType; }
    bool isValid() const { return mbValid; }
    const std::vector<NodeContextPtr>& getChildNodes() const { return maChildNodes; };
    Any getCondition(bool bBegin) const;
};

struct Cond
{
    OString msDelay;
    const char* mpEvent;
    Reference<XShape> mxShape;

    Cond(const Any& rAny, bool bIsMainSeqChild);

    bool isValid() { return msDelay.getLength() || mpEvent; }
    const char* getDelay() const { return msDelay.getLength() ? msDelay.getStr() : nullptr; }
};

Cond::Cond(const Any& rAny, bool bIsMainSeqChild)
    : mpEvent(nullptr)
{
    bool bHasFDelay = false;
    double fDelay = 0;
    Timing eTiming;
    Event aEvent;

    if (rAny >>= eTiming)
    {
        if (eTiming == Timing_INDEFINITE)
            msDelay = "indefinite";
    }
    else if (rAny >>= aEvent)
    {
        if (aEvent.Trigger == EventTrigger::ON_NEXT && bIsMainSeqChild)
            msDelay = "indefinite";
        else
        {
            mpEvent = convertEventTrigger(aEvent.Trigger);
            aEvent.Source >>= mxShape;

            if (aEvent.Offset >>= fDelay)
                bHasFDelay = true;
        }
    }
    else if (rAny >>= fDelay)
        bHasFDelay = true;

    if (bHasFDelay)
    {
        sal_Int32 nDelay = static_cast<sal_uInt32>(fDelay * 1000.0);
        msDelay = OString::number(nDelay);
    }
}

class PPTXAnimationExport
{
    void WriteAnimationNode(const NodeContextPtr& pContext);
    void WriteAnimationNodeAnimate(sal_Int32 nXmlNodeType);
    void WriteAnimationNodeAnimateInside(bool bSimple, bool bWriteTo = true);
    void WriteAnimationNodeSeq();
    void WriteAnimationNodeEffect();
    void WriteAnimationNodeCommand();
    void WriteAnimationNodeAudio();
    void WriteAnimationNodeCommonPropsStart();
    void WriteAnimationTarget(const Any& rTarget);
    void WriteAnimationCondList(const Any& rAny, sal_Int32 nToken);
    void WriteAnimationCond(const Cond& rCond);
    bool isMainSeqChild();
    const Reference<XAnimationNode>& getCurrentNode();

    PowerPointExport& mrPowerPointExport;
    const FSHelperPtr& mpFS;
    const NodeContext* mpContext;

public:
    PPTXAnimationExport(PowerPointExport& rExport, const FSHelperPtr& pFS);
    void WriteAnimations(const Reference<XDrawPage>& rXDrawPage);
};
}

namespace oox
{
namespace core
{
void WriteAnimations(const FSHelperPtr& pFS, const Reference<XDrawPage>& rXDrawPage,
                     PowerPointExport& rExport)
{
    PPTXAnimationExport aAnimationExport(rExport, pFS);
    aAnimationExport.WriteAnimations(rXDrawPage);
}
}
}

PPTXAnimationExport::PPTXAnimationExport(PowerPointExport& rExport, const FSHelperPtr& pFS)
    : mrPowerPointExport(rExport)
    , mpFS(pFS)
    , mpContext(nullptr)
{
}

bool PPTXAnimationExport::isMainSeqChild()
{
    assert(mpContext);
    return mpContext->isMainSeqChild();
}

const Reference<XAnimationNode>& PPTXAnimationExport::getCurrentNode()
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

    mpFS->startElementNS(XML_p, XML_tgtEl, FSEND);
    mpFS->startElementNS(XML_p, XML_spTgt, XML_spid, I32S(nShapeID), FSEND);
    if (bParagraphTarget)
    {
        mpFS->startElementNS(XML_p, XML_txEl, FSEND);
        mpFS->singleElementNS(XML_p, XML_pRg, XML_st, I32S(nParagraph), XML_end, I32S(nParagraph),
                              FSEND);
        mpFS->endElementNS(XML_p, XML_txEl);
    }
    mpFS->endElementNS(XML_p, XML_spTgt);
    mpFS->endElementNS(XML_p, XML_tgtEl);
}

void PPTXAnimationExport::WriteAnimationCondList(const Any& rAny, sal_Int32 nToken)
{
    if (!rAny.hasValue())
        return;

    std::vector<Cond> aList;

    bool bIsMainSeqChild = isMainSeqChild();

    Sequence<Any> aCondSeq;
    if (rAny >>= aCondSeq)
    {
        for (int i = 0; i < aCondSeq.getLength(); i++)
        {
            Cond aCond(aCondSeq[i], bIsMainSeqChild);
            if (aCond.isValid())
                aList.push_back(aCond);
        }
    }
    else
    {
        Cond aCond(rAny, bIsMainSeqChild);
        if (aCond.isValid())
            aList.push_back(aCond);
    }

    if (aList.size() > 0)
    {
        mpFS->startElementNS(XML_p, nToken, FSEND);

        for (const Cond& rCond : aList)
            WriteAnimationCond(rCond);

        mpFS->endElementNS(XML_p, nToken);
    }
}

void PPTXAnimationExport::WriteAnimationCond(const Cond& rCond)
{
    if (rCond.mpEvent)
    {
        if (rCond.mxShape.is())
        {
            mpFS->startElementNS(XML_p, XML_cond, XML_delay, rCond.getDelay(), XML_evt,
                                 rCond.mpEvent, FSEND);
            WriteAnimationTarget(makeAny(rCond.mxShape));
            mpFS->endElementNS(XML_p, XML_cond);
        }
        else
        {
            mpFS->singleElementNS(XML_p, XML_cond, XML_delay, rCond.getDelay(), XML_evt,
                                  rCond.mpEvent, FSEND);
        }
    }
    else
        mpFS->singleElementNS(XML_p, XML_cond, XML_delay, rCond.getDelay(), FSEND);
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

        mpFS->startElementNS(XML_p, nXmlNodeType, XML_origin, "layout", XML_path,
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

        mpFS->startElementNS(XML_p, nXmlNodeType, XML_by, pBy, XML_from, pFrom, XML_to, pTo, FSEND);
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

        mpFS->startElementNS(XML_p, nXmlNodeType, XML_calcmode, pCalcMode, XML_valueType,
                             pValueType, XML_from, sFrom.getLength() ? USS(sFrom) : nullptr, XML_to,
                             sTo.getLength() ? USS(sTo) : nullptr, XML_by,
                             sBy.getLength() ? USS(sBy) : nullptr, FSEND);
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

    mpFS->startElementNS(XML_p, XML_cBhvr, XML_additive, pAdditive, FSEND);
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
    const char* pDuration = nullptr;
    const char* pRestart = nullptr;
    const char* pNodeType = nullptr;
    const char* pPresetClass = nullptr;
    const char* pFill = nullptr;
    double fDuration = 0;
    Any aAny;
    assert(mpContext);

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

    pRestart = convertAnimationRestart(rXNode->getRestart());

    sal_Int16 nType = mpContext->getEffectNodeType();
    if (nType != -1)
    {
        pNodeType = convertEffectNodeType(nType);
        if (nType == EffectNodeType::TIMING_ROOT)
        {
            if (!pDuration)
                pDuration = "indefinite";
            if (!pRestart)
                pRestart = "never";
        }
        else if (nType == EffectNodeType::MAIN_SEQUENCE)
        {
            pDuration = "indefinite";
        }
    }

    sal_uInt32 nPresetClass = mpContext->getEffectPresetClass();
    if (nPresetClass != DFF_ANIM_PRESS_CLASS_USER_DEFINED)
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

    mpFS->startElementNS(
        XML_p, XML_cTn, XML_id, I64S(mrPowerPointExport.GetNextAnimationNodeID()), XML_dur,
        fDuration != 0 ? I32S(static_cast<sal_Int32>(fDuration * 1000.0)) : pDuration, XML_autoRev,
        bAutoReverse ? "1" : nullptr, XML_restart, pRestart, XML_nodeType, pNodeType, XML_fill,
        pFill, XML_presetClass, pPresetClass, XML_presetID, bPresetId ? I64S(nPresetId) : nullptr,
        XML_presetSubtype, bPresetSubType ? I64S(nPresetSubType) : nullptr, FSEND);

    WriteAnimationCondList(mpContext->getCondition(true), XML_stCondLst);
    WriteAnimationCondList(mpContext->getCondition(false), XML_endCondLst);

    if (rXNode->getType() == AnimationNodeType::ITERATE)
    {
        Reference<XIterateContainer> xIterate(rXNode, UNO_QUERY);
        if (xIterate.is())
        {
            const char* sType = convertTextAnimationType(xIterate->getIterateType());

            mpFS->startElementNS(XML_p, XML_iterate, XML_type, sType, FSEND);
            mpFS->singleElementNS(XML_p, XML_tmAbs, XML_val,
                                  I32S(xIterate->getIterateInterval() * 1000), FSEND);
            mpFS->endElementNS(XML_p, XML_iterate);
        }
    }

    const std::vector<NodeContextPtr>& aChildNodes = mpContext->getChildNodes();
    if (!aChildNodes.empty())
    {
        mpFS->startElementNS(XML_p, XML_childTnLst, FSEND);
        for (const NodeContextPtr& pChildContext : aChildNodes)
        {
            if (pChildContext->isValid())
                WriteAnimationNode(pChildContext);
        }
        mpFS->endElementNS(XML_p, XML_childTnLst);
    }
    mpFS->endElementNS(XML_p, XML_cTn);
}

void PPTXAnimationExport::WriteAnimationNodeSeq()
{
    SAL_INFO("sd.eppt", "write animation node SEQ");

    mpFS->startElementNS(XML_p, XML_seq, FSEND);

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
        mpFS->startElementNS(XML_p, XML_animEffect, XML_filter, pFilter, XML_transition, pMode,
                             FSEND);

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

    mpFS->startElementNS(XML_p, XML_cmd, XML_type, pType, XML_cmd, pCommand, FSEND);

    WriteAnimationNodeAnimateInside(false);
    mpFS->startElementNS(XML_p, XML_cBhvr, FSEND);
    WriteAnimationNodeCommonPropsStart();
    WriteAnimationTarget(xCommand->getTarget());
    mpFS->endElementNS(XML_p, XML_cBhvr);

    mpFS->endElementNS(XML_p, XML_cmd);
}

void PPTXAnimationExport::WriteAnimationNodeAudio()
{
    SAL_INFO("sd.eppt", "write animation node audio");
    Reference<XAudio> xAudio(getCurrentNode(), UNO_QUERY);

    OUString sUrl;
    OUString sRelId;
    OUString sName;

    if (!(xAudio.is() && (xAudio->getSource() >>= sUrl) && !sUrl.isEmpty()
          && sUrl.endsWithIgnoreAsciiCase(".wav")))
        return;

    mrPowerPointExport.embedEffectAudio(mpFS, sUrl, sRelId, sName);

    mpFS->startElementNS(XML_p, XML_audio, FSEND);
    mpFS->startElementNS(XML_p, XML_cMediaNode, FSEND);

    mpFS->startElementNS(XML_p, XML_cTn, FSEND);
    WriteAnimationCondList(mpContext->getCondition(true), XML_stCondLst);
    WriteAnimationCondList(mpContext->getCondition(false), XML_endCondLst);
    mpFS->endElementNS(XML_p, XML_cTn);

    mpFS->startElementNS(XML_p, XML_tgtEl, FSEND);
    mpFS->singleElementNS(XML_p, XML_sndTgt, FSNS(XML_r, XML_embed),
                          sRelId.isEmpty() ? nullptr : USS(sRelId), XML_name,
                          sUrl.isEmpty() ? nullptr : USS(sName), FSEND);
    mpFS->endElementNS(XML_p, XML_tgtEl);

    mpFS->endElementNS(XML_p, XML_cMediaNode);
    mpFS->endElementNS(XML_p, XML_audio);
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
            mpFS->startElementNS(XML_p, xmlNodeType, FSEND);
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
            WriteAnimationNodeAudio();
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

    Reference<XEnumeration> xEnumeration(xEnumerationAccess->createEnumeration(), UNO_QUERY);
    if (!(xEnumeration.is() && xEnumeration->hasMoreElements()))
        return;

    auto pNodeContext = std::make_unique<NodeContext>(xNode, false, false);
    if (pNodeContext->isValid())
    {
        mpFS->startElementNS(XML_p, XML_timing, FSEND);
        mpFS->startElementNS(XML_p, XML_tnLst, FSEND);

        WriteAnimationNode(pNodeContext);

        mpFS->endElementNS(XML_p, XML_tnLst);
        mpFS->endElementNS(XML_p, XML_timing);
    }
}

NodeContext::NodeContext(const Reference<XAnimationNode>& xNode, bool bMainSeqChild,
                         bool bIsIterateChild)
    : mxNode(xNode)
    , mbMainSeqChild(bMainSeqChild)
    , mbValid(true)
    , mnEffectNodeType(-1)
    , mnEffectPresetClass(DFF_ANIM_PRESS_CLASS_USER_DEFINED)
{
    assert(xNode.is());

    initUserData();

    initValid(initChildNodes(), bIsIterateChild);
}

void NodeContext::initUserData()
{
    assert(mxNode.is());

    Sequence<NamedValue> aUserData = mxNode->getUserData();
    const Any* aIndexedData[DFF_ANIM_PROPERTY_ID_COUNT];
    AnimationExporter::GetUserData(aUserData, aIndexedData, sizeof(aIndexedData));

    const Any* pAny = aIndexedData[DFF_ANIM_NODE_TYPE];
    if (pAny)
        *pAny >>= mnEffectNodeType;

    pAny = aIndexedData[DFF_ANIM_PRESET_CLASS];
    if (pAny)
        *pAny >>= mnEffectPresetClass;

    pAny = aIndexedData[DFF_ANIM_PRESET_ID];
    if (pAny)
        *pAny >>= msEffectPresetId;

    pAny = aIndexedData[DFF_ANIM_PRESET_SUB_TYPE];
    if (pAny)
        *pAny >>= msEffectPresetSubType;
}

void NodeContext::initValid(bool bHasValidChild, bool bIsIterateChild)
{
    sal_Int16 nType = mxNode->getType();

    if (nType == AnimationNodeType::ITERATE)
    {
        Reference<XIterateContainer> xIterate(mxNode, UNO_QUERY);
        mbValid = xIterate.is() && (bIsIterateChild || isValidTarget(xIterate->getTarget()))
                  && !maChildNodes.empty();
    }
    else if (nType == AnimationNodeType::COMMAND)
    {
        Reference<XCommand> xCommand(mxNode, UNO_QUERY);
        mbValid = xCommand.is() && (bIsIterateChild || isValidTarget(xCommand->getTarget()));
    }
    else if (nType == AnimationNodeType::PAR || nType == AnimationNodeType::SEQ)
    {
        mbValid = bHasValidChild;
    }
    else if (nType == AnimationNodeType::AUDIO)
    {
        Reference<XAudio> xAudio(mxNode, UNO_QUERY);
        OUString sURL;
        mbValid
            = xAudio.is() && (xAudio->getSource() >>= sURL) && sURL.endsWithIgnoreAsciiCase(".wav");
    }
    else
    {
        Reference<XAnimate> xAnimate(mxNode, UNO_QUERY);
        mbValid = xAnimate.is() && (bIsIterateChild || isValidTarget(xAnimate->getTarget()));
    }
}

bool NodeContext::initChildNodes()
{
    bool bValid = false;
    Reference<XEnumerationAccess> xEnumerationAccess(mxNode, UNO_QUERY);
    if (xEnumerationAccess.is())
    {
        Reference<XEnumeration> xEnumeration(xEnumerationAccess->createEnumeration(), UNO_QUERY);
        bool bIsMainSeq = mnEffectNodeType == EffectNodeType::MAIN_SEQUENCE;
        bool bIsIterateChild = mxNode->getType() == AnimationNodeType::ITERATE;
        if (xEnumeration.is())
        {
            while (xEnumeration->hasMoreElements())
            {
                Reference<XAnimationNode> xChildNode(xEnumeration->nextElement(), UNO_QUERY);
                if (xChildNode.is())
                {
                    auto pChildContext
                        = std::make_unique<NodeContext>(xChildNode, bIsMainSeq, bIsIterateChild);
                    if (pChildContext->isValid())
                        bValid = true;
                    maChildNodes.push_back(std::move(pChildContext));
                }
            }
        }
    }
    return bValid;
}

Any NodeContext::getCondition(bool bBegin) const
{
    const bool bParent
        = (mnEffectNodeType != EffectNodeType::INTERACTIVE_SEQUENCE || maChildNodes.empty());
    const Reference<XAnimationNode>& rNode = bParent ? mxNode : maChildNodes[0]->getNode();

    return bBegin ? rNode->getBegin() : rNode->getEnd();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
