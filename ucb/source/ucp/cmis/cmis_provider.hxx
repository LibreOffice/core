/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CMIS_PROVIDER_HXX
#define CMIS_PROVIDER_HXX

#include <com/sun/star/beans/Property.hpp>
#include <ucbhelper/providerhelper.hxx>
#include <libcmis/libcmis.hxx>

namespace cmis
{

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
private:
    std::map< OUString, libcmis::Session* > m_aSessionCache;

public:
    ContentProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

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
               ::com::sun::star::uno::RuntimeException, std::exception );

    libcmis::Session* getSession( const OUString& sBindingUrl );
    void registerSession( const OUString& sBindingUrl, libcmis::Session* pSession );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
