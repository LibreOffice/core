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

#ifndef _UNOTOOLS_NATIVENUMBERWRAPPER_HXX
#define _UNOTOOLS_NATIVENUMBERWRAPPER_HXX
#include <com/sun/star/i18n/XNativeNumberSupplier.hpp>

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}


class UNOTOOLS_DLLPUBLIC NativeNumberWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XNativeNumberSupplier >   xNNS;
                                // not implemented, prevent usage
                                NativeNumberWrapper( const NativeNumberWrapper& );
            NativeNumberWrapper&    operator=( const NativeNumberWrapper& );

public:
                                NativeNumberWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF
                                    );

                                ~NativeNumberWrapper();

    // Wrapper implementations of XNativeNumberSupplier

    ::rtl::OUString getNativeNumberString(
                        const ::rtl::OUString& rNumberString,
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    sal_Bool        isValidNatNum(
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    ::com::sun::star::i18n::NativeNumberXmlAttributes convertToXmlAttributes(
                        const ::com::sun::star::lang::Locale& rLocale,
                        sal_Int16 nNativeNumberMode ) const;

    sal_Int16       convertFromXmlAttributes(
                        const ::com::sun::star::i18n::NativeNumberXmlAttributes& rAttr ) const;

};

#endif // _UNOTOOLS_NATIVENUMBERWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
