
package org.apache.openoffice.ooxml.parser;

import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

/** Container of attribute values of an opening tag.
 */
public class AttributeValues
{
    AttributeValues ()
    {
        maAttributes = new TreeMap<>();
    }




    public void AddAttribute (
        final AttributeDescriptor aAttributeDescriptor,
        final String sValue)
    {
        maAttributes.put(
            aAttributeDescriptor.GetName(),
            sValue);
    }




    public Iterable<Entry<String,Object>> GetAttributes ()
    {
        return maAttributes.entrySet();
    }




    public int GetAttributeCount ()
    {
        return maAttributes.size();
    }




    private Map<String,Object> maAttributes;
}
