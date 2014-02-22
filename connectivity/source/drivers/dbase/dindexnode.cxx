/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "dbase/dindexnode.hxx"
#include "connectivity/CommonTools.hxx"
#include <osl/thread.h>
#include "dbase/DIndex.hxx"
#include <tools/debug.hxx>
#include "diagnose_ex.h"

#include <algorithm>
#include <boost/scoped_array.hpp>


using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace com::sun::star::sdbc;

ONDXKey::ONDXKey(sal_uInt32 nRec)
    :nRecord(nRec)
{
}

ONDXKey::ONDXKey(const ORowSetValue& rVal, sal_Int32 eType, sal_uInt32 nRec)
    : ONDXKey_BASE(eType)
    , nRecord(nRec)
    , xValue(rVal)
{
}

ONDXKey::ONDXKey(const OUString& aStr, sal_uInt32 nRec)
    : ONDXKey_BASE(::com::sun::star::sdbc::DataType::VARCHAR)
     ,nRecord(nRec)
{
    if (!aStr.isEmpty())
    {
        xValue = aStr;
        xValue.setBound(true);
    }
}


ONDXKey::ONDXKey(double aVal, sal_uInt32 nRec)
    : ONDXKey_BASE(::com::sun::star::sdbc::DataType::DOUBLE)
     ,nRecord(nRec)
     ,xValue(aVal)
{
}





ONDXPage::ONDXPage(ODbaseIndex& rInd, sal_uInt32 nPos, ONDXPage* pParent)
           :nPagePos(nPos)
           ,bModified(sal_False)
           ,nCount(0)
           ,aParent(pParent)
           ,rIndex(rInd)
           ,ppNodes(NULL)
{
    sal_uInt16 nT = rIndex.getHeader().db_maxkeys;
    ppNodes = new ONDXNode[nT];
}


ONDXPage::~ONDXPage()
{
    delete[] ppNodes;
}

void ONDXPage::QueryDelete()
{
    
    if (IsModified() && rIndex.m_pFileStream)
        WriteONDXPage( *rIndex.m_pFileStream, *this );

    bModified = sal_False;
    if (rIndex.UseCollector())
    {
        if (aChild.Is())
            aChild->Release(sal_False);

        for (sal_uInt16 i = 0; i < rIndex.getHeader().db_maxkeys;i++)
        {
            if (ppNodes[i].GetChild().Is())
                ppNodes[i].GetChild()->Release(sal_False);

            ppNodes[i] = ONDXNode();
        }
        RestoreNoDelete();

        nCount = 0;
        aParent.Clear();
        rIndex.Collect(this);
    }
    else
        SvRefBase::QueryDelete();
}

ONDXPagePtr& ONDXPage::GetChild(ODbaseIndex* pIndex)
{
    if (!aChild.Is() && pIndex)
    {
        aChild = rIndex.CreatePage(aChild.GetPagePos(),this,aChild.HasPage());
    }
    return aChild;
}


sal_uInt16 ONDXPage::FindPos(const ONDXKey& rKey) const
{
    
    sal_uInt16 i = 0;
    while (i < nCount && rKey > ((*this)[i]).GetKey())
           i++;

    return i;
}


sal_Bool ONDXPage::Find(const ONDXKey& rKey)
{
    
    
    
    
    sal_uInt16 i = 0;
    while (i < nCount && rKey > ((*this)[i]).GetKey())
           i++;

    sal_Bool bResult = sal_False;

    if (!IsLeaf())
    {
        
        ONDXPagePtr aPage = (i==0) ? GetChild(&rIndex) : ((*this)[i-1]).GetChild(&rIndex, this);
        bResult = aPage.Is() && aPage->Find(rKey);
    }
    else if (i == nCount)
    {
        rIndex.m_aCurLeaf = this;
        rIndex.m_nCurNode = i - 1;
        bResult = sal_False;
    }
    else
    {
        bResult = rKey == ((*this)[i]).GetKey();
        rIndex.m_aCurLeaf = this;
        rIndex.m_nCurNode = bResult ? i : i - 1;
    }
    return bResult;
}


