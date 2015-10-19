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
#ifndef INCLUDED_I18NPOOL_INC_INPUTSEQUENCECHECKER_HXX
#define INCLUDED_I18NPOOL_INC_INPUTSEQUENCECHECKER_HXX

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedInputSequenceChecker.hpp>

#include <vector>

namespace com { namespace sun { namespace star { namespace i18n {


//  class InputSequenceCheckerImpl

class InputSequenceCheckerImpl : public cppu::WeakImplHelper
<
    css::i18n::XExtendedInputSequenceChecker,
    css::lang::XServiceInfo
>
{
public:
    InputSequenceCheckerImpl( const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    InputSequenceCheckerImpl(const char *pServiceName);
    virtual ~InputSequenceCheckerImpl();

    virtual sal_Bool SAL_CALL checkInputSequence(const OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL correctInputSequence(OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

private:
    const sal_Char *serviceName;

    struct lookupTableItem {
        lookupTableItem(const sal_Char* rLanguage, const css::uno::Reference < css::i18n::XExtendedInputSequenceChecker >& rxISC) :
            aLanguage(rLanguage), xISC(rxISC) {}
        const sal_Char* aLanguage;
        css::uno::Reference < css::i18n::XExtendedInputSequenceChecker > xISC;
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    css::uno::Reference < css::uno::XComponentContext > m_xContext;

    css::uno::Reference< css::i18n::XExtendedInputSequenceChecker >& SAL_CALL getInputSequenceChecker(sal_Char* rLanguage)
        throw (css::uno::RuntimeException);
    static sal_Char* SAL_CALL getLanguageByScripType(sal_Unicode cChar, sal_Unicode nChar);
};

} } } }

#endif // INCLUDED_I18NPOOL_INC_INPUTSEQUENCECHECKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
