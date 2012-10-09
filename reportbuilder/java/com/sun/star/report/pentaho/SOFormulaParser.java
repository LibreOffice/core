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
package com.sun.star.report.pentaho;


import com.sun.star.lang.XServiceInfo;
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.lib.uno.helper.PropertySetMixin;
import com.sun.star.sheet.FormulaLanguage;
import com.sun.star.sheet.FormulaMapGroup;
import com.sun.star.sheet.FormulaMapGroupSpecialOffset;
import com.sun.star.sheet.FormulaOpCodeMapEntry;
import com.sun.star.sheet.FormulaToken;
import com.sun.star.sheet.XFormulaOpCodeMapper;
import com.sun.star.uno.Any;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import java.io.StringReader;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.pentaho.reporting.libraries.base.config.Configuration;
import org.pentaho.reporting.libraries.formula.DefaultFormulaContext;
import org.pentaho.reporting.libraries.formula.function.FunctionRegistry;
import org.pentaho.reporting.libraries.formula.parser.FormulaParser;
import org.pentaho.reporting.libraries.formula.parser.GeneratedFormulaParserConstants;
import org.pentaho.reporting.libraries.formula.parser.GeneratedFormulaParserTokenManager;
import org.pentaho.reporting.libraries.formula.parser.JavaCharStream;
import org.pentaho.reporting.libraries.formula.parser.ParseException;
import org.pentaho.reporting.libraries.formula.parser.Token;
import org.pentaho.reporting.libraries.formula.parser.TokenMgrError;


