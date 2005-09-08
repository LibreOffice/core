/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HDriver.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:14:43 $
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
#ifndef CONNECTIVITY_HSQLDB_DRIVER_HXX
#define CONNECTIVITY_HSQLDB_DRIVER_HXX

#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCREATECATALOG_HPP_
#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONLISTENER_HPP_
#include <com/sun/star/embed/XTransactionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    class OMetaConnection;

    namespace hsqldb
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ODriverDelegator_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

        typedef ::cppu::WeakComponentImplHelper5<   ::com::sun::star::sdbc::XDriver
                                        ,::com::sun::star::sdbcx::XDataDefinitionSupplier
                                        , ::com::sun::star::lang::XServiceInfo
                                        , ::com::sun::star::sdbcx::XCreateCatalog
                                        , ::com::sun::star::embed::XTransactionListener
                                        >   ODriverDelegator_BASE;

        typedef ::std::pair< ::com::sun::star::uno::WeakReferenceHelper,::com::sun::star::uno::WeakReferenceHelper> TWeakRefPair;
        typedef ::std::pair< ::rtl::OUString ,TWeakRefPair > TWeakConnectionPair;

        typedef ::std::pair< ::com::sun::star::uno::WeakReferenceHelper,TWeakConnectionPair> TWeakPair;
        typedef ::std::vector< TWeakPair > TWeakPairVector;


        /** delegates all calls to the orignal driver and extend the existing one with the SDBCX layer.

        */
        class ODriverDelegator : public ::comphelper::OBaseMutex
                                ,public ODriverDelegator_BASE
        {
            TWeakPairVector                                                     m_aConnections; //  vector containing a list
                                                                                                //  of all the Connection objects
                                                                                                //  for this Driver
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >                 m_xDriver;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xFactory;
            sal_Bool                                                                            m_bInShutDownConnections;

            /** load the driver we want to delegate.
                The <member>m_xDriver</member> may be <NULL/> if the driver could not be loaded.
                @return
                    The driver which was currently selected.
            */
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > loadDriver( );

            /** shut down the connection and revoke the storage from the map
                @param  _aIter
                    The connection to shut down and storage to revoke.
            */
            void shutdownConnection(const TWeakPairVector::iterator& _aIter);

        public:
            /** creates a new delegator for a HSQLDB driver
            */
            ODriverDelegator(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (::com::sun::star::uno::RuntimeException);

            // XDataDefinitionSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByURL( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            // XCreateCatalog
            virtual void SAL_CALL createCatalog( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

            // XTransactionListener
            virtual void SAL_CALL preCommit( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL commited( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL preRevert( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL reverted( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

            void shutdownConnections();
        protected:
            /// dtor
            virtual ~ODriverDelegator();
            // OComponentHelper
            virtual void SAL_CALL disposing(void);

            /** called when we connected to a newly created embedded database
            */
            void onConnectedNewDatabase(
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
            );
        };
    }

//........................................................................
}   // namespace connectivity
//........................................................................
#endif // CONNECTIVITY_HSQLDB_DRIVER_HXX

