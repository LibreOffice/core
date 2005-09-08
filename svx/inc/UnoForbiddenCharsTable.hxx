/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoForbiddenCharsTable.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:14:38 $
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

#ifndef _SVX_UNOFORBIDDENCHARSTABLE_HXX_
#define _SVX_UNOFORBIDDENCHARSTABLE_HXX_

#ifndef _COM_SUN_STAR_I18N_XFORBIDDENCHARACTERS_HPP_
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSUPPORTEDLOCALES_HPP_
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#endif

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#include <cppuhelper/implbase2.hxx>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvxForbiddenCharactersTable;

class SVX_DLLPUBLIC SvxUnoForbiddenCharsTable : public cppu::WeakImplHelper2<
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
