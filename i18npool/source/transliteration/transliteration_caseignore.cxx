/*************************************************************************
 *
 *  $RCSfile: transliteration_caseignore.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 16:54:32 $
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

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <i18nutil/oneToOneMapping.hxx>
#include <i18nutil/casefolding.hxx>
#include "transliteration_caseignore.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

Transliteration_caseignore::Transliteration_caseignore()
{
    nMappingType = MappingTypeFullFolding;
    moduleLoaded = (TransliterationModules)0;
    transliterationName = "case ignore (generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_caseignore";
}

Transliteration_simplecaseignore::Transliteration_simplecaseignore()
{
    nMappingType = MappingTypeSimpleFolding;
    moduleLoaded = (TransliterationModules)0;
    transliterationName = "simple case ignore (generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_simplecaseignore";
}

void SAL_CALL
Transliteration_caseignore::loadModule( TransliterationModules modName, const Locale& rLocale )
    throw(RuntimeException)
{
    moduleLoaded = (TransliterationModules) (moduleLoaded|modName);
    aLocale = rLocale;
}

sal_Int16 SAL_CALL Transliteration_caseignore::getType() throw(RuntimeException)
{
    // It's NOT TransliterationType::ONE_TO_ONE because it's using casefolding
    return TransliterationType::IGNORE;
}


Sequence< OUString > SAL_CALL
Transliteration_caseignore::transliterateRange( const OUString& str1, const OUString& str2 )
    throw( RuntimeException)
{
    if (str1.getLength() != 1 || str2.getLength() != 1)
        throw RuntimeException();

    static Transliteration_u2l u2l;
    static Transliteration_l2u l2u;

    u2l.loadModule((TransliterationModules)0, aLocale);
    l2u.loadModule((TransliterationModules)0, aLocale);

    OUString l1 = u2l.transliterateString2String(str1, 0, str1.getLength());
    OUString u1 = l2u.transliterateString2String(str1, 0, str1.getLength());
    OUString l2 = u2l.transliterateString2String(str2, 0, str2.getLength());
    OUString u2 = l2u.transliterateString2String(str2, 0, str2.getLength());

    if ((l1 == u1) && (l2 == u2)) {
        Sequence< OUString > &r = *new Sequence< OUString > (2);
        r[0] = l1;
        r[1] = l2;
        return r;
    } else {
        Sequence< OUString > &r = *new Sequence< OUString > (4);
        r[0] = l1;
        r[1] = l2;
        r[2] = u1;
        r[3] = u2;
        return r;
    }
}

sal_Bool SAL_CALL
Transliteration_caseignore::equals(
    const ::rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
    const ::rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2)
    throw(::com::sun::star::uno::RuntimeException)
{
    return (compare(str1, pos1, nCount1, nMatch1, str2, pos2, nCount2, nMatch2) == 0);
}

sal_Int32 SAL_CALL
Transliteration_caseignore::compareSubstring(
    const ::rtl::OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const ::rtl::OUString& str2, sal_Int32 off2, sal_Int32 len2)
    throw(RuntimeException)
{
    sal_Int32 nMatch1, nMatch2;
    return compare(str1, off1, len1, nMatch1, str2, off2, len2, nMatch2);
}


sal_Int32 SAL_CALL
Transliteration_caseignore::compareString(
    const ::rtl::OUString& str1,
    const ::rtl::OUString& str2)
    throw(RuntimeException)
{
    sal_Int32 nMatch1, nMatch2;
    return compare(str1, 0, str1.getLength(), nMatch1, str2, 0, str2.getLength(), nMatch2);
}

sal_Int32 SAL_CALL
Transliteration_caseignore::compare(
    const ::rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
    const ::rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2)
    throw(RuntimeException)
{
    const sal_Unicode *unistr1 = (sal_Unicode*) str1.getStr() + pos1;
    const sal_Unicode *unistr2 = (sal_Unicode*) str2.getStr() + pos2;
    sal_Unicode c1, c2;
    MappingElement e1, e2;
    nMatch1 = nMatch2 = 0;

#define NOT_END_OF_STR1 (nMatch1 < nCount1 || e1.current < e1.element.nmap)
#define NOT_END_OF_STR2 (nMatch2 < nCount2 || e2.current < e2.element.nmap)

    while (NOT_END_OF_STR1 && NOT_END_OF_STR2) {
        c1 = casefolding::getNextChar(unistr1, nMatch1, nCount1, e1, aLocale, nMappingType, moduleLoaded);
        c2 = casefolding::getNextChar(unistr2, nMatch2, nCount2, e2, aLocale, nMappingType, moduleLoaded);
        if (c1 != c2) {
        nMatch1--; nMatch2--;
        return c1 > c2 ? 1 : -1;
        }
    }

    return (!NOT_END_OF_STR1 && !NOT_END_OF_STR2) ? 0
                : (NOT_END_OF_STR1 ? 1 : -1);
}

} } } }
