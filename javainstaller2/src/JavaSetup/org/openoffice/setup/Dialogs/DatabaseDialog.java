/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DatabaseDialog.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:53:07 $
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
