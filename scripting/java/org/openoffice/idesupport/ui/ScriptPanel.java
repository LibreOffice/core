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

package org.openoffice.idesupport.ui;

import java.awt.BorderLayout;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.util.ArrayList;
import java.util.Iterator;

import javax.swing.DefaultCellEditor;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;

import com.sun.star.script.framework.container.ScriptEntry;

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
            } catch (CloneNotSupportedException cnse) {
                entry = new ScriptEntry(entries[i].getLanguage(),
                                        entries[i].getLanguageName(),
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

    public Iterator<ScriptEntry> getScriptEntries() {
        return model.getScriptEntries();
    }

    private void initUI() {
        table = new JTable(model);
        TableColumn column = table.getColumnModel().getColumn(1);
        column.setCellEditor(new DefaultCellEditor(new JTextField()));

        table.addFocusListener(new FocusAdapter() {
            @Override
            public void focusLost(FocusEvent evt) {
                tableFocusLost();
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

    private void tableFocusLost() {
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
                                      "Script Name"
                                     };

        private ArrayList<ScriptEntry> scripts;
        private int nextRow;

        public ScriptTableModel(ScriptEntry[] entries) {
            scripts = new ArrayList<ScriptEntry>(entries.length + 11);

            for (int i = 0; i < entries.length; i++) {
                scripts.add(entries[i]);
            }

            nextRow = entries.length;
        }

        public int getColumnCount() {
            return columnNames.length;
        }

        public int getRowCount() {
            return scripts.size();
        }

        @Override
        public String getColumnName(int col) {
            return columnNames[col];
        }

        public void add(ScriptEntry entry) {
            scripts.add(entry);
            fireTableRowsInserted(nextRow, nextRow);
            nextRow++;
        }

        public void remove(int row) {
            scripts.remove(row);
            fireTableRowsDeleted(row, row);
            nextRow--;
        }

        public void removeAll() {
            scripts.clear();
            fireTableRowsDeleted(0, nextRow);
            nextRow = 0;
        }

        public Iterator<ScriptEntry> getScriptEntries() {
            return scripts.iterator();
        }

        public Object getValueAt(int row, int col) {
            String result = "";
            ScriptEntry entry;

            entry = scripts.get(row);

            if (col == 0)
                result = entry.getLanguageName();
            else if (col == 1)
                result = entry.getLogicalName();

            return result;
        }

        @Override
        public boolean isCellEditable(int row, int col) {
            return (col != 0);
        }

        @Override
        public void setValueAt(Object value, int row, int col) {
            ScriptEntry entry = scripts.get(row);
            entry.setLogicalName((String)value);
            fireTableCellUpdated(row, col);
        }
    }
}
