package org.apache.openoffice.ooxml.schema.simple;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

public class SimpleTypeDescriptor
{
    public SimpleTypeDescriptor (
        final QualifiedName aName)
    {
        maName = aName;
        maSubTypes = null;
    }




    public QualifiedName GetName()
    {
        return maName;
    }




    public void SetSubTypes (final ISimpleTypeNode[] aSubTypes)
    {
        maSubTypes = aSubTypes;
    }




    public ISimpleTypeNode[] GetSubType ()
    {
        return maSubTypes;
    }




    @Override
    public String toString ()
    {
        return "simple type "+maName;
    }




    public void Print (final Log aLog)
    {
        aLog.printf("%s\n", toString());
        aLog.StartBlock();
        for (final ISimpleTypeNode aSubType : maSubTypes)
            aSubType.Print(aLog);
        aLog.EndBlock();
    }




    private final QualifiedName maName;
    private ISimpleTypeNode[] maSubTypes;
}
