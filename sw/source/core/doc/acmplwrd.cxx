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

#include <tools/urlobj.hxx>
#include <hintids.hxx>
#include <acmplwrd.hxx>
#include <doc.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <calbck.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <editeng/svxacorr.hxx>
#include <osl/diagnose.h>

#include <editeng/acorrcfg.hxx>
#include <sfx2/docfile.hxx>
#include <docsh.hxx>

#include <cassert>
#include <vector>

class SwAutoCompleteClient : public SwClient
{
    SwAutoCompleteWord* m_pAutoCompleteWord;
    SwDoc*              m_pDoc;
#if OSL_DEBUG_LEVEL > 0
    static sal_uLong s_nSwAutoCompleteClientCount;
#endif
public:
    SwAutoCompleteClient(SwAutoCompleteWord& rToTell, SwDoc& rSwDoc);
    SwAutoCompleteClient(const SwAutoCompleteClient& rClient);
    virtual ~SwAutoCompleteClient() override;

    SwAutoCompleteClient& operator=(const SwAutoCompleteClient& rClient);

    const SwDoc& GetDoc() const {return *m_pDoc;}
#if OSL_DEBUG_LEVEL > 0
    static sal_uLong GetElementCount() {return s_nSwAutoCompleteClientCount;}
#endif
protected:
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;
};

class SwAutoCompleteWord_Impl
{
    std::vector<SwAutoCompleteClient>
                                m_aClientVector;
    SwAutoCompleteWord&         m_rAutoCompleteWord;
public:
    explicit SwAutoCompleteWord_Impl(SwAutoCompleteWord& rParent) :
        m_rAutoCompleteWord(rParent){}
    void AddDocument(SwDoc& rDoc);
    void RemoveDocument(const SwDoc& rDoc);
};

class SwAutoCompleteString
    : public editeng::IAutoCompleteString
{
#if OSL_DEBUG_LEVEL > 0
    static sal_uLong s_nSwAutoCompleteStringCount;
#endif
    std::vector<const SwDoc*> m_aSourceDocs;
    public:
        SwAutoCompleteString(const OUString& rStr, sal_Int32 nLen);

        virtual ~SwAutoCompleteString() override;
        void        AddDocument(const SwDoc& rDoc);
        //returns true if last document reference has been removed
        bool        RemoveDocument(const SwDoc& rDoc);
#if OSL_DEBUG_LEVEL > 0
    static sal_uLong GetElementCount() {return s_nSwAutoCompleteStringCount;}
#endif
};
#if OSL_DEBUG_LEVEL > 0
    sal_uLong SwAutoCompleteClient::s_nSwAutoCompleteClientCount = 0;
    sal_uLong SwAutoCompleteString::s_nSwAutoCompleteStringCount = 0;
#endif

SwAutoCompleteClient::SwAutoCompleteClient(SwAutoCompleteWord& rToTell, SwDoc& rSwDoc) :
        m_pAutoCompleteWord(&rToTell),
        m_pDoc(&rSwDoc)
{
    m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
#if OSL_DEBUG_LEVEL > 0
    ++s_nSwAutoCompleteClientCount;
#endif
}

SwAutoCompleteClient::SwAutoCompleteClient(const SwAutoCompleteClient& rClient) :
    SwClient(),
    m_pAutoCompleteWord(rClient.m_pAutoCompleteWord),
    m_pDoc(rClient.m_pDoc)
{
    m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
#if OSL_DEBUG_LEVEL > 0
    ++s_nSwAutoCompleteClientCount;
#endif
}

SwAutoCompleteClient::~SwAutoCompleteClient()
{
#if OSL_DEBUG_LEVEL > 0
    --s_nSwAutoCompleteClientCount;
#else
    (void) this;
#endif
}

SwAutoCompleteClient& SwAutoCompleteClient::operator=(const SwAutoCompleteClient& rClient)
{
    m_pAutoCompleteWord = rClient.m_pAutoCompleteWord;
    m_pDoc = rClient.m_pDoc;
    StartListeningToSameModifyAs(rClient);
    return *this;
}

void SwAutoCompleteClient::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    auto pLegacy = dynamic_cast<const sw::LegacyModifyHint*>(&rHint);
    if(!pLegacy)
        return;
    switch(pLegacy->GetWhich())
    {
        case RES_REMOVE_UNO_OBJECT:
        case RES_OBJECTDYING:
            EndListeningAll();
            m_pAutoCompleteWord->DocumentDying(*m_pDoc);
    }
}

void SwAutoCompleteWord_Impl::AddDocument(SwDoc& rDoc)
{
    if (std::any_of(m_aClientVector.begin(), m_aClientVector.end(),
            [&rDoc](SwAutoCompleteClient& rClient) { return &rClient.GetDoc() == &rDoc; }))
        return;
    m_aClientVector.emplace_back(m_rAutoCompleteWord, rDoc);
}

