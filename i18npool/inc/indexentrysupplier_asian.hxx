/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexentrysupplier_asian.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:52:24 $
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

#ifndef _I18N_INDEXENTRYSUPPLIER_CJK_HXX_
#define _I18N_INDEXENTRYSUPPLIER_CJK_HXX_

#include <indexentrysupplier_common.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class IndexEntrySupplier_CJK
//  ----------------------------------------------------

class IndexEntrySupplier_CJK : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_CJK( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF ) : IndexEntrySupplier_Common(rxMSF) {
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_CJK";
    };

    virtual rtl::OUString SAL_CALL getIndexString( const sal_Unicode ch, const sal_Unicode idxStr[],
        const sal_uInt16 idx1[], const sal_uInt16 idx2[]) throw (com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getIndexString( const sal_Unicode ch,
        const sal_uInt16 idx1[], const sal_Unicode idx2[]) throw (com::sun::star::uno::RuntimeException);
};

#define INDEXENTRYSUPPLIER_CJK( algorithm ) \
class IndexEntrySupplier_##algorithm : public IndexEntrySupplier_CJK {\
public:\
    IndexEntrySupplier_##algorithm (const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF) : IndexEntrySupplier_CJK (rxMSF) {\
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_"#algorithm;\
    };\
    virtual rtl::OUString SAL_CALL getIndexCharacter( const rtl::OUString& rIndexEntry,\
        const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm ) \
        throw (com::sun::star::uno::RuntimeException);\
    virtual rtl::OUString SAL_CALL getIndexKey( const rtl::OUString& IndexEntry, \
        const rtl::OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )\
        throw (com::sun::star::uno::RuntimeException);\
};

#define INDEXENTRYSUPPLIER_PHONETIC( algorithm ) \
class IndexEntrySupplier_##algorithm : public IndexEntrySupplier_CJK {\
public:\
    IndexEntrySupplier_##algorithm (const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF) : IndexEntrySupplier_CJK (rxMSF) {\
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_"#algorithm;\
    };\
    virtual rtl::OUString SAL_CALL getIndexCharacter( const rtl::OUString& rIndexEntry,\
        const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm ) \
        throw (com::sun::star::uno::RuntimeException);\
    virtual rtl::OUString SAL_CALL getIndexKey( const rtl::OUString& IndexEntry, \
        const rtl::OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )\
        throw (com::sun::star::uno::RuntimeException);\
    virtual sal_Int16 SAL_CALL compareIndexEntry( const rtl::OUString& IndexEntry1,\
        const rtl::OUString& PhoneticEntry1, const com::sun::star::lang::Locale& rLocale1,\
        const rtl::OUString& IndexEntry2, const ::rtl::OUString& PhoneticEntry2,\
        const com::sun::star::lang::Locale& rLocale2 )\
        throw (com::sun::star::uno::RuntimeException);\
};

#define INDEXENTRYSUPPLIER_JA_PHONETIC( algorithm ) \
class IndexEntrySupplier_##algorithm : public IndexEntrySupplier_ja_phonetic {\
public:\
    IndexEntrySupplier_##algorithm (const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF) : IndexEntrySupplier_ja_phonetic (rxMSF) {\
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_"#algorithm;\
    };\
    virtual sal_Bool SAL_CALL loadAlgorithm(\
        const com::sun::star::lang::Locale& rLocale,\
        const rtl::OUString& SortAlgorithm, sal_Int32 collatorOptions ) \
        throw (com::sun::star::uno::RuntimeException);\
};

#if defined( INDEXENTRYSUPPLIER_zh_pinyin ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_PHONETIC( zh_pinyin )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_stroke ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_stroke )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_radical ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_radical )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_zhuyin ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_PHONETIC( zh_zhuyin )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_TW_radical ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_TW_radical )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_TW_stroke ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_TW_stroke )
#endif
#if defined( INDEXENTRYSUPPLIER_ko_dict ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_PHONETIC( ko_dict )
#endif
#if defined( INDEXENTRYSUPPLIER_ja_phonetic ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_PHONETIC( ja_phonetic )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_first_by_syllable )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_first_by_consonant )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_last_by_syllable )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_last_by_consonant )
#endif
#undef INDEXENTRYSUPPLIER_CJK

} } } }
#endif
