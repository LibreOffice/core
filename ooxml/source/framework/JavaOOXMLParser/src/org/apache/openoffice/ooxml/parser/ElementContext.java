package org.apache.openoffice.ooxml.parser;

import org.apache.openoffice.ooxml.parser.attribute.AttributeValues;

/** Context that has the same life time (by default) as the element it represents.
 *  Gives access to the attribute values and the parent context.
 */
public class ElementContext
{
    ElementContext (
        final String sElementName,
        final String sTypeName,
        final boolean bIsSkipping,
        final AttributeValues aValues,
        final ElementContext aParentContext)
    {
        msElementName = sElementName;
        msTypeName = sTypeName;
        mbIsSkipping = bIsSkipping;
        maAttributeValues = aValues;
        maParentContext = aParentContext;
    }




    public String GetElementName ()
    {
        return msElementName;
    }




    public String GetTypeName ()
    {
        return msTypeName;
    }




    public AttributeValues GetAttributes ()
    {
        return maAttributeValues;
    }




    /** Return the context of the parent element.
     *  Can be null when there is no parent element.
     */
    public ElementContext GetParentContext ()
    {
        return maParentContext;
    }




    private final String msElementName;
    private final String msTypeName;
    private final boolean mbIsSkipping;
    private final AttributeValues maAttributeValues;
    private final ElementContext maParentContext;
}