sal_Bool ONDXPage::Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft)
{
    
    
    sal_Bool bAppend = nRowsLeft > 0;
    if (IsFull())
    {
        sal_Bool bResult = sal_True;
        ONDXNode aSplitNode;
        if (bAppend)
            aSplitNode = rNode;
        else
        {
            
            aSplitNode = (*this)[nCount-1];
            if(rNode.GetKey() <= aSplitNode.GetKey())
            {

                
                if (IsLeaf() && this == &rIndex.m_aCurLeaf)
                {
                    
                    --nCount;   
                    bResult = Insert(rIndex.m_nCurNode + 1, rNode);
                }
                else  
                {
                    sal_uInt16 nPos = NODE_NOTFOUND;
                    while (++nPos < nCount && rNode.GetKey() > ((*this)[nPos]).GetKey()) ;

                    --nCount;   
                    bResult = Insert(nPos, rNode);
                }

                
                if (!bResult)
                {
                    nCount++;
                    aSplitNode = rNode;
                }
            }
            else
                aSplitNode = rNode;
        }

        sal_uInt32 nNewPagePos = rIndex.GetPageCount();
        sal_uInt32 nNewPageCount = nNewPagePos + 1;

        
        if (!HasParent())
        {
            
            ONDXPagePtr aNewRoot = rIndex.CreatePage(nNewPagePos + 1);
            aNewRoot->SetChild(this);

            rIndex.m_aRoot = aNewRoot;
            rIndex.SetRootPos(nNewPagePos + 1);
            rIndex.SetPageCount(++nNewPageCount);
        }

        
        ONDXPagePtr aNewPage = rIndex.CreatePage(nNewPagePos,aParent);
        rIndex.SetPageCount(nNewPageCount);

        
        
        ONDXNode aInnerNode;
        if (!IsLeaf() || nRowsLeft < (sal_uInt32)(rIndex.GetMaxNodes() / 2))
            aInnerNode = Split(*aNewPage);
        else
        {
            aInnerNode = (*this)[nCount - 1];

            
            aInnerNode.SetChild(aNewPage);

            
            if (rIndex.isUnique())
                aInnerNode.GetKey().ResetRecord();

            
            if (!IsLeaf())
                aNewPage->SetChild(aInnerNode.GetChild());
        }

        aNewPage->Append(aSplitNode);
        ONDXPagePtr aTempParent = aParent;
        if (IsLeaf())
        {
            rIndex.m_aCurLeaf = aNewPage;
            rIndex.m_nCurNode = rIndex.m_aCurLeaf->Count() - 1;

            
            
            ReleaseFull();
        }

        
        return aTempParent->Insert(aInnerNode);
    }
    else 
    {
        if (bAppend)
        {
            if (IsLeaf())
                rIndex.m_nCurNode = nCount - 1;
            return Append(rNode);
        }
        else
        {
            sal_uInt16 nNodePos = FindPos(rNode.GetKey());
            if (IsLeaf())
                rIndex.m_nCurNode = nNodePos;

            return Insert(nNodePos, rNode);
        }
    }
}


sal_Bool ONDXPage::Insert(sal_uInt16 nPos, ONDXNode& rNode)
{
    sal_uInt16 nMaxCount = rIndex.getHeader().db_maxkeys;
    if (nPos >= nMaxCount)
        return sal_False;

    if (nCount)
    {
        ++nCount;
        
        for (sal_uInt16 i = std::min((sal_uInt16)(nMaxCount-1), (sal_uInt16)(nCount-1)); nPos < i; --i)
            (*this)[i] = (*this)[i-1];
    }
    else
        if (nCount < nMaxCount)
            nCount++;

    
    ONDXNode& rInsertNode = (*this)[nPos];
    rInsertNode = rNode;
    if (rInsertNode.GetChild().Is())
    {
        rInsertNode.GetChild()->SetParent(this);
        rNode.GetChild()->SetParent(this);
    }

    bModified = sal_True;

    return sal_True;
}


sal_Bool ONDXPage::Append(ONDXNode& rNode)
{
    DBG_ASSERT(!IsFull(), "kein Append moeglich");
    return Insert(nCount, rNode);
}

