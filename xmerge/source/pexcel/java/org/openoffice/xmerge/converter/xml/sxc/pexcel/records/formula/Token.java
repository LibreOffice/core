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


/**
 * A Token is the basic building block of any formula.
 * A Token can be of four types (Operator, Operand, Function with fixed
 * arguments and function with a variable number of arguments. Each type can
 * have numerous id's. Thetypes are define in <code>ParseToken</code> and the
 * id's are defined in <code>TokenConstants</code>. The other member variables
 * are priority which is returned from the <code>PrecedenceTable</code>, the
 * value which is the String equivalent of the token (eg. "+", "$A$12", "SUM")
 * and the number of arguments which is only valid for operators and functions.
 * Tokens should never be created directly and instead are created by the
 * <code>TokenFactory</code>
 */
public class Token implements ParseToken {

    private String value;
    private int type;           // operator, operand, function fixed, function variable
    private int id;             // cell reference, SUM, integer
    private int priority;
    private int numArgs=-1;

    public Token(String op, int type, int id, int args) {
        this.value = op;
        this.type = type;
        this.id = id;
        this.numArgs = args;
        if(type==ParseToken.TOKEN_FUNCTION_VARIABLE) {
            priority = PrecedenceTable.getPrecedence("FUNCTION");
        } else if(type==ParseToken.TOKEN_OPERATOR) {
            priority = PrecedenceTable.getPrecedence(op);
        } else {
            priority = PrecedenceTable.getPrecedence("DEFAULT");
        }
    }

    /**
     * Checks if the current token is an operator
     *
     * @return A <code>boolean</code> result of the comaparison
     */
    public boolean isOperator() {
        return type == ParseToken.TOKEN_OPERATOR;
    }

    /**
     * Checks if the current token is an operand
     *
     * @return A <code>boolean</code> result of the comaparison
     */
    public boolean isOperand() {
        return type == ParseToken.TOKEN_OPERAND;
    }

    /**
     * Checks if the current token is a function
     *
     * @return A <code>boolean</code> result of the comaparison
     */
    public boolean isFunction() {
        return (type==ParseToken.TOKEN_FUNCTION_FIXED) || (type==ParseToken.TOKEN_FUNCTION_VARIABLE);
    }

    /**
     * Returns the token type. This can be one of four values (TOKEN_OPERATOR,
     * TOKEN_OPERAND, TOKEN_FUNCTION_FIXED, TOKEN_FUNCTION_VARIABLE) defined in
     * <code>ParseToken</code>
     *
     * @return A <code>boolean</code> result of the comparison
     */
    public int getTokenType() {

        return type;
    }

    /**
     * Returns the ID of this token. This ID is equivalent to the pexcel hex
     * value and is defined in <code>ParseToken</code>
     *
     * @return Returns the id of this token
     */
    public int getTokenID() {

        return id;
    }

    /**
     * Returns the <code>String</code> equivalent of this token
     *
     * @return The <code>String</code> representing this Token
     */
    public String getValue() {
        return value;
    }

    /**
     * Returns the number of arguments if this token represents an operator or
     * function. Otherwise returns -1.
     *
     * @return The number of arguments
     */
    public int getNumArgs() {
        return numArgs;
    }

    /**
     * Checks if the current token is an operator
     *
     * @return A <code>boolean</code> result of the comparison
     */
    public int getTokenPriority() {
        return priority;
    }

    /**
     * Returns the <code>String</code> equivalent of this token
     *
     * @return The <code>String</code> representing this Token
     */
    public String toString() {
        return getValue();
    }
}
