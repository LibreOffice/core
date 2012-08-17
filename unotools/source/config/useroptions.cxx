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


#include <unotools/useroptions.hxx>

#include <unotools/configmgr.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/solar.h>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/ChangesEvent.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>

using namespace utl;
using namespace com::sun::star;
//using namespace com::sun::star::uno;
using rtl::OUString;

//namespace css = ::com::sun::star;

namespace
{

OUString const sData = "org.openoffice.UserProfile/Data";

// vOptionNames[] -- names of the user option entries
// The order corresponds to the #define USER_OPT_* list in useroptions.hxx.
OUString const vOptionNames[] = {
    "l",                         // USER_OPT_CITY
    "o",                         // USER_OPT_COMPANY
    "c",                         // USER_OPT_COUNTRY
    "mail",                      // USER_OPT_EMAIL
    "facsimiletelephonenumber",  // USER_OPT_FAX
    "givenname",                 // USER_OPT_FIRSTNAME
    "sn",                        // USER_OPT_LASTNAME
    "position",                  // USER_OPT_POSITION
    "st",                        // USER_OPT_STATE
    "street",                    // USER_OPT_STREET
    "homephone",                 // USER_OPT_TELEPHONEHOME
    "telephonenumber",           // USER_OPT_TELEPHONEWORK
    "title",                     // USER_OPT_TITLE
    "initials",                  // USER_OPT_ID
    "postalcode",                // USER_OPT_ZIP
    "fathersname",               // USER_OPT_FATHERSNAME
    "apartment",                 // USER_OPT_APARTMENT
    "customernumber",            // USER_OPT_CUSTOMERNUMBER
};
unsigned const nOptionNameCount = sizeof vOptionNames / sizeof vOptionNames[0];

} // namespace

boost::weak_ptr<SvtUserOptions::Impl> SvtUserOptions::pSharedImpl;

// class ChangeListener --------------------------------------------------

class SvtUserOptions::ChangeListener : public cppu::WeakImplHelper1<util::XChangesListener>
{
public:
    ChangeListener (Impl& rParent): m_rParent(rParent) { }

    // XChangesListener
    virtual void SAL_CALL changesOccurred (util::ChangesEvent const& Event) throw(uno::RuntimeException);
    // XEventListener
    virtual void SAL_CALL disposing (lang::EventObject const& Source) throw(uno::RuntimeException);

private:
    Impl& m_rParent;
};

// class Impl ------------------------------------------------------------

class SvtUserOptions::Impl : public utl::ConfigurationBroadcaster
{
public:
    Impl ();

    OUString GetFullName () const;

    sal_Bool IsTokenReadonly (sal_uInt16 nToken) const;
    OUString GetToken (sal_uInt16 nToken) const;
    void     SetToken (sal_uInt16 nToken, OUString const& rNewToken);
    void     Notify ();

private:
    uno::Reference<util::XChangesListener> m_xChangeListener;
    uno::Reference<container::XNameAccess> m_xCfg;
    uno::Reference<beans::XPropertySet>    m_xData;
};

// class SvtUserOptions::ChangeListener ----------------------------------

void SvtUserOptions::ChangeListener::changesOccurred (util::ChangesEvent const& rEvent) throw(uno::RuntimeException)
{
    if (rEvent.Changes.getLength())
        m_rParent.Notify();
}

void SvtUserOptions::ChangeListener::disposing (lang::EventObject const& rSource) throw(uno::RuntimeException)
{
    try
    {
        uno::Reference<util::XChangesNotifier> xChgNot(rSource.Source, uno::UNO_QUERY_THROW);
        xChgNot->removeChangesListener(this);
    }
    catch (uno::Exception&)
    {
    }
}

// class SvtUserOptions::Impl --------------------------------------------

SvtUserOptions::Impl::Impl() :
    m_xChangeListener( new ChangeListener(*this) )
{
    try
    {
        m_xCfg = uno::Reference<container::XNameAccess>(
            comphelper::ConfigurationHelper::openConfig(
                comphelper::getProcessServiceFactory(),
                sData,
                comphelper::ConfigurationHelper::E_STANDARD
            ),
            uno::UNO_QUERY
        );

        m_xData = uno::Reference<beans::XPropertySet>(m_xCfg, uno::UNO_QUERY);
        uno::Reference<util::XChangesNotifier> xChgNot(m_xCfg, uno::UNO_QUERY);
        try
        {
            xChgNot->addChangesListener(m_xChangeListener);
        }
        catch (uno::RuntimeException&)
        {
        }
    }
    catch (uno::Exception const& ex)
    {
        m_xCfg.clear();
        SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
    }
}

