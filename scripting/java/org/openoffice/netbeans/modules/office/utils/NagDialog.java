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
