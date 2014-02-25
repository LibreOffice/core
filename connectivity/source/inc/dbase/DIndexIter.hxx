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

#ifndef _CONNECTIVITY_DBASE_INDEXITER_HXX_
#define _CONNECTIVITY_DBASE_INDEXITER_HXX_

#include "file/fcode.hxx"
#include "dbase/DIndex.hxx"
#include "dbase/dindexnode.hxx"

namespace connectivity
{
    namespace dbase
    {

        // IndexIterator

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
