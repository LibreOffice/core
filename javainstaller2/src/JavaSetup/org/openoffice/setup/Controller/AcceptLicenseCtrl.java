/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AcceptLicenseCtrl.java,v $
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

        getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_CANCEL);
    }

}
