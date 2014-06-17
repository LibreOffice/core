package org.apache.openoffice.ooxml.schema.simple;

import java.util.Map;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltInType;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;

public class DateTimeNode
    implements ISimpleTypeNode
{

    public DateTimeNode (final BuiltInType eType)
    {
        mbIsList = false;
    }




    @Override
    public void ApplyRestriction (
        final Restriction aRestriction,
        final Map<String,Integer> aValueToIdMap)
    {
        if (aRestriction.GetFeatureBits() == 0)
            return;
        System.out.println(aRestriction);
    }




    @Override
    public void Print (final Log aLog)
    {
        aLog.printf("date or time\n");
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




    private boolean mbIsList;
}
