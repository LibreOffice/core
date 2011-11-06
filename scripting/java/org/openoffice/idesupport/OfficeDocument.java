/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
