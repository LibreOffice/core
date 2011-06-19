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

#ifndef _CONNECTIVITY_KAB_STATEMENT_HXX_
#define _CONNECTIVITY_KAB_STATEMENT_HXX_

#include "KConnection.hxx"
#include <list>
#include "connectivity/sqliterator.hxx"
#include "connectivity/sqlparse.hxx"
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/proparrhlp.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
