/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef CONNECTIVITY_HSQLDB_CONNECTION_HXX
#define CONNECTIVITY_HSQLDB_CONNECTION_HXX

#include "connectivity/ConnectionWrapper.hxx"
/** === begin UNO includes === **/
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#ifndef __com_sun_star_sdb_application_XTableUIProvider_hpp__
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#endif
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
/** === end UNO includes === **/
#include <cppuhelper/compbase2.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <memory>

namespace connectivity
{
    namespace hsqldb
    {
        class SAL_NO_VTABLE IMethodGuardAccess
        {
        public:
            virtual ::osl::Mutex&   getMutex() const = 0;
            virtual void            checkDisposed() const = 0;
        };

        //==========================================================================
        //= OHsqlConnection - wraps all methods to the real connection from the driver
        //= but when disposed it doesn't dispose the real connection
        //==========================================================================
        typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::util::XFlushable
                                                ,   ::com::sun::star::sdb::application::XTableUIProvider
                                                >   OHsqlConnection_BASE;

        class OHsqlConnection   :public ::comphelper::OBaseMutex
                                ,public OHsqlConnection_BASE
                                ,public OConnectionWrapper
                                ,public IMethodGuardAccess
        {
        private:
            ::cppu::OInterfaceContainerHelper                                                           m_aFlushListeners;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >                         m_xDriver;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >            m_xORB;
            bool                                                                                        m_bIni;
            bool                                                                                        m_bReadOnly;

        protected:
            virtual void SAL_CALL disposing(void);
            virtual ~OHsqlConnection();

        public:
            OHsqlConnection(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > _rxDriver,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB
            );

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            DECLARE_XTYPEPROVIDER()
            DECLARE_XINTERFACE( )

            // IMethodGuardAccess
            virtual ::osl::Mutex&   getMutex() const;
            virtual void            checkDisposed() const;

            // XFlushable
            virtual void SAL_CALL flush(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);

            // XTableUIProvider
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL getTableIcon( const ::rtl::OUString& TableName, ::sal_Int32 ColorMode ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getTableEditor( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& DocumentUI, const ::rtl::OUString& TableName ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        private:
#if 0
        // TODO: resource
            /** creates the dialog used for editing a linked table

                @param _rTableName
                    the name of the table to create the editor for.

                @param _rxDocumentUI
                    the UI of the database document, for which the editor is to be created.
                    Must not be <NULL/>.

                @return
                    the table editor dialog instance.

                @throws ::com::sun::star::lang::WrappedTargetException
                    if creating the dialog instance fails

                @throws ::com::sun::star::uno::RuntimeException
                    if a serious error occures

                @precond
                    Our mutex is locked.
            */
            ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XExecutableDialog >
                    impl_createLinkedTableEditor_throw(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::application::XDatabaseDocumentUI >& _rxDocumentUI,
                        const ::rtl::OUString& _rTableName
                    );
#endif

            /** retrieves our table container
                @return
                    our table container. Guaranteed to not be <NULL/>.
                @throws ::com::sun::star::lang::WrappedTargetException
                    if a non-RuntimeException is caught during obtaining the container.
                @throws ::com::sun::star::uno::RuntimeException
                    if a serious error occurs
                @precond
                    We're not disposed.
            */
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    impl_getTableContainer_throw();

            /** checks whether the given table name denotes an existing table
                @param _rTableName
                    the fully name of the table to check for existence
                @throws ::com::sun::star::lang::IllegalArgumentException
                    if the name does not denote an existing table
                @precond
                    We're not disposed.
            */
            void    impl_checkExistingTable_throw( const ::rtl::OUString& _rTableName );

            /** checks whether the given table name refers to a HSQL TEXT TABLE
            */
            bool    impl_isTextTable_nothrow( const ::rtl::OUString& _rTableName );

            /** retrieves the icon for HSQL TEXT TABLEs
            */
            ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                impl_getTextTableIcon_nothrow( ::sal_Int32 _ColorMode );
        };

        //==========================================================================
        //= OHsqlConnection
        //==========================================================================
        class MethodGuard : public ::osl::MutexGuard
        {
        private:
            typedef ::osl::MutexGuard   BaseGuard;

        public:
            MethodGuard( const IMethodGuardAccess& _rComponent )
                :BaseGuard( _rComponent.getMutex() )
            {
                _rComponent.checkDisposed();
            }
        };
    }
}
#endif // CONNECTIVITY_HSQLDB_CONNECTION_HXX
