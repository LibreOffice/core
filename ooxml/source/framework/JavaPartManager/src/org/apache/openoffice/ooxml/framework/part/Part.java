package org.apache.openoffice.ooxml.framework.part;

import java.io.InputStream;

public class Part
    implements IReferenceProvider
{
    public Part (
        final ContentType eType,
        final PartManager aPartManager,
        final PartName aPartName)
    {
        meContentType = eType;
        maPartManager = aPartManager;
        maPartName = aPartName;
        maRelatedParts = null;
    }




    public Part getPartById (final String sId)
    {
        final PartName aName = getRelatedParts().GetTargetForId(sId);
        return new Part(
            maPartManager.getContentTypes().getTypeForPartName(aName),
            maPartManager,
            aName);
    }




    public Part getPartByRelationshipType (final RelationshipType eType)
    {
        final PartName aName = getRelatedParts().GetSingleTargetForType(eType);
        return new Part(
            maPartManager.getContentTypes().getTypeForPartName(aName),
            maPartManager,
            aName);
    }




    public PartName getPartName ()
    {
        return maPartName;
    }




    public ContentType getContentType ()
    {
        return meContentType;
    }




    public InputStream getStream()
    {
        return maPartManager.getStreamForPartName(maPartName);
    }




    @Override
    public RelatedParts getRelatedParts ()
    {
        if (maRelatedParts == null)
        {
            maRelatedParts = new RelatedParts(
                maPartName,
                maPartManager);
        }
        return maRelatedParts;
    }




    private final ContentType meContentType;
    private final PartManager maPartManager;
    private final PartName maPartName;
    private RelatedParts maRelatedParts;
}
