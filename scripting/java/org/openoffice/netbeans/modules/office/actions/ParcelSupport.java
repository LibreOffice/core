/*************************************************************************
 *
 *  $RCSfile: ParcelSupport.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-02-12 12:41:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

public class ParcelSupport implements ParcelCookie
{
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
            }
            catch (IOException ioe) {
                return null;
            }
        }
        return language;
    }

    public void mount()
    {
        File parcel = FileUtil.toFile(fo);

        if (parcel != null) {
            try {
                ZipMounter.getZipMounter().mountZipFile(parcel);
            }
            catch (IOException ioe) {
                ErrorManager.getDefault().notify(ioe);
            }
            catch (PropertyVetoException pve) {
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

            if (settings.getWarnBeforeDocDeploy() == true) {
                NagDialog warning = NagDialog.createConfirmationDialog(
                    message, "Show this message in future", true);

                boolean result = warning.show();

                if (warning.getState() == false)
                    settings.setWarnBeforeDocDeploy(false);

                if (result == false)
                    return false;
            }
        }

        OutputWriter out =
            getOutputWindowWriter(fo.getName() + " (deploying)");

        try {
            if (zipper.isOverwriteNeeded(source, target) == true)
                if (promptForOverwrite(source, target) == false)
                    return false;
        }
        catch (IOException ioe) {
           out.println("DEPLOYMENT FAILED: reason: " +
               ioe.getClass().getName() + ": "+ ioe.getMessage());
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
        }
        catch (IOException ioe) {
           out.println("DEPLOYMENT FAILED: reason: " +
               ioe.getClass().getName() + ": "+ ioe.getMessage());
           return false;
        }
        finally {
            if( out != null)
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
        }
        catch( IOException e) {
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

        if (d.getValue() == NotifyDescriptor.CANCEL_OPTION)
            return false;
        else
            return true;
    }
}
