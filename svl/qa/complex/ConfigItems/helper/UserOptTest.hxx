/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UserOptTest.hxx,v $
 *
 *  $Revision: 1.1.4.2 $
 *
 *  last change: $Author: as $ $Date: 2008/03/19 11:09:25 $
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

#ifndef  SVTOOLS_USEROPTTEST_HXX
#define  SVTOOLS_USEROPTTEST_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <unotools/useroptions.hxx>

namespace css = ::com::sun::star;

class UserOptTest
{
public:
    UserOptTest();
    ~UserOptTest();

    void impl_checkUserData();

private:
    void impl_checkSetCompany( const ::rtl::OUString& sUserData );
    void impl_checkSetFirstName( const ::rtl::OUString& sUserData );
    void impl_checkSetLastName( const ::rtl::OUString& sUserData );
    void impl_checkSetID( const ::rtl::OUString& sUserData );
    void impl_checkSetStreet( const ::rtl::OUString& sUserData );
    void impl_checkSetCity( const ::rtl::OUString& sUserData );
    void impl_checkSetState( const ::rtl::OUString& sUserData );
    void impl_checkSetZip( const ::rtl::OUString& sUserData );
    void impl_checkSetCountry( const ::rtl::OUString& sUserData );
    void impl_checkSetPosition( const ::rtl::OUString& sUserData );
    void impl_checkSetTitle( const ::rtl::OUString& sUserData );
    void impl_checkSetTelephoneHome( const ::rtl::OUString& sUserData );
    void impl_checkSetTelephoneWork( const ::rtl::OUString& sUserData );
    void impl_checkSetFax( const ::rtl::OUString& sUserData );
    void impl_checkSetEmail( const ::rtl::OUString& sUserData );
    void impl_checkSetCustomerNumber( const ::rtl::OUString& sUserData );
    void impl_checkSetFathersName( const ::rtl::OUString& sUserData );
    void impl_checkSetApartment( const ::rtl::OUString& sUserData );

private:
    SvtUserOptions m_aConfigItem;

    css::uno::Reference< css::container::XNameAccess > m_xCfg;
};

#endif // #ifndef  SVTOOLS_USEROPTTEST_HXX
