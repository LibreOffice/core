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



/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _FTP_FTPCONTENTIDENTIFIER_HXX_
#define _FTP_FTPCONTENTIDENTIFIER_HXX_

#include <vector>
#include "curl.hxx"
#include <curl/easy.h>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include "ftpdirp.hxx"
#include "ftpurl.hxx"


namespace ftp {


    class FTPContentProvider;


    class FTPContentIdentifier
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::ucb::XContentIdentifier
    {
    public:

        FTPContentIdentifier(const rtl::OUString& ident);

        ~FTPContentIdentifier();

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();

        // XTypeProvider

        virtual
        com::sun::star::uno::Sequence<com::sun::star::uno::Type> SAL_CALL
        getTypes(
        )
            throw(
                com::sun::star::uno::RuntimeException
            );

        virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(
        )
            throw(
                com::sun::star::uno::RuntimeException
            );


        // XContentIdentifier

        virtual ::rtl::OUString SAL_CALL
        getContentIdentifier(
        )
            throw (
                com::sun::star::uno::RuntimeException
            );

        virtual ::rtl::OUString SAL_CALL
        getContentProviderScheme(
        )
            throw (
                ::com::sun::star::uno::RuntimeException
            );


    private:

        rtl::OUString m_ident;
    };

}


#endif
