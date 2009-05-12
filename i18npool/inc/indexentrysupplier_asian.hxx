/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: indexentrysupplier_asian.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _I18N_INDEXENTRYSUPPLIER_ASIAN_HXX_
#define _I18N_INDEXENTRYSUPPLIER_ASIAN_HXX_

#include <indexentrysupplier_common.hxx>
#include <osl/module.h>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class IndexEntrySupplier_asian
//  ----------------------------------------------------

class IndexEntrySupplier_asian : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_asian( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
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
private:
    oslModule hModule;
};

} } } }
#endif
