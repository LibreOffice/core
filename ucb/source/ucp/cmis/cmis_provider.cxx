/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include "cmis_provider.hxx"
#include "cmis_content.hxx"

#include <stdio.h>

using namespace com::sun::star;

namespace cmis
{
uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
    throw( com::sun::star::ucb::IllegalIdentifierException,
           uno::RuntimeException )
{
#ifdef DEBUG
    fprintf(stderr, "QueryContent: '%s'\n",
       (const sal_Char *)rtl::OUStringToOString
       (Identifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8));
#endif

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent = queryExistingContent( Identifier ).get();
    if ( xContent.is() )
        return xContent;

    try
    {
        xContent = new ::cmis::Content(m_xSMgr, this, Identifier);
    }
    catch ( com::sun::star::ucb::ContentCreationException const & )
    {
        throw com::sun::star::ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw com::sun::star::ucb::IllegalIdentifierException();

    return xContent;
}

ContentProvider::ContentProvider(
    const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
: ::ucbhelper::ContentProviderImplHelper( rSMgr )
{
}

ContentProvider::~ContentProvider()
{
}

XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   com::sun::star::ucb::XContentProvider );

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      com::sun::star::ucb::XContentProvider );

XSERVICEINFO_IMPL_1( ContentProvider,
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                       "com.sun.star.comp.CmisContentProvider" )),
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                       "com.sun.star.ucb.CmisContentProvider" )) );

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory( const sal_Char *pImplName,
    void *pServiceManager, void * )
{
    void * pRet = 0;

    uno::Reference< lang::XMultiServiceFactory > xSMgr
        (reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( !::cmis::ContentProvider::getImplementationName_Static().compareToAscii( pImplName ) )
        xFactory = ::cmis::ContentProvider::createServiceFactory( xSMgr );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
