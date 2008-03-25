/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: provider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:22:03 $
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

#ifndef _PROVIDER_HXX
#define _PROVIDER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif


namespace chelp {

//=========================================================================

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.

//#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME         "com.sun.star.ucb.CHelpContentProvider"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME1   "com.sun.star.help.XMLHelp"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME_LENGTH1 25

#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME2  "com.sun.star.ucb.HelpContentProvider"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME_LENGTH2 36

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.

#define MYUCP_URL_SCHEME        "vnd.sun.star.help"
#define MYUCP_URL_SCHEME_LENGTH 18
#define MYUCP_CONTENT_TYPE      "application/vnd.sun.star.xmlhelp"    // UCB Content Type.

//=========================================================================


    class Databases;


    class ContentProvider :
        public ::ucbhelper::ContentProviderImplHelper,
        public ::com::sun::star::container::XContainerListener,
        public ::com::sun::star::lang::XComponent
    {
    public:
        ContentProvider(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );

        virtual ~ContentProvider();

        // XInterface
        XINTERFACE_DECL()

            // XTypeProvider
            XTYPEPROVIDER_DECL()

            // XServiceInfo
            XSERVICEINFO_DECL()

            // XContentProvider
            virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContent > SAL_CALL
        queryContent( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::ucb::XContentIdentifier >& Identifier )
            throw( ::com::sun::star::ucb::IllegalIdentifierException,
                   ::com::sun::star::uno::RuntimeException );

        //////////////////////////////////////////////////////////////////////
        // Additional interfaces
        //////////////////////////////////////////////////////////////////////

        // XComponent

        virtual void SAL_CALL
        dispose(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)xListener;
        }

        virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)aListener;
        }

        // XConainerListener ( deriver from XEventListener )

        virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& Source )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)Source;
            m_xContainer = com::sun::star::uno::Reference<com::sun::star::container::XContainer>(0);
        }

        virtual void SAL_CALL
        elementInserted( const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)Event;
        }

        virtual void SAL_CALL
        elementRemoved( const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException)
        {
            (void)Event;
        }

        virtual void SAL_CALL
        elementReplaced( const ::com::sun::star::container::ContainerEvent& Event )
            throw (::com::sun::star::uno::RuntimeException);


        //////////////////////////////////////////////////////////////////////
        // Non-interface methods.
        //////////////////////////////////////////////////////////////////////

    private:

        osl::Mutex     m_aMutex;
        bool           isInitialized;
        rtl::OUString  m_aScheme;
        Databases*     m_pDatabases;
        com::sun::star::uno::Reference<com::sun::star::container::XContainer> m_xContainer;

        // private methods

        void init();

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
        getConfiguration() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >
        getHierAccess( const ::com::sun::star::uno::Reference<  ::com::sun::star::lang::XMultiServiceFactory >& sProvider,
                       const char* file ) const;

        ::rtl::OUString
        getKey( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
                const char* key ) const;

      sal_Bool
      getBooleanKey(
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::container::XHierarchicalNameAccess >& xHierAccess,
                    const char* key) const;

      void subst( rtl::OUString& instpath ) const;
    };

}

#endif
