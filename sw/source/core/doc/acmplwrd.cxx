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

#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <acmplwrd.hxx>
#include <doc.hxx>
#include <ndindex.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <calbck.hxx>
#include <editeng/svxacorr.hxx>

#include <editeng/acorrcfg.hxx>
#include <sfx2/docfile.hxx>
#include <docsh.hxx>

#include <vector>

class SwAutoCompleteClient : public SwClient
{
    SwAutoCompleteWord* pAutoCompleteWord;
    SwDoc*              pDoc;
#if OSL_DEBUG_LEVEL > 0
    static sal_uLong nSwAutoCompleteClientCount;
#endif
public:
    SwAutoCompleteClient(SwAutoCompleteWord& rToTell, SwDoc& rSwDoc);
    SwAutoCompleteClient(const SwAutoCompleteClient& rClient);
    ~SwAutoCompleteClient();

    SwAutoCompleteClient& operator=(const SwAutoCompleteClient& rClient);

    const SwDoc& GetDoc(){return *pDoc;}
#if OSL_DEBUG_LEVEL > 0
    static sal_uLong GetElementCount() {return nSwAutoCompleteClientCount;}
#endif
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
};

typedef std::vector<SwAutoCompleteClient> SwAutoCompleteClientVector;

class SwAutoCompleteWord_Impl
{
    SwAutoCompleteClientVector  aClientVector;
    SwAutoCompleteWord&         rAutoCompleteWord;
public:
    SwAutoCompleteWord_Impl(SwAutoCompleteWord& rParent) :
        rAutoCompleteWord(rParent){}
    void AddDocument(SwDoc& rDoc);
    void RemoveDocument(const SwDoc& rDoc);
};

typedef const SwDoc* SwDocPtr;
typedef std::vector<SwDocPtr> SwDocPtrVector;
class SwAutoCompleteString
    : public editeng::IAutoCompleteString
{
#if OSL_DEBUG_LEVEL > 0
    static sal_uLong nSwAutoCompleteStringCount;
#endif
    SwDocPtrVector aSourceDocs;
    public:
        SwAutoCompleteString(const String& rStr, xub_StrLen nPos, xub_StrLen nLen);

        ~SwAutoCompleteString();
        void        AddDocument(const SwDoc& rDoc);
        //returns true if last document reference has been removed
        bool        RemoveDocument(const SwDoc& rDoc);
#if OSL_DEBUG_LEVEL > 0
    static sal_uLong GetElementCount() {return nSwAutoCompleteStringCount;}
#endif
};
#if OSL_DEBUG_LEVEL > 0
    sal_uLong SwAutoCompleteClient::nSwAutoCompleteClientCount = 0;
    sal_uLong SwAutoCompleteString::nSwAutoCompleteStringCount = 0;
#endif

SwAutoCompleteClient::SwAutoCompleteClient(SwAutoCompleteWord& rToTell, SwDoc& rSwDoc) :
        pAutoCompleteWord(&rToTell),
        pDoc(&rSwDoc)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
#if OSL_DEBUG_LEVEL > 0
    ++nSwAutoCompleteClientCount;
#endif
}

SwAutoCompleteClient::SwAutoCompleteClient(const SwAutoCompleteClient& rClient) :
    SwClient(),
    pAutoCompleteWord(rClient.pAutoCompleteWord),
    pDoc(rClient.pDoc)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
#if OSL_DEBUG_LEVEL > 0
    ++nSwAutoCompleteClientCount;
#endif
}

SwAutoCompleteClient::~SwAutoCompleteClient()
{
#if OSL_DEBUG_LEVEL > 0
    --nSwAutoCompleteClientCount;
#endif
}

SwAutoCompleteClient& SwAutoCompleteClient::operator=(const SwAutoCompleteClient& rClient)
{
    pAutoCompleteWord = rClient.pAutoCompleteWord;
    pDoc = rClient.pDoc;
    if(rClient.GetRegisteredIn())
        ((SwModify*)rClient.GetRegisteredIn())->Add(this);
    else if(GetRegisteredIn())
        GetRegisteredInNonConst()->Remove(this);
    return *this;
}

