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

package org.openoffice.idesupport.zip;

import java.io.*;
import java.util.Enumeration;
import java.util.zip.*;
import org.openoffice.idesupport.filter.FileFilter;
import org.openoffice.idesupport.filter.BinaryOnlyFilter;
import com.sun.star.script.framework.container.ParcelDescriptor;
import org.openoffice.idesupport.xml.Manifest;

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
        ZipOutputStream out = null;
        try {
            if (tmpfile.exists() == true)
                tmpfile.delete();

            out = new ZipOutputStream(new FileOutputStream(tmpfile));

            File[] children = basedir.listFiles();
            for (int i = 0; i < children.length; i++)
                addFileToParcel(children[i], "", out, filter);
        }
        catch (IOException ioe) {
            out.close();
            tmpfile.delete();
            throw ioe;
        }
        finally {
            if (out != null)
                out.close();
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

            FileInputStream fis = null;
            try {
                fis = new FileInputStream(root);

                while ((len = fis.read(bytes)) != -1)
                    out.write(bytes, 0, len);
            }
            finally {
                if (fis != null) fis.close();
            }
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
        ZipFile documentZip = null;
        boolean result = false;

        try {
            documentZip = new ZipFile(document);

            String name =
                PARCEL_PREFIX_DIR + getParcelLanguage(parcel) +
                    "/" + getParcelDirFromParcelZip(parcel.getName()) +
                    "/" + PARCEL_DESCRIPTOR_XML;

            if (documentZip.getEntry(name) != null)
                result = true;
        }
        catch (IOException ioe) {
            return false;
        }
        finally {
            if (documentZip != null) documentZip.close();
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

        ZipInputStream in = null;
        File parcelDir = new File(targetDirectory,
            getParcelLanguage(parcel) + File.separator +
            getParcelDirFromParcelZip(parcel.getName()));

        if (isDirectoryOverwriteNeeded(parcel, targetDirectory)) {
            if (deleteDir(parcelDir) == false) {
                throw new IOException("Could not overwrite: " +
                    parcelDir.getAbsolutePath());
            }
        }

        try {
            in = new ZipInputStream(new FileInputStream(parcel));

            File outFile;
            ZipEntry inEntry = in.getNextEntry();
            byte[] bytes = new byte[1024];
            int len;

            while (inEntry != null) {
                outFile = new File(parcelDir, inEntry.getName());
                if (inEntry.isDirectory()) {
                    outFile.mkdir();
                }
                else {
                    if (outFile.getParentFile().exists() != true)
                        outFile.getParentFile().mkdirs();

                    FileOutputStream out = null;
                    try {
                        out = new FileOutputStream(outFile);

                        while ((len = in.read(bytes)) != -1)
                            out.write(bytes, 0, len);
                    }
                    finally {
                        if (out != null) out.close();
                    }
                }
                inEntry = in.getNextEntry();
            }
        }
        finally {
            if (in != null) in.close();
        }

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

        ZipInputStream documentStream = null;
        ZipInputStream parcelStream = null;
        ZipOutputStream outStream = null;
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

        try {
            documentStream =
                new ZipInputStream(new FileInputStream(targetDocument));
            parcelStream = new ZipInputStream(new FileInputStream(parcel));
            outStream = new ZipOutputStream(new FileOutputStream(tmpfile));

            copyParcelToZip(parcelStream, outStream, PARCEL_PREFIX_DIR +
                language + "/" + getParcelDirFromParcelZip(parcel.getName()));
            copyDocumentToZip(documentStream, outStream, manifest);
        }
        catch (IOException ioe) {
            tmpfile.delete();
            throw ioe;
        }
        finally {
            if (documentStream != null) documentStream.close();
            if (parcelStream != null) parcelStream.close();
            if (outStream != null) outStream.close();
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
                InputStream manifestStream = null;
                try {
                    manifestStream = manifest.getInputStream();
                    while ((len = manifestStream.read(bytes)) != -1)
                        out.write(bytes, 0, len);
                }
                finally {
                    if (manifestStream != null)
                        manifestStream.close();
                }
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

        ZipInputStream documentStream = null;
        ZipOutputStream outStream = null;
        Manifest manifest = null;

        if (!parcelName.startsWith(PARCEL_PREFIX_DIR))
            parcelName = PARCEL_PREFIX_DIR + parcelName;
        manifest = removeParcelFromManifest(document, parcelName);

        // first write contents of document to tmpfile
        File tmpfile = new File(document.getAbsolutePath() + ".tmp");

        try {
            ZipEntry outEntry;
            ZipEntry inEntry;
            byte[] bytes = new byte[1024];
            int len;

            documentStream = new ZipInputStream(new FileInputStream(document));
            outStream = new ZipOutputStream(new FileOutputStream(tmpfile));

            while ((inEntry = documentStream.getNextEntry()) != null) {

                if(inEntry.getName().startsWith(parcelName))
                    continue;

                outEntry = new ZipEntry(inEntry);
                outStream.putNextEntry(outEntry);

                if(inEntry.getName().equals("META-INF/manifest.xml") &&
                   manifest != null) {
                    InputStream manifestStream = null;
                    try {
                        manifestStream = manifest.getInputStream();
                        while ((len = manifestStream.read(bytes)) != -1)
                            outStream.write(bytes, 0, len);
                    }
                    finally {
                        if (manifestStream != null)
                            manifestStream.close();
                    }
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
            if (documentStream != null)
                documentStream.close();

            if (outStream != null)
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
        ZipFile documentZip = null;
        Manifest result = null;

        try {
            documentZip = new ZipFile(document);
            ZipEntry original = documentZip.getEntry("META-INF/manifest.xml");
            if (original != null) {
                result = new Manifest(documentZip.getInputStream(original));
            }
        }
        catch (IOException ioe) {
            result = null;
        }
        finally {
            try {
                if (documentZip != null)
                    documentZip.close();
            }
            catch (IOException ioe) {}
        }

        return result;
    }

    private Manifest addParcelToManifest(File document, File parcel)
        throws IOException {

        ZipFile parcelZip = null;
        Manifest result = null;

        result = getManifestFromDocument(document);
        if (result == null)
            return null;

        String language = getParcelLanguage(parcel);

        try {
            parcelZip = new ZipFile(parcel);

            String prefix = PARCEL_PREFIX_DIR + language + "/" +
                getParcelDirFromParcelZip(parcel.getName()) + "/";

            Enumeration entries = parcelZip.entries();
            while (entries.hasMoreElements()) {
                ZipEntry entry = (ZipEntry)entries.nextElement();
                result.add(prefix + entry.getName());
            }
        }
        catch (IOException ioe) {
            return null;
        }
        finally {
            try {
                if (parcelZip != null)
                    parcelZip.close();
            }
            catch (IOException ioe) {}
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
        ZipFile zf = null;
        ZipEntry ze = null;
        InputStream is = null;
        ParcelDescriptor pd;

        try {
            zf = new ZipFile(file);
            ze = zf.getEntry(PARCEL_DESCRIPTOR_XML);

            if (ze == null)
                throw new IOException("Could not find Parcel Descriptor in parcel");

            is = zf.getInputStream(ze);
            pd = new ParcelDescriptor(is);
        }
        finally {
            if (zf != null)
                zf.close();

            if (is != null)
                is.close();
        }

        return pd.getLanguage().toLowerCase();
    }
}
