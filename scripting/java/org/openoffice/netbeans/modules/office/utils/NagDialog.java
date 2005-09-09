/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NagDialog.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:20:03 $
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

package org.openoffice.netbeans.modules.office.utils;

import java.awt.BorderLayout;
import java.awt.Dimension;

import javax.swing.JPanel;
import javax.swing.JOptionPane;
import javax.swing.JCheckBox;
import javax.swing.border.EmptyBorder;

import org.openide.TopManager;
import org.openide.NotifyDescriptor;

public class NagDialog {

    private NotifyDescriptor descriptor;
    private JPanel panel;
    private JCheckBox checkbox;

    private NagDialog(String message, String prompt, boolean initialState,
        int type) {
        initUI(message, prompt, initialState, type);
    }

    public static NagDialog createInformationDialog(
        String message, String prompt, boolean initialState) {
        NagDialog result = new NagDialog(
            message, prompt, initialState, JOptionPane.INFORMATION_MESSAGE);

        result.setDescriptor(new NotifyDescriptor.Message(result.getPanel(),
            NotifyDescriptor.PLAIN_MESSAGE));

        return result;
    }

    public static NagDialog createConfirmationDialog(
        String message, String prompt, boolean initialState) {
        NagDialog result = new NagDialog(
            message, prompt, initialState, JOptionPane.QUESTION_MESSAGE);

        result.setDescriptor(new NotifyDescriptor.Confirmation(
            result.getPanel(), NotifyDescriptor.OK_CANCEL_OPTION,
            NotifyDescriptor.PLAIN_MESSAGE));

        return result;
    }

    public boolean show() {
        TopManager.getDefault().notify(descriptor);

        if (descriptor.getValue() == NotifyDescriptor.OK_OPTION)
            return true;
        else
            return false;
    }

    public boolean getState() {
        return checkbox.isSelected();
    }

    private JPanel getPanel() {
        return this.panel;
    }

    private void setDescriptor(NotifyDescriptor descriptor) {
        this.descriptor = descriptor;
    }

    private void initUI(String message, String prompt, boolean initialState,
        int type) {

        this.panel = new JPanel();
        JOptionPane optionPane = new JOptionPane(message, type, 0, null,
            new Object[0], null)
            {
                public int getMaxCharactersPerLineCount() {
                    return 100;
                }
            };
        optionPane.setUI(new javax.swing.plaf.basic.BasicOptionPaneUI() {
            public Dimension getMinimumOptionPaneSize() {
                if (minimumSize == null) {
                    return new Dimension(MinimumWidth, 50);
                }
                return new Dimension(minimumSize.width, 50);
            }
        });
        optionPane.setWantsInput(false);

        JPanel checkPanel = new JPanel();
        checkbox = new JCheckBox(prompt, initialState);
        checkPanel.setLayout(new BorderLayout());
        checkPanel.setBorder(new EmptyBorder(0, 20, 0, 0));
        checkPanel.add(checkbox, BorderLayout.WEST);

        this.panel.setLayout(new BorderLayout());
        this.panel.add(optionPane, BorderLayout.CENTER);
        this.panel.add(checkPanel, BorderLayout.SOUTH);
    }
}
