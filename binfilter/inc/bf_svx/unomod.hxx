/*************************************************************************
 *
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

#ifndef SVX_UNOMOD_HXX
#define SVX_UNOMOD_HXX

#ifndef _COM_SUN_STAR_DOCUMENT_EVENTOBJECT_HPP_ 
#include <com/sun/star/document/EventObject.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
namespace binfilter {

class SdrHint;
class SdrModel;

class SvxUnoDrawMSFactory : public ::com::sun::star::lang::XMultiServiceFactory
{
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

    /** fills the given EventObject from the given SdrHint.
        @returns
            true	if the SdrHint could be translated to an EventObject<br>
            false	if not
    */
    static sal_Bool createEvent( const SdrModel* pDoc, const SdrHint* pSdrHint, ::com::sun::star::document::EventObject& aEvent );
};

}//end of namespace binfilter
#endif

