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
#ifndef INCLUDED_CPPUHELPER_COMPONENT_CONTEXT_HXX
#define INCLUDED_CPPUHELPER_COMPONENT_CONTEXT_HXX

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/cppuhelperdllapi.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

namespace cppu
{

/** Context entries init struct calling createComponentContext().
*/
struct SAL_WARN_UNUSED ContextEntry_Init
{
    /** late init denotes a object that will be raised when first get() is calling for it

        The context implementation expects either a css::lang::XSingleComponentFactory
        object as value (to instantiate the object) or a string as value for raising
        a service via the used service manager.
    */
    bool bLateInitService;
    /** name of context value
    */
    ::rtl::OUString name;
    /** context value
    */
    css::uno::Any value;

    /** Default ctor.
    */
    ContextEntry_Init()
        : bLateInitService( false )
        {}
    /** Ctor.

        @param name_
               name of entry
        @param value_
               value of entry
        @param bLateInitService_
               whether this entry is a late-init named object entry
               (value is object factory or service string)
    */
    ContextEntry_Init(
        ::rtl::OUString const & name_,
        css::uno::Any const & value_,
        bool bLateInitService_ = false )
            : bLateInitService( bLateInitService_ ),
              name( name_ ),
              value( value_ )
        {}
};

/** Creates a component context with the given entries.

    @param pEntries array of entries
    @param nEntries number of entries
    @param xDelegate delegation to further context, if value was not found
    @return new context object
*/
CPPUHELPER_DLLPUBLIC css::uno::Reference< css::uno::XComponentContext >
SAL_CALL createComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    css::uno::Reference< css::uno::XComponentContext > const & xDelegate =
    css::uno::Reference< css::uno::XComponentContext >() );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
