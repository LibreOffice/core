/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FrameworkJarChecker.java,v $
 * $Revision: 1.5 $
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

import java.io.File;
import java.io.IOException;
import java.beans.PropertyVetoException;

import org.openide.filesystems.Repository;
import org.openide.filesystems.FileSystem;
import org.openide.filesystems.JarFileSystem;

import org.openoffice.idesupport.SVersionRCFile;
import org.openoffice.netbeans.modules.office.options.OfficeSettings;

public class FrameworkJarChecker {

    public static void mountDependencies() {
        String unoilPath = SVersionRCFile.getPathForUnoil(
            OfficeSettings.getDefault().getOfficeDirectory().getPath());

        if (unoilPath == null)
            return;

        File unoilFile = new File(unoilPath + File.separator + "unoil.jar");
        JarFileSystem jfs = new JarFileSystem();
        try {
            jfs.setJarFile(unoilFile);
        }
        catch (IOException ioe) {
            return;
        }
        catch (PropertyVetoException pve) {
            return;
        }

        FileSystem result;
        try {
            result =
                Repository.getDefault().findFileSystem(jfs.getSystemName());
        }
        catch(Exception exp) {
            result = null;
        }
        finally {
            jfs.removeNotify();
        }

        if(result == null) {
            // warnBeforeMount();
            JarFileSystem newjfs = new JarFileSystem();
            try {
                newjfs.setJarFile(unoilFile);
            }
            catch (IOException ioe) {
                return;
            }
            catch (PropertyVetoException pve) {
                return;
            }
            Repository.getDefault().addFileSystem(newjfs);
            newjfs.setHidden(true);
        }
    }

    public static void unmountDependencies() {
        String unoilPath = SVersionRCFile.getPathForUnoil(
            OfficeSettings.getDefault().getOfficeDirectory().getPath());

        if (unoilPath == null)
            return;

        File unoilFile = new File(unoilPath + File.separator + "unoil.jar");
        JarFileSystem jfs = new JarFileSystem();
        try {
            jfs.setJarFile(unoilFile);
        }
        catch (IOException ioe) {
            return;
        }
        catch (PropertyVetoException pve) {
            return;
        }

        FileSystem result;
        try {
            result =
                Repository.getDefault().findFileSystem(jfs.getSystemName());
            if(result != null)
                Repository.getDefault().removeFileSystem(result);
        }
        catch(Exception exp) {
        }
    }

    private static void warnBeforeMount() {
        OfficeSettings settings = OfficeSettings.getDefault();

        if (settings.getWarnBeforeMount() == false)
            return;

        String message = "The Office Scripting Framework support jar file " +
            "is not mounted, so Office scripts will not compile. NetBeans " +
            "is going to mount this jar file automatically.";

        String prompt = "Show this message in future.";

        NagDialog warning = NagDialog.createInformationDialog(
            message, prompt, true);

        if (warning.getState() == false) {
            settings.setWarnBeforeMount(false);
        }
    }
}