void SwAutoCompleteClient::Modify( const SfxPoolItem* pOld, const SfxPoolItem *)
{
    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        if( (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
        pAutoCompleteWord->DocumentDying(*pDoc);
        break;

    }
}

void SwAutoCompleteWord_Impl::AddDocument(SwDoc& rDoc)
{
    SwAutoCompleteClientVector::iterator aIt;
    for(aIt = aClientVector.begin(); aIt != aClientVector.end(); ++aIt)
    {
        if(&aIt->GetDoc() == &rDoc)
            return;
    }
    aClientVector.push_back(SwAutoCompleteClient(rAutoCompleteWord, rDoc));
}

void SwAutoCompleteWord_Impl::RemoveDocument(const SwDoc& rDoc)
{
    SwAutoCompleteClientVector::iterator aIt;
    for(aIt = aClientVector.begin(); aIt != aClientVector.end(); ++aIt)
    {
        if(&aIt->GetDoc() == &rDoc)
        {
            aClientVector.erase(aIt);
            return;
        }
    }
}

SwAutoCompleteString::SwAutoCompleteString(
            const String& rStr, xub_StrLen const nPos, xub_StrLen const nLen)
    : editeng::IAutoCompleteString(String(rStr, nPos, nLen))
{
#if OSL_DEBUG_LEVEL > 0
    ++nSwAutoCompleteStringCount;
#endif
}

SwAutoCompleteString::~SwAutoCompleteString()
{
#if OSL_DEBUG_LEVEL > 0
    --nSwAutoCompleteStringCount;
#endif
}

void SwAutoCompleteString::AddDocument(const SwDoc& rDoc)
{
    for(SwDocPtrVector::iterator aIt = aSourceDocs.begin(); aIt != aSourceDocs.end(); ++aIt)
    {
        if( *aIt == &rDoc )
            return;
    }
    aSourceDocs.push_back(&rDoc);
}

bool SwAutoCompleteString::RemoveDocument(const SwDoc& rDoc)
{
    for(SwDocPtrVector::iterator aIt = aSourceDocs.begin(); aIt != aSourceDocs.end(); ++aIt)
    {
        if( *aIt == &rDoc )
        {
            aSourceDocs.erase(aIt);
            return aSourceDocs.empty();
        }
    }
    return false;
}

SwAutoCompleteWord::SwAutoCompleteWord( sal_uInt16 nWords, sal_uInt16 nMWrdLen ) :
    pImpl(new SwAutoCompleteWord_Impl(*this)),
    nMaxCount( nWords ),
    nMinWrdLen( nMWrdLen ),
    bLockWordLst( sal_False )
{
}

SwAutoCompleteWord::~SwAutoCompleteWord()
{
    m_WordList.DeleteAndDestroyAll(); // so the assertion below works
    delete pImpl;
#if OSL_DEBUG_LEVEL > 0
    sal_uLong nStrings = SwAutoCompleteString::GetElementCount();
    sal_uLong nClients = SwAutoCompleteClient::GetElementCount();
    OSL_ENSURE(!nStrings && !nClients, "AutoComplete: clients or string count mismatch");
#endif
}

bool SwAutoCompleteWord::InsertWord( const String& rWord, SwDoc& rDoc )
{
    SwDocShell* pDocShell = rDoc.GetDocShell();
    SfxMedium* pMedium = pDocShell ? pDocShell->GetMedium() : 0;
    // strings from help module should not be added
    if( pMedium )
    {
        const INetURLObject& rURL = pMedium->GetURLObject();
        if ( rURL.GetProtocol() == INET_PROT_VND_SUN_STAR_HELP )
            return sal_False;
    }

    String aNewWord(rWord);
    aNewWord = comphelper::string::remove(aNewWord, CH_TXTATR_INWORD);
    aNewWord = comphelper::string::remove(aNewWord, CH_TXTATR_BREAKWORD);

    pImpl->AddDocument(rDoc);
    bool bRet = false;
    xub_StrLen nWrdLen = aNewWord.Len();
    while( nWrdLen && '.' == aNewWord.GetChar( nWrdLen-1 ))
        --nWrdLen;

    if( !bLockWordLst && nWrdLen >= nMinWrdLen )
    {
        SwAutoCompleteString* pNew = new SwAutoCompleteString( aNewWord, 0, nWrdLen );
        pNew->AddDocument(rDoc);
        std::pair<editeng::SortedAutoCompleteStrings::const_iterator, bool>
            aInsPair = m_WordList.insert(pNew);

        m_LookupTree.insert( OUString(aNewWord).copy(0, nWrdLen) );

        if (aInsPair.second)
        {
            bRet = true;
            if (aLRULst.size() >= nMaxCount)
            {
                // the last one needs to be removed
                // so that there is space for the first one
                SwAutoCompleteString* pDel = aLRULst.back();
                aLRULst.pop_back();
                m_WordList.erase(pDel);
                delete pDel;
            }
            aLRULst.push_front(pNew);
        }
        else
        {
            delete pNew;
            // then move "up"
            pNew = (SwAutoCompleteString*)(*aInsPair.first);

            // add the document to the already inserted string
            pNew->AddDocument(rDoc);

            // move pNew to the front of the LRU list
            SwAutoCompleteStringPtrDeque::iterator it = std::find( aLRULst.begin(), aLRULst.end(), pNew );
            OSL_ENSURE( aLRULst.end() != it, "String not found" );
            if ( aLRULst.begin() != it && aLRULst.end() != it )
            {
                aLRULst.erase( it );
                aLRULst.push_front( pNew );
            }
        }
    }
    return bRet;
}

void SwAutoCompleteWord::SetMaxCount( sal_uInt16 nNewMax )
{
    if( nNewMax < nMaxCount && aLRULst.size() > nNewMax )
    {
        // remove the trailing ones
        sal_uInt16 nLRUIndex = nNewMax-1;
        while (nNewMax < m_WordList.size() && nLRUIndex < aLRULst.size())
        {
            editeng::SortedAutoCompleteStrings::const_iterator it =
                m_WordList.find(aLRULst[ nLRUIndex++ ]);
            OSL_ENSURE( m_WordList.end() != it, "String not found" );
            editeng::IAutoCompleteString *const pDel = *it;
            m_WordList.erase(it - m_WordList.begin());
            delete pDel;
        }
        aLRULst.erase( aLRULst.begin() + nNewMax - 1, aLRULst.end() );
    }
    nMaxCount = nNewMax;
}

void SwAutoCompleteWord::SetMinWordLen( sal_uInt16 n )
{
    // Do you really want to remove all words that are less than the minWrdLen?
    if( n < nMinWrdLen )
    {
        for (size_t nPos = 0; nPos < m_WordList.size(); ++nPos)
            if (m_WordList[ nPos ]->GetAutoCompleteString().getLength() < n)
            {
                SwAutoCompleteString *const pDel =
                    dynamic_cast<SwAutoCompleteString*>(m_WordList[nPos]);
                m_WordList.erase(nPos);

                SwAutoCompleteStringPtrDeque::iterator it = std::find( aLRULst.begin(), aLRULst.end(), pDel );
                OSL_ENSURE( aLRULst.end() != it, "String not found" );
                aLRULst.erase( it );
                --nPos;
                delete pDel;
            }
    }

    nMinWrdLen = n;
}

/** Return all words matching a given prefix
 *
 *  @param aMatch the prefix to search for
 *  @param aWords the words to search in
 */
bool SwAutoCompleteWord::GetWordsMatching(String aMatch, std::vector<String>& aWords) const
{
    OUString aStringRoot = OUString( aMatch );

    std::vector<OUString> suggestions;
    m_LookupTree.findSuggestions(aStringRoot, suggestions);

    if (suggestions.empty())
    {
        return false;
    }

    for (size_t i = 0; i < suggestions.size(); i++)
    {
        aWords.push_back( String(suggestions[i]) );
    }

    return true;
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
            m_WordList.erase(nMyPos);
            SwAutoCompleteStringPtrDeque::iterator it = std::find( aLRULst.begin(), aLRULst.end(), pDel );
            OSL_ENSURE( aLRULst.end() != it, "String not found" );
            aLRULst.erase( it );
            delete pDel;
            if( nMyPos >= --nMyLen )
                break;
        }
    }
    // remove the elements at the end of the array
    if( nMyPos < nMyLen )
    {
        // clear LRU array first then delete the string object
        for( ; nNewPos < nMyLen; ++nNewPos )
        {
            SwAutoCompleteString *const pDel =
                dynamic_cast<SwAutoCompleteString*>(m_WordList[nNewPos]);
            SwAutoCompleteStringPtrDeque::iterator it = std::find( aLRULst.begin(), aLRULst.end(), pDel );
            OSL_ENSURE( aLRULst.end() != it, "String not found" );
            aLRULst.erase( it );
            delete pDel;
        }
        // remove from array
        m_WordList.erase(m_WordList.begin() + nMyPos,
                         m_WordList.begin() + nMyLen);
    }
}

void SwAutoCompleteWord::DocumentDying(const SwDoc& rDoc)
{
    pImpl->RemoveDocument(rDoc);

    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const sal_Bool bDelete = !pACorr->GetSwFlags().bAutoCmpltKeepList;
    for (size_t nPos = m_WordList.size(); nPos; nPos--)
    {
        SwAutoCompleteString *const pCurrent =
            dynamic_cast<SwAutoCompleteString*>(m_WordList[nPos - 1]);
        if(pCurrent->RemoveDocument(rDoc) && bDelete)
        {
            m_WordList.erase(nPos - 1);
            SwAutoCompleteStringPtrDeque::iterator it = std::find( aLRULst.begin(), aLRULst.end(), pCurrent );
            OSL_ENSURE( aLRULst.end() != it, "word not found in LRU list" );
            aLRULst.erase( it );
            delete pCurrent;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
