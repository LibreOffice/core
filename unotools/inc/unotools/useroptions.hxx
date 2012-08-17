/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_unotools_USEROPTIONS_HXX
#define INCLUDED_unotools_USEROPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <unotools/configitem.hxx>
#include <osl/mutex.hxx>
#include <unotools/options.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

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
#define USER_OPT_CUSTOMERNUMBER     ((sal_uInt16)17)

// class SvtUserOptions --------------------------------------------------

class UNOTOOLS_DLLPUBLIC SvtUserOptions : public utl::detail::Options
{
public:
    SvtUserOptions ();
    virtual ~SvtUserOptions ();

    static osl::Mutex& GetInitMutex ();

    // get the address token
    rtl::OUString GetCompany        () const;
    rtl::OUString GetFirstName      () const;
    rtl::OUString GetLastName       () const;
    rtl::OUString GetID             () const;
    rtl::OUString GetStreet         () const;
    rtl::OUString GetCity           () const;
    rtl::OUString GetState          () const;
    rtl::OUString GetZip            () const;
    rtl::OUString GetCountry        () const;
    rtl::OUString GetPosition       () const;
    rtl::OUString GetTitle          () const;
    rtl::OUString GetTelephoneHome  () const;
    rtl::OUString GetTelephoneWork  () const;
    rtl::OUString GetFax            () const;
    rtl::OUString GetEmail          () const;
    rtl::OUString GetCustomerNumber () const;
    rtl::OUString GetFathersName    () const;
    rtl::OUString GetApartment      () const;

    rtl::OUString GetFullName       () const;

    // set the address token
    void SetCompany        (rtl::OUString const&);
    void SetFirstName      (rtl::OUString const&);
    void SetLastName       (rtl::OUString const&);
    void SetID             (rtl::OUString const&);
    void SetStreet         (rtl::OUString const&);
    void SetCity           (rtl::OUString const&);
    void SetState          (rtl::OUString const&);
    void SetZip            (rtl::OUString const&);
    void SetCountry        (rtl::OUString const&);
    void SetPosition       (rtl::OUString const&);
    void SetTitle          (rtl::OUString const&);
    void SetTelephoneHome  (rtl::OUString const&);
    void SetTelephoneWork  (rtl::OUString const&);
    void SetFax            (rtl::OUString const&);
    void SetEmail          (rtl::OUString const&);
    void SetCustomerNumber (rtl::OUString const&);
    void SetFathersName    (rtl::OUString const&);
    void SetApartment      (rtl::OUString const&);

    sal_Bool      IsTokenReadonly (sal_uInt16 nToken) const;
    rtl::OUString GetToken (sal_uInt16 nToken) const;
    void          SetToken (sal_uInt16 nToken, rtl::OUString const& rNewToken);

private:
    class Impl;
    boost::shared_ptr<Impl> pImpl;
    static boost::weak_ptr<Impl> pSharedImpl;
private:
    class ChangeListener;
};

#endif // #ifndef INCLUDED_unotools_USEROPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
