/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCODE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCODE_HXX

#include <connectivity/sqliterator.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <connectivity/CommonTools.hxx>
#include <tools/rtti.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <connectivity/FValue.hxx>
#include "file/filedllapi.hxx"

namespace connectivity
{
    class OSQLParseNode;
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

            inline static void * SAL_CALL operator new( size_t nSize )
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint )
                { return _pHint; }
            inline static void SAL_CALL operator delete( void * pMem )
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ )
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

            sal_Int32 getDBType() const {return m_eDBType;}
            virtual OEvaluateSet* preProcess(OBoolOperator* pOp, OOperand* pRight = 0);
            inline bool isValid() const;

            TYPEINFO_OVERRIDE();
        };

        class OOO_DLLPUBLIC_FILE OOperandRow : public OOperand
        {
            sal_uInt16  m_nRowPos;
        protected:
            OValueRefRow    m_pRow;

            OOperandRow(sal_uInt16 _nPos, sal_Int32 _rType);
        public:
            sal_uInt16 getRowPos() const {return m_nRowPos;}
            virtual const ORowSetValue& getValue() const SAL_OVERRIDE;
            virtual void setValue(const ORowSetValue& _rVal) SAL_OVERRIDE;
            void bindValue(const OValueRefRow& _pRow); // Bind to the value that the operand represents

            TYPEINFO_OVERRIDE();
        };

        // Attributes from a result row
        class OOO_DLLPUBLIC_FILE OOperandAttr : public OOperandRow
        {
        protected:
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xColumn;

        public:
            OOperandAttr(sal_uInt16 _nPos,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn);

            virtual OEvaluateSet* preProcess(OBoolOperator* pOp, OOperand* pRight = 0) SAL_OVERRIDE;
            TYPEINFO_OVERRIDE();
        };

        // Parameter for a predicate
        class OOperandParam : public OOperandRow
        {
        public:
            OOperandParam(connectivity::OSQLParseNode* pNode, sal_Int32 _nPos);
            TYPEINFO_OVERRIDE();
        };

        // Value operands
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
            virtual const ORowSetValue& getValue() const SAL_OVERRIDE;
            virtual void setValue(const ORowSetValue& _rVal) SAL_OVERRIDE;

            TYPEINFO_OVERRIDE();
        };


        // Constants
        class OOperandConst : public OOperandValue
        {
        public:
            OOperandConst(const connectivity::OSQLParseNode& rColumnRef, const OUString& aStrValue);

            TYPEINFO_OVERRIDE();
        };


        // Result operands
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
            TYPEINFO_OVERRIDE();
        };


        class OOperandResultBOOL : public OOperandResult
        {
        public:
            OOperandResultBOOL(bool bResult) : OOperandResult(::com::sun::star::sdbc::DataType::BIT)
            {
                m_aValue = bResult ? 1.0 : 0.0;
                m_aValue.setBound(true);
            }
        };

        class OOperandResultNUM : public OOperandResult
        {
        public:
            OOperandResultNUM(double fNum) : OOperandResult(::com::sun::star::sdbc::DataType::DOUBLE)
            {
                m_aValue = fNum;
                m_aValue.setBound(true);
            }
        };

        /** special stop operand
            is appended when a list of arguments ends
        */
        class OStopOperand : public OOperandValue
        {
        public:
            OStopOperand(){}
            TYPEINFO_OVERRIDE();
        };

        // Operators
        class OOO_DLLPUBLIC_FILE OOperator : public OCode
        {
        public:
            virtual void Exec(OCodeStack&) = 0;
            virtual sal_uInt16 getRequestedOperands() const;    // Count of requested operands
                                                                // Defaults to 2
            TYPEINFO_OVERRIDE();
        };


        // Boolean operators
        class OOO_DLLPUBLIC_FILE OBoolOperator : public OOperator
        {
        public:
            TYPEINFO_OVERRIDE();
            virtual void Exec(OCodeStack&) SAL_OVERRIDE;
            virtual bool operate(const OOperand*, const OOperand*) const;
        };

        class OOp_NOT : public OBoolOperator
        {
        public:
            TYPEINFO_OVERRIDE();

        protected:
            virtual void Exec(OCodeStack&) SAL_OVERRIDE;
            virtual bool operate(const OOperand*, const OOperand* = NULL) const SAL_OVERRIDE;
            virtual sal_uInt16 getRequestedOperands() const SAL_OVERRIDE;
        };

        class OOp_AND : public OBoolOperator
        {
        public:
            TYPEINFO_OVERRIDE();

        protected:
            virtual bool operate(const OOperand*, const OOperand*) const SAL_OVERRIDE;
        };

        class OOp_OR : public OBoolOperator
        {
        public:
            TYPEINFO_OVERRIDE();
        protected:
            virtual bool operate(const OOperand*, const OOperand*) const SAL_OVERRIDE;
        };

        class OOO_DLLPUBLIC_FILE OOp_ISNULL : public OBoolOperator
        {
        public:
            TYPEINFO_OVERRIDE();
        public:
            virtual void Exec(OCodeStack&) SAL_OVERRIDE;
            virtual sal_uInt16 getRequestedOperands() const SAL_OVERRIDE;
            virtual bool operate(const OOperand*, const OOperand* = NULL) const SAL_OVERRIDE;
        };

        class OOO_DLLPUBLIC_FILE OOp_ISNOTNULL : public OOp_ISNULL
        {
        public:
            TYPEINFO_OVERRIDE();
            virtual bool operate(const OOperand*, const OOperand* = NULL) const SAL_OVERRIDE;
        };

        class OOO_DLLPUBLIC_FILE OOp_LIKE : public OBoolOperator
        {
        public:
            TYPEINFO_OVERRIDE();
        protected:
            const sal_Unicode cEscape;

        public:
            OOp_LIKE(const sal_Unicode cEsc = L'\0'):cEscape(cEsc){};

            virtual bool operate(const OOperand*, const OOperand*) const SAL_OVERRIDE;
        };

        class OOp_NOTLIKE : public OOp_LIKE
        {
        public:
            TYPEINFO_OVERRIDE();
        public:
            OOp_NOTLIKE(const sal_Unicode cEsc = L'\0'):OOp_LIKE(cEsc){};

            virtual bool operate(const OOperand*, const OOperand*) const SAL_OVERRIDE;
        };

        class OOO_DLLPUBLIC_FILE OOp_COMPARE : public OBoolOperator
        {
            sal_Int32 aPredicateType;

        public:
            TYPEINFO_OVERRIDE();
            OOp_COMPARE(sal_Int32 aPType)
                         :aPredicateType(aPType) {}

            inline sal_Int32 getPredicateType() const { return aPredicateType; }
            virtual bool operate(const OOperand*, const OOperand*) const SAL_OVERRIDE;
        };

        // Numerical operators
        class ONumOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) SAL_OVERRIDE;

            TYPEINFO_OVERRIDE();

        protected:
            virtual double operate(const double& fLeft,const double& fRight) const = 0;
        };

        class OOp_ADD : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const SAL_OVERRIDE;
        };

        class OOp_SUB : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const SAL_OVERRIDE;
        };

        class OOp_MUL : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const SAL_OVERRIDE;
        };

        class OOp_DIV : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const SAL_OVERRIDE;
        };

        inline bool OOperand::isValid() const
        {
            return getValue().getDouble() != double(0.0);
        }

        // Operator
        class ONthOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) SAL_OVERRIDE;

            TYPEINFO_OVERRIDE();

        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const = 0;
        };

        class OBinaryOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) SAL_OVERRIDE;

            TYPEINFO_OVERRIDE();

        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const = 0;
        };

        class OUnaryOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) SAL_OVERRIDE;
            virtual sal_uInt16 getRequestedOperands() const SAL_OVERRIDE;
            virtual ORowSetValue operate(const ORowSetValue& lhs) const = 0;

            TYPEINFO_OVERRIDE();

        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
