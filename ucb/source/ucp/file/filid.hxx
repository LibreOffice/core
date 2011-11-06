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


#ifndef _FILID_HXX_
#define _FILID_HXX_

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>

namespace fileaccess {

    class shell;

    class FileContentIdentifier :
        public cppu::OWeakObject,
        public com::sun::star::lang::XTypeProvider,
        public com::sun::star::ucb::XContentIdentifier
    {

        // This implementation has to be reworked
    public:
        FileContentIdentifier( shell* pMyShell,
                               const rtl::OUString& aUnqPath,
                               sal_Bool IsNormalized = true );

        virtual ~FileContentIdentifier();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();

        // XTypeProvider
        virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
        getTypes(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
        getImplementationId(
            void )
            throw( com::sun::star::uno::RuntimeException );

        // XContentIdentifier
        virtual rtl::OUString SAL_CALL
        getContentIdentifier(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual rtl::OUString SAL_CALL
        getContentProviderScheme(
            void )
            throw( com::sun::star::uno::RuntimeException );

    private:
        shell* m_pMyShell;
        rtl::OUString m_aContentId;              // The URL string
        rtl::OUString m_aNormalizedId;           // The somehow normalized string
        rtl::OUString m_aProviderScheme;
        sal_Bool      m_bNormalized;
    };

} // end namespace fileaccess


#endif
