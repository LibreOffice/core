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
