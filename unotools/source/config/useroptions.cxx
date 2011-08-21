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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include <unotools/useroptions.hxx>
#include <unotools/useroptions_const.hxx>

#include <unotools/configmgr.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
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
#include <unotools/processfactory.hxx>
#include <unotools/loghelper.hxx>

using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using ::rtl::OUString;

namespace css = ::com::sun::star;

// class SvtUserOptions_Impl ---------------------------------------------
class SvtUserOptions_Impl;
class SvtUserConfigChangeListener_Impl : public cppu::WeakImplHelper1
<
    com::sun::star::util::XChangesListener
>
{
        SvtUserOptions_Impl&    m_rParent;
    public:
        SvtUserConfigChangeListener_Impl(SvtUserOptions_Impl& rParent);
        ~SvtUserConfigChangeListener_Impl();

    //XChangesListener
    virtual void SAL_CALL changesOccurred( const util::ChangesEvent& Event ) throw(RuntimeException);
    //XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) throw(RuntimeException);
};

class SvtUserOptions_Impl : public utl::ConfigurationBroadcaster
{
public:
    SvtUserOptions_Impl();
    ~SvtUserOptions_Impl();

    // get the user token
    ::rtl::OUString   GetCompany() const;
    ::rtl::OUString   GetFirstName() const;
    ::rtl::OUString   GetLastName() const;
    ::rtl::OUString   GetID() const;
    ::rtl::OUString   GetStreet() const;
    ::rtl::OUString   GetCity() const;
    ::rtl::OUString   GetState() const;
    ::rtl::OUString   GetZip() const;
    ::rtl::OUString   GetCountry() const;
    ::rtl::OUString   GetPosition() const;
    ::rtl::OUString   GetTitle() const;
    ::rtl::OUString   GetTelephoneHome() const;
    ::rtl::OUString   GetTelephoneWork() const;
    ::rtl::OUString   GetFax() const;
    ::rtl::OUString   GetEmail() const;
    ::rtl::OUString   GetCustomerNumber() const;
    ::rtl::OUString   GetFathersName() const;
    ::rtl::OUString   GetApartment() const;

    ::rtl::OUString   GetFullName() const;
    ::rtl::OUString   GetLocale() const { return m_aLocale; }

    // set the address token
    void              SetCompany( const ::rtl::OUString& rNewToken );
    void              SetFirstName( const ::rtl::OUString& rNewToken );
    void              SetLastName( const ::rtl::OUString& rNewToken );
    void              SetID( const ::rtl::OUString& rNewToken );
    void              SetStreet( const ::rtl::OUString& rNewToken );
    void              SetCity( const ::rtl::OUString& rNewToken );
    void              SetState( const ::rtl::OUString& rNewToken );
    void              SetZip( const ::rtl::OUString& rNewToken );
    void              SetCountry( const ::rtl::OUString& rNewToken );
    void              SetPosition( const ::rtl::OUString& rNewToken );
    void              SetTitle( const ::rtl::OUString& rNewToken );
    void              SetTelephoneHome( const ::rtl::OUString& rNewToken );
    void              SetTelephoneWork( const ::rtl::OUString& rNewToken );
    void              SetFax( const ::rtl::OUString& rNewToken );
    void              SetEmail( const ::rtl::OUString& rNewToken );
    void              SetCustomerNumber( const ::rtl::OUString& rNewToken );
    void              SetFathersName( const ::rtl::OUString& rNewToken );
    void              SetApartment( const ::rtl::OUString& rNewToken );

    sal_Bool          IsTokenReadonly( sal_uInt16 nToken ) const;
    ::rtl::OUString   GetToken(sal_uInt16 nToken) const;
    void              Notify();

private:
    uno::Reference< util::XChangesListener >           m_xChangeListener;
    css::uno::Reference< css::container::XNameAccess > m_xCfg;
    css::uno::Reference< css::beans::XPropertySet >    m_xData;
    ::rtl::OUString m_aLocale;
};

// global ----------------------------------------------------------------

static SvtUserOptions_Impl* pOptions = NULL;
static sal_Int32            nRefCount = 0;

