package org.apache.openoffice.ooxml.schema.simple;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltInType;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;

public class StringNode
    implements ISimpleTypeNode
{
    public StringNode (
        final BuiltInType eType)
    {
        meType = eType;
        mbIsList = false;
    }




    @Override
    public void ApplyRestriction (
        final Restriction aRestriction,
        final Map<String,Integer> aValueToIdMap)
    {
        assert( ! aRestriction.HasFeature(
            Restriction.MinExclusiveBit
            | Restriction.MinInclusiveBit
            | Restriction.MaxExclusiveBit
            | Restriction.MaxInclusiveBit));

        if (aRestriction.HasFeature(Restriction.EnumerationBit))
        {
            if (aRestriction.HasFeature(Restriction.LengthBit|Restriction.MinLengthBit|Restriction.MaxLengthBit))
                for (final String sValue : aRestriction.GetEnumeration())
                    assert(CheckLengthRestriction(sValue, aRestriction));
            maEnumerationValues = new TreeSet<>();
            maEnumerationValueIds = new HashSet<>();
            for (final String sValue : aRestriction.GetEnumeration())
            {
                if ( ! aValueToIdMap.containsKey(sValue))
                    aValueToIdMap.put(sValue, aValueToIdMap.size());
                maEnumerationValues.add(sValue);
                maEnumerationValueIds.add(aValueToIdMap.get(sValue));
            }
        }
        else if (aRestriction.HasFeature(Restriction.PatternBit))
        {
            msPattern = aRestriction.GetPattern();
            // Make the regular expression understandable by Java (by replacing
            // character class names like IsBasicLatin to InBasicLatin).
            try
            {
                maPattern = Pattern.compile(msPattern.replace("\\p{Is", "\\p{In"), Pattern.UNICODE_CHARACTER_CLASS);
            }
            catch (PatternSyntaxException aException)
            {
                aException.printStackTrace();
            }
        }
        else if (aRestriction.HasFeature(Restriction.LengthBit|Restriction.MinLengthBit|Restriction.MaxLengthBit))
        {
            if (aRestriction.HasFeature(Restriction.LengthBit))
                mnMinimumLength = mnMaximumLength = aRestriction.GetLength();
            if (aRestriction.HasFeature(Restriction.MinLengthBit))
                mnMinimumLength = aRestriction.GetMinimumLength();
            if (aRestriction.HasFeature(Restriction.MaxLengthBit))
                mnMaximumLength = aRestriction.GetMaximumLength();
        }
        else
        {
            // no restriction.
            assert(aRestriction.GetFeatureBits() == 0);
        }
    }




    @Override
    public void Print (final Log aLog)
    {
        aLog.println(toString());
    }




    @Override
    public String toString ()
    {
        final StringBuffer aBuffer = new StringBuffer();
        aBuffer.append(String.format("string (%s)", meType));
        if (maEnumerationValueIds != null)
        {
            aBuffer.append(" [");
            boolean bIsFirst = true;
            for (final String sValue : maEnumerationValues)
            {
                if (bIsFirst)
                    bIsFirst = false;
                else
                    aBuffer.append(", ");
                aBuffer.append(sValue);
            }
            aBuffer.append("]");
        }
        else if (maPattern != null)
        {
            aBuffer.append("pattern=\"");
            aBuffer.append(maPattern);
            aBuffer.append("\"");
        }
        return aBuffer.toString();
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




    /** Try to join the called and the given string types.
     *  If that is possible then return the resulting type.
     *  Otherwise return null.
     */
    public ISimpleTypeNode Join (final StringNode aType)
    {
        if (maEnumerationValues!=null && aType.maEnumerationValues!=null)
        {
            // Join the enumeration values.
            maEnumerationValues.addAll(aType.maEnumerationValues);
            maEnumerationValueIds.addAll(aType.maEnumerationValueIds);
            return this;
        }
        else
            return null;
    }


    public enum RestrictionType
    {
        Pattern,
        Enumeration,
        Length,
        None
    }
    public RestrictionType GetRestrictionType ()
    {
        if (maEnumerationValueIds != null)
            return RestrictionType.Enumeration;
        else if (maPattern != null)
            return RestrictionType.Pattern;
        else if (mnMinimumLength != null)
            return RestrictionType.Length;
        else
            return RestrictionType.None;
    }




    public Set<Integer> GetEnumerationRestriction ()
    {
        final Set<Integer> aSortedIds = new TreeSet<>();
        aSortedIds.addAll(maEnumerationValueIds);
        return aSortedIds;
    }




    public String GetPatternRestriction ()
    {
        return msPattern;
    }




    public int[] GetLengthRestriction ()
    {
        return new int[]{mnMinimumLength, mnMaximumLength};
    }




    private boolean CheckLengthRestriction (
        final String sValue,
        final Restriction aRestriction)
    {
        final int nValueLength = sValue.length();
        if (aRestriction.HasFeature(Restriction.LengthBit))
            return nValueLength == aRestriction.GetLength();
        else if (aRestriction.HasFeature(Restriction.MinLengthBit | Restriction.MaxLengthBit))
            return nValueLength>=aRestriction.GetMinimumLength()
                && nValueLength<=aRestriction.GetMaximumLength();
        else if (aRestriction.HasFeature(Restriction.MinLengthBit))
            return nValueLength>=aRestriction.GetMinimumLength();
        else if (aRestriction.HasFeature(Restriction.MaxLengthBit))
            return nValueLength<=aRestriction.GetMaximumLength();
        else
            throw new RuntimeException();
    }




    @Override
    public void AcceptVisitor (final ISimpleTypeNodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    private final BuiltInType meType;
    private Set<String> maEnumerationValues;
    private Set<Integer> maEnumerationValueIds;
    private Pattern maPattern;
    private String msPattern;
    private Integer mnMinimumLength;
    private Integer mnMaximumLength;
    private boolean mbIsList;
}
