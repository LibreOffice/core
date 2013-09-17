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

#ifndef _TOXWRAP_HXX
#define _TOXWRAP_HXX

#include <tools/solar.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include "swdllapi.h"

namespace com { namespace sun { namespace star {
namespace i18n {
    class XExtendedIndexEntrySupplier;
}
namespace lang {
    class XMultiServiceFactory;
}
}}}


class SW_DLLPUBLIC IndexEntrySupplierWrapper
{
    ::com::sun::star::lang::Locale aLcl;
    ::com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedIndexEntrySupplier > xIES;

public:
    IndexEntrySupplierWrapper();
    ~IndexEntrySupplierWrapper();

    void SetLocale( const ::com::sun::star::lang::Locale& rLocale ) { aLcl = rLocale; }

    OUString GetIndexKey( const OUString& rTxt, const OUString& rTxtReading,
                        const ::com::sun::star::lang::Locale& rLocale ) const;

    OUString GetFollowingText( sal_Bool bMorePages ) const;

    ::com::sun::star::uno::Sequence< OUString >
    GetAlgorithmList( const ::com::sun::star::lang::Locale& rLcl ) const;

    sal_Bool LoadAlgorithm( const ::com::sun::star::lang::Locale& rLcl,
                            const OUString& sSortAlgorithm, long nOptions ) const;

    sal_Int16 CompareIndexEntry( const OUString& rTxt1, const OUString& rTxtReading1,
                                 const ::com::sun::star::lang::Locale& rLcl1,
                                 const OUString& rTxt2, const OUString& rTxtReading2,
                                 const ::com::sun::star::lang::Locale& rLcl2 ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
