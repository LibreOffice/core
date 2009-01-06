/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fasttokenhandler.hxx,v $
 * $Revision: 1.4 $
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

#ifndef OOX_FASTTOKENHANDLER_HXX
#define OOX_FASTTOKENHANDLER_HXX

#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <cppuhelper/implbase1.hxx>

namespace oox {

class FastTokenHandler : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastTokenHandler >
{
public:
    explicit            FastTokenHandler();
    virtual             ~FastTokenHandler();

    virtual ::sal_Int32 SAL_CALL getToken( const ::rtl::OUString& Identifier ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getIdentifier( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getUTF8Identifier( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& Identifier ) throw (::com::sun::star::uno::RuntimeException);
};

}

#endif

