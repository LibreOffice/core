/*************************************************************************
 *
 *  $RCSfile: transliteration_commonclass.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:23:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#include <transliteration_commonclass.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

transliteration_commonclass::transliteration_commonclass()
{
    transliterationName = "";
    implementationName = "";
}

OUString SAL_CALL transliteration_commonclass::getName() throw(RuntimeException)
{
    return OUString::createFromAscii(transliterationName);
}

void SAL_CALL transliteration_commonclass::loadModule( TransliterationModules modName, const Locale& rLocale )
    throw(RuntimeException)
{
    aLocale = rLocale;
}


void SAL_CALL
transliteration_commonclass::loadModuleNew( const Sequence < TransliterationModulesNew >& modName, const Locale& rLocale )
    throw(RuntimeException)
{
    throw RuntimeException();
}


void SAL_CALL
transliteration_commonclass::loadModuleByImplName( const OUString& implName, const Locale& rLocale )
    throw(RuntimeException)
{
    throw RuntimeException();
}

void SAL_CALL
transliteration_commonclass::loadModulesByImplNames(const Sequence< OUString >& modNamelist, const Locale& rLocale)
    throw(RuntimeException)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
transliteration_commonclass::getAvailableModules( const Locale& rLocale, sal_Int16 sType )
    throw(RuntimeException)
{
    throw RuntimeException();
}

sal_Int32 SAL_CALL
transliteration_commonclass::compareSubstring(
    const OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const OUString& str2, sal_Int32 off2, sal_Int32 len2)
    throw(RuntimeException)
{
    const sal_Unicode* unistr1 = NULL;
    const sal_Unicode* unistr2 = NULL;
    sal_uInt32 strlen1;
    sal_uInt32 strlen2;

    Sequence <long> offset1(2*len1);
    Sequence <long> offset2(2*len2);

    OUString in_str1 = this->transliterate(str1, off1, len1, offset1);
    OUString in_str2 = this->transliterate(str2, off2, len2, offset2);
    strlen1 = in_str1.getLength();
    strlen2 = in_str2.getLength();
    unistr1 = in_str1.getStr();
    unistr2 = in_str2.getStr();

    while (strlen1 && strlen2)
    {
        sal_uInt32 ret = *unistr1 - *unistr2;
        if (ret)
           return ret;

        unistr1++;
        unistr2++;
        strlen1--;
        strlen2--;
    }
    return strlen1 - strlen2;
}

sal_Int32 SAL_CALL
transliteration_commonclass::compareString( const OUString& str1, const OUString& str2 ) throw ( RuntimeException)
{
    return( this->compareSubstring(str1, 0, str1.getLength(), str2, 0, str2.getLength()));
}


OUString SAL_CALL transliteration_commonclass::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(implementationName);
}

const sal_Char cTrans[] = "com.sun.star.i18n.Transliteration.l10n";

sal_Bool SAL_CALL transliteration_commonclass::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName.equalsAscii(cTrans);
}

Sequence< OUString > SAL_CALL transliteration_commonclass::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cTrans);
    return aRet;
}

} } } }
