/*************************************************************************
 *
 *  $RCSfile: syslocaleoptions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: er $ $Date: 2001-06-18 09:32:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "syslocaleoptions.hxx"

#ifndef _ZFORLIST_HXX
#include "zforlist.hxx"
#endif

#ifndef _SVT_BROADCAST_HXX
#include <broadcast.hxx>
#endif
#ifndef _SVT_LISTENER_HXX
#include <listener.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <smplhint.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
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

using namespace osl;
using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


SvtSysLocaleOptions_Impl*   SvtSysLocaleOptions::pOptions = NULL;
sal_Int32                   SvtSysLocaleOptions::nRefCount = 0;
Link                        SvtSysLocaleOptions::aCurrencyChangeLink;


class SvtSysLocaleOptions_Impl : public utl::ConfigItem
{
        OUString                m_aLocaleString;    // en-US or de-DE
        OUString                m_aCurrencyString;  // USD-en-US or EUR-de-DE
        SvtBroadcaster          m_aBroadcaster;

    static  const Sequence< /* const */ OUString >  GetPropertyNames();

public:
                                SvtSysLocaleOptions_Impl();
    virtual                     ~SvtSysLocaleOptions_Impl();

    virtual void                Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void                Commit();

            const OUString&     GetLocaleString() const
                                    { return m_aLocaleString; }
            void                SetLocaleString( const OUString& rStr );

            const OUString&     GetCurrencyString() const
                                    { return m_aCurrencyString; }
            void                SetCurrencyString( const OUString& rStr );
            void                ChangeDefaultCurrency() const;

            SvtBroadcaster&     GetBroadcaster()
                                    { return m_aBroadcaster; }

};


#define ROOTNODE_SYSLOCALE  OUString(RTL_CONSTASCII_USTRINGPARAM("Setup/Product"))

#define PROPERTYNAME_LOCALE         OUString(RTL_CONSTASCII_USTRINGPARAM("System_Locale"))
#define PROPERTYNAME_CURRENCY       OUString(RTL_CONSTASCII_USTRINGPARAM("Currency"))

#define PROPERTYHANDLE_LOCALE       0
#define PROPERTYHANDLE_CURRENCY     1

#define PROPERTYCOUNT               2

const Sequence< OUString > SvtSysLocaleOptions_Impl::GetPropertyNames()
{
    static const OUString pProperties[] =
    {
        PROPERTYNAME_LOCALE,
        PROPERTYNAME_CURRENCY
    };
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    return seqPropertyNames;
}


// -----------------------------------------------------------------------

SvtSysLocaleOptions_Impl::SvtSysLocaleOptions_Impl()
    : ConfigItem( ROOTNODE_SYSLOCALE )
{
    const Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
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
                            DBG_ERRORFILE( "Wrong property type!" );
                    }
                    break;
                    case PROPERTYHANDLE_CURRENCY :
                    {
                        OUString aStr;
                        if ( pValues[nProp] >>= aStr )
                            m_aCurrencyString = aStr;
                        else
                            DBG_ERRORFILE( "Wrong property type!" );
                    }
                    break;
                    default:
                        DBG_ERRORFILE( "Wrong property type!" );
                }
            }
        }
    }
    EnableNotification( aNames );
}


SvtSysLocaleOptions_Impl::~SvtSysLocaleOptions_Impl()
{
    if ( IsModified() )
        Commit();
}


void SvtSysLocaleOptions_Impl::Commit()
{
    const Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues( aNames.getLength() );
    Any* pValues = aValues.getArray();
    for ( sal_Int32 nProp = 0; nProp < aNames.getLength(); nProp++ )
    {
        switch ( nProp )
        {
            case PROPERTYHANDLE_LOCALE :
                pValues[nProp] <<= m_aLocaleString;
            break;
            case PROPERTYHANDLE_CURRENCY :
                pValues[nProp] <<= m_aCurrencyString;
            break;
            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }
    }
    PutProperties( aNames, aValues );
    ClearModified();
}


void SvtSysLocaleOptions_Impl::SetLocaleString( const OUString& rStr )
{
    if ( rStr != m_aLocaleString )
    {
        m_aLocaleString = rStr;
        SetModified();
        SfxSimpleHint aHint( SYSLOCALEOPTIONS_HINT_LOCALE );
        GetBroadcaster().Broadcast( aHint );
    }
}


void SvtSysLocaleOptions_Impl::SetCurrencyString( const OUString& rStr )
{
    if ( rStr != m_aCurrencyString )
    {
        m_aCurrencyString = rStr;
        SetModified();
        ChangeDefaultCurrency();
        SfxSimpleHint aHint( SYSLOCALEOPTIONS_HINT_CURRENCY );
        GetBroadcaster().Broadcast( aHint );
    }
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
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProp = 0; nProp < nCount; ++nProp )
    {
        if( seqPropertyNames[nProp] == PROPERTYNAME_LOCALE )
        {
            DBG_ASSERT( seqValues[nProp].getValueTypeClass() == TypeClass_STRING, "Locale property type" );
            seqValues[nProp] >>= m_aLocaleString;
            nHint |= SYSLOCALEOPTIONS_HINT_LOCALE;
        }
        else if( seqPropertyNames[nProp] == PROPERTYNAME_CURRENCY )
        {
            DBG_ASSERT( seqValues[nProp].getValueTypeClass() == TypeClass_STRING, "Currency property type" );
            seqValues[nProp] >>= m_aCurrencyString;
            nHint |= SYSLOCALEOPTIONS_HINT_CURRENCY;
            ChangeDefaultCurrency();
        }
    }
    if ( nHint )
    {
        SfxSimpleHint aHint( nHint );
        GetBroadcaster().Broadcast( aHint );
    }
}


// ====================================================================

SvtSysLocaleOptions::SvtSysLocaleOptions()
{
    MutexGuard aGuard( GetMutex() );
    if ( !pOptions )
        pOptions = new SvtSysLocaleOptions_Impl;
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
            static Mutex aMutex;
            pMutex = &aMutex;
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


// static
void SvtSysLocaleOptions::GetCurrencyAbbrevAndLanguage( String& rAbbrev,
        LanguageType& eLang, const ::rtl::OUString& rConfigString )
{
    sal_Int32 nDelim = rConfigString.indexOf( '-' );
    if ( nDelim >= 0 )
    {
        rAbbrev = rConfigString.copy( 0, nDelim );
        String aIsoStr( rConfigString.copy( nDelim+1 ) );
        eLang = ConvertIsoStringToLanguage( aIsoStr );
    }
    else
    {
        rAbbrev = rConfigString;
        eLang = LANGUAGE_NONE;
    }
}


// static
::rtl::OUString SvtSysLocaleOptions::CreateCurrencyConfigString(
        const String& rAbbrev, LanguageType eLang )
{
    String aIsoStr( ConvertLanguageToIsoString( eLang ) );
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
    DBG_ASSERT( !aCurrencyChangeLink.IsSet(), "SvtSysLocaleOptions::SetCurrencyChangeLink: already set" );
    aCurrencyChangeLink = rLink;
}


// static
const Link& SvtSysLocaleOptions::GetCurrencyChangeLink()
{
    MutexGuard aGuard( GetMutex() );
    return aCurrencyChangeLink;
}

