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


import java.util.ArrayList;

import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;
import org.openoffice.xmerge.util.Debug;

/**
 * This is the Formula Parser based on an article written by Jack Crenshaw. It is a
 * top down parser with some basic error handling. It handles
 * +,-,*,/,>,<,>=,<=,=,<>, unary + and - as well as functions.
 * The BNF notation for this parser is
 * <pre>
 *  &lt;expression&gt; ::= &lt;unary op&gt; &lt;term&gt; [&lt;addop&gt;|&lt;logop&gt; &lt;term&gt;]
 *  &lt;term&gt;       ::= &lt;factor&gt; [&lt;mulop&gt; &lt;factor&gt;]
 *  &lt;factor&gt;     ::= &lt;number&gt;[%] | &lt;CellRef&gt; | &lt;QuoteString&gt; | &lt;expression&gt;
 * </pre>
 */
public class FormulaParser {

    private char look;
    private String formulaStr;
    private int index = 1;
    private TokenFactory tokenFactory;
    private ArrayList<Token> tokenVector;
    private Workbook wb;

     /**
      * Default constructor
      */
     public FormulaParser() {

        Debug.log(Debug.TRACE,"Creating a Formula Parser");
        tokenFactory = new TokenFactory();
        tokenVector = new ArrayList<Token>();
    }

    /**
     *
     */
    public void setWorkbook(Workbook wb) {

        this.wb = wb;
    }

    /**
     * Parse method for parsing from a String to a byte[]
     *
     * @param formula A <code>String</code> representation of a formula
     * starting with the '=' character
     * @return A <code>Vector</code> containing the parsed <code>Token</code>s
     */
    public ArrayList<Token> parse(String formula) throws FormulaParsingException {

        index = 1;
        look = ' ';
        tokenVector.clear();
        if(formula.startsWith("=")) {
            formulaStr = formula;
            Debug.log(Debug.TRACE,"Creating a Formula Parser for " + formulaStr);
            getChar();
            expression();
        } else {
            throw new FormulaParsingException("No equals found!" + makeErrorString());
        }
        return tokenVector;
    }

    /**
     * Identify + and - operators
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
     */
    private boolean isAddOp(char c) {
         return (c == '-') || (c == '+');
     }

    /**
     * Determine if the current character is a multiop
     *
     * @return A boolean returning the result of the comparison
     */
    private boolean isMultiOp() {
        return look=='*' || look =='/' || look == '^' || look == '&';
    }

    /**
     * Identify <, >, <=, >=, =, <> using the index to find the current character(s)
     *
     * @return A boolean returning the result of the comparison
     */
    private boolean isLogicalOp() {
        if (!isLogicalOpChar(look)) {
            return false;
        } else if ((index+1) >= formulaStr.length()) {//logical operators in their own right : if at end then return true
            return true;
        } else if (!isLogicalOpChar(formulaStr.charAt(index))) { // we have >, < or = on their own
            return true;
        } else if ((look == '<') && ((formulaStr.charAt(index) == '>') || formulaStr.charAt(index) == '=')) { // <>, or <=
            return true;
        } else if ((look == '>')  && (formulaStr.charAt(index) == '=')) { // >=
            return true;
        }

        return false;
     }

    /**
     * Identify <, >, <=, >=, =, <>
     *
     * @param  op The <code>String</code> which is to be identified
     * @return A boolean returning the result of the comparison
     */
    private boolean isLogicalOp(String op) {
        return  ((op.compareTo(">") == 0) ||
                 (op.compareTo("<") == 0) ||
                 (op.compareTo(">=") == 0) ||
                 (op.compareTo("<=") == 0) ||
                 (op.compareTo("=") == 0) ||
                 (op.compareTo("<>") == 0));
    }


    /**
     * Identify characters that MAY be logical operator characters
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
     */
    private boolean isLogicalOpChar(char c) {
        return (c == '>') || (c == '<') || (c == '=');
    }

     /**
      * Identify special Cell Reference charaters
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
     private boolean isCellRefSpecialChar(char c) {
         return (c == ':') || (c == '$') || (c == '.');
     }

     /**
      * Identify letters
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
     private boolean isAlpha(char c) {
         return(Character.isLetter(c));
     }

     /**
      * Identify numbers
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
     private boolean isDigit(char c) {
         return(Character.isDigit(c));
     }

     /**
      * Identify numbers
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
     private boolean isPercent(char c) {
         return (c == '%');
     }

     /**
      * Identify letters or numbers
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
     private boolean isAlphaNum(char c) {
         return(isAlpha(c) || isDigit(c));
     }

    /**
      * Identify valid Characters for cell references
     *
     * @param  c The character which is to be identified
     * @return A boolean returning the result of the comparison
      */
     private boolean isCellRefChar(char c) {
         return(isAlpha(c) || isDigit(c) || isCellRefSpecialChar(c));
     }

