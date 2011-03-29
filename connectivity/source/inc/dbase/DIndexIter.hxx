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

#ifndef _CONNECTIVITY_DBASE_INDEXITER_HXX_
#define _CONNECTIVITY_DBASE_INDEXITER_HXX_

#include "file/fcode.hxx"
#include "dbase/DIndex.hxx"
#include "dbase/dindexnode.hxx"

namespace connectivity
{
    namespace dbase
    {
        //==================================================================
        // IndexIterator
        //==================================================================
        class OIndexIterator
        {
        protected:
            file::OBoolOperator*    m_pOperator;
            const file::OOperand*   m_pOperand;
            ODbaseIndex*            m_pIndex;
            ONDXPagePtr             m_aRoot,
                                    m_aCurLeaf;
            sal_uInt16                  m_nCurNode;

        protected:
            sal_uIntPtr Find(sal_Bool bFirst);
            sal_uIntPtr GetCompare(sal_Bool bFirst);
            sal_uIntPtr GetLike(sal_Bool bFirst);
            sal_uIntPtr GetNull(sal_Bool bFirst);
            sal_uIntPtr GetNotNull(sal_Bool bFirst);

            ONDXKey* GetFirstKey(ONDXPage* pPage,
                                const file::OOperand& rKey);
            ONDXKey* GetNextKey();
            ODbaseIndex* GetIndex() const {return m_pIndex;}


        public:
            OIndexIterator(ODbaseIndex* pInd,
                            file::OBoolOperator* pOp,
                            const file::OOperand* pOper)
                :m_pOperator(pOp)
                ,m_pOperand(pOper)
                ,m_pIndex(pInd)
                ,m_nCurNode(NODE_NOTFOUND)
            {
                pInd->acquire();
            }

            virtual ~OIndexIterator();
            sal_uIntPtr First();
            sal_uIntPtr Next();

        };
    }
}
#endif // _CONNECTIVITY_DBASE_INDEXITER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
