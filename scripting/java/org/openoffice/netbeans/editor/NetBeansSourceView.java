/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package org.openoffice.netbeans.editor;

import javax.swing.*;
import javax.swing.text.Document;
import javax.swing.event.DocumentListener;
import javax.swing.event.DocumentEvent;

import java.io.*;
import java.util.ResourceBundle;

import javax.swing.text.Caret;
import org.netbeans.editor.*;
import org.netbeans.editor.ext.*;

import com.sun.star.script.framework.provider.beanshell.ScriptSourceView;
import com.sun.star.script.framework.provider.beanshell.ScriptSourceModel;

public class NetBeansSourceView extends JPanel
    implements ScriptSourceView, DocumentListener {

    private ScriptSourceModel model;
    private JEditorPane pane;
    private boolean isModified = false;

    static {
        // Feed our kits with their default Settings
        Settings.addInitializer(
            new BaseSettingsInitializer(), Settings.CORE_LEVEL);
        Settings.addInitializer(
            new ExtSettingsInitializer(), Settings.CORE_LEVEL);
        Settings.reset();

        try {
            Class kitClass = Class.forName(
                NetBeansSourceView.class.getPackage().getName() + ".JavaKit");

            JEditorPane.registerEditorKitForContentType(
                "text/x-java", kitClass.getName(), kitClass.getClassLoader());
        }
        catch( ClassNotFoundException exc ) {
        }
    }

    private class MyLocalizer implements LocaleSupport.Localizer {
        private ResourceBundle bundle;

        public MyLocalizer( String bundleName ) {
            bundle = ResourceBundle.getBundle( bundleName );
        }

        // Localizer
        public String getString( String key ) {
            return bundle.getString( key );
        }
    }

    public NetBeansSourceView(ScriptSourceModel model) {
        this.model = model;

        LocaleSupport.addLocalizer(
            new MyLocalizer("org.netbeans.editor.Bundle"));

        pane = new JEditorPane("text/x-java", "");
        pane.setText(model.getText());

        JScrollPane spane = new JScrollPane();
        spane.setViewportView(pane);
        setLayout(new java.awt.GridLayout(1, 1));
        add(spane);

        pane.getDocument().addDocumentListener(this);
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("No file specified");
            System.exit(-1);
        }

        File f = new File(args[0]);

        if (!f.exists() || !f.isFile()) {
            System.err.println("Invalid file");
            System.exit(-1);
        }

        java.net.URL url = null;
        try {
            url = f.toURL();
        }
        catch (java.net.MalformedURLException mue) {
            System.err.println("Invalid file");
            System.exit(-1);
        }

        NetBeansSourceView view =
            new NetBeansSourceView(new ScriptSourceModel(url));

        JFrame frame = new JFrame();
        frame.getContentPane().add(view);
        frame.setSize(640, 480);
        frame.show();
    }

    // Code grabbed from NetBeans editor module
    public void scrollToLine(int line)
    {
        BaseDocument doc = Utilities.getDocument(pane);

        int pos = -1;
        if (doc != null) {
            // Obtain the offset where to jump
            pos = Utilities.getRowStartFromLineOffset(doc, line);
        }

        if (pos != -1) {
            Caret caret = pane.getCaret();
            if (caret instanceof BaseCaret) { // support extended scroll mode
                BaseCaret bCaret = (BaseCaret)caret;
                bCaret.setDot(pos, bCaret, EditorUI.SCROLL_FIND);
            }
            else {
                caret.setDot(pos);
            }
        }
    }

    public void clear() {
        pane.setText("");
    }

    public void update() {
        /* Remove ourselves as a DocumentListener while loading the source
           so we don't get a storm of DocumentEvents during loading */
        pane.getDocument().removeDocumentListener(this);

        if (isModified == false)
        {
            pane.setText(model.getText());
        }

        // scroll to current position of the model
        try {
            scrollToLine(model.getCurrentPosition());
        }
        catch (Exception e) {
            // couldn't scroll to line, do nothing
        }

        // Add back the listener
        pane.getDocument().addDocumentListener(this);
    }

    public boolean isModified() {
        return isModified;
    }

    public void setModified(boolean value) {
        isModified = value;
    }

    public String getText() {
        return pane.getText();
    }

    /* Implementation of DocumentListener interface */
    public void insertUpdate(DocumentEvent e) {
        doChanged(e);
    }

    public void removeUpdate(DocumentEvent e) {
        doChanged(e);
    }

    public void changedUpdate(DocumentEvent e) {
        doChanged(e);
    }

    public void doChanged(DocumentEvent e) {
        isModified = true;
    }

}
