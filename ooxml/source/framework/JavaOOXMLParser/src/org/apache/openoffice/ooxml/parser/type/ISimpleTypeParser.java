package org.apache.openoffice.ooxml.parser.type;

import org.apache.openoffice.ooxml.parser.NameMap;

public interface ISimpleTypeParser
{
    Object Parse (
        final String sRawValue,
        final NameMap aAttributeValueMap);
}
