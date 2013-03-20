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
#pragma once
#if 1

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XCollator.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/module.h>

#include "warnings_guard_unicode_tblcoll.h"

//      ----------------------------------------------------
//      class Collator_Unicode
//      ----------------------------------------------------

namespace com { namespace sun { namespace star { namespace i18n {

class Collator_Unicode : public cppu::WeakImplHelper1 < XCollator >
{
public:
    // Constructors
    Collator_Unicode();
    // Destructor
    ~Collator_Unicode();

    sal_Int32 SAL_CALL compareSubstring( const rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) throw(com::sun::star::uno::RuntimeException);

    sal_Int32 SAL_CALL compareString( const rtl::OUString& s1, const rtl::OUString& s2)
        throw(com::sun::star::uno::RuntimeException);

    sal_Int32 SAL_CALL loadCollatorAlgorithm( const rtl::OUString& impl, const lang::Locale& rLocale,
        sal_Int32 collatorOptions) throw(com::sun::star::uno::RuntimeException);


    // following 4 methods are implemented in collatorImpl.
    sal_Int32 SAL_CALL loadDefaultCollator( const lang::Locale&,  sal_Int32)
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    void SAL_CALL loadCollatorAlgorithmWithEndUserOption( const rtl::OUString&, const lang::Locale&,
        const com::sun::star::uno::Sequence< sal_Int32 >&) throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL listCollatorAlgorithms( const lang::Locale&)
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}
    com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions( const rtl::OUString& )
        throw(com::sun::star::uno::RuntimeException) {throw com::sun::star::uno::RuntimeException();}

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

protected:
    const sal_Char *implementationName;
private:
    RuleBasedCollator *uca_base, *collator;
#ifndef DISABLE_DYNLOADING
    oslModule hModule;
#endif
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
