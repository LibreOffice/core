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

package org.apache.openoffice.ooxml.parser.attribute;

import java.util.Iterator;

import javax.xml.stream.XMLStreamReader;

/** Give access to the attributes that are read from an OOXML stream.
 */
public class AttributeProvider
    implements Iterable<String[]>
{
    public AttributeProvider (final XMLStreamReader aReader)
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
    public Iterator<String[]> iterator ()
    {
        return new Iterator<String[]> ()
        {
            int nIndex = 0;
            final int nCount = maReader.getAttributeCount();

            @Override public boolean hasNext()
            {
                return nIndex < nCount;
            }

            @Override public String[] next()
            {
                final String[] aResult = new String[]
                    {
                        maReader.getAttributeNamespace(nIndex),
                        maReader.getAttributeLocalName(nIndex),
                        maReader.getAttributeValue(nIndex)
                    };
                ++nIndex;
                return aResult;
            }

            @Override public void remove()
            {
            }

        };
    }




    public Integer GetAttributeCount ()
    {
        return maReader.getAttributeCount();
    }




    private final XMLStreamReader maReader;
}
