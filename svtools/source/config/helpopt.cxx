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

#include <sal/config.h>

#include <map>

#include <svtools/helpopt.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/help.hxx>
#include <osl/mutex.hxx>

#include "itemholder2.hxx"

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star;

namespace {
    //global
    std::weak_ptr<SvtHelpOptions_Impl> g_pHelpOptions;
}

enum class HelpProperty
{
    ExtendedHelp    = 0,
    HelpTips        = 1,
    Locale          = 2,
    System          = 3,
    StyleSheet      = 4
};

class SvtHelpOptions_Impl : public utl::ConfigItem
{
    bool            bExtendedHelp;
    bool            bHelpTips;
    bool            bWelcomeScreen;
    OUString        aLocale;
    OUString        aSystem;
    OUString        sHelpStyleSheet;

    static Sequence< OUString > GetPropertyNames();

    virtual void    ImplCommit() final override;

public:
                    SvtHelpOptions_Impl();
                    ~SvtHelpOptions_Impl() override;

    virtual void    Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    void            Load( const css::uno::Sequence< OUString>& aPropertyNames);

    void            SetExtendedHelp( bool b )           { bExtendedHelp= b; SetModified(); }
    bool            IsExtendedHelp() const                  { return bExtendedHelp; }
    void            SetHelpTips( bool b )               { bHelpTips = b; SetModified(); }
    bool            IsHelpTips() const                      { return bHelpTips; }

    void            SetWelcomeScreen( bool b )          { bWelcomeScreen = b; SetModified(); }
    bool            IsWelcomeScreen() const                 { return bWelcomeScreen; }
    const OUString& GetSystem() const                       { return aSystem; }

    const OUString& GetHelpStyleSheet()const{return sHelpStyleSheet;}
    void            SetHelpStyleSheet(const OUString& rStyleSheet){sHelpStyleSheet = rStyleSheet; SetModified();}

    static ::osl::Mutex & getInitMutex();
};

Sequence< OUString > SvtHelpOptions_Impl::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "ExtendedTip",
        "Tip",
        "Locale",
        "System",
        "HelpStyleSheet"
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

::osl::Mutex & SvtHelpOptions_Impl::getInitMutex()
{
    static ::osl::Mutex *pMutex = nullptr;

    if( ! pMutex )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! pMutex )
        {
            static ::osl::Mutex mutex;
            pMutex = &mutex;
        }
    }
    return *pMutex;
}

SvtHelpOptions_Impl::SvtHelpOptions_Impl()
    : ConfigItem( "Office.Common/Help" )
    , bExtendedHelp( false )
    , bHelpTips( true )
    , bWelcomeScreen( false )
{
    Sequence< OUString > aNames = GetPropertyNames();
    Load( aNames );
    EnableNotification( aNames );
}

SvtHelpOptions_Impl::~SvtHelpOptions_Impl()
{
    if ( IsModified() )
        Commit();
}

static int lcl_MapPropertyName( const OUString& rCompare,
                const uno::Sequence< OUString>& aInternalPropertyNames)
{
    for(int nProp = 0; nProp < aInternalPropertyNames.getLength(); ++nProp)
    {
        if( aInternalPropertyNames[nProp] == rCompare )
            return nProp;
    }
    return -1;
}

