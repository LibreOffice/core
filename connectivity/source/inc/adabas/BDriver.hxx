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

#ifndef _CONNECTIVITY_ADABAS_BDRIVER_HXX_
#define _CONNECTIVITY_ADABAS_BDRIVER_HXX_

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#include <com/sun/star/sdbcx/XDropCatalog.hpp>

#include <com/sun/star/lang/XEventListener.hpp>

#include <cppuhelper/implbase4.hxx>
#include "odbc/OFunctiondefs.hxx"
#include "odbc/ODriver.hxx"

namespace connectivity
{
    namespace adabas
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ODriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

        typedef ::cppu::ImplHelper4<    ::com::sun::star::sdbcx::XCreateCatalog,
                                        ::com::sun::star::sdbcx::XDataDefinitionSupplier,
                                        ::com::sun::star::lang::XEventListener,
                                        ::com::sun::star::sdbcx::XDropCatalog> ODriver_BASE2;

        typedef odbc::ODBCDriver ODriver_BASE;

        class ODriver : public ODriver_BASE,
                        public ODriver_BASE2
        {
            typedef struct DatabaseStruct
            {
                ::rtl::OUString sControlUser;
                ::rtl::OUString sControlPassword;
                ::rtl::OUString sSysUser;
                ::rtl::OUString sSysPassword;
                ::rtl::OUString sDomainPassword;
                ::rtl::OUString sCacheSize;
                ::rtl::OUString sBackupFile;
                ::rtl::OUString sDataDevName;
                ::rtl::OUString sSysDevSpace;
                ::rtl::OUString sTransLogName;
                ::rtl::OUString sDBName;
                sal_Int32       nDataIncrement;     // which size the database should grow
                sal_Int32       nDataSize;
                sal_Int32       nLogSize;
                sal_Bool        bShutDown;
                sal_Bool        bRestoreDatabase;

                DatabaseStruct() : nDataIncrement(0),nDataSize(0),nLogSize(0),bShutDown(sal_False),bRestoreDatabase(sal_False){ }
            } TDatabaseStruct;

            DECLARE_STL_USTRINGACCESS_MAP(TDatabaseStruct,TDatabaseMap);
            TDatabaseMap    m_aDatabaseMap; // contains all adabas databases with their flag to shut down or not

            // environment vars
            ::rtl::OUString m_sDbWork;
            ::rtl::OUString m_sDbConfig;
            ::rtl::OUString m_sDbRoot;
            ::rtl::OUString m_sDbWorkURL;
            ::rtl::OUString m_sDbConfigURL;
            ::rtl::OUString m_sDbRootURL;
            ::rtl::OUString m_sDbRunDir;
            ::rtl::OUString m_sDelimit;

            void checkAndInsertNewDevSpace(const ::rtl::OUString& _rDBName,const TDatabaseStruct& _rDBInfo);
            void checkAndRestart(const ::rtl::OUString& _rDBName,const TDatabaseStruct& _rDbInfo);
            void X_CONS(const ::rtl::OUString& _DBNAME,const ::rtl::OString& _ACTION,const ::rtl::OUString& _FILENAME);
            sal_Bool getDBName(const ::rtl::OUString& _rName,::rtl::OUString& _rDBName) const;
            ::rtl::OUString getDatabaseInitFile(  const TDatabaseStruct& _aInfo);
            ::rtl::OUString generateInitFile()  const;
            void fillEnvironmentVariables();
            void fillInfo(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info, TDatabaseStruct& _rDBInfo /*out*/);
            void LoadBatch(const ::rtl::OUString& _rDBNAME,
                        const ::rtl::OUString& _rUSR,
                        const ::rtl::OUString& _rPWD,
                        const ::rtl::OUString& _rBatch);
            void XUTIL(const ::rtl::OUString& _rParam,
                    const ::rtl::OUString& _DBNAME,
                    const ::rtl::OUString& _USRNAME,
                    const ::rtl::OUString& _USRPWD);
            int X_STOP(const ::rtl::OUString& _DBNAME);
            int X_START(const ::rtl::OUString& _DBNAME);
            void createDb(const TDatabaseStruct& _aInfo);
            void clearDatabase(const ::rtl::OUString& _rDBName);
            int X_PARAM(const ::rtl::OUString& _DBNAME,
                        const ::rtl::OUString& _USR,
                        const ::rtl::OUString& _PWD,
                        const ::rtl::OUString& _CMD);
            sal_Int32 CreateFiles(const TDatabaseStruct& _aInfo);
            sal_Bool CreateFile(const ::rtl::OUString &_FileName,
                                sal_Int32 _nSize);
            void PutParam(const ::rtl::OUString& rDBNAME,
                        const ::rtl::OUString& rWhat,
                        const ::rtl::OUString& rHow);
            void createNeededDirs(      const ::rtl::OUString& _rDBName);
            sal_Bool isKernelVersion(const char* _pVersion);
            sal_Bool isVersion(     const ::rtl::OUString& _rDBName,
                                    const char* _pVersion);

            void convertOldVersion(     const ::rtl::OUString& _rDBName,
                                        const TDatabaseStruct& _rDbInfo);

            void installSystemTables(   const TDatabaseStruct& _aInfo);

        protected:
            virtual SQLHANDLE EnvironmentHandle(::rtl::OUString &_rPath);
            virtual ~ODriver();
        public:
            explicit ODriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // XInterface
            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);
        private:
            DECLARE_SERVICE_INFO();

            virtual oslGenericFunction getOdbcFunction(sal_Int32 _nIndex)  const;
            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            // XDriver
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException);

            // XDataDefinitionSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByURL( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // XCreateCatalog
            virtual void SAL_CALL createCatalog( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
            // XDropCatalog
            virtual void SAL_CALL dropCatalog( const ::rtl::OUString& catalogName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
        };
    }

}
#endif //_CONNECTIVITY_ADABAS_BDRIVER_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
