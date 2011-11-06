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


package com.sun.star.report.pentaho;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.sheet.FormulaLanguage;
import com.sun.star.sheet.FormulaMapGroup;
import com.sun.star.sheet.FormulaOpCodeMapEntry;
import com.sun.star.sheet.FormulaToken;

import java.util.ArrayList;
import java.util.Map;

public final class SOFormulaOpCodeMapper extends WeakBase
        implements com.sun.star.sheet.XFormulaOpCodeMapper, XServiceInfo
{

    private static final String __serviceName = "com.sun.star.report.pentaho.SOFormulaOpCodeMapper";
    private final SOFormulaParser parser;
    // attributes
    final private int m_OpCodeExternal = 0;
    final private int m_OpCodeUnknown = 0;

    public SOFormulaOpCodeMapper(SOFormulaParser parser)
    {
        this.parser = parser;
    }

    // com.sun.star.sheet.XFormulaOpCodeMapper:
    public int getOpCodeExternal()
    {
        return m_OpCodeExternal;
    }

    public int getOpCodeUnknown()
    {
        return m_OpCodeUnknown;
    }

    public com.sun.star.sheet.FormulaToken[] getMappings(String[] Names, int Language) throws com.sun.star.lang.IllegalArgumentException
    {
        if (Language != FormulaLanguage.ODFF)
        {
            throw new IllegalArgumentException();
        }
        final ArrayList token = new ArrayList();
        final Map parserNames = parser.getNames();
        for (int i = 0; i < Names.length; i++)
        {
            if (parserNames.containsKey(Names[i]))
            {
                token.add(((FormulaOpCodeMapEntry) parserNames.get(Names[i])).Token);
            }

        }
        return (com.sun.star.sheet.FormulaToken[]) token.toArray(new FormulaToken[token.size()]);
    }

    public com.sun.star.sheet.FormulaOpCodeMapEntry[] getAvailableMappings(int Language, int Groups) throws com.sun.star.lang.IllegalArgumentException
    {
        if (Language != FormulaLanguage.ODFF)
        {
            throw new IllegalArgumentException();
        }
        final ArrayList token = new ArrayList();
        if (Groups == FormulaMapGroup.SPECIAL)
        {
            return (com.sun.star.sheet.FormulaOpCodeMapEntry[]) parser.getSpecialOpCodes().toArray(new FormulaOpCodeMapEntry[parser.getSpecialOpCodes().size()]);
        }
        else
        {
            if ((Groups & FormulaMapGroup.ARRAY_SEPARATORS) != 0)
            {
                token.addAll(parser.getGroup(SOFormulaParser.ARRAY_SEPARATORS).values());
            }
            if ((Groups & FormulaMapGroup.SEPARATORS) != 0)
            {
                token.addAll(parser.getGroup(SOFormulaParser.SEPARATORS).values());
            }
            if ((Groups & FormulaMapGroup.ARRAY_SEPARATORS) != 0)
            {
                token.addAll(parser.getGroup(SOFormulaParser.ARRAY_SEPARATORS).values());
            }
            if ((Groups & FormulaMapGroup.UNARY_OPERATORS) != 0)
            {
                token.addAll(parser.getGroup(SOFormulaParser.UNARY_OPERATORS).values());
            }
            if ((Groups & FormulaMapGroup.BINARY_OPERATORS) != 0)
            {
                token.addAll(parser.getGroup(SOFormulaParser.BINARY_OPERATORS).values());
            }
            if ((Groups & FormulaMapGroup.FUNCTIONS) != 0)
            {
                token.addAll(parser.getGroup(SOFormulaParser.FUNCTIONS).values());
            }
        }

        return (com.sun.star.sheet.FormulaOpCodeMapEntry[]) token.toArray(new FormulaOpCodeMapEntry[token.size()]);
    }

    public String getImplementationName()
    {
        return SOFormulaOpCodeMapper.class.getName();
    }

    public boolean supportsService(String sServiceName)
    {
        return sServiceName.equals(__serviceName);
    }

    public String[] getSupportedServiceNames()
    {
        return getServiceNames();
    }

    /**
     * This method is a simple helper function to used in the static component initialisation functions as well as
     * in getSupportedServiceNames.
     * @return
     */
    public static String[] getServiceNames()
    {
        return new String[]
                {
                    __serviceName
                };
    }
}
