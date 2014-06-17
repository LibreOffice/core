
package org.apache.openoffice.ooxml.parser.attribute;

import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

/** Container of attribute values of an opening tag.
 */
public class AttributeValues
{
    AttributeValues ()
    {
        maRawAttributeValues = new TreeMap<>();
        maProcessedAttributeValues = new TreeMap<>();
    }




    public void AddAttribute (
        final AttributeDescriptor aAttributeDescriptor,
        final String sRawValue,
        final Object aProcessedValue)
    {
        maRawAttributeValues.put(
            aAttributeDescriptor.GetName(),
            sRawValue);
        maProcessedAttributeValues.put(
            aAttributeDescriptor.GetName(),
            aProcessedValue);
    }




    public Iterable<Entry<String,String>> GetAttributes ()
    {
        return maRawAttributeValues.entrySet();
    }




    public String GetRawAttributeValue (final String sName)
    {
        return maRawAttributeValues.get(sName);
    }




    public Object GetProcessedAttributeValue (final String sName)
    {
        return maProcessedAttributeValues.get(sName);
    }




    public int GetAttributeCount ()
    {
        return maRawAttributeValues.size();
    }




    private Map<String,String> maRawAttributeValues;
    private Map<String,Object> maProcessedAttributeValues;
}
