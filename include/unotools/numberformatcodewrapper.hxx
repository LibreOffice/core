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

#ifndef INCLUDED_UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX
#define INCLUDED_UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX

#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <unotools/unotoolsdllapi.h>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
}}}

class UNOTOOLS_DLLPUBLIC NumberFormatCodeWrapper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XNumberFormatCode >   xNFC;
    ::com::sun::star::lang::Locale          aLocale;

                                NumberFormatCodeWrapper( const NumberFormatCodeWrapper& ) = delete;
            NumberFormatCodeWrapper&    operator=( const NumberFormatCodeWrapper& ) = delete;

public:
                                NumberFormatCodeWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rxContext,
                                    const ::com::sun::star::lang::Locale& rLocale
                                    );

                                ~NumberFormatCodeWrapper();

    /// set a new Locale
            void                setLocale( const ::com::sun::star::lang::Locale& rLocale );

    // Wrapper implementations of class NumberFormatCodeMapper

    ::com::sun::star::i18n::NumberFormatCode getFormatCode( sal_Int16 nFormatIndex ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > getAllFormatCode( sal_Int16 nFormatUsage ) const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > getAllFormatCodes() const;

};

#endif // INCLUDED_UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
