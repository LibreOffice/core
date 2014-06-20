/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/

package org.apache.openoffice.ooxml.parser.type;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import org.apache.openoffice.ooxml.parser.NameMap;
import org.apache.openoffice.ooxml.parser.attribute.AttributeDescriptor;

public class SimpleTypeManager
{
    public SimpleTypeManager(
        final Vector<String[]> aData,
        final NameMap aAttributeValueMap)
    {
        maAttributeValueMap = aAttributeValueMap;
        maSimpleTypeToParsersMap = new HashMap<>();
        ParseData(aData);
    }




    private void ParseData (final Vector<String[]> aData)
    {
        for (final String[] aLine : aData)
        {
            final int nSimpleTypeId = Integer.parseInt(aLine[1]);
//            final int nVariant = Integer.parseInt(aLine[2]);
            final boolean bIsList = aLine[3].equals("L");
            final ISimpleTypeParser aVariantParser;
            switch (aLine[4])
            {
                case "S":
                    aVariantParser = new StringParser(aLine);
                    break;
                case "N":
                    aVariantParser = new NumberParser(aLine);
                    break;
                case "D":
                    aVariantParser = new DateTimeParser(aLine);
                    break;
                case "B":
                    aVariantParser = new BlobParser(aLine);
                    break;
                default:
                    throw new RuntimeException("unexpected parser type: "+aLine[4]);
            }

            Vector<ISimpleTypeParser> aVariants = maSimpleTypeToParsersMap.get(nSimpleTypeId);
            if (aVariants == null)
            {
                aVariants = new Vector<>();
                maSimpleTypeToParsersMap.put(nSimpleTypeId, aVariants);
            }
            if (bIsList)
                aVariants.add(new ListParser(aVariantParser));
            else
                aVariants.add(aVariantParser);
        }
    }




    public Object PreprocessValue (
        final String sRawValue,
        final AttributeDescriptor aAttributeDescriptor)
    {
        final Vector<ISimpleTypeParser> aTypeParsers = maSimpleTypeToParsersMap.get(aAttributeDescriptor.GetTypeId());
        if (aTypeParsers == null)
            throw new RuntimeException("type "+aAttributeDescriptor.GetTypeId()+" is not supported");

        for (final ISimpleTypeParser aParser : aTypeParsers)
        {
            try
            {
                final Object aProcessedValue = aParser.Parse(
                    sRawValue,
                    maAttributeValueMap);
                if (aProcessedValue != null)
                    return aProcessedValue;
            }
            catch(final Exception aException)
            {
                return "error";
            }
        }
        return null;
    }




    private final NameMap maAttributeValueMap;
    private Map<Integer,Vector<ISimpleTypeParser>> maSimpleTypeToParsersMap;
}
