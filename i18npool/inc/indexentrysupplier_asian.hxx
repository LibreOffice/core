/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier_asian.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: khong $ $Date: 2002-06-18 22:34:57 $
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

#ifndef _I18N_INDEXENTRYSUPPLIER_CJK_HXX_
#define _I18N_INDEXENTRYSUPPLIER_CJK_HXX_

#include <indexentrysupplier_default.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class IndexEntrySupplier_CJK
//  ----------------------------------------------------

class IndexEntrySupplier_CJK : public IndexEntrySupplier_Unicode {
public:
    IndexEntrySupplier_CJK( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF ) : IndexEntrySupplier_Unicode(rxMSF) {
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_CJK";
    };

    virtual rtl::OUString SAL_CALL getIndexString( const sal_Unicode ch, const sal_Unicode idxStr[],
        const sal_uInt16 idx1[], const sal_uInt16 idx2[]) throw (com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getIndexString( const sal_Unicode ch,
        const sal_uInt16 idx1[], const sal_Unicode idx2[]) throw (com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL compareIndexKey( const rtl::OUString& IndexEntry1,
        const rtl::OUString& PhoneticEntry1, const com::sun::star::lang::Locale& rLocale1,
        const rtl::OUString& IndexEntry2, const ::rtl::OUString& PhoneticEntry2,
        const com::sun::star::lang::Locale& rLocale2 )
        throw (com::sun::star::uno::RuntimeException);
};

#define INDEXENTRYSUPPLIER_CJK( algorithm, UsePhonetic ) \
class IndexEntrySupplier_##algorithm : public IndexEntrySupplier_CJK {\
public:\
    IndexEntrySupplier_##algorithm (const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF) : IndexEntrySupplier_CJK (rxMSF) {\
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_##algorithm";\
        usePhonetic = UsePhonetic;\
    };\
    virtual rtl::OUString SAL_CALL getIndexCharacter( const rtl::OUString& rIndexEntry,\
        const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm ) throw (com::sun::star::uno::RuntimeException);\
};

#if defined( INDEXENTRYSUPPLIER_zh_pinyin ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_pinyin, sal_False )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_stroke ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_stroke, sal_False )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_radical ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_radical, sal_False )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_zhuyin ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_zhuyin, sal_False )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_TW_radical ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_TW_radical, sal_False )
#endif
#if defined( INDEXENTRYSUPPLIER_zh_TW_stroke ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( zh_TW_stroke, sal_False )
#endif
#if defined( INDEXENTRYSUPPLIER_ko_dict ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( ko_dict, sal_False )
#endif
#if defined( INDEXENTRYSUPPLIER_ja_phonetic ) || defined( INDEXENTRYSUPPLIER_ALL )
INDEXENTRYSUPPLIER_CJK( ja_phonetic, sal_True )
#endif
#undef INDEXENTRYSUPPLIER_CJK

} } } }
#endif
