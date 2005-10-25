/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeDocument.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-25 11:19:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
