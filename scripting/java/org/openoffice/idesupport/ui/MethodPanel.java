/*************************************************************************
 *
 *  $RCSfile: MethodPanel.java,v $
 *
 *  $Revision: 1.8 $
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
import java.util.ArrayList;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JList;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.JLabel;
import java.awt.BorderLayout;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.MalformedURLException;

import com.sun.star.script.framework.browse.ScriptEntry;
import org.openoffice.idesupport.MethodFinder;
import org.openoffice.idesupport.ExtensionFinder;
import org.openoffice.idesupport.JavaFinder;

public class MethodPanel extends JPanel {

    private File basedir;
    private Vector classpath;
    private final static String FIRST_PARAM =
        "drafts.com.sun.star.script.framework.runtime.XScriptContext";

    // private JTable table;
    // private MethodTableModel model;
    private JList list;
    private ScriptEntry[] values;

    public MethodPanel(File basedir, Vector classpath, String language) {
        this.basedir = basedir;
        this.classpath = classpath;

        initValues(language);
        initUI();
    }

    public void reload(File basedir, Vector classpath, String language) {
        this.basedir = basedir;
        this.classpath = classpath;

        initValues(language);
        list.setListData(values);
    }

    public ScriptEntry[] getSelectedEntries() {
        Object[] selections = list.getSelectedValues();
        ScriptEntry[] entries = new ScriptEntry[selections.length];

        for (int i = 0; i < selections.length; i++) {
            entries[i] = (ScriptEntry)selections[i];
        }

        return entries;
    }

    private void initUI() {
        JLabel label = new JLabel("Available Methods:");
        // table = new JTable(model);
        list = new JList(values);

        JScrollPane pane = new JScrollPane(list);
        label.setLabelFor(pane);

        BorderLayout layout = new BorderLayout();
        setLayout(layout);
        layout.setVgap(5);

        add(label, BorderLayout.NORTH);
        add(pane, BorderLayout.CENTER);
    }

    private void initValues(String language) {
        MethodFinder finder;

        if (language == null)
            finder = JavaFinder.getInstance(classpath);
        else if (language.toLowerCase().equals("beanshell"))
            finder = new ExtensionFinder(language, new String[] {".bsh"});
        else
            finder = JavaFinder.getInstance(classpath);

        values = finder.findMethods(basedir);
    }

    /*
    private class MethodTableModel extends AbstractTableModel {
        final String[] columnNames = {"Method",
                                      "Language"};

        private Vector methods;
        private int nextRow;

        public MethodTableModel() {
            methods = new Vector(11);
        }

        public int getColumnCount() {
            return columnNames.length;
        }

        public int getRowCount() {
            return methods.size();
        }

        public String getColumnName(int col) {
            return columnNames[col];
        }

        public void add(ScriptEntry entry) {
            methods.addElement(entry);
            fireTableRowsInserted(nextRow, nextRow);
            nextRow++;
        }

        public void remove(int row) {
            methods.removeElementAt(row);
            fireTableRowsDeleted(row, row);
            nextRow--;
        }

        public void removeAll() {
            methods.removeAllElements();
            fireTableRowsDeleted(0, nextRow);
            nextRow = 0;
        }

        public Object getValueAt(int row) {
            return methods.elementAt(row);
        }

        public Object getValueAt(int row, int col) {
            String result = "";
            ScriptEntry entry;

            entry = (ScriptEntry)methods.elementAt(row);

            if (col == 0)
                result = entry.getLanguageName();
            else if (col == 1)
                result = entry.getLanguage();

            return result;
        }

        public boolean isCellEditable(int row, int col) {
            return false;
        }
    }
    */
}
