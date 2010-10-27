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
#ifndef _SVX_FMDPAGE_HXX
#define _SVX_FMDPAGE_HXX

#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <svx/unopage.hxx>
#include <comphelper/uno3.hxx>
#include "svx/svxdllapi.h"

//==================================================================
// SvxFmDrawPage
//==================================================================
class SVX_DLLPUBLIC SvxFmDrawPage   :public SvxDrawPage
                                    ,public ::com::sun::star::form::XFormsSupplier2
{
protected:

    // Creating a SdrObject based on a Description. Cann be used by derived classes to
    // support own ::com::sun::star::drawing::Shapes (for example Controls)
    virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape )throw ();

    // The following method is called when a SvxShape object should be created.
    // Derived classes can create a derivation or an object aggregating SvxShape.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const throw ();

public:
    SvxFmDrawPage( SdrPage* pPage );
    virtual ~SvxFmDrawPage() throw ();

    // UNO connection
    DECLARE_UNO3_AGG_DEFAULTS(SvxFmDrawPage, SvxDrawPage);

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    // XFormsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getForms(void) throw( ::com::sun::star::uno::RuntimeException );

    // XFormsSupplier2
    virtual sal_Bool SAL_CALL hasForms(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );
};

#endif // _SVX_FMDPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
