/*************************************************************************
 *
 *  $RCSfile: dindexnode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-30 13:57:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_DBASE_INDEXNODE_HXX_
#include "dbase/dindexnode.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace com::sun::star::sdbc;
//==================================================================
// Index Seite
//==================================================================
ONDXPage::ONDXPage(ODbaseIndex& rInd, sal_uInt32 nPos, ONDXPage* pParent)
           :rIndex(rInd)
           ,nPagePos(nPos)
           ,nCount(0)
           ,bModified(FALSE)
           ,ppNodes(NULL)
           ,m_refCount(0)
{
    sal_uInt16 nT=rIndex.getHeader().db_maxkeys;
    ppNodes = new ONDXNode[nT];
    aParent = new ONDXPagePtr(pParent);
    aChild = new ONDXPagePtr();
}

//------------------------------------------------------------------
ONDXPage::~ONDXPage()
{
    delete[] ppNodes;
    delete aParent;
    delete aChild;
}
// -------------------------------------------------------------------------
void ONDXPage::release()
{
    if (! osl_decrementInterlockedCount( &m_refCount ))
        QueryDelete();
}
//------------------------------------------------------------------
void ONDXPage::QueryDelete()
{
    // Ablegen im GarbageCollector
    if (IsModified())
        (*rIndex.m_pFileStream) << *this;

    bModified = FALSE;
    if (rIndex.UseCollector())
    {
        if ((*aChild).Is())
            (*aChild)->Release(FALSE);

        for (USHORT i = 0; i < rIndex.getHeader().db_maxkeys;i++)
        {
            if (ppNodes[i].GetChild().Is())
                ppNodes[i].GetChild()->Release(FALSE);

            ppNodes[i] = ONDXNode();
        }
        //  RestoreNoDelete();

        nCount = 0;
        (*aParent).Clear();
        rIndex.Collect(this);
    }
    else
        delete this;
//  else
//      SvRefBase::QueryDelete();
}
//------------------------------------------------------------------
ONDXPagePtr& ONDXPage::GetChild(ODbaseIndex* pIndex)
{
    if (!(*aChild).Is() && pIndex)
    {
        (*aChild) = rIndex.CreatePage((*aChild).GetPagePos(),this,(*aChild).HasPage());
    }
    return (*aChild);
}

//------------------------------------------------------------------
USHORT ONDXPage::FindPos(const ONDXKey& rKey) const
{
    // sucht nach Platz fuer den vorgegeben key auf einer Seite
    USHORT i = 0;
    while (i < nCount && rKey > ((*this)[i]).GetKey())
           i++;

    return i;
}

//------------------------------------------------------------------
BOOL ONDXPage::Find(const ONDXKey& rKey)
{
    // sucht den vorgegeben key
    // Besonderheit: gelangt der Algorithmus ans Ende
    // wird immer die aktuelle Seite und die Knotenposition vermerkt
    // auf die die Bedingung <= zutrifft
    // dieses findet beim Insert besondere Beachtung
    USHORT i = 0;
    while (i < nCount && rKey > ((*this)[i]).GetKey())
           i++;

    BOOL bResult = FALSE;

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
        bResult = FALSE;
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
BOOL ONDXPage::Insert(ONDXNode& rNode, sal_uInt32 nRowsLeft)
{
    // beim Erzeugen eines Index koennen auch mehrere Knoten eingefuegt werden
    // diese sin dann aufsteigend sortiert
    BOOL bAppend = nRowsLeft > 0;
    if (IsFull())
    {
        BOOL bResult = TRUE;
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
                if (IsLeaf() && this == rIndex.m_aCurLeaf.getBodyPtr())
                {
                    // geht davon aus, dass der Knoten, auf dem die Bedingung (<=)
                    // zutrifft, als m_nCurNode gesetzt ist
                    --nCount;   // (sonst bekomme ich u.U. Assertions und GPFs - 60593)
                    bResult = Insert(rIndex.m_nCurNode + 1, rNode);
                }
                else  // Position unbekannt
                {
                    USHORT nPos = NODE_NOTFOUND;
                    while (++nPos < nCount && rNode.GetKey() > ((*this)[nPos]).GetKey());

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
        ONDXPagePtr aNewPage = rIndex.CreatePage(nNewPagePos,(*aParent).getBodyPtr());
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

            // neue Seite zeigt nun auf Seite des herausgelösten Knoten
            if (!IsLeaf())
                aNewPage->SetChild(aInnerNode.GetChild());
        }

        aNewPage->Append(aSplitNode);
        ONDXPagePtr aTempParent = (*aParent);
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
            USHORT nNodePos = FindPos(rNode.GetKey());
            if (IsLeaf())
                rIndex.m_nCurNode = nNodePos;

            return Insert(nNodePos, rNode);
        }
    }
}

//------------------------------------------------------------------
BOOL ONDXPage::Insert(USHORT nPos, ONDXNode& rNode)
{
    USHORT nMaxCount = rIndex.getHeader().db_maxkeys;
    if (nPos >= nMaxCount)
        return FALSE;

    if (nCount)
    {
        ++nCount;
        // nach rechts verschieben
        for (USHORT i = min(nMaxCount-1, nCount-1); nPos < i; i--)
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

    bModified = TRUE;

    return TRUE;
}

//------------------------------------------------------------------
BOOL ONDXPage::Append(ONDXNode& rNode)
{
    DBG_ASSERT(!IsFull(), "kein Append moeglich");
    return Insert(nCount, rNode);
}

//------------------------------------------------------------------
void ONDXPage::Remove(USHORT nPos)
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");

    for (USHORT i = nPos; i < (nCount-1); i++)
        (*this)[i] = (*this)[i+1];

    nCount--;
    bModified = TRUE;
}

//------------------------------------------------------------------
void ONDXPage::Release(BOOL bSave)
{
    // freigeben der Pages
    if ((*aChild).Is())
        (*aChild)->Release(bSave);

    // Pointer freigeben
    (*aChild).Clear();

    for (USHORT i = 0; i < rIndex.getHeader().db_maxkeys;i++)
    {
        if (ppNodes[i].GetChild().isValid())
            ppNodes[i].GetChild()->Release(bSave);

        ppNodes[i].GetChild().Clear();
    }
    (*aParent) = NULL;
}
//------------------------------------------------------------------
void ONDXPage::ReleaseFull(BOOL bSave)
{
    ONDXPagePtr aTempParent = (*aParent);
    Release(bSave);

    if (aTempParent.Is())
    {
        // Freigeben nicht benoetigter Seiten, danach besteht keine Referenz
        // mehr auf die Seite, danach kann 'this' nicht mehr gueltig sein!!!
        USHORT nParentPos = aTempParent->Search(this);
        if (nParentPos != NODE_NOTFOUND)
            (*aTempParent)[nParentPos].GetChild().Clear();
        else
            aTempParent->GetChild().Clear();
    }
}


//------------------------------------------------------------------
ONDXNode& ONDXPage::operator[] (USHORT nPos)
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
    return ppNodes[nPos];
}

//------------------------------------------------------------------
const ONDXNode& ONDXPage::operator[] (USHORT nPos) const
{
    DBG_ASSERT(nCount > nPos, "falscher Indexzugriff");
    return ppNodes[nPos];
}

//------------------------------------------------------------------
// laeuft rekursiv
void ONDXPage::SearchAndReplace(const ONDXKey& rSearch,
                                  ONDXKey& rReplace)
{
    if (rSearch == rReplace)
        return;

    USHORT nPos = NODE_NOTFOUND;
    ONDXPage* pPage = this;

    while (pPage && (nPos = pPage->Search(rSearch)) == NODE_NOTFOUND)
        pPage = pPage->aParent->getBodyPtr();

    if (pPage)
    {
        (*pPage)[nPos].GetKey() = rReplace;
        pPage->SetModified(TRUE);
    }
}

//------------------------------------------------------------------
USHORT ONDXPage::Search(const ONDXKey& rSearch)
{
    // binare Suche spaeter
    USHORT i = 0xFFFF;
    while (++i < Count())
        if ((*this)[i].GetKey() == rSearch)
            break;

    return (i < Count()) ? i : NODE_NOTFOUND;
}

//------------------------------------------------------------------
USHORT ONDXPage::Search(const ONDXPage* pPage)
{
    USHORT i = 0xFFFF;
    while (++i < Count())
        if (((*this)[i]).GetChild().getBodyPtr() == pPage)
            break;

    // wenn nicht gefunden, dann wird davon ausgegangen, dass die Seite selbst
    // auf die Page zeigt
    return (i < Count()) ? i : NODE_NOTFOUND;
}

//------------------------------------------------------------------
BOOL ONDXPage::Delete(USHORT nNodePos)
{
    if (IsLeaf())
    {
        // Letztes Element wird geloescht
        if (nNodePos == (nCount - 1))
        {
            ONDXNode aNode = (*this)[nNodePos];

            // beim Parent muss nun der KeyValue ausgetauscht werden
            if (HasParent())
                (*aParent)->SearchAndReplace(aNode.GetKey(),
                                          (*this)[nNodePos-1].GetKey());
        }
    }

    // Loeschen des Knoten
    Remove(nNodePos);

    // Unterlauf
    if (HasParent() && nCount < (rIndex.GetMaxNodes() / 2))
    {
        // Feststellen, welcher Knoten auf die Seite zeigt
        USHORT nParentNodePos = (*aParent)->Search(this);
        // letzte Element auf Vaterseite
        // -> zusammenlegen mit vorletzter Seite
        if (nParentNodePos == ((*aParent)->Count() - 1))
        {
            if (!nParentNodePos)
            // zusammenlegen mit linken nachbarn
                Merge(nParentNodePos,(*aParent)->GetChild(&rIndex));
            else
                Merge(nParentNodePos,(*(*aParent))[nParentNodePos-1].GetChild(&rIndex,(*aParent).getBodyPtr()));
        }
        // sonst Seite mit naechster Seite zusammenlegen
        else
        {
            // zusammenlegen mit rechten nachbarn
            Merge(nParentNodePos + 1,((*(*aParent))[nParentNodePos + 1].GetChild(&rIndex,(*aParent).getBodyPtr())));
            nParentNodePos++;
        }
        if (HasParent() && !(*(*aParent))[nParentNodePos].HasChild())
            (*aParent)->Delete(nParentNodePos);
/*
        // letzte Element auf Vaterseite
        // -> zusammenlegen mit vorletzter Seite
        if (nParentNodePos == ((*aParent)->Count() - 1))
        {
            if (!nParentNodePos)
            // zusammenlegen mit linken nachbarn
                Merge(nParentNodePos,(*aParent)->GetChild(&rIndex));
            else
                Merge(nParentNodePos,(*(*aParent))[nParentNodePos-1].GetChild(&rIndex,(*aParent)));
        }
        // sonst Seite mit naechster Seite zusammenlegen
        else if(nParentNodePos != NODE_NOTFOUND)
        {
            // zusammenlegen mit rechten nachbarn
            Merge(nParentNodePos + 1,((*(*aParent))[nParentNodePos + 1].GetChild(&rIndex,(*aParent))));
            nParentNodePos++;
        }
        else // Sonderbehandlung
        {
            // Page ist (*aChild) Page vom Parent => erste Page aus ppNodes an (*aChild) anhängen
            Merge(0,(*(*aParent))[0].GetChild(&rIndex,(*aParent)));
            nParentNodePos = 0;
        }

        if (HasParent() && !(*(*aParent))[nParentNodePos].HasChild())
            (*aParent)->Delete(nParentNodePos);
*/

    }
    else if (IsRoot())
        // Sicherstellen das die Position der Wurzel festgehalten wird
        rIndex.SetRootPos(nPagePos);
    return TRUE;
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
        for (USHORT i = (nCount - (nCount / 2)), j = 0 ; i < nCount; i++)
            rPage.Insert(j++,(*this)[i]);

        // dieser Knoten enthaelt einen Schluessel der noch einmal im Tree vorkommt
        // und ersetzt werden muss
        ONDXNode aLastNode = (*this)[nCount - 1];
        nCount = nCount - (nCount / 2);
        aResultNode = (*this)[nCount - 1];

        if (HasParent())
            (*aParent)->SearchAndReplace(aLastNode.GetKey(),
                                      aResultNode.GetKey());
    }
    else
    {
        for (USHORT i = (nCount + 1) / 2 + 1, j = 0 ; i < nCount; i++)
            rPage.Insert(j++,(*this)[i]);

        aResultNode = (*this)[(nCount + 1) / 2];
        nCount = (nCount + 1) / 2;

        // neue Seite zeigt nun auf Seite des herausgelösten Knoten
        rPage.SetChild(aResultNode.GetChild());
    }
    // Knoten zeigt auf neue Seite
    aResultNode.SetChild(&rPage);

    // innere Knoten haben keine Recordnummer
    if (rIndex.isUnique())
        aResultNode.GetKey().ResetRecord();
    bModified = TRUE;
    return aResultNode;
}