void SwAutoCompleteWord_Impl::RemoveDocument(const SwDoc& rDoc)
{
    auto aIt = std::find_if(m_aClientVector.begin(), m_aClientVector.end(),
        [&rDoc](SwAutoCompleteClient& rClient) { return &rClient.GetDoc() == &rDoc; });
    if (aIt != m_aClientVector.end())
        m_aClientVector.erase(aIt);
}

SwAutoCompleteString::SwAutoCompleteString(
            const OUString& rStr, sal_Int32 const nLen)
    : editeng::IAutoCompleteString(rStr.copy(0, nLen))
{
#if OSL_DEBUG_LEVEL > 0
    ++s_nSwAutoCompleteStringCount;
#endif
}

SwAutoCompleteString::~SwAutoCompleteString()
{
#if OSL_DEBUG_LEVEL > 0
    --s_nSwAutoCompleteStringCount;
#else
    (void) this;
#endif
}

void SwAutoCompleteString::AddDocument(const SwDoc& rDoc)
{
    auto aIt = std::find(m_aSourceDocs.begin(), m_aSourceDocs.end(), &rDoc);
    if (aIt != m_aSourceDocs.end())
        return;
    m_aSourceDocs.push_back(&rDoc);
}

bool SwAutoCompleteString::RemoveDocument(const SwDoc& rDoc)
{
    auto aIt = std::find(m_aSourceDocs.begin(), m_aSourceDocs.end(), &rDoc);
    if (aIt != m_aSourceDocs.end())
    {
        m_aSourceDocs.erase(aIt);
        return m_aSourceDocs.empty();
    }
    return false;
}

SwAutoCompleteWord::SwAutoCompleteWord(
    editeng::SortedAutoCompleteStrings::size_type nWords, sal_uInt16 nMWrdLen ):
    m_pImpl(new SwAutoCompleteWord_Impl(*this)),
    m_nMaxCount( nWords ),
    m_nMinWordLen( nMWrdLen ),
    m_bLockWordList( false )
{
}

SwAutoCompleteWord::~SwAutoCompleteWord()
{
    m_WordList.DeleteAndDestroyAll(); // so the assertion below works
#if OSL_DEBUG_LEVEL > 0
    sal_uLong nStrings = SwAutoCompleteString::GetElementCount();
    sal_uLong nClients = SwAutoCompleteClient::GetElementCount();
    OSL_ENSURE(!nStrings && !nClients, "AutoComplete: clients or string count mismatch");
#endif
}

bool SwAutoCompleteWord::InsertWord( const OUString& rWord, SwDoc& rDoc )
{
    SwDocShell* pDocShell = rDoc.GetDocShell();
    SfxMedium* pMedium = pDocShell ? pDocShell->GetMedium() : nullptr;
    // strings from help module should not be added
    if( pMedium )
    {
        const INetURLObject& rURL = pMedium->GetURLObject();
        if ( rURL.GetProtocol() == INetProtocol::VndSunStarHelp )
            return false;
    }

    OUString aNewWord = rWord.replaceAll(OUStringChar(CH_TXTATR_INWORD), "")
                             .replaceAll(OUStringChar(CH_TXTATR_BREAKWORD), "");

    m_pImpl->AddDocument(rDoc);
    bool bRet = false;
    sal_Int32 nWrdLen = aNewWord.getLength();
    while( nWrdLen && '.' == aNewWord[ nWrdLen-1 ])
        --nWrdLen;

    if( !m_bLockWordList && nWrdLen >= m_nMinWordLen )
    {
        SwAutoCompleteString* pNew = new SwAutoCompleteString( aNewWord, nWrdLen );
        pNew->AddDocument(rDoc);
        std::pair<editeng::SortedAutoCompleteStrings::const_iterator, bool>
            aInsPair = m_WordList.insert(pNew);

        m_LookupTree.insert( aNewWord.copy(0, nWrdLen) );

        if (aInsPair.second)
        {
            bRet = true;
            if (m_aLRUList.size() >= m_nMaxCount)
            {
                // the last one needs to be removed
                // so that there is space for the first one
                SwAutoCompleteString* pDel = m_aLRUList.back();
                m_aLRUList.pop_back();
                m_WordList.erase(pDel);
                delete pDel;
            }
            m_aLRUList.push_front(pNew);
        }
        else
        {
            delete pNew;
            // then move "up"
            pNew = static_cast<SwAutoCompleteString*>(*aInsPair.first);

            // add the document to the already inserted string
            pNew->AddDocument(rDoc);

            // move pNew to the front of the LRU list
            SwAutoCompleteStringPtrDeque::iterator it = std::find( m_aLRUList.begin(), m_aLRUList.end(), pNew );
            OSL_ENSURE( m_aLRUList.end() != it, "String not found" );
            if ( m_aLRUList.begin() != it && m_aLRUList.end() != it )
            {
                m_aLRUList.erase( it );
                m_aLRUList.push_front( pNew );
            }
        }
    }
    return bRet;
}

