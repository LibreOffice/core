package org.apache.openoffice.ooxml.framework.part;

import java.io.File;
import java.io.InputStream;
import java.lang.ref.SoftReference;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Vector;

public class Package
    implements IReferenceProvider
{
    public static Package Create (final File aOOXMLFile)
    {
        return new Package(
            aOOXMLFile.getAbsolutePath(),
            new PartManager(aOOXMLFile));
    }




    protected Package (
        final String sFullFilename,
        final PartManager aPartManager)
    {
        msFullFilename = sFullFilename;
        maPartManager = aPartManager;
        maRelatedParts = new RelatedParts(
            new PartName(""),
            aPartManager);

        final PartName aDocumentPartName = maRelatedParts.GetSingleTargetForType(RelationshipType.OfficeDocument);
        maOfficeDocumentPart = new Part(
            maPartManager.getContentTypes().getTypeForPartName(aDocumentPartName),
            maPartManager,
            aDocumentPartName);
    }




    public Part getOfficeDocumentPart ()
    {
        return maOfficeDocumentPart;
    }




    public Iterable<Part> getDigitalSignaturesParts ()
    {
        // TODO
        return new Vector<>();
    }




    public RelatedParts getRelatedParts ()
    {
        return maRelatedParts;
    }




    public boolean hasAppDefFilePropertiesPart ()
    {
        // TODO
        return false;
    }




    public Part getAppDefFilePropertiesPart ()
    {
        // TODO
        return null;
    }




    public boolean hasCoreFilePropertiesPart ()
    {
        // TODO
        return false;
    }




    public Part getCoreFilePropertiesPart ()
    {
        // TODO
        return null;
    }




    public boolean hasCustomFilePropertiesPart ()
    {
        // TODO
        return false;
    }




    public Part getCustomFilePropertiesPart ()
    {
        // TODO
        return null;
    }




    public String getFileName()
    {
        return msFullFilename;
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




    private final String msFullFilename;
    protected final PartManager maPartManager;
    private final RelatedParts maRelatedParts;
    private final Part maOfficeDocumentPart;
}