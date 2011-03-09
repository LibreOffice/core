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

#include <tools/solar.h>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations


namespace linguistic
{

class Meaning :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XMeaning
    >
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  aSyn;   // list of synonyms, may be empty.
    ::rtl::OUString         aTerm;
    sal_Int16                   nLanguage;

#if 0
        // this is for future use by a German thesaurus
        sal_Bool                bIsGermanPreReform;
#endif

    // disallow copy-constructor and assignment-operator for now
    Meaning(const Meaning &);
    Meaning & operator = (const Meaning &);

public:
#if 0
    Meaning(const ::rtl::OUString &rTerm, sal_Int16 nLang, const PropertyHelper_Thes &rHelper);
#else
    Meaning(const ::rtl::OUString &rTerm, sal_Int16 nLang);
#endif
    virtual ~Meaning();

    // XMeaning
    virtual ::rtl::OUString SAL_CALL getMeaning() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL querySynonyms() throw(::com::sun::star::uno::RuntimeException);

    // non-interface specific functions
    void    SetSynonyms( const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rSyn );
    void    SetMeaning( const ::rtl::OUString  &rTerm );
};


///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

#endif


