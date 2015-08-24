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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Vector;
import java.util.Enumeration;

import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;

/**
 * This Helper class provides a simplified interface to conversion between PocketXL formula representation
 * and Calc formula representation.<p>
 * The class is used by {@link org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Formula}
 */
public class FormulaHelper {

    private static FormulaParser parser;
    private static FormulaCompiler compiler;
    private static TokenEncoder encoder;
    private static TokenDecoder decoder;
    private boolean rangeType = false;
    private boolean expressionType = false;

    static {
        parser   = new FormulaParser();
        compiler = new FormulaCompiler();
        encoder = new TokenEncoder();
        decoder  = new TokenDecoder();
    }

    /**
     * Sets the workbook cache so that global data such as
     * <code>DefinedNames</code>, <code>Boundsheets</code> can be read
     *
     * @param wb Wrokbook object containing all the global data
     */
    public void setWorkbook(Workbook wb) {

        encoder.setWorkbook(wb);
        decoder.setWorkbook(wb);
        parser.setWorkbook(wb);
    }

    /**
     * Convertes a string representation of a calc formula into an array of PocketXL bytes
     * @param   formula The Formula String (e.g. 1+SUM(A1,B1))
     *
     * @throws  UnsupportedFunctionException    Thrown if a function in the formula is nto supported by Pocket Excel
     * @throws  FormulaParsingException Thrown when the formula is not well formed
     *
     */
    public byte[] convertCalcToPXL(String formula) throws UnsupportedFunctionException, FormulaParsingException {

        Vector parseTokens = parser.parse(formula);
        Vector rpnTokens = compiler.infix2RPN(parseTokens);

        ByteArrayOutputStream bytes = null;
        try {
            bytes = new ByteArrayOutputStream();
            for (Enumeration e = rpnTokens.elements(); e.hasMoreElements();) {
                Token t = (Token)e.nextElement();
                bytes.write(encoder.getByte(t));
            }
        } catch (IOException e) {
        }

        return bytes.toByteArray();
    }

    /**
     * Converts a PocketXL byte array into a Calc function string
     * @param   formula A byte array that contains the PocketXL bytes for a formula
     *
     */
    public String convertPXLToCalc(byte[] formula) {

        Vector parseTokens = decoder.getTokenVector(formula);
        Vector infixTokens = compiler.RPN2Infix(parseTokens);

        StringBuffer buff = new StringBuffer();
        for (Enumeration e = infixTokens.elements();e.hasMoreElements();) {
            Token t = (Token)e.nextElement();
            buff.append(t.toString());
            // If we are parsing a Name definition we need to know if it is of
            // type range or expression
            if(!t.isOperand()) {
                expressionType = true;
            }
        }
        if(!expressionType) {
            rangeType = true;
        }
        return "=" + buff.toString();
    }

    /**
     * Returns a boolean indicating whether or not the byte[] parsed is of
     * type range. This means it contains only a cell reference and no
     * operators. This is necessry because the syntax for range and expression
     * types differs. This is only of interest when dealing with
     * <code>DefinedNames</code> and not <code>Formula</code>
     *
     * @return a boolean true if of type range otherwise false
     *
     */
     public boolean isRangeType() {

        return rangeType;
    }

    /**
     * Returns a boolean indicating whether or not the byte[] parsed is of
     * type expression. This means it contains operators. This is necessry
     * because the syntax for range and expression types differs. This is
     * only of interest when dealing with <code>DefinedNames</code> and not
     * <code>Formula</code>
     *
     * @return a boolean true if of type expression otherwise false
     *
     */
     public boolean isExpressionType() {

        return expressionType;
    }
}
