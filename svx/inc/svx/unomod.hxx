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



#ifndef SVX_UNOMOD_HXX
#define SVX_UNOMOD_HXX

#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include "svx/svxdllapi.h"

class SdrBaseHint;
class SdrModel;
class SdrModel;

SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > SvxCreateNumRule( SdrModel* pModel ) throw();

class SVX_DLLPUBLIC SvxUnoDrawMSFactory : public ::com::sun::star::lang::XMultiServiceFactory
{
protected:
    /** abstract SdrModel provider */
    virtual SdrModel* getSdrModel() const = 0;

public:
    SvxUnoDrawMSFactory() throw() {};

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createTextField( const ::rtl::OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // internal
    ::com::sun::star::uno::Sequence< ::rtl::OUString >
        concatServiceNames( ::com::sun::star::uno::Sequence< ::rtl::OUString >& rServices1,
                            ::com::sun::star::uno::Sequence< ::rtl::OUString >& rServices2 ) throw();

    /** fills the given EventObject from the given SdrBaseHint.
        @returns
            true    if the SdrBaseHint could be translated to an EventObject<br>
            false   if not
    */
    static sal_Bool createEvent( const SdrModel* pDoc, const SdrBaseHint* pSdrHint, ::com::sun::star::document::EventObject& aEvent );
};

#endif