void SwAutoCompleteWord::SetMaxCount(
    editeng::SortedAutoCompleteStrings::size_type nNewMax )
{
    if( nNewMax < m_nMaxCount && m_aLRUList.size() > nNewMax )
    {
        // remove the trailing ones
        SwAutoCompleteStringPtrDeque::size_type nLRUIndex = nNewMax-1;
        while (nNewMax < m_WordList.size() && nLRUIndex < m_aLRUList.size())
        {
            auto it = m_WordList.find(m_aLRUList[nLRUIndex++]);
            OSL_ENSURE( m_WordList.end() != it, "String not found" );
            editeng::IAutoCompleteString *const pDel = *it;
            m_WordList.erase(it);
            delete pDel;
        }
        m_aLRUList.erase( m_aLRUList.begin() + nNewMax - 1, m_aLRUList.end() );
    }
    m_nMaxCount = nNewMax;
}

void SwAutoCompleteWord::SetMinWordLen( sal_uInt16 n )
{
    // Do you really want to remove all words that are less than the minWrdLen?
    if( n < m_nMinWordLen )
    {
        for (size_t nPos = 0; nPos < m_WordList.size(); ++nPos)
            if (m_WordList[ nPos ]->GetAutoCompleteString().getLength() < n)
            {
                SwAutoCompleteString *const pDel =
                    dynamic_cast<SwAutoCompleteString*>(m_WordList[nPos]);
                m_WordList.erase_at(nPos);

                SwAutoCompleteStringPtrDeque::iterator it = std::find( m_aLRUList.begin(), m_aLRUList.end(), pDel );
                OSL_ENSURE( m_aLRUList.end() != it, "String not found" );
                m_aLRUList.erase( it );
                --nPos;
                delete pDel;
            }
    }

    m_nMinWordLen = n;
}

/** Return all words matching a given prefix
 *
 *  @param aMatch the prefix to search for
 *  @param rWords the words found matching
 */
bool SwAutoCompleteWord::GetWordsMatching(const OUString& aMatch, std::vector<OUString>& rWords) const
{
    assert(rWords.empty());
    m_LookupTree.findSuggestions(aMatch, rWords);
    return !rWords.empty();
}

void SwAutoCompleteWord::CheckChangedList(
        const editeng::SortedAutoCompleteStrings& rNewLst)
{
    size_t nMyLen = m_WordList.size(), nNewLen = rNewLst.size();
    size_t nMyPos = 0, nNewPos = 0;

    for( ; nMyPos < nMyLen && nNewPos < nNewLen; ++nMyPos, ++nNewPos )
    {
        const editeng::IAutoCompleteString * pStr = rNewLst[ nNewPos ];
        while (m_WordList[nMyPos] != pStr)
        {
            SwAutoCompleteString *const pDel =
                dynamic_cast<SwAutoCompleteString*>(m_WordList[nMyPos]);
            m_WordList.erase_at(nMyPos);
            SwAutoCompleteStringPtrDeque::iterator it = std::find( m_aLRUList.begin(), m_aLRUList.end(), pDel );
            OSL_ENSURE( m_aLRUList.end() != it, "String not found" );
            m_aLRUList.erase( it );
            delete pDel;
            if( nMyPos >= --nMyLen )
                break;
        }
    }
    // remove the elements at the end of the array
    if( nMyPos >= nMyLen )
        return;

    // clear LRU array first then delete the string object
    for( ; nNewPos < nMyLen; ++nNewPos )
    {
        SwAutoCompleteString *const pDel =
            dynamic_cast<SwAutoCompleteString*>(m_WordList[nNewPos]);
        SwAutoCompleteStringPtrDeque::iterator it = std::find( m_aLRUList.begin(), m_aLRUList.end(), pDel );
        OSL_ENSURE( m_aLRUList.end() != it, "String not found" );
        m_aLRUList.erase( it );
        delete pDel;
    }
    // remove from array
    m_WordList.erase(m_WordList.begin() + nMyPos,
                     m_WordList.begin() + nMyLen);
}

void SwAutoCompleteWord::DocumentDying(const SwDoc& rDoc)
{
    m_pImpl->RemoveDocument(rDoc);

    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const bool bDelete = !pACorr->GetSwFlags().bAutoCmpltKeepList;
    for (size_t nPos = m_WordList.size(); nPos; nPos--)
    {
        SwAutoCompleteString *const pCurrent = dynamic_cast<SwAutoCompleteString*>(m_WordList[nPos - 1]);
        if(pCurrent && pCurrent->RemoveDocument(rDoc) && bDelete)
        {
            m_WordList.erase_at(nPos - 1);
            SwAutoCompleteStringPtrDeque::iterator it = std::find( m_aLRUList.begin(), m_aLRUList.end(), pCurrent );
            OSL_ENSURE( m_aLRUList.end() != it, "word not found in LRU list" );
            m_aLRUList.erase( it );
            delete pCurrent;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
