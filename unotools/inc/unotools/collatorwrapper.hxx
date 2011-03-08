/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
        compareSubstring (
                const ::rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
                const ::rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) const;

           sal_Int32
        compareString (
                const ::rtl::OUString& s1, const ::rtl::OUString& s2) const;

        ::com::sun::star::uno::Sequence< ::rtl::OUString >
        listCollatorAlgorithms (
                const ::com::sun::star::lang::Locale& rLocale) const;

        ::com::sun::star::uno::Sequence< sal_Int32 >
        listCollatorOptions (
                const ::rtl::OUString& rAlgorithm) const;

        sal_Int32
        loadDefaultCollator (
                const ::com::sun::star::lang::Locale& rLocale, sal_Int32 nOption);

        sal_Int32
        loadCollatorAlgorithm (
                const ::rtl::OUString& rAlgorithm,
                const ::com::sun::star::lang::Locale& rLocale, sal_Int32 nOption);

        void
        loadCollatorAlgorithmWithEndUserOption (
                const ::rtl::OUString& rAlgorithm,
                const ::com::sun::star::lang::Locale& rLocale,
                const ::com::sun::star::uno::Sequence< sal_Int32 >& rOption);

    protected:

        CollatorWrapper();
};

#endif /* _UNOTOOLS_COLLATORWRAPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
