/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier_default.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: khong $ $Date: 2002-06-18 22:29:26 $
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

#include <sal/types.h>
#include <indexentrysupplier_default.hxx>
#include <data/indexdata_unicode.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier_Unicode::IndexEntrySupplier_Unicode(const Reference < XMultiServiceFactory >& rxMSF) :
    IndexEntrySupplier(rxMSF)
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier_Unicode";
    collator = new CollatorImpl(rxMSF);
    usePhonetic = sal_False;
}

IndexEntrySupplier_Unicode::~IndexEntrySupplier_Unicode()
{
    delete collator;
}

sal_Bool SAL_CALL IndexEntrySupplier_Unicode::loadAlgorithm( const Locale& rLocale,
    const OUString& SortAlgorithm, sal_Int32 collatorOptions ) throw (RuntimeException)
{
    aSortAlgorithm = SortAlgorithm;
    aLocale = rLocale;
    return collator->loadCollatorAlgorithm(SortAlgorithm, rLocale, collatorOptions) == 0;
}

const OUString& SAL_CALL IndexEntrySupplier_Unicode::getEntry( const OUString& IndexEntry,
    const OUString& PhoneticEntry, const Locale& rLocale ) throw (RuntimeException)
{
    // The condition for using phonetic entry is:
    // usePhonetic is set for the algorithm;
    // rLocale for phonetic entry is same as aLocale for algorithm,
    // which means Chinese phonetic will not be used for Japanese algorithm;
    // phonetic entry is not blank.
    if (usePhonetic && rLocale == aLocale && PhoneticEntry.getLength() > 0)
        return PhoneticEntry;
    else
        return IndexEntry;
}

OUString SAL_CALL IndexEntrySupplier_Unicode::getIndexKey( const OUString& IndexEntry,
    const OUString& PhoneticEntry, const Locale& rLocale ) throw (RuntimeException)
{
    return getIndexCharacter(getEntry(IndexEntry, PhoneticEntry, rLocale), aLocale, OUString());
}

// this method can be overwriten by sub class for better performing key comparison.
sal_Int16 SAL_CALL IndexEntrySupplier_Unicode::compareIndexKey(
    const OUString& IndexEntry1, const OUString& PhoneticEntry1, const Locale& rLocale1,
    const OUString& IndexEntry2, const OUString& PhoneticEntry2, const Locale& rLocale2 )
    throw (RuntimeException)
{
    return collator->compareString( getIndexKey(IndexEntry1, PhoneticEntry1, rLocale1),
                    getIndexKey(IndexEntry2, PhoneticEntry2, rLocale2));
}

sal_Int16 SAL_CALL IndexEntrySupplier_Unicode::compareIndexEntry(
    const OUString& IndexEntry1, const OUString& PhoneticEntry1, const Locale& rLocale1,
    const OUString& IndexEntry2, const OUString& PhoneticEntry2, const Locale& rLocale2 )
    throw (RuntimeException)
{
    sal_Int16 result = compareIndexKey( IndexEntry1, PhoneticEntry1, rLocale1,
                        IndexEntry2, PhoneticEntry2, rLocale2);
    if (result == 0)
        return collator->compareString( getEntry(IndexEntry1, PhoneticEntry1, rLocale1),
                        getEntry(IndexEntry2, PhoneticEntry2, rLocale2));
    return 0;
}

OUString SAL_CALL IndexEntrySupplier_Unicode::getIndexCharacter( const OUString& rIndexEntry,
    const Locale& rLocale, const OUString& rSortAlgorithm ) throw (RuntimeException) {
    sal_uInt16 ch = rIndexEntry.toChar();
    sal_uInt16 address = idx[ch >> 8];
    return OUString((address != 0xFFFF ? &idxStr[address + (ch & 0xFF)] : &ch), 1);
}

} } } }
