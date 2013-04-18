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

#ifndef LOOKUPTREE_H
#define LOOKUPTREE_H

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <editeng/editengdllapi.h>

/** LookupTree is an interface class that allows for unified access to tree
  * structures used for storing dictionnary words as well as their respective
  * probabilities.
  * It allows you to insert or remove words from the tree, navigate threw the
  * tree along its branches and request for a suggestion for autocompletion
  * according to the position within the tree.
  * It also allows you to attribute a specific language to each tree so that
  * it is possible to serve the correct auto completions even within a document
  * that contains content in more than one language.
  */
class EDITENG_DLLPUBLIC LookupTree
{
public:
    explicit inline LookupTree(OUString sLanguage);
    virtual ~LookupTree() {}

    inline OUString language() const;

    // Resets the current item to root.
    virtual void returnToRoot() = 0;

    // Advances from the root position key by key towards the node keyed with
    // the last char of sKey.
    virtual void gotoNode(OUString sNode) = 0;

    // Advances from the current position towards the node keyed with cKey.
    virtual void advance(const sal_Unicode cKey) = 0;

    // Sets the focus to the parent of the current node. Removes the current
    // node if it is invalid.
    virtual void goBack() = 0;

    // Inserts a complete keyword starting from the root node of the tree.
    // Does not change the current position within the tree.
    virtual void insert(OUString sKey, const int nProbability = 1) = 0;

    // Inserts a keyword with the given probability at the current position
    // within the tree. Does not change the current position within the tree.
    virtual void insert(const int nProbability = 1) = 0;

    // Removes a complete keyword starting from the root node of the tree.
    // Does not change the current position within the tree.
    virtual void remove(OUString sKey) = 0;

    // Returns the suggested autocompletion for the current location within
    // the tree.
    virtual OUString suggestAutoCompletion() const = 0;

    // Clears the tree and removes any information it contains.
    virtual void clear() = 0;


private:
    const OUString m_sLanguage;  // language handled by this tree
};

LookupTree::LookupTree(OUString sLanguage) :
    m_sLanguage( sLanguage )
{
}

OUString LookupTree::language() const
{
    return m_sLanguage;
}

#endif // LOOKUPTREE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
