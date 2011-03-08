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
    if ( rStr.getLength() )
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
        ULONG                   m_nBlockedHint;     // pending hints
        sal_Bool                m_bDecimalSeparator; //use decimal separator same as locale

        sal_Bool                m_bROLocale;
        sal_Bool                m_bROUILocale;
        sal_Bool                m_bROCurrency;
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

#define PROPERTYHANDLE_LOCALE           0
#define PROPERTYHANDLE_UILOCALE         1
#define PROPERTYHANDLE_CURRENCY         2
#define PROPERTYHANDLE_DECIMALSEPARATOR 3

#define PROPERTYCOUNT                   4

const Sequence< OUString > SvtSysLocaleOptions_Impl::GetPropertyNames()
{
    static const OUString pProperties[] =
    {
        PROPERTYNAME_LOCALE,
        PROPERTYNAME_UILOCALE,
        PROPERTYNAME_CURRENCY,
        PROPERTYNAME_DECIMALSEPARATOR
    };
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    return seqPropertyNames;
}

// -----------------------------------------------------------------------

SvtSysLocaleOptions_Impl::SvtSysLocaleOptions_Impl()
    : ConfigItem( ROOTNODE_SYSLOCALE )
    , m_nBlockedHint( 0 )
    , m_bDecimalSeparator( sal_True )
    , m_bROLocale(CFG_READONLY_DEFAULT)
    , m_bROUILocale(CFG_READONLY_DEFAULT)
    , m_bROCurrency(CFG_READONLY_DEFAULT)
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
                DBG_ASSERT( pValues[nProp].hasValue(), "property value missing" );
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
                                    DBG_ERRORFILE( "Wrong property type!" );
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
                                    DBG_ERRORFILE( "Wrong property type!" );
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
                                    DBG_ERRORFILE( "Wrong property type!" );
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
                                DBG_ERRORFILE( "Wrong property type!" );
                            }
                            m_bRODecimalSeparator = pROStates[nProp];
                        }
                        break;
                        default:
                            DBG_ERRORFILE( "Wrong property type!" );
                    }
                }
            }
        }
//        UpdateMiscSettings_Impl();
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
    if ( m_aRealLocale.Language.getLength() )
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
    if ( !m_aRealUILocale.Language.getLength() )
    {
        // as we can't switch UILocale at runtime, we only store changes in the configuration
        m_aRealUILocale = lcl_str_to_locale( m_aUILocaleString );
        if ( m_aRealUILocale.Language.getLength() )
        {
            m_eRealUILanguage = MsLangId::convertLocaleToLanguage( m_aRealUILocale );
        }
        else
        {
            m_eRealUILanguage = MsLangId::getSystemUILanguage();
            MsLangId::convertLanguageToLocale( m_eRealUILanguage, m_aRealUILocale );
        }
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
            default:
                DBG_ERRORFILE( "invalid index to save a path" );
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
        ULONG nHint = SYSLOCALEOPTIONS_HINT_LOCALE;
        if ( !m_aCurrencyString.getLength() )
            nHint |= SYSLOCALEOPTIONS_HINT_CURRENCY;
        NotifyListeners( nHint );
    }
}

void SvtSysLocaleOptions_Impl::SetUILocaleString( const OUString& rStr )
{
    if (!m_bROUILocale && rStr != m_aUILocaleString )
    {
        m_aUILocaleString = rStr;
/*
        // as we can't switch UILocale at runtime, we only store changes in the configuration
        MakeRealUILocale();
        MsLangId::setConfiguredSystemLanguage( m_eRealUILanguage );
        SetModified();
        NotifyListeners( SYSLOCALEOPTIONS_HINT_UILOCALE );
*/
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
    ULONG nHint = 0;
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
            if ( !m_aCurrencyString.getLength() )
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

const OUString& SvtSysLocaleOptions::GetUILocaleConfigString() const
{
    MutexGuard aGuard( GetMutex() );
    return pOptions->GetUILocaleString();
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
        aStr.append( rAbbrev.GetBuffer(), rAbbrev.Len() );
        aStr.append( sal_Unicode('-') );
        aStr.append( aIsoStr.GetBuffer(), aIsoStr.Len() );
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

com::sun::star::lang::Locale SvtSysLocaleOptions::GetUILocale() const
{
    return lcl_str_to_locale( GetUILocaleConfigString() );
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
