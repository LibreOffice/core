package org.apache.openoffice.ooxml.parser.type;

import org.apache.openoffice.ooxml.parser.NameMap;

public class ListParser implements ISimpleTypeParser
{

    public ListParser (final ISimpleTypeParser aItemParser)
    {
        maItemParser = aItemParser;
    }




    @Override
    public Object Parse(
        final String sRawValue,
        final NameMap aAttributeValueMap)
    {
        final String[] aParts = sRawValue.split("\\s+");
        final Object[] aValues = new Object[aParts.length];
        for (int nIndex=0; nIndex<aParts.length; ++nIndex)
            aValues[nIndex] = maItemParser.Parse(aParts[nIndex], aAttributeValueMap);
        return aValues;
    }




    private final ISimpleTypeParser maItemParser;
}
