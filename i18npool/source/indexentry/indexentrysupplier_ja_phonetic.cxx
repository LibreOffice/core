/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexentrysupplier_ja_phonetic.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:10:14 $
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

#define INDEXENTRYSUPPLIER_ja_phonetic
#include <indexentrysupplier_asian.hxx>
#include <data/indexdata_ja_phonetic.h>
#include <string.h>

namespace com { namespace sun { namespace star { namespace i18n {

rtl::OUString SAL_CALL IndexEntrySupplier_ja_phonetic::getIndexCharacter( const rtl::OUString& rIndexEntry,
    const lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm )
    throw (com::sun::star::uno::RuntimeException)
{
    return IndexEntrySupplier_CJK::getIndexString(rIndexEntry.toChar(), idx,
            strstr(implementationName, "syllable") ? syllable : consonant);
}

rtl::OUString SAL_CALL IndexEntrySupplier_ja_phonetic::getIndexKey( const rtl::OUString& IndexEntry,
    const rtl::OUString& PhoneticEntry, const lang::Locale& rLocale )
    throw (com::sun::star::uno::RuntimeException)
{
    return IndexEntrySupplier_CJK::getIndexString(
        PhoneticEntry.getLength() > 0 ? PhoneticEntry.toChar() : IndexEntry.toChar(), idx,
            strstr(implementationName, "syllable") ? syllable : consonant);
}

sal_Int16 SAL_CALL IndexEntrySupplier_ja_phonetic::compareIndexEntry(
    const rtl::OUString& IndexEntry1, const rtl::OUString& PhoneticEntry1, const lang::Locale& rLocale1,
    const rtl::OUString& IndexEntry2, const rtl::OUString& PhoneticEntry2, const lang::Locale& rLocale2 )
    throw (com::sun::star::uno::RuntimeException)
{
    sal_Int16 result =  collator->compareString(
        IndexEntrySupplier_ja_phonetic::getIndexKey(IndexEntry1, PhoneticEntry1, rLocale1),
        IndexEntrySupplier_ja_phonetic::getIndexKey(IndexEntry2, PhoneticEntry2, rLocale2));

    if (result == 0)
        return IndexEntrySupplier_Common::compareIndexEntry(
                    IndexEntry1, PhoneticEntry1, rLocale1,
                    IndexEntry2, PhoneticEntry2, rLocale2);
    return result;
}

static sal_Char first[] = "ja_phonetic (alphanumeric first)";
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable::loadAlgorithm(
    const com::sun::star::lang::Locale& rLocale, const rtl::OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (com::sun::star::uno::RuntimeException)
{
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(rtl::OUString::createFromAscii(first), rLocale, collatorOptions) == 0;
}
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_consonant::loadAlgorithm(
    const com::sun::star::lang::Locale& rLocale, const rtl::OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (com::sun::star::uno::RuntimeException)
{
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(rtl::OUString::createFromAscii(first), rLocale, collatorOptions) == 0;
}

static sal_Char last[] = "ja_phonetic (alphanumeric last)";
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable::loadAlgorithm(
    const com::sun::star::lang::Locale& rLocale, const rtl::OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (com::sun::star::uno::RuntimeException)
{
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(rtl::OUString::createFromAscii(last), rLocale, collatorOptions) == 0;
}
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_consonant::loadAlgorithm(
    const com::sun::star::lang::Locale& rLocale, const rtl::OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (com::sun::star::uno::RuntimeException)
{
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(rtl::OUString::createFromAscii(last), rLocale, collatorOptions) == 0;
}

} } } }
