/*************************************************************************
 *
 *  $RCSfile: spelldta.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:30 $
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

#ifndef _LINGUISTIC_SPELLDTA_HXX_
#define _LINGUISTIC_SPELLDTA_HXX_


#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLALTERNATIVES_HPP_
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#endif

#include <tools/solar.h>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations


namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSpellAlternatives >
        MergeProposals(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellAlternatives > &rxAlt1,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XSpellAlternatives > &rxAlt2 );

///////////////////////////////////////////////////////////////////////////


class SpellAlternatives :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XSpellAlternatives
    >
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  aAlt;   // list of alternatives, may be empty.
    ::rtl::OUString         aWord;
    INT16                   nType;          // type of failure
    INT16                   nLanguage;

    // disallow copy-constructor and assignment-operator for now
    SpellAlternatives(const SpellAlternatives &);
    SpellAlternatives & operator = (const SpellAlternatives &);

public:
    SpellAlternatives();
    SpellAlternatives(const ::rtl::OUString &rWord, INT16 nLang, INT16 nFailureType,
                      const ::rtl::OUString &rRplcWord );
    virtual ~SpellAlternatives();

    // XSpellAlternatives
    virtual ::rtl::OUString SAL_CALL
        getWord()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        getFailureType()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL
        getAlternativesCount()
            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getAlternatives()
            throw(::com::sun::star::uno::RuntimeException);

    // non-interface specific functions
    void    SetWordLanguage(const ::rtl::OUString &rWord, INT16 nLang);
    void    SetFailureType(INT16 nTypeP);
    void    SetAlternatives(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rAlt );
};


///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

#endif