#define READONLY_DEFAULT    sal_False

SvtUserConfigChangeListener_Impl::SvtUserConfigChangeListener_Impl(SvtUserOptions_Impl& rParent) :
    m_rParent( rParent )
{
}

SvtUserConfigChangeListener_Impl::~SvtUserConfigChangeListener_Impl()
{
}

void SvtUserConfigChangeListener_Impl::changesOccurred( const util::ChangesEvent& rEvent ) throw(RuntimeException)
{
    if(rEvent.Changes.getLength())
        m_rParent.Notify();
}

void SvtUserConfigChangeListener_Impl::disposing( const lang::EventObject& rSource ) throw(RuntimeException)
{
    try
    {
        uno::Reference< util::XChangesNotifier > xChgNot( rSource.Source, UNO_QUERY_THROW);
        xChgNot->removeChangesListener(this);
    }
    catch(Exception& )
    {
    }
}

// class SvtUserOptions_Impl ---------------------------------------------

// -----------------------------------------------------------------------
SvtUserOptions_Impl::SvtUserOptions_Impl() :
    m_xChangeListener( new SvtUserConfigChangeListener_Impl(*this) )
{
    try
    {
        m_xCfg = Reference< css::container::XNameAccess > (
            ::comphelper::ConfigurationHelper::openConfig(
            utl::getProcessServiceFactory(),
            s_sData,
            ::comphelper::ConfigurationHelper::E_STANDARD),
            css::uno::UNO_QUERY );

        m_xData = css::uno::Reference< css::beans::XPropertySet >(m_xCfg, css::uno::UNO_QUERY);
        uno::Reference< util::XChangesNotifier > xChgNot( m_xCfg, UNO_QUERY);
        try
        {
            xChgNot->addChangesListener( m_xChangeListener );
        }
        catch(RuntimeException& )
        {
        }
    }
    catch(const css::uno::Exception& ex)
    {
        m_xCfg.clear();
        LogHelper::logIt(ex);
    }

    Any aAny = ConfigManager::GetConfigManager().GetDirectConfigProperty( ConfigManager::LOCALE );
    ::rtl::OUString aLocale;
    if ( aAny >>= aLocale )
        m_aLocale = aLocale;
    else
    {
        DBG_ERRORFILE( "SvtUserOptions_Impl::SvtUserOptions_Impl(): no locale found" );
    }
}

// -----------------------------------------------------------------------

SvtUserOptions_Impl::~SvtUserOptions_Impl()
{
}

::rtl::OUString SvtUserOptions_Impl::GetCompany() const
{
    ::rtl::OUString sCompany;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_so) >>= sCompany;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return sCompany;
}

::rtl::OUString SvtUserOptions_Impl::GetFirstName() const
{
    ::rtl::OUString sFirstName;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sgivenname) >>= sFirstName;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sFirstName;
}

::rtl::OUString SvtUserOptions_Impl::GetLastName() const
{
    ::rtl::OUString sLastName;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_ssn) >>= sLastName;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sLastName;
}

::rtl::OUString SvtUserOptions_Impl::GetID() const
{
    ::rtl::OUString sID;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sinitials) >>= sID;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sID;
}

::rtl::OUString SvtUserOptions_Impl::GetStreet() const
{
    ::rtl::OUString sStreet;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sstreet) >>= sStreet;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sStreet;
}

::rtl::OUString SvtUserOptions_Impl::GetCity() const
{
    ::rtl::OUString sCity;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sl) >>= sCity;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sCity;
}

::rtl::OUString SvtUserOptions_Impl::GetState() const
{
    ::rtl::OUString sState;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sst) >>= sState;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sState;
}

::rtl::OUString SvtUserOptions_Impl::GetZip() const
{
    ::rtl::OUString sZip;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_spostalcode) >>= sZip;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sZip;
}

::rtl::OUString SvtUserOptions_Impl::GetCountry() const
{
    ::rtl::OUString sCountry;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sc) >>= sCountry;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sCountry;
}

::rtl::OUString SvtUserOptions_Impl::GetPosition() const
{
    ::rtl::OUString sPosition;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sposition) >>= sPosition;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sPosition;
}

