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
#ifndef INCLUDED_UNOTOOLS_USEROPTIONS_HXX
#define INCLUDED_UNOTOOLS_USEROPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>
#include <memory>

// define ----------------------------------------------------------------
enum class UserOptToken
{
    City               = 0,
    Company            = 1,
    Country            = 2,
    Email              = 3,
    Fax                = 4,
    FirstName          = 5,
    LastName           = 6,
    Position           = 7,
    State              = 8,
    Street             = 9,
    TelephoneHome      = 10,
    TelephoneWork      = 11,
    Title              = 12,
    ID                 = 13,
    Zip                = 14,
    FathersName        = 15,
    Apartment          = 16,
    SigningKey         = 17,
    EncryptionKey      = 18,
    EncryptToSelf      = 19,
    SigningKeyDisplayName = 20,
    EncryptionKeyDisplayName = 21,
    LAST               = EncryptionKeyDisplayName,
};

// class SvtUserOptions --------------------------------------------------

class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtUserOptions final : public utl::detail::Options
{
public:
    SvtUserOptions ();
    virtual ~SvtUserOptions () override;

    // get the address token
    OUString GetCompany        () const;
    OUString GetFirstName      () const;
    OUString GetLastName       () const;
    OUString GetID             () const;
    OUString GetStreet         () const;
    OUString GetCity           () const;
    OUString GetState          () const;
    OUString GetZip            () const;
    OUString GetCountry        () const;
    OUString GetPosition       () const;
    OUString GetTitle          () const;
    OUString GetTelephoneHome  () const;
    OUString GetTelephoneWork  () const;
    OUString GetFax            () const;
    OUString GetEmail          () const;
    OUString GetSigningKey     () const;
    OUString GetEncryptionKey  () const;
    bool GetEncryptToSelf      () const;
    OUString GetSigningKeyDisplayName () const;
    OUString GetEncryptionKeyDisplayName () const;

    OUString GetFullName       () const;

    bool      IsTokenReadonly (UserOptToken nToken) const;
    OUString  GetToken (UserOptToken nToken) const;
    void      SetToken (UserOptToken nToken, OUString const& rNewToken);
    void      SetBoolValue (UserOptToken nToken, bool bNewValue);

private:
    class SAL_DLLPRIVATE Impl;
    std::shared_ptr<Impl> xImpl;
    static std::weak_ptr<Impl> xSharedImpl;
private:
    class SAL_DLLPRIVATE ChangeListener;
};

#endif // INCLUDED_UNOTOOLS_USEROPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
