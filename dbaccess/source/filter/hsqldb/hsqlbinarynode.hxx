/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include "rowinputbinary.hxx"
#include "columndef.hxx"

namespace dbahsql
{
class HsqlBinaryNode
{
private:
    sal_Int32 m_nLeft = -1;
    sal_Int32 m_nRight = -1;
    sal_Int32 m_nPos = -1;

public:
    /**
     * Represents one element of an AVL tree in the binary file which contains
     * the data.
     */
    HsqlBinaryNode(sal_Int32 nPos);

    /**
     * Read position of children from data file.
     *
     * @param rInput input stream where the positions should be read from.
     */
    void readChildren(HsqlRowInputStream const& rInput);

    /**
     * Get Position of left children. It should be called only after position of
     * children is read.
     */
    sal_Int32 getLeft() const;

    /**
     * Get Position of right children. It should be called only after position of
     * children is read.
     */
    sal_Int32 getRight() const;

    /**
     * Read the row represented by this node.
     *
     * @param rInput input stream where the row should be read from.
     */
    std::vector<css::uno::Any> readRow(HsqlRowInputStream& rInput,
                                       const std::vector<ColumnDefinition>& aColTypes,
                                       sal_Int32 nIndexCount);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
