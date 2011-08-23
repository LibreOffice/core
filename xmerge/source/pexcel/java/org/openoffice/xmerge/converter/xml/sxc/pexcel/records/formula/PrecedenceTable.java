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

import java.util.HashMap;

/**
 * This class defines the precedence applied to each operator when performing a conversion
 * {@link org.openoffice.xmerge.converter.xml.sxc.pexcel.Records.formula.FormulaCompiler.infix2	from infix to RPN.}.
 */
public class PrecedenceTable {
    public static final int DEFAULT_PRECEDENCE  = 0;
    public static final int EQNEQ_PRECEDENCE    = 1; // =, <>
    public static final int GTLTEQ_PRECEDENCE	= 1; // >=, <=
    public static final int GTLT_PRECEDENCE		= 2; // >, <
    public static final int ADDOP_PRECEDENCE    = 4; // +, -
    public static final int MULTOP_PRECEDENCE   = 5; // *, /
    public static final int FACTOR_PRECEDENCE	= 6; // ^
    public static final int CONCAT_PRECEDENCE	= 6; // &
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
     *	Retrieve the precedence value for a given operator.
     *	@param	op	Look up the precedence for this operator
     *	@return an integer representing the integer value of the operator
     */
    public static int getPrecedence(String op) {
        Object obj = map.get(op);
        if (obj == null) {
            return DEFAULT_PRECEDENCE; 
        }
        return ((Integer)obj).intValue();
    }
}
