/*************************************************************************
 *
 *  $RCSfile: fcode.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:28:52 $
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
#define _CONNECTIVITY_FILE_FCODE_HXX_

#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include "connectivity/sqliterator.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include "connectivity/FValue.hxx"
#endif


namespace connectivity
{
    class OSQLParseNode;
    namespace dbase
    {
        class ODbaseIndex;
    }
    namespace file
    {

        class OOperand;
        typedef ::std::stack<OOperand*> OCodeStack;
        class OBoolOperator;
        typedef ::std::map<sal_Int32,sal_Int32> OEvaluateSet;

        typedef ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> OFileColumns;


        class OCode
        {
        public:
            OCode();
            virtual ~OCode();

            inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t nSize,void* _pHint ) SAL_THROW( () )
                { return _pHint; }
            inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * pMem,void* _pHint ) SAL_THROW( () )
                {  }

            TYPEINFO();
        };


        // operands that the parsetree generate
        class OOperand : public OCode
        {
        protected:
            sal_Int32 m_eDBType;

            OOperand(const sal_Int32& _rType) : m_eDBType(_rType){}
            OOperand() : m_eDBType(::com::sun::star::sdbc::DataType::OTHER){}

        public:
            virtual const ORowSetValue& getValue() const = 0;
            virtual void setValue(const ORowSetValue& _rVal) = 0;

            virtual sal_Int32 getDBType() const {return m_eDBType;}
            virtual OEvaluateSet* preProcess(OBoolOperator* pOp, OOperand* pRight = 0);
            inline sal_Bool isValid() const;

            TYPEINFO();
        };

        class OOperandRow : public OOperand
        {
            sal_uInt16  m_nRowPos;
        protected:
            OValueRefRow    m_pRow;

            OOperandRow(sal_uInt16 _nPos, sal_Int32 _rType);
        public:
            sal_uInt16 getRowPos() const {return m_nRowPos;}
            virtual const ORowSetValue& getValue() const;
            virtual void setValue(const ORowSetValue& _rVal);
            void bindValue(const OValueRefRow& _pRow);                      // Bindung an den Wert, den der Operand repräsentiert

            TYPEINFO();
        };

        // Attribute aus einer Ergebniszeile
        class OOperandAttr : public OOperandRow
        {
        protected:
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xColumn;

        public:
            OOperandAttr(sal_uInt16 _nPos,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn);

            virtual sal_Bool isIndexed() const;
            virtual OEvaluateSet* preProcess(OBoolOperator* pOp, OOperand* pRight = 0);
            TYPEINFO();
        };

        // Parameter für ein Prädikat
        class OOperandParam : public OOperandRow
        {
        public:
            OOperandParam(connectivity::OSQLParseNode* pNode, sal_Int32 _nPos);
            void describe(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn, ::vos::ORef<connectivity::OSQLColumns> _xParamColumns);

            TYPEINFO();
        };

        // WerteOperanden
        class OOperandValue : public OOperand
        {
        protected:
            ORowSetValue m_aValue;

        protected:
            OOperandValue(){}
            OOperandValue(const ORowSetValue& _rVar, sal_Int32 eDbType)
                : OOperand(eDbType)
                , m_aValue(_rVar)
            {}

            OOperandValue(sal_Int32 eDbType) :OOperand(eDbType){}
        public:
            virtual const ORowSetValue& getValue() const;
            virtual void setValue(const ORowSetValue& _rVal);

            TYPEINFO();
        };


        // Konstanten
        class OOperandConst : public OOperandValue
        {
        public:
            OOperandConst(const connectivity::OSQLParseNode& rColumnRef, const rtl::OUString& aStrValue);

            TYPEINFO();
        };


        // Ergebnis Operanden
        class OOperandResult : public OOperandValue
        {
        protected:
            OOperandResult(const ORowSetValue& _rVar, sal_Int32 eDbType)
                            :OOperandValue(_rVar, eDbType) {}
            OOperandResult(sal_Int32 eDbType)
                            :OOperandValue(eDbType) {}
        public:
            OOperandResult(const ORowSetValue& _rVar)
                            :OOperandValue(_rVar, _rVar.getTypeKind()) {}
            TYPEINFO();
        };


        class OOperandResultBOOL : public OOperandResult
        {
        public:
            OOperandResultBOOL(sal_Bool bResult) : OOperandResult(::com::sun::star::sdbc::DataType::BIT)
            {
                m_aValue = bResult ? 1.0 : 0.0;
                m_aValue.setBound(sal_True);
            }
        };

        class OOperandResultNUM : public OOperandResult
        {
        public:
            OOperandResultNUM(double fNum) : OOperandResult(::com::sun::star::sdbc::DataType::DOUBLE)
            {
                m_aValue = fNum;
                m_aValue.setBound(sal_True);
            }
        };

        /** special stop operand
            is appended when a list of arguments ends
        */
        class OStopOperand : public OOperandValue
        {
        public:
            OStopOperand(){}
            TYPEINFO();
        };

        // Operatoren
        class OOperator : public OCode
        {
        public:
            virtual void Exec(OCodeStack&) = 0;
            virtual sal_uInt16 getRequestedOperands() const;            // Anzahl benötigter Operanden
                                                                // Standard ist 2
            TYPEINFO();
        };


        // boolsche Operatoren

        class OBoolOperator : public OOperator
        {
        public:
            TYPEINFO();
            virtual void Exec(OCodeStack&);
            virtual sal_Bool operate(const OOperand*, const OOperand*) const;
        };


        class OOp_AND : public OBoolOperator
        {
        public:
            TYPEINFO();

        protected:
            virtual sal_Bool operate(const OOperand*, const OOperand*) const;
        };

        class OOp_OR : public OBoolOperator
        {
        public:
            TYPEINFO();
        protected:
            virtual sal_Bool operate(const OOperand*, const OOperand*) const;
        };

        class OOp_ISNULL : public OBoolOperator
        {
        public:
            TYPEINFO();
        public:
            virtual void Exec(OCodeStack&);
            virtual sal_uInt16 getRequestedOperands() const;
            virtual sal_Bool operate(const OOperand*, const OOperand* = NULL) const;
        };

        class OOp_ISNOTNULL : public OOp_ISNULL
        {
        public:
            TYPEINFO();
            virtual sal_Bool operate(const OOperand*, const OOperand* = NULL) const;
        };

        class OOp_LIKE : public OBoolOperator
        {
        public:
            TYPEINFO();
        protected:
            const sal_Unicode cEscape;

        public:
            OOp_LIKE(const sal_Unicode cEsc = L'\0'):cEscape(cEsc){};

            virtual sal_Bool operate(const OOperand*, const OOperand*) const;
        };

        class OOp_NOTLIKE : public OOp_LIKE
        {
        public:
            TYPEINFO();
        public:
            OOp_NOTLIKE(const sal_Unicode cEsc = L'\0'):OOp_LIKE(cEsc){};

            virtual sal_Bool operate(const OOperand*, const OOperand*) const;
        };

        class OOp_COMPARE : public OBoolOperator
        {
            connectivity::OSQLPredicateType aPredicateType;

        public:
            TYPEINFO();
            OOp_COMPARE(connectivity::OSQLPredicateType aPType)
                         :aPredicateType(aPType) {}

            inline connectivity::OSQLPredicateType getPredicateType() const { return aPredicateType; }
            virtual sal_Bool operate(const OOperand*, const OOperand*) const;
        };

        // numerische Operatoren

        class ONumOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&);

            TYPEINFO();

        protected:
            virtual double operate(const double& fLeft,const double& fRight) const = 0;
        };

        class OOp_ADD : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const;
        };

        class OOp_SUB : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const;
        };

        class OOp_MUL : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const;
        };

        class OOp_DIV : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const;
        };

        inline sal_Bool OOperand::isValid() const
        {
            return getValue().getDouble() != double(0.0);
        }

        // operator
        class ONthOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&);

            TYPEINFO();

        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const = 0;
        };

        class OBinaryOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&);

            TYPEINFO();

        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const = 0;
        };

        class OUnaryOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&);
            virtual sal_uInt16 getRequestedOperands() const;
            virtual ORowSetValue operate(const ORowSetValue& lhs) const = 0;

            TYPEINFO();

        };
    }
}

#endif // _CONNECTIVITY_FILE_FCODE_HXX_