void ONDXPage::Release(sal_Bool bSave)
{
    
    if (aChild.Is())
        aChild->Release(bSave);

    
    aChild.Clear();

    for (sal_uInt16 i = 0; i < rIndex.getHeader().db_maxkeys;i++)
    {
        if (ppNodes[i].GetChild())
            ppNodes[i].GetChild()->Release(bSave);

        ppNodes[i].GetChild().Clear();
    }
    aParent = NULL;
}

void ONDXPage::ReleaseFull(sal_Bool bSave)
{
    ONDXPagePtr aTempParent = aParent;
    Release(bSave);

    if (aTempParent.Is())
    {
        
        
        sal_uInt16 nParentPos = aTempParent->Search(this);
        if (nParentPos != NODE_NOTFOUND)
            (*aTempParent)[nParentPos].GetChild().Clear();
        else
            aTempParent->GetChild().Clear();
    }
}

sal_Bool ONDXPage::Delete(sal_uInt16 nNodePos)
{
    if (IsLeaf())
    {
        
        if (nNodePos == (nCount - 1))
        {
            ONDXNode aNode = (*this)[nNodePos];

            
            if (HasParent())
                aParent->SearchAndReplace(aNode.GetKey(),
                                          (*this)[nNodePos-1].GetKey());
        }
    }

    
    Remove(nNodePos);

    
    if (HasParent() && nCount < (rIndex.GetMaxNodes() / 2))
    {
        
        sal_uInt16 nParentNodePos = aParent->Search(this);
        
        if (nParentNodePos == (aParent->Count() - 1))
        {
            if (!nParentNodePos)
            
                Merge(nParentNodePos,aParent->GetChild(&rIndex));
            else
                Merge(nParentNodePos,(*aParent)[nParentNodePos-1].GetChild(&rIndex,aParent));
        }
        
        else
        {
            
            Merge(nParentNodePos + 1,((*aParent)[nParentNodePos + 1].GetChild(&rIndex,aParent)));
            nParentNodePos++;
        }
        if (HasParent() && !(*aParent)[nParentNodePos].HasChild())
            aParent->Delete(nParentNodePos);
    }
    else if (IsRoot())
        
        rIndex.SetRootPos(nPagePos);
    return sal_True;
}



ONDXNode ONDXPage::Split(ONDXPage& rPage)
{
    DBG_ASSERT(IsFull(), "Falsches Splitting");
    /*  devide one page into two
        leaf:
            Page 1 is (n - (n/2))
            Page 2 is (n/2)
            Node n/2 will be duplicated
        inner node:
            Page 1 is (n+1)/2
            Page 2 is (n/2-1)
            Node ((n+1)/2 + 1) : will be taken out
    */
    ONDXNode aResultNode;
    if (IsLeaf())
    {
        for (sal_uInt16 i = (nCount - (nCount / 2)), j = 0 ; i < nCount; i++)
            rPage.Insert(j++,(*this)[i]);

        
        ONDXNode aLastNode = (*this)[nCount - 1];
        nCount = nCount - (nCount / 2);
        aResultNode = (*this)[nCount - 1];

        if (HasParent())
            aParent->SearchAndReplace(aLastNode.GetKey(),
                                      aResultNode.GetKey());
    }
    else
    {
        for (sal_uInt16 i = (nCount + 1) / 2 + 1, j = 0 ; i < nCount; i++)
            rPage.Insert(j++,(*this)[i]);

        aResultNode = (*this)[(nCount + 1) / 2];
        nCount = (nCount + 1) / 2;

        
        rPage.SetChild(aResultNode.GetChild());
    }
    
    aResultNode.SetChild(&rPage);

    
    if (rIndex.isUnique())
        aResultNode.GetKey().ResetRecord();
    bModified = sal_True;
    return aResultNode;
}


