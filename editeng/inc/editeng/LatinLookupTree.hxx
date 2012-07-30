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

#ifndef LATINLOOKUPTREE_HXX
#define LATINLOOKUPTREE_HXX

#include <editeng/LookupTree.hxx>
#include <editeng/TreeHead.hxx>
#include <editeng/editengdllapi.h>

using namespace rtl;

/**
 * LatinLookupTree implements a tree that is optimized for storing and looking
 * up words that mainly consist of roman characters, although any other
 * language can be handled, too.
 */
class EDITENG_DLLPUBLIC LatinLookupTree : public LookupTree, public TreeHead
{
public:

    explicit LatinLookupTree(OUString sLanguage);
    ~LatinLookupTree();


    /* =================== Implemented Virtuals From LookupTree =================== */
    void returnToRoot();
    void gotoNode(OUString sNode);
    void advance(const sal_Unicode a);
    void goBack();
    void insert(OUString sKey, const int nProbability = 1);
    void insert(const int nProbability = 1);
    void remove(OUString sKey);
    OUString suggestAutoCompletion() const;
    void clear();

    /* =================== Implemented Virtuals From Node =================== */
    bool isSeparatedlyHandled(const sal_Unicode cKey) const;
    Node*& getChildRef(const sal_Unicode cKey, bool bCreatePlaceholder = false);
    void evaluateSeparateStorage(int& nSuggest, Node*& pSuggest) const;
    void freeMemory();


    /* =================== Implemented Virtual From TreeHead =================== */
    Node* newNode(Node* pParent, const sal_Unicode cKey, const int nProbability = 0);

    /* =================== Member Variables =================== */
    // position of lower case letter 'a' within the selected char encoding.
    static const unsigned int our_nLowerCaseA;

    // position of upper case letter 'A' within the selected char encoding.
    static const unsigned int our_nUpperCaseA;

private:
    Node*   m_pLeaves[52];  // handles [a-z] and [A-Z]
};

#endif // LATINLOOKUPTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
