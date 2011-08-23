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

import java.util.*;
import org.openoffice.xmerge.util.Debug;

/**
 * FormulaCompiler converts Calc formula string into PocketXL bytes 
 * and PocketXL formula bytes into Calc Formula strings
 * 
 * For converting from infix to Reverse Polish (or Postfix) notation the string is
 * converted into a vector of Tokens and then re-ordered based on a modified version
 * of the standard Infix to RPN conversion algorithms.
 * <pre>
 *	Infix2Rpn(tokens)
 *		while have more tokens
 *			if token is operand 
 *				push to stack
 *			else if token is function, argument separater, or open bracket
 *				push token
 *				extract tokens to matching close bracket into param
 *				Infix2Rpn(param)
 *			else if token is close bracket
 *				pop from stack into result until close bracket or function
 *			else 
 *				while stack.top.priority >= token.priority
 *					add stack.pop to result
 *				push token onto stack
 * </pre>
 * For converting from RPN to Infix the following algorithm is applied:
 * <pre>
 * 		while have more tokens
 * 			if token is operand
 *				push token to stack
 *			else if token is function or operator
 *				pop from stack number of args required by token
 *				apply token to params to make expr
 *				push expr to stack
 *		return stack.pop
 * </pre>
 */
public class FormulaCompiler {
    /**
     *	Constructs a FormulaCompiler object
     */
    public FormulaCompiler() {
    }
    
    private boolean isPercent(Token pt) {
        return pt.getTokenID() == TokenConstants.TPERCENT;
    }
    
    private boolean isOpenBrace(Token pt) {
        return pt.getTokenID() == TokenConstants.TPAREN;
    }
    
    private boolean isCloseBrace(Token pt) {
        return pt.getValue().compareTo(")") == 0;
    }
    
    private boolean isParamDelimiter(Token pt) {
        return pt.getTokenID() == TokenConstants.TARGSEP;
    }    

    private boolean isBinaryOperator(Token pt) {
        return false;
    }
    
    /**
     * Re-order into Infix format
     * @param	tokens	The tokens in RPN form
     * @return	The vector of tokens re-ordered in Infix notation
     */
    public Vector RPN2Infix(Vector tokens) {
        Vector infixExpr = new Vector(15);
        ListIterator iter = tokens.listIterator();
        Stack evalStack = new Stack();
        Stack args = new Stack();
        
        while (iter.hasNext()) {
            Token pt = (Token)iter.next();
            if (pt.isOperand()) {
                Vector expr = new Vector(5);
                expr.add(pt);
                evalStack.push(expr);
            } else if (pt.isOperator() || pt.isFunction()) {
                args.clear();
                for (int i=0; i< pt.getNumArgs(); i++) {
                    args.push(evalStack.pop());
                }
                evalStack.push(makeExpression(pt, args));
            }
        }
        return (Vector)evalStack.elementAt(0);
    }
    