    /**
     * Test if current character is a match and move to next character
     *
     * @param  c The character which is to be matched
     */
     private void match(char c) throws FormulaParsingException {

         if(look==c) {
            Debug.log(Debug.TRACE,"Operator Found : " + look);
             getChar();
             skipWhite();
         }
         else
             throw new FormulaParsingException("Unexpected character '" + c + "'" + makeErrorString());
     }

    /**
     * Test if current character is a match and move to next character
     *
     * @param symbol The <code>String</code> to be matched.
     */
     private void match(String symbol) throws FormulaParsingException {

         int numChars = symbol.length();
        boolean bContinue = true;
        for (int i=0;i<numChars && bContinue; i++) {
            if (look == symbol.charAt(i)) {
                bContinue = getChar();
                skipWhite();
            } else {
                throw new FormulaParsingException("Unexpected character '" + symbol + "'" + makeErrorString());
            }
        }
     }

     /**
      * Skip over whitespaces (ie. spaces and tabs)
      */
     private void skipWhite() throws FormulaParsingException {

         boolean success = true;

         while(Character.isWhitespace(look) && success) {
             success = getChar();
         }
     }

     /**
      * This is a factor for multiplication and division operators
      */
     private void factor() throws FormulaParsingException {
        if(isAddOp(look)) {         // handle unary addop
            Character ch = new Character(look);
            match(look);
            tokenVector.add(tokenFactory.getOperatorToken(ch.toString(), 1));
        }
         if(look=='(') {
             match('(');
            tokenVector.add(tokenFactory.getOperatorToken("(", 1));
             expression();
             match(')');
                        tokenVector.add(tokenFactory.getOperatorToken(")", 1));
         } else if(isDigit(look)){
             getNum();
         } else {
            ident();
         }
     }

     /**
      * Pulls the next character from the <code>String</code>
      *
      * @return boolean false if the end if the statement
      *                     is reached otherwise true
      */
     private boolean getChar() throws FormulaParsingException {

             boolean success = true;

             if(index<formulaStr.length()) {
                 look = formulaStr.charAt(index);
                 index++;
                                if(look==',')
                                    success = false;
             } else {
                 success = false;
             }
             return success;
     }

    /**
     * Parses the number of arguments in a function
     *
     * @return The number of arguments
     */
    private int arguments() throws FormulaParsingException {
        int numArgs;

        skipWhite();
        if(look==')')
            numArgs = 0;
        else
            numArgs = 1;

        while(look!=')') {
            expression();
            if(look==',') {
                numArgs++;
                match(',');
                tokenVector.add(tokenFactory.getOperatorToken(",", 1));
            }
        }
        return numArgs;
    }

     /**
      * Test to see if we have come across a cell reference or a Name
     * Definition.
     */
     private boolean isCellRef(String s) {
         char c;
        boolean result = false;

        for(int i = 0;i<s.length();i++) {
            c = s.charAt(i);
            if(isCellRefSpecialChar(c)) {
                result = true;
                break;
            }
        }

        // if it is a simple cell reference then there will not be a cell
        // reference 'special char' so we should also look for a digit
        if(!result) {
            if(isDigit(s.charAt(1)) || isDigit(s.charAt(2))) {
                result = true;
            }
        }
        return result;
     }

     /**
      * Test to see if we have come across a cell reference or a function and
     * add the resulting toek nto the tokenVector.
     */
     private void ident() throws FormulaParsingException {

         String cell = getTokenString();
        if(look=='(') {
            Debug.log(Debug.TRACE,"Found Function : " + cell);

            int index = tokenVector.size();
             match('(');
            tokenVector.add(tokenFactory.getOperatorToken("(", 1));
            int numArgs = arguments();
            match(')');
            tokenVector.add(tokenFactory.getOperatorToken(")", 1));
            tokenVector.add(index, tokenFactory.getFunctionToken(cell, numArgs));
         } else {

            if(cell.indexOf('.')!=-1) {
                String cellRef = cell.substring(cell.indexOf('.') + 1, cell.length());
                if(cellRef.indexOf(':')!=-1) {
                    tokenVector.add(tokenFactory.getOperandToken(cell, "3D_CELL_AREA_REFERENCE"));
                } else {
                    tokenVector.add(tokenFactory.getOperandToken(cell, "3D_CELL_REFERENCE"));
                }
            } else if(cell.indexOf(':')!=-1) {
                tokenVector.add(tokenFactory.getOperandToken(cell, "CELL_AREA_REFERENCE"));
            } else if(isCellRef(cell)) {
                tokenVector.add(tokenFactory.getOperandToken(cell, "CELL_REFERENCE"));
            } else {
                tokenVector.add(tokenFactory.getOperandToken(cell, "NAME"));
            }
         }
     }