//------------------------------------------------------------------
void ONDXPage::Merge(USHORT nParentNodePos, ONDXPagePtr xPage)
{
    DBG_ASSERT(HasParent(), "kein Vater vorhanden");
    DBG_ASSERT(nParentNodePos != NODE_NOTFOUND, "Falscher Indexaufbau");

    /*  Zusammenlegen zweier Seiten */
    ONDXNode aResultNode;
    USHORT nMaxNodes = rIndex.GetMaxNodes(),
           nMaxNodes_2 = nMaxNodes / 2;

    // Feststellen ob Seite rechter oder linker Nachbar
    BOOL    bRight    = ((*xPage)[0].GetKey() > (*this)[0].GetKey()); // TRUE, wenn xPage die rechte Seite ist
    USHORT  nNewCount = (*xPage).Count() + Count();

    if (IsLeaf())
    {
        // Bedingung fuers zusammenlegen
        if (nNewCount < (nMaxNodes_2 * 2))
        {
            USHORT nLastNode = bRight ? Count() - 1 : xPage->Count() - 1;
            if (bRight)
            {
                DBG_ASSERT(xPage.getBodyPtr() != this,"xPage und THIS dürfen nicht gleich sein: Endlosschleife");
                // alle Knoten aus xPage auf den linken Knoten verschieben (anhängen)
                while (xPage->Count())
                {
                    Append((*xPage)[0]);
                    xPage->Remove(0);
                }
            }
            else
            {
                DBG_ASSERT(xPage.getBodyPtr() != this,"xPage und THIS dürfen nicht gleich sein: Endlosschleife");
                // xPage ist die linke Page und THIS die rechte
                while (xPage->Count())
                {
                    Insert(0,(*xPage)[xPage->Count()-1]);
                    xPage->Remove(xPage->Count()-1);
                }
                // alte Position von xPage beim Parent mit this ersetzen
                if (nParentNodePos)
                    (*(*aParent))[nParentNodePos-1].SetChild(this,(*aParent).getBodyPtr());
                else // oder als rechten Knoten setzen
                    (*aParent)->SetChild(this);
                (*aParent)->SetModified(TRUE);

            }

            // Child beziehung beim Vaterknoten aufheben
            (*(*aParent))[nParentNodePos].SetChild();
            // Austauschen des KnotenWertes, nur wenn geaenderte Page
            // die linke ist, ansonsten werde

            if((*aParent)->IsRoot() && (*aParent)->Count() == 1)
            {
                (*(*aParent))[0].SetChild();
                (*aParent)->ReleaseFull();
                (*aParent) = NULL;
                rIndex.SetRootPos(nPagePos);
                rIndex.m_aRoot = this;
                SetModified(TRUE);
            }
            else
                (*aParent)->SearchAndReplace((*this)[nLastNode].GetKey(),(*this)[nCount-1].GetKey());

            xPage->SetModified(FALSE);
            xPage->ReleaseFull(); // wird nicht mehr benoetigt
        }
        // Ausgleichen der Elemente   nNewCount >= (nMaxNodes_2 * 2)
        else
        {
            if (bRight)
            {
                // alle Knoten aus xPage auf den linken Knoten verschieben (anhängen)
                ONDXNode aReplaceNode = (*this)[nCount - 1];
                while (nCount < nMaxNodes_2)
                {
                    Append((*xPage)[0]);
                    xPage->Remove(0);
                }
                // Austauschen des KnotenWertes: Setzt alten letzten Wert durch den letzten von xPage
                (*aParent)->SearchAndReplace(aReplaceNode.GetKey(),(*this)[nCount-1].GetKey());
            }
            else
            {
                // alle Knoten aus this vor die xPage Knoten einfügen
                ONDXNode aReplaceNode = (*this)[nCount - 1];
                while (xPage->Count() < nMaxNodes_2)
                {
                    xPage->Insert(0,(*this)[nCount-1]);
                    Remove(nCount-1);
                }
                // Austauschen des KnotenWertes
                (*aParent)->SearchAndReplace(aReplaceNode.GetKey(),(*this)[Count()-1].GetKey());
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
                DBG_ASSERT(xPage.getBodyPtr() != this,"xPage und THIS dürfen nicht gleich sein: Endlosschleife");
                // Vaterknoten wird mit integriert
                // erhaelt zunaechst Child von xPage
                (*(*aParent))[nParentNodePos].SetChild(xPage->GetChild(),(*aParent).getBodyPtr());
                Append((*(*aParent))[nParentNodePos]);
                for (USHORT i = 0 ; i < xPage->Count(); i++)
                    Append((*xPage)[i]);
            }
            else
            {
                DBG_ASSERT(xPage.getBodyPtr() != this,"xPage und THIS dürfen nicht gleich sein: Endlosschleife");
                // Vaterknoten wird mit integriert
                // erhaelt zunaechst Child
                (*(*aParent))[nParentNodePos].SetChild(GetChild(),(*aParent).getBodyPtr()); // Parent merkt sich mein Child
                Insert(0,(*(*aParent))[nParentNodePos]); // Node vom Parent bei mir einfügen
                while (xPage->Count())
                {
                    Insert(0,(*xPage)[xPage->Count()-1]);
                    xPage->Remove(xPage->Count()-1);
                }
                SetChild(xPage->GetChild());

                if (nParentNodePos)
                    (*(*aParent))[nParentNodePos-1].SetChild(this,(*aParent).getBodyPtr());
                else
                    (*aParent)->SetChild(this);
            }

            // danach wird der Vaterknoten zurueckgesetzt
            (*(*aParent))[nParentNodePos].SetChild();
            (*aParent)->SetModified(TRUE);

            if((*aParent)->IsRoot() && (*aParent)->Count() == 1)
            {
                (*(*aParent)).SetChild();
                (*aParent)->ReleaseFull();
                (*aParent) = NULL;
                rIndex.SetRootPos(nPagePos);
                rIndex.m_aRoot = this;
                SetModified(TRUE);
            }
            else if(nParentNodePos)
                // Austauschen des KnotenWertes
                // beim Append wird der Bereich erweitert, beim INsert verweist der alte Knoten von xPage auf this
                // deshalb muß der Knoten auch hier aktualisiert werden
                (*aParent)->SearchAndReplace((*(*aParent))[nParentNodePos-1].GetKey(),(*(*aParent))[nParentNodePos].GetKey());

            xPage->SetModified(FALSE);
            xPage->ReleaseFull();
        }
        // Ausgleichen der Elemente
        else
        {
            if (bRight)
            {
                while (nCount < nMaxNodes_2)
                {
                    (*(*aParent))[nParentNodePos].SetChild(xPage->GetChild(),(*aParent).getBodyPtr());
                    Append((*(*aParent))[nParentNodePos]);
                    (*(*aParent))[nParentNodePos] = (*xPage)[0];
                    xPage->Remove(0);
                }
                xPage->SetChild((*(*aParent))[nParentNodePos].GetChild());
                (*(*aParent))[nParentNodePos].SetChild(xPage,(*aParent).getBodyPtr());
            }
            else
            {
                while (nCount < nMaxNodes_2)
                {
                    (*(*aParent))[nParentNodePos].SetChild(GetChild(),(*aParent).getBodyPtr());
                    Insert(0,(*(*aParent))[nParentNodePos]);
                    (*(*aParent))[nParentNodePos] = (*xPage)[xPage->Count()-1];
                    xPage->Remove(xPage->Count()-1);
                }
                SetChild((*(*aParent))[nParentNodePos].GetChild());
                (*(*aParent))[nParentNodePos].SetChild(this,(*aParent).getBodyPtr());

            }
            (*aParent)->SetModified(TRUE);
        }
    }
}
// -------------------------------------------------------------------------
BOOL ONDXPage::IsFull() const
{
    return Count() == rIndex.getHeader().db_maxkeys;
}

