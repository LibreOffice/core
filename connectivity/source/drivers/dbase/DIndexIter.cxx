/*************************************************************************
 *
 *  $RCSfile: DIndexIter.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:21 $
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
#ifndef _CONNECTIVITY_DBASE_INDEXITER_HXX_
#include "dbase/DIndexIter.hxx"
#endif

using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
//==================================================================
// OIndexIterator
//==================================================================
//------------------------------------------------------------------
OIndexIterator::~OIndexIterator()
{
    //  m_pIndex->UnLock();
    m_pIndex->release();
}

//------------------------------------------------------------------
ULONG OIndexIterator::First()
{
    return Find(TRUE);
}

//------------------------------------------------------------------
ULONG OIndexIterator::Next()
{
    return Find(FALSE);
}
//------------------------------------------------------------------
ULONG OIndexIterator::Find(BOOL bFirst)
{
    //  ONDXIndex* m_pIndex = GetNDXIndex();

    ULONG nRes = STRING_NOTFOUND;
//  if (!m_pIndex->IsOpen())
//      return nRes;

    if (bFirst)
    {
        m_aRoot = m_pIndex->getRoot();
        m_aCurLeaf = NULL;
    }

    if (!m_pOperator)
    {
        // Vorbereitung , auf kleinstes Element positionieren
        if (bFirst)
        {
            ONDXPage* pPage = m_aRoot.getBodyPtr();
            while (pPage && !pPage->IsLeaf())
                pPage = pPage->GetChild(m_pIndex).getBodyPtr();

            m_aCurLeaf = pPage;
            m_nCurNode = NODE_NOTFOUND;
        }
        ONDXKey* pKey = GetNextKey();
        nRes = pKey ? pKey->GetRecord() : STRING_NOTFOUND;
    }
    else if (m_pOperator->IsA(TYPE(OOp_ISNOTNULL)))
        nRes = GetNotNull(bFirst);
    else if (m_pOperator->IsA(TYPE(OOp_ISNULL)))
        nRes = GetNull(bFirst);
    else if (m_pOperator->IsA(TYPE(OOp_LIKE)))
        nRes = GetLike(bFirst);
    else if (m_pOperator->IsA(TYPE(OOp_COMPARE)))
        nRes = GetCompare(bFirst);

    return nRes;
}

//------------------------------------------------------------------
ONDXKey* OIndexIterator::GetFirstKey(ONDXPage* pPage, const OOperand& rKey)
{
    // sucht den vorgegeben key
    // Besonderheit: gelangt der Algorithmus ans Ende
    // wird immer die aktuelle Seite und die Knotenposition vermerkt
    // auf die die Bedingung <= zutrifft
    // dieses findet beim Insert besondere Beachtung
    //  ONDXIndex* m_pIndex = GetNDXIndex();
    OOp_COMPARE aTempOp(SQL_PRED_GREATER);
    USHORT i = 0;

    if (pPage->IsLeaf())
    {
        // im blatt wird die eigentliche Operation ausgefuehrt, sonst die temp. (>)
        while (i < pPage->Count() && !m_pOperator->operate(&((*pPage)[i]).GetKey(),&rKey))
               i++;
    }
    else
        while (i < pPage->Count() && !aTempOp.operate(&((*pPage)[i]).GetKey(),&rKey))
               i++;


    ONDXKey* pFoundKey = NULL;
    if (!pPage->IsLeaf())
    {
        // weiter absteigen
        ONDXPagePtr aPage = (i==0) ? pPage->GetChild(m_pIndex)
                                     : ((*pPage)[i-1]).GetChild(m_pIndex, pPage);
        pFoundKey = aPage.Is() ? GetFirstKey(aPage.getBodyPtr(), rKey) : NULL;
    }
    else if (i == pPage->Count())
    {
        pFoundKey = NULL;
    }
    else
    {
        pFoundKey = &(*pPage)[i].GetKey();
        if (!m_pOperator->operate(pFoundKey,&rKey))
            pFoundKey = NULL;

        m_aCurLeaf = pPage;
        m_nCurNode = pFoundKey ? i : i - 1;
    }
    return pFoundKey;
}

//------------------------------------------------------------------
ULONG OIndexIterator::GetCompare(BOOL bFirst)
{
    ONDXKey* pKey = NULL;
    //  ONDXIndex* m_pIndex = GetNDXIndex();
    OSQLPredicateType ePredicateType = PTR_CAST(file::OOp_COMPARE,m_pOperator)->getPredicateType();

    if (bFirst)
    {
        // Vorbereitung , auf kleinstes Element positionieren
        ONDXPage* pPage = m_aRoot.getBodyPtr();
        switch (ePredicateType)
        {
            case SQL_PRED_NOTEQUAL:
            case SQL_PRED_LESS:
            case SQL_PRED_LESSOREQUAL:
                while (pPage && !pPage->IsLeaf())
                    pPage = pPage->GetChild(m_pIndex).getBodyPtr();

                m_aCurLeaf = pPage;
                m_nCurNode = NODE_NOTFOUND;
        }


        switch (ePredicateType)
        {
            case SQL_PRED_NOTEQUAL:
                while ((pKey = GetNextKey()) && !m_pOperator->operate(pKey,m_pOperand));
                break;
            case SQL_PRED_LESS:
                while ((pKey = GetNextKey()) && !pKey->getValue().hasValue());
                break;
            case SQL_PRED_LESSOREQUAL:
                while (pKey = GetNextKey());
                break;
            case SQL_PRED_GREATEROREQUAL:
            case SQL_PRED_EQUAL:
                pKey = GetFirstKey(m_aRoot.getBodyPtr(),*m_pOperand);
                break;
            case SQL_PRED_GREATER:
                if (!(pKey = GetFirstKey(m_aRoot.getBodyPtr(),*m_pOperand)))
                    while ((pKey = GetNextKey()) && !m_pOperator->operate(pKey,m_pOperand));
        }
    }
    else
    {
        switch (ePredicateType)
        {
            case SQL_PRED_NOTEQUAL:
                while ((pKey = GetNextKey()) && !m_pOperator->operate(pKey,m_pOperand))
                    ;
                break;
            case SQL_PRED_LESS:
            case SQL_PRED_LESSOREQUAL:
            case SQL_PRED_EQUAL:
                if (!(pKey = GetNextKey()) || !m_pOperator->operate(pKey,m_pOperand))
                {
                    pKey = NULL;
                    m_aCurLeaf = NULL;
                }
                break;
            case SQL_PRED_GREATEROREQUAL:
            case SQL_PRED_GREATER:
                pKey = GetNextKey();
        }
    }

    return pKey ? pKey->GetRecord() : STRING_NOTFOUND;
}

//------------------------------------------------------------------
ULONG OIndexIterator::GetLike(BOOL bFirst)
{
    //  ONDXIndex* m_pIndex = GetNDXIndex();
    if (bFirst)
    {
        ONDXPage* pPage = m_aRoot.getBodyPtr();

        while (pPage && !pPage->IsLeaf())
            pPage = pPage->GetChild(m_pIndex).getBodyPtr();

        m_aCurLeaf = pPage;
        m_nCurNode = NODE_NOTFOUND;
    }

    ONDXKey* pKey;
    while ((pKey = GetNextKey()) && !m_pOperator->operate(pKey,m_pOperand))
        ;
    return pKey ? pKey->GetRecord() : STRING_NOTFOUND;
}

//------------------------------------------------------------------
ULONG OIndexIterator::GetNull(BOOL bFirst)
{
    //  ONDXIndex* m_pIndex = GetNDXIndex();
    if (bFirst)
    {
        ONDXPage* pPage = m_aRoot.getBodyPtr();
        while (pPage && !pPage->IsLeaf())
            pPage = pPage->GetChild(m_pIndex).getBodyPtr();

        m_aCurLeaf = pPage;
        m_nCurNode = NODE_NOTFOUND;
    }

    ONDXKey* pKey;
    if (!(pKey = GetNextKey()) || pKey->getValue().hasValue())
    {
        pKey = NULL;
        m_aCurLeaf = NULL;
    }
    return pKey ? pKey->GetRecord() : STRING_NOTFOUND;
}

//------------------------------------------------------------------
ULONG OIndexIterator::GetNotNull(BOOL bFirst)
{
    ONDXKey* pKey;
    //  ONDXIndex* m_pIndex = GetNDXIndex();
    if (bFirst)
    {
        // erst alle NULL werte abklappern
        for (ULONG nRec = GetNull(bFirst);
             nRec != STRING_NOTFOUND;
             nRec = GetNull(FALSE))
                 ;
        pKey = m_aCurLeaf.Is() ? &(*m_aCurLeaf)[m_nCurNode].GetKey() : NULL;
    }
    else
        pKey = GetNextKey();

    return pKey ? pKey->GetRecord() : STRING_NOTFOUND;
}

//------------------------------------------------------------------
ONDXKey* OIndexIterator::GetNextKey()
{
    //  ONDXIndex* m_pIndex = GetNDXIndex();
    if (m_aCurLeaf.Is() && ((++m_nCurNode) >= m_aCurLeaf->Count()))
    {
        ONDXPage* pPage = m_aCurLeaf.getBodyPtr();
        // naechste Seite suchen
        while (pPage)
        {
            ONDXPage* pParentPage = pPage->GetParent().getBodyPtr();
            if (pParentPage)
            {
                USHORT nPos = pParentPage->Search(pPage);
                if (nPos != pParentPage->Count() - 1)
                {   // Seite gefunden
                    pPage = (*pParentPage)[nPos+1].GetChild(m_pIndex,pParentPage).getBodyPtr();
                    break;
                }
            }
            pPage = pParentPage;
        }

        // jetzt wieder zum Blatt
        while (pPage && !pPage->IsLeaf())
            pPage = pPage->GetChild(m_pIndex).getBodyPtr();

        m_aCurLeaf = pPage;
        m_nCurNode = 0;
    }
    return m_aCurLeaf.Is() ? &(*m_aCurLeaf)[m_nCurNode].GetKey() : NULL;
}

