/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
