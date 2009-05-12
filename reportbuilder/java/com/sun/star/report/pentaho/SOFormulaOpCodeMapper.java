/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SOFormulaOpCodeMapper.java,v $
 *
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.report.pentaho;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.XComponentContext;
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

    private final XComponentContext m_xContext;
    private static final String __serviceName = "com.sun.star.report.pentaho.SOFormulaOpCodeMapper";
    private final SOFormulaParser parser;
    // attributes
    final private int m_OpCodeExternal = 0;
    final private int m_OpCodeUnknown = 0;

    public SOFormulaOpCodeMapper(XComponentContext context, SOFormulaParser parser)
    {
        m_xContext = context;
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
        if ( Language != FormulaLanguage.ODFF )
            throw new IllegalArgumentException();
        final ArrayList token = new ArrayList();
        final Map parserNames = parser.getNames();
        for (int i = 0; i < Names.length; i++)
        {
            if ( parserNames.containsKey(Names[i]) )
            {
                token.add(((FormulaOpCodeMapEntry) parserNames.get(Names[i])).Token);
            }

        }
        return (com.sun.star.sheet.FormulaToken[]) token.toArray(new FormulaToken[token.size()]);
    }

    public com.sun.star.sheet.FormulaOpCodeMapEntry[] getAvailableMappings(int Language, int Groups) throws com.sun.star.lang.IllegalArgumentException
    {
        if ( Language != FormulaLanguage.ODFF )
            throw new IllegalArgumentException();
        final ArrayList token = new ArrayList();
        if ( Groups == FormulaMapGroup.SPECIAL )
        {
            return (com.sun.star.sheet.FormulaOpCodeMapEntry[]) parser.getSpecialOpCodes().toArray(new FormulaOpCodeMapEntry[parser.getSpecialOpCodes().size()]);
        }
        else
        {
            if ( (Groups & FormulaMapGroup.ARRAY_SEPARATORS) != 0 )
            {
                token.addAll(parser.getGroup(SOFormulaParser.ARRAY_SEPARATORS).values());
            }
            if ( (Groups & FormulaMapGroup.SEPARATORS) != 0 )
            {
                token.addAll(parser.getGroup(SOFormulaParser.SEPARATORS).values());
            }
            if ( (Groups & FormulaMapGroup.ARRAY_SEPARATORS) != 0 )
            {
                token.addAll(parser.getGroup(SOFormulaParser.ARRAY_SEPARATORS).values());
            }
            if ( (Groups & FormulaMapGroup.UNARY_OPERATORS) != 0 )
            {
                token.addAll(parser.getGroup(SOFormulaParser.UNARY_OPERATORS).values());
            }
            if ( (Groups & FormulaMapGroup.BINARY_OPERATORS) != 0 )
            {
                token.addAll(parser.getGroup(SOFormulaParser.BINARY_OPERATORS).values());
            }
            if ( (Groups & FormulaMapGroup.FUNCTIONS) != 0 )
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
     */
    public static String[] getServiceNames()
    {
        return new String[]
                {
                    __serviceName
                };
    }
}
