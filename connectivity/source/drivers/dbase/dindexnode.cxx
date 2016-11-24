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

#include "dbase/dindexnode.hxx"
#include <connectivity/CommonTools.hxx>
#include <osl/thread.h>
#include "dbase/DIndex.hxx"
#include <tools/debug.hxx>
#include <tools/stream.hxx>

#include <algorithm>
#include <memory>


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


// index page

ONDXPage::ONDXPage(ODbaseIndex& rInd, sal_uInt32 nPos, ONDXPage* pParent)
           :bNoDelete(1)
           ,nRefCount(0)
           ,nPagePos(nPos)
           ,bModified(false)
           ,nCount(0)
           ,aParent(pParent)
           ,rIndex(rInd)
           ,ppNodes(nullptr)
{
    sal_uInt16 nT = rIndex.getHeader().db_maxkeys;
    ppNodes = new ONDXNode[nT];
}


ONDXPage::~ONDXPage()
{
    delete[] ppNodes;
}

void ONDXPage::ReleaseRef()
{
    assert( nRefCount >= 1);
    if(--nRefCount == 0 && !bNoDelete)
    {
        QueryDelete();
    }
}

void ONDXPage::QueryDelete()
{
    // Store in GarbageCollector
    if (IsModified() && rIndex.m_pFileStream)
        WriteONDXPage( *rIndex.m_pFileStream, *this );

    bModified = false;
    if (rIndex.UseCollector())
    {
        if (aChild.Is())
            aChild->Release(false);

        for (sal_uInt16 i = 0; i < rIndex.getHeader().db_maxkeys;i++)
        {
            if (ppNodes[i].GetChild().Is())
                ppNodes[i].GetChild()->Release(false);

            ppNodes[i] = ONDXNode();
        }
        bNoDelete = 1;

        nCount = 0;
        aParent.Clear();
        rIndex.Collect(this);
    }
    else
    {
        // I'm not sure about the original purpose of this line, but right now
        // it serves the purpose that anything that attempts to do an AddFirstRef()
        // after an object is deleted will trip an assert.
        nRefCount = 1 << 30;
        delete this;
    }
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
    // searches the position for the given key in a page
    sal_uInt16 i = 0;
    while (i < nCount && rKey > ((*this)[i]).GetKey())
           i++;

    return i;
}


bool ONDXPage::Find(const ONDXKey& rKey)
{
    // searches the given key
    // Speciality: At the end of the method
    // the actual page and the position of the node, fulfilling the '<=' condition, are saved
    // This is considered at insert.
    sal_uInt16 i = 0;
    while (i < nCount && rKey > ((*this)[i]).GetKey())
           i++;

    bool bResult = false;

    if (!IsLeaf())
    {
        // descend further
        ONDXPagePtr aPage = (i==0) ? GetChild(&rIndex) : ((*this)[i-1]).GetChild(&rIndex, this);
        bResult = aPage.Is() && aPage->Find(rKey);
    }
    else if (i == nCount)
    {
        rIndex.m_aCurLeaf = this;
        rIndex.m_nCurNode = i - 1;
        bResult = false;
    }
    else
    {
        bResult = rKey == ((*this)[i]).GetKey();
        rIndex.m_aCurLeaf = this;
        rIndex.m_nCurNode = bResult ? i : i - 1;
    }
    return bResult;
}


