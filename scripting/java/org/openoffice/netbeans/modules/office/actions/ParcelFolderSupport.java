/*************************************************************************
 *
 *  $RCSfile: ParcelFolderSupport.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-02-24 12:53:09 $
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

import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.idesupport.filter.FileFilter;
import org.openoffice.idesupport.ui.ConfigurePanel;
import org.openoffice.idesupport.xml.ParcelDescriptor;

public class ParcelFolderSupport implements ParcelFolderCookie
{
    protected ParcelFolder pf;
    private ConfigurePanel configuror = null;

    public ParcelFolderSupport(ParcelFolder pf) {
        this.pf = pf;
    }

    public String getClasspath() {
        FileObject primary = pf.getPrimaryFile();

        File contents = FileUtil.toFile(
            primary.getFileObject(ParcelZipper.CONTENTS_DIRNAME));

        try {
            ParcelDescriptor pd = new ParcelDescriptor(
                new File(contents, ParcelZipper.PARCEL_DESCRIPTOR_XML));

            return pd.getLanguageProperty("classpath");
        }
        catch (IOException ioe) {
            ErrorManager.getDefault().notify(ioe);
        }
        return "";
    }

    public void setClasspath(String value) {
        FileObject primary = pf.getPrimaryFile();

        File contents = FileUtil.toFile(
            primary.getFileObject(ParcelZipper.CONTENTS_DIRNAME));

        try {
            ParcelDescriptor pd = new ParcelDescriptor(
                new File(contents, ParcelZipper.PARCEL_DESCRIPTOR_XML));

            pd.setLanguageProperty("classpath", value);
            pd.write();
        }
        catch (IOException ioe) {
            ErrorManager.getDefault().notify(ioe);
        }
    }

    public void generate() {
        ParcelFolder.ParcelFolderNode node =
            (ParcelFolder.ParcelFolderNode)pf.getNodeDelegate();

        FileObject parcelBase = pf.getPrimaryFile();
        FileObject contentsBase =
            parcelBase.getFileObject(ParcelZipper.CONTENTS_DIRNAME);

        File parcelDir = FileUtil.toFile(parcelBase);
        File contentsDir = FileUtil.toFile(contentsBase);

        // The Location property is not displayed so just
        // use the Parcel Recipe directory as the target directory
        File targetDir = FileUtil.toFile(pf.getPrimaryFile());
        File targetfile = new File(targetDir, File.separator +
            parcelBase.getName() + "." + ParcelZipper.PARCEL_EXTENSION);

        boolean proceed = configure();
        if (proceed == false)
            return;

        final OutputWriter out =
            ParcelSupport.getOutputWindowWriter(parcelDir.getName() + " (generating)");
        try {
            out.println("Generating: " + parcelDir.getName(), null);
            ParcelZipper.getParcelZipper().zipParcel(contentsDir, targetfile, node.getFileFilter());
            out.println("\nGENERATION SUCCESSFUL.");
            out.println("\nRight click on the generated parcel to deploy it");

            if (targetDir.equals(parcelDir))
                parcelBase.refresh(true);
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

        FileObject primary = pf.getPrimaryFile();

        File contents = FileUtil.toFile(
            primary.getFileObject(ParcelZipper.CONTENTS_DIRNAME));

        File parcelDescriptor = new File(contents,
            ParcelZipper.PARCEL_DESCRIPTOR_XML);

        Vector classpath = getConfigureClasspath();
        classpath.addElement(contents.getAbsolutePath());

        try {
            if (configuror == null)
                configuror = new ConfigurePanel(
                    contents.getAbsolutePath(), classpath);
            else
                configuror.reload(contents.getAbsolutePath(), classpath);
        }
        catch (IOException ioe) {
            ErrorManager.getDefault().notify(ioe);
        }

        DialogDescriptor dd = new DialogDescriptor(configuror,
            ConfigurePanel.DIALOG_TITLE);

        Dialog dialog = TopManager.getDefault().createDialog(dd);
        dialog.show();

        if (dd.getValue() == DialogDescriptor.OK_OPTION) {
            try {
                Document doc = configuror.getConfiguration();
                FileOutputStream fos = new FileOutputStream(parcelDescriptor);
                XMLUtil.write(doc, fos, "");
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
