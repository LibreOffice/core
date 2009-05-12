/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: transliteration_Numeric.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _I18N_TRANSLITERATION_TRANSLITERATION_NUMERIC_H_
#define _I18N_TRANSLITERATION_TRANSLITERATION_NUMERIC_H_

#include <transliteration_commonclass.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class transliteration_Numeric : public transliteration_commonclass {
public:
        virtual ::rtl::OUString SAL_CALL
        transliterate( const ::rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, ::com::sun::star::uno::Sequence< sal_Int32 >& offset )
        throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(com::sun::star::i18n::MultipleCharsOutputException,
                com::sun::star::uno::RuntimeException);

        // Methods which are shared.
        virtual sal_Int16 SAL_CALL getType(  ) throw(::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString SAL_CALL
        folding( const ::rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, ::com::sun::star::uno::Sequence< sal_Int32 >& offset )
        throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        equals( const ::rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1, const ::rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        transliterateRange( const ::rtl::OUString& str1, const ::rtl::OUString& str2 )
        throw(::com::sun::star::uno::RuntimeException);
protected:
        sal_Int16 nNativeNumberMode;
        sal_Int16 tableSize;
        sal_Unicode* table;
        sal_Bool recycleSymbol;
private:
        rtl::OUString SAL_CALL
        transliterateBullet( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        com::sun::star::uno::Sequence< sal_Int32 >& offset )
        throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif // _I18N_TRANSLITERATION_TRANSLITERATION_NUMERIC_H_
