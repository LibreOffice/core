/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier_ja_phonetic.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: khong $Date: 2002/06/18 22:29:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define INDEXENTRYSUPPLIER_ja_phonetic
#include <indexentrysupplier_asian.hxx>
#include <data/indexdata_ja_phonetic.h>
#include <strings.h>

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
        return collator->compareString(
            PhoneticEntry1.getLength() > 0 ? PhoneticEntry1 : IndexEntry1,
            PhoneticEntry2.getLength() > 0 ? PhoneticEntry2 : IndexEntry2);
    else
        return result;
}

static sal_Char first[] = "ja_phonetic_alphanumeric_first";
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable::loadAlgorithm(
    const com::sun::star::lang::Locale& rLocale, const rtl::OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (com::sun::star::uno::RuntimeException)
{
    aSortAlgorithm = SortAlgorithm;
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(rtl::OUString::createFromAscii(first), rLocale, collatorOptions) == 0;
}
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_consonant::loadAlgorithm(
    const com::sun::star::lang::Locale& rLocale, const rtl::OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (com::sun::star::uno::RuntimeException)
{
    aSortAlgorithm = SortAlgorithm;
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(rtl::OUString::createFromAscii(first), rLocale, collatorOptions) == 0;
}
static sal_Char last[] = "ja_phonetic_alphanumeric_last";
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable::loadAlgorithm(
    const com::sun::star::lang::Locale& rLocale, const rtl::OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (com::sun::star::uno::RuntimeException)
{
    aSortAlgorithm = SortAlgorithm;
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(rtl::OUString::createFromAscii(last), rLocale, collatorOptions) == 0;
}
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_consonant::loadAlgorithm(
    const com::sun::star::lang::Locale& rLocale, const rtl::OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (com::sun::star::uno::RuntimeException)
{
    aSortAlgorithm = SortAlgorithm;
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(rtl::OUString::createFromAscii(last), rLocale, collatorOptions) == 0;
}

} } } }
