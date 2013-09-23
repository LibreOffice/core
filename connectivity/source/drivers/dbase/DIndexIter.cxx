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

#include "dbase/DIndexIter.hxx"
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <tools/string.hxx>

using namespace ::com::sun::star::sdb;
using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
//==================================================================
// OIndexIterator
//==================================================================
//------------------------------------------------------------------
OIndexIterator::~OIndexIterator()
{
    m_pIndex->release();
}

//------------------------------------------------------------------
sal_uIntPtr OIndexIterator::First()
{
    return Find(sal_True);
}

//------------------------------------------------------------------
sal_uIntPtr OIndexIterator::Next()
{
    return Find(sal_False);
}
//------------------------------------------------------------------
sal_uIntPtr OIndexIterator::Find(sal_Bool bFirst)
{
    sal_uIntPtr nRes = STRING_NOTFOUND;

    if (bFirst)
    {
        m_aRoot = m_pIndex->getRoot();
        m_aCurLeaf = NULL;
    }

    if (!m_pOperator)
    {
        // Preparation, position on the smallest element
        if (bFirst)
        {
            ONDXPage* pPage = m_aRoot;
            while (pPage && !pPage->IsLeaf())
                pPage = pPage->GetChild(m_pIndex);

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
    // searches a given key
    // Speciality: At the end of the algorithm
    // the actual page and the position of the node which fulfil the
    // '<='-condition are saved. this is considered for inserts.
    //  ONDXIndex* m_pIndex = GetNDXIndex();
    OOp_COMPARE aTempOp(SQLFilterOperator::GREATER);
    sal_uInt16 i = 0;

    if (pPage->IsLeaf())
    {
        // in the leaf the actual operation is run, otherwise temp. (>)
        while (i < pPage->Count() && !m_pOperator->operate(&((*pPage)[i]).GetKey(),&rKey))
               i++;
    }
    else
        while (i < pPage->Count() && !aTempOp.operate(&((*pPage)[i]).GetKey(),&rKey))
               i++;


    ONDXKey* pFoundKey = NULL;
    if (!pPage->IsLeaf())
    {
        // descend further
        ONDXPagePtr aPage = (i==0) ? pPage->GetChild(m_pIndex)
                                     : ((*pPage)[i-1]).GetChild(m_pIndex, pPage);
        pFoundKey = aPage.Is() ? GetFirstKey(aPage, rKey) : NULL;
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
sal_uIntPtr OIndexIterator::GetCompare(sal_Bool bFirst)
{
    ONDXKey* pKey = NULL;
    sal_Int32 ePredicateType = PTR_CAST(file::OOp_COMPARE,m_pOperator)->getPredicateType();

    if (bFirst)
    {
        // Preparation, position on the smallest element
        ONDXPage* pPage = m_aRoot;
        switch (ePredicateType)
        {
            case SQLFilterOperator::NOT_EQUAL:
            case SQLFilterOperator::LESS:
            case SQLFilterOperator::LESS_EQUAL:
                while (pPage && !pPage->IsLeaf())
                    pPage = pPage->GetChild(m_pIndex);

                m_aCurLeaf = pPage;
                m_nCurNode = NODE_NOTFOUND;
        }


        switch (ePredicateType)
        {
            case SQLFilterOperator::NOT_EQUAL:
                while ( ( ( pKey = GetNextKey() ) != NULL ) && !m_pOperator->operate(pKey,m_pOperand)) ;
                break;
            case SQLFilterOperator::LESS:
                while ( ( ( pKey = GetNextKey() ) != NULL ) && pKey->getValue().isNull()) ;
                break;
            case SQLFilterOperator::LESS_EQUAL:
                while ( ( pKey = GetNextKey() ) != NULL ) ;
                break;
            case SQLFilterOperator::GREATER_EQUAL:
            case SQLFilterOperator::EQUAL:
                pKey = GetFirstKey(m_aRoot,*m_pOperand);
                break;
            case SQLFilterOperator::GREATER:
                pKey = GetFirstKey(m_aRoot,*m_pOperand);
                if ( !pKey )
                    while ( ( ( pKey = GetNextKey() ) != NULL ) && !m_pOperator->operate(pKey,m_pOperand)) ;
        }
    }
    else
    {
        switch (ePredicateType)
        {
            case SQLFilterOperator::NOT_EQUAL:
                while ( ( ( pKey = GetNextKey() ) != NULL ) && !m_pOperator->operate(pKey,m_pOperand))
                    ;
                break;
            case SQLFilterOperator::LESS:
            case SQLFilterOperator::LESS_EQUAL:
            case SQLFilterOperator::EQUAL:
                if ( ( ( pKey = GetNextKey() ) == NULL )  || !m_pOperator->operate(pKey,m_pOperand))
                {
                    pKey = NULL;
                    m_aCurLeaf = NULL;
                }
                break;
            case SQLFilterOperator::GREATER_EQUAL:
            case SQLFilterOperator::GREATER:
                pKey = GetNextKey();
        }
    }

    return pKey ? pKey->GetRecord() : STRING_NOTFOUND;
}

//------------------------------------------------------------------
sal_uIntPtr OIndexIterator::GetLike(sal_Bool bFirst)
{
    if (bFirst)
    {
        ONDXPage* pPage = m_aRoot;

        while (pPage && !pPage->IsLeaf())
            pPage = pPage->GetChild(m_pIndex);

        m_aCurLeaf = pPage;
        m_nCurNode = NODE_NOTFOUND;
    }

    ONDXKey* pKey;
    while ( ( ( pKey = GetNextKey() ) != NULL ) && !m_pOperator->operate(pKey,m_pOperand))
        ;
    return pKey ? pKey->GetRecord() : STRING_NOTFOUND;
}

//------------------------------------------------------------------
sal_uIntPtr OIndexIterator::GetNull(sal_Bool bFirst)
{
    if (bFirst)
    {
        ONDXPage* pPage = m_aRoot;
        while (pPage && !pPage->IsLeaf())
            pPage = pPage->GetChild(m_pIndex);

        m_aCurLeaf = pPage;
        m_nCurNode = NODE_NOTFOUND;
    }

    ONDXKey* pKey;
    if ( ( ( pKey = GetNextKey() ) == NULL ) || !pKey->getValue().isNull())
    {
        pKey = NULL;
        m_aCurLeaf = NULL;
    }
    return pKey ? pKey->GetRecord() : STRING_NOTFOUND;
}

//------------------------------------------------------------------
sal_uIntPtr OIndexIterator::GetNotNull(sal_Bool bFirst)
{
    ONDXKey* pKey;
    if (bFirst)
    {
        // go through all NULL values first
        for (sal_uIntPtr nRec = GetNull(bFirst);
             nRec != STRING_NOTFOUND;
             nRec = GetNull(sal_False))
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
    if (m_aCurLeaf.Is() && ((++m_nCurNode) >= m_aCurLeaf->Count()))
    {
        ONDXPage* pPage = m_aCurLeaf;
        // search next page
        while (pPage)
        {
            ONDXPage* pParentPage = pPage->GetParent();
            if (pParentPage)
            {
                sal_uInt16 nPos = pParentPage->Search(pPage);
                if (nPos != pParentPage->Count() - 1)
                {   // page found
                    pPage = (*pParentPage)[nPos+1].GetChild(m_pIndex,pParentPage);
                    break;
                }
            }
            pPage = pParentPage;
        }

        // now go on with leaf
        while (pPage && !pPage->IsLeaf())
            pPage = pPage->GetChild(m_pIndex);

        m_aCurLeaf = pPage;
        m_nCurNode = 0;
    }
    return m_aCurLeaf.Is() ? &(*m_aCurLeaf)[m_nCurNode].GetKey() : NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
