/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#ifndef INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_SSTATEMENT_HXX
#define INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_SSTATEMENT_HXX

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <cppuhelper/compbase5.hxx>
#include "SConnection.hxx"
#include <vector>
#include "OSubComponent.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity
{
    namespace skeleton
    {

        typedef ::cppu::WeakComponentImplHelper5<   ::com::sun::star::sdbc::XStatement,
                                                    ::com::sun::star::sdbc::XWarningsSupplier,
                                                    ::com::sun::star::util::XCancellable,
                                                    ::com::sun::star::sdbc::XCloseable,
                                                    ::com::sun::star::sdbc::XMultipleResults> OStatement_BASE;

        // A base class for the normal statement and for the prepared statement
        class OStatement_Base       :   public OBase_Mutex,
                                        public  OStatement_BASE,
                                        public  ::cppu::OPropertySetHelper,
                                        public  OPropertyArrayUsageHelper<OStatement_Base>

        {
        ::com::sun::star::sdbc::SQLWarning                            m_aLastWarning;
        protected:
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
            //  for this Statement

            ::std::vector< ::rtl::OUString>               m_aBatchVector;

            OConnection*                                m_pConnection;  // The owning Connection object
        protected:

            void disposeResultSet();

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                                                ::com::sun::star::uno::Any & rConvertedValue,
                                                                ::com::sun::star::uno::Any & rOldValue,
                                                                sal_Int32 nHandle,
                                                                const ::com::sun::star::uno::Any& rValue );
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                                                sal_Int32 nHandle,
                                                                const ::com::sun::star::uno::Any& rValue);
            virtual void SAL_CALL getFastPropertyValue(
                                                                ::com::sun::star::uno::Any& rValue,
                                                                sal_Int32 nHandle) const;
            virtual ~OStatement_Base();

        public:
            ::cppu::OBroadcastHelper& rBHelper;
            OStatement_Base(OConnection* _pConnection );
            using OStatement_BASE::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

            // OComponentHelper
            virtual void SAL_CALL disposing(){OStatement_BASE::disposing();}
            // XInterface
            virtual void SAL_CALL release() SAL_NOEXCEPT;
            virtual void SAL_CALL acquire() SAL_NOEXCEPT;
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType );
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  );

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  );
            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery( const ::rtl::OUString& sql ) ;
            virtual sal_Int32 SAL_CALL executeUpdate( const ::rtl::OUString& sql ) ;
            virtual sal_Bool SAL_CALL execute( const ::rtl::OUString& sql ) ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) ;
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  );
            virtual void SAL_CALL clearWarnings(  );
            // XCancellable
            virtual void SAL_CALL cancel(  );
            // XCloseable
            virtual void SAL_CALL close(  );
            // XMultipleResults
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL getResultSet(  );
            virtual sal_Int32 SAL_CALL getUpdateCount(  );
            virtual sal_Bool SAL_CALL getMoreResults(  );

            // other methods
            OConnection* getOwnConnection() const { return m_pConnection;}
        };

        class OStatement_BASE2  :public OStatement_Base
                                ,public OSubComponent<OStatement_BASE2, OStatement_BASE>

        {
            friend class OSubComponent<OStatement_BASE2, OStatement_BASE>;
        public:
            OStatement_BASE2(OConnection* _pConnection ) :  OStatement_Base(_pConnection ),
                                    OSubComponent<OStatement_BASE2, OStatement_BASE>((::cppu::OWeakObject*)_pConnection, this){}
            // OComponentHelper
            virtual void SAL_CALL disposing();
            // XInterface
            virtual void SAL_CALL release() SAL_NOEXCEPT;
        };

        class OStatement :  public OStatement_BASE2,
                            public ::com::sun::star::sdbc::XBatchExecution,
                            public ::com::sun::star::lang::XServiceInfo
        {
        protected:
            virtual ~OStatement(){}
        public:
            // a constructor, which is required for returning objects:
            OStatement( OConnection* _pConnection) : OStatement_BASE2( _pConnection){}
            DECLARE_SERVICE_INFO();

            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType );
            virtual void SAL_CALL acquire() SAL_NOEXCEPT;
            virtual void SAL_CALL release() SAL_NOEXCEPT;
            // XBatchExecution
            virtual void SAL_CALL addBatch( const ::rtl::OUString& sql );
            virtual void SAL_CALL clearBatch(  );
            virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  );
        };
    }
}

#endif // INCLUDED_EXAMPLES_DATABASE_DRIVERSKELETON_SSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
