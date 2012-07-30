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

#include <editeng/LatinLookupTree.hxx>
#include <editeng/LatinTreeNode.hxx>

LatinLookupTree::LatinLookupTree(OUString sLanguage) :
    LookupTree( sLanguage )
{
    for ( sal_Unicode i = 0; i < 52; ++i )
    {
        m_pLeaves[i] = NULL;
    }
}

LatinLookupTree::~LatinLookupTree()
{
    freeMemory();
}

void LatinLookupTree::returnToRoot()
{
    if ( m_pCurrent == m_pHead )
        return;

    // If there is no entry in this node or the tree that sprouts from it.
    if ( m_pCurrent &&
            m_pCurrent->m_pParent &&
            !m_pCurrent->m_nChildren &&
            !m_pCurrent->m_nKeyProbability )
    {
        m_pCurrent->m_pParent->childHasChanged( m_pCurrent, 0, true );
    }

    m_pCurrent = m_pHead;
}

void LatinLookupTree::gotoNode(OUString sNode)
{
    returnToRoot();

    // walk down the tree...
    for ( int i = 0; i < sNode.getLength(); i++ )
    {
        m_pCurrent = m_pCurrent->advanceKey( sNode[i] );
    }
}

void LatinLookupTree::advance(const sal_Unicode cKey)
{
    m_pCurrent = m_pCurrent->advanceKey( cKey );
}

void LatinLookupTree::goBack()
{
    if ( m_pCurrent->m_pParent ) // if we're not at the root
    {
        const Node* const pChild = m_pCurrent;
        m_pCurrent = m_pCurrent->m_pParent; // set focus to parent

        // if this is an unused tree leaf
        if (  !pChild->m_nChildren && !pChild->m_nKeyProbability )
        {
            m_pCurrent->removeChild( m_pCurrent->getChildRef( pChild->m_cKey ) );
        }
    }
}

void LatinLookupTree::insert(OUString sKey, const int nProbability)
{
    if ( !sKey.isEmpty() && nProbability > 0 )
    {
        insertKey( sKey, nProbability );
    }
}

void LatinLookupTree::insert(const int nProbability)
{
    if ( m_pCurrent == this )
        return;

    // change probability
    int proba = m_pCurrent->m_nKeyProbability += nProbability;

    // inform his parent
    m_pCurrent->m_pParent->childHasChanged( m_pCurrent, proba );
}

void LatinLookupTree::remove(OUString sKey)
{
    if ( !sKey.isEmpty() )
    {
        removeKey( sKey );
    }
}

OUString LatinLookupTree::suggestAutoCompletion() const
{
    if ( !m_pCurrent )
        return OUString();

    Node* pWalker = m_pCurrent;

    int distance = 0, nTargetProbability = 0;
    OUString sSuggestion;

    while ( pWalker->m_pSuggest && ( distance < 2 ||
        // Make sure the suggestion is at least 2 chars long.
        nTargetProbability == pWalker->m_nHighestProbaInSubtree ) )
    {
        if ( distance < 2 )
            nTargetProbability = pWalker->m_nHighestProbaInSubtree;

        // follow the tree along the suggested route
        pWalker = pWalker->m_pSuggest;
        ++distance;
        sSuggestion += OUString(pWalker->m_cKey);
    }

    return sSuggestion;
}

void LatinLookupTree::clear()
{
    freeMemory();
}

bool LatinLookupTree::isSeparatedlyHandled(const sal_Unicode cKey) const
{
    return
        ( cKey >= sal_Unicode('a') && cKey <= sal_Unicode('z') )
    ||  ( cKey >= sal_Unicode('A') && cKey <= sal_Unicode('Z') );
}

Node*& LatinLookupTree::getChildRef(const sal_Unicode cKey, bool bCreatePlaceholder)
{
    int pos = -1;

    // determine position in array if possible
    if ( cKey >= sal_Unicode('a') && cKey <= sal_Unicode('z') )
    {
        pos = cKey - our_nLowerCaseA;
    }
    else if ( cKey >= sal_Unicode('A') && cKey <= sal_Unicode('Z') )
    {
        pos = cKey - our_nUpperCaseA + 26;
    }

    if ( pos != -1 )
    {
        return m_pLeaves[pos];
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

void LatinLookupTree::evaluateSeparateStorage(int& nSuggest, Node*& pSuggest) const
{
    for ( sal_Unicode i = 0; i < 52; ++i )
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

void LatinLookupTree::freeMemory()
{
    // remove nodes from array
    for ( sal_Unicode i = 0; i < 52; ++i )
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

Node* LatinLookupTree::newNode(Node* pParent, const sal_Unicode cKey, const int nProbability)
{
    return new LatinTreeNode( this, pParent, cKey, nProbability );
}

const unsigned int LatinLookupTree::our_nLowerCaseA = 97;
const unsigned int LatinLookupTree::our_nUpperCaseA = 65;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
