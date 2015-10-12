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


static SvtHelpOptions_Impl* pOptions = NULL;
static sal_Int32           nRefCount = 0;

#define EXTENDEDHELP        0
#define HELPTIPS            1
#define LOCALE              2
#define SYSTEM              3
#define STYLESHEET          4

class SvtHelpOptions_Impl : public utl::ConfigItem
{
    bool            bExtendedHelp;
    bool            bHelpTips;
    bool            bWelcomeScreen;
    OUString        aLocale;
    OUString        aSystem;
    OUString        sHelpStyleSheet;

    typedef std::map <OUString, sal_Int32> MapString2Int;
    MapString2Int   aURLIgnoreCounters;

    static Sequence< OUString > GetPropertyNames();

    virtual void    ImplCommit() override;

public:
                    SvtHelpOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
    void            Load( const ::com::sun::star::uno::Sequence< OUString>& aPropertyNames);

    void            SetExtendedHelp( bool b )           { bExtendedHelp= b; SetModified(); }
    bool            IsExtendedHelp() const                  { return bExtendedHelp; }
    void            SetHelpTips( bool b )               { bHelpTips = b; SetModified(); }
    bool            IsHelpTips() const                      { return bHelpTips; }

    void            SetWelcomeScreen( bool b )          { bWelcomeScreen = b; SetModified(); }
    bool            IsWelcomeScreen() const                 { return bWelcomeScreen; }
    OUString        GetSystem() const                       { return aSystem; }

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
    static ::osl::Mutex *pMutex = 0;

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
    : ConfigItem( OUString( "Office.Common/Help" ) )
    , bExtendedHelp( false )
    , bHelpTips( true )
    , bWelcomeScreen( false )
{
    Sequence< OUString > aNames = GetPropertyNames();
    Load( aNames );
    EnableNotification( aNames );
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
#if OSL_DEBUG_LEVEL > 1
            DBG_ASSERT( pValues[nProp].hasValue(), "property value missing" );
#endif
            if ( pValues[nProp].hasValue() )
            {
                bool bTmp;
                OUString aTmpStr;
                sal_Int32 nTmpInt = 0;
                if ( pValues[nProp] >>= bTmp )
                {
                    switch ( lcl_MapPropertyName(rPropertyNames[nProp], aInternalPropertyNames) )
                    {
                        case EXTENDEDHELP :
                            bExtendedHelp = bTmp;
                            break;
                        case HELPTIPS :
                            bHelpTips = bTmp;
                            break;
                        default:
                            SAL_WARN( "svtools.config", "Wrong Member!" );
                            break;
                    }
                }
                else if ( pValues[nProp] >>= aTmpStr )
                {
                    switch ( nProp )
                    {
                        case LOCALE:
                            aLocale = aTmpStr;
                            break;

                        case SYSTEM:
                            aSystem = aTmpStr;
                            break;
                        case STYLESHEET :
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
        switch ( nProp )
        {
            case EXTENDEDHELP :
                pValues[nProp] <<= bExtendedHelp;
                break;

            case HELPTIPS :
                pValues[nProp] <<= bHelpTips;
                break;

            case LOCALE:
                pValues[nProp] <<= OUString(aLocale);
                break;

            case SYSTEM:
                pValues[nProp] <<= OUString(aSystem);
                break;
            case STYLESHEET :
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
    ++nRefCount;
    if ( !pOptions )
    {
        pOptions = new SvtHelpOptions_Impl;

        svtools::ItemHolder2::holdConfigItem(E_HELPOPTIONS);
    }
    pImp = pOptions;
}



SvtHelpOptions::~SvtHelpOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( SvtHelpOptions_Impl::getInitMutex() );
    if ( !--nRefCount )
    {
        if ( pOptions->IsModified() )
            pOptions->Commit();
        DELETEZ( pOptions );
    }
}

void SvtHelpOptions::SetExtendedHelp( bool b )
{
    pImp->SetExtendedHelp( b );
}

bool SvtHelpOptions::IsExtendedHelp() const
{
    return pImp->IsExtendedHelp();
}

void SvtHelpOptions::SetHelpTips( bool b )
{
    pImp->SetHelpTips( b );
}

bool SvtHelpOptions::IsHelpTips() const
{
    return pImp->IsHelpTips();
}



void SvtHelpOptions::SetWelcomeScreen( bool b )
{
    pImp->SetWelcomeScreen( b );
}

bool SvtHelpOptions::IsWelcomeScreen() const
{
    return pImp->IsWelcomeScreen();
}

OUString SvtHelpOptions::GetSystem() const
{
    return pImp->GetSystem();
}

const OUString&   SvtHelpOptions::GetHelpStyleSheet()const
{
    return pImp->GetHelpStyleSheet();
}

void  SvtHelpOptions::SetHelpStyleSheet(const OUString& rStyleSheet)
{
    pImp->SetHelpStyleSheet(rStyleSheet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
