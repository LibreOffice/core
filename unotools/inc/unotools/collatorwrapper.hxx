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
#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#define _UNOTOOLS_COLLATORWRAPPER_HXX

#include <com/sun/star/i18n/XCollator.hpp>

namespace com { namespace sun { namespace star { namespace lang {
        class XMultiServiceFactory;
}}}}

class UNOTOOLS_DLLPUBLIC CollatorWrapper
{
    private:

        ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory
            > mxServiceFactory;

        ::com::sun::star::uno::Reference<
                ::com::sun::star::i18n::XCollator
            > mxInternationalCollator;

    public:

        CollatorWrapper (
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory > &xServiceFactoryIn);

        ~CollatorWrapper();

           sal_Int32
        compareString (
                const OUString& s1, const OUString& s2) const;

        ::com::sun::star::uno::Sequence< OUString >
        listCollatorAlgorithms (
                const ::com::sun::star::lang::Locale& rLocale) const;

        sal_Int32
        loadDefaultCollator (
                const ::com::sun::star::lang::Locale& rLocale, sal_Int32 nOption);

        sal_Int32
        loadCollatorAlgorithm (
                const OUString& rAlgorithm,
                const ::com::sun::star::lang::Locale& rLocale, sal_Int32 nOption);

    protected:

        CollatorWrapper();
};

#endif /* _UNOTOOLS_COLLATORWRAPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
