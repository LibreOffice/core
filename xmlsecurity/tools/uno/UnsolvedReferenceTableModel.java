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

package com.sun.star.xml.security.uno;

import javax.swing.table.AbstractTableModel;

/*
 * this class is used to maintain the unsolved reference
 * table.
 */
class UnsolvedReferenceTableModel extends AbstractTableModel
{
    private String[] m_columnNames = {"id",
                           "refNum",
                           "EC's id"};

    private TestTool m_testTool;

    UnsolvedReferenceTableModel(TestTool testTool)
    {
        m_testTool = testTool;
    }

    public String getColumnName(int col)
    {
        return m_columnNames[col].toString();
    }

    public int getRowCount()
    {
        return m_testTool.getUnsolvedReferenceIds().size();
    }

    public int getColumnCount()
    {
        return m_columnNames.length;
    }

    public Object getValueAt(int row, int col)
    {
            if (col == 0)
            {
                return (String)m_testTool.getUnsolvedReferenceIds().elementAt(row);
        }
        else if (col == 1)
        {
            return ((Integer)m_testTool.getUnsolvedReferenceRefNum().elementAt(row)).toString();
        }
        else if (col == 2)
        {
            return ((Integer)m_testTool.getUnsolvedReferenceKeeperIds().elementAt(row)).toString();
        }
        else
        {
            return null;
        }
    }

    public boolean isCellEditable(int row, int col)
    {
        return false;
    }
}

