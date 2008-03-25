/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: content.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:20:31 $
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

#ifndef _CONTENT_HXX
#define _CONTENT_HXX

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _UCBHELPER_CONTENTHELPER_HXX
#include <ucbhelper/contenthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#include "urlparameter.hxx"

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace chelp
{

//=========================================================================

// UNO service name for the content.
#define MYUCP_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.CHelpContent"

//=========================================================================

    class Databases;

    struct ContentProperties
    {
        ::rtl::OUString aTitle;         // Title
        ::rtl::OUString aContentType;   // ContentType
        sal_Bool        bIsDocument;    // IsDocument
        sal_Bool        bIsFolder;      // IsFolder

        ContentProperties()
            : bIsDocument( sal_True ), bIsFolder( sal_False ) {}
    };

//=========================================================================

    class Content : public ::ucbhelper::ContentImplHelper
    {
    public:

        Content( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                 ::ucbhelper::ContentProviderImplHelper* pProvider,
                 const ::com::sun::star::uno::Reference<
                 ::com::sun::star::ucb::XContentIdentifier >& Identifier,
                 Databases* pDatabases );

        virtual ~Content();

        // XInterface
        XINTERFACE_DECL()

        // XTypeProvider
        XTYPEPROVIDER_DECL()

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL
        getImplementationName()
            throw( ::com::sun::star::uno::RuntimeException );

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw( ::com::sun::star::uno::RuntimeException );

        // XContent
        virtual rtl::OUString SAL_CALL
        getContentType()
            throw( com::sun::star::uno::RuntimeException );

        // XCommandProcessor
        virtual com::sun::star::uno::Any SAL_CALL
        execute( const com::sun::star::ucb::Command& aCommand,
                 sal_Int32 CommandId,
                 const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::ucb::CommandAbortedException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        abort( sal_Int32 CommandId )
            throw( com::sun::star::uno::RuntimeException );


    private:

        // private members;

        ContentProperties m_aProps;
        URLParameter      m_aURLParameter;
        Databases*        m_pDatabases;


        // private methods

        virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
        getProperties( const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment > & xEnv );
        virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
        getCommands( const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment > & xEnv );

        virtual ::rtl::OUString getParentURL() { return ::rtl::OUString(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
        getPropertyValues( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties );
        void setPropertyValues(
            const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& rValues );


    };

}

#endif
