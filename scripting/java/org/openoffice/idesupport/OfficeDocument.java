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
