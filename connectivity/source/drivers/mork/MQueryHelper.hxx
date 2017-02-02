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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MQUERYHELPER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MQUERYHELPER_HXX

#include <connectivity/FValue.hxx>
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
                StringExpr,
                Expr
            } node_type;

        protected:
            node_type   m_eNodeType;

            explicit MQueryExpressionBase( node_type _eNodeType ) : m_eNodeType( _eNodeType ) {}

        public:
            virtual ~MQueryExpressionBase() {}

            bool   isStringExpr( ) const { return m_eNodeType == StringExpr; }
            bool   isExpr( ) const { return m_eNodeType == Expr; }
        };

        class MQueryExpressionString : public MQueryExpressionBase {
        protected:
            OUString     m_aName;         // LHS
            MQueryOp::cond_type m_aBooleanCondition;
            OUString     m_aValue;        // RHS

        public:

            MQueryExpressionString( const OUString&     lhs,
                                    MQueryOp::cond_type cond,
                                    const OUString&     rhs )
                : MQueryExpressionBase( MQueryExpressionBase::StringExpr )
                , m_aName( lhs )
                , m_aBooleanCondition( cond )
                , m_aValue( rhs )
            {
            }

            MQueryExpressionString( const OUString&     lhs,
                                    MQueryOp::cond_type cond )
                : MQueryExpressionBase( MQueryExpressionBase::StringExpr )
                , m_aName( lhs )
                , m_aBooleanCondition( cond )
                , m_aValue( OUString() )
            {
            }

            const OUString&    getName() const { return m_aName; }
            MQueryOp::cond_type getCond() const { return m_aBooleanCondition; }
            const OUString&    getValue() const { return m_aValue; }
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

            // All expressions on a peer level use same condition operator
            void setExpressionCondition( bool_cond _cond )
                            { m_aExprCondType = _cond; }

            void addExpression(MQueryExpressionBase * expr)
                            { m_aExprVector.push_back(expr); }

            ExprVector const & getExpressions( ) const
                            { return m_aExprVector; }

            // All expressions on a peer level use same condition operator
            bool_cond getExpressionCondition( ) const
                            { return m_aExprCondType; }

            MQueryExpression() : MQueryExpressionBase( MQueryExpressionBase::Expr ),
                                 m_aExprCondType( OR )
                            {}

            virtual ~MQueryExpression() override {
                for (ExprVector::iterator i(m_aExprVector.begin());
                     i != m_aExprVector.end(); ++i)
                {
                    delete *i;
                }
            }

        protected:
            ExprVector          m_aExprVector;
            bool_cond           m_aExprCondType;

        private:
           MQueryExpression(const MQueryExpression&) = delete;
           MQueryExpression& operator=(const MQueryExpression&) = delete;
        };

        class MQueryHelperResultEntry
        {
        private:
            typedef std::unordered_map< OString, OUString, OStringHash >  FieldMap;

            FieldMap                m_Fields;

        public:
            MQueryHelperResultEntry();
            ~MQueryHelperResultEntry();

            OUString   getValue( const OString &key ) const;
            void            setValue( const OString &key, const OUString & rValue);
        };

        class MQueryHelper final
        {
        private:
            typedef std::vector< MQueryHelperResultEntry* > resultsArray;

            mutable ::osl::Mutex        m_aMutex;
            resultsArray        m_aResults;
            void            append(MQueryHelperResultEntry* resEnt );
            void            clear_results();
            OColumnAlias        m_rColumnAlias;
            ErrorDescriptor     m_aError;
            OUString     m_aAddressbook;

        public:
            explicit                   MQueryHelper(const OColumnAlias& _ca);
                                       ~MQueryHelper();

            void                       reset();
            MQueryHelperResultEntry*   getByIndex( sal_uInt32 nRow );
            static bool                queryComplete() { return true; }
            sal_Int32                  getResultCount() const;
            bool                       checkRowAvailable( sal_Int32 nDBRow );
            bool                       getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const OUString& aDBColumnName, sal_Int32 nType );
            sal_Int32                  executeQuery(OConnection* xConnection, MQueryExpression & expr);
            const OColumnAlias&        getColumnAlias() const { return m_rColumnAlias; }
            bool                       hadError() const { return m_aError.is(); }
            inline ErrorDescriptor&    getError() { return m_aError; }

            void                       setAddressbook( OUString&);
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MQUERYHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
