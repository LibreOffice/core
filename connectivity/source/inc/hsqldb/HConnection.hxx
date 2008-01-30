/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HConnection.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:02:22 $
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

#ifndef CONNECTIVITY_HSQLDB_CONNECTION_HXX
#define CONNECTIVITY_HSQLDB_CONNECTION_HXX

#ifndef _CONNECTIVITY_ZCONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif
/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef __com_sun_star_sdb_application_XTableUIProvider_hpp__
#include <com/sun/star/sdb/application/XTableUIProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
/** === end UNO includes === **/
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
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
