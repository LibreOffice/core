/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TRIE_HXX
#define TRIE_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <editeng/editengdllapi.h>

namespace editeng
{

struct TrieNode
{
    static const int LATIN_ARRAY_SIZE = 26;

    sal_Unicode             mCharacter;
    bool                    mMarker;
    std::vector<TrieNode*>  mChildren;
    TrieNode*               mLatinArray[LATIN_ARRAY_SIZE];


    TrieNode(sal_Unicode aCharacter = '\0');
    virtual ~TrieNode();

    void      markWord();
    TrieNode* findChild(sal_Unicode aCharacter);
    TrieNode* traversePath(OUString sPath);
    void      addNewChild(TrieNode* pChild);
    void      collectSuggestions(OUString sPath, std::vector<OUString>& rSuggestionList);
};

class EDITENG_DLLPUBLIC Trie
{
private:
    TrieNode* mRoot;

public:
    Trie();
    virtual ~Trie();

    void insert(OUString sInputString) const;
    void findSuggestions(OUString sWordPart, std::vector<OUString>& rSuggesstionList) const;

};

}

#endif // TRIE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
