/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnsolvedReferenceTableModel.java,v $
 * $Revision: 1.3 $
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

