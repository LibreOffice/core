/*************************************************************************
 *
 *  $RCSfile: OfficeDocument.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: toconnor $ $Date: 2002-11-13 17:44:06 $
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
import java.util.Enumeration;
import java.util.StringTokenizer;
import java.beans.PropertyVetoException;
import javax.naming.InvalidNameException;

import org.openoffice.idesupport.filter.FileFilter;
import org.openoffice.idesupport.filter.BinaryOnlyFilter;
import org.openoffice.idesupport.zip.ParcelZipper;

public class OfficeDocument
{
    public static final String PARCEL_PREFIX_DIR = "Scripts/java/";
    public static final String OFFICE_EXTENSIONS = "sxc,sxw";
    public static final String ARCHIVE_TAG = "[PARCEL_FILE]";

    private static ParcelZipper zipper = ParcelZipper.getParcelZipper();
    private File officeFile = null;
    private String parcelName = null;
    private String extension = null;

    public OfficeDocument(File officeFile) throws InvalidNameException
    {
        this.officeFile = officeFile;
        if( !checkIfOfficeDocument() )
        {
            throw new InvalidNameException("This is not a valid StarOffice document.");
        }
    }

    private boolean checkIfOfficeDocument()
    {
        if( officeFile.isDirectory() )
        {
            return false;
        }
        String tmpName = officeFile.getName();
        if( tmpName.lastIndexOf(".") == 0 )
        {
            return false;
        }
        this.extension = tmpName.substring(tmpName.lastIndexOf(".")+1);
        if( (OFFICE_EXTENSIONS.indexOf(extension)==-1) )
        {
            return false;
        }
        this.parcelName = tmpName.substring(0,tmpName.lastIndexOf("."));
        return true;
    }

    public Enumeration getParcels()
    {
        java.util.Vector parcelEntries = new java.util.Vector();
        try
        {
            ZipFile zp = new ZipFile(this.officeFile);

            for (Enumeration officeEntries = zp.entries(); officeEntries.hasMoreElements(); )
            {
                ZipEntry ze = (ZipEntry)officeEntries.nextElement();
                if (ze.getName().endsWith(ParcelZipper.PARCEL_DESCRIPTOR_XML))
                {
                    String tmp = ze.getName();
                    int end = tmp.lastIndexOf("/");
                    tmp = tmp.substring(0, end);
                    int start = tmp.lastIndexOf("/") + 1;

                    String parcelName = ARCHIVE_TAG +
                        ze.getName().substring(start, end);
                    parcelEntries.add(parcelName);
                }
            }
        }
        catch(ZipException ze)
        {
            ze.printStackTrace();
        }
        catch(IOException ioe)
        {
            ioe.printStackTrace();
        }
        return parcelEntries.elements();
    }

    public String getParcelNameFromEntry(String parcelName)
    {
        return parcelName.substring(PARCEL_PREFIX_DIR.length(), parcelName.length()-1);
    }

    public String getParcelEntryFromName(String parcelName)
    {
        return parcelName.substring(ARCHIVE_TAG.length()) + "/";
    }

    public boolean removeParcel(String parcelName)
    {
        try {
            ParcelZipper.getParcelZipper().unzipToZipExceptParcel(this.officeFile, getParcelEntryFromName(parcelName));
        }
        catch (IOException ioe) {
            ioe.printStackTrace();
            return false;
        }
        return true;
    }

    public String unzipOneParcel(String parcelName)
    {
        return new String("location");
    }
}
