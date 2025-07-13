/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <swtable.hxx>
#include <itabenum.hxx>
#include <ndtxt.hxx>

#include "swmd.hxx"

class MDTable
{
    const SwTable* m_pTable;
    SwMarkdownParser* m_pParser;
    sal_Int32 m_nRow;
    sal_Int32 m_nCol;
    sal_Int32 m_nCurRow;
    sal_Int32 m_nCurCol;

public:
    MDTable(SwMarkdownParser* pParser)
        : m_pTable(nullptr)
        , m_pParser(pParser)
        , m_nRow(-1)
        , m_nCol(-1)
        , m_nCurRow(-1)
        , m_nCurCol(-1)
    {
        m_pParser->RegisterTable(this);
    }

    ~MDTable() { m_pParser->DeRegisterTable(this); }

    const SwTable* GetTable() { return m_pTable; }
    sal_Int32 GetCurRow() { return m_nCurRow; }
    sal_Int32 GetCurCol() { return m_nCurCol; }

    void SetTable(const SwTable* pTable, sal_Int32 nRow, sal_Int32 nCol);
    inline void IncCurRow();
    inline void IncCurCol();
};

void MDTable::SetTable(const SwTable* pTable, sal_Int32 nRow, sal_Int32 nCol)
{
    m_pTable = pTable;
    m_nRow = nRow;
    m_nCol = nCol;
}

void SwMarkdownParser::StartTable(sal_Int32 nRow, sal_Int32 nCol)
{
    if (m_pPam->GetPoint()->GetContentIndex())
        AppendTextNode(AM_SPACE);
    else
        AddParSpace();

    std::shared_ptr<MDTable> xTable = std::make_shared<MDTable>(this);
    m_xTable = xTable;

    const SwTable* pTable
        = m_xDoc->InsertTable(SwInsertTableOptions(SwInsertTableFlags::All, 1), *m_pPam->GetPoint(),
                              nRow, nCol, text::HoriOrientation::FULL);
    m_xTable->SetTable(pTable, nRow, nCol);
}

void SwMarkdownParser::EndTable()
{
    m_pPam->Move(fnMoveForward);
    AppendTextNode(AM_SPACE);
    m_xTable.reset();
}

void SwMarkdownParser::StartRow() { m_xTable->IncCurRow(); }

void SwMarkdownParser::StartCell(MD_ALIGN eAdjust)
{
    m_xTable->IncCurCol();
    SwTableBox* pBox = m_xTable->GetTable()
                           ->GetTabLines()[m_xTable->GetCurRow()]
                           ->GetTabBoxes()[m_xTable->GetCurCol()];
    const SwStartNode* pSttNd = pBox->GetSttNd();
    SwTextNode* pTextNode
        = pSttNd->GetNodes()[pSttNd->GetIndex() + 1]->GetContentNode()->GetTextNode();

    if (!pTextNode)
    {
        return;
    }

    SvxAdjustItem aAdjustItem(adjustMap.at(eAdjust), RES_PARATR_ADJUST);
    pTextNode->SetAttr(aAdjustItem);

    m_pPam->GetPoint()->Assign(*pTextNode, 0);
}

inline void MDTable::IncCurRow()
{
    m_nCurRow++;
    m_nCurCol = -1;
}

inline void MDTable::IncCurCol() { m_nCurCol++; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
