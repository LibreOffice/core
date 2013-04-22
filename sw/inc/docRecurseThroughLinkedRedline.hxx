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

#ifndef SW_DOCRECURSETHROUGHLINKEDREDLINE_HX
#define SW_DOCRECURSETHROUGHLINKEDREDLINE_HX

#include <redline.hxx>
#include <node.hxx>

// wait for c++11 variadic template to merge all this ugly copy/past into one template

template<typename T>
void SwCntntNode::RecurseThroughLinkedRedline(const T methode)
{
    // if the redline table isn't empty and the redlines are shown
    const SwRedlineTbl& rRedlineTbl = GetDoc()->GetRedlineTbl();
    if ((GetDoc()->GetRedlineMode() & nsRedlineMode_t::REDLINE_SHOW_MASK) && rRedlineTbl.size() != 0)
    {
        // get the start start and end position of the current node
        SwPosition aNodeStart = SwPosition(*this);
        SwPosition aNodeEnd   = SwPosition(*this, Len());

        // loop through the redline table
        unsigned int nRedlineIndex = 0;
        while (nRedlineIndex < rRedlineTbl.size())
        {
            // if the current redline is a deletion redline and it contain the start or the end
            // of the current node
            if ((rRedlineTbl[ nRedlineIndex ]->GetType() & nsRedlineType_t::REDLINE_DELETE) &&
                    (rRedlineTbl[ nRedlineIndex ]->ContainsPosition(aNodeStart) ||
                     rRedlineTbl[ nRedlineIndex ]->ContainsPosition(aNodeEnd)))
            {
                // get the start position of the first and last node of the redline
                SwPosition aRedlineFirstNodeStart = SwPosition(rRedlineTbl[ nRedlineIndex ]->Start()->nNode.GetNode());
                SwPosition aRedlineLastNodeStart  = SwPosition(rRedlineTbl[ nRedlineIndex ]->End()->nNode.GetNode());

                // if the redline impact only one node
                if (aRedlineFirstNodeStart == aRedlineLastNodeStart)
                {
                    // nothing to do…
                    // (it doesn't make sense to copy the format of a node to past it on the same node)
                    // we continu with the next redline
                    ++nRedlineIndex;
                    continue;
                }

                // unless the only direction handled is "previous"
                // or the current node is in the middle of the redline (it is not the first node of the redline)
                if(m_eHandleRedlineDirection != SwCntntNode::REDLINE_DIRECTION_PREVIOUS && aNodeStart == aRedlineFirstNodeStart)
                {
                    // get the first node of the redline
                    SwCntntNode* pRedlineFirstNode = aRedlineLastNodeStart.nNode.GetNode().GetCntntNode();

                    if (pRedlineFirstNode != NULL)
                    {
                        // limit the recurse redline direction to "next", explain :
                        // "methode" will call RecurseThroughLinkedRedline, if we don't limit the direction
                        // the RecurseThroughLinkedRedline function will re-call "methode" on the current
                        // node
                        // → infinit loop
                        pRedlineFirstNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_NEXT;

                        // call "methode" (modify the format) on the first node of the redline
                        (pRedlineFirstNode->*methode)();

                        // "methode" is finished, next call to RecurseThroughLinkedRedline have to
                        // be able to go on both direction
                        pRedlineFirstNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_BOTH;
                    }
                }
                // unless the only direction handled is "next"
                // or the current node is in the middle of the redline (it is not the last node of the redline)
                else if (m_eHandleRedlineDirection != SwCntntNode::REDLINE_DIRECTION_NEXT && aNodeStart == aRedlineLastNodeStart)
                {
                    // get the last node of the redline
                    SwCntntNode* pRedlineLastNode = aRedlineFirstNodeStart.nNode.GetNode().GetCntntNode();

                    if (pRedlineLastNode != NULL)
                    {
                        // limit the recurse redline direction to "previous" (see above)
                        pRedlineLastNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_PREVIOUS;
                        // call "methode" (modify the format) on the last node of the redline
                        (pRedlineLastNode->*methode)();
                        // "methode" is finished, next call to RecurseThroughLinkedRedline have to
                        // be able to go on both direction
                        pRedlineLastNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_BOTH;
                    }
                }
            }
            // the current redline is beyond the current node
            else if (aNodeEnd < *(rRedlineTbl[ nRedlineIndex ]->Start()))
            {
                // no need to continu the loop : all the next redline will also be beyond
                // the current node (the redline table is sorted).
                break;
            }

            // goto the next redline in the redline table
            ++nRedlineIndex;
        }
    }
}

