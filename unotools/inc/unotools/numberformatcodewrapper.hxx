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

#ifndef _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX
#define _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include "unotools/unotoolsdllapi.h"

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}


class UNOTOOLS_DLLPUBLIC NumberFormatCodeWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XNumberFormatCode >   xNFC;
    ::com::sun::star::lang::Locale          aLocale;

                                // not implemented, prevent usage
                                NumberFormatCodeWrapper( const NumberFormatCodeWrapper& );
            NumberFormatCodeWrapper&    operator=( const NumberFormatCodeWrapper& );

public:
                                NumberFormatCodeWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    const ::com::sun::star::lang::Locale& rLocale
                                    );

                                ~NumberFormatCodeWrapper();


    /// set a new Locale
            void                setLocale( const ::com::sun::star::lang::Locale& rLocale );

    /// get current Locale
    const ::com::sun::star::lang::Locale& getLocale() const { return aLocale; }


    // Wrapper implementations of class NumberFormatCodeMapper

    ::com::sun::star::i18n::NumberFormatCode getDefault( sal_Int16 nFormatType, sal_Int16 nFormatUsage ) const;
    ::com::sun::star::i18n::NumberFormatCode getFormatCode( sal_Int16 nFormatIndex ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > getAllFormatCode( sal_Int16 nFormatUsage ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > getAllFormatCodes() const;

};

#endif // _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
