package org.openoffice.setup.Controller;

import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.AcceptLicense;
import org.openoffice.setup.ResourceManager;

public class AcceptLicenseCtrl extends PanelController {

    private String helpFile;

    public AcceptLicenseCtrl() {
        super("AcceptLicense", new AcceptLicense());
        helpFile = "String_Helpfile_AcceptLicense";
    }

    public String getNext() {
        return new String("ChooseDirectory");
    }

    public String getPrevious() {
        return new String("Prologue");
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public void beforeShow() {
        String StringInstall = ResourceManager.getString("String_AcceptLicense");
        getSetupFrame().setButtonText(StringInstall, getSetupFrame().BUTTON_NEXT);
        String StringDecline = ResourceManager.getString("String_Decline");
        getSetupFrame().setButtonText(StringDecline, getSetupFrame().BUTTON_CANCEL);
    }

}
