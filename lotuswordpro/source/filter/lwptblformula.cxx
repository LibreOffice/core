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
/*************************************************************************
 * Change History
 Mar 2005           Created
 ************************************************************************/

#include "lwpoverride.hxx"
#include "lwptblcell.hxx"
#include "lwppara.hxx"
#include "lwptblformula.hxx"

#include "lwptablelayout.hxx"
#include <rtl/ustrbuf.hxx>
#include <boost/scoped_array.hpp>

LwpFormulaArg::~LwpFormulaArg()
{
}

//////////////////////////////////////////////////////////////////
 LwpFormulaInfo::LwpFormulaInfo(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpCellList(objHdr, pStrm)
    , m_bSupported(sal_True)
    , m_nFormulaRow(0)
{}

LwpFormulaInfo::~LwpFormulaInfo()
{
    try{
        while(m_aStack.size()>0)
        {
            LwpFormulaArg* pArg=m_aStack.back();
            m_aStack.pop_back();
            delete pArg; pArg=NULL;
        }
    }catch (...)
    {
        assert(false);
    }
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
sal_Bool LwpFormulaInfo::ReadConst()
{
    double Constant = m_pObjStrm->QuickReadDouble();

    m_aStack.push_back( new LwpFormulaConst(Constant) );

    return sal_True;
}
/**
*   Need more effort for unicode.
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
sal_Bool LwpFormulaInfo::ReadText()
{
    m_pObjStrm->QuickReadInt16(); //Disk Size
    sal_uInt16 nStrLen = m_pObjStrm->QuickReadInt16();

    boost::scoped_array<char> pBuf(new char[nStrLen+1]);
    m_pObjStrm->QuickRead( pBuf.get(), nStrLen );
    *(pBuf.get()+nStrLen)='\0';
    String aText;
    aText += OUString("\"");
    aText.Append(String(pBuf.get(),nStrLen,osl_getThreadTextEncoding()));
    aText += OUString("\"");

    m_aStack.push_back(new LwpFormulaText(aText));
    return sal_True;
}
/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
sal_Bool LwpFormulaInfo::ReadCellID()
{
    LwpRowSpecifier RowSpecifier;
    LwpColumnSpecifier ColumnSpecifier;
    sal_Bool readSucceeded = true;

    RowSpecifier.QuickRead(m_pObjStrm);
    ColumnSpecifier.QuickRead(m_pObjStrm);

    m_aStack.push_back( new LwpFormulaCellAddr(ColumnSpecifier.ColumnID(cColumn),
                                                RowSpecifier.RowID(m_nFormulaRow)) );
    return readSucceeded;
}
/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
sal_Bool LwpFormulaInfo::ReadCellRange()
{
    sal_Bool readSucceeded = sal_True;
    if (!ReadCellID( )) // start
        readSucceeded = false;
    LwpFormulaCellAddr* pStartCellAddr = (LwpFormulaCellAddr*)m_aStack.back();
    m_aStack.pop_back();

    if (!ReadCellID()) // end
        readSucceeded = false;
    LwpFormulaCellAddr* pEndCellAddr = (LwpFormulaCellAddr*)m_aStack.back();
    m_aStack.pop_back();

    m_aStack.push_back( new LwpFormulaCellRangeAddr(pStartCellAddr->GetCol(),
                                                    pStartCellAddr->GetRow(),
                                                    pEndCellAddr->GetCol(),
                                                    pEndCellAddr->GetRow()) );
    delete pStartCellAddr;
    delete pEndCellAddr;

    return readSucceeded;
}

/**
*   Read expression from wordpro file
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
sal_Bool LwpFormulaInfo::ReadExpression()
{
    sal_uInt16 TokenType, DiskLength;
    sal_Bool readSucceeded = sal_True;

    /* Read the compiled expression length */
//  Len = m_pObjStrm->QuickReaduInt16();
    m_pObjStrm->SeekRel(2);

    while ((TokenType = m_pObjStrm->QuickReaduInt16()) != TK_END)
    {
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
                if (!ReadCellID())
                    readSucceeded = sal_False;
                break;

            case TK_CELLRANGE:
                readSucceeded = ReadCellRange();
                break;

            case TK_SUM:
            case TK_IF:
            case TK_COUNT:
            case TK_MINIMUM:
            case TK_MAXIMUM:
            case TK_AVERAGE:
                {
                    LwpFormulaFunc* pFunc = new LwpFormulaFunc(TokenType);
                    if (!ReadArguments(*pFunc))
                        readSucceeded = sal_False;
                    m_aStack.push_back(pFunc);
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

                {//binary operator
                    LwpFormulaOp* pOp = new LwpFormulaOp(TokenType);
                    pOp->AddArg(m_aStack.back()); m_aStack.pop_back();
                    pOp->AddArg(m_aStack.back()); m_aStack.pop_back();
                    m_aStack.push_back(pOp);
                }
                break;
            case TK_UNARY_MINUS:
                {
                    LwpFormulaUnaryOp* pOp = new LwpFormulaUnaryOp(TokenType);
                    pOp->AddArg(m_aStack.back()); m_aStack.pop_back();
                    m_aStack.push_back(pOp);
                }
                break;
            default:
                // We don't know what to do with this token, so eat it.
                m_pObjStrm->SeekRel(DiskLength);
                readSucceeded = sal_False;
                break;
        }
        MarkUnsupported(TokenType);
    }
    return readSucceeded;
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return
*/
void LwpFormulaInfo::MarkUnsupported(sal_uInt16 TokenType)
{
    switch(TokenType)
    {
    case TK_IF:
    case TK_COUNT:
    case TK_NOT:
        {
            m_bSupported = sal_False;//Not supported formulas
        }
        break;
    default:
        break;
    }
}
/**
*   Read arguments of functions from wordpro file
*   @date   03/26/2005
*   @param  LwpFormulaFunc& aFunc, functions object
*   @return sal_Bool.
*/
sal_Bool LwpFormulaInfo::ReadArguments(LwpFormulaFunc& aFunc)
{
    sal_uInt16 NumberOfArguments = m_pObjStrm->QuickReaduInt16();
    sal_uInt16 ArgumentDiskLength, Count;
    sal_uInt8 ArgumentType;
    sal_Bool bArgument = sal_False;
    sal_Bool readSucceeded = sal_True;

    for (Count = 0; Count < NumberOfArguments; Count++)
    {
        ArgumentType = (sal_uInt8) m_pObjStrm->QuickReaduInt16(); // written as lushort
        ArgumentDiskLength = m_pObjStrm->QuickReaduInt16();
        bArgument = sal_True;

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
                bArgument = sal_False;
                m_pObjStrm->SeekRel(ArgumentDiskLength);
                readSucceeded = sal_False;
                break;
        }

        if (bArgument)
        {
            aFunc.AddArg( m_aStack.back() );
            m_aStack.pop_back();
        }
    }
    return readSucceeded;
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return
*/
void LwpFormulaInfo::Read()
{
    LwpCellList::Read();
    {
        LwpRowList* pRowList =(LwpRowList*)cParent.obj();
        if (pRowList)
        {
            m_nFormulaRow = pRowList->GetRowID();
        }
        else
        {
            assert(false);
        }
    }
//  sal_uInt8 cFlags = (sal_uInt8) m_pObjStrm->QuickReaduInt16(); // written as a sal_uInt16
    m_pObjStrm->SeekRel(2);//flags, size in file: sal_uInt16

    LwpNotifyListPersistent cNotifyList;
    cNotifyList.Read(m_pObjStrm);

    ReadExpression();

    m_pObjStrm->SkipExtra();
}

/**
*   Make the formula string.
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
String  LwpFormulaInfo::Convert(LwpTableLayout* pCellsMap)
{
    String aFormula;
    if (m_bSupported)
    {
        if(1==m_aStack.size())
        {
            LwpFormulaArg* pFormula = m_aStack.back();
            aFormula = pFormula->ToString(pCellsMap);
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
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
void LwpFormulaInfo::Convert(XFCell * pCell,LwpTableLayout* pCellsMap)
{
    String aFormula;
    aFormula = Convert(pCellsMap);
    if (aFormula.Len())
    {
        pCell->SetFormula(aFormula);
    }
    LwpCellList::Convert(pCell);
}

//////////////////////////////////////////////////////////////////

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
LwpFormulaConst::LwpFormulaConst(double dVal)
{
    m_dVal = dVal;
}

OUString LwpFormulaConst::ToString(LwpTableLayout* /*pCellsMap*/)
{
    return OUString::number(m_dVal);
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return
*/
LwpFormulaText::LwpFormulaText( String aText)
{
    m_aText = aText;
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return
*/
LwpFormulaCellAddr::LwpFormulaCellAddr(sal_Int16 aCol, sal_Int16 aRow)
{
    m_aCol = aCol;
    m_aRow = aRow;
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return String
*/
OUString LwpFormulaCellAddr::ToString(LwpTableLayout* pCellsMap)
{
    OUString aCellAddr = "<" + LwpFormulaTools::GetCellAddr(m_aRow,m_aCol,pCellsMap) + ">";
    return aCellAddr;
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return
*/
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
*   @date   03/26/2005
*   @param
*   @param
*   @return String.
*/
OUString LwpFormulaCellRangeAddr::ToString(LwpTableLayout* pCellsMap)
{
    OUString aCellAddr = "<"
        + LwpFormulaTools::GetCellAddr(m_aStartRow,m_aStartCol,pCellsMap) + ":"
        + LwpFormulaTools::GetCellAddr(m_aEndRow,m_aEndCol,pCellsMap) + ">";

    return aCellAddr;
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return
*/
LwpFormulaFunc::LwpFormulaFunc(sal_uInt16 nTokenType)
{
    m_nTokenType = nTokenType;
}

/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return
*/
LwpFormulaFunc::~LwpFormulaFunc()
{
    try
    {
        while(m_aArgs.size()>0)
        {
            LwpFormulaArg* pArg = m_aArgs.back();
            m_aArgs.pop_back();
            delete pArg;pArg=NULL;
        }
    }catch (...) {
        assert(false);
    }

}
/**
*
*   @date   03/26/2005
*   @param
*   @param
*   @return
*/
void LwpFormulaFunc::AddArg(LwpFormulaArg* pArg)
{
    m_aArgs.push_back(pArg);
}
/**
*   Convert the functions to a string, which is a argument of other formula
*   @date   03/26/2005
*   @param
*   @param
*   @return  String.
*/
String LwpFormulaFunc::ToArgString(LwpTableLayout* pCellsMap)
{
    OUStringBuffer aFormula;
    aFormula.append(static_cast<sal_Unicode>('('));
    aFormula.append(ToString(pCellsMap));
    aFormula.append(static_cast<sal_Unicode>(')'));
    return aFormula.makeStringAndClear();
}
/**
*   Convert the function to a formula string.
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
OUString LwpFormulaFunc::ToString(LwpTableLayout* pCellsMap)
{
    OUString aFormula;

    OUString aFuncName = LwpFormulaTools::GetName(m_nTokenType);
    aFormula += aFuncName;
    aFormula += " ";//Append a blank space

    //Append args
    vector<LwpFormulaArg*>::iterator aItr;
    for (aItr=m_aArgs.begin();aItr!=m_aArgs.end();++aItr)
    {
        aFormula += (*aItr)->ToArgString(pCellsMap) + "|"; //separator
    }

    //erase the last "|"
    if (!m_aArgs.empty())
    {
        aFormula = aFormula.replaceAt(aFormula.getLength()-1,1,"");
    }
    else
    {
        assert(false);
    }

    return aFormula;
}

/**
*   Convert the formula in operators to a string : e.g. 1+2+3
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
OUString LwpFormulaOp::ToString(LwpTableLayout* pCellsMap)
{
    OUString aFormula;
    if (2==m_aArgs.size())
    {
        vector<LwpFormulaArg*>::iterator aItr = m_aArgs.end();
        --aItr;

        aFormula += (*aItr)->ToArgString(pCellsMap) + " ";
        OUString aFuncName = LwpFormulaTools::GetName(m_nTokenType);

        aFormula += aFuncName + " ";

        --aItr;
        aFormula += (*aItr)->ToArgString(pCellsMap);
    }
    else
    {
        assert(false);
    }
    return aFormula;
}

/**
*   convert the formula in unary operators into string : e.g. -2
*   @date   03/26/2005
*   @param
*   @param
*   @return sal_Bool.
*/
OUString LwpFormulaUnaryOp::ToString(LwpTableLayout* pCellsMap)
{
    String aFormula;
    if (1==m_aArgs.size())
    {
        String aFuncName = LwpFormulaTools::GetName(m_nTokenType);
        aFormula.Append(aFuncName);

        vector<LwpFormulaArg*>::iterator aItr = m_aArgs.begin();
        aFormula.Append( (*aItr)->ToArgString(pCellsMap) );
    }
    else
    {
        assert(false);
    }
    return aFormula;
}
/**
*   Get token name
*   @date   03/26/2005
*   @param
*   @param
*   @return String.
*/
String LwpFormulaTools::GetName(sal_uInt16 nTokenType)
{
    String aName;
    switch(nTokenType)
    {
    case TK_SUM:
        aName = OUString("SUM");
        break;
    case TK_IF:
        aName = OUString("IF");//Not supported by SODC
        break;
    case TK_COUNT:
        aName = OUString("COUNT");//Not supported by SODC
        break;
    case TK_MINIMUM:
        aName = OUString("MIN");
        break;
    case TK_MAXIMUM:
        aName = OUString("MAX");
        break;
    case TK_AVERAGE:
        aName = OUString("MEAN");
        break;
    case TK_ADD:
        aName = OUString("+");
        break;
    case TK_SUBTRACT:
        aName = OUString("-");
        break;
    case TK_MULTIPLY:
        aName = OUString("*");
        break;
    case TK_DIVIDE:
        aName = OUString("/");
        break;
    case TK_UNARY_MINUS:
        aName = OUString("-");
        break;
    case TK_LESS:
        aName = OUString("L");
        break;
    case TK_LESS_OR_EQUAL:
        aName = OUString("LEQ");
        break;
    case TK_GREATER:
        aName = OUString("G");
        break;
    case TK_GREATER_OR_EQUAL:
        aName = OUString("GEQ");
        break;
    case TK_EQUAL:
        aName = OUString("EQ");
        break;
    case TK_NOT_EQUAL:
        aName = OUString("NEQ");
        break;
    case TK_NOT:
        aName = OUString("NOT");
        break;
    case TK_AND:
        aName = OUString("AND");
        break;
    case TK_OR:
        aName = OUString("OR");
        break;
    default:
        assert(false);
        break;
    }
    return aName;
}

/**
*   Get cell address in String
*   @date   03/26/2005
*   @param
*   @param
*   @return String.
*/
String LwpFormulaTools::GetCellAddr(sal_Int16 nRow, sal_Int16 nCol, LwpTableLayout* pCellsMap)
{
    String aCellAddr;
    XFCell* pCell = pCellsMap->GetCellsMap(nRow,(sal_uInt8)nCol);
    if (pCell)
    {
        aCellAddr = pCell->GetCellName();
    }
    else
    {
        assert( -1==nRow || -1==(sal_Int8)nCol);
    }
    return aCellAddr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
