/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report.pentaho;

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

    private static final String __serviceName = "org.libreoffice.report.pentaho.SOFormulaOpCodeMapper";
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
        final ArrayList<FormulaToken> token = new ArrayList<FormulaToken>();
        final Map parserNames = parser.getNames();
        for (int i = 0; i < Names.length; i++)
        {
            if (parserNames.containsKey(Names[i]))
            {
                token.add(((FormulaOpCodeMapEntry) parserNames.get(Names[i])).Token);
            }

        }
        return token.toArray(new FormulaToken[token.size()]);
    }

    public com.sun.star.sheet.FormulaOpCodeMapEntry[] getAvailableMappings(int Language, int Groups) throws com.sun.star.lang.IllegalArgumentException
    {
        if (Language != FormulaLanguage.ODFF)
        {
            throw new IllegalArgumentException();
        }
        final ArrayList<FormulaOpCodeMapEntry> token = new ArrayList<FormulaOpCodeMapEntry>();
        if (Groups == FormulaMapGroup.SPECIAL)
        {
            return parser.getSpecialOpCodes().toArray(new FormulaOpCodeMapEntry[parser.getSpecialOpCodes().size()]);
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

        return token.toArray(new FormulaOpCodeMapEntry[token.size()]);
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
