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
 *  For LWP filter architecture prototype - table cell numerics format
 */

#include <lwpoverride.hxx>
#include "lwptblcell.hxx"
#include "lwppara.hxx"
#include "lwptblformula.hxx"

#include "lwptablelayout.hxx"
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <memory>

LwpFormulaArg::~LwpFormulaArg()
{
}

 LwpFormulaInfo::LwpFormulaInfo(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpCellList(objHdr, pStrm)
    , m_bSupported(true)
    , m_nFormulaRow(0)
{}

LwpFormulaInfo::~LwpFormulaInfo()
{
}

void LwpFormulaInfo::ReadConst()
{
    double Constant = m_pObjStrm->QuickReadDouble();

    m_aStack.push_back( std::make_unique<LwpFormulaConst>(Constant) );
}

/**
*   Need more effort for unicode.
*/
void LwpFormulaInfo::ReadText()
{
    m_pObjStrm->QuickReadInt16(); //Disk Size
    sal_uInt16 nStrLen = m_pObjStrm->QuickReadInt16();

    std::vector<char> aBuf(nStrLen + 1);
    m_pObjStrm->QuickRead(aBuf.data(), nStrLen);
    aBuf[nStrLen]= '\0';
    OUString aText;
    aText += "\"";
    aText += OUString(aBuf.data(), nStrLen, osl_getThreadTextEncoding());
    aText += "\"";

    m_aStack.push_back(std::make_unique<LwpFormulaText>(aText));
}

void LwpFormulaInfo::ReadCellID()
{
    LwpRowSpecifier RowSpecifier;
    LwpColumnSpecifier ColumnSpecifier;

    RowSpecifier.QuickRead(m_pObjStrm.get());
    ColumnSpecifier.QuickRead(m_pObjStrm.get());

    m_aStack.push_back( std::make_unique<LwpFormulaCellAddr>(ColumnSpecifier.ColumnID(cColumn),
                                                RowSpecifier.RowID(m_nFormulaRow)) );
}

void LwpFormulaInfo::ReadCellRange()
{
    ReadCellID( ); // start
    std::unique_ptr<LwpFormulaCellAddr> pStartCellAddr( static_cast<LwpFormulaCellAddr*>(m_aStack.back().release()));
    m_aStack.pop_back();

    ReadCellID(); // end
    std::unique_ptr<LwpFormulaCellAddr> pEndCellAddr(static_cast<LwpFormulaCellAddr*>(m_aStack.back().release()));
    m_aStack.pop_back();

    m_aStack.push_back( std::make_unique<LwpFormulaCellRangeAddr>(pStartCellAddr->GetCol(),
                                                    pStartCellAddr->GetRow(),
                                                    pEndCellAddr->GetCol(),
                                                    pEndCellAddr->GetRow()) );
}

/**
*   Read expression from wordpro file
*/
void LwpFormulaInfo::ReadExpression()
{
    sal_uInt16 TokenType, DiskLength;

    /* Read the compiled expression length */
    m_pObjStrm->SeekRel(2);

    bool bError = false;
    while ((TokenType = m_pObjStrm->QuickReaduInt16(&bError)) != TK_END)
    {

        if (bError)
            throw std::runtime_error("error reading expression");

        // Get the disk length of this token
        DiskLength = m_pObjStrm->QuickReaduInt16();

        switch (TokenType)
        {
            case TK_CONSTANT:
            {
                ReadConst();
                break;
            }

            case TK_CELLID:
                ReadCellID();
                break;

            case TK_CELLRANGE:
                ReadCellRange();
                break;

            case TK_SUM:
            case TK_IF:
            case TK_COUNT:
            case TK_MINIMUM:
            case TK_MAXIMUM:
            case TK_AVERAGE:
                {
                    std::unique_ptr<LwpFormulaFunc> xFunc(new LwpFormulaFunc(TokenType));
                    ReadArguments(*xFunc);
                    m_aStack.push_back(std::move(xFunc));
                }
                break;

            case TK_ADD://7
            case TK_SUBTRACT:
            case TK_MULTIPLY:
            case TK_DIVIDE:
            case TK_LESS:
            case TK_LESS_OR_EQUAL:
            case TK_GREATER:
            case TK_GREATER_OR_EQUAL:
            case TK_EQUAL:
            case TK_NOT_EQUAL:
            case TK_AND:
            case TK_OR:
            case TK_NOT:
                m_pObjStrm->SeekRel(DiskLength); // extensible for future

                if (m_aStack.size() >= 2)
                {//binary operator
                    std::unique_ptr<LwpFormulaOp> pOp(new LwpFormulaOp(TokenType));
                    pOp->AddArg(std::move(m_aStack.back())); m_aStack.pop_back();
                    pOp->AddArg(std::move(m_aStack.back())); m_aStack.pop_back();
                    m_aStack.push_back(std::move(pOp));
                }
                break;
            case TK_UNARY_MINUS:
                if (!m_aStack.empty())
                {
                    std::unique_ptr<LwpFormulaUnaryOp> pOp(new LwpFormulaUnaryOp(TokenType));
                    pOp->AddArg(std::move(m_aStack.back())); m_aStack.pop_back();
                    m_aStack.push_back(std::move(pOp));
                }
                break;
            default:
                // We don't know what to do with this token, so eat it.
                m_pObjStrm->SeekRel(DiskLength);
                break;
        }
        MarkUnsupported(TokenType);
    }
}

void LwpFormulaInfo::MarkUnsupported(sal_uInt16 TokenType)
{
    switch(TokenType)
    {
    case TK_IF:
    case TK_COUNT:
    case TK_NOT:
        {
            m_bSupported = false;//Not supported formulas
        }
        break;
    default:
        break;
    }
}
/**
*   Read arguments of functions from wordpro file
*   @param  LwpFormulaFunc& aFunc, functions object
*/
void LwpFormulaInfo::ReadArguments(LwpFormulaFunc& aFunc)
{
    sal_uInt16 NumberOfArguments = m_pObjStrm->QuickReaduInt16();

    for (sal_uInt16 Count = 0; Count < NumberOfArguments; Count++)
    {
        sal_uInt8 ArgumentType = static_cast<sal_uInt8>(m_pObjStrm->QuickReaduInt16()); // written as lushort
        sal_uInt16 ArgumentDiskLength = m_pObjStrm->QuickReaduInt16();
        bool bArgument = true;

        switch(ArgumentType)
        {
            case TK_CELLID:
                ReadCellID();
                break;

            case TK_CELLRANGE:
                ReadCellRange();
                break;

            case TK_CONSTANT:
                ReadConst();
                break;

            case TK_TEXT:
                ReadText();
                break;

            case TK_EXPRESSION:
                ReadExpression();
                break;

            default:
                bArgument = false;
                m_pObjStrm->SeekRel(ArgumentDiskLength);
                break;
        }

        if (bArgument && !m_aStack.empty())
        {
            aFunc.AddArg(std::move(m_aStack.back()));
            m_aStack.pop_back();
        }
    }
}

void LwpFormulaInfo::Read()
{
    LwpCellList::Read();
    {
        LwpRowList* pRowList = dynamic_cast<LwpRowList*>(cParent.obj().get());
        if (pRowList)
        {
            m_nFormulaRow = pRowList->GetRowID();
        }
        else
        {
            SAL_WARN("lwp", "missing row list");
        }
    }
    m_pObjStrm->SeekRel(2);//flags, size in file: sal_uInt16

    LwpNotifyListPersistent cNotifyList;
    cNotifyList.Read(m_pObjStrm.get());

    ReadExpression();

    m_pObjStrm->SkipExtra();
}

/**
*   Make the formula string.
*/
OUString  LwpFormulaInfo::Convert(LwpTableLayout* pCellsMap)
{
    OUString aFormula;
    if (m_bSupported)
    {
        if(1==m_aStack.size())
        {
            aFormula = m_aStack[0]->ToString(pCellsMap);
        }
        else
        {
            assert(false);
        }
    }
    return aFormula;
}

/**
*   Fill the XFCell content
*/
void LwpFormulaInfo::Convert(XFCell * pCell,LwpTableLayout* pCellsMap)
{
    OUString aFormula;
    aFormula = Convert(pCellsMap);
    if (!aFormula.isEmpty())
    {
        pCell->SetFormula(aFormula);
    }
    LwpCellList::Convert(pCell);
}

LwpFormulaConst::LwpFormulaConst(double dVal)
{
    m_dVal = dVal;
}

OUString LwpFormulaConst::ToString(LwpTableLayout* /*pCellsMap*/)
{
    return OUString::number(m_dVal);
}

LwpFormulaText::LwpFormulaText( const OUString& aText)
{
    m_aText = aText;
}

LwpFormulaCellAddr::LwpFormulaCellAddr(sal_Int16 aCol, sal_Int16 aRow)
{
    m_aCol = aCol;
    m_aRow = aRow;
}

OUString LwpFormulaCellAddr::ToString(LwpTableLayout* pCellsMap)
{
    OUString aCellAddr = "<" + LwpFormulaTools::GetCellAddr(m_aRow,m_aCol,pCellsMap) + ">";
    return aCellAddr;
}

LwpFormulaCellRangeAddr::LwpFormulaCellRangeAddr(sal_Int16 aStartCol,
                                                 sal_Int16 aStartRow,
                                                 sal_Int16 aEndCol,
                                                 sal_Int16 aEndRow)
{
    m_aStartCol = aStartCol;
    m_aStartRow = aStartRow;
    m_aEndCol = aEndCol;
    m_aEndRow = aEndRow;
}

/**
*   Convert the cell range into a string
*/
OUString LwpFormulaCellRangeAddr::ToString(LwpTableLayout* pCellsMap)
{
    OUString aCellAddr = "<"
        + LwpFormulaTools::GetCellAddr(m_aStartRow,m_aStartCol,pCellsMap) + ":"
        + LwpFormulaTools::GetCellAddr(m_aEndRow,m_aEndCol,pCellsMap) + ">";

    return aCellAddr;
}

LwpFormulaFunc::LwpFormulaFunc(sal_uInt16 nTokenType)
{
    m_nTokenType = nTokenType;
}

LwpFormulaFunc::~LwpFormulaFunc()
{
}
void LwpFormulaFunc::AddArg(std::unique_ptr<LwpFormulaArg> pArg)
{
    m_aArgs.push_back(std::move(pArg));
}
/**
*   Convert the functions to a string, which is a argument of other formula
*/
OUString LwpFormulaFunc::ToArgString(LwpTableLayout* pCellsMap)
{
    OUStringBuffer aFormula;
    aFormula.append('(');
    aFormula.append(ToString(pCellsMap));
    aFormula.append(')');
    return aFormula.makeStringAndClear();
}
/**
*   Convert the function to a formula string.
*/
OUString LwpFormulaFunc::ToString(LwpTableLayout* pCellsMap)
{
    OUStringBuffer aFormula;

    OUString aFuncName = LwpFormulaTools::GetName(m_nTokenType);
    aFormula.append(aFuncName);
    aFormula.append(" ");//Append a blank space

    //Append args
    for (auto const& elem : m_aArgs)
    {
        aFormula.append(elem->ToArgString(pCellsMap)).append("|"); //separator
    }

    //erase the last "|"
    if (!m_aArgs.empty())
    {
        aFormula.setLength(aFormula.getLength()-1);
    }
    else
    {
        assert(false);
    }

    return aFormula.makeStringAndClear();
}

/**
*   Convert the formula in operators to a string : e.g. 1+2+3
*/
OUString LwpFormulaOp::ToString(LwpTableLayout* pCellsMap)
{
    OUString aFormula;
    if (2==m_aArgs.size())
    {
        aFormula += m_aArgs[1]->ToArgString(pCellsMap) + " ";
        OUString aFuncName = LwpFormulaTools::GetName(m_nTokenType);

        aFormula += aFuncName + " ";

        aFormula += m_aArgs[0]->ToArgString(pCellsMap);
    }
    else
    {
        assert(false);
    }
    return aFormula;
}

/**
*   convert the formula in unary operators into string : e.g. -2
*/
OUString LwpFormulaUnaryOp::ToString(LwpTableLayout* pCellsMap)
{
    OUString aFormula;
    if (1==m_aArgs.size())
    {
        OUString aFuncName = LwpFormulaTools::GetName(m_nTokenType);
        aFormula += aFuncName;
        aFormula += m_aArgs[0]->ToArgString(pCellsMap);
    }
    else
    {
        assert(false);
    }
    return aFormula;
}
/**
*   Get token name
*/
OUString LwpFormulaTools::GetName(sal_uInt16 nTokenType)
{
    OUString aName;
    switch(nTokenType)
    {
    case TK_SUM:
        aName = "SUM";
        break;
    case TK_IF:
        aName = "IF";//Not supported by SODC
        break;
    case TK_COUNT:
        aName = "COUNT";//Not supported by SODC
        break;
    case TK_MINIMUM:
        aName = "MIN";
        break;
    case TK_MAXIMUM:
        aName = "MAX";
        break;
    case TK_AVERAGE:
        aName = "MEAN";
        break;
    case TK_ADD:
        aName = "+";
        break;
    case TK_SUBTRACT:
        aName = "-";
        break;
    case TK_MULTIPLY:
        aName = "*";
        break;
    case TK_DIVIDE:
        aName = "/";
        break;
    case TK_UNARY_MINUS:
        aName = "-";
        break;
    case TK_LESS:
        aName = "L";
        break;
    case TK_LESS_OR_EQUAL:
        aName = "LEQ";
        break;
    case TK_GREATER:
        aName = "G";
        break;
    case TK_GREATER_OR_EQUAL:
        aName = "GEQ";
        break;
    case TK_EQUAL:
        aName = "EQ";
        break;
    case TK_NOT_EQUAL:
        aName = "NEQ";
        break;
    case TK_NOT:
        aName = "NOT";
        break;
    case TK_AND:
        aName = "AND";
        break;
    case TK_OR:
        aName = "OR";
        break;
    default:
        assert(false);
        break;
    }
    return aName;
}

/**
*   Get cell address in String
*/
OUString LwpFormulaTools::GetCellAddr(sal_Int16 nRow, sal_Int16 nCol, LwpTableLayout* pCellsMap)
{
    OUString aCellAddr;
    XFCell* pCell = pCellsMap->GetCellsMap(nRow,static_cast<sal_uInt8>(nCol));
    if (pCell)
    {
        aCellAddr = pCell->GetCellName();
    }
    else
    {
        assert( -1==nRow || -1==static_cast<sal_Int8>(nCol));
    }
    return aCellAddr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
