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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
#if OSL_DEBUG_LEVEL > 1
    static sal_uLong nSwAutoCompleteClientCount;
#endif
public:
    SwAutoCompleteClient(SwAutoCompleteWord& rToTell, SwDoc& rSwDoc);
    SwAutoCompleteClient(const SwAutoCompleteClient& rClient);
    ~SwAutoCompleteClient();

    SwAutoCompleteClient& operator=(const SwAutoCompleteClient& rClient);

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);
    const SwDoc& GetDoc(){return *pDoc;}
#if OSL_DEBUG_LEVEL > 1
    static sal_uLong GetElementCount() {return nSwAutoCompleteClientCount;}
#endif
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
class SwAutoCompleteString : public String
{
#if OSL_DEBUG_LEVEL > 1
    static sal_uLong nSwAutoCompleteStringCount;
#endif
    SwDocPtrVector aSourceDocs;
    public:
        SwAutoCompleteString(const String& rStr, xub_StrLen nPos, xub_StrLen nLen);

        ~SwAutoCompleteString();
        void        AddDocument(const SwDoc& rDoc);
        //returns true if last document reference has been removed
        sal_Bool     RemoveDocument(const SwDoc& rDoc);
#if OSL_DEBUG_LEVEL > 1
    static sal_uLong GetElementCount() {return nSwAutoCompleteStringCount;}
#endif
};
#if OSL_DEBUG_LEVEL > 1
    sal_uLong SwAutoCompleteClient::nSwAutoCompleteClientCount = 0;
    sal_uLong SwAutoCompleteString::nSwAutoCompleteStringCount = 0;
#endif

SwAutoCompleteClient::SwAutoCompleteClient(SwAutoCompleteWord& rToTell, SwDoc& rSwDoc) :
        pAutoCompleteWord(&rToTell),
        pDoc(&rSwDoc)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
#if OSL_DEBUG_LEVEL > 1
    ++nSwAutoCompleteClientCount;
#endif
}

SwAutoCompleteClient::SwAutoCompleteClient(const SwAutoCompleteClient& rClient) :
    SwClient(),
    pAutoCompleteWord(rClient.pAutoCompleteWord),
    pDoc(rClient.pDoc)
{
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
#if OSL_DEBUG_LEVEL > 1
    ++nSwAutoCompleteClientCount;
#endif
}

SwAutoCompleteClient::~SwAutoCompleteClient()
{
#if OSL_DEBUG_LEVEL > 1
    --nSwAutoCompleteClientCount;
#endif
}

SwAutoCompleteClient& SwAutoCompleteClient::operator=(const SwAutoCompleteClient& rClient)
{
    pAutoCompleteWord = rClient.pAutoCompleteWord;
    pDoc = rClient.pDoc;
    if(rClient.GetRegisteredIn())
        rClient.pRegisteredIn->Add(this);
    else if(GetRegisteredIn())
        pRegisteredIn->Remove(this);
    return *this;
}

void SwAutoCompleteClient::Modify(SfxPoolItem *pOld, SfxPoolItem *)
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

SwAutoCompleteString::SwAutoCompleteString(const String& rStr, xub_StrLen nPos, xub_StrLen nLen) :
            String( rStr, nPos, nLen )
{
#if OSL_DEBUG_LEVEL > 1
    ++nSwAutoCompleteStringCount;
#endif
}

SwAutoCompleteString::~SwAutoCompleteString()
{
#if OSL_DEBUG_LEVEL > 1
    --nSwAutoCompleteStringCount;
#endif
}

void SwAutoCompleteString::AddDocument(const SwDoc& rDoc)
{
    SwDocPtrVector::iterator aIt;
    for(aIt = aSourceDocs.begin(); aIt != aSourceDocs.end(); ++aIt)
    {
        if(*aIt == &rDoc)
            return;
    }
    SwDocPtr pNew = &rDoc;
    aSourceDocs.push_back(pNew);
}

sal_Bool SwAutoCompleteString::RemoveDocument(const SwDoc& rDoc)
{
    SwDocPtrVector::iterator aIt;
    for(aIt = aSourceDocs.begin(); aIt != aSourceDocs.end(); ++aIt)
    {
        if(*aIt == &rDoc)
        {
            aSourceDocs.erase(aIt);
            return !aSourceDocs.size();
        }
    }
    return sal_False;
}

