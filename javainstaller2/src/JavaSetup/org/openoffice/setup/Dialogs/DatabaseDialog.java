/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DatabaseDialog.java,v $
 * $Revision: 1.3 $
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

package org.openoffice.setup.Dialogs;

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.SetupFrame;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Insets;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import org.openoffice.setup.ResourceManager;

public class DatabaseDialog extends JDialog {

    public DatabaseDialog(SetupFrame setupFrame) {

        // super(setupFrame.getDialog());

        String text1 = ResourceManager.getString("String_Analyzing_Database1");
        String text2 = ResourceManager.getString("String_Analyzing_Database2");

        setTitle(text1);
        this.getContentPane().setLayout(new java.awt.BorderLayout());

        JPanel toppanel = new JPanel();
        toppanel.setLayout(new java.awt.BorderLayout());
        toppanel.setBorder(new EmptyBorder(new Insets(50, 50, 50, 50)));

        PanelLabel label1 = new PanelLabel(text2);
        toppanel.add(label1, BorderLayout.NORTH);

        Container contentPane = this.getContentPane();
        contentPane.add(toppanel);
        // this.getContentPane().add(toppanel, BorderLayout.CENTER);
    }

}
