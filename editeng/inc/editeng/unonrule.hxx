/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVX_UNONRULE_HXX
#define _SVX_UNONRULE_HXX

#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/ucb/XAnyCompare.hpp>
#include "editeng/editengdllapi.h"
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <editeng/numitem.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > SvxCreateNumRule( const SvxNumRule* pRule ) throw();
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > SvxCreateNumRule() throw();
const SvxNumRule& SvxGetNumRule( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > xRule ) throw( ::com::sun::star::lang::IllegalArgumentException );
bool SvxGetNumRule( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > xRule, SvxNumRule& rNumRule );
EDITENG_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > SvxCreateNumRuleCompare() throw();

class SvxUnoNumberingRules : public ::cppu::WeakAggImplHelper5< com::sun::star::container::XIndexReplace, com::sun::star::ucb::XAnyCompare,
    com::sun::star::lang::XUnoTunnel, com::sun::star::util::XCloneable, com::sun::star::lang::XServiceInfo >
{
private:
    SvxNumRule maRule;
public:
    SvxUnoNumberingRules( const SvxNumRule& rRule ) throw();
    virtual ~SvxUnoNumberingRules() throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoNumberingRules )

    //XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const com::sun::star::uno::Any& Element ) throw(
    com::sun::star::lang::IllegalArgumentException, com::sun::star::lang::IndexOutOfBoundsException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(com::sun::star::uno::RuntimeException) ;
    virtual com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(com::sun::star::lang::IndexOutOfBoundsException,
        com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException);

    //XElementAccess
    virtual com::sun::star::uno::Type SAL_CALL getElementType() throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(com::sun::star::uno::RuntimeException);

    // XAnyCompare
    virtual sal_Int16 SAL_CALL compare( const com::sun::star::uno::Any& Any1, const com::sun::star::uno::Any& Any2 ) throw(com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual com::sun::star::uno::Reference< com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(  ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(com::sun::star::uno::RuntimeException);

    // internal
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> getNumberingRuleByIndex( sal_Int32 nIndex) const throw();
    void setNumberingRuleByIndex( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rProperties, sal_Int32 nIndex) throw( com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException );

    static sal_Int16 Compare( const com::sun::star::uno::Any& rAny1, const com::sun::star::uno::Any& rAny2 );

    const SvxNumRule& getNumRule() const { return maRule; }
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
