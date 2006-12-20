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
