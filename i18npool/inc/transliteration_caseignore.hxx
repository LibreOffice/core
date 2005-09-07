/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transliteration_caseignore.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:59:06 $
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
#ifndef TRANSLITERATION_CASE_IGNORE_H
#define TRANSLITERATION_CASE_IGNORE_H

#define TRANSLITERATION_UPPER_LOWER
#include "transliteration_body.hxx"

namespace com { namespace sun { namespace star { namespace i18n {

class Transliteration_caseignore: public Transliteration_body
{
public:
    Transliteration_caseignore();

    // Methods which are shared.
    sal_Int16 SAL_CALL getType() throw(com::sun::star::uno::RuntimeException);

    void SAL_CALL loadModule( TransliterationModules modName, const com::sun::star::lang::Locale& rLocale )
    throw(com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL transliterateRange(
        const rtl::OUString& str1, const rtl::OUString& str2 )
    throw(com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL equals(
        const rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const rtl::OUString& src2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2)
    throw(com::sun::star::uno::RuntimeException);

    sal_Int32 SAL_CALL compareSubstring(
        const rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2)
    throw(com::sun::star::uno::RuntimeException);

    sal_Int32 SAL_CALL compareString(
        const rtl::OUString& s1,
        const rtl::OUString& s2)
    throw(com::sun::star::uno::RuntimeException);

protected:
    TransliterationModules moduleLoaded;
private:
    sal_Int32 SAL_CALL compare(
        const rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2)
    throw(com::sun::star::uno::RuntimeException);
};

class Transliteration_simplecaseignore: public Transliteration_caseignore
{
public:
    Transliteration_simplecaseignore();
};

} } } }

#endif
