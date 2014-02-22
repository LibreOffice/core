/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <editeng/Trie.hxx>

namespace editeng
{

using namespace std;

/* TrieNode */

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
    void      collectSuggestionsForCurrentNode(TrieNode* pCurrent, OUString sPath, vector<OUString>& rSuggestionList);
};

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
    if (pChild->mCharacter >= 'a' &&
        pChild->mCharacter <= 'z')
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
    if (aInputCharacter >= 'a' &&
        aInputCharacter <= 'z')
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
    
    for (int i=0; i<LATIN_ARRAY_SIZE; i++)
    {
        TrieNode* pCurrent = mLatinArray[i];
        if (pCurrent != NULL)
            collectSuggestionsForCurrentNode(pCurrent, sPath, rSuggestionList);
    }

    
    vector<TrieNode*>::iterator iNode;
    for(iNode = mChildren.begin(); iNode != mChildren.end(); ++iNode)
    {
        TrieNode* pCurrent = *iNode;
        if (pCurrent != NULL)
            collectSuggestionsForCurrentNode(pCurrent, sPath, rSuggestionList);
    }
}

void TrieNode::collectSuggestionsForCurrentNode(TrieNode* pCurrent, OUString sPath, vector<OUString>& rSuggestionList)
{
    OUString aStringPath = sPath + OUString(pCurrent->mCharacter);
    if(pCurrent->mMarker)
    {
        rSuggestionList.push_back(aStringPath);
    }
    
    pCurrent->collectSuggestions(aStringPath, rSuggestionList);
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

Trie::Trie() :
    mRoot(new TrieNode())
{}

Trie::~Trie()
{}

void Trie::insert(OUString sInputString) const
{
    
    if ( sInputString.isEmpty() )
    {
        return;
    }

    

    TrieNode* pCurrent = mRoot.get();
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

void Trie::getAllEntries(std::vector<OUString>& entries)
{
    if (mRoot)
    {
        mRoot->collectSuggestions(OUString(), entries);
    }
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
