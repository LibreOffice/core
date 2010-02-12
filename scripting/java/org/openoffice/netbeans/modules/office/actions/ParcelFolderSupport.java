/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.netbeans.modules.office.actions;

import java.util.Vector;
import java.util.StringTokenizer;

import java.io.*;
import java.beans.PropertyVetoException;
import java.awt.Dialog;

import org.w3c.dom.Document;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import org.openide.TopManager;
import org.openide.DialogDescriptor;
import org.openide.ErrorManager;
import org.openide.xml.XMLUtil;
import org.openide.execution.NbClassPath;

import org.openide.cookies.OpenCookie;
import org.openide.loaders.DataObject;
import org.openide.loaders.DataNode;

import org.openide.filesystems.FileObject;
import org.openide.filesystems.FileSystem;
import org.openide.filesystems.JarFileSystem;
import org.openide.filesystems.FileUtil;
import org.openide.filesystems.Repository;

import org.openide.nodes.*;
import org.openide.windows.OutputWriter;

import org.openoffice.netbeans.modules.office.loader.ParcelFolder;
import org.openoffice.netbeans.modules.office.options.OfficeSettings;
import org.openoffice.netbeans.modules.office.utils.ManifestParser;

import com.sun.star.script.framework.container.ParcelDescriptor;

import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.idesupport.filter.FileFilter;
import org.openoffice.idesupport.ui.ConfigurePanel;

public class ParcelFolderSupport implements ParcelFolderCookie
{
    protected ParcelFolder parcelFolder;
    private ConfigurePanel configuror = null;

    public ParcelFolderSupport(ParcelFolder parcelFolder) {
        this.parcelFolder = parcelFolder;
    }

    public String getLanguage() {
        ParcelDescriptor descriptor = getParcelDescriptor();

        if (descriptor == null) {
            return "";
        }
        else {
            return descriptor.getLanguage();
        }
    }

    public String getClasspath() {
        ParcelDescriptor descriptor = getParcelDescriptor();

        if (descriptor == null) {
            return "";
        }
        else {
            return descriptor.getLanguageProperty("classpath");
        }
    }

    public void setClasspath(String value) {
        ParcelDescriptor descriptor = getParcelDescriptor();

        if (descriptor != null) {
            descriptor.setLanguageProperty("classpath", value);

            try {
                descriptor.write();
            }
            catch (IOException ioe) {
                ErrorManager.getDefault().notify(ioe);
            }
        }
    }

    private ParcelDescriptor getParcelDescriptor() {
        FileObject primary = parcelFolder.getPrimaryFile();

        File contents = FileUtil.toFile(
            primary.getFileObject(ParcelZipper.CONTENTS_DIRNAME));

        return ParcelDescriptor.getParcelDescriptor(contents);
    }

    public void generate() {
        ParcelFolder.ParcelFolderNode node =
            (ParcelFolder.ParcelFolderNode)parcelFolder.getNodeDelegate();

        FileObject parcelBase = parcelFolder.getPrimaryFile();
        FileObject contentsBase =
            parcelBase.getFileObject(ParcelZipper.CONTENTS_DIRNAME);

        File parcelDir = FileUtil.toFile(parcelBase);
        File contentsDir = FileUtil.toFile(contentsBase);

        // The Location property is not displayed so just
        // use the Parcel Recipe directory as the target directory
        File targetDir = FileUtil.toFile(parcelFolder.getPrimaryFile());
        File targetfile = new File(targetDir, File.separator +
            parcelBase.getName() + "." + ParcelZipper.PARCEL_EXTENSION);

        boolean proceed = configure();
        if (proceed == false) {
            return;
        }

        final OutputWriter out =
            ParcelSupport.getOutputWindowWriter(parcelDir.getName() + " (generating)");
        try {
            out.println("Generating: " + parcelDir.getName(), null);
            ParcelZipper.getParcelZipper().zipParcel(contentsDir, targetfile, node.getFileFilter());
            out.println("\nGENERATION SUCCESSFUL.");
            out.println("\nRight click on the generated parcel to deploy it");

            if (targetDir.equals(parcelDir)) {
                parcelBase.refresh(true);
            }
        }
        catch (IOException ioe) {
            out.println("GENERATION FAILED: reason: " + ioe.getClass().getName() + ": "+ ioe.getMessage());
        }
        finally
        {
            if( out != null)
            {
                out.close();
            }
        }
    }

    public boolean configure() {

        FileObject primary = parcelFolder.getPrimaryFile();

        File contents = FileUtil.toFile(
            primary.getFileObject(ParcelZipper.CONTENTS_DIRNAME));

        Vector classpath = getConfigureClasspath();
        classpath.addElement(contents.getAbsolutePath());

        try {
            ParcelDescriptor descriptor = getParcelDescriptor();
            if (descriptor == null) {
                descriptor = ParcelDescriptor.createParcelDescriptor(contents);
            }

            if (configuror == null) {
                configuror = new ConfigurePanel(contents.getAbsolutePath(),
                    classpath, descriptor);
            }
            else {
                configuror.reload(contents.getAbsolutePath(), classpath,
                    descriptor);
            }
        }
        catch (IOException ioe) {
            ErrorManager.getDefault().notify(ioe);
            return false;
        }

        DialogDescriptor dd = new DialogDescriptor(configuror,
            ConfigurePanel.DIALOG_TITLE);

        Dialog dialog = TopManager.getDefault().createDialog(dd);
        dialog.show();

        if (dd.getValue() == DialogDescriptor.OK_OPTION) {
            try {
                ParcelDescriptor descriptor = configuror.getConfiguration();
                descriptor.write();
            }
            catch (Exception e) {
                ErrorManager.getDefault().notify(e);
            }
        }
        else {
            return false;
        }
        return true;
    }

    private Vector getConfigureClasspath() {
        Vector result = new Vector();

        String classpath = NbClassPath.createRepositoryPath().getClassPath();
        if ( System.getProperty( "os.name" ).startsWith( "Windows" ) )
        {
            // under windows path is enclosed by quotes
            // e.g. C:\path1;d:\path2 would appear as
            // "C:\path1;d:\path2" therefore for us
            // we need to remove 1 character at either end of the
            // classpath returned from "createRepositoryPath().getClassPath()"

            if ( classpath.startsWith("\"") && classpath.endsWith("\"") )
            {
               StringBuffer buff = new StringBuffer(classpath);
               buff.delete(0,1);
               buff.delete( buff.length() - 1, buff.length() );
               classpath = buff.toString();
            }
        }
        StringTokenizer tokens = new StringTokenizer(classpath, File.pathSeparator);

        while(tokens.hasMoreTokens())
            result.addElement(tokens.nextToken());

        OfficeSettings settings = OfficeSettings.getDefault();
        File classesDir = new File(settings.getOfficeDirectory().getPath(
            File.separator + "program" + File.separator + "classes"));
        File[] jarfiles = classesDir.listFiles();

        for (int i = 0; i < jarfiles.length; i++)
            result.addElement(jarfiles[i].getAbsolutePath());

        return result;
    }
}
