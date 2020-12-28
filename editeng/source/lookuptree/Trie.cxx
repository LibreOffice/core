/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <editeng/Trie.hxx>

namespace editeng
{

using namespace std;

/* TrieNode */

struct TrieNode final
{
    static const int LATIN_ARRAY_SIZE = 26;

    sal_Unicode             mCharacter;
    bool                    mMarker;
    std::vector<std::unique_ptr<TrieNode>>  mChildren;
    std::unique_ptr<TrieNode> mLatinArray[LATIN_ARRAY_SIZE];

    explicit TrieNode(sal_Unicode aCharacter = '\0');

    void      markWord();
    TrieNode* findChild(sal_Unicode aCharacter);
    TrieNode* traversePath(const OUString& sPath);
    void      addNewChild(TrieNode* pChild);
    void      collectSuggestions(std::u16string_view sPath, std::vector<OUString>& rSuggestionList);
    static void  collectSuggestionsForCurrentNode(TrieNode* pCurrent, std::u16string_view sPath, vector<OUString>& rSuggestionList);
};

TrieNode::TrieNode(sal_Unicode aCharacter) :
    mCharacter(aCharacter),
    mMarker(false)
{
    for (auto & i : mLatinArray)
    {
        i = nullptr;
    }
}

void TrieNode::markWord()
{
    mMarker = true;
}

void TrieNode::addNewChild(TrieNode* pChild)
{
    if (pChild->mCharacter >= 'a' &&
        pChild->mCharacter <= 'z')
    {
        mLatinArray[pChild->mCharacter - u'a'].reset(pChild);
    }
    else
    {
        mChildren.push_back(std::unique_ptr<TrieNode>(pChild));
    }
}

TrieNode* TrieNode::findChild(sal_Unicode aInputCharacter)
{
    if (aInputCharacter >= 'a' &&
        aInputCharacter <= 'z')
    {
        return mLatinArray[aInputCharacter - u'a'].get();
    }

    for(auto const & pCurrent : mChildren)
    {
        if ( pCurrent->mCharacter == aInputCharacter )
            return pCurrent.get();
    }

    return nullptr;
}

void TrieNode::collectSuggestions(std::u16string_view sPath, vector<OUString>& rSuggestionList)
{
    // first traverse nodes for alphabet characters
    for (auto const & pCurrent : mLatinArray)
    {
        if (pCurrent != nullptr)
            collectSuggestionsForCurrentNode(pCurrent.get(), sPath, rSuggestionList);
    }

    // traverse nodes for other characters
    for(auto const & pCurrent : mChildren)
    {
        if (pCurrent != nullptr)
            collectSuggestionsForCurrentNode(pCurrent.get(), sPath, rSuggestionList);
    }
}

void TrieNode::collectSuggestionsForCurrentNode(TrieNode* pCurrent, std::u16string_view sPath, vector<OUString>& rSuggestionList)
{
    OUString aStringPath = sPath + OUStringChar(pCurrent->mCharacter);
    if(pCurrent->mMarker)
    {
        rSuggestionList.push_back(aStringPath);
    }
    // recursively descend tree
    pCurrent->collectSuggestions(aStringPath, rSuggestionList);
}

TrieNode* TrieNode::traversePath(const OUString& sPath)
{
    TrieNode* pCurrent = this;

    for ( sal_Int32 i = 0; i < sPath.getLength(); i++ )
    {
        sal_Unicode aCurrentChar = sPath[i];
        pCurrent = pCurrent->findChild(aCurrentChar);
        if ( pCurrent == nullptr )
            return nullptr;
    }

    return pCurrent;
}

/* TRIE */

Trie::Trie() :
    mRoot(new TrieNode())
{}

Trie::~Trie()
{}

void Trie::insert(const OUString& sInputString) const
{
    // adding an empty word is not allowed
    if ( sInputString.isEmpty() )
    {
        return;
    }

    // traverse the input string and modify the tree with new nodes / characters

    TrieNode* pCurrent = mRoot.get();
    sal_Unicode aCurrentChar;

    for ( sal_Int32 i = 0; i < sInputString.getLength(); i++ )
    {
        aCurrentChar = sInputString[i];
        TrieNode* pChild = pCurrent->findChild(aCurrentChar);
        if ( pChild == nullptr )
        {
            TrieNode* pNewNode = new TrieNode(aCurrentChar);
            pCurrent->addNewChild(pNewNode);
            pCurrent = pNewNode;
        }
        else
        {
            pCurrent = pChild;
        }
    }

    pCurrent->markWord();
}

void Trie::findSuggestions(const OUString& sWordPart, vector<OUString>& rSuggestionList) const
{
    TrieNode* pNode = mRoot->traversePath(sWordPart);

    if (pNode != nullptr)
    {
        pNode->collectSuggestions(sWordPart, rSuggestionList);
    }
}

size_t Trie::size() const
{
    if (!mRoot)
        return 0;
    std::vector<OUString> entries;
    mRoot->collectSuggestions(std::u16string_view(), entries);
    return entries.size();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
