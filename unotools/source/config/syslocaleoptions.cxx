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

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <rtl/logfile.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.hxx>

#include "itemholder1.hxx"

#define CFG_READONLY_DEFAULT    sal_False

using namespace osl;
using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

using ::rtl::OUString;

SvtSysLocaleOptions_Impl*   SvtSysLocaleOptions::pOptions = NULL;
sal_Int32                   SvtSysLocaleOptions::nRefCount = 0;
namespace
{
    struct CurrencyChangeLink
        : public rtl::Static<Link, CurrencyChangeLink> {};
}

com::sun::star::lang::Locale lcl_str_to_locale( const ::rtl::OUString rStr )
{
    com::sun::star::lang::Locale aRet;
    if ( !rStr.isEmpty() )
    {
        aRet = com::sun::star::lang::Locale();
        sal_Int32 nSep = rStr.indexOf('-');
        if (nSep < 0)
            aRet.Language = rStr;
        else
        {
            aRet.Language = rStr.copy(0, nSep);
            if (nSep < rStr.getLength())
                aRet.Country = rStr.copy(nSep+1, rStr.getLength() - (nSep+1));
        }
    }

    return aRet;
}

class SvtSysLocaleOptions_Impl : public utl::ConfigItem
{
        Locale                  m_aRealLocale;
        Locale                  m_aRealUILocale;
        LanguageType            m_eRealLanguage;
        LanguageType            m_eRealUILanguage;
        OUString                m_aLocaleString;    // en-US or de-DE or empty for SYSTEM
        OUString                m_aUILocaleString;    // en-US or de-DE or empty for SYSTEM
        OUString                m_aCurrencyString;  // USD-en-US or EUR-de-DE
        OUString                m_aDatePatternsString;  // "Y-M-D;M-D"
        sal_Bool                m_bDecimalSeparator; //use decimal separator same as locale

        sal_Bool                m_bROLocale;
        sal_Bool                m_bROUILocale;
        sal_Bool                m_bROCurrency;
        sal_Bool                m_bRODatePatterns;
        sal_Bool                m_bRODecimalSeparator;

        static  const Sequence< /* const */ OUString >  GetPropertyNames();
        void                    MakeRealLocale();
        void                    MakeRealUILocale();

public:
                                SvtSysLocaleOptions_Impl();
    virtual                     ~SvtSysLocaleOptions_Impl();

    virtual void                Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void                Commit();

            const OUString&     GetLocaleString() const
                                    { return m_aLocaleString; }
            void                SetLocaleString( const OUString& rStr );

            const OUString&     GetUILocaleString() const
                                    { return m_aUILocaleString; }
            void                SetUILocaleString( const OUString& rStr );

            const OUString&     GetCurrencyString() const
                                    { return m_aCurrencyString; }
            void                SetCurrencyString( const OUString& rStr );

            const OUString&     GetDatePatternsString() const
                                    { return m_aDatePatternsString; }
            void                SetDatePatternsString( const OUString& rStr );

            sal_Bool            IsDecimalSeparatorAsLocale() const { return m_bDecimalSeparator;}
            void                SetDecimalSeparatorAsLocale( sal_Bool bSet);

            sal_Bool            IsReadOnly( SvtSysLocaleOptions::EOption eOption ) const;
            const Locale&       GetRealLocale() { return m_aRealLocale; }
            const Locale&       GetRealUILocale() { return m_aRealUILocale; }
            LanguageType        GetRealLanguage() { return m_eRealLanguage; }
            LanguageType        GetRealUILanguage() { return m_eRealUILanguage; }
};


#define ROOTNODE_SYSLOCALE              OUString(RTL_CONSTASCII_USTRINGPARAM("Setup/L10N"))

#define PROPERTYNAME_LOCALE             OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupSystemLocale"))
#define PROPERTYNAME_UILOCALE           OUString(RTL_CONSTASCII_USTRINGPARAM("ooLocale"))
#define PROPERTYNAME_CURRENCY           OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupCurrency"))
#define PROPERTYNAME_DECIMALSEPARATOR   OUString(RTL_CONSTASCII_USTRINGPARAM("DecimalSeparatorAsLocale"))
#define PROPERTYNAME_DATEPATTERNS       OUString(RTL_CONSTASCII_USTRINGPARAM("DateAcceptancePatterns"))

