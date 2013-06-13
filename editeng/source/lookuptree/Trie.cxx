/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <editeng/Trie.hxx>

namespace editeng
{

using namespace std;

/* TrieNode */

TrieNode::TrieNode(sal_Unicode aCharacter) :
    mCharacter(aCharacter),
    mMarker(false)
{
    for (int i=0; i<LATIN_ARRAY_SIZE; i++)
    {
        mLatinArray[i] = NULL;
    }
}

TrieNode::~TrieNode()
{
    vector<TrieNode*>::iterator iNode;
    for(iNode = mChildren.begin(); iNode != mChildren.end(); ++iNode)
    {
        delete *iNode;
    }

    for (int i=0; i<LATIN_ARRAY_SIZE; i++)
    {
        delete mLatinArray[i];
    }
}

void TrieNode::markWord()
{
    mMarker = true;
}

void TrieNode::addNewChild(TrieNode* pChild)
{
    if (pChild->mCharacter >= sal_Unicode('a') &&
        pChild->mCharacter <= sal_Unicode('z'))
    {
        mLatinArray[pChild->mCharacter - sal_Unicode('a')] = pChild;
    }
    else
    {
        mChildren.push_back(pChild);
    }
}

TrieNode* TrieNode::findChild(sal_Unicode aInputCharacter)
{
    if (aInputCharacter >= sal_Unicode('a') &&
        aInputCharacter <= sal_Unicode('z'))
    {
        return mLatinArray[aInputCharacter - sal_Unicode('a')];
    }

    vector<TrieNode*>::iterator iNode;

    for(iNode = mChildren.begin(); iNode != mChildren.end(); ++iNode)
    {
        TrieNode* pCurrent = *iNode;
        if ( pCurrent->mCharacter == aInputCharacter )
            return pCurrent;
    }

    return NULL;
}

void TrieNode::collectSuggestions(OUString sPath, vector<OUString>& rSuggestionList)
{
    // first traverse nodes for alphabet characters
    for (int i=0; i<LATIN_ARRAY_SIZE; i++)
    {
        TrieNode* pCurrent = mLatinArray[i];
        if (pCurrent != NULL)
        {
            OUString aStringPath = sPath + OUString(pCurrent->mCharacter);
            if(pCurrent->mMarker)
                rSuggestionList.push_back(aStringPath);
            // recursivly traverse tree
            pCurrent->collectSuggestions(aStringPath, rSuggestionList);
        }
    }

    // traverse nodes for other characters
    vector<TrieNode*>::iterator iNode;
    for(iNode = mChildren.begin(); iNode != mChildren.end(); ++iNode)
    {
        TrieNode* pCurrent = *iNode;
        if (pCurrent != NULL)
        {
            OUString aStringPath = sPath + OUString(pCurrent->mCharacter);
            if(pCurrent->mMarker)
                rSuggestionList.push_back(aStringPath);
            // recursivly traverse tree
            pCurrent->collectSuggestions(aStringPath, rSuggestionList);
        }
    }
}

TrieNode* TrieNode::traversePath(OUString sPath)
{
    TrieNode* pCurrent = this;

    for ( sal_Int32 i = 0; i < sPath.getLength(); i++ )
    {
        sal_Unicode aCurrentChar = sPath[i];
        pCurrent = pCurrent->findChild(aCurrentChar);
        if ( pCurrent == NULL )
            return NULL;
    }

    return pCurrent;
}

/* TRIE */

Trie::Trie()
{
    mRoot = new TrieNode();
}

Trie::~Trie()
{
    delete mRoot;
}

void Trie::insert(OUString sInputString) const
{
    // adding an empty word is not allowed
    if ( sInputString.isEmpty() )
    {
        return;
    }

    // traverse the input string and modify the tree with new nodes / characters

    TrieNode* pCurrent = mRoot;
    sal_Unicode aCurrentChar;

    for ( sal_Int32 i = 0; i < sInputString.getLength(); i++ )
    {
        aCurrentChar = sInputString[i];
        TrieNode* pChild = pCurrent->findChild(aCurrentChar);
        if ( pChild == NULL )
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

void Trie::findSuggestions(OUString sWordPart, vector<OUString>& rSuggesstionList) const
{
    TrieNode* pNode = mRoot->traversePath(sWordPart);

    if (pNode != NULL)
    {
        pNode->collectSuggestions(sWordPart, rSuggesstionList);
    }
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
