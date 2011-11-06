/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