template<typename T, typename U>
void SwCntntNode::RecurseThroughLinkedRedline(const T methode, U arg1)
{
    // if the redline table isn't empty and the redlines are shown
    const SwRedlineTbl& rRedlineTbl = GetDoc()->GetRedlineTbl();
    if ((GetDoc()->GetRedlineMode() & nsRedlineMode_t::REDLINE_SHOW_MASK) && rRedlineTbl.size() != 0)
    {
        // get the start start and end position of the current node
        SwPosition aNodeStart = SwPosition(*this);
        SwPosition aNodeEnd   = SwPosition(*this, Len());

        // loop through the redline table
        unsigned int nRedlineIndex = 0;
        while (nRedlineIndex < rRedlineTbl.size())
        {
            // if the current redline is a deletion redline and it contain the start or the end
            // of the current node
            if ((rRedlineTbl[ nRedlineIndex ]->GetType() & nsRedlineType_t::REDLINE_DELETE) &&
                    (rRedlineTbl[ nRedlineIndex ]->ContainsPosition(aNodeStart) ||
                     rRedlineTbl[ nRedlineIndex ]->ContainsPosition(aNodeEnd)))
            {
                // get the start position of the first and last node of the redline
                SwPosition aRedlineFirstNodeStart = SwPosition(rRedlineTbl[ nRedlineIndex ]->Start()->nNode.GetNode());
                SwPosition aRedlineLastNodeStart  = SwPosition(rRedlineTbl[ nRedlineIndex ]->End()->nNode.GetNode());

                // if the redline impact only one node
                if (aRedlineFirstNodeStart == aRedlineLastNodeStart)
                {
                    // nothing to do…
                    // (it doesn't make sense to copy the format of a node to past it on the same node)
                    // we continu with the next redline
                    ++nRedlineIndex;
                    continue;
                }

                // unless the only direction handled is "previous"
                // or the current node is in the middle of the redline (it is not the first node of the redline)
                if(m_eHandleRedlineDirection != SwCntntNode::REDLINE_DIRECTION_PREVIOUS && aNodeStart == aRedlineFirstNodeStart)
                {
                    // get the first node of the redline
                    SwCntntNode* pRedlineFirstNode = aRedlineLastNodeStart.nNode.GetNode().GetCntntNode();

                    if (pRedlineFirstNode != NULL)
                    {
                        // limit the recurse redline direction to "next", explain :
                        // "methode" will call RecurseThroughLinkedRedline, if we don't limit the direction
                        // the RecurseThroughLinkedRedline function will re-call "methode" on the current
                        // node
                        // → infinit loop
                        pRedlineFirstNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_NEXT;

                        // call "methode" (modify the format) on the first node of the redline
                        (pRedlineFirstNode->*methode)(arg1);

                        // "methode" is finished, next call to RecurseThroughLinkedRedline have to
                        // be able to go on both direction
                        pRedlineFirstNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_BOTH;
                    }
                }
                // unless the only direction handled is "next"
                // or the current node is in the middle of the redline (it is not the last node of the redline)
                else if (m_eHandleRedlineDirection != SwCntntNode::REDLINE_DIRECTION_NEXT && aNodeStart == aRedlineLastNodeStart)
                {
                    // get the last node of the redline
                    SwCntntNode* pRedlineLastNode = aRedlineFirstNodeStart.nNode.GetNode().GetCntntNode();

                    if (pRedlineLastNode != NULL)
                    {
                        // limit the recurse redline direction to "previous" (see above)
                        pRedlineLastNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_PREVIOUS;
                        // call "methode" (modify the format) on the last node of the redline
                        (pRedlineLastNode->*methode)(arg1);
                        // "methode" is finished, next call to RecurseThroughLinkedRedline have to
                        // be able to go on both direction
                        pRedlineLastNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_BOTH;
                    }
                }
            }
            // the current redline is beyond the current node
            else if (aNodeEnd < *(rRedlineTbl[ nRedlineIndex ]->Start()))
            {
                // no need to continu the loop : all the next redline will also be beyond
                // the current node (the redline table is sorted).
                break;
            }

            // goto the next redline in the redline table
            ++nRedlineIndex;
        }
    }
}

