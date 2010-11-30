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

#include "UserOptTest.hxx"

namespace css = ::com::sun::star;

//=============================================================================
static const ::rtl::OUString MESSAGE_SETCOMPANY_FAILED        (RTL_CONSTASCII_USTRINGPARAM("set company failed"))       ;
static const ::rtl::OUString MESSAGE_SETFIRSTNAME_FAILED      (RTL_CONSTASCII_USTRINGPARAM("set firstname failed"))     ;
static const ::rtl::OUString MESSAGE_SETLASTNAME_FAILED       (RTL_CONSTASCII_USTRINGPARAM("set lastname failed"))      ;
static const ::rtl::OUString MESSAGE_SETID_FAILED             (RTL_CONSTASCII_USTRINGPARAM("set ID failed"))            ;
static const ::rtl::OUString MESSAGE_SETSTREET_FAILED         (RTL_CONSTASCII_USTRINGPARAM("set street failed"))        ;
static const ::rtl::OUString MESSAGE_SETCITY_FAILED           (RTL_CONSTASCII_USTRINGPARAM("set city failed"))          ;
static const ::rtl::OUString MESSAGE_SETSTATE_FAILED          (RTL_CONSTASCII_USTRINGPARAM("set state failed"))         ;
static const ::rtl::OUString MESSAGE_SETZIP_FAILED            (RTL_CONSTASCII_USTRINGPARAM("set zip failed"))           ;
static const ::rtl::OUString MESSAGE_SETCOUNTRY_FAILED        (RTL_CONSTASCII_USTRINGPARAM("set country failed"))       ;
static const ::rtl::OUString MESSAGE_SETPOSITION_FAILED       (RTL_CONSTASCII_USTRINGPARAM("set position failed"))      ;
static const ::rtl::OUString MESSAGE_SETTITLE_FAILED          (RTL_CONSTASCII_USTRINGPARAM("set title failed"))         ;
static const ::rtl::OUString MESSAGE_SETTELEPHONEHOME_FAILED  (RTL_CONSTASCII_USTRINGPARAM("set telephonehome failed")) ;
static const ::rtl::OUString MESSAGE_SETTELEPHONEWORK_FAILED  (RTL_CONSTASCII_USTRINGPARAM("set telephonework failed")) ;
static const ::rtl::OUString MESSAGE_SETFAX_FAILED            (RTL_CONSTASCII_USTRINGPARAM("set fax failed"))           ;
static const ::rtl::OUString MESSAGE_SETEMAIL_FAILED          (RTL_CONSTASCII_USTRINGPARAM("set email failed"))         ;
static const ::rtl::OUString MESSAGE_SETCUSTOMERNUMBER_FAILED (RTL_CONSTASCII_USTRINGPARAM("set customernumber failed"));
static const ::rtl::OUString MESSAGE_SETFATHERSNAME_FAILED    (RTL_CONSTASCII_USTRINGPARAM("set fathersname failed"))   ;
static const ::rtl::OUString MESSAGE_SETAPARTMENT_FAILED      (RTL_CONSTASCII_USTRINGPARAM("set apartment failed"))     ;

//=============================================================================


UserOptTest::UserOptTest()
   :m_aConfigItem()
   ,m_xCfg()
{
}

UserOptTest::~UserOptTest()
{
}