// -----------------------------------------------------------------------

OUString SvtUserOptions::Impl::GetToken (sal_uInt16 nToken) const
{
    OUString sToken;
    if (nToken < nOptionNameCount)
    {
        try
        {
            if (m_xData.is())
                m_xData->getPropertyValue(vOptionNames[nToken]) >>= sToken;
        }
        catch (uno::Exception const& ex)
        {
            SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
        }
    }
    else
        SAL_WARN("unotools.config", "SvtUserOptions::Impl::GetToken(): invalid token");
    return sToken;
}

// -----------------------------------------------------------------------

void SvtUserOptions::Impl::SetToken (sal_uInt16 nToken, OUString const& sToken)
{
    if (nToken < nOptionNameCount)
    {
        try
        {
            if (m_xData.is())
                m_xData->setPropertyValue(vOptionNames[nToken], uno::makeAny(sToken));
            comphelper::ConfigurationHelper::flush(m_xCfg);
        }
        catch (uno::Exception const& ex)
        {
            SAL_WARN("unotools", "Caught unexpected: " << ex.Message);
        }
    }
    else
        SAL_WARN("unotools.config", "SvtUserOptions::Impl::GetToken(): invalid token");
}

// -----------------------------------------------------------------------

OUString SvtUserOptions::Impl::GetFullName () const
{
    // TODO international name
    OUString sFullName = GetToken(USER_OPT_FIRSTNAME).trim();
    if (!sFullName.isEmpty())
        sFullName += " ";
    sFullName += GetToken(USER_OPT_LASTNAME).trim();
    return sFullName;
}

// -----------------------------------------------------------------------

void SvtUserOptions::Impl::Notify ()
{
    NotifyListeners(0);
}

// -----------------------------------------------------------------------

sal_Bool SvtUserOptions::Impl::IsTokenReadonly (sal_uInt16 nToken) const
{
    if (nToken < nOptionNameCount)
    {
        uno::Reference<beans::XPropertySet> xData(m_xCfg, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySetInfo> xInfo = xData->getPropertySetInfo();
        beans::Property aProp = xInfo->getPropertyByName(vOptionNames[nToken]);
        return ((aProp.Attributes & beans::PropertyAttribute::READONLY) ==
            beans::PropertyAttribute::READONLY);
    }
    else
    {
        SAL_WARN("unotools.config", "SvtUserOptions::Impl::IsTokenReadonly(): invalid token");
        return sal_False;
    }
}

// class SvtUserOptions --------------------------------------------------

SvtUserOptions::SvtUserOptions ()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard(GetInitMutex());

    if (pSharedImpl.expired())
    {
        RTL_LOGFILE_CONTEXT(aLog, "unotools ( ??? ) SvtUserOptions::Impl::ctor()");
        pImpl.reset(new Impl);
        pSharedImpl = pImpl;
        ItemHolder1::holdConfigItem(E_USEROPTIONS);
    }
    pImpl = pSharedImpl.lock();
    pImpl->AddListener(this);
}

// -----------------------------------------------------------------------

SvtUserOptions::~SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    osl::MutexGuard aGuard( GetInitMutex() );
    pImpl->RemoveListener(this);
}

// -----------------------------------------------------------------------

namespace
{
    class theUserOptionsMutex : public rtl::Static<osl::Mutex, theUserOptionsMutex>{};
}

osl::Mutex& SvtUserOptions::GetInitMutex()
{
    return theUserOptionsMutex::get();
}

// -----------------------------------------------------------------------