#define PROPERTYHANDLE_LOCALE           0
#define PROPERTYHANDLE_UILOCALE         1
#define PROPERTYHANDLE_CURRENCY         2
#define PROPERTYHANDLE_DECIMALSEPARATOR 3
#define PROPERTYHANDLE_DATEPATTERNS     4

#define PROPERTYCOUNT                   5

const Sequence< OUString > SvtSysLocaleOptions_Impl::GetPropertyNames()
{
    const OUString pProperties[] =
    {
        PROPERTYNAME_LOCALE,
        PROPERTYNAME_UILOCALE,
        PROPERTYNAME_CURRENCY,
        PROPERTYNAME_DECIMALSEPARATOR,
        PROPERTYNAME_DATEPATTERNS
    };
    const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    return seqPropertyNames;
}

// -----------------------------------------------------------------------

SvtSysLocaleOptions_Impl::SvtSysLocaleOptions_Impl()
    : ConfigItem( ROOTNODE_SYSLOCALE )
    , m_bDecimalSeparator( sal_True )
    , m_bROLocale(CFG_READONLY_DEFAULT)
    , m_bROUILocale(CFG_READONLY_DEFAULT)
    , m_bROCurrency(CFG_READONLY_DEFAULT)
    , m_bRODatePatterns(CFG_READONLY_DEFAULT)
    , m_bRODecimalSeparator(sal_False)

{
    if ( IsValidConfigMgr() )
    {
        const Sequence< OUString > aNames = GetPropertyNames();
        Sequence< Any > aValues = GetProperties( aNames );
        Sequence< sal_Bool > aROStates = GetReadOnlyStates( aNames );
        const Any* pValues = aValues.getConstArray();
        const sal_Bool* pROStates = aROStates.getConstArray();
        DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
        DBG_ASSERT( aROStates.getLength() == aNames.getLength(), "GetReadOnlyStates failed" );
        if ( aValues.getLength() == aNames.getLength() && aROStates.getLength() == aNames.getLength() )
        {
            for ( sal_Int32 nProp = 0; nProp < aNames.getLength(); nProp++ )
            {
                if ( pValues[nProp].hasValue() )
                {
                    switch ( nProp )
                    {
                        case PROPERTYHANDLE_LOCALE :
                            {
                                OUString aStr;
                                if ( pValues[nProp] >>= aStr )
                                    m_aLocaleString = aStr;
                                else
                                {
                                    SAL_WARN( "unotools.config", "Wrong property type!" );
                                }
                                m_bROLocale = pROStates[nProp];
                            }
                            break;
                        case PROPERTYHANDLE_UILOCALE :
                            {
                                OUString aStr;
                                if ( pValues[nProp] >>= aStr )
                                    m_aUILocaleString = aStr;
                                else
                                {
                                    SAL_WARN( "unotools.config", "Wrong property type!" );
                                }
                                m_bROUILocale = pROStates[nProp];
                            }
                            break;
                        case PROPERTYHANDLE_CURRENCY :
                            {
                                OUString aStr;
                                if ( pValues[nProp] >>= aStr )
                                    m_aCurrencyString = aStr;
                                else
                                {
                                    SAL_WARN( "unotools.config", "Wrong property type!" );
                                }
                                m_bROCurrency = pROStates[nProp];
                            }
                        break;
                        case  PROPERTYHANDLE_DECIMALSEPARATOR:
                        {
                                    sal_Bool bValue = sal_Bool();
                            if ( pValues[nProp] >>= bValue )
                                m_bDecimalSeparator = bValue;
                            else
                            {
                                SAL_WARN( "unotools.config", "Wrong property type!" );
                            }
                            m_bRODecimalSeparator = pROStates[nProp];
                        }
                        break;
                        case PROPERTYHANDLE_DATEPATTERNS :
                            {
                                OUString aStr;
                                if ( pValues[nProp] >>= aStr )
                                    m_aDatePatternsString = aStr;
                                else
                                {
                                    SAL_WARN( "unotools.config", "Wrong property type!" );
                                }
                                m_bRODatePatterns = pROStates[nProp];
                            }
                        break;
                        default:
                            SAL_WARN( "unotools.config", "Wrong property type!" );
                    }
                }
            }
        }
        EnableNotification( aNames );
    }

    MakeRealLocale();
    MakeRealUILocale();
}


