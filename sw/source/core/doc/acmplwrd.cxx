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
    m_LookupTree = new LatinLookupTree(OUString("default"));
}

SwAutoCompleteWord::~SwAutoCompleteWord()
{
    m_WordList.DeleteAndDestroyAll(); // so the assertion below works
    delete m_LookupTree;
    delete pImpl;
#if OSL_DEBUG_LEVEL > 0
    sal_uLong nStrings = SwAutoCompleteString::GetElementCount();
    sal_uLong nClients = SwAutoCompleteClient::GetElementCount();
    OSL_ENSURE(!nStrings && !nClients, "AutoComplete: clients or string count mismatch");
#endif
}

sal_Bool SwAutoCompleteWord::InsertWord( const String& rWord, SwDoc& rDoc )
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
    sal_Bool bRet = sal_False;
    xub_StrLen nWrdLen = aNewWord.Len();
    while( nWrdLen && '.' == aNewWord.GetChar( nWrdLen-1 ))
        --nWrdLen;

    if( !bLockWordLst && nWrdLen >= nMinWrdLen )
    {
        SwAutoCompleteString* pNew = new SwAutoCompleteString( aNewWord, 0, nWrdLen );
        pNew->AddDocument(rDoc);
        std::pair<editeng::SortedAutoCompleteStrings::const_iterator, bool>
            aInsPair = m_WordList.insert(pNew);

        m_LookupTree->insert( OUString(aNewWord) );

        if (aInsPair.second)
        {
            bRet = sal_True;
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
            if (m_WordList[ nPos ]->GetAutoCompleteString().Len() < n)
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

// Resets the current position within the tree to its root node.
void SwAutoCompleteWord::returnToRoot()
{
    m_LookupTree->returnToRoot();
}

// Advances to a given node within the AutoComplete tree.
void SwAutoCompleteWord::gotoNode(OUString sNode)
{
    m_LookupTree->gotoNode( sNode );
}

// Advances from the current position towards the node keyed with cKey.
void SwAutoCompleteWord::advance(const sal_Unicode cKey)
{
    m_LookupTree->advance( cKey );
}

// Goes back one char within the tree, except if the current node is already the root node.
void SwAutoCompleteWord::goBack()
{
    m_LookupTree->goBack();
}

// Returns all words matching a given prefix aMatch. If bIgnoreCurrentPos is set, the current
// position within the tree is ignored and replaced by aMatch.
bool SwAutoCompleteWord::GetWordsMatching(String aMatch, std::vector<String>& aWords, sal_Bool bIgnoreCurrentPos) const
{
    OUString aStringRoot = OUString( aMatch );

    // The lookup tree already contains the information about the root keyword in most cases. Only if we don't trust that
    // information (e.g. if we need some autocompletion for a place other than the main writing area), the location within
    // the tree needs to be refreshed.
    if (bIgnoreCurrentPos)
    {
        m_LookupTree->gotoNode( aStringRoot );
    }

    OUString aAutocompleteWord = m_LookupTree->suggestAutoCompletion();
    if (aAutocompleteWord.isEmpty())
    {
        return false;
    }

    OUString aCompleteWord = aStringRoot + aAutocompleteWord;
    aWords.push_back( String(aCompleteWord) );
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
