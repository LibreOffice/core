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

#ifndef NODE_HXX
#define NODE_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <list>

class TreeHead;

/**
  * Node represents a node within a LookupTree. As en external caller, you
  * should never have to do anything with this class directly.
  * Use any of the classes derived from LookupTree instead for constructing a
  * new tree.
  */
class Node
{
public:
    //explicit Node(TreeHead* const pHead);
    explicit Node(TreeHead* const pHead, Node* const pParent = NULL,
                    const sal_Unicode cKey = 0, const int nProbability = 0);

    virtual ~Node();

    // Removes the specified child from this node. Make sure you may remove it
    // before doing so.
    void removeChild(Node*& pChild);

    // Inserts a complete keyword starting from this node of the tree.
    void insertKey(OUString sKey, const int nProbability);
    // Removes a complete keyword starting from this node of the tree.
    void removeKey(OUString sKey);

    // Returns the child node keyed with cKey.
    Node* advanceKey(const sal_Unicode cKey);

    // Use this to inform a parent about its child having changed.
    // Call this only with nProbability = 0 if you have made sure the node can
    // be removed.
    void childHasChanged(Node* pChild, const int nProbability, bool bAllowRemoval = false);

    // Rechose the node that is suggested for auto-completion
    void reevaluateSuggestion(bool& bNodeProbabilityChanged);


    /*  =================== Virtuals =================== */
    virtual bool isSeparatedlyHandled(const sal_Unicode cKey) const = 0;

    // Returns a reference to the pointer to the child node for the requested
    // char. Returns NULL if no such child could be found.
    // IMPORTANT: In the latter case, you may NOT overwrite the return value,
    // if you did not set bCreatePlaceholder to true.
    virtual Node*& getChildRef(const sal_Unicode cKey, bool bCreatePlaceholder = false) = 0;

    // Sets nSuggest to the highest probability within the subtree and pSuggest
    // to point to the (first) node with this probability.
    virtual void evaluateSeparateStorage(int& nSuggest, Node*& pSuggest) const = 0;

    // Removes all child nodes and clears all memory.
    virtual void freeMemory() = 0;

    /* =================== Member Variables =================== */
    const sal_Unicode m_cKey;             // the char represented by this node
    int               m_nKeyProbability;  // the number of occurrences of this key

    // the highest KeyProbability in the tree sprouting from this node
    int m_nHighestProbaInSubtree;

    Node* const m_pParent; // the parent of this node
    Node* m_pSuggest;      // next node in chain to the suggested autocompletion

    TreeHead* const m_pHead; // head of the tree

    unsigned short m_nChildren;     // the number of children of the node
    std::list<Node*> m_lChildren;   // all chars not handled by array

    // Allows returning a reference to a valid Null pointer. May NOT be overwritten.
    static Node* our_pNodeNullPointer;
};

#endif // NODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