void  SvtHelpOptions_Impl::Load(const uno::Sequence< OUString>& rPropertyNames)
{
    const uno::Sequence< OUString> aInternalPropertyNames( GetPropertyNames());
    Sequence< Any > aValues = GetProperties( rPropertyNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == rPropertyNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == rPropertyNames.getLength() )
    {
        for ( int nProp = 0; nProp < rPropertyNames.getLength(); nProp++ )
        {
            assert(pValues[nProp].hasValue() && "property value missing");
            if ( pValues[nProp].hasValue() )
            {
                bool bTmp;
                OUString aTmpStr;
                sal_Int32 nTmpInt = 0;
                if ( pValues[nProp] >>= bTmp )
                {
                    switch ( static_cast< HelpProperty >(
                        lcl_MapPropertyName(rPropertyNames[nProp], aInternalPropertyNames) ) )
                    {
                        case HelpProperty::ExtendedHelp:
                            bExtendedHelp = bTmp;
                            break;
                        case HelpProperty::HelpTips:
                            bHelpTips = bTmp;
                            break;
                        default:
                            SAL_WARN( "svtools.config", "Wrong Member!" );
                            break;
                    }
                }
                else if ( pValues[nProp] >>= aTmpStr )
                {
                    switch ( static_cast< HelpProperty >(nProp) )
                    {
                        case HelpProperty::Locale:
                            aLocale = aTmpStr;
                            break;

                        case HelpProperty::System:
                            aSystem = aTmpStr;
                            break;
                        case HelpProperty::StyleSheet:
                            sHelpStyleSheet = aTmpStr;
                        break;
                        default:
                            SAL_WARN( "svtools.config", "Wrong Member!" );
                            break;
                    }
                }
                else if ( pValues[nProp] >>= nTmpInt )
                {
                    SAL_WARN( "svtools.config", "Wrong Member!" );
                }
                else
                {
                    SAL_WARN( "svtools.config", "Wrong Type!" );
                }
            }
        }
        if ( IsHelpTips() != Help::IsQuickHelpEnabled() )
            IsHelpTips() ? Help::EnableQuickHelp() : Help::DisableQuickHelp();
        if ( IsExtendedHelp() != Help::IsBalloonHelpEnabled() )
            IsExtendedHelp() ? Help::EnableBalloonHelp() : Help::DisableBalloonHelp();
    }
}

void SvtHelpOptions_Impl::ImplCommit()
{
    Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues( aNames.getLength() );
    Any* pValues = aValues.getArray();
    for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
    {
        switch ( static_cast< HelpProperty >(nProp) )
        {
            case HelpProperty::ExtendedHelp:
                pValues[nProp] <<= bExtendedHelp;
                break;

            case HelpProperty::HelpTips:
                pValues[nProp] <<= bHelpTips;
                break;

            case HelpProperty::Locale:
                pValues[nProp] <<= OUString(aLocale);
                break;

            case HelpProperty::System:
                pValues[nProp] <<= OUString(aSystem);
                break;
            case HelpProperty::StyleSheet:
                pValues[nProp] <<= OUString(sHelpStyleSheet);
            break;

        }
    }

    PutProperties( aNames, aValues );
}

void SvtHelpOptions_Impl::Notify( const Sequence<OUString>& aPropertyNames )
{
    Load( aPropertyNames );
}

SvtHelpOptions::SvtHelpOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( SvtHelpOptions_Impl::getInitMutex() );

    pImpl = g_pHelpOptions.lock();
    if ( !pImpl )
    {
        pImpl = std::make_shared<SvtHelpOptions_Impl>();
        g_pHelpOptions = pImpl;
        svtools::ItemHolder2::holdConfigItem(EItem::HelpOptions);
    }
}

SvtHelpOptions::~SvtHelpOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( SvtHelpOptions_Impl::getInitMutex() );

    pImpl.reset();
}

void SvtHelpOptions::SetExtendedHelp( bool b )
{
    pImpl->SetExtendedHelp( b );
}

bool SvtHelpOptions::IsExtendedHelp() const
{
    return pImpl->IsExtendedHelp();
}

void SvtHelpOptions::SetHelpTips( bool b )
{
    pImpl->SetHelpTips( b );
}

bool SvtHelpOptions::IsHelpTips() const
{
    return pImpl->IsHelpTips();
}

void SvtHelpOptions::SetWelcomeScreen( bool b )
{
    pImpl->SetWelcomeScreen( b );
}

bool SvtHelpOptions::IsWelcomeScreen() const
{
    return pImpl->IsWelcomeScreen();
}

OUString SvtHelpOptions::GetSystem() const
{
    return pImpl->GetSystem();
}

const OUString&   SvtHelpOptions::GetHelpStyleSheet()const
{
    return pImpl->GetHelpStyleSheet();
}

void  SvtHelpOptions::SetHelpStyleSheet(const OUString& rStyleSheet)
{
    pImpl->SetHelpStyleSheet(rStyleSheet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
