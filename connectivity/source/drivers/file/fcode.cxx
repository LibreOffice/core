/*************************************************************************
 *
 *  $RCSfile: fcode.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:26:20 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_FILE_FCODE_HXX_
#include "file/fcode.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include "connectivity/sqlparse.hxx"
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::file;
//using namespace ::com::sun::star::uno;
//using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
//using namespace ::com::sun::star::container;
//using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::sdbcx;

TYPEINIT0(OCode);
TYPEINIT1(OOperand, OCode);
TYPEINIT1(OOperandRow, OOperand);
TYPEINIT1(OOperandAttr, OOperandRow);
TYPEINIT1(OOperandParam, OOperandRow);
TYPEINIT1(OOperandValue, OOperand);
TYPEINIT1(OOperandConst, OOperandValue);
TYPEINIT1(OOperandResult, OOperandValue);
TYPEINIT1(OStopOperand, OOperandValue);

TYPEINIT1(OOperator, OCode);
TYPEINIT1(OBoolOperator,OOperator);
TYPEINIT1(OOp_AND, OBoolOperator);
TYPEINIT1(OOp_OR, OBoolOperator);
TYPEINIT1(OOp_ISNULL, OBoolOperator);
TYPEINIT1(OOp_ISNOTNULL, OOp_ISNULL);
TYPEINIT1(OOp_LIKE, OBoolOperator);
TYPEINIT1(OOp_NOTLIKE, OOp_LIKE);
TYPEINIT1(OOp_COMPARE, OBoolOperator);
TYPEINIT1(ONumOperator, OOperator);
TYPEINIT1(ONthOperator, OOperator);
TYPEINIT1(OBinaryOperator, OOperator);
TYPEINIT1(OUnaryOperator, OOperator);

//------------------------------------------------------------------
DBG_NAME(OCode )
OCode::OCode()
{
    DBG_CTOR(OCode ,NULL);
}
// -----------------------------------------------------------------------------
OCode::~OCode()
{
    DBG_DTOR(OCode,NULL);
}

//------------------------------------------------------------------
OEvaluateSet* OOperand::preProcess(OBoolOperator* pOp, OOperand* pRight)
{
    return NULL;
}
// -----------------------------------------------------------------------------
OOperandRow::OOperandRow(sal_uInt16 _nPos, sal_Int32 _rType)
    : OOperand(_rType)
    , m_nRowPos(_nPos)
{}
//------------------------------------------------------------------
void OOperandRow::bindValue(const OValueRefRow& _pRow)
{
    OSL_ENSURE(_pRow.isValid(),"NO EMPTY row allowed!");
    m_pRow = _pRow;
    OSL_ENSURE(m_pRow.isValid() && m_nRowPos < m_pRow->size(),"Invalid RowPos is >= vector.size()");
    (*m_pRow)[m_nRowPos]->setBound(sal_True);
}
// -----------------------------------------------------------------------------
void OOperandRow::setValue(const ORowSetValue& _rVal)
{
    OSL_ENSURE(m_pRow.isValid() && m_nRowPos < m_pRow->size(),"Invalid RowPos is >= vector.size()");
    (*(*m_pRow)[m_nRowPos]) = _rVal;
}
//------------------------------------------------------------------
const ORowSetValue& OOperandRow::getValue() const
{
    OSL_ENSURE(m_pRow.isValid() && m_nRowPos < m_pRow->size(),"Invalid RowPos is >= vector.size()");
    return (*m_pRow)[m_nRowPos]->getValue();
}

// -----------------------------------------------------------------------------
void OOperandValue::setValue(const ORowSetValue& _rVal)
{
    m_aValue = _rVal;
}
// -------------------------------------------------------------------------
sal_Bool OOperandAttr::isIndexed() const
{
    return sal_False;
}
//------------------------------------------------------------------
OOperandParam::OOperandParam(OSQLParseNode* pNode, sal_Int32 _nPos)
    : OOperandRow(_nPos, DataType::VARCHAR)      // Standard-Typ
{
    OSL_ENSURE(SQL_ISRULE(pNode,parameter),"Argument ist kein Parameter");
    OSL_ENSURE(pNode->count() > 0,"Fehler im Parse Tree");
    OSQLParseNode *pMark = pNode->getChild(0);

    String aParameterName;
    if (SQL_ISPUNCTUATION(pMark,"?"))
        aParameterName = '?';
    else if (SQL_ISPUNCTUATION(pMark,":"))
        aParameterName = pNode->getChild(1)->getTokenValue();
    else
    {
        OSL_ASSERT("Fehler im Parse Tree");
    }

    // Parameter-Column aufsetzen mit defult typ, kann zu einem späteren Zeitpunkt über DescribeParameter
    // genauer spezifiziert werden

    // Identitaet merken (hier eigentlich nicht erforderlich, aber aus
    // Symmetriegruenden ...)

    // todo
    //  OColumn* pColumn = new OFILEColumn(aParameterName,eDBType,255,0,SQL_FLAGS_NULLALLOWED);
    //  rParamColumns->AddColumn(pColumn);

    // der Wert wird erst kurz vor der Auswertung gesetzt
}


//------------------------------------------------------------------
const ORowSetValue& OOperandValue::getValue() const
{
    return m_aValue;
}

//------------------------------------------------------------------
OOperandConst::OOperandConst(const OSQLParseNode& rColumnRef, const rtl::OUString& aStrValue)
{
    switch (rColumnRef.getNodeType())
    {
        case SQL_NODE_STRING:
            m_aValue    = aStrValue;
            m_eDBType   = DataType::VARCHAR;
            m_aValue.setBound(sal_True);
            return;
        case SQL_NODE_INTNUM:
        case SQL_NODE_APPROXNUM:
        {
            m_aValue    = aStrValue.toDouble();
            m_eDBType   = DataType::DOUBLE;
            m_aValue.setBound(sal_True);
            return;
        }
    }

    if (SQL_ISTOKEN(&rColumnRef,TRUE))
    {
        m_aValue = 1.0;
        m_eDBType = DataType::BIT;
    }
    else if (SQL_ISTOKEN(&rColumnRef,FALSE))
    {
        m_aValue = 0.0;
        m_eDBType = DataType::BIT;
    }
    else
    {
        OSL_ASSERT("Parse Error");
    }
    m_aValue.setBound(sal_True);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Implementation of the operators

//------------------------------------------------------------------
sal_uInt16 OOperator::getRequestedOperands() const {return 2;}

//------------------------------------------------------------------
sal_Bool OBoolOperator::operate(const OOperand*, const OOperand*) const
{
    return sal_False;
}


//------------------------------------------------------------------
void OBoolOperator::Exec(OCodeStack& rCodeStack)
{
    OOperand  *pRight   = rCodeStack.top();
    rCodeStack.pop();
    OOperand  *pLeft    = rCodeStack.top();
    rCodeStack.pop();

    rCodeStack.push(new OOperandResultBOOL(operate(pLeft, pRight)));
    if (IS_TYPE(OOperandResult,pLeft))
        delete pLeft;
    if (IS_TYPE(OOperandResult,pRight))
        delete pRight;
}

//------------------------------------------------------------------
sal_Bool OOp_AND::operate(const OOperand* pLeft, const OOperand* pRight) const
{
    return pLeft->isValid() && pRight->isValid();
}

//------------------------------------------------------------------
sal_Bool OOp_OR::operate(const OOperand* pLeft, const OOperand* pRight) const
{
    return pLeft->isValid() || pRight->isValid();
}

//------------------------------------------------------------------
sal_uInt16 OOp_ISNULL::getRequestedOperands() const {return 1;}

//------------------------------------------------------------------
void OOp_ISNULL::Exec(OCodeStack& rCodeStack)
{
    OOperand* pOperand = rCodeStack.top();
    rCodeStack.pop();

    rCodeStack.push(new OOperandResultBOOL(operate(pOperand)));
    if (IS_TYPE(OOperandResult,pOperand))
        delete pOperand;
}

//------------------------------------------------------------------
sal_Bool OOp_ISNULL::operate(const OOperand* pOperand, const OOperand*) const
{
    return pOperand->getValue().isNull();
}

//------------------------------------------------------------------
sal_Bool OOp_ISNOTNULL::operate(const OOperand* pOperand, const OOperand*) const
{
    return !OOp_ISNULL::operate(pOperand);
}

//------------------------------------------------------------------
sal_Bool OOp_LIKE::operate(const OOperand* pLeft, const OOperand* pRight) const
{
    sal_Bool bMatch;
    ORowSetValue aLH(pLeft->getValue());
    ORowSetValue aRH(pRight->getValue());

    if (aLH.isNull() || aRH.isNull())
        bMatch = sal_False;
    else
    {
        bMatch = match(aRH.getString(), aLH.getString(), cEscape);
    }
    return bMatch;
}

//------------------------------------------------------------------
sal_Bool OOp_NOTLIKE::operate(const OOperand* pLeft, const OOperand* pRight) const
{
    return !OOp_LIKE::operate(pLeft, pRight);
}

//------------------------------------------------------------------
sal_Bool OOp_COMPARE::operate(const OOperand* pLeft, const OOperand* pRight) const
{
    ORowSetValue aLH(pLeft->getValue());
    ORowSetValue aRH(pRight->getValue());

    if (aLH.isNull() || aRH.isNull()) // if (!aLH.getValue() || !aRH.getValue())
        return sal_False;

    sal_Bool bResult = sal_False;
    sal_Int32 eDBType = pLeft->getDBType();

    // Vergleich (je nach Datentyp):
    switch (eDBType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        {
#if SUPD < 632
            static String sLanguage;
            static String sCountry;
            if (!sLanguage.Len())
                ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);

            static rtl::OLocale aLocale = rtl::OLocale::registerLocale(sLanguage, sCountry);
            INT32 nRes = compareIgnoreCase(aLH, aRH, aLocale);
#else
            INT32 nRes = rtl_ustr_compareIgnoreAsciiCase_WithLength
                (
                 static_cast<rtl::OUString>(aLH).pData->buffer,
                 static_cast<rtl::OUString>(aLH).pData->length,
                 static_cast<rtl::OUString>(aRH).pData->buffer,
                 static_cast<rtl::OUString>(aRH).pData->length );
#endif
            switch(aPredicateType)
            {
                case SQL_PRED_EQUAL:            bResult = (nRes == 0); break;
                case SQL_PRED_NOTEQUAL:         bResult = (nRes != 0); break;
                case SQL_PRED_LESS:             bResult = (nRes <  0); break;
                case SQL_PRED_LESSOREQUAL:      bResult = (nRes <= 0); break;
                case SQL_PRED_GREATER:          bResult = (nRes >  0); break;
                case SQL_PRED_GREATEROREQUAL:   bResult = (nRes >= 0); break;
                default:                        bResult = sal_False;
            }
        } break;
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
        case DataType::DECIMAL:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::BIT:
        case DataType::TIMESTAMP:
        case DataType::DATE:
        case DataType::TIME:
        {
            double n = aLH ,m = aRH;

            switch (aPredicateType)
            {
                case SQL_PRED_EQUAL:            bResult = (n == m); break;
                case SQL_PRED_LIKE:             bResult = (n == m); break;
                case SQL_PRED_NOTEQUAL:         bResult = (n != m); break;
                case SQL_PRED_NOTLIKE:          bResult = (n != m); break;
                case SQL_PRED_LESS:             bResult = (n < m); break;
                case SQL_PRED_LESSOREQUAL:      bResult = (n <= m); break;
                case SQL_PRED_GREATER:          bResult = (n > m); break;
                case SQL_PRED_GREATEROREQUAL:   bResult = (n >= m); break;
                default:                        bResult = sal_False;
            }
        } break;
        default:
            bResult = aLH == aRH;
    }
    return bResult;
}

//------------------------------------------------------------------
void ONumOperator::Exec(OCodeStack& rCodeStack)
{

    OOperand  *pRight   = rCodeStack.top();
    rCodeStack.pop();
    OOperand  *pLeft    = rCodeStack.top();
    rCodeStack.pop();

    rCodeStack.push(new OOperandResultNUM(operate(pLeft->getValue(), pRight->getValue())));
    if (IS_TYPE(OOperandResult,pLeft))
        delete pLeft;
    if (IS_TYPE(OOperandResult,pRight))
        delete pRight;
}
//------------------------------------------------------------------
double OOp_ADD::operate(const double& fLeft,const double& fRight) const
{
    return fLeft + fRight;
}

//------------------------------------------------------------------
double OOp_SUB::operate(const double& fLeft,const double& fRight) const
{
    return fLeft - fRight;
}

//------------------------------------------------------------------
double OOp_MUL::operate(const double& fLeft,const double& fRight) const
{
    return fLeft * fRight;
}

//------------------------------------------------------------------
double OOp_DIV::operate(const double& fLeft,const double& fRight) const
{
    return fLeft / fRight;
}
// -----------------------------------------------------------------------------
OEvaluateSet* OOperandAttr::preProcess(OBoolOperator* pOp, OOperand* pRight)
{
    return NULL;
}
//------------------------------------------------------------------
void ONthOperator::Exec(OCodeStack& rCodeStack)
{
    ::std::vector<ORowSetValue> aValues;
    ::std::vector<OOperand*> aOperands;
    OOperand* pOperand;
    do
    {
        OSL_ENSURE(!rCodeStack.empty(),"Stack must be none empty!");
        pOperand    = rCodeStack.top();
        rCodeStack.pop();
        if ( !IS_TYPE(OStopOperand,pOperand) )
            aValues.push_back( pOperand->getValue() );
        aOperands.push_back( pOperand );
    }
    while ( !IS_TYPE(OStopOperand,pOperand) );

    rCodeStack.push(new OOperandResult(operate(aValues)));

    ::std::vector<OOperand*>::iterator aIter = aOperands.begin();
    ::std::vector<OOperand*>::iterator aEnd = aOperands.end();
    for (; aIter != aEnd; ++aIter)
    {
        if (IS_TYPE(OOperandResult,*aIter))
            delete *aIter;
    }
}
//------------------------------------------------------------------
void OBinaryOperator::Exec(OCodeStack& rCodeStack)
{
    OOperand  *pRight   = rCodeStack.top();
    rCodeStack.pop();
    OOperand  *pLeft    = rCodeStack.top();
    rCodeStack.pop();

    if ( !rCodeStack.empty() && IS_TYPE(OStopOperand,rCodeStack.top()) )
        rCodeStack.pop();

    rCodeStack.push(new OOperandResult(operate(pLeft->getValue(),pRight->getValue())));
    if (IS_TYPE(OOperandResult,pRight))
        delete pRight;
    if (IS_TYPE(OOperandResult,pLeft))
        delete pLeft;
}
//------------------------------------------------------------------
void OUnaryOperator::Exec(OCodeStack& rCodeStack)
{
    OSL_ENSURE(!rCodeStack.empty(),"Stack is empty!");
    OOperand* pOperand = rCodeStack.top();
    rCodeStack.pop();

    rCodeStack.push(new OOperandResult(operate(pOperand->getValue())));
    if (IS_TYPE(OOperandResult,pOperand))
        delete pOperand;
}
// -----------------------------------------------------------------------------
sal_uInt16 OUnaryOperator::getRequestedOperands() const {return 1;}



