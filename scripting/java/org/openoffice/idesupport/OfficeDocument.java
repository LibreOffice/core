/*************************************************************************
 *
 *  $RCSfile: OfficeDocument.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-03-12 18:26:28 $
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

            for (Enumeration enum = zp.entries(); enum.hasMoreElements(); )
            {
                ZipEntry ze = (ZipEntry)enum.nextElement();
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