::rtl::OUString SvtUserOptions_Impl::GetTitle() const
{
    ::rtl::OUString sTitle;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_stitle) >>= sTitle;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sTitle;
}

::rtl::OUString SvtUserOptions_Impl::GetTelephoneHome() const
{
    ::rtl::OUString sTelephoneHome;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_shomephone) >>= sTelephoneHome;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sTelephoneHome;
}

::rtl::OUString SvtUserOptions_Impl::GetTelephoneWork() const
{
    ::rtl::OUString sTelephoneWork;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_stelephonenumber) >>= sTelephoneWork;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sTelephoneWork;
}

::rtl::OUString SvtUserOptions_Impl::GetFax() const
{
    ::rtl::OUString sFax;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sfacsimiletelephonenumber) >>= sFax;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sFax;
}

::rtl::OUString SvtUserOptions_Impl::GetEmail() const
{
    ::rtl::OUString sEmail;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_smail) >>= sEmail;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sEmail;
}

::rtl::OUString SvtUserOptions_Impl::GetCustomerNumber() const
{
    ::rtl::OUString sCustomerNumber;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_scustomernumber) >>= sCustomerNumber;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sCustomerNumber;
}

::rtl::OUString SvtUserOptions_Impl::GetFathersName() const
{
    ::rtl::OUString sFathersName;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sfathersname) >>= sFathersName;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sFathersName;
}

::rtl::OUString SvtUserOptions_Impl::GetApartment() const
{
    ::rtl::OUString sApartment;

    try
    {
        if (m_xData.is())
            m_xData->getPropertyValue(s_sapartment) >>= sApartment;
    }
    catch ( const css::uno::Exception& ex )
    {
        LogHelper::logIt(ex);
    }

    return  sApartment;
}