SvtSysLocaleOptions_Impl::~SvtSysLocaleOptions_Impl()
{
    if ( IsModified() )
        Commit();
}

void SvtSysLocaleOptions_Impl::MakeRealLocale()
{
    m_aRealLocale = lcl_str_to_locale( m_aLocaleString );
    if ( !m_aRealLocale.Language.isEmpty() )
    {
        m_eRealLanguage = MsLangId::convertLocaleToLanguage( m_aRealLocale );
    }
    else
    {
        m_eRealLanguage = MsLangId::getSystemLanguage();
        MsLangId::convertLanguageToLocale( m_eRealLanguage, m_aRealLocale );
    }
}

void SvtSysLocaleOptions_Impl::MakeRealUILocale()
{
    // as we can't switch UILocale at runtime, we only store changes in the configuration
    m_aRealUILocale = lcl_str_to_locale( m_aUILocaleString );
    if ( !m_aRealUILocale.Language.isEmpty() )
    {
        m_eRealUILanguage = MsLangId::convertLocaleToLanguage( m_aRealUILocale );
    }
    else
    {
        m_eRealUILanguage = MsLangId::getSystemUILanguage();
        MsLangId::convertLanguageToLocale( m_eRealUILanguage, m_aRealUILocale );
    }
}

sal_Bool SvtSysLocaleOptions_Impl::IsReadOnly( SvtSysLocaleOptions::EOption eOption ) const
{
    sal_Bool bReadOnly = CFG_READONLY_DEFAULT;
    switch(eOption)
    {
        case SvtSysLocaleOptions::E_LOCALE :
            {
                bReadOnly = m_bROLocale;
                break;
            }
        case SvtSysLocaleOptions::E_UILOCALE :
            {
                bReadOnly = m_bROUILocale;
                break;
            }
        case SvtSysLocaleOptions::E_CURRENCY :
            {
                bReadOnly = m_bROCurrency;
                break;
            }
        case SvtSysLocaleOptions::E_DATEPATTERNS :
            {
                bReadOnly = m_bRODatePatterns;
                break;
            }
    }
    return bReadOnly;
}


void SvtSysLocaleOptions_Impl::Commit()
{
    const Sequence< OUString > aOrgNames = GetPropertyNames();
    sal_Int32 nOrgCount = aOrgNames.getLength();

    Sequence< OUString > aNames( nOrgCount );
    Sequence< Any > aValues( nOrgCount );

    OUString* pNames = aNames.getArray();
    Any* pValues = aValues.getArray();
    sal_Int32 nRealCount = 0;

    for ( sal_Int32 nProp = 0; nProp < nOrgCount; nProp++ )
    {
        switch ( nProp )
        {
            case PROPERTYHANDLE_LOCALE :
                {
                    if (!m_bROLocale)
                    {
                        pNames[nRealCount] = aOrgNames[nProp];
                        pValues[nRealCount] <<= m_aLocaleString;
                        ++nRealCount;
                    }
                }
                break;
            case PROPERTYHANDLE_UILOCALE :
                {
                    if (!m_bROUILocale)
                    {
                        pNames[nRealCount] = aOrgNames[nProp];
                        pValues[nRealCount] <<= m_aUILocaleString;
                        ++nRealCount;
                    }
                }
                break;
            case PROPERTYHANDLE_CURRENCY :
                {
                    if (!m_bROCurrency)
                    {
                        pNames[nRealCount] = aOrgNames[nProp];
                        pValues[nRealCount] <<= m_aCurrencyString;
                        ++nRealCount;
                    }
                }
                break;
            case  PROPERTYHANDLE_DECIMALSEPARATOR:
                if( !m_bRODecimalSeparator )
                {
                    pNames[nRealCount] = aOrgNames[nProp];
                    pValues[nRealCount] <<= m_bDecimalSeparator;
                    ++nRealCount;
                }
            break;
            case PROPERTYHANDLE_DATEPATTERNS :
                if (!m_bRODatePatterns)
                {
                    pNames[nRealCount] = aOrgNames[nProp];
                    pValues[nRealCount] <<= m_aDatePatternsString;
                    ++nRealCount;
                }
                break;
            default:
                SAL_WARN( "unotools.config", "invalid index to save a path" );
        }
    }
    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties( aNames, aValues );
    ClearModified();
}


