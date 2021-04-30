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

SVXCORE_DLLPUBLIC css::uno::Reference< css::container::XIndexReplace > SvxCreateNumRule(SdrModel* pModel);

class SVXCORE_DLLPUBLIC SvxUnoDrawMSFactory : public css::lang::XMultiServiceFactory
{
protected:
    /** abstract SdrModel provider */
    // Every App has a DrwingLayer as internal Tooling, thus ist is
    // not too hard to asl a UnoModel to hand back a DrawingLayer Model in the
    // form of an SdrModel. Thus, returning a reference and make usages
    // safer and easier is justified.
    virtual SdrModel& getSdrModelFromUnoModel() const = 0; //TTTT make reference return

public:
    SvxUnoDrawMSFactory() noexcept {};

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;

    /// @throws css::uno::Exception
    /// @throws css::uno::RuntimeException
    static css::uno::Reference< css::uno::XInterface > createTextField( const OUString& aServiceSpecifier );

    /** fills the given EventObject from the given SdrHint.
        @returns
            true    if the SdrHint could be translated to an EventObject<br>
            false   if not
    */
    static bool createEvent( const SdrModel* pDoc, const SdrHint* pSdrHint, css::document::EventObject& aEvent );

protected:
    ~SvxUnoDrawMSFactory() {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
