/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: thesdta.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:47:19 $
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

#ifndef _LINGUISTIC_THESDTA_HXX_
#define _LINGUISTIC_THESDTA_HXX_


#ifndef _COM_SUN_STAR_LINGUISTIC2_XMEANING_HPP_
#include <com/sun/star/linguistic2/XMeaning.hpp>
#endif

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
    INT16           nLookUpLanguage;    // language of the text that was looked up

    // disallow copy-constructor and assignment-operator for now
    ThesaurusMeaning(const ThesaurusMeaning &);
    ThesaurusMeaning & operator = (const ThesaurusMeaning &);

public:
    ThesaurusMeaning(const ::rtl::OUString &rText,
            const ::rtl::OUString &rLookUpText, INT16 nLookUpLang );
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

