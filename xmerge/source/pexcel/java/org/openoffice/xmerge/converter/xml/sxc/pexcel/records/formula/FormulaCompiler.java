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
import java.util.ListIterator;
import java.util.Stack;

import org.openoffice.xmerge.util.Debug;

/**
 * FormulaCompiler converts Calc formula string into PocketXL bytes
 * and PocketXL formula bytes into Calc Formula strings
 *
 * For converting from infix to Reverse Polish (or Postfix) notation the string is
 * converted into a vector of Tokens and then re-ordered based on a modified version
 * of the standard Infix to RPN conversion algorithms.
 * <pre>
 *  Infix2Rpn(tokens)
 *      while have more tokens
 *          if token is operand
 *              push to stack
 *          else if token is function, argument separater, or open bracket
 *              push token
 *              extract tokens to matching close bracket into param
 *              Infix2Rpn(param)
 *          else if token is close bracket
 *              pop from stack into result until close bracket or function
 *          else
 *              while stack.top.priority >= token.priority
 *                  add stack.pop to result
 *              push token onto stack
 * </pre>
 * For converting from RPN to Infix the following algorithm is applied:
 * <pre>
 *      while have more tokens
 *          if token is operand
 *              push token to stack
 *          else if token is function or operator
 *              pop from stack number of args required by token
 *              apply token to params to make expr
 *              push expr to stack
 *      return stack.pop
 * </pre>
 */
public class FormulaCompiler {
    /**
     *  Constructs a FormulaCompiler object
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

    /**
     * Re-order into Infix format
     * @param   tokens  The tokens in RPN form
     * @return  The vector of tokens re-ordered in Infix notation
     */
    public ArrayList<Token> RPN2Infix(ArrayList<Token> tokens) {
        ListIterator<Token> iter = tokens.listIterator();
        Stack<ArrayList<Token>> evalStack = new Stack<ArrayList<Token>>();
        Stack<ArrayList<Token>> args = new Stack<ArrayList<Token>>();

        while (iter.hasNext()) {
            Token pt = iter.next();
            if (pt.isOperand()) {
                ArrayList<Token> expr = new ArrayList<Token>(5);
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
        return evalStack.elementAt(0);
    }

    /**
     * Convert the infix expression to RPN. Note that open brackets are saved onto the stack to preserve the users bracketing.
     * <p>Also note that the open bracket following functions is not pushed onto the stack - it is always implied when
     * writing out results
     *
     * @param   tokens  The vector of tokens in Infix form
     *
     * @return  A vector of tokens for the expression in Reverse Polish Notation order
     */
    public ArrayList<Token> infix2RPN(ArrayList<Token> tokens) {
        ArrayList<Token> rpnExpr = new ArrayList<Token>(15);
        Stack<Token> evalStack = new Stack<Token>();
        ListIterator<Token> iter = tokens.listIterator();
        while (iter.hasNext()) {
            Token pt = iter.next();

            if (pt.isOperand()) { //Operands are output immediately
                rpnExpr.add(pt);
            } else if (pt.isFunction() || isParamDelimiter(pt) || isOpenBrace(pt)) { //Extract parameters after afunction or comma
                evalStack.push(pt);
                if (pt.isFunction()) {
                    iter.next();
                }
                ArrayList<Token> param = extractParameter(iter);
                Debug.log(Debug.TRACE, "Extracted parameter " + param);
                rpnExpr.addAll(infix2RPN(param));
            } else if (isCloseBrace(pt)) { //Pop off stack till you meet a function or an open bracket
                Token tmpTok = null;
                boolean bPop = true;
                while (bPop) {
                    if (evalStack.isEmpty()) {
                        bPop = false;
                    } else {
                        tmpTok = evalStack.pop();
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
                            (evalStack.peek().getTokenPriority() >=pt.getTokenPriority())) {
                        Token topTok = evalStack.peek();
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
            Token topTok = evalStack.peek();
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
    protected ArrayList<Token> extractParameter(ListIterator<Token> iter) {
        ArrayList<Token> param = new ArrayList<Token>(5);
        int subExprCount = 0;

        while (iter.hasNext()) {
            Token pt = iter.next();
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
     * @param   pt  The operator token
     * @param   args    The arguments for this operator
     * @return  A correctly ordered expression
     */
    protected ArrayList<Token> makeExpression(Token pt, Stack<ArrayList<Token>> args) {
        ArrayList<Token> tmp = new ArrayList<Token>(5);
        TokenFactory tf = new TokenFactory();
        if (pt.isOperator()) {
            if (pt.getNumArgs()==2) { //Binary operator
                tmp.addAll(args.pop());
                tmp.add(pt);
                tmp.addAll(args.pop());
            } else if (pt.getNumArgs() == 1) {
                if(isPercent(pt)) {
                    tmp.addAll(args.elementAt(0));
                    tmp.add(pt);
                } else {
                    tmp.add(pt);
                    tmp.addAll(args.elementAt(0));
                }
                if (isOpenBrace(pt)) {
                    tmp.add(tf.getOperatorToken(")",1));
                }
            }
        } else if (pt.isFunction()) {
            tmp.add(pt);
            tmp.add(tf.getOperatorToken("(",1));
            if (!args.isEmpty()) {
                ArrayList<Token> v = args.pop();
                tmp.addAll(v);
            }
            while (!args.isEmpty()) {
                tmp.add(tf.getOperatorToken(",",1));
                ArrayList<Token> v = args.pop();
                tmp.addAll(v);

            }
            tmp.add(tf.getOperatorToken(")",1));
        }

        return tmp;
    }
}