    /**
     * Convert the infix expression to RPN. Note that open brackets are saved onto the stack to preserve the users bracketing.
     * <p>Also note that the open bracket following functions is not pushed onto the stack - it is always implied when
     * writing out results
     *
     * @param	tokens	The vector of tokens in Infix form
     *
     * @return	A vector of tokens for the expression in Reverse Polish Notation order
     */
    public Vector infix2RPN(Vector tokens) {
        Vector rpnExpr = new Vector(15);
        Stack evalStack = new Stack();
        ListIterator iter = tokens.listIterator();
        while (iter.hasNext()) {
            Token pt = (Token)iter.next();
            
            if (pt.isOperand()) { //Operands are output immediately
                rpnExpr.add(pt);
            } else if (pt.isFunction() || isParamDelimiter(pt) || isOpenBrace(pt)) { //Extract parameters after afunction or comma
                evalStack.push(pt);
                if (pt.isFunction()) {
                    iter.next();
                }
                Vector param = extractParameter(iter);
                Debug.log(Debug.TRACE, "Extracted parameter " + param);        
                rpnExpr.addAll(infix2RPN(param));
            } else if (isCloseBrace(pt)) { //Pop off stack till you meet a function or an open bracket
                Token tmpTok = null;
                boolean bPop = true;
                while (bPop) {
                    if (evalStack.isEmpty()) {
                        bPop = false;
                    } else {
                        tmpTok = (Token)evalStack.pop();
                        //if (!(isOpenBrace(tmpTok) || isParamDelimiter(tmpTok))) { //Don't output brackets and commas
                        if (!isParamDelimiter(tmpTok)) { //Don't output commas
                            rpnExpr.add(tmpTok);
                        }
                        if (tmpTok.isFunction() || isOpenBrace(tmpTok)) {
                            bPop = false;        
                        }
                    }
                }
            } else {
                if (!evalStack.isEmpty()) {
                    while (!evalStack.isEmpty() && 
                            (((Token)evalStack.peek()).getTokenPriority() >=pt.getTokenPriority())) {
                        Token topTok = (Token)evalStack.peek();
                        if (topTok.isFunction() || isOpenBrace(topTok)) {
                            break;
                        }
                           rpnExpr.add(evalStack.pop());
                    }
                }
                evalStack.push(pt);                
            }
        }
        
        while (!evalStack.isEmpty()) {
            Token topTok = (Token)evalStack.peek();
            if (!(isOpenBrace(topTok) || isParamDelimiter(topTok))) { //Don't output brackets and commas
                rpnExpr.add(evalStack.pop());
            }
            else
            {
                evalStack.pop();
            }
        }
        return rpnExpr;
    }
    
    /**
     * Extract a parameter or bracketed sub-expression
     * @param iter an iterator into the list
     * @return A complete sub-expression
     */
    protected Vector extractParameter(ListIterator iter) {
        Vector param = new Vector(5);
        int subExprCount = 0;
        
        while (iter.hasNext()) {
            Token pt = (Token)iter.next();
            Debug.log(Debug.TRACE, "Token is " + pt + " and subExprCount is " + subExprCount);        
            if (isOpenBrace(pt)) {
                subExprCount++;
                param.add(pt);
            } else if (isCloseBrace(pt)) {
                if (subExprCount == 0) {
                    iter.previous();
                    return param;
                } else {
                    subExprCount--;
                    param.add(pt);
                }
            } else if (isParamDelimiter(pt) && (subExprCount == 0)) {
                iter.previous();
                return param;
            } else {
                param.add(pt);
            }
        }
        return param;
    }
    
    /**
     * Given the operator and it's operators
     * @param 	pt	The operator token
     * @param	args	The arguments for this operator
     * @return	A correctly ordered expression
     */
    protected Vector makeExpression(Token pt, Stack args) {
        Vector tmp = new Vector(5);
        TokenFactory tf = new TokenFactory();
        if (pt.isOperator()) {
            if (pt.getNumArgs()==2) { //Binary operator
                tmp.addAll((Vector)args.pop());
                tmp.add(pt);
                tmp.addAll((Vector)args.pop());
            } else if (pt.getNumArgs() == 1) {
                if(isPercent(pt)) {
                    tmp.addAll((Vector)args.elementAt(0));
                    tmp.add(pt);
                } else {
                    tmp.add(pt);
                    tmp.addAll((Vector)args.elementAt(0));
                }
                if (isOpenBrace(pt)) {
                    tmp.add(tf.getOperatorToken(")",1));
                }
            }
        } else if (pt.isFunction()) {
            tmp.add(pt);
            tmp.add(tf.getOperatorToken("(",1));
            if (!args.isEmpty()) {
                Vector v = (Vector)args.pop();
                tmp.addAll(v);
            }
            while (!args.isEmpty()) {
                tmp.add(tf.getOperatorToken(",",1));
                Vector v = (Vector)args.pop();
                tmp.addAll(v);

            }
            tmp.add(tf.getOperatorToken(")",1));
        }

        return tmp;
    }
}
