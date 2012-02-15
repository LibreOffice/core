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



package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
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
        InstallData data = InstallData.getInstance();

        if ( data.isRootInstallation() ) {
            if ( data.olderVersionExists() ) {
                return new String("InstallationImminent");
            } else if ( data.sameVersionExists() ) {
                return new String("ChooseComponents");
            } else {
                return new String("ChooseInstallationType");
            }
        } else {
            return new String("ChooseDirectory");
        }
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

        getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_CANCEL);
    }

}
