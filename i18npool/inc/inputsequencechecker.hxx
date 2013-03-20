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

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>

#include <vector>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class InputSequenceCheckerImpl
//  ----------------------------------------------------
class InputSequenceCheckerImpl : public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedInputSequenceChecker,
    com::sun::star::lang::XServiceInfo
>
{
public:
    InputSequenceCheckerImpl( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );
    InputSequenceCheckerImpl();
    ~InputSequenceCheckerImpl();

    virtual sal_Bool SAL_CALL checkInputSequence(const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL correctInputSequence(rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

protected:
    sal_Int16 checkMode;
    const sal_Char *serviceName;

private :
    struct lookupTableItem {
        lookupTableItem(const sal_Char* rLanguage, const com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedInputSequenceChecker >& rxISC) :
            aLanguage(rLanguage), xISC(rxISC) {}
        const sal_Char* aLanguage;
        com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedInputSequenceChecker > xISC;
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_xContext;

    com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedInputSequenceChecker >& SAL_CALL getInputSequenceChecker(sal_Char* rLanguage)
        throw (com::sun::star::uno::RuntimeException);
    sal_Char* SAL_CALL getLanguageByScripType(sal_Unicode cChar, sal_Unicode nChar);
};

} } } }

#endif // _I18N_BREAKITERATOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
