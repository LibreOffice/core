/*************************************************************************
 *
 *  $RCSfile: MethodPanel.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-01-28 20:52:31 $
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

import org.openoffice.idesupport.ScriptEntry;
import org.openoffice.idesupport.SVersionRCFile;
import org.openoffice.idesupport.zip.ParcelZipper;

public class MethodPanel extends JPanel {

    private File basedir;
    private Vector classpath;
    private final static String FIRST_PARAM =
        "drafts.com.sun.star.script.framework.XScriptContext";

    // private JTable table;
    // private MethodTableModel model;
    private JList list;
    private Vector values = new Vector(11);

    public MethodPanel(File basedir, Vector classpath, String language) {
        this.basedir = basedir;
        this.classpath = classpath;

        initValues(language);
        initUI();
    }

    public void reload(File basedir, Vector classpath, String language) {
        this.basedir = basedir;
        this.classpath = classpath;

    values.removeAllElements();
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
        String parcelName;

        if (basedir == null || basedir.exists() == false ||
            basedir.isDirectory() == false)
            return;

        parcelName = basedir.getName();
        if (parcelName.equals(ParcelZipper.CONTENTS_DIRNAME))
            parcelName = basedir.getParentFile().getName();

        if (language == null)
            initJavaValues(parcelName);
        else if (language.toLowerCase().equals("beanshell"))
            initBeanShellValues(parcelName);
        else
            initJavaValues(parcelName);
    }

    private void initJavaValues(String parcelName) {
        String[] classNames;

        classNames = findClassNames();
        if (classNames != null && classNames.length != 0) {

            ClassLoader classloader = getClassLoader();

            for (int i = 0; i < classNames.length; i++)
            {
                try
                {
                    Class clazz = classloader.loadClass(classNames[i]);
                    Method[] methods = clazz.getDeclaredMethods();
                    for (int k = 0; k < methods.length; k++)
                    {
                        if (Modifier.isPublic(methods[k].getModifiers()))
                        {
                            Class[] params = methods[k].getParameterTypes();
                            if(params.length > 0)
                            {
                                if(params[0].getName().equals(FIRST_PARAM))
                                {
                                    ScriptEntry entry =
                                        new ScriptEntry(classNames[i] + "." +
                                            methods[k].getName(), parcelName);
                                    values.addElement(entry);
                                }
                            }
                        }
                    }
                }
                catch (ClassNotFoundException e)
                {
                    System.err.println("Class Not Found Exception...");
                    continue;
                }
                catch (NoClassDefFoundError nc)
                {
                    System.err.println("No Class Definition Found...");
                    continue;
                }
            }
        }
    }

    private ClassLoader getClassLoader() {

        int len = classpath.size();
        ArrayList urls = new ArrayList(len);

        for (int i = 0; i < len; i++) {
            try {
                String s = (String)classpath.elementAt(i);
                s = SVersionRCFile.toFileURL(s);

                if (s != null)
                    urls.add(new URL(s));
            }
            catch (MalformedURLException mue) {
            }
        }

        return new URLClassLoader((URL[])urls.toArray(new URL[0]));
    }

    private void initBeanShellValues(String parcelName) {

        ArrayList bshFiles = findFiles(basedir, ".bsh");

        if (bshFiles != null) {
            for (int i = 0; i < bshFiles.size(); i++) {
                File f = (File)bshFiles.get(i);
                values.addElement(new ScriptEntry("BeanShell", f.getName(),
                    f.getName(), parcelName));
            }
        }
    }

    private ArrayList findFiles(File basedir, String suffix) {
        ArrayList result = new ArrayList();
        File[] children = basedir.listFiles();

        for (int i = 0; i < children.length; i++) {
            if (children[i].isDirectory())
                result.addAll(findFiles(children[i], suffix));
            else if (children[i].getName().endsWith(suffix))
                    result.add(children[i]);
        }
        return result;
    }

    private String[] findClassNames()
    {
        ArrayList classFiles = findFiles(basedir, ".class");
        if(classFiles == null || classFiles.size() == 0)
            return null;

        ArrayList javaFiles = findFiles(basedir, ".java");
        if(javaFiles == null || javaFiles.size() == 0)
            return null;

        ArrayList result = new ArrayList();
        for (int i = 0; i < classFiles.size(); i++)
        {
            File classFile = (File)classFiles.get(i);
            String className = classFile.getName();
            className = className.substring(0, className.lastIndexOf(".class"));
            boolean finished = false;


            for (int j = 0; j < javaFiles.size() && finished == false; j++)
            {
                File javaFile = (File)javaFiles.get(j);
                String javaName = javaFile.getName();
                javaName = javaName.substring(0, javaName.lastIndexOf(".java"));


                if (javaName.equals(className))
                {
                    String path = classFile.getAbsolutePath();
                    path = path.substring(basedir.getAbsolutePath().length() + 1);
                    path = path.replace(File.separatorChar, '.');
                    path = path.substring(0, path.lastIndexOf(".class"));

                    result.add(path);
                    javaFiles.remove(j);
                    finished = true;
                }
            }
        }
        return (String[])result.toArray(new String[0]);
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
