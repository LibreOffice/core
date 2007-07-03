/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UninstallationCompletedCtrl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:52:17 $
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

package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.UninstallationCompleted;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.ProductDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.InfoCtrl;
import org.openoffice.setup.Util.LogManager;
import java.util.Vector;

public class UninstallationCompletedCtrl extends PanelController {

    private String helpFile;
    private String mDialogText;
    private String htmlInfoText;

    public UninstallationCompletedCtrl() {
        super("UninstallationCompleted", new UninstallationCompleted());
        helpFile = "String_Helpfile_UninstallationCompleted";
    }

    public void beforeShow() {
        InstallData installData = InstallData.getInstance();
        ProductDescription productData = SetupDataProvider.getProductDescription();

        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_PREVIOUS);
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_CANCEL);
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_HELP);
        getSetupFrame().removeButtonIcon(getSetupFrame().BUTTON_NEXT);
        getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_NEXT);

        UninstallationCompleted panel = (UninstallationCompleted)getPanel();
        panel.setDetailsButtonActionCommand(getSetupFrame().ACTION_DETAILS);
        panel.addDetailsButtonActionListener(getSetupFrame().getSetupActionListener());

        if (( installData.isCustomInstallation() ) && ( ! installData.isMaskedCompleteUninstallation() )) {
            String dialogText = ResourceManager.getString("String_UninstallationCompleted2_Partial");
            panel.setDialogText(dialogText);
        }

        if ( installData.isAbortedInstallation() ) {
            String titleText = ResourceManager.getString("String_UninstallationCompleted1_Abort");
            panel.setTitleText(titleText);
            String dialogText = ResourceManager.getString("String_UninstallationCompleted2_Abort");
            panel.setDialogText(dialogText);
        } else if ( installData.isErrorInstallation() ) {
            String titleText = ResourceManager.getString("String_UninstallationCompleted1_Error");
            panel.setTitleText(titleText);
            String dialogText = ResourceManager.getString("String_UninstallationCompleted2_Error");
            panel.setDialogText(dialogText);
        }

        htmlInfoText = InfoCtrl.setHtmlFrame("header", htmlInfoText);
        htmlInfoText = InfoCtrl.setInstallLogInfoText(productData, htmlInfoText);
        htmlInfoText = InfoCtrl.setHtmlFrame("end", htmlInfoText);
    }

    public String getNext() {
        return null;
    }

    public String getPrevious() {
        return null;
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public String getDialogText() {
        return htmlInfoText;
    }

}
