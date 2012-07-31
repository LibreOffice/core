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

#include <editeng/TreeHead.hxx>
#include <editeng/Node.hxx>

Node::Node(TreeHead* const pHead, Node* const pParent,
            const sal_Unicode cKey, const int nProbability ) :
    m_cKey( cKey ),
    m_nKeyProbability( nProbability ),
    m_nHighestProbaInSubtree( 0 ),
    m_pParent( pParent ),
    m_pSuggest( NULL ),
    m_pHead( pHead ),
    m_nChildren( 0 )
{
}

Node::~Node()
{
}

void Node::removeChild(Node*& pChild)
{
    const sal_Unicode cKey = pChild->m_cKey;

    if ( pChild )
    {
        delete pChild;
        pChild = NULL;
        --m_nChildren;
    }

    if ( !isSeparatedlyHandled( cKey ) )
    {
        std::list<Node*>::iterator i = m_lChildren.begin();
        while ( i != m_lChildren.end() )
        {
            if ( !(*i) )
            {
                i = m_lChildren.erase( i );
            }
            else
            {
                ++i;
            }
        }
    }
}

void Node::insertKey(OUString sKey, const int nProbability)
{
    if ( !sKey.isEmpty() )
    {
        const sal_Unicode cKey = sKey[0];
        sKey = sKey.copy( 1 );

        Node*& pChild = getChildRef( cKey, true );

        if ( !pChild )
        {
            pChild = m_pHead->newNode( this, cKey );
            ++m_nChildren;
        }

        pChild->insertKey( sKey, nProbability );
    }
    else
    {
        m_nKeyProbability += nProbability;
        if ( m_pParent )
        {
            // inform parent about change
            int probability = m_nHighestProbaInSubtree > m_nKeyProbability ? m_nHighestProbaInSubtree : m_nKeyProbability;
            m_pParent->childHasChanged( this, probability);
        }
    }
}

// Removes a complete keyword starting from this node of the tree.
void Node::removeKey(OUString sKey)
{
    if ( !sKey.isEmpty() )
    {
        Node*& pChild = getChildRef( sKey[0] );

        if ( pChild )
        {
            // recursive call downwards
            pChild->removeKey( sKey.copy( 1 ) );
        }
        // Else: Keyword to be removed couldn't be found within the tree.
        // No further changes are going to be made.
    }
    else    // If we are the node to be removed...
    {
        // ... remove our entry from tree...
        m_nKeyProbability = 0;
        // ... and make sure our parent is updated.
        m_pParent->childHasChanged( this, m_nHighestProbaInSubtree, this != m_pHead->m_pCurrent );
    }
}

Node *Node::advanceKey(const sal_Unicode cKey)
{
    Node*& pChild = getChildRef( cKey, true );

    if ( !pChild )
    {
        pChild = m_pHead->newNode( this, cKey );
    }

    return pChild;
}

void Node::childHasChanged(Node *pChild, const int nProbability, bool bAllowRemoval)
{
    if ( nProbability > m_nHighestProbaInSubtree )
    {
        m_pSuggest = pChild;
        m_nHighestProbaInSubtree = nProbability;

        if ( m_pParent ) // recursive call upwards
        {
            int probabilityChange = nProbability > m_nKeyProbability ? nProbability : m_nKeyProbability;
            m_pParent->childHasChanged( this, probabilityChange );
        }
    }
    else if ( !nProbability || nProbability < m_nHighestProbaInSubtree )
    {
        bool bNewEvaluationRequired = m_pSuggest == pChild;

        if ( !nProbability && bAllowRemoval )
        {
            // Remove child. Caller needs to make sure we are allowed to.
            removeChild( getChildRef( pChild->m_cKey ) );
        }

        if ( bNewEvaluationRequired )
        {
            // This is used to store whether we need to inform our parent about
            // the changes within this node.
            bool bNodeProbabilityChanged;

            reevaluateSuggestion( bNodeProbabilityChanged );

            // If necessary, inform our parent about change via recursive call
            if ( bNodeProbabilityChanged && m_pParent )
            {
                bAllowRemoval = bAllowRemoval && this != m_pHead->m_pCurrent;
                int probabilityChange = m_nHighestProbaInSubtree > m_nKeyProbability ? m_nHighestProbaInSubtree : m_nKeyProbability;
                m_pParent->childHasChanged( this, probabilityChange, bAllowRemoval );
            }
        }
    }
}

void Node::reevaluateSuggestion(bool& bNodeProbabilityChanged)
{
    if ( m_nChildren ) // find child with highest probability
    {
        int nSuggest = 0;
        Node* pSuggest = NULL;

        // find child with highest probability in array
        evaluateSeparateStorage( nSuggest, pSuggest );

        // do the same thing within list
        for ( std::list<Node*>::iterator i = m_lChildren.begin(); i != m_lChildren.end(); ++i )
        {
            if ( (*i)->m_nHighestProbaInSubtree > nSuggest )
            {
                nSuggest = (*i)->m_nHighestProbaInSubtree;
                pSuggest = (*i);
            }
            if ( (*i)->m_nKeyProbability > nSuggest )
            {
                nSuggest = (*i)->m_nKeyProbability;
                pSuggest = (*i);
            }
        }

        // determine whether we need to inform our parent
        bNodeProbabilityChanged = m_nHighestProbaInSubtree != nSuggest;

        m_pSuggest = pSuggest;
        m_nHighestProbaInSubtree = nSuggest;
    }
    else // there are no children
    {
        m_pSuggest = NULL;
        m_nHighestProbaInSubtree = 0;

        bNodeProbabilityChanged = true;
    }
}

Node* Node::our_pNodeNullPointer = NULL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