bool ONDXPage::Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft)
{
    // When creating an index there can be multiple nodes added,
    // these are sorted ascending
    bool bAppend = nRowsLeft > 0;
    if (IsFull())
    {
        ONDXNode aSplitNode;
        if (bAppend)
            aSplitNode = rNode;
        else
        {
            // Save the last node
            aSplitNode = (*this)[nCount-1];
            if(rNode.GetKey() <= aSplitNode.GetKey())
            {
                bool bResult = true;
                // this practically reduces the number of nodes by 1
                if (IsLeaf() && this == rIndex.m_aCurLeaf)
                {
                    // assumes, that the node, for which the condition (<=) holds, is stored in m_nCurNode
                    --nCount;   // (otherwise we might get Assertions and GPFs - 60593)
                    bResult = Insert(rIndex.m_nCurNode + 1, rNode);
                }
                else  // position unknown
                {
                    sal_uInt16 nPos = NODE_NOTFOUND;
                    while (++nPos < nCount && rNode.GetKey() > ((*this)[nPos]).GetKey()) ;

                    --nCount;   // (otherwise we might get Assertions and GPFs - 60593)
                    bResult = Insert(nPos, rNode);
                }

                // can the new node be inserted
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

        // insert extracted node into parent node
        if (!HasParent())
        {
            // No parent, then new root
            ONDXPagePtr aNewRoot = rIndex.CreatePage(nNewPagePos + 1);
            aNewRoot->SetChild(this);

            rIndex.m_aRoot = aNewRoot;
            rIndex.SetRootPos(nNewPagePos + 1);
            rIndex.SetPageCount(++nNewPageCount);
        }

        // create new leaf and divide page
        ONDXPagePtr aNewPage = rIndex.CreatePage(nNewPagePos,aParent);
        rIndex.SetPageCount(nNewPageCount);

        // How many nodes are being inserted?
        // Enough, then we can fill the page to the brim
        ONDXNode aInnerNode;
        if (!IsLeaf() || nRowsLeft < (sal_uInt32)(rIndex.GetMaxNodes() / 2))
            aInnerNode = Split(*aNewPage);
        else
        {
            aInnerNode = (*this)[nCount - 1];

            // Node points to the new page
            aInnerNode.SetChild(aNewPage);

            // Inner nodes have no record number
            if (rIndex.isUnique())
                aInnerNode.GetKey().ResetRecord();

            // new page points to the page of the extracted node
            if (!IsLeaf())
                aNewPage->SetChild(aInnerNode.GetChild());
        }

        aNewPage->Append(aSplitNode);
        ONDXPagePtr aTempParent = aParent;
        if (IsLeaf())
        {
            rIndex.m_aCurLeaf = aNewPage;
            rIndex.m_nCurNode = rIndex.m_aCurLeaf->Count() - 1;

            // free not needed pages, there are no references to those on the page
            // afterwards 'this' can't be valid anymore!!!
            ReleaseFull();
        }

        // Insert extracted node
        return aTempParent->Insert(aInnerNode);
    }
    else // Fill the page up
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


bool ONDXPage::Insert(sal_uInt16 nPos, ONDXNode& rNode)
{
    sal_uInt16 nMaxCount = rIndex.getHeader().db_maxkeys;
    if (nPos >= nMaxCount)
        return false;

    if (nCount)
    {
        ++nCount;
        // shift right
        for (sal_uInt16 i = std::min((sal_uInt16)(nMaxCount-1), (sal_uInt16)(nCount-1)); nPos < i; --i)
            (*this)[i] = (*this)[i-1];
    }
    else
        if (nCount < nMaxCount)
            nCount++;

    // insert at the position
    ONDXNode& rInsertNode = (*this)[nPos];
    rInsertNode = rNode;
    if (rInsertNode.GetChild().Is())
    {
        rInsertNode.GetChild()->SetParent(this);
        rNode.GetChild()->SetParent(this);
    }

    bModified = true;

    return true;
}


bool ONDXPage::Append(ONDXNode& rNode)
{
    DBG_ASSERT(!IsFull(), "kein Append moeglich");
    return Insert(nCount, rNode);
}

void ONDXPage::Release(bool bSave)
{
    // free pages
    if (aChild.Is())
        aChild->Release(bSave);

    // free pointer
    aChild.Clear();

    for (sal_uInt16 i = 0; i < rIndex.getHeader().db_maxkeys;i++)
    {
        if (ppNodes[i].GetChild())
            ppNodes[i].GetChild()->Release(bSave);

        ppNodes[i].GetChild().Clear();
    }
    aParent.Clear();
}

void ONDXPage::ReleaseFull(bool bSave)
{
    ONDXPagePtr aTempParent = aParent;
    Release(bSave);

    if (aTempParent.Is())
    {
        // Free pages not needed, there will be no reference anymore to the pages
        // afterwards 'this' can't be valid anymore!!!
        sal_uInt16 nParentPos = aTempParent->Search(this);
        if (nParentPos != NODE_NOTFOUND)
            (*aTempParent)[nParentPos].GetChild().Clear();
        else
            aTempParent->GetChild().Clear();
    }
}

bool ONDXPage::Delete(sal_uInt16 nNodePos)
{
    if (IsLeaf())
    {
        // The last element will not be deleted
        if (nNodePos == (nCount - 1))
        {
            ONDXNode aNode = (*this)[nNodePos];

            // parent's KeyValue has to be replaced
            if (HasParent())
                aParent->SearchAndReplace(aNode.GetKey(),
                                          (*this)[nNodePos-1].GetKey());
        }
    }

    // Delete the node
    Remove(nNodePos);

    // Underflow
    if (HasParent() && nCount < (rIndex.GetMaxNodes() / 2))
    {
        // determine, which node points to the page
        sal_uInt16 nParentNodePos = aParent->Search(this);
        // last element on parent-page -> merge with secondlast page
        if (nParentNodePos == (aParent->Count() - 1))
        {
            if (!nParentNodePos)
            // merge with left neighbour
                Merge(nParentNodePos,aParent->GetChild(&rIndex));
            else
                Merge(nParentNodePos,(*aParent)[nParentNodePos-1].GetChild(&rIndex,aParent));
        }
        // otherwise merge page with next page
        else
        {
            // merge with right neighbour
            Merge(nParentNodePos + 1,((*aParent)[nParentNodePos + 1].GetChild(&rIndex,aParent)));
            nParentNodePos++;
        }
        if (HasParent() && !(*aParent)[nParentNodePos].HasChild())
            aParent->Delete(nParentNodePos);
    }
    else if (IsRoot())
        // make sure that the position of the root is kept
        rIndex.SetRootPos(nPagePos);
    return true;
}


ONDXNode ONDXPage::Split(ONDXPage& rPage)
{
    DBG_ASSERT(IsFull(), "Incorrect Splitting");
    /*  divide one page into two
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

        // this node contains a key that already exists in the tree and must be replaced
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

        // new page points to page with extraced node
        rPage.SetChild(aResultNode.GetChild());
    }
    // node points to new page
    aResultNode.SetChild(&rPage);

    // inner nodes have no record number
    if (rIndex.isUnique())
        aResultNode.GetKey().ResetRecord();
    bModified = true;
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

    // Determine if page is right or left neighbour
    bool    bRight    = ((*xPage)[0].GetKey() > (*this)[0].GetKey()); // sal_True, whenn xPage the right side is
    sal_uInt16  nNewCount = (*xPage).Count() + Count();

    if (IsLeaf())
    {
        // Condition for merge
        if (nNewCount < (nMaxNodes_2 * 2))
        {
            sal_uInt16 nLastNode = bRight ? Count() - 1 : xPage->Count() - 1;
            if (bRight)
            {
                DBG_ASSERT(xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                // shift all nodes from xPage to the left node (append)
                while (xPage->Count())
                {
                    Append((*xPage)[0]);
                    xPage->Remove(0);
                }
            }
            else
            {
                DBG_ASSERT(xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                // xPage is the left page and THIS the right one
                while (xPage->Count())
                {
                    Insert(0,(*xPage)[xPage->Count()-1]);
                    xPage->Remove(xPage->Count()-1);
                }
                // replace old position of xPage in parent with this
                if (nParentNodePos)
                    (*aParent)[nParentNodePos-1].SetChild(this,aParent);
                else // or set as right node
                    aParent->SetChild(this);
                aParent->SetModified(true);

            }

            // cancel Child-relationship at parent node
            (*aParent)[nParentNodePos].SetChild();
            // replace the Node-value, only if changed page is the left one, otherwise become
            if(aParent->IsRoot() && aParent->Count() == 1)
            {
                (*aParent)[0].SetChild();
                aParent->ReleaseFull();
                aParent.Clear();
                rIndex.SetRootPos(nPagePos);
                rIndex.m_aRoot = this;
                SetModified(true);
            }
            else
                aParent->SearchAndReplace((*this)[nLastNode].GetKey(),(*this)[nCount-1].GetKey());

            xPage->SetModified(false);
            xPage->ReleaseFull(); // is not needed anymore
        }
        // balance the elements   nNewCount >= (nMaxNodes_2 * 2)
        else
        {
            if (bRight)
            {
                // shift all nodes from xPage to the left node (append)
                ONDXNode aReplaceNode = (*this)[nCount - 1];
                while (nCount < nMaxNodes_2)
                {
                    Append((*xPage)[0]);
                    xPage->Remove(0);
                }
                // Replace the node values: replace old last value by the last of xPage
                aParent->SearchAndReplace(aReplaceNode.GetKey(),(*this)[nCount-1].GetKey());
            }
            else
            {
                // insert all nodes from this in front of the xPage nodes
                ONDXNode aReplaceNode = (*this)[nCount - 1];
                while (xPage->Count() < nMaxNodes_2)
                {
                    xPage->Insert(0,(*this)[nCount-1]);
                    Remove(nCount-1);
                }
                // Replace the node value
                aParent->SearchAndReplace(aReplaceNode.GetKey(),(*this)[Count()-1].GetKey());
            }
        }
    }
    else // !IsLeaf()
    {
        // Condition for merge
        if (nNewCount < nMaxNodes_2 * 2)
        {
            if (bRight)
            {
                DBG_ASSERT(xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                // Parent node will be integrated; is initialized with Child from xPage
                (*aParent)[nParentNodePos].SetChild(xPage->GetChild(),aParent);
                Append((*aParent)[nParentNodePos]);
                for (sal_uInt16 i = 0 ; i < xPage->Count(); i++)
                    Append((*xPage)[i]);
            }
            else
            {
                DBG_ASSERT(xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                // Parent-node will be integrated; is initialized with child
                (*aParent)[nParentNodePos].SetChild(GetChild(),aParent); // Parent memorizes my child
                Insert(0,(*aParent)[nParentNodePos]); // insert parent node into myself
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

            // afterwards parent node will be reset
            (*aParent)[nParentNodePos].SetChild();
            aParent->SetModified(true);

            if(aParent->IsRoot() && aParent->Count() == 1)
            {
                (*aParent).SetChild();
                aParent->ReleaseFull();
                aParent.Clear();
                rIndex.SetRootPos(nPagePos);
                rIndex.m_aRoot = this;
                SetModified(true);
            }
            else if(nParentNodePos)
                // replace the node value
                // for Append the range will be enlarged, for Insert the old node from xPage will reference to this
                // thats why the node must be updated here
                aParent->SearchAndReplace((*aParent)[nParentNodePos-1].GetKey(),(*aParent)[nParentNodePos].GetKey());

            xPage->SetModified(false);
            xPage->ReleaseFull();
        }
        // balance the elements
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
            aParent->SetModified(true);
        }
    }
}

// ONDXNode


void ONDXNode::Read(SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.ReadUInt32( aKey.nRecord ); // key

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
        //get length minus trailing whitespace
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
        rStream.WriteUInt32( aKey.nRecord ); // key
    else
        rStream.WriteUInt32( 0 );   // key

    if (rIndex.getHeader().db_keytype) // double
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
        ::std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[nLen]);
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


// ONDXKey


bool ONDXKey::IsText(sal_Int32 eType)
{
    return eType == DataType::VARCHAR || eType == DataType::CHAR;
}


int ONDXKey::Compare(const ONDXKey& rKey) const
{
    sal_Int32 nRes;

    if (getValue().isNull())
    {
        if (rKey.getValue().isNull() || (IsText(getDBType()) && rKey.getValue().getString().isEmpty()))
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

    // compare record, if index !Unique
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


// ONDXPagePtr


ONDXPagePtr::ONDXPagePtr(const ONDXPagePtr& rRef)
              :mpPage(rRef.mpPage)
              ,nPagePos(rRef.nPagePos)
{
    if (mpPage != nullptr)
        mpPage->AddNextRef();
}


ONDXPagePtr::ONDXPagePtr(ONDXPage* pRefPage)
              :mpPage(pRefPage)
              ,nPagePos(0)
{
    if (mpPage != nullptr)
        mpPage->AddFirstRef();
    if (pRefPage)
        nPagePos = pRefPage->GetPagePos();
}

ONDXPagePtr& ONDXPagePtr::operator=(ONDXPagePtr const & rOther)
{
    if (rOther.mpPage != nullptr) {
        rOther.mpPage->AddNextRef();
    }
    ONDXPage * pOldObj = mpPage;
    mpPage = rOther.mpPage;
    nPagePos = rOther.nPagePos;
    if (pOldObj != nullptr) {
        pOldObj->ReleaseRef();
    }
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
    // Page doesn't exist yet
    sal_Size nSize = rPage.GetPagePos() + 1;
    nSize *= DINDEX_PAGE_SIZE;
    if (nSize > rStream.Seek(STREAM_SEEK_TO_END))
    {
        rStream.SetStreamSize(nSize);
        rStream.Seek(rPage.GetPagePos() * DINDEX_PAGE_SIZE);

        char aEmptyData[DINDEX_PAGE_SIZE];
        memset(aEmptyData,0x00,DINDEX_PAGE_SIZE);
        rStream.Write(aEmptyData, DINDEX_PAGE_SIZE);
    }
    rStream.Seek(rPage.GetPagePos() * DINDEX_PAGE_SIZE);

    nValue = rPage.nCount;
    rStream.WriteUInt32( nValue );
    WriteONDXPagePtr( rStream, rPage.aChild );

    sal_uInt16 i = 0;
    for (; i < rPage.nCount; i++)
        rPage[i].Write(rStream, rPage);

    // check if we have to fill the stream with '\0'
    if(i < rPage.rIndex.getHeader().db_maxkeys)
    {
        sal_Size nTell = rStream.Tell() % DINDEX_PAGE_SIZE;
        sal_uInt16 nBufferSize = rStream.GetBufferSize();
        sal_Size nRemainSize = nBufferSize - nTell;
        if ( nRemainSize <= nBufferSize )
        {
            std::unique_ptr<char[]> pEmptyData( new char[nRemainSize] );
            memset(pEmptyData.get(), 0x00, nRemainSize);
            rStream.Write(pEmptyData.get(), nRemainSize);
            rStream.Seek(nTell);
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

bool ONDXPage::IsFull() const
{
    return Count() == rIndex.getHeader().db_maxkeys;
}


sal_uInt16 ONDXPage::Search(const ONDXKey& rSearch)
{
    // binary search later
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

    // if not found, then we assume, that the page itself points to the page
    return (i < Count()) ? i : NODE_NOTFOUND;
}

// runs recursively
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
            pPage->SetModified(true);
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
    bModified = true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
