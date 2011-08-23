/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/**
 * @file
 *  For LWP filter architecture prototype - table object
 */
/*************************************************************************
 * Change History
 Mar 2005			Created
 ************************************************************************/
#ifndef _LWPTBLFORMULA_HXX_
#define _LWPTBLFORMULA_HXX_

using namespace std;

/* These token types are written to the file.  Don't change their
values unless you filter them.
*/
enum lTokenType
{
    TK_BAD					= 0,
    TK_OPERAND				= 1,
    TK_END					= 2,
    TK_RIGHTPAREN			= 3,
    TK_FUNCTION				= 4,
    TK_LEFTPAREN			= 5,
    TK_UNARY_MINUS			= 6,
    TK_ADD					= 7,
    TK_SUBTRACT				= 8,
    TK_MULTIPLY				= 9,
    TK_DIVIDE				= 10,
    TK_EQUAL				= 11,
    TK_LESS					= 12,
    TK_GREATER				= 13,
    TK_NOT_EQUAL			= 14,
    TK_GREATER_OR_EQUAL		= 15,
    TK_LESS_OR_EQUAL		= 16,
    TK_NOT					= 17,
    TK_AND					= 18,
    TK_OR					= 19,
    TK_CELLID				= 20,
    TK_CONSTANT				= 21,
    TK_TEXT					= 22,
    TK_SUM					= 23,
    TK_IF					= 24,
    TK_AVERAGE				= 25,
    TK_MAXIMUM				= 26,
    TK_MINIMUM				= 27,
    TK_COUNT				= 28,
    TK_CELLRANGE			= 29,
    TK_EXPRESSION			= 30,
    TK_OPEN_FUNCTION		= 31,
    TK_LIST_SEPARATOR		= 32
};
class LwpTableLayout;
class LwpFormulaArg
{
public:
    virtual String ToString(LwpTableLayout* pCellsMap)=0;
    virtual String ToArgString(LwpTableLayout* pCellsMap){ return ToString(pCellsMap);}
};

class LwpFormulaTools
{
public:
    static String GetName(sal_uInt16 nTokenType);
    static String GetCellAddr(sal_Int16 nRow, sal_Int16 nCol, LwpTableLayout* pCellsMap);
};

class LwpFormulaConst:public LwpFormulaArg
{
public:
    LwpFormulaConst( double dVal);
    virtual String ToString(LwpTableLayout* pCellsMap);
private:
    double m_dVal;
};

class LwpFormulaText:public LwpFormulaArg
{
public:
    LwpFormulaText( String aText);
    virtual String ToString(LwpTableLayout* /*pCellsMap*/){return m_aText;}
private:
    String m_aText;
};

class LwpFormulaCellAddr:public LwpFormulaArg
{
public:
    LwpFormulaCellAddr(sal_Int16 aCol, sal_Int16 aRow);

    sal_Int16 GetCol(){return m_aCol;}
    sal_Int16 GetRow(){return m_aRow;}

    virtual String ToString(LwpTableLayout* pCellsMap);
private:
    sal_Int16 m_aCol;
    sal_Int16 m_aRow;
};

class LwpFormulaCellRangeAddr:public LwpFormulaArg
{
public:
    LwpFormulaCellRangeAddr(sal_Int16 aStartCol, sal_Int16 aStartRow, sal_Int16 aEndCol, sal_Int16 aEndRow);

    virtual String ToString(LwpTableLayout* pCellsMap);
private:
    sal_Int16 m_aStartCol;
    sal_Int16 m_aStartRow;
    sal_Int16 m_aEndCol;
    sal_Int16 m_aEndRow;
};

class LwpFormulaFunc :public LwpFormulaArg
{
public:
    LwpFormulaFunc(sal_uInt16 nTokenType);
    virtual ~LwpFormulaFunc();

    void AddArg(LwpFormulaArg* pArg);

    virtual String ToString(LwpTableLayout* pCellsMap);
    String ToArgString(LwpTableLayout* pCellsMap);

protected:
    vector<LwpFormulaArg*> m_aArgs;
    sal_uInt16 m_nTokenType;
};

class LwpFormulaOp : public LwpFormulaFunc
{
public:
    LwpFormulaOp(sal_uInt16 nTokenType):LwpFormulaFunc(nTokenType){;}
    String ToString(LwpTableLayout* pCellsMap);
};

class LwpFormulaUnaryOp : public LwpFormulaFunc
{
public:
    LwpFormulaUnaryOp(sal_uInt16 nTokenType):LwpFormulaFunc(nTokenType){;}
    String ToString(LwpTableLayout* pCellsMap);
};


class LwpFormulaInfo : public LwpCellList
{
public:
    LwpFormulaInfo(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpFormulaInfo();
    sal_Bool IsFormula(){return sal_True;}
    void SetRow(USHORT nRow){ m_nFormulaRow = nRow;}
    String Convert(LwpTableLayout* pCellsMap);
    void Convert(XFCell * pCell, LwpTableLayout* pCellsMap);
protected:
    void Read();
    sal_Bool ReadCellID();
    sal_Bool ReadText();
    sal_Bool ReadCellRange();
    sal_Bool ReadExpression();
    sal_Bool ReadArguments(LwpFormulaFunc& aFunc);
    sal_Bool m_bSupported;
private:
    vector<LwpFormulaArg*> m_aStack;
    sal_Bool ReadConst();
    void MarkUnsupported(sal_uInt16 TokenType);

    USHORT m_nFormulaRow;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
