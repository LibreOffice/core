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
#include "precompiled_connectivity.hxx"
#include "dbase/dindexnode.hxx"
#include "connectivity/CommonTools.hxx"
#include <osl/thread.h>
#include "dbase/DIndex.hxx"
#include <tools/debug.hxx>
#include "diagnose_ex.h"

#include <algorithm>


using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace com::sun::star::sdbc;
// -----------------------------------------------------------------------------
ONDXKey::ONDXKey(sal_uInt32 nRec)
    :nRecord(nRec)
{
}
// -----------------------------------------------------------------------------
ONDXKey::ONDXKey(const ORowSetValue& rVal, sal_Int32 eType, sal_uInt32 nRec)
    : ONDXKey_BASE(eType)
    , nRecord(nRec)
    , xValue(rVal)
{
}
// -----------------------------------------------------------------------------
ONDXKey::ONDXKey(const rtl::OUString& aStr, sal_uInt32 nRec)
    : ONDXKey_BASE(::com::sun::star::sdbc::DataType::VARCHAR)
     ,nRecord(nRec)
{
    if (aStr.getLength())
    {
        xValue = aStr;
        xValue.setBound(sal_True);
    }
}
// -----------------------------------------------------------------------------

ONDXKey::ONDXKey(double aVal, sal_uInt32 nRec)
    : ONDXKey_BASE(::com::sun::star::sdbc::DataType::DOUBLE)
     ,nRecord(nRec)
     ,xValue(aVal)
{
}
// -----------------------------------------------------------------------------

//==================================================================
// Index Seite
//==================================================================
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

