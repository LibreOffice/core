package org.apache.openoffice.ooxml.framework.part;

import java.io.File;
import java.io.InputStream;

public class OOXMLPackage
    extends Package
{
    public static OOXMLPackage Create (final File aOOXMLFile)
    {
        return new OOXMLPackage(
            aOOXMLFile.getAbsolutePath(),
            new PartManager(aOOXMLFile));
    }




    private OOXMLPackage (final String sPath, final PartManager aPartManager)
    {
        super(sPath, aPartManager);
    }




    /** Return a list of stream names.
     *  Note that that list is not necessarily identical to the list of part
     *  names.  It can contain entries that are not parts.
     */
    public String[] listStreamNames ()
    {
        return maPartManager.listStreamNames();
    }




    /** Return an InputStream object for the specified stream.
     */
    public InputStream getStream (final String sStreamName)
    {
        return maPartManager.getStreamForStreamName(sStreamName);
    }




    public Part getPart (final PartName aPartName)
    {
        return new Part (
            maPartManager.getContentTypes().getTypeForPartName(aPartName),
            maPartManager,
            aPartName);
    }
}