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

#ifndef LATINTREENODE_HXX
#define LATINTREENODE_HXX

#include <editeng/Node.hxx>

/**
  * LatinTreeNode represents a node within a LatinLookupTree. As en external
  * caller, you should never have to do anything with this class directly.
  * Use the class LatinLookupTree instead for constructing a new tree.
  */
class LatinTreeNode : public Node
{
public:
    explicit LatinTreeNode(TreeHead *pHead, Node* pParent, const sal_Unicode cKey, const int nProbability = 0);
    ~LatinTreeNode();

    /* =================== Implemented Virtuals From Node =================== */
    bool isSeparatedlyHandled(const sal_Unicode cKey) const;
    Node*& getChildRef(const sal_Unicode cKey, bool bCreatePlaceholder = false);
    void evaluateSeparateStorage(int& nSuggest, Node*& pSuggest) const;
    void freeMemory();

private:
    Node* m_pLeaves[26];  // handles [a-z]
};

#endif // LATINTREENODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
