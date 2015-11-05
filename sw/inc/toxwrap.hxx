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

#ifndef INCLUDED_SW_INC_TOXWRAP_HXX
#define INCLUDED_SW_INC_TOXWRAP_HXX

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
    css::lang::Locale aLcl;
    css::uno::Reference < css::i18n::XExtendedIndexEntrySupplier > xIES;

public:
    IndexEntrySupplierWrapper();
    ~IndexEntrySupplierWrapper();

    void SetLocale( const css::lang::Locale& rLocale ) { aLcl = rLocale; }

    OUString GetIndexKey( const OUString& rText, const OUString& rTextReading,
                        const css::lang::Locale& rLocale ) const;

    OUString GetFollowingText( bool bMorePages ) const;

    css::uno::Sequence< OUString >
    GetAlgorithmList( const css::lang::Locale& rLcl ) const;

    bool LoadAlgorithm( const css::lang::Locale& rLcl,
                            const OUString& sSortAlgorithm, long nOptions ) const;

    sal_Int16 CompareIndexEntry( const OUString& rText1, const OUString& rTextReading1,
                                 const css::lang::Locale& rLcl1,
                                 const OUString& rText2, const OUString& rTextReading2,
                                 const css::lang::Locale& rLcl2 ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
