/*************************************************************************
 *
 *  $RCSfile: fcode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 08:48:24 $
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
#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

using namespace connectivity;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

TYPEINIT0(OCode);
TYPEINIT1(OOperand, OCode);
TYPEINIT1(OOperandRow, OOperand);
TYPEINIT1(OOperandAttr, OOperandRow);
TYPEINIT1(OFILEOperandAttr, OOperandAttr);
TYPEINIT1(OOperandParam, OOperandRow);
TYPEINIT1(OOperandValue, OOperand);
TYPEINIT1(OOperandConst, OOperandValue);
TYPEINIT1(OOperandResult, OOperandValue);

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

//------------------------------------------------------------------
sal_Int32 compareIgnoreCase(const rtl::OUString& rStr1, const rtl::OUString& rStr2, const ::rtl::OLocale& rLocale)
{
    rtl::OUString aString1 = rStr1.toUpperCase(rLocale);
    rtl::OUString aString2 = rStr2.toUpperCase(rLocale);
    return aString1.compareTo(aString2);
}
//------------------------------------------------------------------
OCode::~OCode(){}

//------------------------------------------------------------------
OEvaluateSet* OOperand::preProcess(OBoolOperator* pOp, OOperand* pRight)
{
    return NULL;
}

//------------------------------------------------------------------
void OOperandRow::bindValue(OValueRow _pRow)
{
    OSL_ENSHURE(_pRow.isValid(),"NO EMPTY row allowed!");
    m_pRow = _pRow;
}

//------------------------------------------------------------------
Any OOperandRow::getValue() const
{
    OSL_ENSHURE(m_pRow.isValid() && m_nRowPos < m_pRow->size(),"Invalid RowPos is >= vector.size()");
    return (*m_pRow)[m_nRowPos];
}
//------------------------------------------------------------------
OOperandAttr::OOperandAttr(sal_uInt16 _nPos,const Reference< XFastPropertySet>& _xColumn)
    : OOperandRow(_nPos,getINT32(_xColumn->getFastPropertyValue(PROPERTY_ID_TYPE)))
    , m_xColumn(_xColumn)
{
}
//------------------------------------------------------------------
OFILEOperandAttr::OFILEOperandAttr(sal_uInt16 _nPos,const Reference< XFastPropertySet>& _xColumn)
               :OOperandAttr(_nPos,_xColumn)
{
}

//------------------------------------------------------------------
OEvaluateSet* OFILEOperandAttr::preProcess(OBoolOperator* pOp, OOperand* pRight)
{
    OEvaluateSet* pEvaluateSet = NULL;
    if (isIndexed())
    {
        OSL_ENSHURE(0,"TODO: OFILEOperandAttr::preProcess");
//      OFILEIndexIterator* pIter = pCol->GetIndex()->CreateIterator(pOp,pRight);
//
//      if (pIter->Status().IsSuccessful())
//      {
//          pEvaluateSet = new OEvaluateSet();
//          ULONG nRec = pIter->First();
//          while (nRec != SQL_INDEX_ENTRY_NOTFOUND)
//          {
//              pEvaluateSet->Insert(nRec);
//              nRec = pIter->Next();
//          }
//      }
//      delete pIter;
    }
    return pEvaluateSet;
}

//------------------------------------------------------------------
OOperandParam::OOperandParam(OSQLParseNode* pNode, ::vos::ORef<connectivity::OSQLColumns> _xParamColumns)
    : OOperandRow(_xParamColumns->size(), DataType::VARCHAR)         // Standard-Typ
{
    OSL_ENSHURE(SQL_ISRULE(pNode,parameter),"Argument ist kein Parameter");
    OSL_ENSHURE(pNode->count() > 0,"Fehler im Parse Tree");
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
void OOperandParam::describe(const Reference< XFastPropertySet>& rColumn, ::vos::ORef<connectivity::OSQLColumns> rParameterColumns)
{
    // den alten namen beibehalten

    Reference< XFastPropertySet> xColumn = (*rParameterColumns)[getRowPos()];

    xColumn->setFastPropertyValue(PROPERTY_ID_TYPENAME,rColumn->getFastPropertyValue(PROPERTY_ID_TYPENAME));
    xColumn->setFastPropertyValue(PROPERTY_ID_DEFAULTVALUE,rColumn->getFastPropertyValue(PROPERTY_ID_DEFAULTVALUE));
    xColumn->setFastPropertyValue(PROPERTY_ID_PRECISION,rColumn->getFastPropertyValue(PROPERTY_ID_PRECISION));
    xColumn->setFastPropertyValue(PROPERTY_ID_TYPE,rColumn->getFastPropertyValue(PROPERTY_ID_TYPE));
    xColumn->setFastPropertyValue(PROPERTY_ID_SCALE,rColumn->getFastPropertyValue(PROPERTY_ID_SCALE));
    xColumn->setFastPropertyValue(PROPERTY_ID_ISNULLABLE,rColumn->getFastPropertyValue(PROPERTY_ID_ISNULLABLE));
    xColumn->setFastPropertyValue(PROPERTY_ID_ISAUTOINCREMENT,rColumn->getFastPropertyValue(PROPERTY_ID_ISAUTOINCREMENT));

    m_eDBType = getINT32(rColumn->getFastPropertyValue(PROPERTY_ID_TYPE));
}

//------------------------------------------------------------------
Any OOperandValue::getValue() const
{
    return m_aValue;
}

//------------------------------------------------------------------
OOperandConst::OOperandConst(const OSQLParseNode& rColumnRef, const rtl::OUString& aStrValue)
{
    switch (rColumnRef.getNodeType())
    {
        case SQL_NODE_STRING:
            m_aValue <<= aStrValue;
            m_eDBType = DataType::VARCHAR;
            return;
        case SQL_NODE_INTNUM:
        case SQL_NODE_APPROXNUM:
        {
            m_aValue <<= aStrValue.toDouble();

            m_eDBType = DataType::DOUBLE;
            return;
        }
    }

    if (SQL_ISTOKEN(&rColumnRef,TRUE))
    {
        m_aValue <<= 1.0;
        m_eDBType = DataType::BIT;
    }
    else if (SQL_ISTOKEN(&rColumnRef,FALSE))
    {
        m_aValue <<= 0.0;
        m_eDBType = DataType::BIT;
    }
    else
    {
        OSL_ASSERT("Parse Error");
    }
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
    if (IS_TYPE(OOperandResult,pLeft)) delete pLeft;
    if (IS_TYPE(OOperandResult,pRight)) delete pRight;
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
    if (IS_TYPE(OOperandResult,pOperand)) delete pOperand;
}

//------------------------------------------------------------------
sal_Bool OOp_ISNULL::operate(const OOperand* pOperand, const OOperand*) const
{
    Any aRet(pOperand->getValue());
    return !aRet.hasValue() || !aRet.getValue();
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
    Any aLH(pLeft->getValue());
    Any aRH(pRight->getValue());

    if (!aLH.hasValue() || !aRH.hasValue())
        bMatch = TRUE;
    else if (!aLH.getValue())
        bMatch = sal_False;
    else
    {
        bMatch = match(getString(aRH), getString(aLH), cEscape);
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
    Any aLH(pLeft->getValue());
    Any aRH(pRight->getValue());

//  if (!aLH.hasValue() || !aRH.hasValue())
//      return TRUE;

    if (!aLH.hasValue() || !aRH.hasValue()) // if (!aLH.getValue() || !aRH.getValue())
        return sal_False;

    sal_Bool bResult = sal_False;
    sal_Int32 eDBType = pLeft->getDBType();

    // Vergleich (je nach Datentyp):
    switch (eDBType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        {
            static String sLanguage;
            static String sCountry;
            if (!sLanguage.Len())
                ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);

            static rtl::OLocale aLocale = rtl::OLocale::registerLocale(sLanguage, sCountry);

            INT32 nRes = compareIgnoreCase(getString(aLH), getString(aRH), aLocale);
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
            double n,m;
            aLH >>= n;
            aRH >>= m;

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
            DBG_ERROR("OFILECursor::ExecuteRow: Vergleich mit diesem Datentyp nicht implementiert");
            bResult = sal_False;
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

    rCodeStack.push(new OOperandResultNUM(operate(getDouble(pLeft->getValue()), getDouble(pRight->getValue()))));
    if (IS_TYPE(OOperandResult,pLeft)) delete pLeft;
    if (IS_TYPE(OOperandResult,pRight)) delete pRight;
}
//------------------------------------------------------------------
double OOp_ADD::operate(double fLeft, double fRight) const {return fLeft + fRight;};

//------------------------------------------------------------------
double OOp_SUB::operate(double fLeft, double fRight) const {return fLeft - fRight;};

//------------------------------------------------------------------
double OOp_MUL::operate(double fLeft, double fRight) const {return fLeft * fRight;};

//------------------------------------------------------------------
double OOp_DIV::operate(double fLeft, double fRight) const {return fLeft / fRight;};



