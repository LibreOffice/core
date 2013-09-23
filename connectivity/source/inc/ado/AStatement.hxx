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

#ifndef _CONNECTIVITY_ADO_ASTATEMENT_HXX_
#define _CONNECTIVITY_ADO_ASTATEMENT_HXX_

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <cppuhelper/compbase5.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include "ado/AConnection.hxx"
#include <list>
#include "ado/Awrapado.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity
{
    namespace ado
    {
        typedef ::cppu::WeakComponentImplHelper5<   ::com::sun::star::sdbc::XStatement,
                                                    ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::util::XCancellable,
                                                    ::com::sun::star::sdbc::XCloseable,
                                                    ::com::sun::star::sdbc::XMultipleResults> OStatement_BASE;

        //**************************************************************
        //************ Class: java.sql.Statement
        //**************************************************************
        class OStatement_Base       :   public comphelper::OBaseMutex,
                                        public  OStatement_BASE,
                                        public  ::cppu::OPropertySetHelper,
                                        public  ::comphelper::OPropertyArrayUsageHelper<OStatement_Base>,
                                        public  connectivity::OSubComponent<OStatement_Base, OStatement_BASE>

        {
            friend class connectivity::OSubComponent<OStatement_Base, OStatement_BASE>;
            friend class OResultSet;

            ::com::sun::star::sdbc::SQLWarning          m_aLastWarning;

        protected:
            ::std::list< OUString>               m_aBatchList;

            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
                                                                        //  for this Statement

            OConnection*                                m_pConnection;// The owning Connection object
            WpADOCommand                                m_Command;
            WpADORecordset                              m_RecordSet;
            OLEVariant                                  m_RecordsAffected;
            OLEVariant                                  m_Parameters;
            ::std::vector<connectivity::OTypeInfo>      m_aTypeInfo;    // Hashtable containing an entry
                                                                        //  for each row returned by
                                                                        //  DatabaseMetaData.getTypeInfo.
            ADO_LONGPTR                                 m_nMaxRows;
            sal_Int32                                   m_nFetchSize;
            LockTypeEnum                                m_eLockType;
            CursorTypeEnum                              m_eCursorType;

            using OStatement_BASE::rBHelper;
        private:

            sal_Int32 getQueryTimeOut()         const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getMaxFieldSize()         const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getMaxRows()              const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getResultSetConcurrency() const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getResultSetType()        const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getFetchDirection()       const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getFetchSize()            const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OUString getCursorName()     const throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            void setQueryTimeOut(sal_Int32 _par0)           throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setMaxFieldSize(sal_Int32 _par0)           throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setMaxRows(sal_Int32 _par0)                throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setResultSetConcurrency(sal_Int32 _par0)   throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setResultSetType(sal_Int32 _par0)          throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setFetchDirection(sal_Int32 _par0)         throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setFetchSize(sal_Int32 _par0)              throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            void setCursorName(const OUString &_par0) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        protected:

            void assignRecordSet( ADORecordset* _pRS );

            void reset () throw( ::com::sun::star::sdbc::SQLException);
            void clearMyResultSet () throw( ::com::sun::star::sdbc::SQLException);
            void setWarning (const  ::com::sun::star::sdbc::SQLWarning &ex) throw( ::com::sun::star::sdbc::SQLException);
            sal_Int32 getColumnCount () throw( ::com::sun::star::sdbc::SQLException);
            sal_Int32 getRowCount () throw( ::com::sun::star::sdbc::SQLException);
            sal_Int32 getPrecision ( sal_Int32 sqlType);

            void disposeResultSet();

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                    )
                                    throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                    ) const;
        public:
            OStatement_Base(OConnection* _pConnection );

            using OStatement_BASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;
            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) ;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XCancellable
            virtual void SAL_CALL cancel(  ) throw(::com::sun::star::uno::RuntimeException);
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XMultipleResults
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSet(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getUpdateCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL getMoreResults(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };

        class OStatement :  public OStatement_Base,
                            public ::com::sun::star::sdbc::XBatchExecution,
                            public ::com::sun::star::lang::XServiceInfo
        {
        public:
            // a Constructor, that is needed for when Returning the Object is needed:
            OStatement( OConnection* _pConnection) : OStatement_Base( _pConnection){};
            ~OStatement();

            DECLARE_SERVICE_INFO();

            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            // XBatchExecution
            virtual void SAL_CALL addBatch( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // _CONNECTIVITY_ADO_ASTATEMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
