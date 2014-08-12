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
import java.util.zip.*;
import org.openoffice.idesupport.filter.FileFilter;
import org.openoffice.idesupport.filter.BinaryOnlyFilter;
import org.openoffice.idesupport.xml.Manifest;

public class ParcelZipper
{
    public static final String PARCEL_PREFIX_DIR = "Scripts/";
    private static final String PARCEL_EXTENSION = "sxp";
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

    private Manifest removeParcelFromManifest(File document, String name) {
        Manifest result = null;

        result = getManifestFromDocument(document);
        if (result == null)
            return null;

        result.remove(name);
        return result;
    }
}
