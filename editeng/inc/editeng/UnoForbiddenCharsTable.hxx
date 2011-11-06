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



#ifndef _SVX_UNOFORBIDDENCHARSTABLE_HXX_
#define _SVX_UNOFORBIDDENCHARSTABLE_HXX_

#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <vos/ref.hxx>

#include <cppuhelper/implbase2.hxx>
#include "editeng/editengdllapi.h"

class SvxForbiddenCharactersTable;

class EDITENG_DLLPUBLIC SvxUnoForbiddenCharsTable : public cppu::WeakImplHelper2<
                                        com::sun::star::i18n::XForbiddenCharacters,
                                        com::sun::star::linguistic2::XSupportedLocales>
{
protected:
    /** this virtual function is called if the forbidden characters are changed */
    virtual void onChange();

    vos::ORef<SvxForbiddenCharactersTable> mxForbiddenChars;

public:
    SvxUnoForbiddenCharsTable(vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars);
    ~SvxUnoForbiddenCharsTable();

    // XForbiddenCharacters
    virtual com::sun::star::i18n::ForbiddenCharacters SAL_CALL getForbiddenCharacters( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::container::NoSuchElementException, com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasForbiddenCharacters( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setForbiddenCharacters( const com::sun::star::lang::Locale& rLocale, const com::sun::star::i18n::ForbiddenCharacters& rForbiddenCharacters ) throw(com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeForbiddenCharacters( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);

    // XSupportedLocales
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasLocale( const ::com::sun::star::lang::Locale& aLocale ) throw (::com::sun::star::uno::RuntimeException);
};

#endif // _SVX_UNOFORBIDDENCHARSTABLE_HXX_
