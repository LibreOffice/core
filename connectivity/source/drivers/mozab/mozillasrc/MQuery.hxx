/*************************************************************************
 *
 *  $RCSfile: MQuery.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 14:40:55 $
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


#ifndef _CONNECTIVITY_MAB_QUERY_HXX_
#define _CONNECTIVITY_MAB_QUERY_HXX_

#ifndef _CONNECTIVITY_MAB_COLUMNALIAS_HXX_
#include "MColumnAlias.hxx"
#endif
#include <connectivity/FValue.hxx>

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
            sal_Bool   isUnknown( ) { return m_eNodeType == Unknown; }
            sal_Bool   isStringExpr( ) { return m_eNodeType == StringExpr; }
            sal_Bool   isExpr( ) { return m_eNodeType == Expr; }
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

            const ::rtl::OUString&    getName()  { return m_aName; }
            const MQueryOp::cond_type getCond()  { return m_aBooleanCondition; }
            const ::rtl::OUString&    getValue() { return m_aValue; }
        };

        class MQuery;

        class MQueryExpression : public MQueryExpressionBase
        {
            friend MQuery;

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
            bool_cond getExpressionCondition( )
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
            ::std::vector< ::rtl::OUString> m_aAttributes;
            ::rtl::OUString                 m_aAddressbook;
            sal_Int32                       m_nMaxNrOfReturns;
            sal_Bool                        m_bQuerySubDirs;
            MQueryExpression                m_aExpr;
            ::std::map< ::rtl::OUString,
                        ::rtl::OUString>    m_aColumnAliasMap;
            mutable ::rtl::OUString         m_aErrorString;
            mutable sal_Bool                m_aErrorOccurred;

            void construct();
        protected:
            ::osl::Mutex                    m_aMutex;

        public:
            /*
             * - Contains accessors to the members of this class.
             * - executeQuery() initiates a non-blocking query.
             */
            void                            setAttributes( ::std::vector< ::rtl::OUString>&);
            const ::std::vector< ::rtl::OUString> &getAttributes(void) const;

            void                            setAddressbook( ::rtl::OUString&);
            ::rtl::OUString                 getAddressbook(void) const;

            const ::std::map< ::rtl::OUString,::rtl::OUString>&
                                            getColumnAliasMap() const { return m_aColumnAliasMap; }

            void                            setExpression( MQueryExpression &_expr );

            void                            setMaxNrOfReturns( const sal_Int32);
            sal_Int32                       getMaxNrOfReturns(void) const;

            void                            setQuerySubDirs( sal_Bool&);
            sal_Bool                        getQuerySubDirs(void) const;

            sal_Int32                       executeQuery(OConnection* _pCon);

            sal_Int32                       getRowCount( void );

            sal_uInt32                      getRealRowCount( void );

            sal_Bool                        queryComplete( void );

            sal_Bool                        waitForQueryComplete( void );

            sal_Bool                        checkRowAvailable( sal_Int32 nDBRow );

            sal_Bool                        getRowValue( connectivity::ORowSetValue& rValue,
                                                         sal_Int32 nDBRow,
                                                         const rtl::OUString& aDBColumnName,
                                                         sal_Int32 nType ) const;

            sal_Bool                        errorOccurred() const
                                            { return m_aErrorOccurred; };

            const ::rtl::OUString&          getErrorString() const
                                            { return m_aErrorString; }

        public:
            MQuery();
            MQuery(const ::std::map< ::rtl::OUString, ::rtl::OUString> &);
            virtual ~MQuery();
            static MNameMapper* CreateNameMapper();
            static void FreeNameMapper( MNameMapper* _ptr );
        };
    }
}

#endif // _CONNECTIVITY_MAB_QUERY_HXX_

