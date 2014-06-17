package org.apache.openoffice.ooxml.parser.type;

import org.apache.openoffice.ooxml.parser.NameMap;

public class BlobParser implements ISimpleTypeParser
{
    public BlobParser(final String[] aLine)
    {
        switch(aLine[5])
        {
            case "B":
                meType = Type.Base64Binary;
                break;
            case "H":
                meType = Type.HexBinary;
                break;
            default:
                throw new RuntimeException();
        }
        switch(aLine[6])
        {
            case "L":
                mnLengthRestriction = Integer.parseInt(aLine[7]);
                break;
            case "N":
                mnLengthRestriction = null;
                break;
            default:
                throw new RuntimeException();
        }
    }




    @Override
    public Object Parse (
        final String sRawValue,
        final NameMap aAttributeValueMap)
    {
        if (mnLengthRestriction != null)
            if (sRawValue.length()/2 != mnLengthRestriction)
                return null;
        /*
                throw new RuntimeException(
                    String.format(
                        "length restriction (=%d) is violated, actual length is %d",
                        mnLengthRestriction,
                        sRawValue.length()));
          */
        switch(meType)
        {
            case Base64Binary:
                throw new RuntimeException("not yet implemented");

            case HexBinary:
                try
                {
                    return Integer.parseInt(sRawValue, 16);
                }
                catch (NumberFormatException aException)
                {
                    return null;
                }

            default:
                throw new RuntimeException();
        }
    }




    enum Type
    {
        Base64Binary,
        HexBinary
    };
    private final Type meType;
    private final Integer mnLengthRestriction;
}
