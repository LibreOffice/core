/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _LINGUISTIC_THESDTA_HXX_
#define _LINGUISTIC_THESDTA_HXX_


#include <com/sun/star/linguistic2/XMeaning.hpp>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations


namespace linguistic
{

///////////////////////////////////////////////////////////////////////////


class ThesaurusMeaning :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XMeaning
    >
{

protected:
    ::rtl::OUString aText;              // one of the found 'meanings' for the looked up text
    ::rtl::OUString aLookUpText;        // text that was looked up in the thesaurus
    sal_Int16           nLookUpLanguage;    // language of the text that was looked up

    // disallow copy-constructor and assignment-operator for now
    ThesaurusMeaning(const ThesaurusMeaning &);
    ThesaurusMeaning & operator = (const ThesaurusMeaning &);

public:
    ThesaurusMeaning(const ::rtl::OUString &rText,
            const ::rtl::OUString &rLookUpText, sal_Int16 nLookUpLang );
    virtual ~ThesaurusMeaning();

    // XMeaning
    virtual ::rtl::OUString SAL_CALL
        getMeaning()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        querySynonyms()
            throw(::com::sun::star::uno::RuntimeException) = 0;

    // non-interface specific functions
    const ::rtl::OUString & getLookUpText() const       { return aLookUpText; }
    short                   getLookUpLanguage() const   { return nLookUpLanguage; }
};


///////////////////////////////////////////////////////////////////////////

} // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
