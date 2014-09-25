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

package org.openoffice.netbeans.modules.office.actions;

import java.io.File;
import java.io.IOException;
import java.beans.PropertyVetoException;
import java.util.Enumeration;
import java.util.Calendar;

import org.openide.TopManager;
import org.openide.NotifyDescriptor;
import org.openide.windows.OutputWriter;
import org.openide.windows.InputOutput;

import org.openide.ErrorManager;
import org.openide.nodes.Node;
import org.openide.filesystems.Repository;
import org.openide.filesystems.FileSystem;
import org.openide.filesystems.FileObject;
import org.openide.filesystems.FileUtil;
import org.openide.filesystems.FileEvent;

import org.openoffice.idesupport.zip.ParcelZipper;

import org.openoffice.netbeans.modules.office.options.OfficeSettings;
import org.openoffice.netbeans.modules.office.utils.NagDialog;
import org.openoffice.netbeans.modules.office.utils.ZipMounter;
import org.openoffice.netbeans.modules.office.utils.ManifestParser;

public class ParcelSupport implements ParcelCookie {
    private FileObject fo;
    private ParcelZipper zipper = ParcelZipper.getParcelZipper();
    private String language = null;

    public ParcelSupport(FileObject fo) {
        this.fo = fo;
    }

    public File getFile() {
        return FileUtil.toFile(fo);
    }

    public String getLanguage() {
        if (language == null) {
            try {
                language = zipper.getParcelLanguage(getFile());
            } catch (IOException ioe) {
                return null;
            }
        }

        return language;
    }

    public void mount() {
        File parcel = FileUtil.toFile(fo);

        if (parcel != null) {
            try {
                ZipMounter.getZipMounter().mountZipFile(parcel);
            } catch (IOException ioe) {
                ErrorManager.getDefault().notify(ioe);
            } catch (PropertyVetoException pve) {
                ErrorManager.getDefault().notify(pve);
            }
        }
    }

    public boolean deploy(File target) {
        File source = FileUtil.toFile(fo);

        if (!target.isDirectory()) {
            OfficeSettings settings = OfficeSettings.getDefault();
            String message = "If you already have this document open in " +
                             "Office, please close it before continuing. Click OK to " +
                             "continue deployment.";

            if (settings.getWarnBeforeDocDeploy()) {
                NagDialog warning = NagDialog.createConfirmationDialog(
                                        message, "Show this message in future", true);

                boolean result = warning.show();

                if (!warning.getState())
                    settings.setWarnBeforeDocDeploy(false);

                if (!result)
                    return false;
            }
        }

        OutputWriter out =
            getOutputWindowWriter(fo.getName() + " (deploying)");

        try {
            if (!zipper.isOverwriteNeeded(source, target))
                if (!promptForOverwrite(source, target))
                    return false;
        } catch (IOException ioe) {
            out.println("DEPLOYMENT FAILED: reason: " +
                        ioe.getClass().getName() + ": " + ioe.getMessage());
            return false;
        }

        try {
            out.println("Deploying: " + fo.getName() +
                        "\nTo: " + target.getAbsolutePath(), null);

            zipper.deployParcel(source, target);

            out.println("\nDEPLOYMENT SUCCESSFUL.");

            FileObject[] fileobjs = FileUtil.fromFile(target);

            if (fileobjs != null) {
                for (int i = 0; i < fileobjs.length; i++)
                    fileobjs[i].refresh(true);
            }
        } catch (IOException ioe) {
            out.println("DEPLOYMENT FAILED: reason: " +
                        ioe.getClass().getName() + ": " + ioe.getMessage());
            return false;
        } finally {
            if (out != null)
                out.close();
        }

        return true;
    }

    public static OutputWriter getOutputWindowWriter(String title) {
        InputOutput io = TopManager.getDefault().getIO(title, false);
        io.setFocusTaken(true);
        io.setOutputVisible(true);

        OutputWriter out = io.getOut();

        try {
            out.reset();
        } catch (IOException e) {
            e.printStackTrace(System.err);
        }

        out.println(Calendar.getInstance().getTime() + ":\n");
        return out;
    }

    private boolean promptForOverwrite(File source, File target) {
        String message = source.getName() + " has already been deployed " +
                         "to this target. Do you wish to overwrite it?";

        NotifyDescriptor d = new NotifyDescriptor.Confirmation(
            message, NotifyDescriptor.OK_CANCEL_OPTION);
        TopManager.getDefault().notify(d);

        return (d.getValue() != NotifyDescriptor.CANCEL_OPTION);
    }
}
