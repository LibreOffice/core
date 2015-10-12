/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_FMDPAGE_HXX
#define INCLUDED_SVX_FMDPAGE_HXX

#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <svx/unopage.hxx>
#include <comphelper/uno3.hxx>
#include <svx/svxdllapi.h>


// SvxFmDrawPage

class SVX_DLLPUBLIC SvxFmDrawPage   :public SvxDrawPage
                                    ,public ::com::sun::star::form::XFormsSupplier2
{
protected:

    // Creating a SdrObject based on a Description. Cann be used by derived classes to
    // support own ::com::sun::star::drawing::Shapes (for example Controls)
    virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape )
        throw (css::uno::RuntimeException, std::exception) override;

    // The following method is called when a SvxShape object should be created.
    // Derived classes can create a derivation or an object aggregating SvxShape.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const
        throw (css::uno::RuntimeException, std::exception) override;

public:
    SvxFmDrawPage( SdrPage* pPage );
    virtual ~SvxFmDrawPage() throw ();

    // UNO connection
    DECLARE_UNO3_AGG_DEFAULTS(SvxFmDrawPage, SvxDrawPage)

    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XFormsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getForms() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XFormsSupplier2
    virtual sal_Bool SAL_CALL hasForms() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // ::com::sun::star::lang::XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif // INCLUDED_SVX_FMDPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
