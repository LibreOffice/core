/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier_default.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:37:18 $
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
#ifndef _I18N_INDEXENTRYSUPPLIER_DEFAULT_HXX_
#define _I18N_INDEXENTRYSUPPLIER_DEFAULT_HXX_

#include <indexentrysupplier_common.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class Index;

//  ----------------------------------------------------
//  class IndexEntrySupplier_Unicode
//  ----------------------------------------------------
class IndexEntrySupplier_Unicode : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_Unicode( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~IndexEntrySupplier_Unicode();

    virtual sal_Bool SAL_CALL loadAlgorithm(
        const com::sun::star::lang::Locale& rLocale,
        const rtl::OUString& SortAlgorithm, sal_Int32 collatorOptions )
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getIndexKey( const rtl::OUString& IndexEntry,
        const rtl::OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL compareIndexEntry( const rtl::OUString& IndexEntry1,
        const rtl::OUString& PhoneticEntry1, const com::sun::star::lang::Locale& rLocale1,
        const rtl::OUString& IndexEntry2, const ::rtl::OUString& PhoneticEntry2,
        const com::sun::star::lang::Locale& rLocale2 )
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getIndexCharacter( const rtl::OUString& rIndexEntry,
        const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm )
        throw (com::sun::star::uno::RuntimeException);

private:
    Index *index;
};

struct IndexKey {
    sal_Unicode key;
    rtl::OUString desc;
};

class IndexTable
{
public:
    IndexTable();
    ~IndexTable();

    void init(sal_Unicode start_, sal_Unicode end_, IndexKey* keys, sal_Int16 key_count, Index *index);

    sal_Unicode start;
    sal_Unicode end;
    sal_uInt8 *table;
};

#define MAX_KEYS 0xff
#define MAX_TABLES 20

class Index
{
public:
    Index(CollatorImpl *col);

    void init(const com::sun::star::lang::Locale& rLocale, const rtl::OUString& algorithm) throw (com::sun::star::uno::RuntimeException);

    void makeIndexKeys(const com::sun::star::lang::Locale &rLocale, const rtl::OUString &algorithm) throw (com::sun::star::uno::RuntimeException);
    sal_Int16 getIndexWeight(const rtl::OUString& rIndexEntry);
    rtl::OUString getIndexDescription(const rtl::OUString& rIndexEntry);

    IndexTable tables[MAX_TABLES];
    sal_Int16 table_count;
    IndexKey keys[MAX_KEYS];
    sal_Int16 key_count;
    CollatorImpl *collator;
    sal_Int16 compare(sal_Unicode c1, sal_Unicode c2);
};

} } } }

#endif
