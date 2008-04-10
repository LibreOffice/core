/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SetupActionListener.java,v $
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
