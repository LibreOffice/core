/*************************************************************************
 *
 *  $RCSfile: UnoForbiddenCharsTable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-04-05 16:46:44 $
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

class SvxForbiddenCharactersTable;

class SvxUnoForbiddenCharsTable : public cppu::WeakImplHelper2<
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
