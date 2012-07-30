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

#include <editeng/LatinTreeNode.hxx>
#include <editeng/LatinLookupTree.hxx>

LatinTreeNode::LatinTreeNode(TreeHead* pHead, Node* pParent, const sal_Unicode cKey, const int nProbability) :
    Node( pHead, pParent, cKey, nProbability )
{
    for ( sal_Unicode i = 0; i < 26; ++i )
    {
        m_pLeaves[i] = NULL;
    }
}

LatinTreeNode::~LatinTreeNode()
{
    freeMemory();
}

bool LatinTreeNode::isSeparatedlyHandled(const sal_Unicode cKey) const
{
    return ( cKey >= sal_Unicode('a') && cKey <= sal_Unicode('z') );
}

Node*& LatinTreeNode::getChildRef(const sal_Unicode cKey, bool bCreatePlaceholder)
{
    // determine position in array if possible
    if ( cKey >= sal_Unicode('a') && cKey <= sal_Unicode('z') )
    {
        return m_pLeaves[cKey - LatinLookupTree::our_nLowerCaseA];
    }
    else
    {
        for ( std::list<Node*>::iterator i = m_lChildren.begin(); i != m_lChildren.end(); ++i )
        {
            if ( (*i)->m_cKey == cKey )
            {
                return *i;
            }
        }
        if ( bCreatePlaceholder )
        {
            // Create new entry in case there isn't one.
            m_lChildren.push_back( NULL );
            return *(--m_lChildren.end());
        }
        else
        {
            return our_pNodeNullPointer;
        }
    }
}

void LatinTreeNode::evaluateSeparateStorage(int& nSuggest, Node*& pSuggest) const
{
    for ( sal_Unicode i = 0; i < 26; ++i )
    {
        if ( m_pLeaves[i] )
        {
            if ( m_pLeaves[i]->m_nHighestProbaInSubtree > nSuggest )
            {
                nSuggest = m_pLeaves[i]->m_nHighestProbaInSubtree;
                pSuggest = m_pLeaves[i];
            }
            if ( m_pLeaves[i]->m_nKeyProbability > nSuggest )
            {
                nSuggest = m_pLeaves[i]->m_nKeyProbability;
                pSuggest = m_pLeaves[i];
            }
        }
    }
}

void LatinTreeNode::freeMemory()
{
    // remove nodes from array
    for ( sal_Unicode i = 0; i < 26; ++i )
    {
        if ( m_pLeaves[i] )
        {
            m_pLeaves[i]->freeMemory();
            delete m_pLeaves[i];
            m_pLeaves[i] = NULL;
        }
    }
    // clear list
    while ( m_lChildren.size() )
    {
        Node* pTmp = m_lChildren.front();
        m_lChildren.pop_front();
        delete pTmp;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
