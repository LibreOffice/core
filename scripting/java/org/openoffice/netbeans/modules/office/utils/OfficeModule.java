/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeModule.java,v $
 * $Revision: 1.7 $
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
