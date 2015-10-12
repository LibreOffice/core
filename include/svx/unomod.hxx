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

#ifndef INCLUDED_SVX_UNOMOD_HXX
#define INCLUDED_SVX_UNOMOD_HXX

#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <svx/svxdllapi.h>

class SdrHint;
class SdrModel;
class SdrModel;

SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > SvxCreateNumRule( SdrModel* pModel ) throw();

class SVX_DLLPUBLIC SvxUnoDrawMSFactory : public ::com::sun::star::lang::XMultiServiceFactory
{
public:
    SvxUnoDrawMSFactory() throw() {};

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createTextField( const OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // internal
    static ::com::sun::star::uno::Sequence< OUString >
        concatServiceNames( ::com::sun::star::uno::Sequence< OUString >& rServices1,
                            ::com::sun::star::uno::Sequence< OUString >& rServices2 ) throw();

    /** fills the given EventObject from the given SdrHint.
        @returns
            true    if the SdrHint could be translated to an EventObject<br>
            false   if not
    */
    static bool createEvent( const SdrModel* pDoc, const SdrHint* pSdrHint, ::com::sun::star::document::EventObject& aEvent );

protected:
    ~SvxUnoDrawMSFactory() {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
