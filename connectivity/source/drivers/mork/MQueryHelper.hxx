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
#include "MColumnAlias.hxx"

namespace connectivity::mork
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
            enum class node_type {
                StringExpr,
                Expr
            };

        protected:
            node_type   m_eNodeType;

            explicit MQueryExpressionBase( node_type _eNodeType ) : m_eNodeType( _eNodeType ) {}

        public:
            virtual ~MQueryExpressionBase() {}

            bool   isStringExpr( ) const { return m_eNodeType == node_type::StringExpr; }
            bool   isExpr( ) const { return m_eNodeType == node_type::Expr; }
        };

        class MQueryExpressionString final : public MQueryExpressionBase {
            OUString     m_aName;         // LHS
            MQueryOp::cond_type m_aBooleanCondition;
            OUString     m_aValue;        // RHS

        public:

            MQueryExpressionString( const OUString&     lhs,
                                    MQueryOp::cond_type cond,
                                    const OUString&     rhs )
                : MQueryExpressionBase( MQueryExpressionBase::node_type::StringExpr )
                , m_aName( lhs )
                , m_aBooleanCondition( cond )
                , m_aValue( rhs )
            {
            }

            MQueryExpressionString( const OUString&     lhs,
                                    MQueryOp::cond_type cond )
                : MQueryExpressionBase( MQueryExpressionBase::node_type::StringExpr )
                , m_aName( lhs )
                , m_aBooleanCondition( cond )
                , m_aValue( OUString() )
            {
            }

            const OUString&    getName() const { return m_aName; }
            MQueryOp::cond_type getCond() const { return m_aBooleanCondition; }
            const OUString&    getValue() const { return m_aValue; }
        };

        class MQueryExpression final : public MQueryExpressionBase
        {
            friend class MQueryHelper;

        public:
            typedef std::vector< MQueryExpressionBase* > ExprVector;

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

            MQueryExpression() : MQueryExpressionBase( MQueryExpressionBase::node_type::Expr ),
                                 m_aExprCondType( OR )
                            {}

        private:
            ExprVector          m_aExprVector;
            bool_cond           m_aExprCondType;

           MQueryExpression(const MQueryExpression&) = delete;
           MQueryExpression& operator=(const MQueryExpression&) = delete;
        };

        class MQueryHelperResultEntry
        {
        private:
            typedef std::unordered_map< OString, OUString >  FieldMap;

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

            mutable ::osl::Mutex        m_aMutex;
            std::vector< std::unique_ptr<MQueryHelperResultEntry> > m_aResults;
            void            append(std::unique_ptr<MQueryHelperResultEntry> resEnt );
            void            clear_results();
            OColumnAlias        m_rColumnAlias;
            ErrorDescriptor     m_aError;
            OUString     m_aAddressbook;

        public:
            explicit                   MQueryHelper(const OColumnAlias& _ca);
                                       ~MQueryHelper();

            void                       reset();
            MQueryHelperResultEntry*   getByIndex( sal_uInt32 nRow );
            sal_Int32                  getResultCount() const;
            bool                       getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const OUString& aDBColumnName, sal_Int32 nType );
            sal_Int32                  executeQuery(OConnection* xConnection, MQueryExpression & expr);
            const OColumnAlias&        getColumnAlias() const { return m_rColumnAlias; }
            bool                       hadError() const { return m_aError.is(); }
            ErrorDescriptor&    getError() { return m_aError; }

            void                       setAddressbook( OUString const &);
        };

}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MQUERYHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
