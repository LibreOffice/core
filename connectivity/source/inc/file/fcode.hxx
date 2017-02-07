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
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <connectivity/FValue.hxx>
#include "file/filedllapi.hxx"

#include <stack>

namespace connectivity
{
    class OSQLParseNode;
    namespace file
    {

        class OOperand;
        typedef ::std::stack<OOperand*> OCodeStack;

        class OOO_DLLPUBLIC_FILE OCode
        {
        public:
            //virtual dtor to allow this to be the root of the class hierarchy
            virtual ~OCode();
#if !defined _MSC_VER || _MSC_VER >= 1900
            //but that disables the default move ctor
            OCode(OCode&&) = default;
            //but that disables the rest of default ctors
            OCode(const OCode&) = default;
            OCode() = default;
            //and same issue for the assignment operators
            OCode& operator=(const OCode&) = default;
            OCode& operator=(OCode&&) = default;
#endif

            inline static void * SAL_CALL operator new( size_t nSize )
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint )
                { return _pHint; }
            inline static void SAL_CALL operator delete( void * pMem )
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ )
                {  }

        };


        // operands that the parsetree generate
        class OOO_DLLPUBLIC_FILE OOperand : public OCode
        {
        protected:
            sal_Int32 m_eDBType;

            OOperand(sal_Int32 _rType) : m_eDBType(_rType){}
            OOperand() : m_eDBType(css::sdbc::DataType::OTHER){}

        public:
            virtual const ORowSetValue& getValue() const = 0;
            virtual void setValue(const ORowSetValue& _rVal) = 0;

            sal_Int32 getDBType() const {return m_eDBType;}
            inline bool isValid() const;

        };

        class OOO_DLLPUBLIC_FILE OOperandRow : public OOperand
        {
            sal_uInt16  m_nRowPos;
        protected:
            OValueRefRow    m_pRow;

            OOperandRow(sal_uInt16 _nPos, sal_Int32 _rType);
        public:
            virtual const ORowSetValue& getValue() const override;
            virtual void setValue(const ORowSetValue& _rVal) override;
            void bindValue(const OValueRefRow& _pRow); // Bind to the value that the operand represents

        };

        // Attributes from a result row
        class OOO_DLLPUBLIC_FILE OOperandAttr : public OOperandRow
        {
        public:
            OOperandAttr(sal_uInt16 _nPos,
                         const css::uno::Reference< css::beans::XPropertySet>& _xColumn);

        };

        // Parameter for a predicate
        class OOperandParam : public OOperandRow
        {
        public:
            OOperandParam(connectivity::OSQLParseNode* pNode, sal_Int32 _nPos);
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
            virtual const ORowSetValue& getValue() const override;
            virtual void setValue(const ORowSetValue& _rVal) override;

        };


        // Constants
        class OOperandConst : public OOperandValue
        {
        public:
            OOperandConst(const connectivity::OSQLParseNode& rColumnRef, const OUString& aStrValue);

        };


        // Result operands
        class OOperandResult : public OOperandValue
        {
        protected:
            OOperandResult(sal_Int32 eDbType)
                            :OOperandValue(eDbType) {}
        public:
            OOperandResult(const ORowSetValue& _rVar)
                            :OOperandValue(_rVar, _rVar.getTypeKind()) {}
        };


        class OOperandResultBOOL : public OOperandResult
        {
        public:
            OOperandResultBOOL(bool bResult) : OOperandResult(css::sdbc::DataType::BIT)
            {
                m_aValue = bResult ? 1.0 : 0.0;
                m_aValue.setBound(true);
            }
        };

        class OOperandResultNUM : public OOperandResult
        {
        public:
            OOperandResultNUM(double fNum) : OOperandResult(css::sdbc::DataType::DOUBLE)
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
        };

        // Operators
        class OOO_DLLPUBLIC_FILE OOperator : public OCode
        {
        public:
            virtual void Exec(OCodeStack&) = 0;
        };


        // Boolean operators
        class OOO_DLLPUBLIC_FILE OBoolOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) override;
            virtual bool operate(const OOperand*, const OOperand*) const;
        };

        class OOp_NOT : public OBoolOperator
        {
        public:

        protected:
            virtual void Exec(OCodeStack&) override;
            virtual bool operate(const OOperand*, const OOperand*) const override;
        };

        class OOp_AND : public OBoolOperator
        {
        public:

        protected:
            virtual bool operate(const OOperand*, const OOperand*) const override;
        };

        class OOp_OR : public OBoolOperator
        {
        public:
        protected:
            virtual bool operate(const OOperand*, const OOperand*) const override;
        };

        class OOO_DLLPUBLIC_FILE OOp_ISNULL : public OBoolOperator
        {
        public:
        public:
            virtual void Exec(OCodeStack&) override;
            virtual bool operate(const OOperand*, const OOperand*) const override;
        };

        class OOO_DLLPUBLIC_FILE OOp_ISNOTNULL : public OOp_ISNULL
        {
        public:
            virtual bool operate(const OOperand*, const OOperand*) const override;
        };

        class OOO_DLLPUBLIC_FILE OOp_LIKE : public OBoolOperator
        {
        public:
        protected:
            const sal_Unicode cEscape;

        public:
            OOp_LIKE(const sal_Unicode cEsc = L'\0'):cEscape(cEsc){};

            virtual bool operate(const OOperand*, const OOperand*) const override;
        };

        class OOp_NOTLIKE : public OOp_LIKE
        {
        public:
        public:
            OOp_NOTLIKE(const sal_Unicode cEsc = L'\0'):OOp_LIKE(cEsc){};

            virtual bool operate(const OOperand*, const OOperand*) const override;
        };

        class OOO_DLLPUBLIC_FILE OOp_COMPARE : public OBoolOperator
        {
            sal_Int32 aPredicateType;

        public:
            OOp_COMPARE(sal_Int32 aPType)
                         :aPredicateType(aPType) {}

            inline sal_Int32 getPredicateType() const { return aPredicateType; }
            virtual bool operate(const OOperand*, const OOperand*) const override;
        };

        // Numerical operators
        class ONumOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) override;


        protected:
            virtual double operate(const double& fLeft,const double& fRight) const = 0;
        };

        class OOp_ADD : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const override;
        };

        class OOp_SUB : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const override;
        };

        class OOp_MUL : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const override;
        };

        class OOp_DIV : public ONumOperator
        {
        protected:
            virtual double operate(const double& fLeft,const double& fRight) const override;
        };

        inline bool OOperand::isValid() const
        {
            return getValue().getDouble() != double(0.0);
        }

        // Operator
        class ONthOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) override;


        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const = 0;
        };

        class OBinaryOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) override;


        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const = 0;
        };

        class OUnaryOperator : public OOperator
        {
        public:
            virtual void Exec(OCodeStack&) override;
            virtual ORowSetValue operate(const ORowSetValue& lhs) const = 0;


        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
