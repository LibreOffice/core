/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef UCB_UCPEXT_CONTENT_HXX
#define UCB_UCPEXT_CONTENT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
/** === end UNO includes === **/

#include <rtl/ref.hxx>
#include <ucbhelper/contenthelper.hxx>

#include <list>
#include <boost/optional.hpp>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    //==================================================================================================================
    //= ExtensionContentType
    //==================================================================================================================
    enum ExtensionContentType
    {
        E_ROOT,
        E_EXTENSION_ROOT,
        E_EXTENSION_CONTENT,

        E_UNKNOWN
    };

    //==================================================================================================================
    //= ContentProvider
    //==================================================================================================================
    typedef ::ucbhelper::ContentImplHelper  Content_Base;
    class Content : public Content_Base
    {
    public:
        Content(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            ::ucbhelper::ContentProviderImplHelper* pProvider,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier >& Identifier
        );

        static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
            getArtificialNodePropertyValues(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& rProperties,
                const ::rtl::OUString& rTitle
            );

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
            getPropertyValues(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& rProperties,
                const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& xEnv
            );

        static ::rtl::OUString
            encodeIdentifier( const ::rtl::OUString& i_rIdentifier );
        static ::rtl::OUString
            decodeIdentifier( const ::rtl::OUString& i_rIdentifier );

        virtual ::rtl::OUString getParentURL();

        ExtensionContentType getExtensionContentType() const { return m_eExtContentType; }

        /** retrieves the URL of the underlying physical content. Not to be called when getExtensionContentType()
            returns E_ROOT.
        */
        ::rtl::OUString getPhysicalURL() const;

    protected:
        virtual ~Content();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XContent
        virtual rtl::OUString SAL_CALL getContentType() throw( com::sun::star::uno::RuntimeException );

        // XCommandProcessor
        virtual com::sun::star::uno::Any SAL_CALL
                execute(
                    const com::sun::star::ucb::Command& aCommand,
                    sal_Int32 CommandId,
                    const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& Environment
                )
                throw   (   ::com::sun::star::uno::Exception
                        ,   ::com::sun::star::ucb::CommandAbortedException
                        ,   ::com::sun::star::uno::RuntimeException
                        );

        virtual void SAL_CALL
                abort(
                    sal_Int32 CommandId
                )
                throw   (   ::com::sun::star::uno::RuntimeException
                        );

    private:
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > getProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& i_rEnv );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::ucb::CommandInfo > getCommands( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& i_rEnv );

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
            setPropertyValues(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rValues,
                const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& xEnv
            );

        static bool denotesRootContent( const ::rtl::OUString& i_rContentIdentifier );

        bool impl_isFolder();
        void impl_determineContentType();

    private:
        ExtensionContentType                    m_eExtContentType;
        ::boost::optional< bool >               m_aIsFolder;
        ::boost::optional< ::rtl::OUString >    m_aContentType;
        ::rtl::OUString                         m_sExtensionId;
        ::rtl::OUString                         m_sPathIntoExtension;
    };

//......................................................................................................................
} } }   // namespace ucb::ucp::ext
//......................................................................................................................

#endif  // UCB_UCPEXT_CONTENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
