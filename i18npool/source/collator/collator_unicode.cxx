/*************************************************************************
 *
 *  $RCSfile: collator_unicode.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 15:47:48 $
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

#include <assert.h>

#include <collator_unicode.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <casefolding.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

Collator_Unicode::Collator_Unicode()
{
    implementationName = "com.sun.star.i18n.Collator_Unicode";
    tranModules = TransliterationModules_END_OF_MODULE;
}

Collator_Unicode::~Collator_Unicode()
{
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
        c1 = casefolding::getNextChar(uniStr1, idx1, len1, e1, aLocale, MappingTypeFullFolding, tranModules);
        c2 = casefolding::getNextChar(uniStr2, idx2, len2, e2, aLocale, MappingTypeFullFolding, tranModules);
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
    aLocale = rLocale;

    tranModules = TransliterationModules_END_OF_MODULE;
    if (options & CollatorOptions::CollatorOptions_IGNORE_CASE)
        tranModules = (TransliterationModules)(tranModules | TransliterationModules_IGNORE_CASE);
    if (options & CollatorOptions::CollatorOptions_IGNORE_KANA)
        tranModules = (TransliterationModules) (tranModules | TransliterationModules_IGNORE_KANA);
    if (options & CollatorOptions::CollatorOptions_IGNORE_WIDTH)
        tranModules = (TransliterationModules) (tranModules | TransliterationModules_IGNORE_WIDTH);

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