void SvtSysLocaleOptions_Impl::SetLocaleString( const OUString& rStr )
{
    if (!m_bROLocale && rStr != m_aLocaleString )
    {
        m_aLocaleString = rStr;
        MakeRealLocale();
        MsLangId::setConfiguredSystemLanguage( m_eRealLanguage );
        SetModified();
        sal_uLong nHint = SYSLOCALEOPTIONS_HINT_LOCALE;
        if ( m_aCurrencyString.isEmpty() )
            nHint |= SYSLOCALEOPTIONS_HINT_CURRENCY;
        NotifyListeners( nHint );
    }
}

void SvtSysLocaleOptions_Impl::SetUILocaleString( const OUString& rStr )
{
    if (!m_bROUILocale && rStr != m_aUILocaleString )
    {
        m_aUILocaleString = rStr;

        // as we can't switch UILocale at runtime, we only store changes in the configuration
        MakeRealUILocale();
        MsLangId::setConfiguredSystemLanguage( m_eRealUILanguage );
        SetModified();
        NotifyListeners( SYSLOCALEOPTIONS_HINT_UILOCALE );
    }
}

void SvtSysLocaleOptions_Impl::SetCurrencyString( const OUString& rStr )
{
    if (!m_bROCurrency && rStr != m_aCurrencyString )
    {
        m_aCurrencyString = rStr;
        SetModified();
        NotifyListeners( SYSLOCALEOPTIONS_HINT_CURRENCY );
    }
}

void SvtSysLocaleOptions_Impl::SetDatePatternsString( const OUString& rStr )
{
    if (!m_bRODatePatterns && rStr != m_aDatePatternsString )
    {
        m_aDatePatternsString = rStr;
        SetModified();
        NotifyListeners( SYSLOCALEOPTIONS_HINT_DATEPATTERNS );
    }
}

void SvtSysLocaleOptions_Impl::SetDecimalSeparatorAsLocale( sal_Bool bSet)
{
    if(bSet != m_bDecimalSeparator)
    {
        m_bDecimalSeparator = bSet;
        SetModified();
        NotifyListeners( SYSLOCALEOPTIONS_HINT_DECSEP );
    }
}

