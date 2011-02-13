/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_slideshow.hxx"

#include "debug.hxx"

#if OSL_DEBUG_LEVEL > 1

#include "animationnodes/basecontainernode.hxx"
#include "animationnodes/paralleltimecontainer.hxx"
#include "animationnodes/sequentialtimecontainer.hxx"
#include "animationnodes/animationtransitionfilternode.hxx"
#include "animationnodes/animationaudionode.hxx"
#include "animationnodes/animationcolornode.hxx"
#include "animationnodes/animationcommandnode.hxx"
#include "animationnodes/animationpathmotionnode.hxx"
#include "animationnodes/animationsetnode.hxx"
#include "animationnodes/animationtransformnode.hxx"
#include "animationnodes/propertyanimationnode.hxx"

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/Event.hpp>

#include <cstdio>
#include <cstdarg>

using ::rtl::OUString;
using namespace ::com::sun::star;


namespace slideshow { namespace internal {

namespace {

class NodeContainer : public BaseContainerNode
{
public:
    void ShowChildrenState (void) const;
};




OUString DebugGetDescription (const AnimationNodeSharedPtr& rpNode)
{
    if (::boost::dynamic_pointer_cast<BaseContainerNode>(rpNode))
    {
        // Node is a container.
        if (::boost::dynamic_pointer_cast<ParallelTimeContainer>(rpNode))
            return OUString::createFromAscii("ParallelTimeContainer");
        else if (::boost::dynamic_pointer_cast<SequentialTimeContainer>(rpNode))
            return OUString::createFromAscii("SequentialTimeContainer");
        else
            return OUString::createFromAscii("<unknown container>");
    }
    else if (::boost::dynamic_pointer_cast<AnimationTransitionFilterNode>(rpNode))
        return OUString::createFromAscii("AnimationTransitionFilterNode");
    else if (::boost::dynamic_pointer_cast<AnimationAudioNode>(rpNode))
        return OUString::createFromAscii("AnimationAudioNode");
    else if (::boost::dynamic_pointer_cast<AnimationColorNode>(rpNode))
        return OUString::createFromAscii("AnimationColorNode");
    else if (::boost::dynamic_pointer_cast<AnimationCommandNode>(rpNode))
        return OUString::createFromAscii("AnimationCommandNode");
    else if (::boost::dynamic_pointer_cast<AnimationPathMotionNode>(rpNode))
        return OUString::createFromAscii("AnimationPathMotionNode");
    else if (::boost::dynamic_pointer_cast<AnimationSetNode>(rpNode))
        return OUString::createFromAscii("AnimationSetNode");
    else if (::boost::dynamic_pointer_cast<AnimationTransformNode>(rpNode))
        return OUString::createFromAscii("AnimationTransformNode");
    else if (::boost::dynamic_pointer_cast<PropertyAnimationNode>(rpNode))
        return OUString::createFromAscii("PropertyAnimationNode");
    else
        return OUString::createFromAscii("<unknown node type>");
}




void DebugShowState (const AnimationNodeSharedPtr& rpNode)
{
    if ( ! rpNode)
        return;

    OUString sState;
    OUString sStateColor;
    switch (rpNode->getState())
    {
        default:
        case AnimationNode::INVALID:
            sState = OUString::createFromAscii("Invalid");
            sStateColor = OUString::createFromAscii("firebrick1");
            break;
        case AnimationNode::UNRESOLVED:
            sState = OUString::createFromAscii("Unresolved");
            sStateColor = OUString::createFromAscii("dodgerblue4");
            break;
        case AnimationNode::RESOLVED:
            sState = OUString::createFromAscii("Resolved");
            sStateColor = OUString::createFromAscii("dodgerblue");
            break;
        case AnimationNode::ACTIVE:
            sState = OUString::createFromAscii("Active");
            sStateColor = OUString::createFromAscii("seagreen1");
            break;
        case AnimationNode::FROZEN:
            sState = OUString::createFromAscii("Frozen");
            sStateColor = OUString::createFromAscii("lightskyblue1");
            break;
        case AnimationNode::ENDED:
            sState = OUString::createFromAscii("Ended");
            sStateColor = OUString::createFromAscii("slategray3");
            break;
    }

    const uno::Any aBegin (rpNode->getXAnimationNode()->getBegin());
    OUString sTrigger;
    if (aBegin.hasValue())
    {
        animations::Event aEvent;
        double nTimeOffset;
        const static char* sEventTriggers[] = {
            "NONE", "ON_BEGIN", "ON_END", "BEGIN_EVENT", "END_EVENT", "ON_CLICK",
            "ON_DBL_CLICK", "ON_MOUSE_ENTER", "ON_MOUSE_LEAVE", "ON_NEXT", "ON_PREV",
            "ON_STOP_AUDIO", "REPEAT"};
        if (aBegin >>= aEvent)
        {
            sTrigger = OUString::createFromAscii(sEventTriggers[aEvent.Trigger]);
        }
        else if (aBegin >>= nTimeOffset)
        {
            sTrigger = OUString::valueOf(nTimeOffset);
        }
        else
        {
            sTrigger = OUString::createFromAscii("other");
        }
    }
    else
        sTrigger = ::rtl::OUString::createFromAscii("void");

    TRACE("Node state: n%x [label=\"%x / %x / %s\\n%s\\n%s\",style=filled,fillcolor=\"%s\"]\r",
        rpNode.get(),
        rpNode.get(),
        rpNode->getXAnimationNode().get(),
        ::rtl::OUStringToOString(sState, RTL_TEXTENCODING_ASCII_US).getStr(),
        ::rtl::OUStringToOString(DebugGetDescription(rpNode), RTL_TEXTENCODING_ASCII_US).getStr(),
        ::rtl::OUStringToOString(sTrigger, RTL_TEXTENCODING_ASCII_US).getStr(),
        ::rtl::OUStringToOString(sStateColor, RTL_TEXTENCODING_ASCII_US).getStr());

    BaseContainerNodeSharedPtr pContainer (
        ::boost::dynamic_pointer_cast<BaseContainerNode>(rpNode));
    if (pContainer)
        ::boost::static_pointer_cast<NodeContainer>(rpNode)->ShowChildrenState();
}




void NodeContainer::ShowChildrenState (void) const
{
    for (std::size_t nIndex=0; nIndex<maChildren.size(); ++nIndex)
    {
        TRACE("Node connection: n%x -> n%x", this, maChildren[nIndex].get());
        DebugShowState(maChildren[nIndex]);
    }
}




AnimationNodeSharedPtr DebugGetTreeRoot (const BaseNodeSharedPtr& rpNode)
{
    BaseNodeSharedPtr pNode (rpNode);
    if (pNode)
    {
        BaseNodeSharedPtr pParent (pNode->getParentNode());
        while (pParent)
        {
            pNode = pParent;
            pParent = pNode->getParentNode();
        }
    }
    return pNode;
}

} // end of anonymous namespace




void Debug_ShowNodeTree (const AnimationNodeSharedPtr& rpNode)
{
    DebugTraceScope aTraceScope ("NodeTree");

    DebugShowState(DebugGetTreeRoot(::boost::dynamic_pointer_cast<BaseNode>(rpNode)));
}




//----- Tracing ---------------------------------------------------------------

extern "C" {

    namespace {

    class TraceData
    {
    public:
        TraceData (void)
            : mnIndentation(0),
              mpFile(fopen(TRACE_LOG_FILE_NAME, "w")),
              maTime()
        {
        }

        int mnIndentation;
        FILE* mpFile;
        ::canvas::tools::ElapsedTime maTime;
    };
    static TraceData gTraceData;

    inline void SAL_CALL DebugTrace (
        const int nIndentationOffset,
        const sal_Char* sFormat,
        va_list args)
    {
        if (gTraceData.mpFile != NULL)
        {
            // Write line head with current time and indentation.
            // Adapt indentation.
            if (nIndentationOffset < 0)
                gTraceData.mnIndentation += nIndentationOffset;
            fprintf(gTraceData.mpFile, "%10.8f ", gTraceData.maTime.getElapsedTime());
            for (int nIndentation=0; nIndentation<gTraceData.mnIndentation; ++nIndentation)
                fprintf(gTraceData.mpFile, "    ");
            if (nIndentationOffset > 0)
                gTraceData.mnIndentation += nIndentationOffset;

            // Write message.
            vfprintf(gTraceData.mpFile, sFormat, args);
            fprintf(gTraceData.mpFile, "\n");
            fflush(gTraceData.mpFile);
        }
    }

} // end of anonymous namespace


} // end of extern "C"

void SAL_CALL DebugTraceBegin (const sal_Char* sFormat, ...)
{
    va_list args;
    va_start(args, sFormat);
    DebugTrace(+1,sFormat, args);
    va_end(args);
}

void SAL_CALL DebugTraceEnd (const sal_Char* sFormat, ...)
{
    va_list args;
    va_start(args, sFormat);
    DebugTrace(-1,sFormat, args);
    va_end(args);
}

void SAL_CALL DebugTraceMessage (const sal_Char* sFormat, ...)
{
    va_list args;
    va_start(args, sFormat);
    DebugTrace(0,sFormat, args);
    va_end(args);
}



DebugTraceScope::DebugTraceScope (const sal_Char* sFormat, ...)
    : msMessage(new sal_Char[mnBufferSize])
{
    va_list args;
    va_start(args, sFormat);

    msMessage[mnBufferSize-1] = 0;
    vsnprintf(msMessage, mnBufferSize-1, sFormat, args);
    TRACE_BEGIN("[ %s", msMessage);
    va_end(args);
}

DebugTraceScope::~DebugTraceScope (void)
{
    TRACE_END("] %s", msMessage);
    delete [] msMessage;
}


} }

#endif // OSL_DEBUG_LEVEL > 1
