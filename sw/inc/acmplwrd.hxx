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

#ifndef SW_ACMPLWRD_HXX
#define SW_ACMPLWRD_HXX

#include <deque>

#include <editeng/swafopt.hxx>
#include <editeng/Trie.hxx>
#include <tools/string.hxx>

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
    editeng::Trie m_LookupTree;
    SwAutoCompleteStringPtrDeque aLRULst;

    SwAutoCompleteWord_Impl* pImpl;
    sal_uInt16 nMaxCount, nMinWrdLen;
    bool bLockWordLst;

    void DocumentDying(const SwDoc& rDoc);
public:
    SwAutoCompleteWord( sal_uInt16 nWords = 500, sal_uInt16 nMWrdLen = 10 );
    ~SwAutoCompleteWord();

    bool InsertWord( const String& rWord, SwDoc& rDoc );

    const OUString& operator[](size_t n) const
        { return m_WordList[n]->GetAutoCompleteString(); }

    bool IsLockWordLstLocked() const           { return bLockWordLst; }
    void SetLockWordLstLocked( bool bFlag ) { bLockWordLst = bFlag; }

    void SetMaxCount( sal_uInt16 n );

    sal_uInt16 GetMinWordLen() const                { return nMinWrdLen; }
    void SetMinWordLen( sal_uInt16 n );

    const editeng::SortedAutoCompleteStrings& GetWordList() const
        { return m_WordList; }

    void CheckChangedList(const editeng::SortedAutoCompleteStrings& rNewLst);

    // Returns all words matching a given prefix aMatch.
    bool GetWordsMatching(String aMatch, std::vector<String>& aWords) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
