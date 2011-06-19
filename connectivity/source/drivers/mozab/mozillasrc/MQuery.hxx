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


#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#define _CONNECTIVITY_MAB_QUERY_HXX_

#include "MColumnAlias.hxx"
#include "MErrorResource.hxx"
#include <connectivity/FValue.hxx>
#include "MNSDeclares.hxx"
#include <osl/thread.hxx>
#include <com/sun/star/mozilla/MozillaProductType.hpp>

namespace connectivity
{
    namespace mozab
    {

        class  MQueryHelper;
        struct MQueryDirectory;

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
                 SoundsLike     = 8,
                 RegExp         = 9
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

        class MQuery;

        class MQueryExpression : public MQueryExpressionBase
        {
            friend class MQuery;

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


        class MQuery
        {
            /*
             * A query resultset with a maximum limit of
             * m_nMaxNrOfReturns return items, is created from
             * the following SQL statement:
             *
             * -------------------------------------------------
             * SELECT m_aAttributes FROM m_aAddressbook
             * WHERE  m_aMatchItems SQL_OPR m_aMatchValue
             * -------------------------------------------------
             *
             * We are\are not, depending on boolean m_bQuerySubDirs,
             * interested in querying the sub-directories of the
             * addressbook directory, if any.
             *
             * SQL_OPR:
             * m_aSqlOppr contains the SQL operations for every
             * attribute in m_aAttributes.
             * This member must be initialised together with
             * m_aAttributes.
             *
             * The SQL operations defined for 'SQL_OPR' are:
             * matchExists         = 0,
             * matchDoesNotExist   = 1,
             * matchContains       = 2,
             * matchDoesNotContain = 3,
             * matchIs             = 4,
             * matchIsNot          = 5,
             * matchBeginsWith     = 6,
             * matchEndsWith       = 7,
             * matchSoundsLike     = 8,
             * matchRegExp         = 9.
             * There must be mapping to one of these values.
             *
             * The following members MUST be initialised before
             * a query is executed:
             * m_Attributes, m_aMapAttrOppr, m_aAddressbook,
             * m_aMatchItems and m_aMatchValue.
             *
             * m_bQuerySubDirs and m_nMaxNrReturns are set to a
             * default value in the constructor which can be
             * overridden. If (element of) m_aSqlOppr is not set,
             * the default SQL operation is 'matchIs'.
             *
             */
        private:
            MQueryDirectory                *m_aQueryDirectory;
            MQueryHelper                   *m_aQueryHelper;
            ::rtl::OUString                 m_aAddressbook;
            sal_Int32                       m_nMaxNrOfReturns;
            sal_Bool                        m_bQuerySubDirs;
            MQueryExpression                m_aExpr;
            const OColumnAlias&             m_rColumnAlias;
            ::com::sun::star::mozilla::MozillaProductType
                                            m_Product;
            ::rtl::OUString                 m_Profile;
            ErrorDescriptor                 m_aError;

            void construct();
        protected:
            ::osl::Mutex                    m_aMutex;
#if OSL_DEBUG_LEVEL > 0
            oslThreadIdentifier m_oThreadID;
#endif

        public:
            /*
             * - Contains accessors to the members of this class.
             * - executeQuery() initiates a non-blocking query.
             */
            sal_Int32                       executeQuery(OConnection* _pCon);
            sal_Int32                       executeQueryProxied(OConnection* _pCon); //Used only by MNSMozabProxy

            sal_Int32                       createNewCard(); //return Row count number
            sal_Int32                       deleteRow(const sal_Int32 rowIndex);
            sal_Int32                       commitRow(const sal_Int32 rowIndex);
            sal_Bool                        resyncRow(sal_Int32 nDBRow);

            sal_Bool                        isWritable(OConnection* _pCon);

            sal_uInt32                      InsertLoginInfo(OConnection* _pCon);

            void                            setAddressbook( ::rtl::OUString&);
            ::rtl::OUString                 getAddressbook(void) const;

            const OColumnAlias&             getColumnAlias() const { return m_rColumnAlias; }

            void                            setExpression( MQueryExpression &_expr );

            void                            setMaxNrOfReturns( const sal_Int32);
            sal_Int32                       getMaxNrOfReturns(void) const;

            void                            setQuerySubDirs( sal_Bool&);
            sal_Bool                        getQuerySubDirs(void) const;

            sal_Int32                       getRowCount( void );
            sal_uInt32                      getRealRowCount( void );
            sal_Bool                        queryComplete( void );
            sal_Bool                        waitForQueryComplete( void );
            sal_Bool                        checkRowAvailable( sal_Int32 nDBRow );
            sal_Bool                        getRowValue( connectivity::ORowSetValue& rValue,
                                                         sal_Int32 nDBRow,
                                                         const rtl::OUString& aDBColumnName,
                                                         sal_Int32 nType ) const;
            sal_Bool                        setRowValue( connectivity::ORowSetValue& rValue,
                                                         sal_Int32 nDBRow,
                                                         const rtl::OUString& aDBColumnName,
                                                         sal_Int32 nType ) const;
            sal_Int32                       getRowStates(sal_Int32 nDBRow);
            sal_Bool                        setRowStates(sal_Int32 nDBRow,sal_Int32 aState);

            bool                            hadError() const { return m_aError.is(); }
            inline const ErrorDescriptor&   getError() const { return m_aError; }

        public:
//          MQuery();
            MQuery( const OColumnAlias& _ca );
            virtual ~MQuery();
            static MNameMapper* CreateNameMapper();
            static void FreeNameMapper( MNameMapper* _ptr );
        };
    }
}

#endif // _CONNECTIVITY_MAB_QUERY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
