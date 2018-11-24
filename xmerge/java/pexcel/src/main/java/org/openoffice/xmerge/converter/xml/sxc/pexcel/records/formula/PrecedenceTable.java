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



package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

import java.util.HashMap;

/**
 * This class defines the precedence applied to each operator when performing a conversion
 * {@link org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula.FormulaCompiler#infix2RPN  from infix to RPN.}.
 */
public class PrecedenceTable {
    public static final int DEFAULT_PRECEDENCE  = 0;
    public static final int EQNEQ_PRECEDENCE    = 1; // =, <>
    public static final int GTLTEQ_PRECEDENCE   = 1; // >=, <=
    public static final int GTLT_PRECEDENCE     = 2; // >, <
    public static final int ADDOP_PRECEDENCE    = 4; // +, -
    public static final int MULTOP_PRECEDENCE   = 5; // *, /
    public static final int FACTOR_PRECEDENCE   = 6; // ^
    public static final int CONCAT_PRECEDENCE   = 6; // &
    public static final int UNARY_PRECEDENCE    = 7; // !, Unary +, Unary -
    public static final int PAREN_PRECEDENCE    = 8; // (,  )
    public static final int FUNCTION_PRECEDENCE = 8;
    public static final int COMMA_PRECEDENCE    = 8;

    private static HashMap map;
    static {
        map = new HashMap();

        map.put("%", new Integer(UNARY_PRECEDENCE));
        map.put("+", new Integer(ADDOP_PRECEDENCE));
        map.put("-", new Integer(ADDOP_PRECEDENCE));
        map.put("*", new Integer(MULTOP_PRECEDENCE));
        map.put("/", new Integer(MULTOP_PRECEDENCE));
        map.put("(", new Integer(PAREN_PRECEDENCE));
        map.put(")", new Integer(PAREN_PRECEDENCE));
        map.put(",", new Integer(COMMA_PRECEDENCE));
        map.put(">", new Integer(GTLT_PRECEDENCE));
        map.put("<", new Integer(GTLT_PRECEDENCE));
        map.put("=", new Integer(EQNEQ_PRECEDENCE));
        map.put("&", new Integer(CONCAT_PRECEDENCE));
        map.put("^", new Integer(FACTOR_PRECEDENCE));
        map.put(">=", new Integer(GTLTEQ_PRECEDENCE));
        map.put("<=", new Integer(GTLTEQ_PRECEDENCE));
        map.put("<>", new Integer(EQNEQ_PRECEDENCE));
        map.put("FUNCTION", new Integer(FUNCTION_PRECEDENCE));
    }

    /**
     *  Retrieve the precedence value for a given operator.
     *  @param  op  Look up the precedence for this operator
     *  @return an integer representing the integer value of the operator
     */
    public static int getPrecedence(String op) {
        Object obj = map.get(op);
        if (obj == null) {
            return DEFAULT_PRECEDENCE;
        }
        return ((Integer)obj).intValue();
    }
}