#ifdef DEBUG
//------------------------------------------------------------------
void ONDXPage::PrintPage()
{
    DBG_TRACE4("\nSDB: -----------Page: %d  Parent: %d  Count: %d  Child: %d-----",
        nPagePos, HasParent() ? (*aParent)->GetPagePos() : 0 ,nCount, (*aChild).GetPagePos());

    for (USHORT i = 0; i < nCount; i++)
    {
        ONDXNode rNode = (*this)[i];
        ONDXKey&  rKey = rNode.GetKey();
        if (!IsLeaf())
            rNode.GetChild(&rIndex, this);

        if (!rKey.getValue().hasValue())
        {
            DBG_TRACE2("SDB: [%d,NULL,%d]",rKey.GetRecord(), rNode.GetChild().GetPagePos());
        }
        else if (rIndex.getHeader().db_keytype)
        {
            DBG_TRACE3("SDB: [%d,%f,%d]",rKey.GetRecord(), getDouble(rKey.getValue()),rNode.GetChild().GetPagePos());
        }
        else
        {
            DBG_TRACE3("SDB: [%d,%s,%d]",rKey.GetRecord(), (const char* )ByteString(getString(rKey.getValue()).getStr(), rIndex.m_pTable->getConnection()->getTextEncoding()).GetBuffer(),rNode.GetChild().GetPagePos());
        }
    }
    DBG_TRACE("SDB: -----------------------------------------------\n");
    if (!IsLeaf())
    {
        GetChild(&rIndex)->PrintPage();
        for (USHORT i = 0; i < nCount; i++)
        {
            ONDXNode rNode = (*this)[i];
            rNode.GetChild(&rIndex,this)->PrintPage();
        }
    }
    DBG_TRACE("SDB: ===============================================\n");
}
#endif