void SvtSysLocaleOptions_Impl::Notify( const Sequence< rtl::OUString >& seqPropertyNames )
{
    sal_uLong nHint = 0;
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    Sequence< sal_Bool > seqROStates = GetReadOnlyStates( seqPropertyNames );
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProp = 0; nProp < nCount; ++nProp )
    {
        if( seqPropertyNames[nProp] == PROPERTYNAME_LOCALE )
        {
            DBG_ASSERT( seqValues[nProp].getValueTypeClass() == TypeClass_STRING, "Locale property type" );
            seqValues[nProp] >>= m_aLocaleString;
            m_bROLocale = seqROStates[nProp];
            nHint |= SYSLOCALEOPTIONS_HINT_LOCALE;
            if ( m_aCurrencyString.isEmpty() )
                nHint |= SYSLOCALEOPTIONS_HINT_CURRENCY;
            MakeRealLocale();
        }
        if( seqPropertyNames[nProp] == PROPERTYNAME_UILOCALE )
        {
            DBG_ASSERT( seqValues[nProp].getValueTypeClass() == TypeClass_STRING, "Locale property type" );
            seqValues[nProp] >>= m_aUILocaleString;
            m_bROUILocale = seqROStates[nProp];
            nHint |= SYSLOCALEOPTIONS_HINT_UILOCALE;
            MakeRealUILocale();
        }
        else if( seqPropertyNames[nProp] == PROPERTYNAME_CURRENCY )
        {
            DBG_ASSERT( seqValues[nProp].getValueTypeClass() == TypeClass_STRING, "Currency property type" );
            seqValues[nProp] >>= m_aCurrencyString;
            m_bROCurrency = seqROStates[nProp];
            nHint |= SYSLOCALEOPTIONS_HINT_CURRENCY;
        }
        else if( seqPropertyNames[nProp] == PROPERTYNAME_DECIMALSEPARATOR )
        {
            seqValues[nProp] >>= m_bDecimalSeparator;
            m_bRODecimalSeparator = seqROStates[nProp];
        }
        else if( seqPropertyNames[nProp] == PROPERTYNAME_DATEPATTERNS )
        {
            DBG_ASSERT( seqValues[nProp].getValueTypeClass() == TypeClass_STRING, "DatePatterns property type" );
            seqValues[nProp] >>= m_aDatePatternsString;
            m_bRODatePatterns = seqROStates[nProp];
            nHint |= SYSLOCALEOPTIONS_HINT_DATEPATTERNS;
        }
    }
    if ( nHint )
        NotifyListeners( nHint );
}

// ====================================================================

SvtSysLocaleOptions::SvtSysLocaleOptions()
{
    MutexGuard aGuard( GetMutex() );
    if ( !pOptions )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svl ( ??? ) ::SvtSysLocaleOptions_Impl::ctor()");
        pOptions = new SvtSysLocaleOptions_Impl;

        ItemHolder1::holdConfigItem(E_SYSLOCALEOPTIONS);
    }
    ++nRefCount;
    pOptions->AddListener(this);
}


SvtSysLocaleOptions::~SvtSysLocaleOptions()
{
    MutexGuard aGuard( GetMutex() );
    pOptions->RemoveListener(this);
    if ( !--nRefCount )
    {
        delete pOptions;
        pOptions = NULL;
    }
}


// static
Mutex& SvtSysLocaleOptions::GetMutex()
{
    static Mutex* pMutex = NULL;
    if( !pMutex )
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if( !pMutex )
        {
            // #i77768# Due to a static reference in the toolkit lib
            // we need a mutex that lives longer than the svl library.
            // Otherwise the dtor would use a destructed mutex!!
            pMutex = new Mutex;
        }
    }
    return *pMutex;
}


sal_Bool SvtSysLocaleOptions::IsModified()
{
    MutexGuard aGuard( GetMutex() );
    return pOptions->IsModified();
}


void SvtSysLocaleOptions::Commit()
{
    MutexGuard aGuard( GetMutex() );
    pOptions->Commit();
}


void SvtSysLocaleOptions::BlockBroadcasts( bool bBlock )
{
    MutexGuard aGuard( GetMutex() );
    pOptions->BlockBroadcasts( bBlock );
}


const OUString& SvtSysLocaleOptions::GetLocaleConfigString() const
{
    MutexGuard aGuard( GetMutex() );
    return pOptions->GetLocaleString();
}

void SvtSysLocaleOptions::SetLocaleConfigString( const OUString& rStr )
{
    MutexGuard aGuard( GetMutex() );
    pOptions->SetLocaleString( rStr );
}

void SvtSysLocaleOptions::SetUILocaleConfigString( const OUString& rStr )
{
    MutexGuard aGuard( GetMutex() );
    pOptions->SetUILocaleString( rStr );
}

const OUString& SvtSysLocaleOptions::GetCurrencyConfigString() const
{
    MutexGuard aGuard( GetMutex() );
    return pOptions->GetCurrencyString();
}


void SvtSysLocaleOptions::SetCurrencyConfigString( const OUString& rStr )
{
    MutexGuard aGuard( GetMutex() );
    pOptions->SetCurrencyString( rStr );
}

