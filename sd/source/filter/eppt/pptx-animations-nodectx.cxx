/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pptx-animations-nodectx.hxx"
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <o3tl/any.hxx>
#include <o3tl/string_view.hxx>

using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::drawing::XShape;

using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::uno;

namespace oox::core
{
namespace
{
bool isValidTarget(const Any& rTarget)
{
    Reference<XShape> xShape;

    if ((rTarget >>= xShape) && xShape.is())
        return true;

    ParagraphTarget aParagraphTarget;

    return (rTarget >>= aParagraphTarget) && aParagraphTarget.Shape.is();
}

bool IsAudioURL(std::u16string_view rURL)
{
    return o3tl::endsWithIgnoreAsciiCase(rURL, ".wav")
           || o3tl::endsWithIgnoreAsciiCase(rURL, ".m4a");
}

/// Returns if rURL has an extension which is a video format.
bool IsVideoURL(std::u16string_view rURL) { return o3tl::endsWithIgnoreAsciiCase(rURL, ".mp4"); }

bool initCondList(const Any& rAny, std::vector<Cond>& rList, bool bIsMainSeqChild)
{
    bool bEventTrigger = false;
    if (!rAny.hasValue())
        return false;

    Sequence<Any> aCondSeq;
    if (rAny >>= aCondSeq)
    {
        for (const auto& rCond : aCondSeq)
        {
            Cond aCond(rCond, bIsMainSeqChild);
            if (aCond.isValid())
            {
                rList.push_back(aCond);
                if (aCond.mpEvent)
                    bEventTrigger = true;
            }
        }
    }
    else
    {
        Cond aCond(rAny, bIsMainSeqChild);
        if (aCond.isValid())
        {
            rList.push_back(aCond);
            if (aCond.mpEvent)
                bEventTrigger = true;
        }
    }
    return bEventTrigger;
}
}

NodeContext::NodeContext(const Reference<XAnimationNode>& xNode, bool bMainSeqChild,
                         bool bIsIterateChild)
    : mxNode(xNode)
    , mbValid(true)
    , mbOnSubTnLst(false)
    , mnEffectNodeType(-1)
    , mnEffectPresetClass(css::presentation::EffectPresetClass::CUSTOM)
{
    assert(xNode.is());

    initUserData();

    initValid(initChildNodes(), bIsIterateChild);

    // Put event triggered Audio time nodes to SubTnLst.
    // Add other types of nodes once we find more test cases.
    mbOnSubTnLst = initCondList(getNodeForCondition()->getBegin(), maBeginCondList, bMainSeqChild)
                   && mxNode->getType() == AnimationNodeType::AUDIO;

    initCondList(getNodeForCondition()->getEnd(), maEndCondList, bMainSeqChild);
}

void NodeContext::initUserData()
{
    assert(mxNode.is());

    Sequence<NamedValue> aUserData = mxNode->getUserData();
    for (const NamedValue& rProp : aUserData)
    {
        if (rProp.Name == "node-type")
        {
            rProp.Value >>= mnEffectNodeType;
        }
        else if (rProp.Name == "preset-class")
        {
            rProp.Value >>= mnEffectPresetClass;
        }
        else if (rProp.Name == "preset-id")
        {
            rProp.Value >>= msEffectPresetId;
        }
        else if (rProp.Name == "preset-sub-type")
        {
            rProp.Value >>= msEffectPresetSubType;
        }
    }
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
        Reference<XShape> xShape;
        mbValid = false;
        if (xAudio.is())
        {
            if (xAudio->getSource() >>= sURL)
            {
                mbValid = IsAudioURL(sURL);
            }
            else if (xAudio->getSource() >>= xShape)
            {
                Reference<XPropertySet> xShapeProps(xShape, UNO_QUERY);
                bool bHasMediaURL
                    = xShapeProps->getPropertySetInfo()->hasPropertyByName(u"MediaURL"_ustr);
                if (bHasMediaURL && (xShapeProps->getPropertyValue(u"MediaURL"_ustr) >>= sURL))
                {
                    mbValid = IsAudioURL(sURL) || IsVideoURL(sURL);
                }
            }
        }
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
        Reference<XEnumeration> xEnumeration = xEnumerationAccess->createEnumeration();
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

const Reference<XAnimationNode>& NodeContext::getNodeForCondition() const
{
    const bool bParent
        = (mnEffectNodeType != EffectNodeType::INTERACTIVE_SEQUENCE || maChildNodes.empty());
    const Reference<XAnimationNode>& rNode = bParent ? mxNode : maChildNodes[0]->getNode();
    return rNode;
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
