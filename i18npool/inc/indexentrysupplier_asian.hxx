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

#include <indexentrysupplier_common.hxx>
#include <osl/module.h>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class IndexEntrySupplier_asian
//  ----------------------------------------------------

class IndexEntrySupplier_asian : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_asian( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );
    ~IndexEntrySupplier_asian();

    rtl::OUString SAL_CALL getIndexCharacter( const rtl::OUString& rIndexEntry,
            const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rAlgorithm )
            throw (com::sun::star::uno::RuntimeException);
    rtl::OUString SAL_CALL getIndexKey( const rtl::OUString& rIndexEntry,
            const rtl::OUString& rPhoneticEntry, const com::sun::star::lang::Locale& rLocale)
            throw (com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL compareIndexEntry(
            const rtl::OUString& rIndexEntry1, const rtl::OUString& rPhoneticEntry1,
            const com::sun::star::lang::Locale& rLocale1,
            const rtl::OUString& rIndexEntry2, const rtl::OUString& rPhoneticEntry2,
            const com::sun::star::lang::Locale& rLocale2 )
            throw (com::sun::star::uno::RuntimeException);
    rtl::OUString SAL_CALL getPhoneticCandidate( const rtl::OUString& rIndexEntry,
            const com::sun::star::lang::Locale& rLocale )
            throw (com::sun::star::uno::RuntimeException);
#ifndef DISABLE_DYNLOADING
private:
    oslModule hModule;
#endif
};

} } } }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