public final class SOFormulaParser extends ComponentBase
        implements com.sun.star.report.meta.XFormulaParser, XServiceInfo
{
    /* Need this to get around generics array creation restriction */
    private static class StringOpcodeMap extends HashMap<Integer,FormulaOpCodeMapEntry> {}

    public static final int SEPARATORS = 0;
    public static final int ARRAY_SEPARATORS = 1;
    public static final int UNARY_OPERATORS = 2;
    public static final int BINARY_OPERATORS = 3;
    public static final int FUNCTIONS = 4;
    private final XComponentContext m_xContext;
    private final PropertySetMixin m_prophlp;
    private static final String __serviceName = "com.sun.star.report.meta.FormulaParser";
    private static final String OPERATORS = "org.pentaho.reporting.libraries.formula.operators.";
    // attributes
    final private List<FormulaOpCodeMapEntry> m_OpCodeMap = new ArrayList<FormulaOpCodeMapEntry>();
    private XFormulaOpCodeMapper formulaOpCodeMapper = null;
    private final Map<Integer,FormulaOpCodeMapEntry> parserAllOpCodes = new HashMap<Integer,FormulaOpCodeMapEntry>();
    private final Map<String,FormulaOpCodeMapEntry> parserNames = new HashMap<String,FormulaOpCodeMapEntry>();
    private final StringOpcodeMap[] groupOpCodes = new StringOpcodeMap[5];
    private final List<FormulaOpCodeMapEntry> specialOpCodes = new ArrayList<FormulaOpCodeMapEntry>();
    private int ownTokenCounter = 1000;
    private final FormulaOpCodeMapEntry opCodePush;
    private final FormulaParser parser;

    public SOFormulaParser(final XComponentContext context)
    {

        m_xContext = context;
        final ClassLoader cl = java.lang.Thread.currentThread().getContextClassLoader();
        Thread.currentThread().setContextClassLoader(this.getClass().getClassLoader());

        parser = new FormulaParser();
        try
        {
            final XFormulaOpCodeMapper mapper = (XFormulaOpCodeMapper) UnoRuntime.queryInterface(XFormulaOpCodeMapper.class, m_xContext.getServiceManager().createInstanceWithContext("simple.formula.FormulaOpCodeMapperObj", m_xContext));
            FormulaOpCodeMapEntry[] opCodes = mapper.getAvailableMappings(FormulaLanguage.ODFF, FormulaMapGroup.FUNCTIONS);
            final DefaultFormulaContext defaultContext = new DefaultFormulaContext();
            final FunctionRegistry functionRegistry = defaultContext.getFunctionRegistry();

            String[] names = functionRegistry.getFunctionNames();
            addOpCodes(names, opCodes, FUNCTIONS);
            names = getOperators(defaultContext, OPERATORS);
            opCodes = mapper.getAvailableMappings(FormulaLanguage.ODFF, FormulaMapGroup.UNARY_OPERATORS);
            addOpCodes(names, opCodes, UNARY_OPERATORS);
            opCodes = mapper.getAvailableMappings(FormulaLanguage.ODFF, FormulaMapGroup.BINARY_OPERATORS);
            addOpCodes(names, opCodes, BINARY_OPERATORS);

            names = GeneratedFormulaParserConstants.tokenImage.clone();
            for (int i = 0; i < names.length; i++)
            {
                final String token = names[i];
                if (token != null && token.length() > 0 && token.charAt(0) == '"')
                {
                    names[i] = token.substring(1, token.length() - 1);
                }
            }
            opCodes = mapper.getAvailableMappings(FormulaLanguage.ODFF, FormulaMapGroup.SEPARATORS);
            addOpCodes(names, opCodes, SEPARATORS, false);

            opCodes = mapper.getAvailableMappings(FormulaLanguage.ODFF, FormulaMapGroup.ARRAY_SEPARATORS);
            addOpCodes(names, opCodes, ARRAY_SEPARATORS, false);

            opCodes = mapper.getAvailableMappings(FormulaLanguage.ODFF, FormulaMapGroup.SPECIAL);

            for (int i = 0; i < opCodes.length; i++)
            {
                final FormulaOpCodeMapEntry opCode = opCodes[i];
                parserAllOpCodes.put(opCode.Token.OpCode, opCode);
                specialOpCodes.add(opCode);
            }
            // addOpCodes(names, opCodes,SPECIAL,false);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
        opCodePush = specialOpCodes.get(FormulaMapGroupSpecialOffset.PUSH);
        Thread.currentThread().setContextClassLoader(cl);
        // use the last parameter of the PropertySetMixin constructor
        // for your optional attributes if necessary. See the documentation
        // of the PropertySetMixin helper for further information.
        // Ensure that your attributes are initialized correctly!
        m_prophlp = new PropertySetMixin(m_xContext, this,
                new Type(com.sun.star.report.meta.XFormulaParser.class), null);
    }

    // com.sun.star.sheet.XFormulaParser:
    public com.sun.star.sheet.FormulaToken[] parseFormula(String aFormula, com.sun.star.table.CellAddress aReferencePos)
    {
        final ArrayList<FormulaToken> tokens = new ArrayList<FormulaToken>();
        if (!"=".equals(aFormula))
        {
            String formula;
            if (aFormula.charAt(0) == '=')
            {
                formula = aFormula.substring(1);
            }
            else
            {
                formula = aFormula;
            }
            final ArrayList<String> images = new ArrayList<String>();
            try
            {
                int brackets = 0;
                final GeneratedFormulaParserTokenManager tokenParser = new GeneratedFormulaParserTokenManager(new JavaCharStream(new StringReader(formula), 1, 1));
                Token token = tokenParser.getNextToken();
                while (token.kind != GeneratedFormulaParserConstants.EOF)
                {
                    final FormulaToken formulaToken;
                    images.add(token.image);
                    final String upper = token.image.toUpperCase();
                    if (parserNames.containsKey(upper))
                    {
                        if ("(".equals(token.image))
                        {
                            brackets++;
                        }
                        else if (")".equals(token.image))
                        {
                            --brackets;
                        }
                        final FormulaOpCodeMapEntry opCode = parserNames.get(upper);
                        formulaToken = opCode.Token;
                    }
                    else if (token.kind == GeneratedFormulaParserConstants.WHITESPACE)
                    {
                        final FormulaOpCodeMapEntry opCode = specialOpCodes.get(FormulaMapGroupSpecialOffset.SPACES);
                        formulaToken = opCode.Token;
                    }
                    else
                    {
                        formulaToken = new FormulaToken();
                        formulaToken.OpCode = opCodePush.Token.OpCode;
                        formulaToken.Data = new Any(Type.STRING, token.image);
                    }

                    tokens.add(formulaToken);
                    token = tokenParser.getNextToken();
                }
                if (brackets > 0)
                {
                    final FormulaOpCodeMapEntry opCode = parserNames.get(")");
                    while (brackets-- != 0)
                    {
                        formula = formula.concat(")");
                        images.add(")");
                        tokens.add(opCode.Token);
                    }

                }

                parser.parse(formula);
            }
            catch (ParseException ex)
            {
                boolean found = false;
                // error occured so all token must be bad
                for (int i = 0; i < tokens.size(); i++)
                {
                    if (!found && ex.currentToken != null && images.get(i).equals(ex.currentToken.image))
                    {
                        found = true;
                    }
                    if (found)
                    {
                        final FormulaToken dest = new FormulaToken();
                        dest.OpCode = specialOpCodes.get(FormulaMapGroupSpecialOffset.BAD).Token.OpCode;
                        dest.Data = new Any(Type.STRING, images.get(i));
                        tokens.remove(i);
                        tokens.add(i, dest);
                    }
                }
            }
            catch (java.lang.Exception e)
            {
            }
            catch (TokenMgrError e)
            {
            }
        }
        return tokens.toArray(new FormulaToken[tokens.size()]);
    }

    public String printFormula(com.sun.star.sheet.FormulaToken[] aTokens, com.sun.star.table.CellAddress aReferencePos)
    {
        final StringBuffer ret = new StringBuffer();
        for (int i = 0; i < aTokens.length; i++)
        {
            final FormulaToken formulaToken = aTokens[i];
            if (formulaToken.OpCode == opCodePush.Token.OpCode && !formulaToken.Data.equals(Any.VOID))
            {
                ret.append(formulaToken.Data);
            }
            else if (parserAllOpCodes.containsKey(formulaToken.OpCode))
            {
                final FormulaOpCodeMapEntry opCode = parserAllOpCodes.get(formulaToken.OpCode);
                if (opCode.Name.length() > 0)
                {
                    ret.append(opCode.Name);
                }
                else if (!formulaToken.Data.equals(Any.VOID))
                {
                    ret.append(formulaToken.Data);
                }
            }
        }
        return ret.toString();
    }

    // com.sun.star.beans.XPropertySet:
    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        return m_prophlp.getPropertySetInfo();
    }

    public void setPropertyValue(String aPropertyName, Object aValue) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.beans.PropertyVetoException, com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.setPropertyValue(aPropertyName, aValue);
    }

    public Object getPropertyValue(String aPropertyName) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        return m_prophlp.getPropertyValue(aPropertyName);
    }

    public void addPropertyChangeListener(String aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.addPropertyChangeListener(aPropertyName, xListener);
    }

    public void removePropertyChangeListener(String aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.removePropertyChangeListener(aPropertyName, xListener);
    }

    public void addVetoableChangeListener(String aPropertyName, com.sun.star.beans.XVetoableChangeListener xListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.addVetoableChangeListener(aPropertyName, xListener);
    }

    public void removeVetoableChangeListener(String aPropertyName, com.sun.star.beans.XVetoableChangeListener xListener) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.WrappedTargetException
    {
        m_prophlp.removeVetoableChangeListener(aPropertyName, xListener);
    }

    // com.sun.star.report.meta.XFormulaParser:
    public com.sun.star.sheet.FormulaOpCodeMapEntry[] getOpCodeMap()
    {
        return m_OpCodeMap.toArray(new FormulaOpCodeMapEntry[m_OpCodeMap.size()]);
    }

    public void setOpCodeMap(com.sun.star.sheet.FormulaOpCodeMapEntry[] the_value)
    {
    }

    public String getImplementationName()
    {
        return SOFormulaParser.class.getName();
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

    public XFormulaOpCodeMapper getFormulaOpCodeMapper()
    {
        if (formulaOpCodeMapper == null)
        {
            formulaOpCodeMapper = new SOFormulaOpCodeMapper(this);
        }

        return formulaOpCodeMapper;
    }

    private void addOpCodes(String[] names, FormulaOpCodeMapEntry[] opCodes, int group)
    {
        addOpCodes(names, opCodes, group, true);
    }

    private void addOpCodes(String[] names, FormulaOpCodeMapEntry[] opCodes, int group, boolean add)
    {
        groupOpCodes[group] = new StringOpcodeMap();
        for (int j = 0; j < names.length; j++)
        {
            FormulaOpCodeMapEntry opCode = null;
            int i = 0;
            for (; i < opCodes.length; i++)
            {
                opCode = opCodes[i];
                if (names[j].equals(opCode.Name))
                {
                    break;
                }
            }
            if (i >= opCodes.length)
            {
                if (!add)
                {
                    continue;
                }
                final FormulaToken token = new FormulaToken(ownTokenCounter++, Any.VOID);
                opCode = new FormulaOpCodeMapEntry(names[j], token);
            }
            parserNames.put(names[j], opCode);
            parserAllOpCodes.put(opCode.Token.OpCode, opCode);
            groupOpCodes[group].put(opCode.Token.OpCode, opCode);
        }
    }

    public Map<String,FormulaOpCodeMapEntry> getNames()
    {
        return parserNames;
    }

    public Map<Integer,FormulaOpCodeMapEntry> getGroup(int group)
    {
        return groupOpCodes[group];
    }

    private String[] getOperators(DefaultFormulaContext defaultContext, final String _kind)
    {
        final ArrayList<String> ops = new ArrayList<String>();
        final Configuration configuration = defaultContext.getConfiguration();
        final Iterator iter = configuration.findPropertyKeys(_kind);
        while (iter.hasNext())
        {
            final String configKey = (String) iter.next();
            if (!configKey.endsWith(".class"))
            {
                continue;
            }
            final String operatorClass = configuration.getConfigProperty(configKey);
            if (operatorClass == null)
            {
                continue;
            }
            if (operatorClass.length() == 0)
            {
                continue;
            }
            final String tokenKey = configKey.substring(0, configKey.length() - ".class".length()) + ".token";
            final String token = configuration.getConfigProperty(tokenKey);
            if (token == null)
            {
                continue;
            }
            ops.add(token.trim());
        }
        return ops.toArray(new String[ops.size()]);
    }

    public List<FormulaOpCodeMapEntry> getSpecialOpCodes()
    {
        return specialOpCodes;
    }
}

