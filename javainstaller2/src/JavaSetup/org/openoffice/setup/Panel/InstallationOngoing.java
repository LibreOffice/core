/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InstallationOngoing.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:56:42 $
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