void SvtUserOptions_Impl::SetCompany( const ::rtl::OUString& sCompany )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_so, css::uno::makeAny(::rtl::OUString(sCompany)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetFirstName( const ::rtl::OUString& sFirstName )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sgivenname, css::uno::makeAny(::rtl::OUString(sFirstName)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetLastName( const ::rtl::OUString& sLastName )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_ssn, css::uno::makeAny(::rtl::OUString(sLastName)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}
void SvtUserOptions_Impl::SetID( const ::rtl::OUString& sID )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sinitials, css::uno::makeAny(::rtl::OUString(sID)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetStreet( const ::rtl::OUString& sStreet )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sstreet, css::uno::makeAny(::rtl::OUString(sStreet)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetCity( const ::rtl::OUString& sCity )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sl, css::uno::makeAny(::rtl::OUString(sCity)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetState( const ::rtl::OUString& sState )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sst, css::uno::makeAny(::rtl::OUString(sState)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetZip( const ::rtl::OUString& sZip )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_spostalcode, css::uno::makeAny(::rtl::OUString(sZip)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetCountry( const ::rtl::OUString& sCountry )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sc, css::uno::makeAny(::rtl::OUString(sCountry)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetPosition( const ::rtl::OUString& sPosition )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sposition, css::uno::makeAny(::rtl::OUString(sPosition)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetTitle( const ::rtl::OUString& sTitle )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_stitle, css::uno::makeAny(::rtl::OUString(sTitle)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetTelephoneHome( const ::rtl::OUString& sTelephoneHome )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_shomephone, css::uno::makeAny(::rtl::OUString(sTelephoneHome)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetTelephoneWork( const ::rtl::OUString& sTelephoneWork )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_stelephonenumber, css::uno::makeAny(::rtl::OUString(sTelephoneWork)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetFax( const ::rtl::OUString& sFax )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sfacsimiletelephonenumber, css::uno::makeAny(::rtl::OUString(sFax)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetEmail( const ::rtl::OUString& sEmail )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_smail, css::uno::makeAny(::rtl::OUString(sEmail)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetCustomerNumber( const ::rtl::OUString& sCustomerNumber )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_scustomernumber, css::uno::makeAny(::rtl::OUString(sCustomerNumber)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetFathersName( const ::rtl::OUString& sFathersName )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sfathersname, css::uno::makeAny(::rtl::OUString(sFathersName)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

void SvtUserOptions_Impl::SetApartment( const ::rtl::OUString& sApartment )
{
    try
    {
        if (m_xData.is())
            m_xData->setPropertyValue(s_sapartment, css::uno::makeAny(::rtl::OUString(sApartment)));
        ::comphelper::ConfigurationHelper::flush(m_xCfg);
    }
    catch ( const css::uno::Exception& ex)
    {
        LogHelper::logIt(ex);
    }
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions_Impl::GetFullName() const
{
    ::rtl::OUString sFullName;

    sFullName = GetFirstName();
    sFullName.trim();
    if ( sFullName.getLength() )
        sFullName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
    sFullName += GetLastName();
    sFullName.trim();

    return sFullName;
}

// -----------------------------------------------------------------------

void SvtUserOptions_Impl::Notify()
{
    NotifyListeners(0);
}

// -----------------------------------------------------------------------

sal_Bool SvtUserOptions_Impl::IsTokenReadonly( sal_uInt16 nToken ) const
{
    css::uno::Reference< css::beans::XPropertySet >     xData(m_xCfg, css::uno::UNO_QUERY);
    css::uno::Reference< css::beans::XPropertySetInfo > xInfo = xData->getPropertySetInfo();
    css::beans::Property aProp;
    sal_Bool             bRet = sal_False;

    switch ( nToken )
    {
        case USER_OPT_COMPANY:
        {
            aProp = xInfo->getPropertyByName(s_so);
            bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
            break;
        }
        case USER_OPT_FIRSTNAME:
        {
                aProp = xInfo->getPropertyByName(s_sgivenname);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_LASTNAME:
        {
                aProp = xInfo->getPropertyByName(s_ssn);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_ID:
        {
                aProp = xInfo->getPropertyByName(s_sinitials);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_STREET:
        {
                aProp = xInfo->getPropertyByName(s_sstreet);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_CITY:
        {
                aProp = xInfo->getPropertyByName(s_sl);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_STATE:
        {
                aProp = xInfo->getPropertyByName(s_sst);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_ZIP:
        {
                aProp = xInfo->getPropertyByName(s_spostalcode);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_COUNTRY:
        {
                aProp = xInfo->getPropertyByName(s_sc);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_POSITION:
        {
                aProp = xInfo->getPropertyByName(s_sposition);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_TITLE:
        {
                aProp = xInfo->getPropertyByName(s_stitle);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_TELEPHONEHOME:
        {
                aProp = xInfo->getPropertyByName(s_shomephone);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_TELEPHONEWORK:
        {
                aProp = xInfo->getPropertyByName(s_stelephonenumber);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_FAX:
        {
                aProp = xInfo->getPropertyByName(s_sfacsimiletelephonenumber);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_EMAIL:
        {
                aProp = xInfo->getPropertyByName(s_smail);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
        }
        case USER_OPT_FATHERSNAME:
            {
                aProp = xInfo->getPropertyByName(s_sfathersname);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
            }
        case USER_OPT_APARTMENT:
            {
                aProp = xInfo->getPropertyByName(s_sapartment);
                bRet = ((aProp.Attributes & css::beans::PropertyAttribute::READONLY) == css::beans::PropertyAttribute::READONLY);
                break;
            }
        default:
            DBG_ERRORFILE( "SvtUserOptions_Impl::IsTokenReadonly(): invalid token" );
    }

    return bRet;
}

//------------------------------------------------------------------------
::rtl::OUString SvtUserOptions_Impl::GetToken(sal_uInt16 nToken) const
{
    ::rtl::OUString pRet;
    switch(nToken)
    {
        case USER_OPT_COMPANY:        pRet = GetCompany();     break;
        case USER_OPT_FIRSTNAME:      pRet = GetFirstName();   break;
        case USER_OPT_LASTNAME:       pRet = GetLastName();    break;
        case USER_OPT_ID:             pRet = GetID();          break;
        case USER_OPT_STREET:         pRet = GetStreet();      break;
        case USER_OPT_CITY:           pRet = GetCity();        break;
        case USER_OPT_STATE:          pRet = GetState();       break;
        case USER_OPT_ZIP:            pRet = GetZip();         break;
        case USER_OPT_COUNTRY:        pRet = GetCountry();     break;
        case USER_OPT_POSITION:       pRet = GetPosition();    break;
        case USER_OPT_TITLE:          pRet = GetTitle();       break;
        case USER_OPT_TELEPHONEHOME:  pRet = GetTelephoneHome(); break;
        case USER_OPT_TELEPHONEWORK:  pRet = GetTelephoneWork(); break;
        case USER_OPT_FAX:            pRet = GetFax();           break;
        case USER_OPT_EMAIL:          pRet = GetEmail();         break;
        case USER_OPT_FATHERSNAME:    pRet = GetFathersName();   break;
        case USER_OPT_APARTMENT:      pRet = GetApartment();     break;
        default:
            DBG_ERRORFILE( "SvtUserOptions_Impl::GetToken(): invalid token" );
    }
    return pRet;
}

// class SvtUserOptions --------------------------------------------------

SvtUserOptions::SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( GetInitMutex() );

    if ( !pOptions )
    {
        RTL_LOGFILE_CONTEXT(aLog, "unotools ( ??? ) ::SvtUserOptions_Impl::ctor()");
        pOptions = new SvtUserOptions_Impl;

        ItemHolder1::holdConfigItem(E_USEROPTIONS);
    }
    ++nRefCount;
    pImp = pOptions;
    pImp->AddListener(this);
}

// -----------------------------------------------------------------------

SvtUserOptions::~SvtUserOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->RemoveListener(this);
    if ( !--nRefCount )
    {
        //if ( pOptions->IsModified() )
        //  pOptions->Commit();
        DELETEZ( pOptions );
    }
}

// -----------------------------------------------------------------------

namespace
{
    class theUserOptionsMutex : public rtl::Static<osl::Mutex, theUserOptionsMutex>{};
}

::osl::Mutex& SvtUserOptions::GetInitMutex()
{
    return theUserOptionsMutex::get();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetCompany() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetCompany();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetFirstName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetFirstName();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetLastName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetLastName();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetID() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetID();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetStreet() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetStreet();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetCity() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetCity();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetState() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetState();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetZip() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetZip();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetCountry() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetCountry();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetPosition() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetPosition();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetTitle() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetTitle();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetTelephoneHome() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetTelephoneHome();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetTelephoneWork() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetTelephoneWork();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetFax() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetFax();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetEmail() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetEmail();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetCustomerNumber() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetCustomerNumber();
}
// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetFathersName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetFathersName() ;
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetApartment() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetApartment();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetFullName() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetFullName();
}

// -----------------------------------------------------------------------

::rtl::OUString SvtUserOptions::GetLocale() const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetLocale();
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCompany( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetCompany( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetFirstName( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetFirstName( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetLastName( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetLastName( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetID( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetID( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetStreet( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetStreet( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCity( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetCity( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetState( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetState( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetZip( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetZip( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCountry( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetCountry( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetPosition( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetPosition( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTitle( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetTitle( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTelephoneHome( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetTelephoneHome( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetTelephoneWork( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetTelephoneWork( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetFax( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetFax( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetEmail( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetEmail( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetCustomerNumber( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetCustomerNumber( rNewToken );
}
// -----------------------------------------------------------------------

void SvtUserOptions::SetFathersName( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetFathersName( rNewToken );
}

// -----------------------------------------------------------------------

void SvtUserOptions::SetApartment( const ::rtl::OUString& rNewToken )
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    pImp->SetApartment( rNewToken );
}

// -----------------------------------------------------------------------

sal_Bool SvtUserOptions::IsTokenReadonly( sal_uInt16 nToken ) const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->IsTokenReadonly( nToken );
}
//------------------------------------------------------------------------
::rtl::OUString   SvtUserOptions::GetToken(sal_uInt16 nToken) const
{
    ::osl::MutexGuard aGuard( GetInitMutex() );
    return pImp->GetToken( nToken );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
