/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

