/*************************************************************************
 *
 *  $RCSfile: ftpcontentprovider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: abi $ $Date: 2002-06-20 14:49:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FTP_FTPCONTENTPROVIDER_HXX_
#define _FTP_FTPCONTENTPROVIDER_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif
#ifndef __CURL_TYPES_H
#include <curl/types.h>
#endif
//  #ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIERFACTORY_HPP_
//  #include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
//  #endif


// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.

#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME         "com.sun.star.ucb.FtpContentProvider"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME_LENGTH  35
#define MYUCP_URL_SCHEME        "ftp"
#define MYUCP_URL_SCHEME_LENGTH 3
#define MYUCP_CONTENT_TYPE      "application/ftp-content"    // UCB Content Type.


/**
 *  Definition of ftpcontentprovider
 */



namespace ftp {


    class FtpLoaderThread;


    class FtpContentProvider:
        public ::ucb::ContentProviderImplHelper
    {
    public:

        FtpContentProvider(const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xMSF );
        ~FtpContentProvider();

        // XInterface
        XINTERFACE_DECL()

        // XTypeProvider
        XTYPEPROVIDER_DECL()

        // XServiceInfo
        XSERVICEINFO_DECL()

        // XContentProvider
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        queryContent(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier )
            throw( com::sun::star::ucb::IllegalIdentifierException,
                   com::sun::star::uno::RuntimeException );


        /**
         *  Commented out, because I currently do not know wether this interface will be useful.
         */

//      // XContentIdentifierFactory

//          virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
//          createContentIdentifier(
//              const rtl::OUString& ContentId )
//              throw( com::sun::star::uno::RuntimeException );

//          virtual sal_Int32 SAL_CALL
//          compareContentIds(
//              const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id1,
//              const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id2 )
//              throw( com::sun::star::uno::RuntimeException );

        CURL* handle();

    private:

        osl::Mutex m_aMutex;
        FtpLoaderThread *m_ftpLoaderThread;
        void init();



    };  // end class FtpContentProvider

}       // end namespace ftp

#endif

