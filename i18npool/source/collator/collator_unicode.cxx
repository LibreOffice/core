/*************************************************************************
 *
 *  $RCSfile: collator_unicode.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 06:32:21 $
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
#include <assert.h>

#include <collator_unicode.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <transliteration_caseignore.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

Collator_Unicode::Collator_Unicode()
{
    implementationName = "com.sun.star.i18n.Collator_Unicode";
    tranModules = TransliterationModules_END_OF_MODULE;
    ignore = NULL;
}

Collator_Unicode::~Collator_Unicode()
{
    if (ignore)
        delete ignore;
}

sal_Int32 SAL_CALL
Collator_Unicode::compareSubstring( const OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const OUString& str2, sal_Int32 off2, sal_Int32 len2) throw(RuntimeException)
{
    sal_Unicode *uniStr1 = (sal_Unicode*) str1.getStr() + off1;
    sal_Unicode *uniStr2 = (sal_Unicode*) str2.getStr() + off2;

    MappingElement e1, e2;
    sal_Int32 idx1, idx2;
    sal_Unicode c1, c2;

    idx1 = idx2 = 0;
    while (idx1 < len1 && idx2 < len2) {
        if (tranModules != TransliterationModules_END_OF_MODULE) {
        c1 = ((Transliteration_caseignore*)ignore)->getNextChar(uniStr1, idx1, len1, e1);
        c2 = ((Transliteration_caseignore*)ignore)->getNextChar(uniStr2, idx2, len2, e2);
        } else {
        c1 = uniStr1[idx1++];
        c2 = uniStr1[idx2++];
        }
        if (c1 != c2)
        return c1 < c2 ? -1 : 1;
    }
    return idx1 == len1 && idx2 == len2 ? 0 : (idx1 == len1 ? -1 : 1);
}

sal_Int32 SAL_CALL
Collator_Unicode::compareString( const OUString& str1, const OUString& str2) throw(RuntimeException)
{
    return compareSubstring(str1, 0, str1.getLength(), str2, 0, str2.getLength());
}


sal_Int32 SAL_CALL
Collator_Unicode::loadCollatorAlgorithm(const OUString& rAlgorithm, const lang::Locale& rLocale, sal_Int32 options)
    throw(RuntimeException)
{
    tranModules = TransliterationModules_END_OF_MODULE;
    if (options & CollatorOptions::CollatorOptions_IGNORE_CASE)
        tranModules = (TransliterationModules)(tranModules | TransliterationModules_IGNORE_CASE);
    if (options & CollatorOptions::CollatorOptions_IGNORE_KANA)
        tranModules = (TransliterationModules) (tranModules | TransliterationModules_IGNORE_KANA);
    if (options & CollatorOptions::CollatorOptions_IGNORE_WIDTH)
        tranModules = (TransliterationModules) (tranModules | TransliterationModules_IGNORE_WIDTH);

    if (tranModules != TransliterationModules_END_OF_MODULE) {
        if (ignore == NULL)
        ignore = new Transliteration_caseignore();
        ignore->loadModule(tranModules, rLocale);
    }

    return(0);
}


OUString SAL_CALL
Collator_Unicode::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(implementationName);
}

sal_Bool SAL_CALL
Collator_Unicode::supportsService(const rtl::OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(implementationName);
}

Sequence< OUString > SAL_CALL
Collator_Unicode::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(implementationName);
    return aRet;
}

} } } }
