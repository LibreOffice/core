/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fasttokenhandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:34:40 $
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
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& Identifier ) throw (::com::sun::star::uno::RuntimeException);
};

}

#endif