void UserOptTest::impl_checkUserData()
{
    impl_checkSetCompany( ::rtl::OUString() );
    impl_checkSetFirstName( ::rtl::OUString() );
    impl_checkSetLastName( ::rtl::OUString() );
    impl_checkSetID( ::rtl::OUString() );
    impl_checkSetStreet( ::rtl::OUString() );
    impl_checkSetCity( ::rtl::OUString() );
    impl_checkSetState( ::rtl::OUString() );
    impl_checkSetZip( ::rtl::OUString() );
    impl_checkSetCountry( ::rtl::OUString() );
    impl_checkSetPosition( ::rtl::OUString() );
    impl_checkSetTitle( ::rtl::OUString() );
    impl_checkSetTelephoneHome( ::rtl::OUString() );
    impl_checkSetTelephoneWork( ::rtl::OUString() );
    impl_checkSetFax( ::rtl::OUString() );
    impl_checkSetEmail( ::rtl::OUString() );
    //impl_checkSetCustomerNumber( ::rtl::OUString() );
    impl_checkSetFathersName( ::rtl::OUString() );
    impl_checkSetApartment( ::rtl::OUString() );

    impl_checkSetCompany( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RedFlag2000")) );
    impl_checkSetFirstName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Yan")) );
    impl_checkSetLastName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Wu")) );
    impl_checkSetID( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("wuy")) );
    impl_checkSetStreet( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SouthFifthRing")) );
    impl_checkSetCity( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Beijing")) );
    impl_checkSetState( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Beijing")) );
    impl_checkSetZip( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("100176")) );
    impl_checkSetCountry( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("China")) );
    impl_checkSetPosition( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Engineer")) );
    impl_checkSetTitle( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Software Engineer")) );
    impl_checkSetTelephoneHome( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("010-51570010")) );
    impl_checkSetTelephoneWork( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("010-51570010")) );
    impl_checkSetFax( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("010-51570010")) );
    impl_checkSetEmail( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("wuy@redflag2000.cn")) );
    //impl_checkSetCustomerNumber( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("87654321")) );
    impl_checkSetFathersName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("father")) );
    impl_checkSetApartment( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("apartment")) );
}

void UserOptTest::impl_checkSetCompany( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetCompany( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetCompany();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETCOMPANY_FAILED, 0);
}

void UserOptTest::impl_checkSetFirstName( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetFirstName( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetFirstName();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETFIRSTNAME_FAILED, 0);
}

void UserOptTest::impl_checkSetLastName( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetLastName( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetLastName();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETLASTNAME_FAILED, 0);
}

void UserOptTest::impl_checkSetID( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetID( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetID();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETID_FAILED, 0);
}

void UserOptTest::impl_checkSetStreet( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetStreet( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetStreet();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETSTREET_FAILED, 0);
}

void UserOptTest::impl_checkSetCity( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetCity( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetCity();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETCITY_FAILED, 0);
}

void UserOptTest::impl_checkSetState( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetState( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetState();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETSTATE_FAILED, 0);
}

void UserOptTest::impl_checkSetZip( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetZip( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetZip();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETZIP_FAILED, 0);
}

void UserOptTest::impl_checkSetCountry( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetCountry( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetCountry();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETCOUNTRY_FAILED, 0);
}

void UserOptTest::impl_checkSetPosition( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetPosition( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetPosition();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETPOSITION_FAILED, 0);
}

void UserOptTest::impl_checkSetTitle( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetTitle( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetTitle();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETTITLE_FAILED, 0);
}

void UserOptTest::impl_checkSetTelephoneHome( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetTelephoneHome( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetTelephoneHome();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETTELEPHONEHOME_FAILED, 0);
}

void UserOptTest::impl_checkSetTelephoneWork( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetTelephoneWork( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetTelephoneWork();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETTELEPHONEWORK_FAILED, 0);
}

void UserOptTest::impl_checkSetFax( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetFax( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetFax();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETFAX_FAILED, 0);
}

void UserOptTest::impl_checkSetEmail( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetEmail( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetEmail();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETEMAIL_FAILED, 0);
}

void UserOptTest::impl_checkSetCustomerNumber( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetCustomerNumber( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetCustomerNumber();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETCUSTOMERNUMBER_FAILED, 0);
}

void UserOptTest::impl_checkSetFathersName( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetFathersName( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetFathersName();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETFATHERSNAME_FAILED, 0);
}

void UserOptTest::impl_checkSetApartment( const ::rtl::OUString& sUserData )
{
    m_aConfigItem.SetApartment( sUserData );

    ::rtl::OUString sCheck = m_aConfigItem.GetApartment();
    if ( sCheck != sUserData )
        throw css::uno::Exception(MESSAGE_SETAPARTMENT_FAILED, 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
