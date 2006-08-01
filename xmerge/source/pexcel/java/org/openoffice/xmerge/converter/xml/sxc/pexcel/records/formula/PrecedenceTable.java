/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PrecedenceTable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:07:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

import java.util.HashMap;

/**
 * This class defines the precedence applied to each operator when performing a conversion
 * {@link org.openoffice.xmerge.converter.xml.sxc.pexcel.Records.formula.FormulaCompiler.infix2 from infix to RPN.}.
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
