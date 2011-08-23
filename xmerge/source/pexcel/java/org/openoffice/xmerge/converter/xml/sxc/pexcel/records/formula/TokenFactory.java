/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
