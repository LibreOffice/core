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

#include <bf_svtools/syslocaleoptions.hxx>

#ifndef _SVT_BROADCAST_HXX
#include <broadcast.hxx>
#endif
#ifndef _SVT_LISTENER_HXX
#include <listener.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <bf_svtools/smplhint.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <rtl/logfile.hxx>

#include "itemholder2.hxx" 

namespace binfilter
{

#define CFG_READONLY_DEFAULT    sal_False

using namespace osl;
using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


SvtSysLocaleOptions_Impl*   SvtSysLocaleOptions::pOptions = NULL;
sal_Int32                   SvtSysLocaleOptions::nRefCount = 0;
namespace 
{ 
    struct CurrencyChangeLink 
        : public rtl::Static<Link, CurrencyChangeLink> {}; 
}

class SvtSysLocaleOptions_Impl : public utl::ConfigItem
{
        OUString                m_aLocaleString;    // en-US or de-DE or empty for SYSTEM
        LanguageType            m_eLocaleLanguageType;  // same for convenience access
        OUString                m_aCurrencyString;  // USD-en-US or EUR-de-DE
        SvtBroadcaster          m_aBroadcaster;
        ULONG                   m_nBlockedHint;     // pending hints
        sal_Int32               m_nBroadcastBlocked;     // broadcast only if this is 0
        sal_Bool                m_bDecimalSeparator; //use decimal separator same as locale


        sal_Bool                m_bROLocale;
        sal_Bool                m_bROCurrency;
        sal_Bool                m_bRODecimalSeparator;

    static  const Sequence< /* const */ OUString >  GetPropertyNames();

            void                UpdateMiscSettings_Impl();
            ULONG               ChangeLocaleSettings();
            void                ChangeDefaultCurrency() const;
            void                Broadcast( ULONG nHint );

public:
                                SvtSysLocaleOptions_Impl();
    virtual                     ~SvtSysLocaleOptions_Impl();

    virtual void                Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void                Commit();

            const OUString&     GetLocaleString() const
                                    { return m_aLocaleString; }
            LanguageType        GetLocaleLanguageType() const
                                    { return m_eLocaleLanguageType; }

            const OUString&     GetCurrencyString() const
                                    { return m_aCurrencyString; }

            sal_Bool            IsDecimalSeparatorAsLocale() const { return m_bDecimalSeparator;}

            SvtBroadcaster&     GetBroadcaster()
                                    { return m_aBroadcaster; }
};


#define ROOTNODE_SYSLOCALE  OUString(RTL_CONSTASCII_USTRINGPARAM("Setup/L10N"))

#define PROPERTYNAME_LOCALE         OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupSystemLocale"))
#define PROPERTYNAME_CURRENCY       OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupCurrency"))
#define PROPERTYNAME_DECIMALSEPARATOR OUString(RTL_CONSTASCII_USTRINGPARAM("DecimalSeparatorAsLocale"))

#define PROPERTYHANDLE_LOCALE       0
#define PROPERTYHANDLE_CURRENCY     1
#define PROPERTYHANDLE_DECIMALSEPARATOR 2

#define PROPERTYCOUNT               3

const Sequence< OUString > SvtSysLocaleOptions_Impl::GetPropertyNames()
{
    static const OUString pProperties[] =
    {
        PROPERTYNAME_LOCALE,
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
    , m_nBroadcastBlocked( 0 )
    , m_bDecimalSeparator( sal_True )
    , m_bROLocale(CFG_READONLY_DEFAULT)
    , m_bROCurrency(CFG_READONLY_DEFAULT)
    , m_bRODecimalSeparator(sal_False)

{
    if ( !IsValidConfigMgr() )
        ChangeLocaleSettings();     // assume SYSTEM defaults during Setup
    else
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
        UpdateMiscSettings_Impl();
        ChangeLocaleSettings();
        EnableNotification( aNames );
    }
}


SvtSysLocaleOptions_Impl::~SvtSysLocaleOptions_Impl()
{
    if ( IsModified() )
        Commit();
}


void SvtSysLocaleOptions_Impl::Broadcast( ULONG nHint )
{
    if ( m_nBroadcastBlocked )
        m_nBlockedHint |= nHint;
    else
    {
        nHint |= m_nBlockedHint;
        m_nBlockedHint = 0;
        if ( nHint )
        {
            if ( nHint & SYSLOCALEOPTIONS_HINT_CURRENCY )
                ChangeDefaultCurrency();
            SfxSimpleHint aHint( nHint );
            GetBroadcaster().Broadcast( aHint );
        }
    }
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
            case PROPERTYHANDLE_CURRENCY :
                {
                    if (!m_bROLocale)
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


ULONG SvtSysLocaleOptions_Impl::ChangeLocaleSettings()
{
    // An empty config value denotes SYSTEM locale
    if ( m_aLocaleString.getLength() )
        m_eLocaleLanguageType = MsLangId::convertIsoStringToLanguage( m_aLocaleString );
    else
        m_eLocaleLanguageType = LANGUAGE_SYSTEM;
    ULONG nHint = 0;
    // new locale and no fixed currency => locale default currency might change
    if ( !m_aCurrencyString.getLength() )
        nHint |= SYSLOCALEOPTIONS_HINT_CURRENCY;
    return nHint;
}


void SvtSysLocaleOptions_Impl::ChangeDefaultCurrency() const
{
    const Link& rLink = SvtSysLocaleOptions::GetCurrencyChangeLink();
    if ( rLink.IsSet() )
        rLink.Call( NULL );
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
            nHint |= ChangeLocaleSettings();
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
            UpdateMiscSettings_Impl();
        }        
    }
    if ( nHint )
        Broadcast( nHint );
}
/* -----------------10.02.2004 15:25-----------------

 --------------------------------------------------*/
void SvtSysLocaleOptions_Impl::UpdateMiscSettings_Impl()
{        
    AllSettings aAllSettings( Application::GetSettings() );
    MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
    aMiscSettings.SetEnableLocalizedDecimalSep(m_bDecimalSeparator);
    aAllSettings.SetMiscSettings( aMiscSettings );
    Application::SetSettings( aAllSettings );
}

// ====================================================================

SvtSysLocaleOptions::SvtSysLocaleOptions()
{
    MutexGuard aGuard( GetMutex() );
    if ( !pOptions )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtSysLocaleOptions_Impl::ctor()");
        pOptions = new SvtSysLocaleOptions_Impl;
         
        ItemHolder2::holdConfigItem(E_SYSLOCALEOPTIONS);
    }
    ++nRefCount;
}


SvtSysLocaleOptions::~SvtSysLocaleOptions()
{
    MutexGuard aGuard( GetMutex() );
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
            // we need a mutex that lives longer than the svtools library.
            // Otherwise the dtor would use a destructed mutex!!
            pMutex = new Mutex;
        }
    }
    return *pMutex;
}

BOOL SvtSysLocaleOptions::AddListener( SvtListener& rLst )
{
    MutexGuard aGuard( GetMutex() );
    return rLst.StartListening( pOptions->GetBroadcaster() );
}


BOOL SvtSysLocaleOptions::RemoveListener( SvtListener& rLst )
{
    MutexGuard aGuard( GetMutex() );
    return rLst.EndListening( pOptions->GetBroadcaster() );
}


const OUString& SvtSysLocaleOptions::GetCurrencyConfigString() const
{
    MutexGuard aGuard( GetMutex() );
    return pOptions->GetCurrencyString();
}

/*-- 11.02.2004 13:31:41---------------------------------------------------

  -----------------------------------------------------------------------*/

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

}
