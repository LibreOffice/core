/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UserOptTest.cxx,v $
 *
 *  $Revision: 1.1.4.2 $
 *
 *  last change: $Author: as $ $Date: 2008/03/19 11:09:24 $
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

#include "UserOptTest.hxx"

namespace css = ::com::sun::star;

//=============================================================================
static const ::rtl::OUString MESSAGE_SETCOMPANY_FAILED        = ::rtl::OUString::createFromAscii("set company failed")       ;
static const ::rtl::OUString MESSAGE_SETFIRSTNAME_FAILED      = ::rtl::OUString::createFromAscii("set firstname failed")     ;
static const ::rtl::OUString MESSAGE_SETLASTNAME_FAILED       = ::rtl::OUString::createFromAscii("set lastname failed")      ;
static const ::rtl::OUString MESSAGE_SETID_FAILED             = ::rtl::OUString::createFromAscii("set ID failed")            ;
static const ::rtl::OUString MESSAGE_SETSTREET_FAILED         = ::rtl::OUString::createFromAscii("set street failed")        ;
static const ::rtl::OUString MESSAGE_SETCITY_FAILED           = ::rtl::OUString::createFromAscii("set city failed")          ;
static const ::rtl::OUString MESSAGE_SETSTATE_FAILED          = ::rtl::OUString::createFromAscii("set state failed")         ;
static const ::rtl::OUString MESSAGE_SETZIP_FAILED            = ::rtl::OUString::createFromAscii("set zip failed")           ;
static const ::rtl::OUString MESSAGE_SETCOUNTRY_FAILED        = ::rtl::OUString::createFromAscii("set country failed")       ;
static const ::rtl::OUString MESSAGE_SETPOSITION_FAILED       = ::rtl::OUString::createFromAscii("set position failed")      ;
static const ::rtl::OUString MESSAGE_SETTITLE_FAILED          = ::rtl::OUString::createFromAscii("set title failed")         ;
static const ::rtl::OUString MESSAGE_SETTELEPHONEHOME_FAILED  = ::rtl::OUString::createFromAscii("set telephonehome failed") ;
static const ::rtl::OUString MESSAGE_SETTELEPHONEWORK_FAILED  = ::rtl::OUString::createFromAscii("set telephonework failed") ;
static const ::rtl::OUString MESSAGE_SETFAX_FAILED            = ::rtl::OUString::createFromAscii("set fax failed")           ;
static const ::rtl::OUString MESSAGE_SETEMAIL_FAILED          = ::rtl::OUString::createFromAscii("set email failed")         ;
static const ::rtl::OUString MESSAGE_SETCUSTOMERNUMBER_FAILED = ::rtl::OUString::createFromAscii("set customernumber failed");
static const ::rtl::OUString MESSAGE_SETFATHERSNAME_FAILED    = ::rtl::OUString::createFromAscii("set fathersname failed")   ;
static const ::rtl::OUString MESSAGE_SETAPARTMENT_FAILED      = ::rtl::OUString::createFromAscii("set apartment failed")     ;

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

    impl_checkSetCompany( ::rtl::OUString::createFromAscii("RedFlag2000") );    
    impl_checkSetFirstName( ::rtl::OUString::createFromAscii("Yan") ); 
    impl_checkSetLastName( ::rtl::OUString::createFromAscii("Wu") );    
    impl_checkSetID( ::rtl::OUString::createFromAscii("wuy") );       
    impl_checkSetStreet( ::rtl::OUString::createFromAscii("SouthFifthRing") );   
    impl_checkSetCity( ::rtl::OUString::createFromAscii("Beijing") );   
    impl_checkSetState( ::rtl::OUString::createFromAscii("Beijing") );     
    impl_checkSetZip( ::rtl::OUString::createFromAscii("100176") );     
    impl_checkSetCountry( ::rtl::OUString::createFromAscii("China") );
    impl_checkSetPosition( ::rtl::OUString::createFromAscii("Engineer") );
    impl_checkSetTitle( ::rtl::OUString::createFromAscii("Software Engineer") );     
    impl_checkSetTelephoneHome( ::rtl::OUString::createFromAscii("010-51570010") );
    impl_checkSetTelephoneWork( ::rtl::OUString::createFromAscii("010-51570010") );
    impl_checkSetFax( ::rtl::OUString::createFromAscii("010-51570010") ); 
    impl_checkSetEmail( ::rtl::OUString::createFromAscii("wuy@redflag2000.cn") );
    //impl_checkSetCustomerNumber( ::rtl::OUString::createFromAscii("87654321") );  
    impl_checkSetFathersName( ::rtl::OUString::createFromAscii("father") );
    impl_checkSetApartment( ::rtl::OUString::createFromAscii("apartment") );
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
