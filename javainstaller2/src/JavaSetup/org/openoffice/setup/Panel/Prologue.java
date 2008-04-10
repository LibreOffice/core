/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Prologue.java,v $
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
import java.awt.Insets;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

public class Prologue extends JPanel {

    public Prologue() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText    = ResourceManager.getString("String_Prologue1");
        PanelTitle titleBox = new PanelTitle(titleText);
        add(titleBox, BorderLayout.NORTH);

        JPanel contentPanel = new JPanel();
        contentPanel.setLayout(new java.awt.BorderLayout());

        String text1 = ResourceManager.getString("String_Prologue2");
        PanelLabel label1 = new PanelLabel(text1, true);
        String text2 = ResourceManager.getString("String_Prologue3");
        PanelLabel label2 = new PanelLabel(text2);

        contentPanel.add(label1, BorderLayout.NORTH);
        contentPanel.add(label2, BorderLayout.CENTER);

        InstallData data = InstallData.getInstance();

        if ( data.isUserInstallation() ) {
            String text3 = ResourceManager.getString("String_Prologue4");
            PanelLabel label3 = new PanelLabel(text3, true);
            contentPanel.add(label3, BorderLayout.SOUTH);
        }

        add(contentPanel, BorderLayout.CENTER);
    }
}