//------------------------------------------------------------------
ONDXPage::~ONDXPage()
{
    delete[] ppNodes;
    //  delete aParent;
    //  delete aChild;
}
//------------------------------------------------------------------
void ONDXPage::QueryDelete()
{
    // Ablegen im GarbageCollector
    if (IsModified() && rIndex.m_pFileStream)
        (*rIndex.m_pFileStream) << *this;

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
//------------------------------------------------------------------
ONDXPagePtr& ONDXPage::GetChild(ODbaseIndex* pIndex)
{
    if (!aChild.Is() && pIndex)
    {
        aChild = rIndex.CreatePage(aChild.GetPagePos(),this,aChild.HasPage());
    }
    return aChild;
}

//------------------------------------------------------------------
sal_uInt16 ONDXPage::FindPos(const ONDXKey& rKey) const
{
    // sucht nach Platz fuer den vorgegeben key auf einer Seite
    sal_uInt16 i = 0;
    while (i < nCount && rKey > ((*this)[i]).GetKey())
           i++;

    return i;
}

//------------------------------------------------------------------
sal_Bool ONDXPage::Find(const ONDXKey& rKey)
{
    // sucht den vorgegeben key
    // Besonderheit: gelangt der Algorithmus ans Ende
    // wird immer die aktuelle Seite und die Knotenposition vermerkt
    // auf die die Bedingung <= zutrifft
    // dieses findet beim Insert besondere Beachtung
    sal_uInt16 i = 0;
    while (i < nCount && rKey > ((*this)[i]).GetKey())
           i++;

    sal_Bool bResult = sal_False;

    if (!IsLeaf())
    {
        // weiter absteigen
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

//------------------------------------------------------------------
sal_Bool ONDXPage::Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft)
{
    // beim Erzeugen eines Index koennen auch mehrere Knoten eingefuegt werden
    // diese sin dann aufsteigend sortiert
    sal_Bool bAppend = nRowsLeft > 0;
    if (IsFull())
    {
        sal_Bool bResult = sal_True;
        ONDXNode aSplitNode;
        if (bAppend)
            aSplitNode = rNode;
        else
        {
            // merken des letzten Knotens
            aSplitNode = (*this)[nCount-1];
            if(rNode.GetKey() <= aSplitNode.GetKey())
            {

                // und damit habe ich im folgenden praktisch eine Node weniger
                if (IsLeaf() && this == &rIndex.m_aCurLeaf)
                {
                    // geht davon aus, dass der Knoten, auf dem die Bedingung (<=)
                    // zutrifft, als m_nCurNode gesetzt ist
                    --nCount;   // (sonst bekomme ich u.U. Assertions und GPFs - 60593)
                    bResult = Insert(rIndex.m_nCurNode + 1, rNode);
                }
                else  // Position unbekannt
                {
                    sal_uInt16 nPos = NODE_NOTFOUND;
                    while (++nPos < nCount && rNode.GetKey() > ((*this)[nPos]).GetKey()) ;

                    --nCount;   // (sonst bekomme ich u.U. Assertions und GPFs - 60593)
                    bResult = Insert(nPos, rNode);
                }

                // konnte der neue Knoten eingefuegt werden
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

        // Herausgeloesten Knoten beim Vater einfuegen
        if (!HasParent())
        {
            // Kein Vater, dann neue Wurzel
            ONDXPagePtr aNewRoot = rIndex.CreatePage(nNewPagePos + 1);
            aNewRoot->SetChild(this);

            rIndex.m_aRoot = aNewRoot;
            rIndex.SetRootPos(nNewPagePos + 1);
            rIndex.SetPageCount(++nNewPageCount);
        }

        // neues blatt erzeugen und Seite aufteilen
        ONDXPagePtr aNewPage = rIndex.CreatePage(nNewPagePos,aParent);
        rIndex.SetPageCount(nNewPageCount);

        // wieviele Knoten weren noch eingefuegt
        // kommen noch ausreichend, dann koennen die Seiten bis zum Rand vollgestopft werden

        ONDXNode aInnerNode;
        if (!IsLeaf() || nRowsLeft < (sal_uInt32)(rIndex.GetMaxNodes() / 2))
            aInnerNode = Split(*aNewPage);
        else
        {
            aInnerNode = (*this)[nCount - 1];
            //aInnerNode = aSplitNode;

            // Knoten zeigt auf neue Seite
            aInnerNode.SetChild(aNewPage);

            // innere Knoten haben keine Recordnummer
            if (rIndex.isUnique())
                aInnerNode.GetKey().ResetRecord();

            // neue Seite zeigt nun auf Seite des herausgeloesten Knoten
            if (!IsLeaf())
                aNewPage->SetChild(aInnerNode.GetChild());
        }

        aNewPage->Append(aSplitNode);
        ONDXPagePtr aTempParent = aParent;
        if (IsLeaf())
        {
            rIndex.m_aCurLeaf = aNewPage;
            rIndex.m_nCurNode = rIndex.m_aCurLeaf->Count() - 1;

            // Freigeben nicht benoetigter Seiten, danach besteht keine Referenz
            // mehr auf die Seite, danach kann 'this' nicht mehr gueltig sein!!!
            ReleaseFull();
        }

        // Einfuegen des herausgeloesten Knotens
        return aTempParent->Insert(aInnerNode);
    }
    else // Seite einfach weiter auffuellen
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

//------------------------------------------------------------------
sal_Bool ONDXPage::Insert(sal_uInt16 nPos, ONDXNode& rNode)
{
    sal_uInt16 nMaxCount = rIndex.getHeader().db_maxkeys;
    if (nPos >= nMaxCount)
        return sal_False;

    if (nCount)
    {
        ++nCount;
        // nach rechts verschieben
        for (sal_uInt16 i = std::min((sal_uInt16)(nMaxCount-1), (sal_uInt16)(nCount-1)); nPos < i; --i)
            (*this)[i] = (*this)[i-1];
    }
    else
        if (nCount < nMaxCount)
            nCount++;

    // einfuegen an der Position
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

//------------------------------------------------------------------
sal_Bool ONDXPage::Append(ONDXNode& rNode)
{
    DBG_ASSERT(!IsFull(), "kein Append moeglich");
    return Insert(nCount, rNode);
}
//------------------------------------------------------------------
void ONDXPage::Release(sal_Bool bSave)
{
    // freigeben der Pages
    if (aChild.Is())
        aChild->Release(bSave);

    // Pointer freigeben
    aChild.Clear();

    for (sal_uInt16 i = 0; i < rIndex.getHeader().db_maxkeys;i++)
    {
        if (ppNodes[i].GetChild())
            ppNodes[i].GetChild()->Release(bSave);

        ppNodes[i].GetChild().Clear();
    }
    aParent = NULL;
}
//------------------------------------------------------------------
void ONDXPage::ReleaseFull(sal_Bool bSave)
{
    ONDXPagePtr aTempParent = aParent;
    Release(bSave);

    if (aTempParent.Is())
    {
        // Freigeben nicht benoetigter Seiten, danach besteht keine Referenz
        // mehr auf die Seite, danach kann 'this' nicht mehr gueltig sein!!!
        sal_uInt16 nParentPos = aTempParent->Search(this);
        if (nParentPos != NODE_NOTFOUND)
            (*aTempParent)[nParentPos].GetChild().Clear();
        else
            aTempParent->GetChild().Clear();
    }
}
//------------------------------------------------------------------
sal_Bool ONDXPage::Delete(sal_uInt16 nNodePos)
{
    if (IsLeaf())
    {
        // Letztes Element wird geloescht
        if (nNodePos == (nCount - 1))
        {
            ONDXNode aNode = (*this)[nNodePos];

            // beim Parent muss nun der KeyValue ausgetauscht werden
            if (HasParent())
                aParent->SearchAndReplace(aNode.GetKey(),
                                          (*this)[nNodePos-1].GetKey());
        }
    }

    // Loeschen des Knoten
    Remove(nNodePos);

    // Unterlauf
    if (HasParent() && nCount < (rIndex.GetMaxNodes() / 2))
    {
        // Feststellen, welcher Knoten auf die Seite zeigt
        sal_uInt16 nParentNodePos = aParent->Search(this);
        // letzte Element auf Vaterseite
        // -> zusammenlegen mit vorletzter Seite
        if (nParentNodePos == (aParent->Count() - 1))
        {
            if (!nParentNodePos)
            // zusammenlegen mit linken nachbarn
                Merge(nParentNodePos,aParent->GetChild(&rIndex));
            else
                Merge(nParentNodePos,(*aParent)[nParentNodePos-1].GetChild(&rIndex,aParent));
        }
        // sonst Seite mit naechster Seite zusammenlegen
        else
        {
            // zusammenlegen mit rechten nachbarn
            Merge(nParentNodePos + 1,((*aParent)[nParentNodePos + 1].GetChild(&rIndex,aParent)));
            nParentNodePos++;
        }
        if (HasParent() && !(*aParent)[nParentNodePos].HasChild())
            aParent->Delete(nParentNodePos);
/*
        // letzte Element auf Vaterseite
        // -> zusammenlegen mit vorletzter Seite
        if (nParentNodePos == (aParent->Count() - 1))
        {
            if (!nParentNodePos)
            // zusammenlegen mit linken nachbarn
                Merge(nParentNodePos,aParent->GetChild(&rIndex));
            else
                Merge(nParentNodePos,(*aParent)[nParentNodePos-1].GetChild(&rIndex,aParent));
        }
        // sonst Seite mit naechster Seite zusammenlegen
        else if(nParentNodePos != NODE_NOTFOUND)
        {
            // zusammenlegen mit rechten nachbarn
            Merge(nParentNodePos + 1,((*aParent)[nParentNodePos + 1].GetChild(&rIndex,aParent)));
            nParentNodePos++;
        }
        else // Sonderbehandlung
        {
            // Page ist aChild Page vom Parent => erste Page aus ppNodes an aChild anhaengen
            Merge(0,(*aParent)[0].GetChild(&rIndex,aParent));
            nParentNodePos = 0;
        }

        if (HasParent() && !(*aParent)[nParentNodePos].HasChild())
            aParent->Delete(nParentNodePos);
*/

    }
    else if (IsRoot())
        // Sicherstellen das die Position der Wurzel festgehalten wird
        rIndex.SetRootPos(nPagePos);
    return sal_True;
}


//------------------------------------------------------------------
ONDXNode ONDXPage::Split(ONDXPage& rPage)
{
    DBG_ASSERT(IsFull(), "Falsches Splitting");
    /*  Aufteilen einer Seite auf zwei
        Blatt:
            Seite 1 behaelt (n - (n/2))
            Seite 2 erhaelt (n/2)
            Knoten n/2 wird dupliziert
        Innerer Knoten:
            Seite 1 behaelt (n+1)/2
            Seite 2 erhaelt (n/2-1)
            Knoten ((n+1)/2 + 1) : wird herausgenommen
    */
    ONDXNode aResultNode;
    if (IsLeaf())
    {
        for (sal_uInt16 i = (nCount - (nCount / 2)), j = 0 ; i < nCount; i++)
            rPage.Insert(j++,(*this)[i]);

        // dieser Knoten enthaelt einen Schluessel der noch einmal im Tree vorkommt
        // und ersetzt werden muss
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

        // neue Seite zeigt nun auf Seite des herausgeloesten Knoten
        rPage.SetChild(aResultNode.GetChild());
    }
    // Knoten zeigt auf neue Seite
    aResultNode.SetChild(&rPage);

    // innere Knoten haben keine Recordnummer
    if (rIndex.isUnique())
        aResultNode.GetKey().ResetRecord();
    bModified = sal_True;
    return aResultNode;
}

//------------------------------------------------------------------
void ONDXPage::Merge(sal_uInt16 nParentNodePos, ONDXPagePtr xPage)
{
    DBG_ASSERT(HasParent(), "kein Vater vorhanden");
    DBG_ASSERT(nParentNodePos != NODE_NOTFOUND, "Falscher Indexaufbau");

    /*  Zusammenlegen zweier Seiten */
    ONDXNode aResultNode;
    sal_uInt16 nMaxNodes = rIndex.GetMaxNodes(),
           nMaxNodes_2 = nMaxNodes / 2;

    // Feststellen ob Seite rechter oder linker Nachbar
    sal_Bool    bRight    = ((*xPage)[0].GetKey() > (*this)[0].GetKey()); // sal_True, wenn xPage die rechte Seite ist
    sal_uInt16  nNewCount = (*xPage).Count() + Count();

    if (IsLeaf())
    {
        // Bedingung fuers zusammenlegen
        if (nNewCount < (nMaxNodes_2 * 2))
        {
            sal_uInt16 nLastNode = bRight ? Count() - 1 : xPage->Count() - 1;
            if (bRight)
            {
                DBG_ASSERT(&xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                // alle Knoten aus xPage auf den linken Knoten verschieben (anhaengen)
                while (xPage->Count())
                {
                    Append((*xPage)[0]);
                    xPage->Remove(0);
                }
            }
            else
            {
                DBG_ASSERT(&xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                // xPage ist die linke Page und THIS die rechte
                while (xPage->Count())
                {
                    Insert(0,(*xPage)[xPage->Count()-1]);
                    xPage->Remove(xPage->Count()-1);
                }
                // alte Position von xPage beim Parent mit this ersetzen
                if (nParentNodePos)
                    (*aParent)[nParentNodePos-1].SetChild(this,aParent);
                else // oder als rechten Knoten setzen
                    aParent->SetChild(this);
                aParent->SetModified(sal_True);

            }

            // Child beziehung beim Vaterknoten aufheben
            (*aParent)[nParentNodePos].SetChild();
            // Austauschen des KnotenWertes, nur wenn geaenderte Page
            // die linke ist, ansonsten werde

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
            xPage->ReleaseFull(); // wird nicht mehr benoetigt
        }
        // Ausgleichen der Elemente   nNewCount >= (nMaxNodes_2 * 2)
        else
        {
            if (bRight)
            {
                // alle Knoten aus xPage auf den linken Knoten verschieben (anhaengen)
                ONDXNode aReplaceNode = (*this)[nCount - 1];
                while (nCount < nMaxNodes_2)
                {
                    Append((*xPage)[0]);
                    xPage->Remove(0);
                }
                // Austauschen des KnotenWertes: Setzt alten letzten Wert durch den letzten von xPage
                aParent->SearchAndReplace(aReplaceNode.GetKey(),(*this)[nCount-1].GetKey());
            }
            else
            {
                // alle Knoten aus this vor die xPage Knoten einfuegen
                ONDXNode aReplaceNode = (*this)[nCount - 1];
                while (xPage->Count() < nMaxNodes_2)
                {
                    xPage->Insert(0,(*this)[nCount-1]);
                    Remove(nCount-1);
                }
                // Austauschen des KnotenWertes
                aParent->SearchAndReplace(aReplaceNode.GetKey(),(*this)[Count()-1].GetKey());
            }
        }
    }
    else // !IsLeaf()
    {
        // Bedingung fuers zusammenlegen
        if (nNewCount < nMaxNodes_2 * 2)
        {
            if (bRight)
            {
                DBG_ASSERT(&xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                // Vaterknoten wird mit integriert
                // erhaelt zunaechst Child von xPage
                (*aParent)[nParentNodePos].SetChild(xPage->GetChild(),aParent);
                Append((*aParent)[nParentNodePos]);
                for (sal_uInt16 i = 0 ; i < xPage->Count(); i++)
                    Append((*xPage)[i]);
            }
            else
            {
                DBG_ASSERT(&xPage != this,"xPage und THIS duerfen nicht gleich sein: Endlosschleife");
                // Vaterknoten wird mit integriert
                // erhaelt zunaechst Child
                (*aParent)[nParentNodePos].SetChild(GetChild(),aParent); // Parent merkt sich mein Child
                Insert(0,(*aParent)[nParentNodePos]); // Node vom Parent bei mir einfuegen
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

            // danach wird der Vaterknoten zurueckgesetzt
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
                // Austauschen des KnotenWertes
                // beim Append wird der Bereich erweitert, beim INsert verweist der alte Knoten von xPage auf this
                // deshalb muss der Knoten auch hier aktualisiert werden
                aParent->SearchAndReplace((*aParent)[nParentNodePos-1].GetKey(),(*aParent)[nParentNodePos].GetKey());

            xPage->SetModified(sal_False);
            xPage->ReleaseFull();
        }
        // Ausgleichen der Elemente
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
//==================================================================
// ONDXNode
//==================================================================

//------------------------------------------------------------------
void ONDXNode::Read(SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream >> aKey.nRecord; // schluessel

    if (rIndex.getHeader().db_keytype)
    {
        double aDbl;
        rStream >> aDbl;
        aKey = ONDXKey(aDbl,aKey.nRecord);
    }
    else
    {
        ByteString aBuf;
        sal_uInt16 nLen = rIndex.getHeader().db_keylen;
        char* pStr = aBuf.AllocBuffer(nLen+1);

        rStream.Read(pStr,nLen);
        pStr[nLen] = 0;
        aBuf.ReleaseBufferAccess();
        aBuf.EraseTrailingChars();

        //  aKey = ONDXKey((aBuf,rIndex.GetDBFConnection()->GetCharacterSet()) ,aKey.nRecord);
        aKey = ONDXKey(::rtl::OUString(aBuf.GetBuffer(),aBuf.Len(),rIndex.m_pTable->getConnection()->getTextEncoding()) ,aKey.nRecord);
    }
    rStream >> aChild;
}

union NodeData
{
    double aDbl;
    char   aData[128];
} aNodeData;
//------------------------------------------------------------------
void ONDXNode::Write(SvStream &rStream, const ONDXPage& rPage) const
{
    const ODbaseIndex& rIndex = rPage.GetIndex();
    if (!rIndex.isUnique() || rPage.IsLeaf())
        rStream << (sal_uInt32)aKey.nRecord; // schluessel
    else
        rStream << (sal_uInt32)0;   // schluessel

    if (rIndex.getHeader().db_keytype) // double
    {
        if (aKey.getValue().isNull())
        {
            memset(aNodeData.aData,0,rIndex.getHeader().db_keylen);
            rStream.Write((sal_uInt8*)aNodeData.aData,rIndex.getHeader().db_keylen);
        }
        else
            rStream << (double) aKey.getValue();
    }
    else
    {
        memset(aNodeData.aData,0x20,rIndex.getHeader().db_keylen);
        if (!aKey.getValue().isNull())
        {
            ::rtl::OUString sValue = aKey.getValue();
            ByteString aText(sValue.getStr(), rIndex.m_pTable->getConnection()->getTextEncoding());
            strncpy(aNodeData.aData,aText.GetBuffer(),std::min(rIndex.getHeader().db_keylen, aText.Len()));
        }
        rStream.Write((sal_uInt8*)aNodeData.aData,rIndex.getHeader().db_keylen);
    }
    rStream << aChild;
}


//------------------------------------------------------------------
ONDXPagePtr& ONDXNode::GetChild(ODbaseIndex* pIndex, ONDXPage* pParent)
{
    if (!aChild.Is() && pIndex)
    {
        aChild = pIndex->CreatePage(aChild.GetPagePos(),pParent,aChild.HasPage());
    }
    return aChild;
}

//==================================================================
// ONDXKey
//==================================================================
//------------------------------------------------------------------
sal_Bool ONDXKey::IsText(sal_Int32 eType)
{
    return eType == DataType::VARCHAR || eType == DataType::CHAR;
}

//------------------------------------------------------------------
StringCompare ONDXKey::Compare(const ONDXKey& rKey) const
{
    //  DBG_ASSERT(is(), "Falscher Indexzugriff");
    StringCompare eResult;

    if (getValue().isNull())
    {
        if (rKey.getValue().isNull() || (rKey.IsText(getDBType()) && !rKey.getValue().getString().getLength()))
            eResult = COMPARE_EQUAL;
        else
            eResult = COMPARE_LESS;
    }
    else if (rKey.getValue().isNull())
    {
        if (getValue().isNull() || (IsText(getDBType()) && !getValue().getString().getLength()))
            eResult = COMPARE_EQUAL;
        else
            eResult = COMPARE_GREATER;
    }
    else if (IsText(getDBType()))
    {
        sal_Int32 nRes = getValue().getString().compareTo(rKey.getValue());
        eResult = (nRes > 0) ? COMPARE_GREATER : (nRes == 0) ? COMPARE_EQUAL : COMPARE_LESS;
    }
    else
    {
        double m = getValue(),n = rKey.getValue();
        eResult = (m > n) ? COMPARE_GREATER : (n == m) ? COMPARE_EQUAL : COMPARE_LESS;
    }

    // Record vergleich, wenn Index !Unique
    if (eResult == COMPARE_EQUAL && nRecord && rKey.nRecord)
        eResult = (nRecord > rKey.nRecord) ? COMPARE_GREATER :
                  (nRecord == rKey.nRecord) ? COMPARE_EQUAL : COMPARE_LESS;

    return eResult;
}
// -----------------------------------------------------------------------------
void ONDXKey::setValue(const ORowSetValue& _rVal)
{
    xValue = _rVal;
}
// -----------------------------------------------------------------------------
const ORowSetValue& ONDXKey::getValue() const
{
    return xValue;
}
// -----------------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPagePtr& rPage)
{
    rStream >> rPage.nPagePos;
    return rStream;
}
// -----------------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, const ONDXPagePtr& rPage)
{
    rStream << rPage.nPagePos;
    return rStream;
}
// -----------------------------------------------------------------------------
//==================================================================
// ONDXPagePtr
//==================================================================
//------------------------------------------------------------------
ONDXPagePtr::ONDXPagePtr(const ONDXPagePtr& rRef)
              :ONDXPageRef(rRef)
              ,nPagePos(rRef.nPagePos)
{
}

//------------------------------------------------------------------
ONDXPagePtr::ONDXPagePtr(ONDXPage* pRefPage)
              :ONDXPageRef(pRefPage)
              ,nPagePos(0)
{
    if (pRefPage)
        nPagePos = pRefPage->GetPagePos();
}
//------------------------------------------------------------------
ONDXPagePtr& ONDXPagePtr::operator=(const ONDXPagePtr& rRef)
{
    ONDXPageRef::operator=(rRef);
    nPagePos = rRef.nPagePos;
    return *this;
}

//------------------------------------------------------------------
ONDXPagePtr& ONDXPagePtr::operator= (ONDXPage* pRef)
{
    ONDXPageRef::operator=(pRef);
    nPagePos = (pRef) ? pRef->GetPagePos() : 0;
    return *this;
}
// -----------------------------------------------------------------------------
static sal_uInt32 nValue;
//------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPage& rPage)
{
    rStream.Seek(rPage.GetPagePos() * PAGE_SIZE);
    rStream >> nValue >> rPage.aChild;
    rPage.nCount = sal_uInt16(nValue);

//  DBG_ASSERT(rPage.nCount && rPage.nCount < rPage.GetIndex().GetMaxNodes(), "Falscher Count");
    for (sal_uInt16 i = 0; i < rPage.nCount; i++)
        rPage[i].Read(rStream, rPage.GetIndex());
    return rStream;
}

//------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, const ONDXPage& rPage)
{
    // Seite existiert noch nicht
    sal_uIntPtr nSize = (rPage.GetPagePos() + 1) * PAGE_SIZE;
    if (nSize > rStream.Seek(STREAM_SEEK_TO_END))
    {
        rStream.SetStreamSize(nSize);
        rStream.Seek(rPage.GetPagePos() * PAGE_SIZE);

        char aEmptyData[PAGE_SIZE];
        memset(aEmptyData,0x00,PAGE_SIZE);
        rStream.Write((sal_uInt8*)aEmptyData,PAGE_SIZE);
    }
    sal_uIntPtr nCurrentPos = rStream.Seek(rPage.GetPagePos() * PAGE_SIZE);
    OSL_UNUSED( nCurrentPos );

    nValue = rPage.nCount;
    rStream << nValue << rPage.aChild;

    sal_uInt16 i = 0;
    for (; i < rPage.nCount; i++)
        rPage[i].Write(rStream, rPage);

    // check if we have to fill the stream with '\0'
    if(i < rPage.rIndex.getHeader().db_maxkeys)
    {
        sal_uIntPtr nTell = rStream.Tell() % PAGE_SIZE;
        sal_uInt16 nBufferSize = rStream.GetBufferSize();
        sal_uIntPtr nRemainSize = nBufferSize - nTell;
        char* pEmptyData = new char[nRemainSize];
        memset(pEmptyData,0x00,nRemainSize);
        rStream.Write((sal_uInt8*)pEmptyData,nRemainSize);
        rStream.Seek(nTell);
        delete [] pEmptyData;
    }
    return rStream;
}
// -----------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 1
//------------------------------------------------------------------
void ONDXPage::PrintPage()
{
    DBG_TRACE4("\nSDB: -----------Page: %d  Parent: %d  Count: %d  Child: %d-----",
        nPagePos, HasParent() ? aParent->GetPagePos() : 0 ,nCount, aChild.GetPagePos());

    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        ONDXNode rNode = (*this)[i];
        ONDXKey&  rKey = rNode.GetKey();
        if (!IsLeaf())
            rNode.GetChild(&rIndex, this);

        if (rKey.getValue().isNull())
        {
            DBG_TRACE2("SDB: [%d,NULL,%d]",rKey.GetRecord(), rNode.GetChild().GetPagePos());
        }
        else if (rIndex.getHeader().db_keytype)
        {
            DBG_TRACE3("SDB: [%d,%f,%d]",rKey.GetRecord(), rKey.getValue().getDouble(),rNode.GetChild().GetPagePos());
        }
        else
        {
            DBG_TRACE3("SDB: [%d,%s,%d]",rKey.GetRecord(), (const char* )ByteString(rKey.getValue().getString().getStr(), rIndex.m_pTable->getConnection()->getTextEncoding()).GetBuffer(),rNode.GetChild().GetPagePos());
        }
    }
    DBG_TRACE("SDB: -----------------------------------------------\n");
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
    DBG_TRACE("SDB: ===============================================\n");
}
#endif
// -----------------------------------------------------------------------------
sal_Bool ONDXPage::IsFull() const
{
    return Count() == rIndex.getHeader().db_maxkeys;
}
// -----------------------------------------------------------------------------
//------------------------------------------------------------------
sal_uInt16 ONDXPage::Search(const ONDXKey& rSearch)
{
    // binare Suche spaeter
    sal_uInt16 i = NODE_NOTFOUND;
    while (++i < Count())
        if ((*this)[i].GetKey() == rSearch)
            break;

    return (i < Count()) ? i : NODE_NOTFOUND;
}

//------------------------------------------------------------------
sal_uInt16 ONDXPage::Search(const ONDXPage* pPage)
{
    sal_uInt16 i = NODE_NOTFOUND;
    while (++i < Count())
        if (((*this)[i]).GetChild() == pPage)
            break;

    // wenn nicht gefunden, dann wird davon ausgegangen, dass die Seite selbst
    // auf die Page zeigt
    return (i < Count()) ? i : NODE_NOTFOUND;
}
// -----------------------------------------------------------------------------
// laeuft rekursiv
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
// -----------------------------------------------------------------------------
ONDXNode& ONDXPage::operator[] (sal_uInt16 nPos)
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
    return ppNodes[nPos];
}

//------------------------------------------------------------------
const ONDXNode& ONDXPage::operator[] (sal_uInt16 nPos) const
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
    return ppNodes[nPos];
}
// -----------------------------------------------------------------------------
void ONDXPage::Remove(sal_uInt16 nPos)
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");

    for (sal_uInt16 i = nPos; i < (nCount-1); i++)
        (*this)[i] = (*this)[i+1];

    nCount--;
    bModified = sal_True;
}
// -----------------------------------------------------------------------------

