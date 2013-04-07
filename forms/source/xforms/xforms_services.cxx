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

#include "services.hxx"

#include "binding.hxx"
#include "model.hxx"
#include "NameContainer.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::uno::RuntimeException;
using com::sun::star::form::binding::XValueBinding;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameContainer;


namespace frm
{

Reference<XInterface> Model_CreateInstance(
    const Reference<XMultiServiceFactory>& )
    throw( RuntimeException )
{
    return static_cast<XPropertySet*>( static_cast<PropertySetBase*>( new xforms::Model ) );
}

Reference<XInterface> XForms_CreateInstance(
    const Reference<XMultiServiceFactory>& )
    throw( RuntimeException )
{
    return static_cast<XNameContainer*>( new NameContainer<Reference<XPropertySet> >() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