static UINT32 nValue;
//------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ONDXPage& rPage)
{
    rStream.Seek(rPage.GetPagePos() * 512);
    rStream >> nValue >> (*rPage.aChild);
    rPage.nCount = USHORT(nValue);

//  DBG_ASSERT(rPage.nCount && rPage.nCount < rPage.GetIndex().GetMaxNodes(), "Falscher Count");
    for (USHORT i = 0; i < rPage.nCount; i++)
        rPage[i].Read(rStream, rPage.GetIndex());
    return rStream;
}

//------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, const ONDXPage& rPage)
{
    // Seite existiert noch nicht
    if ((rPage.GetPagePos() + 1) * 512 > rStream.Seek(STREAM_SEEK_TO_END))
        rStream.SetStreamSize((rPage.GetPagePos() + 1) * 512);
    rStream.Seek(rPage.GetPagePos() * 512);

    nValue = rPage.nCount;
    rStream << nValue << (*rPage.aChild);

    for (USHORT i = 0; i < rPage.nCount; i++)
        rPage[i].Write(rStream, rPage);
    return rStream;
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
        USHORT nLen = rIndex.getHeader().db_keylen;
        char* pStr = aBuf.AllocBuffer(nLen+1);

        rStream.Read(pStr,nLen);
        pStr[nLen] = 0;
        aBuf.ReleaseBufferAccess();
        aBuf.EraseTrailingChars();

        //  aKey = ONDXKey((aBuf,rIndex.GetDBFConnection()->GetCharacterSet()) ,aKey.nRecord);
        aKey = ONDXKey((aBuf,rIndex.m_pTable->getConnection()->getTextEncoding()) ,aKey.nRecord);
    }
    rStream >> aChild;
}

