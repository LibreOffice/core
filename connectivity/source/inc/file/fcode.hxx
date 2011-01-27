/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _CONNECTIVITY_FILE_FCODE_HXX_
#define _CONNECTIVITY_FILE_FCODE_HXX_

#include "connectivity/sqliterator.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include "connectivity/CommonTools.hxx"
#include <tools/rtti.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "connectivity/FValue.hxx"
#include "file/filedllapi.hxx"

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


        class OOO_DLLPUBLIC_FILE OCode
        {
        public:
            OCode();
            virtual ~OCode();

            inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint ) SAL_THROW( () )
                { return _pHint; }
            inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ ) SAL_THROW( () )
                {  }

            TYPEINFO();
        };


        // operands that the parsetree generate
        class OOO_DLLPUBLIC_FILE OOperand : public OCode
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

        class OOO_DLLPUBLIC_FILE OOperandRow : public OOperand
        {
            sal_uInt16  m_nRowPos;
        protected:
            OValueRefRow    m_pRow;

            OOperandRow(sal_uInt16 _nPos, sal_Int32 _rType);
        public:
            sal_uInt16 getRowPos() const {return m_nRowPos;}
            virtual const ORowSetValue& getValue() const;
            virtual void setValue(const ORowSetValue& _rVal);
            void bindValue(const OValueRefRow& _pRow);                      // Bindung an den Wert, den der Operand repraesentiert

            TYPEINFO();
        };

        // Attribute aus einer Ergebniszeile
        class OOO_DLLPUBLIC_FILE OOperandAttr : public OOperandRow
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

        // Parameter fuer ein Praedikat
        class OOperandParam : public OOperandRow
        {
        public:
            OOperandParam(connectivity::OSQLParseNode* pNode, sal_Int32 _nPos);
            void describe(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn, ::rtl::Reference<connectivity::OSQLColumns> _xParamColumns);

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
        class OOO_DLLPUBLIC_FILE OOperator : public OCode
        {
        public:
            virtual void Exec(OCodeStack&) = 0;
            virtual sal_uInt16 getRequestedOperands() const;    // Anzahl benoetigter Operanden
                                                                // Standard ist 2
            TYPEINFO();
        };


        // boolsche Operatoren

        class OOO_DLLPUBLIC_FILE OBoolOperator : public OOperator
        {
        public:
            TYPEINFO();
            virtual void Exec(OCodeStack&);
            virtual sal_Bool operate(const OOperand*, const OOperand*) const;
        };

        class OOp_NOT : public OBoolOperator
        {
        public:
            TYPEINFO();

        protected:
            virtual void Exec(OCodeStack&);
            virtual sal_Bool operate(const OOperand*, const OOperand* = NULL) const;
            virtual sal_uInt16 getRequestedOperands() const;
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

        class OOO_DLLPUBLIC_FILE OOp_ISNULL : public OBoolOperator
        {
        public:
            TYPEINFO();
        public:
            virtual void Exec(OCodeStack&);
            virtual sal_uInt16 getRequestedOperands() const;
            virtual sal_Bool operate(const OOperand*, const OOperand* = NULL) const;
        };

        class OOO_DLLPUBLIC_FILE OOp_ISNOTNULL : public OOp_ISNULL
        {
        public:
            TYPEINFO();
            virtual sal_Bool operate(const OOperand*, const OOperand* = NULL) const;
        };

        class OOO_DLLPUBLIC_FILE OOp_LIKE : public OBoolOperator
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

        class OOO_DLLPUBLIC_FILE OOp_COMPARE : public OBoolOperator
        {
            sal_Int32 aPredicateType;

        public:
            TYPEINFO();
            OOp_COMPARE(sal_Int32 aPType)
                         :aPredicateType(aPType) {}

            inline sal_Int32 getPredicateType() const { return aPredicateType; }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
