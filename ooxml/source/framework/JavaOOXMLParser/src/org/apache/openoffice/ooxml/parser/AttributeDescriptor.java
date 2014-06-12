package org.apache.openoffice.ooxml.parser;

/** Store information about a single attribute (per state) that was read
 *  from the parse table.
 *
 *  Note that an attribute that is defined for more than one state has one
 *  AttributeDescriptor object per state.
 *
 */
public class AttributeDescriptor
{
    public AttributeDescriptor (
        final int nPrefixId,
        final int nAttributeId,
        final boolean bCanBeUnqualified,
        final boolean bIsOptional,
        final String sDefaultValue,
        final String sAttributeName,
        final String sAttributeType)
    {
        mnNamespaceId = nPrefixId;
        mnAttributeId = nAttributeId;
        mbCanBeUnqualified = bCanBeUnqualified;
        mbIsOptional = bIsOptional;
        msDefaultValue = sDefaultValue;
        msAttributeName = sAttributeName;
        msAttributeType = sAttributeType;
    }




    public String GetType()
    {
        if (msAttributeType != null)
            return msAttributeType;
        else
            return "<undefined>";
    }




    public int GetNamespaceId ()
    {
        return mnNamespaceId;
    }




    public int GetNameId ()
    {
        return mnAttributeId;
    }




    public boolean CanBeUnqualified ()
    {
        return mbCanBeUnqualified;
    }




    public boolean IsOptional ()
    {
        return mbIsOptional;
    }




    public String GetDefaultValue ()
    {
        return msDefaultValue;
    }




    public String GetName ()
    {
        return msAttributeName;
    }




    @Override
    public String toString ()
    {
        return String.format(
            "attribute %s(%d) of type %s",
            msAttributeName,
            mnAttributeId,
            msAttributeType);
    }




    private final int mnNamespaceId;
    private final int mnAttributeId;
    private final boolean mbCanBeUnqualified;
    private final boolean mbIsOptional;
    private final String msDefaultValue;
    private final String msAttributeName;
    private final String msAttributeType;
}
