/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnsolvedReferenceTableModel.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:45:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

