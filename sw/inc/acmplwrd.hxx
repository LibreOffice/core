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

#ifndef INCLUDED_SW_INC_ACMPLWRD_HXX
#define INCLUDED_SW_INC_ACMPLWRD_HXX

#include <deque>
#include <memory>

#include <editeng/swafopt.hxx>
#include <editeng/Trie.hxx>
#include <rtl/ustring.hxx>

class SwDoc;
class SwAutoCompleteWord_Impl;
class SwAutoCompleteString;

typedef std::deque<SwAutoCompleteString*> SwAutoCompleteStringPtrDeque;

class SwAutoCompleteWord
{
    friend class SwAutoCompleteClient;

    /// contains extended strings carrying source information
    editeng::SortedAutoCompleteStrings m_WordList;
    editeng::Trie const m_LookupTree;
    SwAutoCompleteStringPtrDeque m_aLRUList;

    std::unique_ptr<SwAutoCompleteWord_Impl> m_pImpl;
    editeng::SortedAutoCompleteStrings::size_type m_nMaxCount;
    sal_uInt16 m_nMinWordLen;
    bool m_bLockWordList;

    void DocumentDying(const SwDoc& rDoc);
public:
    SwAutoCompleteWord(
        editeng::SortedAutoCompleteStrings::size_type nWords,
        sal_uInt16 nMWrdLen );
    ~SwAutoCompleteWord();

    bool InsertWord( const OUString& rWord, SwDoc& rDoc );

    bool IsLockWordLstLocked() const           { return m_bLockWordList; }
    void SetLockWordLstLocked( bool bFlag ) { m_bLockWordList = bFlag; }

    void SetMaxCount( editeng::SortedAutoCompleteStrings::size_type n );

    sal_uInt16 GetMinWordLen() const                { return m_nMinWordLen; }
    void SetMinWordLen( sal_uInt16 n );

    const editeng::SortedAutoCompleteStrings& GetWordList() const
        { return m_WordList; }

    void CheckChangedList(const editeng::SortedAutoCompleteStrings& rNewLst);

    // Returns all words matching a given prefix aMatch.
    bool GetWordsMatching(const OUString& aMatch, std::vector<OUString>& aWords) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
