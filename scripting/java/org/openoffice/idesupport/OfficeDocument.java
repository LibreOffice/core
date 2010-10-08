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

package org.openoffice.idesupport;

import java.io.*;
import java.util.zip.*;
import java.util.Vector;
import java.util.Enumeration;
import java.util.StringTokenizer;

import org.openoffice.idesupport.filter.FileFilter;
import org.openoffice.idesupport.filter.BinaryOnlyFilter;
import org.openoffice.idesupport.zip.ParcelZipper;

public class OfficeDocument
{
    public static final String PARCEL_PREFIX_DIR =
        ParcelZipper.PARCEL_PREFIX_DIR;

    public static final String[] OFFICE_EXTENSIONS =
        {".sxc" , ".sxw", ".sxi", ".sxd"};
    public static final String OFFICE_PRODUCT_NAME = "OpenOffice.org";

    private File file = null;

    public OfficeDocument(File file) throws IllegalArgumentException
    {
        if (!file.exists() || file.isDirectory() || !isOfficeFile(file)) {
            throw new IllegalArgumentException("This is not a valid " +
                OFFICE_PRODUCT_NAME + " document.");
        }
        this.file = file;
    }

    private boolean isOfficeFile(File file) {
        for (int i = 0; i < OFFICE_EXTENSIONS.length; i++)
            if (file.getName().endsWith(OFFICE_EXTENSIONS[i]))
                return true;
        return false;
    }

    public Enumeration getParcels() {

        Vector parcels = new Vector();
        ZipFile zp = null;

        try
        {
            zp = new ZipFile(this.file);

            for (Enumeration enumer = zp.entries(); enumer.hasMoreElements(); )
            {
                ZipEntry ze = (ZipEntry)enumer.nextElement();
                if (ze.getName().endsWith(ParcelZipper.PARCEL_DESCRIPTOR_XML))
                {
                    String tmp = ze.getName();
                    int end = tmp.lastIndexOf("/");
                    tmp = tmp.substring(0, end);

                    String parcelName = ze.getName().substring(0, end);
                    parcels.add(parcelName);
                }
            }
        }
        catch(ZipException ze) {
            ze.printStackTrace();
        }
        catch(IOException ioe) {
            ioe.printStackTrace();
        }
        finally {
            if (zp != null) {
                try {
                    zp.close();
                }
                catch (IOException asdf) {
                }
            }
        }

        return parcels.elements();
    }

    public boolean removeParcel(String parcelName) {

        try {
            ParcelZipper.getParcelZipper().removeParcel(file, parcelName);
        }
        catch (IOException ioe) {
            ioe.printStackTrace();
            return false;
        }
        return true;
    }
}
