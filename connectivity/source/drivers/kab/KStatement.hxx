/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: KStatement.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-06 14:21:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_KAB_STATEMENT_HXX_
#define _CONNECTIVITY_KAB_STATEMENT_HXX_

#ifndef _CONNECTIVITY_KAB_CONNECTION_HXX_
#include "KConnection.hxx"
#endif
#include <list>

#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include "connectivity/sqliterator.hxx"
#endif
#ifndef _CONNECTIVITY_PARSE_SQLPARSE_HXX_
#include "connectivity/sqlparse.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XSTATEMENT_HPP_
#include <com/sun/star/sdbc/XStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

namespace connectivity
{
    namespace kab
    {
        typedef ::cppu::WeakComponentImplHelper4<   ::com::sun::star::sdbc::XStatement,
                                                    ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::util::XCancellable,
                                                    ::com::sun::star::sdbc::XCloseable> KabCommonStatement_BASE;

        //**************************************************************
        // Class KabCommonStatement
        // is a base class for the normal statement and for the prepared statement
        //**************************************************************
        class KabCommonStatement :  public comphelper::OBaseMutex,
                        public  KabCommonStatement_BASE,
                        public  ::cppu::OPropertySetHelper,
                        public  comphelper::OPropertyArrayUsageHelper<KabCommonStatement>

        {
            ::com::sun::star::sdbc::SQLWarning  m_aLastWarning;

        protected:
            ::std::list< ::rtl::OUString>       m_aBatchList;
            connectivity::OSQLParser            m_aParser;
            connectivity::OSQLParseTreeIterator m_aSQLIterator;
            connectivity::OSQLParseNode*        m_pParseTree;
            KabConnection*                      m_pConnection;  // The owning Connection object

        protected:
            class KabCondition *analyseWhereClause(
                const OSQLParseNode *pParseNode) const throw(::com::sun::star::sdbc::SQLException);
            class KabOrder *analyseOrderByClause(
                const OSQLParseNode *pParseNode) const throw(::com::sun::star::sdbc::SQLException);
            sal_Bool isTableKnown(class KabResultSet *pResult) const;
            void setKabFields(class KabResultSet *pResult) const throw(::com::sun::star::sdbc::SQLException);
            void selectAddressees(KabResultSet *pResult) const throw(::com::sun::star::sdbc::SQLException);
            void sortAddressees(KabResultSet *pResult) const throw(::com::sun::star::sdbc::SQLException);

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;

            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                    ::com::sun::star::uno::Any & rConvertedValue,
                    ::com::sun::star::uno::Any & rOldValue,
                    sal_Int32 nHandle,
                    const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::lang::IllegalArgumentException);
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                    ::com::sun::star::uno::Any& rValue,
                    sal_Int32 nHandle) const;

            virtual void resetParameters() const throw(::com::sun::star::sdbc::SQLException);
            virtual void getNextParameter(::rtl::OUString &rParameter) const throw(::com::sun::star::sdbc::SQLException);
            virtual ~KabCommonStatement();

        public:
            ::cppu::OBroadcastHelper& rBHelper;

            KabCommonStatement(KabConnection *_pConnection);
            using KabCommonStatement_BASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

            // OComponentHelper
            virtual void SAL_CALL disposing();

            // XInterface
            virtual void SAL_CALL release() throw();
            virtual void SAL_CALL acquire() throw();
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                    const ::com::sun::star::uno::Type & rType
                    ) throw(::com::sun::star::uno::RuntimeException);

            // XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(
                    ) throw(::com::sun::star::uno::RuntimeException);

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(
                    ) throw(::com::sun::star::uno::RuntimeException);

            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery(
                    const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL executeUpdate(
                     const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL execute(
                    const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(
                    ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(
                    ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearWarnings(
                    ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // XCancellable
            virtual void SAL_CALL cancel(
                    ) throw(::com::sun::star::uno::RuntimeException);

            // XCloseable
            virtual void SAL_CALL close(
                    ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // other methods
            inline KabConnection* getOwnConnection() const { return m_pConnection; }
        };

        //**************************************************************
        // Class KabStatement
        //**************************************************************
        typedef ::cppu::ImplInheritanceHelper1<
                KabCommonStatement, ::com::sun::star::lang::XServiceInfo > KabStatement_BASE;

        class KabStatement : public KabStatement_BASE
        {
        protected:
            virtual ~KabStatement() { }

        public:
            KabStatement(KabConnection* _pConnection);
            DECLARE_SERVICE_INFO();
        };
    }
}

#endif // _CONNECTIVITY_KAB_STATEMENT_HXX_