     /**
     * Will keep pulling valid logical operators from the formula and return
     * the resultant <code>String</code>.
     *
     * @return a <code>String<code> representing a logical operator
     */
    private String getLogicalOperator() throws FormulaParsingException {
        String op = new String();
        boolean status;

        do {
            op += look;
            status = getChar();
        } while(isLogicalOpChar(look) && status);
         skipWhite();
        return op;
    }

     /**
      * Keeps pulling characters from the statement until we get an
     * operator and returns the resulting string.
     *
     * @return A <code>String</code>representing the next token
      */
      private String getTokenString() throws FormulaParsingException {

         if(!isAlpha(look) && look!='$')
             throw new FormulaParsingException("Expected Cell Reference" + makeErrorString());
         else {
             String cell = new String();
                        boolean status;
             do {
                 cell += look;
                status = getChar();
             } while(isCellRefChar(look) && status);
             skipWhite();
                        return cell;
           }
     }

     /**
      * Keeps pulling numbers from the statement and add the resulting integer
     * token to the tokenVector.
      */
     private void getNum() throws FormulaParsingException {

        Debug.log(Debug.TRACE,"getNum : ");
         if(!isDigit(look))
             throw new FormulaParsingException("Expected Integer" + makeErrorString());
         else {
             String num = new String();
             boolean status;

             do {
                 num += look;
                status = getChar();
             } while((isDigit(look) || ((look == '.') && isDigit(formulaStr.charAt(index)))) && status);
             skipWhite();
            tokenVector.add(tokenFactory.getOperandToken(num, "INTEGER"));
            if(isPercent(look)) {
                match(look);
                tokenVector.add(tokenFactory.getOperatorToken("%", 1));
                Debug.log(Debug.TRACE,"Added Percent token to Vector: ");
            }
            Debug.log(Debug.TRACE,"Number parsed : " + num);
         }
     }


    /**
     * Term will parse multiplication/division expressions
     */
     private void term() throws FormulaParsingException {
        factor();
        while(isMultiOp()) {
            multiOp(Character.toString(look));
        }
     }

     /**
      * Expression is the entry point for the parser. It is the code
     * that parses addition/subtraction expressions.
      */
     private void expression() throws FormulaParsingException {

        if (look == '"') { //Extract a quoted string...
            StringBuffer buff = new StringBuffer();
            boolean success = true;
            success = getChar();
            while (look != '"' && success) {
                buff.append(look);
                success = getChar();
            }

            if (look != '"') { //We've reached the end of the string without getting a closing quote
                throw new FormulaParsingException("Expected closing quote." + makeErrorString());
            } else {
                tokenVector.add(tokenFactory.getOperandToken(buff.toString(), "STRING"));
                getChar();      //Move on to the next character
            }
        } else {
            term();
        }
        while(isAddOp(look) || isLogicalOp()) {
            if (isAddOp(look)) {
                addOp(Character.toString(look));
            } else if (isLogicalOp()) {
                logicalOp();
            }
        }
     }

    /**
     * Test to see if the next token (represented as a <code>String</code>) is
     * the same as the String passed in. Move the index along to the end of
     * that String and add that <code>Token</code> to the tokenVector. Then
     * call <code>term</code> to parse the right hand side of the operator.
     *
     * @param op A <code>String</code> representing the operator
     */
    private void addOp(String op) throws FormulaParsingException {
         match(op);
        tokenVector.add(tokenFactory.getOperatorToken(op, 2));
         term();
    }

    /**
     * Test to see if the next token (represented as a <code>String</code>) is
     * the same as the String passed in. Move the index along to the end of
     * that String and add that <code>Token</code> to the tokenVector. Then
     * call <code>factor</code> to parse the right hand side of the operator.
     *
     * @param op A <code>String</code> representing the operator
     */
    private void multiOp(String op) throws FormulaParsingException {
        match(op);
        tokenVector.add(tokenFactory.getOperatorToken(op, 2));
         factor();
    }

    /**
     * Pull a logical operator starting at the current index, add a token for
     * that operator to the tokenVector and call <code>term<code> to parse the
     * right hand side of the operator
     */
    private void logicalOp()  throws FormulaParsingException {
        String op = getLogicalOperator();
        tokenVector.add(tokenFactory.getOperatorToken(op, 2));
        term();
    }

    private String makeErrorString() {
        StringBuffer buff = new StringBuffer();
        for (int i=0; i<index-1; i++) {
            buff.append(' ');
        }

        buff.append('^');
        return "\n\t" + formulaStr + "\n\t" + buff.toString();
    }
 }