const OUString& SvtSysLocaleOptions::GetDatePatternsConfigString() const
{
    MutexGuard aGuard( GetMutex() );
    return pOptions->GetDatePatternsString();
}

void SvtSysLocaleOptions::SetDatePatternsConfigString( const OUString& rStr )
{
    MutexGuard aGuard( GetMutex() );
    pOptions->SetDatePatternsString( rStr );
}

sal_Bool SvtSysLocaleOptions::IsDecimalSeparatorAsLocale() const
{
    MutexGuard aGuard( GetMutex() );
    return pOptions->IsDecimalSeparatorAsLocale();
}

void SvtSysLocaleOptions::SetDecimalSeparatorAsLocale( sal_Bool bSet)
{
    MutexGuard aGuard( GetMutex() );
    pOptions->SetDecimalSeparatorAsLocale(bSet);
}


sal_Bool SvtSysLocaleOptions::IsReadOnly( EOption eOption ) const
{
    MutexGuard aGuard( GetMutex() );
    return pOptions->IsReadOnly( eOption );
}

// static
void SvtSysLocaleOptions::GetCurrencyAbbrevAndLanguage( String& rAbbrev,
        LanguageType& eLang, const ::rtl::OUString& rConfigString )
{
    sal_Int32 nDelim = rConfigString.indexOf( '-' );
    if ( nDelim >= 0 )
    {
        rAbbrev = rConfigString.copy( 0, nDelim );
        String aIsoStr( rConfigString.copy( nDelim+1 ) );
        eLang = MsLangId::convertIsoStringToLanguage( aIsoStr );
    }
    else
    {
        rAbbrev = rConfigString;
        eLang = (rAbbrev.Len() ? LANGUAGE_NONE : LANGUAGE_SYSTEM);
    }
}


// static
::rtl::OUString SvtSysLocaleOptions::CreateCurrencyConfigString(
        const String& rAbbrev, LanguageType eLang )
{
    String aIsoStr( MsLangId::convertLanguageToIsoString( eLang ) );
    if ( aIsoStr.Len() )
    {
        ::rtl::OUStringBuffer aStr( rAbbrev.Len() + 1 + aIsoStr.Len() );
        aStr.append( rAbbrev );
        aStr.append( sal_Unicode('-') );
        aStr.append( aIsoStr );
        return aStr.makeStringAndClear();
    }
    else
        return rAbbrev;
}


// static
void SvtSysLocaleOptions::SetCurrencyChangeLink( const Link& rLink )
{
    MutexGuard aGuard( GetMutex() );
    DBG_ASSERT( !CurrencyChangeLink::get().IsSet(), "SvtSysLocaleOptions::SetCurrencyChangeLink: already set" );
    CurrencyChangeLink::get() = rLink;
}


// static
const Link& SvtSysLocaleOptions::GetCurrencyChangeLink()
{
    MutexGuard aGuard( GetMutex() );
    return CurrencyChangeLink::get();
}


void SvtSysLocaleOptions::ConfigurationChanged( utl::ConfigurationBroadcaster* p, sal_uInt32 nHint  )
{
    if ( nHint & SYSLOCALEOPTIONS_HINT_CURRENCY )
    {
        const Link& rLink = GetCurrencyChangeLink();
        if ( rLink.IsSet() )
            rLink.Call( NULL );
    }

    ::utl::detail::Options::ConfigurationChanged( p, nHint );
}

com::sun::star::lang::Locale SvtSysLocaleOptions::GetLocale() const
{
    return lcl_str_to_locale( GetLocaleConfigString() );
}

com::sun::star::lang::Locale SvtSysLocaleOptions::GetRealLocale() const
{
    return pOptions->GetRealLocale();
}

com::sun::star::lang::Locale SvtSysLocaleOptions::GetRealUILocale() const
{
    return pOptions->GetRealUILocale();
}

LanguageType SvtSysLocaleOptions::GetRealLanguage() const
{
    return pOptions->GetRealLanguage();
}

LanguageType SvtSysLocaleOptions::GetRealUILanguage() const
{
    return pOptions->GetRealUILanguage();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
