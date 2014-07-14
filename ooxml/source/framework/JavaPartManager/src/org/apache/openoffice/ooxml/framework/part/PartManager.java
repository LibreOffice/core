package org.apache.openoffice.ooxml.framework.part;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.SoftReference;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class PartManager
{
    public PartManager (final File aFile)
    {
        ZipFile aZipFile = null;
        try
        {
            aZipFile = new ZipFile(aFile);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        maZipFile = aZipFile;
        maPartNameToPartMap = new HashMap<>();
    }




    public InputStream getStreamForPartName (final PartName aPartName)
    {
        final ZipEntry aEntry = maZipFile.getEntry(
            ToZipEntryName(aPartName.GetFullname()));
        if (aEntry == null)
            return null;

        try
        {
            return maZipFile.getInputStream(aEntry);
        }
        catch (IOException e)
        {
            e.printStackTrace();
            return null;
        }
    }




    /** This is the low-level variant of getStreamForPartName().
     *  It can return streams for entries in the OOXML zip package
     *  that are not, technically, parts.
     *  @return
     *      Returns null when the named stream does not exist or can not be
     *      opened.
     */
    public InputStream getStreamForStreamName (final String sStreamName)
    {
        final ZipEntry aEntry = maZipFile.getEntry(
            ToZipEntryName(sStreamName));
        try
        {
            return maZipFile.getInputStream(aEntry);
        }
        catch (IOException e)
        {
            e.printStackTrace();
            return null;
        }
    }




    public ContentTypes getContentTypes ()
    {
        if (maContentTypes == null)
        {
            maContentTypes = new ContentTypes(this);
        }
        return maContentTypes;
    }




    private final String ToZipEntryName (final String sPath)
    {
        return sPath.substring(1);
    }




    /** Return a list of the names of all streams.
     *  Note that that list is not necessarily identical to the list of part
     *  names.  It can contain entries that are not parts.
     */
    public final String[] listStreamNames ()
    {
        final Vector<String> aStreamNames =  new Vector<>();

        final Enumeration<? extends ZipEntry> aEntries = maZipFile.entries();
        while (aEntries.hasMoreElements())
        {
            final ZipEntry aEntry = aEntries.nextElement();
            aStreamNames.add(aEntry.getName());
        }

        return aStreamNames.toArray(new String[0]);
    }




    public Part getPart (final PartName aName)
    {
        SoftReference<Part> aSoftPart = maPartNameToPartMap.get(aName);
        Part aPart = null;
        if (aSoftPart != null)
            aPart = aSoftPart.get();
        if (aPart == null)
        {
            aPart = new Part(
                getContentTypes().getTypeForPartName(aName),
                this,
                aName);
            maPartNameToPartMap.put(aName, new SoftReference<Part>(aPart));
        }

        return aPart;
    }




    private ZipFile maZipFile;
    private ContentTypes maContentTypes;
    private final Map<PartName, SoftReference<Part>> maPartNameToPartMap;
}