union
{
    double aDbl;
    char   aData[128];
} aNodeData;
//------------------------------------------------------------------
void ONDXNode::Write(SvStream &rStream, const ONDXPage& rPage) const
{
    const ODbaseIndex& rIndex = rPage.GetIndex();
    if (!rIndex.isUnique() || rPage.IsLeaf())
        rStream << aKey.nRecord; // schluessel
    else
        rStream << (sal_uInt32)0;   // schluessel

    if (rIndex.getHeader().db_keytype) // double
    {
        if (!aKey.getValue().hasValue())
        {
            memset(aNodeData.aData,0,rIndex.getHeader().db_keylen);
            rStream.Write((BYTE*)aNodeData.aData,rIndex.getHeader().db_keylen);
        }
        else
            rStream << (double) getDouble(aKey.getValue());
    }
    else
    {
        memset(aNodeData.aData,0x20,rIndex.getHeader().db_keylen);
        if (aKey.getValue().hasValue())
        {
            ByteString aText(getString(aKey.getValue()).getStr(), rIndex.m_pTable->getConnection()->getTextEncoding());
            strncpy(aNodeData.aData,aText.GetBuffer(),min(rIndex.getHeader().db_keylen, aText.Len()));
        }
        rStream.Write((BYTE*)aNodeData.aData,rIndex.getHeader().db_keylen);
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
BOOL ONDXKey::IsText(sal_Int32 eType)
{
    return eType == DataType::VARCHAR || eType == DataType::CHAR;
}

//------------------------------------------------------------------
StringCompare ONDXKey::Compare(const ONDXKey& rKey) const
{
    //  DBG_ASSERT(is(), "Falscher Indexzugriff");
    StringCompare eResult;

    if (!getValue().getValue() || !getValue().hasValue())
    {
        if (!rKey.getValue().getValue() || !rKey.getValue().hasValue() || (rKey.IsText(getDBType()) && !getString(rKey.getValue()).getLength()))
            eResult = COMPARE_EQUAL;
        else
            eResult = COMPARE_LESS;
    }
    else if (!rKey.getValue().getValue() || !rKey.getValue().hasValue())
    {
        if (!getValue().getValue() || !getValue().hasValue() || (IsText(getDBType()) && !getString(getValue()).getLength()))
            eResult = COMPARE_EQUAL;
        else
            eResult = COMPARE_GREATER;
    }
    else if (IsText(getDBType()))
    {
        INT32 nRes = getString(getValue()).compareTo(getString(rKey.getValue()));
        eResult = (nRes > 0) ? COMPARE_GREATER : (nRes == 0) ? COMPARE_EQUAL : COMPARE_LESS;
    }
    else
    {
        double m,n;
        getValue() >>= m;
        rKey.getValue() >>= n;

        eResult = (m > n) ? COMPARE_GREATER : (n == m) ? COMPARE_EQUAL : COMPARE_LESS;
    }

    // Record vergleich, wenn Index !Unique
    if (eResult == COMPARE_EQUAL && nRecord && rKey.nRecord)
        eResult = (nRecord > rKey.nRecord) ? COMPARE_GREATER :
                  (nRecord == rKey.nRecord) ? COMPARE_EQUAL : COMPARE_LESS;

    return eResult;
}

