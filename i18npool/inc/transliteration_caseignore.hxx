/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

} } } }

#endif