SwAutoCompleteWord::SwAutoCompleteWord( sal_uInt16 nWords, sal_uInt16 nMWrdLen )
    : aWordLst( 0, 255 ), aLRULst( 0, 255 ),
    pImpl(new SwAutoCompleteWord_Impl(*this)),
    nMaxCount( nWords ),
    nMinWrdLen( nMWrdLen ),
    bLockWordLst( sal_False )
{
}

SwAutoCompleteWord::~SwAutoCompleteWord()
{
    for(sal_uInt16 nPos = aWordLst.Count(); nPos; nPos--)
    {
        SwAutoCompleteString* pCurrent = (SwAutoCompleteString*)aWordLst[ nPos - 1 ];
        aWordLst.Remove( nPos - 1 );
        delete pCurrent;
    }
    delete pImpl;
#if OSL_DEBUG_LEVEL > 1
    sal_uLong nStrings = SwAutoCompleteString::GetElementCount();
    sal_uLong nClients = SwAutoCompleteClient::GetElementCount();
    DBG_ASSERT(!nStrings && !nClients, "AutoComplete: clients or string count mismatch");
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
    aNewWord.EraseAllChars( CH_TXTATR_INWORD );
    aNewWord.EraseAllChars( CH_TXTATR_BREAKWORD );

    pImpl->AddDocument(rDoc);
    sal_Bool bRet = sal_False;
    xub_StrLen nWrdLen = aNewWord.Len();
    while( nWrdLen && '.' == aNewWord.GetChar( nWrdLen-1 ))
        --nWrdLen;

    if( !bLockWordLst && nWrdLen >= nMinWrdLen )
    {
        SwAutoCompleteString* pAutoString;
        StringPtr pNew = pAutoString = new SwAutoCompleteString( aNewWord, 0, nWrdLen );
        pAutoString->AddDocument(rDoc);
        sal_uInt16 nInsPos;
        if( aWordLst.Insert( pNew, nInsPos ) )
        {
            bRet = sal_True;
            if( aLRULst.Count() < nMaxCount )
                aLRULst.Insert( pNew, 0 );
            else
            {
                // der letzte muss entfernt werden
                // damit der neue vorne Platz hat
                String* pDel = (String*)aLRULst[ nMaxCount - 1 ];

                void** ppData = (void**)aLRULst.GetData();
                memmove( ppData+1, ppData, (nMaxCount - 1) * sizeof( void* ));
                *ppData = pNew;

                aWordLst.Remove( pDel );
                delete (SwAutoCompleteString*)pDel;
            }
        }
        else
        {
            delete (SwAutoCompleteString*)pNew;
            // dann aber auf jedenfall nach "oben" moven
            pNew = aWordLst[ nInsPos ];

            //add the document to the already inserted string
            SwAutoCompleteString* pCurrent = (SwAutoCompleteString*)pNew;
            pCurrent->AddDocument(rDoc);

            nInsPos = aLRULst.GetPos( (void*)pNew );
            OSL_ENSURE( USHRT_MAX != nInsPos, "String nicht gefunden" );
            if( nInsPos )
            {
                void** ppData = (void**)aLRULst.GetData();
                memmove( ppData+1, ppData, nInsPos * sizeof( void* ) );
                *ppData = pNew;
            }
        }
    }
    return bRet;
}

void SwAutoCompleteWord::SetMaxCount( sal_uInt16 nNewMax )
{
    if( nNewMax < nMaxCount && aLRULst.Count() > nNewMax )
    {
        // dann die unten ueberhaengenden entfernen
        sal_uInt16 nLRUIndex = nNewMax-1;
        while( nNewMax < aWordLst.Count() && nLRUIndex < aLRULst.Count())
        {
            sal_uInt16 nPos = aWordLst.GetPos( (String*)aLRULst[ nLRUIndex++ ] );
            OSL_ENSURE( USHRT_MAX != nPos, "String nicht gefunden" );
            void * pDel = aWordLst[nPos];
            aWordLst.Remove(nPos);
            delete (SwAutoCompleteString*)pDel;
        }
        aLRULst.Remove( nNewMax-1, aLRULst.Count() - nNewMax );
    }
    nMaxCount = nNewMax;
}

