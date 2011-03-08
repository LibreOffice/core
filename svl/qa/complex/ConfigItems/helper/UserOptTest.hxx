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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
