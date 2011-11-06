/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.setup;

import org.openoffice.setup.Dialogs.DetailsDialog;
import org.openoffice.setup.Dialogs.HelpDialog;
import org.openoffice.setup.Util.AbortInstaller;
import java.awt.Dimension;
import java.awt.event.ActionListener;
import javax.swing.JDialog;
import javax.swing.JOptionPane;

public class SetupActionListener implements ActionListener {

    private SetupFrame setupFrame;

    public SetupActionListener(SetupFrame setup) {
        setupFrame = setup;
    }

    public void actionPerformed (java.awt.event.ActionEvent evt) {
        if (evt.getActionCommand().equals(SetupFrame.ACTION_CANCEL)) {
            String StringCancelDialog;
            String StringCancelDialogTitle;
            InstallData data = InstallData.getInstance();
            if ( data.isInstallationMode() ) {
                StringCancelDialog = ResourceManager.getString("String_Cancel_Dialog");
            } else {
                StringCancelDialog = ResourceManager.getString("String_Cancel_Dialog_Uninstallation");
            }
            StringCancelDialogTitle = ResourceManager.getString("String_Cancel_Dialog_Title");
            JDialog dialog = setupFrame.getDialog();
            int n = JOptionPane.showConfirmDialog(dialog, StringCancelDialog, StringCancelDialogTitle,
                                                      JOptionPane.YES_NO_OPTION);
            if ( n == 0 ) {
                setupFrame.close(SetupFrame.CODE_CANCEL);
            }
            setupFrame.setButtonSelected(setupFrame.BUTTON_CANCEL);
        } else if (evt.getActionCommand().equals(SetupFrame.ACTION_STOP)) {
            String StringStopDialog;
            String StringStopDialogTitle;
            InstallData data = InstallData.getInstance();
            if ( data.isInstallationMode() ) {
                StringStopDialog = ResourceManager.getString("String_Stop_Dialog");
                StringStopDialogTitle = ResourceManager.getString("String_Stop_Dialog_Title");
            } else {
                StringStopDialog = ResourceManager.getString("String_Stop_Dialog_Uninstallation");
                StringStopDialogTitle = ResourceManager.getString("String_Stop_Dialog_Title_Uninstallation");
            }
            JDialog dialog = setupFrame.getDialog();
            int n = JOptionPane.showConfirmDialog(dialog, StringStopDialog, StringStopDialogTitle,
                                                      JOptionPane.YES_NO_OPTION);
            if ( n == 0 ) {
                AbortInstaller.abortInstallProcess();
            }
            // setting focus on help button, if not aborted
            setupFrame.setButtonSelected(setupFrame.BUTTON_HELP);
            // PanelController panel = setupFrame.getCurrentPanel();
            // panel.setStopButtonSelected();
        } else if (evt.getActionCommand().equals(SetupFrame.ACTION_PREVIOUS)) {
            PanelController panel = setupFrame.getCurrentPanel();
            String previous = panel.getPrevious();
            setupFrame.setCurrentPanel(previous, true, false);
        } else if (evt.getActionCommand().equals(SetupFrame.ACTION_NEXT)) {
            PanelController panel = setupFrame.getCurrentPanel();
            String next = panel.getNext();
            if (next == null) {
                setupFrame.close(SetupFrame.CODE_OK);
            } else {
                setupFrame.setCurrentPanel(next, false, true);
            }
        } else if (evt.getActionCommand().equals(SetupFrame.ACTION_DETAILS)) {
            JDialog dialog = setupFrame.getDialog();
            DetailsDialog detailsdialog = new DetailsDialog(setupFrame);
            detailsdialog.setModal(true);
            detailsdialog.setSize(new Dimension(600, 300));
            detailsdialog.setLocationRelativeTo(dialog);
            detailsdialog.setVisible(true);
            // setting focus on next button, if details dialog is closed
            setupFrame.setButtonSelected(setupFrame.BUTTON_NEXT);
        } else if (evt.getActionCommand().equals(SetupFrame.ACTION_HELP)) {
            JDialog dialog = setupFrame.getDialog();
            HelpDialog helpdialog = new HelpDialog(setupFrame);
            helpdialog.setModal(true);
            helpdialog.setSize(new Dimension(400, 300));
            helpdialog.setLocationRelativeTo(dialog);
            helpdialog.setVisible(true);
            setupFrame.setButtonSelected(setupFrame.BUTTON_HELP);
        }
    }
}
