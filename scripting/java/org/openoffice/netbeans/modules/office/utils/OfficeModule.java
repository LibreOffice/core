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

import org.openide.TopManager;
import org.openide.WizardDescriptor;
import org.openide.NotifyDescriptor;
import org.openide.modules.ModuleInstall;

import com.sun.star.script.framework.container.XMLParserFactory;
import org.openoffice.idesupport.OfficeInstallation;
import org.openoffice.netbeans.modules.office.wizard.InstallationPathDescriptor;
import org.openoffice.netbeans.modules.office.options.OfficeSettings;

public class OfficeModule extends ModuleInstall {

    private static final long serialVersionUID = -8499324854301243852L;

    public void installed () {
        WizardDescriptor wiz = new InstallationPathDescriptor();
        TopManager.getDefault().createDialog(wiz).show();

        if(wiz.getValue() == NotifyDescriptor.OK_OPTION) {
            OfficeInstallation oi = (OfficeInstallation)
                wiz.getProperty(InstallationPathDescriptor.PROP_INSTALLPATH);

            OfficeSettings settings = OfficeSettings.getDefault();
            settings.setOfficeDirectory(oi);
        }
        FrameworkJarChecker.mountDependencies();
        XMLParserFactory.setParser(ManifestParser.getManifestParser());
    }

    public void restored () {
        FrameworkJarChecker.mountDependencies();
        XMLParserFactory.setParser(ManifestParser.getManifestParser());
    }

    public boolean closing () {
        FrameworkJarChecker.unmountDependencies();
        return true;
    }
}
