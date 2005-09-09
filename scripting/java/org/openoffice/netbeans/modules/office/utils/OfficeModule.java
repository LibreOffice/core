/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeModule.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:20:16 $
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