//
// getters
//
OUString SvtUserOptions::GetCompany        () const { return GetToken(USER_OPT_COMPANY); }
OUString SvtUserOptions::GetFirstName      () const { return GetToken(USER_OPT_FIRSTNAME); }
OUString SvtUserOptions::GetLastName       () const { return GetToken(USER_OPT_LASTNAME); }
OUString SvtUserOptions::GetID             () const { return GetToken(USER_OPT_ID); }
OUString SvtUserOptions::GetStreet         () const { return GetToken(USER_OPT_STREET); }
OUString SvtUserOptions::GetCity           () const { return GetToken(USER_OPT_CITY); }
OUString SvtUserOptions::GetState          () const { return GetToken(USER_OPT_STATE); }
OUString SvtUserOptions::GetZip            () const { return GetToken(USER_OPT_ZIP); }
OUString SvtUserOptions::GetCountry        () const { return GetToken(USER_OPT_COUNTRY); }
OUString SvtUserOptions::GetPosition       () const { return GetToken(USER_OPT_POSITION); }
OUString SvtUserOptions::GetTitle          () const { return GetToken(USER_OPT_TITLE); }
OUString SvtUserOptions::GetTelephoneHome  () const { return GetToken(USER_OPT_TELEPHONEHOME); }
OUString SvtUserOptions::GetTelephoneWork  () const { return GetToken(USER_OPT_TELEPHONEWORK); }
OUString SvtUserOptions::GetFax            () const { return GetToken(USER_OPT_FAX); }
OUString SvtUserOptions::GetEmail          () const { return GetToken(USER_OPT_EMAIL); }
OUString SvtUserOptions::GetCustomerNumber () const { return GetToken(USER_OPT_CUSTOMERNUMBER); }
OUString SvtUserOptions::GetFathersName    () const { return GetToken(USER_OPT_FATHERSNAME); }
OUString SvtUserOptions::GetApartment      () const { return GetToken(USER_OPT_APARTMENT); }

// -----------------------------------------------------------------------

//
// setters
//
void SvtUserOptions::SetCompany   (OUString const& sToken) { SetToken(USER_OPT_COMPANY,   sToken); }
void SvtUserOptions::SetFirstName (OUString const& sToken) { SetToken(USER_OPT_FIRSTNAME, sToken); }
void SvtUserOptions::SetLastName  (OUString const& sToken) { SetToken(USER_OPT_LASTNAME,  sToken); }
void SvtUserOptions::SetID        (OUString const& sToken) { SetToken(USER_OPT_ID,        sToken); }
void SvtUserOptions::SetStreet    (OUString const& sToken) { SetToken(USER_OPT_STREET,    sToken); }
void SvtUserOptions::SetCity      (OUString const& sToken) { SetToken(USER_OPT_CITY,      sToken); }
void SvtUserOptions::SetState     (OUString const& sToken) { SetToken(USER_OPT_STATE,     sToken); }
void SvtUserOptions::SetZip       (OUString const& sToken) { SetToken(USER_OPT_ZIP,       sToken); }
void SvtUserOptions::SetCountry   (OUString const& sToken) { SetToken(USER_OPT_COUNTRY,   sToken); }
void SvtUserOptions::SetPosition  (OUString const& sToken) { SetToken(USER_OPT_POSITION,  sToken); }
void SvtUserOptions::SetTitle     (OUString const& sToken) { SetToken(USER_OPT_TITLE,     sToken); }
void SvtUserOptions::SetFax       (OUString const& sToken) { SetToken(USER_OPT_FAX,       sToken); }
void SvtUserOptions::SetEmail     (OUString const& sToken) { SetToken(USER_OPT_EMAIL,     sToken); }
void SvtUserOptions::SetTelephoneHome  (OUString const& sToken) { SetToken(USER_OPT_TELEPHONEHOME,  sToken); }
void SvtUserOptions::SetTelephoneWork  (OUString const& sToken) { SetToken(USER_OPT_TELEPHONEWORK,  sToken); }
void SvtUserOptions::SetCustomerNumber (OUString const& sToken) { SetToken(USER_OPT_CUSTOMERNUMBER, sToken); }
void SvtUserOptions::SetFathersName    (OUString const& sToken) { SetToken(USER_OPT_FATHERSNAME,    sToken); }
void SvtUserOptions::SetApartment      (OUString const& sToken) { SetToken(USER_OPT_APARTMENT,      sToken); }

// -----------------------------------------------------------------------

sal_Bool SvtUserOptions::IsTokenReadonly (sal_uInt16 nToken) const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return pImpl->IsTokenReadonly(nToken);
}

// -----------------------------------------------------------------------

OUString SvtUserOptions::GetToken (sal_uInt16 nToken) const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return pImpl->GetToken(nToken);
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetToken (sal_uInt16 nToken, OUString const& rNewToken)
{
    osl::MutexGuard aGuard(GetInitMutex());
    pImpl->SetToken(nToken, rNewToken);
}

// -----------------------------------------------------------------------

OUString SvtUserOptions::GetFullName () const
{
    osl::MutexGuard aGuard(GetInitMutex());
    return pImpl->GetFullName();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
