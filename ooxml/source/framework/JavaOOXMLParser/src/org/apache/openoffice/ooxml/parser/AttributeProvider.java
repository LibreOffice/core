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

package org.apache.openoffice.ooxml.parser;

import java.util.Iterator;
import java.util.Map.Entry;

import javax.xml.stream.XMLStreamReader;

public class AttributeProvider
    implements Iterable<Entry<String,String>>
{
    public AttributeProvider(XMLStreamReader aReader)
    {
        maReader = aReader;
    }



    public boolean HasAttributes ()
    {
        return maReader.getAttributeCount() > 0;
    }




    public String GetValue (final String sKey)
    {
        return maReader.getAttributeValue(null,  sKey);
    }



    @Override
    public Iterator<Entry<String,String>> iterator ()
    {
        return new Iterator<Entry<String,String>> ()
        {
            int nIndex = 0;
            final int nCount = maReader.getAttributeCount();

            @Override public boolean hasNext()
            {
                return nIndex < nCount;
            }

            @Override public Entry<String, String> next()
            {
                final Entry<String,String> aEntry = new Entry<String,String>()
                {
                    final String msKey = maReader.getAttributeLocalName(nIndex);
                    final String msValue = maReader.getAttributeValue(nIndex);

                    @Override public String getKey()
                    {
                        return msKey;
                    }

                    @Override public String getValue()
                    {
                        return msValue;
                    }

                    @Override public String setValue (final String sValue)
                    {
                        return null;
                    }
                };
                ++nIndex;
                return aEntry;
            }

            @Override public void remove()
            {
            }

        };
    }

    private final XMLStreamReader maReader;
}
