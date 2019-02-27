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

#include "hsqlbinarynode.hxx"
#include "rowinputbinary.hxx"

#include <cppuhelper/implbase.hxx>
#include <vector>

namespace dbahsql
{
using ColumnTypeVector = std::vector<ColumnDefinition>;

HsqlBinaryNode::HsqlBinaryNode(sal_Int32 nPos)
    : m_nPos(nPos)
{
}

void HsqlBinaryNode::readChildren(HsqlRowInputStream const& input)
{
    SvStream* pStream = input.getInputStream();
    if (!pStream)
        return;

    pStream->Seek(m_nPos + 8); // skip size and balance
    pStream->ReadInt32(m_nLeft);
    if (m_nLeft <= 0)
        m_nLeft = -1;
    pStream->ReadInt32(m_nRight);
    if (m_nRight <= 0)
        m_nRight = -1;
}

std::vector<css::uno::Any> HsqlBinaryNode::readRow(HsqlRowInputStream& input,
                                                   const std::vector<ColumnDefinition>& aColTypes,
                                                   sal_Int32 nIndexCount)
{
    // skip first 4 bytes (size), and index nodes, 16 bytes each
    input.seek(m_nPos + 4 + nIndexCount * 16);
    return input.readOneRow(aColTypes);
}

sal_Int32 HsqlBinaryNode::getLeft() const { return m_nLeft; }
sal_Int32 HsqlBinaryNode::getRight() const { return m_nRight; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
