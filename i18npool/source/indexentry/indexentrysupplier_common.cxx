/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexentrysupplier_common.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:09:35 $
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

#include <indexentrysupplier_common.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <localedata.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier_Common::IndexEntrySupplier_Common(const Reference < lang::XMultiServiceFactory >& rxMSF)
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier_Common";
    collator = new CollatorImpl(rxMSF);
    usePhonetic = sal_False;
}

IndexEntrySupplier_Common::~IndexEntrySupplier_Common()
{
    delete collator;
}

Sequence < lang::Locale > SAL_CALL IndexEntrySupplier_Common::getLocaleList() throw (RuntimeException)
{
    throw RuntimeException();
}

Sequence < OUString > SAL_CALL IndexEntrySupplier_Common::getAlgorithmList( const lang::Locale& rLocale ) throw (RuntimeException)
{
    throw RuntimeException();
}

OUString SAL_CALL IndexEntrySupplier_Common::getPhoneticCandidate( const OUString& rIndexEntry,
    const lang::Locale& rLocale ) throw (RuntimeException)
{
    throw RuntimeException();
}

sal_Bool SAL_CALL IndexEntrySupplier_Common::usePhoneticEntry( const lang::Locale& rLocale ) throw (RuntimeException)
{
    throw RuntimeException();
}

sal_Bool SAL_CALL IndexEntrySupplier_Common::loadAlgorithm( const lang::Locale& rLocale,
    const OUString& rAlgorithm, sal_Int32 collatorOptions ) throw (RuntimeException)
{
    usePhonetic = LocaleData().isPhonetic(rLocale, rAlgorithm);
    collator->loadCollatorAlgorithm(rAlgorithm, rLocale, collatorOptions);
    aLocale = rLocale;
    return sal_True;
}

const OUString& SAL_CALL IndexEntrySupplier_Common::getEntry( const OUString& IndexEntry,
    const OUString& PhoneticEntry, const lang::Locale& rLocale ) throw (RuntimeException)
{
    // The condition for using phonetic entry is:
    // usePhonetic is set for the algorithm;
    // rLocale for phonetic entry is same as aLocale for algorithm,
    // which means Chinese phonetic will not be used for Japanese algorithm;
    // phonetic entry is not blank.
    if (usePhonetic && PhoneticEntry.getLength() > 0 && rLocale.Language == aLocale.Language &&
            rLocale.Country == aLocale.Country && rLocale.Variant == aLocale.Variant)
        return PhoneticEntry;
    else
        return IndexEntry;
}

OUString SAL_CALL IndexEntrySupplier_Common::getIndexKey( const OUString& rIndexEntry,
    const OUString& rPhoneticEntry, const lang::Locale& rLocale ) throw (RuntimeException)
{
    return rIndexEntry.copy(0, 1);
}

sal_Int16 SAL_CALL IndexEntrySupplier_Common::compareIndexEntry(
    const OUString& rIndexEntry1, const OUString& rPhoneticEntry1, const lang::Locale& rLocale1,
    const OUString& rIndexEntry2, const OUString& rPhoneticEntry2, const lang::Locale& rLocale2 )
    throw (RuntimeException)
{
    sal_Int16 result =
            collator->compareString(getEntry(rIndexEntry1, rPhoneticEntry1, rLocale1),
                                    getEntry(rIndexEntry2, rPhoneticEntry2, rLocale2));

    // equivalent of phonetic entries does not mean equivalent of index entries.
    // we have to continue comparing index entry here.
    if (result == 0 && usePhonetic && rPhoneticEntry1.getLength() > 0 &&
            rLocale1.Language == rLocale2.Language && rLocale1.Country == rLocale2.Country &&
            rLocale1.Variant == rLocale2.Variant)
        return collator->compareString(rIndexEntry1, rIndexEntry2);

    return result;
}

OUString SAL_CALL IndexEntrySupplier_Common::getIndexCharacter( const OUString& rIndexEntry,
    const lang::Locale& rLocale, const OUString& rAlgorithm ) throw (RuntimeException)
{
    return rIndexEntry.copy(0, 1);
}

OUString SAL_CALL IndexEntrySupplier_Common::getIndexFollowPageWord( sal_Bool bMorePages,
    const lang::Locale& rLocale ) throw (RuntimeException)
{
    throw RuntimeException();
}

OUString SAL_CALL
IndexEntrySupplier_Common::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii( implementationName );
}

sal_Bool SAL_CALL
IndexEntrySupplier_Common::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName.compareToAscii(implementationName) == 0;
}

Sequence< OUString > SAL_CALL
IndexEntrySupplier_Common::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii( implementationName );
    return aRet;
}

} } } }