template<typename T, typename U, typename V>
void SwCntntNode::RecurseThroughLinkedRedline(const T methode, U arg1, V arg2)
{
    // if the redline table isn't empty and the redlines are shown
    const SwRedlineTbl& rRedlineTbl = GetDoc()->GetRedlineTbl();
    if ((GetDoc()->GetRedlineMode() & nsRedlineMode_t::REDLINE_SHOW_MASK) && rRedlineTbl.size() != 0)
    {
        // get the start start and end position of the current node
        SwPosition aNodeStart = SwPosition(*this);
        SwPosition aNodeEnd   = SwPosition(*this, Len());

        // loop through the redline table
        unsigned int nRedlineIndex = 0;
        while (nRedlineIndex < rRedlineTbl.size())
        {
            // if the current redline is a deletion redline and it contain the start or the end
            // of the current node
            if ((rRedlineTbl[ nRedlineIndex ]->GetType() & nsRedlineType_t::REDLINE_DELETE) &&
                    (rRedlineTbl[ nRedlineIndex ]->ContainsPosition(aNodeStart) ||
                     rRedlineTbl[ nRedlineIndex ]->ContainsPosition(aNodeEnd)))
            {
                // get the start position of the first and last node of the redline
                SwPosition aRedlineFirstNodeStart = SwPosition(rRedlineTbl[ nRedlineIndex ]->Start()->nNode.GetNode());
                SwPosition aRedlineLastNodeStart  = SwPosition(rRedlineTbl[ nRedlineIndex ]->End()->nNode.GetNode());

                // if the redline impact only one node
                if (aRedlineFirstNodeStart == aRedlineLastNodeStart)
                {
                    // nothing to do…
                    // (it doesn't make sense to copy the format of a node to past it on the same node)
                    // we continu with the next redline
                    ++nRedlineIndex;
                    continue;
                }

                // unless the only direction handled is "previous"
                // or the current node is in the middle of the redline (it is not the first node of the redline)
                if(m_eHandleRedlineDirection != SwCntntNode::REDLINE_DIRECTION_PREVIOUS && aNodeStart == aRedlineFirstNodeStart)
                {
                    // get the first node of the redline
                    SwCntntNode* pRedlineFirstNode = aRedlineLastNodeStart.nNode.GetNode().GetCntntNode();

                    if (pRedlineFirstNode != NULL)
                    {
                        // limit the recurse redline direction to "next", explain :
                        // "methode" will call RecurseThroughLinkedRedline, if we don't limit the direction
                        // the RecurseThroughLinkedRedline function will re-call "methode" on the current
                        // node
                        // → infinit loop
                        pRedlineFirstNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_NEXT;

                        // call "methode" (modify the format) on the first node of the redline
                        (pRedlineFirstNode->*methode)(arg1, arg2);

                        // "methode" is finished, next call to RecurseThroughLinkedRedline have to
                        // be able to go on both direction
                        pRedlineFirstNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_BOTH;
                    }
                }
                // unless the only direction handled is "next"
                // or the current node is in the middle of the redline (it is not the last node of the redline)
                else if (m_eHandleRedlineDirection != SwCntntNode::REDLINE_DIRECTION_NEXT && aNodeStart == aRedlineLastNodeStart)
                {
                    // get the last node of the redline
                    SwCntntNode* pRedlineLastNode = aRedlineFirstNodeStart.nNode.GetNode().GetCntntNode();

                    if (pRedlineLastNode != NULL)
                    {
                        // limit the recurse redline direction to "previous" (see above)
                        pRedlineLastNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_PREVIOUS;
                        // call "methode" (modify the format) on the last node of the redline
                        (pRedlineLastNode->*methode)(arg1, arg2);
                        // "methode" is finished, next call to RecurseThroughLinkedRedline have to
                        // be able to go on both direction
                        pRedlineLastNode->m_eHandleRedlineDirection = SwCntntNode::REDLINE_DIRECTION_BOTH;
                    }
                }
            }
            // the current redline is beyond the current node
            else if (aNodeEnd < *(rRedlineTbl[ nRedlineIndex ]->Start()))
            {
                // no need to continu the loop : all the next redline will also be beyond
                // the current node (the redline table is sorted).
                break;
            }

            // goto the next redline in the redline table
            ++nRedlineIndex;
        }
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
