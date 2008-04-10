/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InstallationImminent.java,v $
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

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.Point;
import javax.swing.JEditorPane;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JViewport;
import javax.swing.border.EmptyBorder;

public class InstallationImminent extends JPanel {

    private String infoText;
    private JEditorPane ProductInformation;
    private JScrollPane ProductPane;
    private PanelTitle titlebox;

    public InstallationImminent() {
        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titletext = ResourceManager.getString("String_InstallationImminent1");
        titlebox = new PanelTitle(titletext);
        add(titlebox, BorderLayout.NORTH);

        JPanel contentpanel = new JPanel();
        contentpanel.setLayout(new java.awt.BorderLayout());

        String text1 = ResourceManager.getString("String_InstallationImminent2");
        PanelLabel label1 = new PanelLabel(text1);

        ProductInformation = new JEditorPane("text/html", getInfoText());
        ProductInformation.setEditable(false);

        ProductPane = new JScrollPane(ProductInformation);
        ProductPane.setPreferredSize(new Dimension(250, 145));
        ProductPane.setBorder(new EmptyBorder(10, 0, 10, 0));

        contentpanel.add(label1, BorderLayout.NORTH);
        contentpanel.add(ProductPane, BorderLayout.CENTER);

        add(contentpanel, BorderLayout.CENTER);
    }

    public void setInfoText(String text) {
        infoText = text;
        updateInfoText();
    }

    public void setTitleText(String s) {
        titlebox.setTitle(s);
    }

    public String getInfoText() {
        return infoText;
    }

    public void updateInfoText() {
        ProductInformation.setText(infoText);
    }

    public void setTabOrder() {
        JScrollBar ScrollBar = ProductPane.getVerticalScrollBar();
        if ( ScrollBar.isShowing() ) {
            ProductInformation.setFocusable(true);
        } else {
            ProductInformation.setFocusable(false);
        }
    }

    public void setCaretPosition() {
        ProductInformation.setCaretPosition(0);
    }

}
