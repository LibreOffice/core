/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexentrysupplier_asian.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-01-31 18:34:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