void ONDXPage::Merge(sal_uInt16 nParentNodePos, ONDXPagePtr xPage)
{
    DBG_ASSERT(HasParent(), "kein Vater vorhanden");
    DBG_ASSERT(nParentNodePos != NODE_NOTFOUND, "Falscher Indexaufbau");

    /*  Merge 2 pages   */
    ONDXNode aResultNode;
    sal_uInt16 nMaxNodes = rIndex.GetMaxNodes(),
           nMaxNodes_2 = nMaxNodes / 2;

    
    sal_Bool    bRight    = ((*xPage)[0].GetKey() > (*this)[0].GetKey()); 
    sal_uInt16  nNewCount = (*xPage).Count() + Count();

    if (IsLeaf())
    {
        
        if (nNewCount < (nMaxNodes_2 * 2))
        {
            sal_uInt16 nLastNode = bRight ? Count() - 1 : xPage->Count() - 1;
            if (bRight)
            {
                DBG_ASSERT(&xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                
                while (xPage->Count())
                {
                    Append((*xPage)[0]);
                    xPage->Remove(0);
                }
            }
            else
            {
                DBG_ASSERT(&xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                
                while (xPage->Count())
                {
                    Insert(0,(*xPage)[xPage->Count()-1]);
                    xPage->Remove(xPage->Count()-1);
                }
                
                if (nParentNodePos)
                    (*aParent)[nParentNodePos-1].SetChild(this,aParent);
                else 
                    aParent->SetChild(this);
                aParent->SetModified(sal_True);

            }

            
            (*aParent)[nParentNodePos].SetChild();
            
            if(aParent->IsRoot() && aParent->Count() == 1)
            {
                (*aParent)[0].SetChild();
                aParent->ReleaseFull();
                aParent = NULL;
                rIndex.SetRootPos(nPagePos);
                rIndex.m_aRoot = this;
                SetModified(sal_True);
            }
            else
                aParent->SearchAndReplace((*this)[nLastNode].GetKey(),(*this)[nCount-1].GetKey());

            xPage->SetModified(sal_False);
            xPage->ReleaseFull(); 
        }
        
        else
        {
            if (bRight)
            {
                
                ONDXNode aReplaceNode = (*this)[nCount - 1];
                while (nCount < nMaxNodes_2)
                {
                    Append((*xPage)[0]);
                    xPage->Remove(0);
                }
                
                aParent->SearchAndReplace(aReplaceNode.GetKey(),(*this)[nCount-1].GetKey());
            }
            else
            {
                
                ONDXNode aReplaceNode = (*this)[nCount - 1];
                while (xPage->Count() < nMaxNodes_2)
                {
                    xPage->Insert(0,(*this)[nCount-1]);
                    Remove(nCount-1);
                }
                
                aParent->SearchAndReplace(aReplaceNode.GetKey(),(*this)[Count()-1].GetKey());
            }
        }
    }
    else 
    {
        
        if (nNewCount < nMaxNodes_2 * 2)
        {
            if (bRight)
            {
                DBG_ASSERT(&xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                
                (*aParent)[nParentNodePos].SetChild(xPage->GetChild(),aParent);
                Append((*aParent)[nParentNodePos]);
                for (sal_uInt16 i = 0 ; i < xPage->Count(); i++)
                    Append((*xPage)[i]);
            }
            else
            {
                DBG_ASSERT(&xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                
                (*aParent)[nParentNodePos].SetChild(GetChild(),aParent); 
                Insert(0,(*aParent)[nParentNodePos]); 
                while (xPage->Count())
                {
                    Insert(0,(*xPage)[xPage->Count()-1]);
                    xPage->Remove(xPage->Count()-1);
                }
                SetChild(xPage->GetChild());

                if (nParentNodePos)
                    (*aParent)[nParentNodePos-1].SetChild(this,aParent);
                else
                    aParent->SetChild(this);
            }

            
            (*aParent)[nParentNodePos].SetChild();
            aParent->SetModified(sal_True);

            if(aParent->IsRoot() && aParent->Count() == 1)
            {
                (*aParent).SetChild();
                aParent->ReleaseFull();
                aParent = NULL;
                rIndex.SetRootPos(nPagePos);
                rIndex.m_aRoot = this;
                SetModified(sal_True);
            }
            else if(nParentNodePos)
                
                
                
                aParent->SearchAndReplace((*aParent)[nParentNodePos-1].GetKey(),(*aParent)[nParentNodePos].GetKey());

            xPage->SetModified(sal_False);
            xPage->ReleaseFull();
        }
        
        else
        {
            if (bRight)
            {
                while (nCount < nMaxNodes_2)
                {
                    (*aParent)[nParentNodePos].SetChild(xPage->GetChild(),aParent);
                    Append((*aParent)[nParentNodePos]);
                    (*aParent)[nParentNodePos] = (*xPage)[0];
                    xPage->Remove(0);
                }
                xPage->SetChild((*aParent)[nParentNodePos].GetChild());
                (*aParent)[nParentNodePos].SetChild(xPage,aParent);
            }
            else
            {
                while (nCount < nMaxNodes_2)
                {
                    (*aParent)[nParentNodePos].SetChild(GetChild(),aParent);
                    Insert(0,(*aParent)[nParentNodePos]);
                    (*aParent)[nParentNodePos] = (*xPage)[xPage->Count()-1];
                    xPage->Remove(xPage->Count()-1);
                }
                SetChild((*aParent)[nParentNodePos].GetChild());
                (*aParent)[nParentNodePos].SetChild(this,aParent);

            }
            aParent->SetModified(sal_True);
        }
    }
}





void ONDXNode::Read(SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.ReadUInt32( aKey.nRecord ); 

    if (rIndex.getHeader().db_keytype)
    {
        double aDbl;
        rStream.ReadDouble( aDbl );
        aKey = ONDXKey(aDbl,aKey.nRecord);
    }
    else
    {
        sal_uInt16 nLen = rIndex.getHeader().db_keylen;
        OString aBuf = read_uInt8s_ToOString(rStream, nLen);
        
        sal_Int32 nContentLen = aBuf.getLength();
        while (nContentLen && aBuf[nContentLen-1] == ' ')
            --nContentLen;
        aKey = ONDXKey(OUString(aBuf.getStr(), nContentLen, rIndex.m_pTable->getConnection()->getTextEncoding()) ,aKey.nRecord);
    }
    rStream >> aChild;
}


void ONDXNode::Write(SvStream &rStream, const ONDXPage& rPage) const
{
    const ODbaseIndex& rIndex = rPage.GetIndex();
    if (!rIndex.isUnique() || rPage.IsLeaf())
        rStream.WriteUInt32( (sal_uInt32)aKey.nRecord ); 
    else
        rStream.WriteUInt32( (sal_uInt32)0 );   

    if (rIndex.getHeader().db_keytype) 
    {
        if (sizeof(double) != rIndex.getHeader().db_keylen)
        {
            OSL_TRACE("this key length cannot possibly be right?");
        }
        if (aKey.getValue().isNull())
        {
            sal_uInt8 buf[sizeof(double)];
            memset(&buf[0], 0, sizeof(double));
            rStream.Write(&buf[0], sizeof(double));
        }
        else
            rStream.WriteDouble( (double) aKey.getValue() );
    }
    else
    {
        sal_uInt16 const nLen(rIndex.getHeader().db_keylen);
        ::boost::scoped_array<sal_uInt8> pBuf(new sal_uInt8[nLen]);
        memset(&pBuf[0], 0x20, nLen);
        if (!aKey.getValue().isNull())
        {
            OUString sValue = aKey.getValue();
            OString aText(OUStringToOString(sValue, rIndex.m_pTable->getConnection()->getTextEncoding()));
            strncpy(reinterpret_cast<char *>(&pBuf[0]), aText.getStr(),
                std::min<size_t>(nLen, aText.getLength()));
        }
        rStream.Write(&pBuf[0], nLen);
    }
    WriteONDXPagePtr( rStream, aChild );
}



ONDXPagePtr& ONDXNode::GetChild(ODbaseIndex* pIndex, ONDXPage* pParent)
{
    if (!aChild.Is() && pIndex)
    {
        aChild = pIndex->CreatePage(aChild.GetPagePos(),pParent,aChild.HasPage());
    }
    return aChild;
}





sal_Bool ONDXKey::IsText(sal_Int32 eType)
{
    return eType == DataType::VARCHAR || eType == DataType::CHAR;
}


int ONDXKey::Compare(const ONDXKey& rKey) const
{
    sal_Int32 nRes;

    if (getValue().isNull())
    {
        if (rKey.getValue().isNull() || (rKey.IsText(getDBType()) && rKey.getValue().getString().isEmpty()))
            nRes = 0;
        else
            nRes = -1;
    }
    else if (rKey.getValue().isNull())
    {
        if (getValue().isNull() || (IsText(getDBType()) && getValue().getString().isEmpty()))
            nRes = 0;
        else
            nRes = 1;
    }
    else if (IsText(getDBType()))
    {
        nRes = getValue().getString().compareTo(rKey.getValue());
    }
    else
    {
        double m = getValue();
        double n = rKey.getValue();
        nRes = (m > n) ? 1 : ( m < n) ? -1 : 0;
    }

    
    if (nRes == 0 && nRecord && rKey.nRecord)
    {
        nRes = (nRecord > rKey.nRecord) ? 1 :
            (nRecord == rKey.nRecord) ? 0 : -1;
    }
    return nRes;
}

void ONDXKey::setValue(const ORowSetValue& _rVal)
{
    xValue = _rVal;
}

const ORowSetValue& ONDXKey::getValue() const
{
    return xValue;
}

SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPagePtr& rPage)
{
    rStream.ReadUInt32( rPage.nPagePos );
    return rStream;
}

SvStream& connectivity::dbase::WriteONDXPagePtr(SvStream &rStream, const ONDXPagePtr& rPage)
{
    rStream.WriteUInt32( rPage.nPagePos );
    return rStream;
}





ONDXPagePtr::ONDXPagePtr(const ONDXPagePtr& rRef)
              :ONDXPageRef(rRef)
              ,nPagePos(rRef.nPagePos)
{
}


ONDXPagePtr::ONDXPagePtr(ONDXPage* pRefPage)
              :ONDXPageRef(pRefPage)
              ,nPagePos(0)
{
    if (pRefPage)
        nPagePos = pRefPage->GetPagePos();
}

ONDXPagePtr& ONDXPagePtr::operator=(const ONDXPagePtr& rRef)
{
    ONDXPageRef::operator=(rRef);
    nPagePos = rRef.nPagePos;
    return *this;
}


ONDXPagePtr& ONDXPagePtr::operator= (ONDXPage* pRef)
{
    ONDXPageRef::operator=(pRef);
    nPagePos = (pRef) ? pRef->GetPagePos() : 0;
    return *this;
}

static sal_uInt32 nValue;

SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPage& rPage)
{
    rStream.Seek(rPage.GetPagePos() * DINDEX_PAGE_SIZE);
    rStream.ReadUInt32( nValue ) >> rPage.aChild;
    rPage.nCount = sal_uInt16(nValue);

    for (sal_uInt16 i = 0; i < rPage.nCount; i++)
        rPage[i].Read(rStream, rPage.GetIndex());
    return rStream;
}


SvStream& connectivity::dbase::WriteONDXPage(SvStream &rStream, const ONDXPage& rPage)
{
    
    sal_Size nSize = rPage.GetPagePos() + 1;
    nSize *= DINDEX_PAGE_SIZE;
    if (nSize > rStream.Seek(STREAM_SEEK_TO_END))
    {
        rStream.SetStreamSize(nSize);
        rStream.Seek(rPage.GetPagePos() * DINDEX_PAGE_SIZE);

        char aEmptyData[DINDEX_PAGE_SIZE];
        memset(aEmptyData,0x00,DINDEX_PAGE_SIZE);
        rStream.Write((sal_uInt8*)aEmptyData,DINDEX_PAGE_SIZE);
    }
    sal_uIntPtr nCurrentPos = rStream.Seek(rPage.GetPagePos() * DINDEX_PAGE_SIZE);
    OSL_UNUSED( nCurrentPos );

    nValue = rPage.nCount;
    rStream.WriteUInt32( nValue );
    WriteONDXPagePtr( rStream, rPage.aChild );

    sal_uInt16 i = 0;
    for (; i < rPage.nCount; i++)
        rPage[i].Write(rStream, rPage);

    
    if(i < rPage.rIndex.getHeader().db_maxkeys)
    {
        sal_uIntPtr nTell = rStream.Tell() % DINDEX_PAGE_SIZE;
        sal_uInt16 nBufferSize = rStream.GetBufferSize();
        sal_uIntPtr nRemainSize = nBufferSize - nTell;
        if ( nRemainSize <= nBufferSize )
        {
            char* pEmptyData = new char[nRemainSize];
            memset(pEmptyData,0x00,nRemainSize);
            rStream.Write((sal_uInt8*)pEmptyData,nRemainSize);
            rStream.Seek(nTell);
            delete [] pEmptyData;
        }
    }
    return rStream;
}

#if OSL_DEBUG_LEVEL > 1

void ONDXPage::PrintPage()
{
    OSL_TRACE("\nSDB: -----------Page: %d  Parent: %d  Count: %d  Child: %d-----",
        nPagePos, HasParent() ? aParent->GetPagePos() : 0 ,nCount, aChild.GetPagePos());

    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        ONDXNode rNode = (*this)[i];
        ONDXKey&  rKey = rNode.GetKey();
        if (!IsLeaf())
            rNode.GetChild(&rIndex, this);

        if (rKey.getValue().isNull())
        {
            OSL_TRACE("SDB: [%d,NULL,%d]",rKey.GetRecord(), rNode.GetChild().GetPagePos());
        }
        else if (rIndex.getHeader().db_keytype)
        {
            OSL_TRACE("SDB: [%d,%f,%d]",rKey.GetRecord(), rKey.getValue().getDouble(),rNode.GetChild().GetPagePos());
        }
        else
        {
            OSL_TRACE("SDB: [%d,%s,%d]",rKey.GetRecord(), OUStringToOString(rKey.getValue().getString(), rIndex.m_pTable->getConnection()->getTextEncoding()).getStr(),rNode.GetChild().GetPagePos());
        }
    }
    OSL_TRACE("SDB: -----------------------------------------------");
    if (!IsLeaf())
    {
#if OSL_DEBUG_LEVEL > 1
        GetChild(&rIndex)->PrintPage();
        for (sal_uInt16 i = 0; i < nCount; i++)
        {
            ONDXNode rNode = (*this)[i];
            rNode.GetChild(&rIndex,this)->PrintPage();
        }
#endif
    }
    OSL_TRACE("SDB: ===============================================");
}
#endif

sal_Bool ONDXPage::IsFull() const
{
    return Count() == rIndex.getHeader().db_maxkeys;
}


sal_uInt16 ONDXPage::Search(const ONDXKey& rSearch)
{
    
    sal_uInt16 i = NODE_NOTFOUND;
    while (++i < Count())
        if ((*this)[i].GetKey() == rSearch)
            break;

    return (i < Count()) ? i : NODE_NOTFOUND;
}


sal_uInt16 ONDXPage::Search(const ONDXPage* pPage)
{
    sal_uInt16 i = NODE_NOTFOUND;
    while (++i < Count())
        if (((*this)[i]).GetChild() == pPage)
            break;

    
    return (i < Count()) ? i : NODE_NOTFOUND;
}


void ONDXPage::SearchAndReplace(const ONDXKey& rSearch,
                                  ONDXKey& rReplace)
{
    OSL_ENSURE(rSearch != rReplace,"Invalid here:rSearch == rReplace");
    if (rSearch != rReplace)
    {
        sal_uInt16 nPos = NODE_NOTFOUND;
        ONDXPage* pPage = this;

        while (pPage && (nPos = pPage->Search(rSearch)) == NODE_NOTFOUND)
            pPage = pPage->aParent;

        if (pPage)
        {
            (*pPage)[nPos].GetKey() = rReplace;
            pPage->SetModified(sal_True);
        }
    }
}

ONDXNode& ONDXPage::operator[] (sal_uInt16 nPos)
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
    return ppNodes[nPos];
}


const ONDXNode& ONDXPage::operator[] (sal_uInt16 nPos) const
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
    return ppNodes[nPos];
}

void ONDXPage::Remove(sal_uInt16 nPos)
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");

    for (sal_uInt16 i = nPos; i < (nCount-1); i++)
        (*this)[i] = (*this)[i+1];

    nCount--;
    bModified = sal_True;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
