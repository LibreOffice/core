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

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase3.hxx>

#include <unomodel.hxx>
namespace binfilter {

class SvUnoWeakContainer;

class SdUnoStyleFamilies : public ::cppu::WeakImplHelper3<
                                        ::com::sun::star::lang::XServiceInfo,
                                        ::com::sun::star::container::XIndexAccess,
                                        ::com::sun::star::container::XNameAccess >
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
    SdXImpressDocument*	mpModel;

    sal_Bool mbImpress;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess > mxGraphicStyles;
    SvUnoWeakContainer*	mpStyleFamilies;

    sal_uInt16 getLayoutCount() throw();
    ::rtl::OUString getLayoutNameByIndex( sal_uInt16 nIndex ) throw();
    sal_uInt16 getLayoutIndexByName( const ::rtl::OUString& rName ) throw();

    void createGraphicStyleFamily( ::com::sun::star::uno::Any& rAny ) throw( ::com::sun::star::container::NoSuchElementException ) ;
    void createStyleFamilyByIndex( sal_uInt16 nLayout, ::com::sun::star::uno::Any& rAny ) throw();

public:
    SdUnoStyleFamilies( SdXImpressDocument* pModel ) throw();
    virtual ~SdUnoStyleFamilies() throw();

    sal_Bool isValid() const throw() { return mxModel.is(); }

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

} //namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
