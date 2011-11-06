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


#ifndef _FILPRP_HXX_
#define _FILPRP_HXX_

#include <ucbhelper/macros.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>


namespace fileaccess {

    class shell;

    class XPropertySetInfo_impl
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::beans::XPropertySetInfo
    {
    public:
        XPropertySetInfo_impl( shell* pMyShell,const rtl::OUString& aUnqPath );
        XPropertySetInfo_impl( shell* pMyShell,const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq );

        virtual ~XPropertySetInfo_impl();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()

        virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property > SAL_CALL
        getProperties(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::beans::Property SAL_CALL
        getPropertyByName(
            const rtl::OUString& aName )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        hasPropertyByName( const rtl::OUString& Name )
            throw( com::sun::star::uno::RuntimeException );

    private:
        shell*                                                                     m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >    m_xProvider;
        sal_Int32                                                                  m_count;
        com::sun::star::uno::Sequence< com::sun::star::beans::Property >           m_seq;
    };
}

#endif

