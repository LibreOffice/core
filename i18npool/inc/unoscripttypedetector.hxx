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
#ifndef _I18N_UNOSCRIPTTYPEDETECTOR_HXX_
#define _I18N_UNOSCRIPTTYPEDETECTOR_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/i18n/XScriptTypeDetector.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations

//  ----------------------------------------------------
//  class UnoScriptTypeDetector
//  ----------------------------------------------------
class UnoScriptTypeDetector : public cppu::WeakImplHelper2
<
    ::com::sun::star::i18n::XScriptTypeDetector,
    ::com::sun::star::lang::XServiceInfo
>
{
public:
    // Methods
    virtual sal_Int32 SAL_CALL beginOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 scriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getScriptDirection( const ::rtl::OUString& Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL beginOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getCTLScriptType( const ::rtl::OUString& Text, sal_Int32 nPos ) throw (::com::sun::star::uno::RuntimeException);


    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( ::com::sun::star::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
