package org.apache.openoffice.ooxml.schema.simple;

import java.util.Map;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltInType;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;

public class BlobNode
    implements ISimpleTypeNode
{
    public BlobNode (final BuiltInType eType)
    {
        meType = eType;
        mbIsList = false;
        mnLengthRestriction = null;
    }




    public BuiltInType GetBlobType ()
    {
        return meType;
    }




    @Override
    public void ApplyRestriction (
        final Restriction aRestriction,
        final Map<String,Integer> aValueToIdMap)
    {
        if (aRestriction.GetFeatureBits() == 0)
            return;

        if (aRestriction.GetFeatureBits() != Restriction.LengthBit)
            throw new RuntimeException("unsupported restriction on blob: "+aRestriction);

        mnLengthRestriction = aRestriction.GetLength();
    }




    @Override
    public void Print (final Log aLog)
    {
        aLog.printf("blob of type %s\n", meType);
    }




    @Override
    public boolean IsList ()
    {
        return mbIsList;
    }




    @Override
    public void SetIsList ()
    {
        mbIsList = true;
    }




    @Override
    public void AcceptVisitor (final ISimpleTypeNodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    public enum RestrictionType
    {
        Length,
        None;
    }
    public RestrictionType GetRestrictionType ()
    {
        if (mnLengthRestriction != null)
            return RestrictionType.Length;
        else
            return RestrictionType.None;
    }




    public int GetLengthRestriction ()
    {
        return mnLengthRestriction;
    }




    private final BuiltInType meType;
    private boolean mbIsList;
    private Integer mnLengthRestriction;
}
