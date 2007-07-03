/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InstallationImminent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:56:29 $
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
