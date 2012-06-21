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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

import org.openoffice.xmerge.util.Debug;

/**
 * This is the Factory class responsible for creating a <code>Token</code>.
 * It has three methods for returning three different types of Tokens
 * (Operator, Operand and Function).
 * This utility class is used by either the <code>FormulaParser</code> or the
 * <code>FormulaDecoder</code>.
 */
public class TokenFactory {

    private OperatorLookup operatorLookup;
    private OperandLookup operandLookup;
    private FunctionLookup fl;

    /**
     * Default Constructor
     */
    public TokenFactory() {
        operatorLookup = new OperatorLookup();
        operandLookup = new OperandLookup();
        fl = new FunctionLookup();
    }

    /**
     * The Factory method for creating function Tokens
     *
     * @return The created <code>Token</code>
     */
    public Token getFunctionToken(String s, int args) {
        Token t = null;
        // We will have to fix this later to include fixed function tokens
        // Also will need to handle errors where functions names are incorrect???
        Debug.log(Debug.TRACE,"TokenFactory creating function Token : " + s);
        try {
            t = new Token(s, ParseToken.TOKEN_FUNCTION_VARIABLE, fl.getIDFromString(s), args);
        } catch (UnsupportedFunctionException eFn) {

            Debug.log(Debug.ERROR, eFn.getMessage());
        }
        return t;
    }

    /**
     * The Factory method for creating operator Tokens
     *
     * @return The created <code>Token</code>
     */
    public Token getOperatorToken(String s, int args) {

        Token t = null;

        Debug.log(Debug.TRACE,"TokenFactory creating operator Token : " + s);
        try  {
            if(args==1) {
                if(s.equals("+")) {
                    t = new Token(s, ParseToken.TOKEN_OPERATOR, operatorLookup.getIDFromString("UNARY_PLUS"), args);
                } else if (s.equals("-")) {
                    t = new Token(s, ParseToken.TOKEN_OPERATOR, operatorLookup.getIDFromString("UNARY_MINUS"), args);
                } else {
                    t = new Token(s, ParseToken.TOKEN_OPERATOR, operatorLookup.getIDFromString(s), args);
                }
            } else {
                t = new Token(s, ParseToken.TOKEN_OPERATOR, operatorLookup.getIDFromString(s), args);
            }
        } catch (UnsupportedFunctionException eFn) {
            Debug.log(Debug.ERROR, eFn.getMessage());
        }
        return t;
    }

    /**
     * The Factory method for creating Operand Tokens
     *
     * @return The created <code>Token</code>
     */
    public Token getOperandToken(String s, String type) {
        Token t = null;

        Debug.log(Debug.TRACE,"TokenFactory creating operand (" + type + ") Token : " + s);
        try {
            t = new Token(s, ParseToken.TOKEN_OPERAND, operandLookup.getIDFromString(type), 0);
        } catch (UnsupportedFunctionException eFn) {
            Debug.log(Debug.ERROR, eFn.getMessage());
        }

        return t;
    }
}
