/*************************************************************************
*
*  $RCSfile: ScriptEditorForBeanShell.java,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: rt $ $Date: 2004-01-05 13:14:15 $
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
package com.sun.star.script.framework.provider.beanshell;

import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;

import java.awt.FlowLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.net.URL;

import bsh.Interpreter;

import drafts.com.sun.star.script.provider.XScriptContext;
import com.sun.star.script.framework.provider.PathUtils;
import com.sun.star.script.framework.browse.ScriptMetaData;


public class ScriptEditorForBeanShell implements ActionListener {

    private JFrame frame;
    private String filename;

    private ScriptSourceModel model;
    private ScriptSourceView view;

    private XScriptContext context;
    private URL scriptURL = null;
    public void edit(XScriptContext context, ScriptMetaData entry) {
        this.context = context;
        if (entry != null ) {
            try {
                String sUrl = entry.getParcelLocation();
                if ( !sUrl.endsWith( "/" ) )
                {
                    sUrl += "/";
                }
                sUrl +=  entry.getLanguageName();
                scriptURL = PathUtils.createScriptURL( sUrl );
                this.filename = scriptURL.getFile();
            }
            catch (IOException ioe) {

                showErrorMessage( "Error loading file: " + ioe.getMessage() );
            }
        }
        this.model = new ScriptSourceModel(scriptURL);

        try {
            Class c = Class.forName(
                "org.openoffice.netbeans.editor.NetBeansSourceView");

            Class[] types = new Class[] { ScriptSourceModel.class };

            java.lang.reflect.Constructor ctor = c.getConstructor(types);

            if (ctor != null) {
                Object[] args = new Object[] { this.model };
                this.view = (ScriptSourceView) ctor.newInstance(args);
            }
            else {
                this.view = new PlainSourceView(model);
            }
        }
        catch (java.lang.Error err) {
            this.view = new PlainSourceView(model);
        }
        catch (Exception e) {
            this.view = new PlainSourceView(model);
        }

        this.model.setView(this.view);
        initUI();
        frame.show();
    }

    public ScriptEditorForBeanShell() {
    }

    public void showErrorMessage(String message) {
        JOptionPane.showMessageDialog(frame, message,
            "Error", JOptionPane.ERROR_MESSAGE);
    }

    private void initUI() {
        this.frame = new JFrame("BeanShell Debug Window");

        String[] labels = {"Run", "Clear", "Save", "Close"};
        JPanel p = new JPanel();
        p.setLayout(new FlowLayout());

        for (int i = 0; i < labels.length; i++) {
            JButton b = new JButton(labels[i]);
            b.addActionListener(this);
            p.add(b);

            if (labels[i].equals("Save") && filename == null) {
                b.setEnabled(false);
            }
        }

        frame.getContentPane().add((JComponent)view, "Center");
        frame.getContentPane().add(p, "South");
        frame.pack();
        frame.setSize(590, 480);
        frame.setLocation(300, 200);
    }

    private void promptForSaveName() {
        JFileChooser chooser = new JFileChooser();
        chooser.setFileFilter(new javax.swing.filechooser.FileFilter() {
            public boolean accept(File f) {
                if (f.isDirectory() || f.getName().endsWith(".bsh")) {
                    return true;
                }
                return false;
            }

            public String getDescription() {
                return ("BeanShell files: *.bsh");
            }
        });

        int ret = chooser.showSaveDialog(frame);

        if (ret == JFileChooser.APPROVE_OPTION) {
            filename = chooser.getSelectedFile().getAbsolutePath();
            if (!filename.endsWith(".bsh")) {
                filename += ".bsh";
            }
        }

    }

    private void saveTextArea() {
        if (!view.isModified()) {
            return;
        }

        /*if (filename == null) {
            promptForSaveName();
        }*/
        OutputStream fos = null;
        try {
            String s = view.getText();
            if ( scriptURL.getProtocol().equals("file") )
            {
                fos = new FileOutputStream( filename );
            }
            else
            {
                fos = scriptURL.openConnection().getOutputStream();
            }
            if ( fos  != null) {
                fos.write(s.getBytes());
            }
            else
            {
                showErrorMessage( "Error saving file: couldn't open stream for file" );
            }

       }
       catch (IOException ioe) {
           showErrorMessage( "Error saving file: " + ioe.getMessage() );
        }
       catch (Exception e) {
           showErrorMessage( "Error saving file: " + e.getMessage() );
       }
        finally {
            if (fos != null) {
                try {
                    fos.flush();
                    if ( fos != null )
                    {
                        fos.close();
                    }
                }
                catch (IOException ignore) {
                }
            }
        }

    }

    public void actionPerformed(ActionEvent e) {
        if (e.getActionCommand().equals("Run")) {
            try {
                model.execute(context);
            }
            catch (java.lang.reflect.InvocationTargetException ite) {
                showErrorMessage(ite.getMessage());
            }
        }
        else if (e.getActionCommand().equals("Close")) {
            frame.dispose();
        }
        else if (e.getActionCommand().equals("Save")) {
            saveTextArea();
        }
        else if (e.getActionCommand().equals("Clear")) {
            view.clear();
        }
    }
}
