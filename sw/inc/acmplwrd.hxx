/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SW_ACMPLWRD_HXX
#define SW_ACMPLWRD_HXX

#include <deque>

#include <editeng/swafopt.hxx>
#include <editeng/LatinLookupTree.hxx>

class SwDoc;
class SwAutoCompleteWord_Impl;
class SwAutoCompleteClient;
class SwAutoCompleteString;

typedef std::deque<SwAutoCompleteString*> SwAutoCompleteStringPtrDeque;

class SwAutoCompleteWord
{
    friend class SwAutoCompleteClient;

    /// contains extended strings carrying source information
    editeng::SortedAutoCompleteStrings m_WordList;
    LookupTree* m_LookupTree;
    SwAutoCompleteStringPtrDeque aLRULst;

    SwAutoCompleteWord_Impl* pImpl;
    sal_uInt16 nMaxCount, nMinWrdLen;
    bool bLockWordLst;

    void DocumentDying(const SwDoc& rDoc);
public:
    SwAutoCompleteWord( sal_uInt16 nWords = 500, sal_uInt16 nMWrdLen = 10 );
    ~SwAutoCompleteWord();

    sal_Bool InsertWord( const String& rWord, SwDoc& rDoc );

    sal_Bool GetRange( const String& rWord, sal_uInt16& rStt, sal_uInt16& rEnd ) const;

    const String& operator[](size_t n) const
        { return m_WordList[n]->GetAutoCompleteString(); }

    bool IsLockWordLstLocked() const           { return bLockWordLst; }
    void SetLockWordLstLocked( bool bFlag ) { bLockWordLst = bFlag; }

    void SetMaxCount( sal_uInt16 n );

    sal_uInt16 GetMinWordLen() const                { return nMinWrdLen; }
    void SetMinWordLen( sal_uInt16 n );

    const editeng::SortedAutoCompleteStrings& GetWordList() const
        { return m_WordList; }

    void CheckChangedList(const editeng::SortedAutoCompleteStrings& rNewLst);

    // Resets the current position within the tree to its root node.
    void returnToRoot();

    // Advances to a given node within the AutoComplete tree.
    void gotoNode(OUString sNode);

    // Advances from the current position towards the node keyed with cKey.
    void advance(const sal_Unicode cKey);

    // Goes back one char within the tree, except if the current node is already the root node.
    void goBack();

    // Returns all words matching a given prefix aMatch. If bIgnoreCurrentPos is set, the current
    // position within the tree is ignored and replaced by aMatch.
    bool GetWordsMatching(String aMatch, std::vector<String>& aWords, sal_Bool bIgnoreCurrentPos) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