void SwAutoCompleteWord::SetMinWordLen( sal_uInt16 n )
{
    // will man wirklich alle Worte, die kleiner als die neue Min Laenge
    // sind entfernen?
    // JP 02.02.99 - erstmal nicht.

    // JP 11.03.99 - mal testhalber eingebaut
    if( n < nMinWrdLen )
    {
        for( sal_uInt16 nPos = 0; nPos < aWordLst.Count(); ++nPos  )
            if( aWordLst[ nPos ]->Len() < n )
            {
                void* pDel = aWordLst[ nPos ];
                aWordLst.Remove(nPos);

                sal_uInt16 nDelPos = aLRULst.GetPos( pDel );
                OSL_ENSURE( USHRT_MAX != nDelPos, "String nicht gefunden" );
                aLRULst.Remove( nDelPos );
                --nPos;
                delete (SwAutoCompleteString*)pDel;
            }
    }

    nMinWrdLen = n;
}

sal_Bool SwAutoCompleteWord::GetRange( const String& rWord, sal_uInt16& rStt,
                                    sal_uInt16& rEnd ) const
{
    const StringPtr pStr = (StringPtr)&rWord;
    aWordLst.Seek_Entry( pStr, &rStt );
    rEnd = rStt;

    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    while( rEnd < aWordLst.Count() && rSCmp.isMatch( rWord, *aWordLst[ rEnd ]))
        ++rEnd;

    return rStt < rEnd;
}

void SwAutoCompleteWord::CheckChangedList( const SvStringsISortDtor& rNewLst )
{
    sal_uInt16 nMyLen = aWordLst.Count(), nNewLen = rNewLst.Count();
    sal_uInt16 nMyPos = 0, nNewPos = 0;

    for( ; nMyPos < nMyLen && nNewPos < nNewLen; ++nMyPos, ++nNewPos )
    {
        const StringPtr pStr = rNewLst[ nNewPos ];
        while( aWordLst[ nMyPos ] != pStr )
        {
            void* pDel = aWordLst[ nMyPos ];
            aWordLst.Remove(nMyPos);

            sal_uInt16 nPos = aLRULst.GetPos( pDel );
            OSL_ENSURE( USHRT_MAX != nPos, "String nicht gefunden" );
            aLRULst.Remove( nPos );
            delete (SwAutoCompleteString*)pDel;
            if( nMyPos >= --nMyLen )
                break;
        }
    }
    //remove the elements at the end of the array
    if( nMyPos < nMyLen )
    {
        //clear LRU array first then delete the string object
        for( ; nNewPos < nMyLen; ++nNewPos )
        {
            void* pDel = aWordLst[ nNewPos ];
            sal_uInt16 nPos = aLRULst.GetPos( pDel );
            OSL_ENSURE( USHRT_MAX != nPos, "String nicht gefunden" );
            aLRULst.Remove( nPos );
            delete (SwAutoCompleteString*)pDel;
        }
        //remove from array
        aWordLst.Remove( nMyPos, nMyLen - nMyPos );
    }
}

void SwAutoCompleteWord::DocumentDying(const SwDoc& rDoc)
{
    pImpl->RemoveDocument(rDoc);

    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get()->GetAutoCorrect();
    const sal_Bool bDelete = !pACorr->GetSwFlags().bAutoCmpltKeepList;
    for(sal_uInt16 nPos = aWordLst.Count(); nPos; nPos--)
    {
        SwAutoCompleteString* pCurrent = (SwAutoCompleteString*)aWordLst[ nPos - 1 ];
        if(pCurrent->RemoveDocument(rDoc) && bDelete)
        {
            aWordLst.Remove( nPos - 1 );
            sal_uInt16 nLRUPos = aLRULst.GetPos( (void*)pCurrent );
            DBG_ASSERT(nLRUPos < USHRT_MAX, "word not found in LRU list" );
            aLRULst.Remove( nLRUPos );
            delete pCurrent;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
