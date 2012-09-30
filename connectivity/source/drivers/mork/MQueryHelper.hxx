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

#ifndef _CONNECTIVITY_MORK_QUERYHELPER_HXX_
#define _CONNECTIVITY_MORK_QUERYHELPER_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <comphelper/stl_types.hxx>
#include <osl/thread.hxx>
#include <connectivity/FValue.hxx>

#include <boost/unordered_map.hpp>

#include "MErrorResource.hxx"

namespace connectivity
{
    namespace mork
    {
        class OConnection;
        class MQueryHelper;
        class ErrorDescriptor;

        namespace MQueryOp {
             typedef enum {
                 Exists         = 0,
                 DoesNotExist   = 1,
                 Contains       = 2,
                 DoesNotContain = 3,
                 Is             = 4,
                 IsNot          = 5,
                 BeginsWith     = 6,
                 EndsWith       = 7,
                 RegExp         = 8
            } cond_type;
        }

        class MQueryExpressionBase {
        public:
            typedef enum {
                Unknown,
                StringExpr,
                Expr
            } node_type;

        protected:
            node_type   m_eNodeType;

            MQueryExpressionBase() : m_eNodeType( Unknown ) {}
            MQueryExpressionBase( node_type _eNodeType ) : m_eNodeType( _eNodeType ) {}

        public:
            sal_Bool   isUnknown( ) const { return m_eNodeType == Unknown; }
            sal_Bool   isStringExpr( ) const { return m_eNodeType == StringExpr; }
            sal_Bool   isExpr( ) const { return m_eNodeType == Expr; }
        };

        class MQueryExpressionString : public MQueryExpressionBase {
        protected:
            ::rtl::OUString     m_aName;         // LHS
            MQueryOp::cond_type m_aBooleanCondition;
            ::rtl::OUString     m_aValue;        // RHS

        public:

            MQueryExpressionString( ::rtl::OUString&    lhs,
                                    MQueryOp::cond_type cond,
                                    ::rtl::OUString     rhs )
                : MQueryExpressionBase( MQueryExpressionBase::StringExpr )
                , m_aName( lhs )
                , m_aBooleanCondition( cond )
                , m_aValue( rhs )
            {
            }

            MQueryExpressionString( ::rtl::OUString&    lhs,
                                    MQueryOp::cond_type cond )
                : MQueryExpressionBase( MQueryExpressionBase::StringExpr )
                , m_aName( lhs )
                , m_aBooleanCondition( cond )
                , m_aValue( ::rtl::OUString() )
            {
            }

            const ::rtl::OUString&    getName() const { return m_aName; }
            MQueryOp::cond_type getCond() const { return m_aBooleanCondition; }
            const ::rtl::OUString&    getValue() const { return m_aValue; }
        };

        class MQueryExpression : public MQueryExpressionBase
        {
            friend class MQueryHelper;

        public:
            typedef ::std::vector< MQueryExpressionBase* > ExprVector;

            typedef enum {
                AND,
                OR
            } bool_cond;

            void setExpressions( ExprVector& _exprVector )
                            { m_aExprVector = _exprVector; }

            // All expressions on a peer level use same condition operator
            void setExpressionCondition( bool_cond _cond )
                            { m_aExprCondType = _cond; }

            ExprVector& getExpressions( )
                            { return m_aExprVector; }

            // All expressions on a peer level use same condition operator
            bool_cond getExpressionCondition( ) const
                            { return m_aExprCondType; }

            MQueryExpression() : MQueryExpressionBase( MQueryExpressionBase::Expr ),
                                 m_aExprCondType( OR )
                            { m_aExprVector.clear(); }


        protected:
            ExprVector          m_aExprVector;
            bool_cond           m_aExprCondType;

        };

        class MQueryHelperResultEntry
        {
        private:
            typedef ::boost::unordered_map< ::rtl::OString, ::rtl::OUString, ::rtl::OStringHash >  FieldMap;

            mutable ::osl::Mutex    m_aMutex;
            FieldMap                m_Fields;

        public:
            MQueryHelperResultEntry();
            ~MQueryHelperResultEntry();

            void            insert( const rtl::OString &key, rtl::OUString &value );
            rtl::OUString   getValue( const rtl::OString &key ) const;
            void            setValue( const rtl::OString &key, const rtl::OUString & rValue);
        };

        class MQueryHelper
        {
        private:
            typedef std::vector< MQueryHelperResultEntry* > resultsArray;

            mutable ::osl::Mutex        m_aMutex;
            ::osl::Condition    m_aCondition;
            resultsArray        m_aResults;
            sal_uInt32          m_nIndex;
            sal_Bool            m_bHasMore;
            sal_Bool            m_bAtEnd;
            void            append(MQueryHelperResultEntry* resEnt );
            void            clear_results();
            OColumnAlias        m_rColumnAlias;
            ErrorDescriptor     m_aError;
            ::rtl::OUString     m_aAddressbook;
            MQueryExpression    m_aExpr;

/*
            void            clearResultOrComplete();
            void            notifyResultOrComplete();
            sal_Bool        waitForResultOrComplete( );
            void            getCardValues(nsIAbCard  *card,sal_uInt32 rowIndex=0);
*/

            sal_Int32 doQueryDefaultTable(OConnection* xConnection);
            sal_Int32 doQueryListTable(OConnection* xConnection, rtl::OString& ouStringTable);

        public:
                                       MQueryHelper(const OColumnAlias& _ca);
            virtual                    ~MQueryHelper();

            void                       reset();
            MQueryHelperResultEntry*   next();
            MQueryHelperResultEntry*   getByIndex( sal_uInt32 nRow );
            sal_Bool                   isError() const;
            sal_Bool                   queryComplete() const;
            sal_Int32                  getResultCount() const;
            sal_Bool                   checkRowAvailable( sal_Int32 nDBRow );
            sal_Bool getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const rtl::OUString& aDBColumnName, sal_Int32 nType );
            sal_Int32 executeQuery(OConnection* xConnection);
            const OColumnAlias&             getColumnAlias() const { return m_rColumnAlias; }
            bool                            hadError() const { return m_aError.is(); }
            inline ErrorDescriptor& getError() { return m_aError; }

            void                            setAddressbook( ::rtl::OUString&);
            void                            setExpression( MQueryExpression &_expr );

        };
    }
}

#endif // _CONNECTIVITY_MORK_QUERYHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
