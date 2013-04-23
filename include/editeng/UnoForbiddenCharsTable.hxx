/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SVX_UNOFORBIDDENCHARSTABLE_HXX_
#define _SVX_UNOFORBIDDENCHARSTABLE_HXX_

#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <rtl/ref.hxx>

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

    rtl::Reference<SvxForbiddenCharactersTable> mxForbiddenChars;

public:
    SvxUnoForbiddenCharsTable(rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
