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
#ifndef INCLUDED_unotools_USEROPTIONS_HXX
#define INCLUDED_unotools_USEROPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <unotools/configitem.hxx>
#include <tools/string.hxx>
#include <osl/mutex.hxx>
#include <unotools/options.hxx>

// define ----------------------------------------------------------------

#define USER_OPT_CITY               ((sal_uInt16)0)
#define USER_OPT_COMPANY            ((sal_uInt16)1)
#define USER_OPT_COUNTRY            ((sal_uInt16)2)
#define USER_OPT_EMAIL              ((sal_uInt16)3)
#define USER_OPT_FAX                ((sal_uInt16)4)
#define USER_OPT_FIRSTNAME          ((sal_uInt16)5)
#define USER_OPT_LASTNAME           ((sal_uInt16)6)
#define USER_OPT_POSITION           ((sal_uInt16)7)
#define USER_OPT_STATE              ((sal_uInt16)8)
#define USER_OPT_STREET             ((sal_uInt16)9)
#define USER_OPT_TELEPHONEHOME      ((sal_uInt16)10)
#define USER_OPT_TELEPHONEWORK      ((sal_uInt16)11)
#define USER_OPT_TITLE              ((sal_uInt16)12)
#define USER_OPT_ID                 ((sal_uInt16)13)
#define USER_OPT_ZIP                ((sal_uInt16)14)
#define USER_OPT_FATHERSNAME        ((sal_uInt16)15)
#define USER_OPT_APARTMENT          ((sal_uInt16)16)

// class SvtUserOptions --------------------------------------------------

class SvtUserOptions_Impl;

class UNOTOOLS_DLLPUBLIC SvtUserOptions: public utl::detail::Options
{
private:
    SvtUserOptions_Impl*    pImp;

public:
    SvtUserOptions();
    virtual ~SvtUserOptions();

    static ::osl::Mutex&    GetInitMutex();

    // get the address token
     ::rtl::OUString    GetCompany() const;
     ::rtl::OUString    GetFirstName() const;
     ::rtl::OUString    GetLastName() const;
     ::rtl::OUString    GetID() const;
     ::rtl::OUString    GetStreet() const;
     ::rtl::OUString    GetCity() const;
     ::rtl::OUString    GetState() const;
     ::rtl::OUString    GetZip() const;
     ::rtl::OUString    GetCountry() const;
     ::rtl::OUString    GetPosition() const;
     ::rtl::OUString    GetTitle() const;
     ::rtl::OUString    GetTelephoneHome() const;
     ::rtl::OUString    GetTelephoneWork() const;
     ::rtl::OUString    GetFax() const;
     ::rtl::OUString    GetEmail() const;
     ::rtl::OUString    GetCustomerNumber() const;
     ::rtl::OUString    GetFathersName() const;
     ::rtl::OUString    GetApartment() const;

     ::rtl::OUString    GetFullName() const;
     ::rtl::OUString    GetLocale() const;

    // set the address token
    void            SetCompany( const ::rtl::OUString& rNewToken );
    void            SetFirstName( const ::rtl::OUString& rNewToken );
    void            SetLastName( const ::rtl::OUString& rNewToken );
    void            SetID( const ::rtl::OUString& rNewToken );
    void            SetStreet( const ::rtl::OUString& rNewToken );
    void            SetCity( const ::rtl::OUString& rNewToken );
    void            SetState( const ::rtl::OUString& rNewToken );
    void            SetZip( const ::rtl::OUString& rNewToken );
    void            SetCountry( const ::rtl::OUString& rNewToken );
    void            SetPosition( const ::rtl::OUString& rNewToken );
    void            SetTitle( const ::rtl::OUString& rNewToken );
    void            SetTelephoneHome( const ::rtl::OUString& rNewToken );
    void            SetTelephoneWork( const ::rtl::OUString& rNewToken );
    void            SetFax( const ::rtl::OUString& rNewToken );
    void            SetEmail( const ::rtl::OUString& rNewToken );
    void            SetCustomerNumber( const ::rtl::OUString& rNewToken );
    void            SetFathersName( const ::rtl::OUString& rNewToken );
    void            SetApartment( const ::rtl::OUString& rNewToken );

    sal_Bool        IsTokenReadonly( sal_uInt16 nToken ) const;
    ::rtl::OUString GetToken(sal_uInt16 nToken) const;
};

#endif // #ifndef INCLUDED_unotools_USEROPTIONS_HXX
