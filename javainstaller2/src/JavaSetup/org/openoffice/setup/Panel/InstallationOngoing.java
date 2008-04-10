/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InstallationOngoing.java,v $
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
import org.openoffice.setup.SetupActionListener;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Insets;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.border.EmptyBorder;

public class InstallationOngoing extends JPanel {

    private PanelLabel currentProgress;
    private JProgressBar progressBar;
    private JButton mStopButton;
    private String mTitle = "";
    private PanelTitle mTitlebox;

    public InstallationOngoing() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        // String titleText = ResourceManager.getString("String_InstallationOngoing1");
        // PanelTitle titlebox = new PanelTitle(titleText);
        // PanelTitle titlebox = new PanelTitle(mTitle);
        mTitlebox = new PanelTitle(mTitle);
        mTitlebox.addVerticalStrut(20);
        add(mTitlebox, BorderLayout.NORTH);

        Container contentbox = Box.createVerticalBox();

        // String progressText = ResourceManager.getString("String_InstallationOngoing2");
        String progressText = "";
        currentProgress = new PanelLabel(progressText);

        Container innerbox = Box.createHorizontalBox();

        progressBar = new JProgressBar(0, 100);
        mStopButton = new JButton();
        String progressButtonText = ResourceManager.getString("String_InstallationOngoing3");
        mStopButton.setText(progressButtonText);
        mStopButton.setEnabled(true);

        innerbox.add(progressBar);
        innerbox.add(Box.createHorizontalStrut(10));
        innerbox.add(mStopButton);

        contentbox.add(currentProgress);
        contentbox.add(Box.createVerticalStrut(10));
        contentbox.add(innerbox);
        contentbox.add(Box.createVerticalStrut(20));

        add(contentbox, BorderLayout.SOUTH);
    }

    public void setProgressText(String s) {
        currentProgress.setText(s);
    }

    public void setProgressValue(int i) {
        progressBar.setValue(i);
    }

    public void setTitle(String title) {
        mTitlebox.setTitle(title);
        mTitle = title;
    }

    public void setStopButtonActionCommand(String actionCommand) {
        mStopButton.setActionCommand(actionCommand);
    }

    public void addStopButtonActionListener(SetupActionListener actionListener) {
        mStopButton.addActionListener(actionListener);
    }

    public void setStopButtonEnabled(boolean enabled) {
        mStopButton.setEnabled(enabled);
    }

}
