/*************************************************************************
 *
 *  $RCSfile: ScriptPanel.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-09-10 10:45:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.idesupport.ui;

import java.io.File;
import java.util.Vector;
import java.util.Enumeration;

import java.awt.BorderLayout;
import java.awt.event.FocusEvent;
import java.awt.event.FocusAdapter;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.JTable;
import javax.swing.DefaultCellEditor;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;
import javax.swing.table.AbstractTableModel;

import com.sun.star.script.framework.browse.ScriptEntry;

public class ScriptPanel extends JPanel {
    private ScriptTableModel model;
    private JTable table;

    public ScriptPanel(ScriptEntry[] scripts) {
        model = new ScriptTableModel(scripts);
        initUI();
    }

    public void reload(ScriptEntry[] entries) {
        model.removeAll();
        addScriptEntries(entries);
    }

    public void addScriptEntries(ScriptEntry[] entries) {
        for (int i = 0; i < entries.length; i++) {
            ScriptEntry entry;

            try {
                entry = (ScriptEntry) entries[i].clone();
            }
            catch (CloneNotSupportedException cnse) {
                entry = new ScriptEntry(entries[i].getLanguage(),
                                        entries[i].getLanguageName(),
                                        entries[i].getLogicalName(),
                                        entries[i].getLocation());
            }

            model.add(entry);
        }
    }

    public void removeSelectedRows() {
        int[] selections = table.getSelectedRows();

        for (int i = selections.length - 1; i >= 0; i--) {
            model.remove(selections[i]);
        }
    }

    public void removeAllRows() {
        model.removeAll();
    }

    public Enumeration getScriptEntries() {
        return model.getScriptEntries();
    }

    private void initUI() {
        table = new JTable(model);
        TableColumn column = table.getColumnModel().getColumn(1);
        column.setCellEditor(new DefaultCellEditor(new JTextField()));

        table.addFocusListener(new FocusAdapter() {
            public void focusLost(FocusEvent evt) {
                tableFocusLost(evt);
            }
        });

        JScrollPane pane = new JScrollPane(table);
        JLabel label = new JLabel("Scripts:");
        label.setLabelFor(pane);

        BorderLayout layout = new BorderLayout();
        setLayout(layout);
        layout.setVgap(5);
        add(label, BorderLayout.NORTH);
        add(pane, BorderLayout.CENTER);
    }

    private void tableFocusLost(FocusEvent evt) {
        TableCellEditor editor = table.getCellEditor();
        if (editor != null) {
            Object value = editor.getCellEditorValue();
            if (value != null)
                model.setValueAt(value,
                                 table.getEditingRow(), table.getEditingColumn());
        }
    }

    private class ScriptTableModel extends AbstractTableModel {
        final String[] columnNames = {"Exported Method",
                                      "Script Name"};

        private Vector scripts;
        private int nextRow;

        public ScriptTableModel(ScriptEntry[] entries) {
            scripts = new Vector(entries.length + 11);
            for (int i = 0; i < entries.length; i++) {
                scripts.addElement(entries[i]);
            }
            nextRow = entries.length;
        }

        public int getColumnCount() {
            return columnNames.length;
        }

        public int getRowCount() {
            return scripts.size();
        }

        public String getColumnName(int col) {
            return columnNames[col];
        }

        public void add(ScriptEntry entry) {
            scripts.addElement(entry);
            fireTableRowsInserted(nextRow, nextRow);
            nextRow++;
        }

        public void remove(int row) {
            scripts.removeElementAt(row);
            fireTableRowsDeleted(row, row);
            nextRow--;
        }

        public void removeAll() {
            scripts.removeAllElements();
            fireTableRowsDeleted(0, nextRow);
            nextRow = 0;
        }

        public Enumeration getScriptEntries() {
            return scripts.elements();
        }

        public Object getValueAt(int row, int col) {
            String result = "";
            ScriptEntry entry;

            entry = (ScriptEntry)scripts.elementAt(row);

            if (col == 0)
                result = entry.getLanguageName();
            else if (col == 1)
                result = entry.getLogicalName();

            return result;
        }

        public boolean isCellEditable(int row, int col) {
            if (col == 0)
                return false;
            else
                return true;
        }

        public void setValueAt(Object value, int row, int col) {
            ScriptEntry entry = (ScriptEntry)scripts.elementAt(row);
            entry.setLogicalName((String)value);
            fireTableCellUpdated(row, col);
        }
    }
}
