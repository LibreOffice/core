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
#ifndef _SVX_UNOSHGRP_HXX
#define _SVX_UNOSHGRP_HXX

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#include <cppuhelper/implbase3.hxx> 
namespace binfilter {

class XShapeList;

class SvxShapeCollectionMutex
{
public:
    ::osl::Mutex maMutex;
};

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SvxShapeCollection_NewInstance() throw();

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeCollection :	public ::cppu::WeakAggImplHelper3<
                                            ::com::sun::star::drawing::XShapes,
                                            ::com::sun::star::lang::XServiceInfo,
                                            ::com::sun::star::lang::XComponent
                                            >,
                            public SvxShapeCollectionMutex
{
private:
    cppu::OInterfaceContainerHelper maShapeContainer;

    cppu::OBroadcastHelper mrBHelper;

    virtual void disposing() throw();

public:
    SvxShapeCollection() throw();
    virtual ~SvxShapeCollection() throw();

    // XInterface
    virtual void SAL_CALL release() throw();

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

}//end of namespace binfilter
#endif

