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

#pragma once

#include <file/fcode.hxx>
#include <dbase/DIndex.hxx>
#include <dbase/dindexnode.hxx>

namespace connectivity::dbase
    {

        // IndexIterator

        class OIndexIterator final
        {
            file::OBoolOperator*    m_pOperator;
            const file::OOperand*   m_pOperand;
            rtl::Reference<ODbaseIndex> m_xIndex;
            ONDXPagePtr             m_aRoot,
                                    m_aCurLeaf;
            sal_uInt16              m_nCurNode;

            sal_uInt32 Find(bool bFirst);
            sal_uInt32 GetCompare(bool bFirst);
            sal_uInt32 GetLike(bool bFirst);
            sal_uInt32 GetNull(bool bFirst);
            sal_uInt32 GetNotNull(bool bFirst);

            ONDXKey* GetFirstKey(ONDXPage* pPage,
                                const file::OOperand& rKey);
            ONDXKey* GetNextKey();

        public:
            OIndexIterator(ODbaseIndex* pInd)
                :m_pOperator(nullptr)
                ,m_pOperand(nullptr)
                ,m_xIndex(pInd)
                ,m_nCurNode(NODE_NOTFOUND)
            {
            }

            ~OIndexIterator();
            sal_uInt32 First();
            sal_uInt32 Next();

        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
