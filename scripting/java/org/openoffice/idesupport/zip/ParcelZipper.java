/*************************************************************************
 *
 *  $RCSfile: ParcelZipper.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-02-20 12:03:27 $
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

package org.openoffice.idesupport.zip;

import java.io.*;
import java.util.Enumeration;
import java.util.zip.*;
import java.beans.PropertyVetoException;

import org.openoffice.idesupport.filter.FileFilter;
import org.openoffice.idesupport.filter.BinaryOnlyFilter;
import org.openoffice.idesupport.filter.ExceptParcelFilter;

import org.openoffice.idesupport.xml.Manifest;
import org.openoffice.idesupport.xml.ParcelDescriptor;

public class ParcelZipper
{
    public static final String PARCEL_PREFIX_DIR = "Scripts/";
    public static final String PARCEL_EXTENSION = "sxp";
    public static final String CONTENTS_DIRNAME = "Contents";
    public static final String PARCEL_DESCRIPTOR_XML = "parcel-descriptor.xml";

    private static ParcelZipper zipper = null;

    private static final FileFilter DEFAULT_FILTER =
        BinaryOnlyFilter.getInstance();

    private ParcelZipper() {
    }

    public static ParcelZipper getParcelZipper() {
        if (zipper == null) {
            synchronized(ParcelZipper.class) {
                if (zipper == null)
                    zipper = new ParcelZipper();
            }
        }
        return zipper;
    }

    public String zipParcel(File basedir) throws IOException {
        File targetfile, targetdir;

        if (basedir.getName().equals(CONTENTS_DIRNAME))
            targetdir = basedir.getParentFile();
        else
            targetdir = basedir;

        targetfile = new File(targetdir, targetdir.getName() + "." + PARCEL_EXTENSION);

        return zipParcel(basedir, targetfile, DEFAULT_FILTER);
    }

    public String zipParcel(File basedir, File targetfile) throws IOException {
        return zipParcel(basedir, targetfile, DEFAULT_FILTER);
    }

    public String zipParcel(File basedir, FileFilter filter) throws IOException {
        File targetfile, targetdir;

        if (basedir.getName().equals(CONTENTS_DIRNAME))
            targetdir = basedir.getParentFile();
        else
            targetdir = basedir;

        targetfile = new File(targetdir, targetdir.getName() + "." + PARCEL_EXTENSION);

        return zipParcel(basedir, targetfile, filter);
    }

    public String zipParcel(File basedir, File targetfile, FileFilter filter)
        throws IOException {
        String realpath, tmppath;

        realpath = targetfile.getPath();
        tmppath = realpath + ".tmp";

        File tmpfile = new File(tmppath);
        try {
            if (tmpfile.exists() == true)
                tmpfile.delete();

            ZipOutputStream out =
                new ZipOutputStream(new FileOutputStream(tmpfile));

            File[] children = basedir.listFiles();
            for (int i = 0; i < children.length; i++)
                addFileToParcel(children[i], "", out, filter);

            out.close();
        }
        catch (IOException ioe) {
            tmpfile.delete();
            throw ioe;
        }

        if (targetfile.exists() == true)
            targetfile.delete();
        tmpfile.renameTo(targetfile);
        return targetfile.getAbsolutePath();
    }

    private void addFileToParcel(File root, String path, ZipOutputStream out, FileFilter filter)
        throws IOException {
        ZipEntry ze;

        if (root.isDirectory() == true) {
            ze = new ZipEntry(/* PARCEL_PREFIX_DIR + */ path + root.getName() + "/");
            out.putNextEntry(ze);
            out.closeEntry();
            File[] children = root.listFiles();

            for (int i = 0; i < children.length; i++)
                addFileToParcel(children[i], path + root.getName() + "/", out, filter);
        }
        else {
            if (filter.validate(root.getName()) == false &&
                root.getName().equals("parcel-descriptor.xml") == false)
                return;

            ze = new ZipEntry(/* PARCEL_PREFIX_DIR + */ path + root.getName());
            out.putNextEntry(ze);

            byte[] bytes = new byte[1024];
            int len;
            FileInputStream fis = new FileInputStream(root);

            while ((len = fis.read(bytes)) != -1)
                out.write(bytes, 0, len);

            out.closeEntry();
        }
    }

    public boolean isOverwriteNeeded(File parcel, File target)
        throws IOException
    {
        boolean result;

        if (target.isDirectory())
            result = isDirectoryOverwriteNeeded(parcel, target);
        else
            result = isDocumentOverwriteNeeded(parcel, target);

        return result;
    }

    private boolean isDirectoryOverwriteNeeded(File parcel, File target) {
        String parcelDir = getParcelDirFromParcelZip(parcel.getName());

        File langdir;
        try {
            langdir = new File(target, getParcelLanguage(parcel));
        }
        catch (IOException ioe) {
            return false;
        }

        if (langdir.exists() == false)
            return false;

        File[] children = langdir.listFiles();

        for (int i = 0; i < children.length; i++)
            if (children[i].getName().equals(parcelDir))
                return true;

        return false;
    }

    private boolean isDocumentOverwriteNeeded(File parcel, File document)
        throws IOException
    {
        ZipFile documentZip;
        boolean result = false;

        try {
            documentZip = new ZipFile(document);
        }
        catch (IOException ioe) {
            return false;
        }

        String name =
            PARCEL_PREFIX_DIR + getParcelLanguage(parcel) +
                "/" + getParcelDirFromParcelZip(parcel.getName()) +
                "/" + PARCEL_DESCRIPTOR_XML;

        if (documentZip.getEntry(name) != null)
            result = true;

        try {
            documentZip.close();
        }
        catch (IOException ioe) {
        }

        return result;
    }

    public String deployParcel(File parcel, File target)
        throws IOException {

        String output = null;
        if (target.isDirectory())
            output = unzipToDirectory(parcel, target);
        else
            output = unzipToZip(parcel, target);
        return output;
    }

    private String getParcelDirFromParcelZip(String zipname) {
        String result = zipname.substring(0, zipname.lastIndexOf("."));
        return result;
    }

    private String unzipToDirectory(File parcel, File targetDirectory)
        throws IOException {

        ZipInputStream in;
        File parcelDir = new File(targetDirectory,
            getParcelLanguage(parcel) + File.separator +
            getParcelDirFromParcelZip(parcel.getName()));

        if (isDirectoryOverwriteNeeded(parcel, targetDirectory)) {
            if (deleteDir(parcelDir) == false) {
                throw new IOException("Could not overwrite: " +
                    parcelDir.getAbsolutePath());
            }
        }

        in = new ZipInputStream(new FileInputStream(parcel));

        File outFile;
        ZipEntry inEntry = in.getNextEntry();
        byte[] bytes = new byte[1024];
        int len;

        while (inEntry != null) {
            outFile = new File(parcelDir, inEntry.getName());
            if (inEntry.isDirectory()) {
                //System.err.println("Creating directory: " + outFile.getAbsolutePath());
                outFile.mkdir();
            }
            else {
                if (outFile.getParentFile().exists() != true)
                    outFile.getParentFile().mkdirs();

                FileOutputStream out = new FileOutputStream(outFile);
                // System.out.println("Writing file: " + outFile.getAbsolutePath());
                while ((len = in.read(bytes)) != -1)
                    out.write(bytes, 0, len);
                out.close();
            }

            inEntry = in.getNextEntry();
        }
        in.close();
        return parcelDir.getAbsolutePath();
    }

    private boolean deleteDir(File basedir) {
        if (basedir.isDirectory()) {
            String[] children = basedir.list();
            for (int i=0; i<children.length; i++) {
                boolean success = deleteDir(new File(basedir, children[i]));
                if (!success) {
                    return false;
                }
            }
        }
        return basedir.delete();
    }

    private String unzipToZip(File parcel, File targetDocument)
        throws IOException {

        ZipInputStream documentStream, parcelStream;
        ZipOutputStream outStream;
        Manifest manifest;

        String language = getParcelLanguage(parcel);

        if (isDocumentOverwriteNeeded(parcel, targetDocument)) {
            String parcelName = language + "/" +
                getParcelDirFromParcelZip(parcel.getName());
            removeParcel(targetDocument, parcelName);
        }

        // first write contents of document to tmpfile
        File tmpfile = new File(targetDocument.getAbsolutePath() + ".tmp");

        manifest = addParcelToManifest(targetDocument, parcel);

        documentStream =
            new ZipInputStream(new FileInputStream(targetDocument));
        parcelStream = new ZipInputStream(new FileInputStream(parcel));
        outStream = new ZipOutputStream(new FileOutputStream(tmpfile));

        try {
            copyParcelToZip(parcelStream, outStream, PARCEL_PREFIX_DIR +
                language + "/" + getParcelDirFromParcelZip(parcel.getName()));
            copyDocumentToZip(documentStream, outStream, manifest);
            documentStream.close();
            parcelStream.close();
            outStream.close();
        }
        catch (IOException ioe) {
            tmpfile.delete();
            throw ioe;
        }

        if (targetDocument.delete() == false) {
            tmpfile.delete();
            throw new IOException("Could not overwrite " + targetDocument);
        }
        else
            tmpfile.renameTo(targetDocument);

        return targetDocument.getAbsolutePath();
    }

    private void copyParcelToZip(ZipInputStream in, ZipOutputStream out,
        String parcelName) throws IOException {

        ZipEntry outEntry;
        ZipEntry inEntry = in.getNextEntry();
        byte[] bytes = new byte[1024];
        int len;

        while (inEntry != null) {
            if(parcelName!=null)
                outEntry = new ZipEntry(parcelName + "/" + inEntry.getName());
            else
                outEntry = new ZipEntry(inEntry);
            out.putNextEntry(outEntry);

            if (inEntry.isDirectory() == false)
                while ((len = in.read(bytes)) != -1)
                    out.write(bytes, 0, len);

            out.closeEntry();
            inEntry = in.getNextEntry();
        }
    }

    private void copyDocumentToZip(ZipInputStream in, ZipOutputStream out,
        Manifest manifest) throws IOException {

        ZipEntry outEntry;
        ZipEntry inEntry;
        byte[] bytes = new byte[1024];
        int len;

        while ((inEntry = in.getNextEntry()) != null) {

            outEntry = new ZipEntry(inEntry);
            out.putNextEntry(outEntry);

            if(inEntry.getName().equals("META-INF/manifest.xml") &&
               manifest != null) {
                InputStream manifestStream = manifest.getInputStream();
                while ((len = manifestStream.read(bytes)) != -1)
                    out.write(bytes, 0, len);
            }
            else if (inEntry.isDirectory() == false) {
                while ((len = in.read(bytes)) != -1)
                    out.write(bytes, 0, len);
            }

            out.closeEntry();
        }
    }

    public String removeParcel(File document, String parcelName)
        throws IOException {

        ZipInputStream documentStream;
        ZipOutputStream outStream;
        Manifest manifest = null;

        if (!parcelName.startsWith(PARCEL_PREFIX_DIR))
            parcelName = PARCEL_PREFIX_DIR + parcelName;
        manifest = removeParcelFromManifest(document, parcelName);

        // first write contents of document to tmpfile
        File tmpfile = new File(document.getAbsolutePath() + ".tmp");

        documentStream = new ZipInputStream(new FileInputStream(document));
        outStream = new ZipOutputStream(new FileOutputStream(tmpfile));

        try {
            ZipEntry outEntry;
            ZipEntry inEntry;
            byte[] bytes = new byte[1024];
            int len;

            while ((inEntry = documentStream.getNextEntry()) != null) {

                if(inEntry.getName().startsWith(parcelName))
                    continue;

                outEntry = new ZipEntry(inEntry);
                outStream.putNextEntry(outEntry);

                if(inEntry.getName().equals("META-INF/manifest.xml") &&
                   manifest != null) {
                    InputStream manifestStream = manifest.getInputStream();
                    while ((len = manifestStream.read(bytes)) != -1)
                        outStream.write(bytes, 0, len);
                }
                else if (inEntry.isDirectory() == false) {
                    while ((len = documentStream.read(bytes)) != -1)
                        outStream.write(bytes, 0, len);
                }

                outStream.closeEntry();
            }
        }
        catch (IOException ioe) {
            tmpfile.delete();
            throw ioe;
        }
        finally {
            documentStream.close();
            outStream.close();
        }

        if (document.delete() == false) {
            tmpfile.delete();
            throw new IOException("Could not overwrite " + document);
        }
        else
            tmpfile.renameTo(document);

        return document.getAbsolutePath();
    }

    private Manifest getManifestFromDocument(File document) {
        ZipFile documentZip;
        Manifest result = null;

        try {
            documentZip = new ZipFile(document);
        }
        catch (IOException ioe) {
            return null;
        }

        ZipEntry original = documentZip.getEntry("META-INF/manifest.xml");
        if (original != null) {
            try {
                result =
                    new Manifest(documentZip.getInputStream(original));
            }
            catch (IOException ioe) {
                result = null;
            }
        }

        try {
            documentZip.close();
        }
        catch (IOException ioe) {
        }

        return result;
    }

    private Manifest addParcelToManifest(File document, File parcel)
        throws IOException {

        ZipFile parcelZip;
        Manifest result = null;

        result = getManifestFromDocument(document);
        if (result == null)
            return null;

        String language = getParcelLanguage(parcel);

        try {
            parcelZip = new ZipFile(parcel);
        }
        catch (IOException ioe) {
            return null;
        }

        String prefix = PARCEL_PREFIX_DIR + language + "/" +
            getParcelDirFromParcelZip(parcel.getName()) + "/";

        Enumeration entries = parcelZip.entries();
        while (entries.hasMoreElements()) {
            ZipEntry entry = (ZipEntry)entries.nextElement();
            result.add(prefix + entry.getName());
        }

        try {
            parcelZip.close();
        }
        catch (IOException ioe) {
        }

        return result;
    }

    private Manifest removeParcelFromManifest(File document, String name) {
        Manifest result = null;

        result = getManifestFromDocument(document);
        if (result == null)
            return null;

        result.remove(name);
        return result;
    }

    public String getParcelLanguage(File file) throws IOException {
        ZipFile zf = new ZipFile(file);
        ZipEntry ze = zf.getEntry(PARCEL_DESCRIPTOR_XML);

        if (ze == null)
            throw new IOException("Could not find Parcel Descriptor in parcel");

        InputStream is = zf.getInputStream(ze);
        ParcelDescriptor pd = new ParcelDescriptor(is);

        return pd.getLanguage().toLowerCase();
    }
}
