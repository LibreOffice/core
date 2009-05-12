/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AcceptLicense.java,v $
 * $Revision: 1.4 $
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

package org.openoffice.setup.Panel;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Insets;
import java.io.File;
import javax.swing.JPanel;
import javax.swing.JEditorPane;
import javax.swing.JScrollPane;
import javax.swing.border.EmptyBorder;
public class AcceptLicense extends JPanel {

    public AcceptLicense() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titletext = ResourceManager.getString("String_AcceptLicense1");
        PanelTitle titlebox = new PanelTitle(titletext);
        add(titlebox, BorderLayout.NORTH);

        JPanel contentpanel = new JPanel();
        contentpanel.setLayout(new java.awt.BorderLayout());

        String text1 = ResourceManager.getString("String_AcceptLicense2");
        PanelLabel label1 = new PanelLabel(text1);

        String text2 = ResourceManager.getString("String_AcceptLicense3");
        PanelLabel label2 = new PanelLabel(text2, true);

        JEditorPane editorPane = createEditorPane();
        JScrollPane editorScrollPane = new JScrollPane(editorPane);

        editorScrollPane.setPreferredSize(new Dimension(250, 145));
        editorScrollPane.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));

        contentpanel.add(label1, BorderLayout.NORTH);
        contentpanel.add(editorScrollPane, BorderLayout.CENTER);
        contentpanel.add(label2, BorderLayout.SOUTH);

        add(contentpanel, BorderLayout.CENTER);
    }

    private JEditorPane createEditorPane() {
        JEditorPane editorPane = new JEditorPane();
        editorPane.setEditable(false);

        InstallData data = InstallData.getInstance();
        File htmlDirectory = data.getInfoRoot("html");
        String licenseFile = ResourceManager.getFileName("String_License_Filename");

        if ( htmlDirectory != null) {
            File htmlFile = new File(htmlDirectory, licenseFile);
            if (! htmlFile.exists()) {
                System.err.println("Couldn't find file: " + htmlFile.toString());
            }

            try {
                // System.err.println("URLPath: " + htmlFile.toURL());
                editorPane.setContentType("text/html;charset=utf-8");
                editorPane.setPage(htmlFile.toURL());
            } catch (Exception e) {
                e.printStackTrace();
                System.err.println("Attempted to read a bad URL");
            }
        } else {
            System.err.println("Did not find html directory");
        }

        return editorPane;
    }
}
