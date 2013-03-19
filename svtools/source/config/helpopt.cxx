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


#include <svtools/helpopt.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/help.hxx>
#include <osl/mutex.hxx>
#include <comphelper/stl_types.hxx>

#include <rtl/logfile.hxx>
#include "itemholder2.hxx"

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star;

using ::rtl::OUString;

static SvtHelpOptions_Impl* pOptions = NULL;
static sal_Int32           nRefCount = 0;

#define EXTENDEDHELP        0
#define HELPTIPS            1
#define AGENT_ENABLED       2
#define AGENT_TIMEOUT       3
#define AGENT_RETRYLIMIT    4
#define LOCALE              5
#define SYSTEM              6
#define STYLESHEET          7

class SvtHelpOptions_Impl : public utl::ConfigItem
{
    sal_Int32       nHelpAgentTimeoutPeriod;
    sal_Int32       nHelpAgentRetryLimit;
    sal_Bool        bExtendedHelp;
    sal_Bool        bHelpTips;
    sal_Bool        bHelpAgentEnabled;
    sal_Bool        bWelcomeScreen;
    String          aLocale;
    String          aSystem;
    String          sHelpStyleSheet;

    DECLARE_STL_USTRINGACCESS_MAP( sal_Int32, MapString2Int );
    MapString2Int   aURLIgnoreCounters;
    ::osl::Mutex    aIgnoreCounterSafety;

    Sequence< OUString > GetPropertyNames();

public:
                    SvtHelpOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    void            Load( const ::com::sun::star::uno::Sequence< ::rtl::OUString>& aPropertyNames);
    virtual void    Commit();

    void            SetExtendedHelp( sal_Bool b )           { bExtendedHelp= b; SetModified(); }
    sal_Bool        IsExtendedHelp() const                  { return bExtendedHelp; }
    void            SetHelpTips( sal_Bool b )               { bHelpTips = b; SetModified(); }
    sal_Bool        IsHelpTips() const                      { return bHelpTips; }

    void            SetHelpAgentEnabled( sal_Bool b )       { bHelpAgentEnabled = b; SetModified(); }
    sal_Bool        IsHelpAgentEnabled() const              { return bHelpAgentEnabled; }
    void            SetHelpAgentTimeoutPeriod( sal_Int32 _nSeconds )    { nHelpAgentTimeoutPeriod = _nSeconds; SetModified(); }
    sal_Int32       GetHelpAgentTimeoutPeriod( ) const      { return nHelpAgentTimeoutPeriod; }
    void            SetHelpAgentRetryLimit( sal_Int32 _nTrials )        { nHelpAgentRetryLimit = _nTrials; SetModified(); }
    sal_Int32       GetHelpAgentRetryLimit( ) const         { return nHelpAgentRetryLimit; }

    void            resetAgentIgnoreURLCounter();

    void            SetWelcomeScreen( sal_Bool b )          { bWelcomeScreen = b; SetModified(); }
    sal_Bool        IsWelcomeScreen() const                 { return bWelcomeScreen; }
    String          GetLocale() const                       { return aLocale; }
    String          GetSystem() const                       { return aSystem; }

    const String&   GetHelpStyleSheet()const{return sHelpStyleSheet;}
    void            SetHelpStyleSheet(const String& rStyleSheet){sHelpStyleSheet = rStyleSheet; SetModified();}

    static ::osl::Mutex & getInitMutex();

protected:
    void    implLoadURLCounters();
    void    implSaveURLCounters();
    // to be called with aIgnoreCounterSafety locked
    void    implGetURLCounters( Sequence< ::rtl::OUString >& _rNodeNames, Sequence< Any >& _rURLs, Sequence< Any >& _rCounter );
};

Sequence< OUString > SvtHelpOptions_Impl::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "ExtendedTip",
        "Tip",
        "HelpAgent/Enabled",
        "HelpAgent/Timeout",
        "HelpAgent/RetryLimit",
        "Locale",
        "System",
        "HelpStyleSheet",
//      "HowTo/Show"
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


// -----------------------------------------------------------------------

SvtHelpOptions_Impl::SvtHelpOptions_Impl()
    : ConfigItem( OUString( "Office.Common/Help" ) )
    , bExtendedHelp( sal_False )
    , bHelpTips( sal_True )
    , bHelpAgentEnabled( sal_False )
    , bWelcomeScreen( sal_False )
{
    Sequence< OUString > aNames = GetPropertyNames();
    Load( aNames );
    EnableNotification( aNames );
    implLoadURLCounters();
}

// -----------------------------------------------------------------------
static int lcl_MapPropertyName( const ::rtl::OUString rCompare,
                const uno::Sequence< ::rtl::OUString>& aInternalPropertyNames)
{
    for(int nProp = 0; nProp < aInternalPropertyNames.getLength(); ++nProp)
    {
        if( aInternalPropertyNames[nProp] == rCompare )
            return nProp;
    }
    return -1;
}

void  SvtHelpOptions_Impl::Load(const uno::Sequence< ::rtl::OUString>& rPropertyNames)
{
    const uno::Sequence< ::rtl::OUString> aInternalPropertyNames( GetPropertyNames());
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
                sal_Bool bTmp = sal_Bool();
                ::rtl::OUString aTmpStr;
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
                        case AGENT_ENABLED :
                            bHelpAgentEnabled = bTmp;
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
                    switch ( nProp )
                    {
                        case AGENT_TIMEOUT:
                            nHelpAgentTimeoutPeriod = nTmpInt;
                            break;

                        case AGENT_RETRYLIMIT:
                            nHelpAgentRetryLimit = nTmpInt;
                            break;

                        default:
                            SAL_WARN( "svtools.config", "Wrong Member!" );
                            break;
                    }
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

// -----------------------------------------------------------------------

void SvtHelpOptions_Impl::implGetURLCounters( Sequence< ::rtl::OUString >& _rNodeNames, Sequence< Any >& _rURLs, Sequence< Any >& _rCounters )
{
    // the ignore counters for the help agent URLs
    const ::rtl::OUString sIgnoreListNodePath( "HelpAgent/IgnoreList" );
    const ::rtl::OUString sPathSeparator( "/" );
    const ::rtl::OUString sURLLocalPath( "/Name" );
    const ::rtl::OUString sCounterLocalPath( "/Counter" );

    // get the names of all the nodes containing ignore counters
    // collect the node names we have to ask
    // first get the node names of all children of HelpAgent/IgnoreList
    _rNodeNames = GetNodeNames(sIgnoreListNodePath);
    const ::rtl::OUString* pIgnoredURLsNodes = _rNodeNames.getConstArray();
    const ::rtl::OUString* pIgnoredURLsNodesEnd = pIgnoredURLsNodes + _rNodeNames.getLength();

    // then assemble the two lists (of node paths) for the URLs and the counters
    Sequence< ::rtl::OUString > aIgnoredURLs(_rNodeNames.getLength());
    Sequence< ::rtl::OUString > aIgnoredURLsCounter(_rNodeNames.getLength());
    ::rtl::OUString* pIgnoredURLs = aIgnoredURLs.getArray();
    ::rtl::OUString* pIgnoredURLsCounter = aIgnoredURLsCounter.getArray();
    for (;pIgnoredURLsNodes != pIgnoredURLsNodesEnd; ++pIgnoredURLsNodes, ++pIgnoredURLs, ++pIgnoredURLsCounter)
    {
        ::rtl::OUString sLocalURLAccess = sIgnoreListNodePath;
        sLocalURLAccess += sPathSeparator;
        sLocalURLAccess += *pIgnoredURLsNodes;

        // the path to the URL of this specific entry
        *pIgnoredURLs = sLocalURLAccess;
        *pIgnoredURLs += sURLLocalPath;

        // the path of the counter for that URL
        *pIgnoredURLsCounter = sLocalURLAccess;
        *pIgnoredURLsCounter += sCounterLocalPath;
    }

    // now collect the values
    _rURLs = GetProperties(aIgnoredURLs);
    _rCounters = GetProperties(aIgnoredURLsCounter);

    sal_Int32 nURLs = _rURLs.getLength();
    sal_Int32 nCounters = _rCounters.getLength();
    DBG_ASSERT(nURLs == nCounters, "SvtHelpOptions_Impl::implGetURLCounters: inconsistence while retrieving the visited URLs!");

    // normalize in case something went wrong
    sal_Int32 nKnownURLs = nURLs < nCounters ? nURLs : nCounters;
    if (nURLs < nCounters)
    {
        _rCounters.realloc(nKnownURLs);
        _rNodeNames.realloc(nKnownURLs);
    }
    else if (nURLs > nCounters)
    {
        _rURLs.realloc(nKnownURLs);
        _rNodeNames.realloc(nKnownURLs);
    }
}

// -----------------------------------------------------------------------

void SvtHelpOptions_Impl::implSaveURLCounters()
{
    ::osl::MutexGuard aGuard(aIgnoreCounterSafety);

    const ::rtl::OUString sIgnoreListNodePath( "HelpAgent/IgnoreList" );
    const ::rtl::OUString sPathSeparator( "/" );
    const ::rtl::OUString sURLLocalPath( "/Name" );
    const ::rtl::OUString sCounterLocalPath( "/Counter" );

    // get the current URL/counter pairs (as they're persistent at the moment)
    Sequence< ::rtl::OUString > aNodeNames;
    Sequence< Any >             aURLs;
    Sequence< Any >             aCounters;

    implGetURLCounters(aNodeNames, aURLs, aCounters);
    sal_Int32 nKnownURLs = aURLs.getLength();

    const ::rtl::OUString* pNodeNames   = aNodeNames.getConstArray();
    const Any* pURLs                    = aURLs.getConstArray();
    const Any* pCounters                = aCounters.getConstArray();

    // check which of them must be deleted/modified
    Sequence< ::rtl::OUString >     aDeleteFromConfig(nKnownURLs);  // names of nodes to be deleted
    ::rtl::OUString*                pDeleteFromConfig = aDeleteFromConfig.getArray();
    ::std::set< ::rtl::OUString >   aAlreadyPresent;    // URLs currently persistent

    // for modifying already existent nodes
    Sequence< ::rtl::OUString > aNewCounterNodePaths(nKnownURLs);
    Sequence< Any >             aNewCounterValues(nKnownURLs);
    ::rtl::OUString*            pNewCounterNodePaths = aNewCounterNodePaths.getArray();
    Any*                        pNewCounterValues = aNewCounterValues.getArray();

    // temporaries needed inside the loop
    ::rtl::OUString sCurrentURL, sCurrentURLNodeName;

    for (sal_Int32 i=0; i<nKnownURLs; ++i, ++pNodeNames, ++pURLs, ++pCounters)
    {
        if (!((*pURLs) >>= sCurrentURL))
            continue;

        ConstMapString2IntIterator aThisURLNewCounter = aURLIgnoreCounters.find(sCurrentURL);
        if (aURLIgnoreCounters.end() == aThisURLNewCounter)
        {   // we do not know anything about this URL anymore.
            // -> have to removed it from the configuration later on
            *pDeleteFromConfig = *pNodeNames;
            ++pDeleteFromConfig;
        }
        else
        {   // we know this URL
            sCurrentURLNodeName = sIgnoreListNodePath;
            sCurrentURLNodeName += sPathSeparator;
            sCurrentURLNodeName += *pNodeNames;

            // -> remember this (so we don't need to add a new node for this URL later on)
            aAlreadyPresent.insert(sCurrentURL);

            sal_Int32 nThisURLPersistentCounter = 0;
            (*pCounters) >>= nThisURLPersistentCounter;

            if (aThisURLNewCounter->second != nThisURLPersistentCounter)
            {   // the counter changed
                // -> remember the path and the new counter for the adjustment below
                *pNewCounterNodePaths = sCurrentURLNodeName;
                *pNewCounterNodePaths += sCounterLocalPath;
                ++pNewCounterNodePaths;

                (*pNewCounterValues) <<= aThisURLNewCounter->second;
                ++pNewCounterValues;
            }
        }
    }

    // delete the nodes which are flagged so ...
    aDeleteFromConfig.realloc(pDeleteFromConfig - aDeleteFromConfig.getArray());
    if (0 != aDeleteFromConfig.getLength())
    {
        ClearNodeElements(sIgnoreListNodePath, aDeleteFromConfig);
    }

    // modify the nodes which need to be
    aNewCounterNodePaths.realloc(pNewCounterNodePaths - aNewCounterNodePaths.getArray());
    aNewCounterValues.realloc(pNewCounterValues - aNewCounterValues.getArray());
    if (0 != aNewCounterNodePaths.getLength())
    {
        PutProperties(aNewCounterNodePaths, aNewCounterValues);
    }

    // and for the new ones ...
    ::rtl::OUString sNewNodeName;
    Sequence< ::rtl::OUString > aNewCounterDataNodeNames(2);
    Sequence< Any >             aNewCounterDataValues(2);
    const ::rtl::OUString sNodeNameBase( "URL" );
    for (   ConstMapString2IntIterator aCollectNew = aURLIgnoreCounters.begin();
            aCollectNew != aURLIgnoreCounters.end();
            ++aCollectNew
        )
    {
        if (aAlreadyPresent.end() == aAlreadyPresent.find(aCollectNew->first))
        {   // this URL is not persistent, yet
            // -> add a new node
            sNewNodeName = sNodeNameBase;
            if (!getUniqueSetElementName(sIgnoreListNodePath, sNewNodeName))
            {
                SAL_WARN( "svtools.config", "SvtHelpOptions_Impl::implSaveURLCounters: could not get a free name!" );
                continue;
            }
            AddNode(sIgnoreListNodePath, sNewNodeName);

            // and set the URL/counter pair
            aNewCounterDataNodeNames[0] = sIgnoreListNodePath;
            aNewCounterDataNodeNames[0] += sPathSeparator;
            aNewCounterDataNodeNames[0] += sNewNodeName;
            aNewCounterDataNodeNames[0] += sURLLocalPath;
            aNewCounterDataValues[0]    <<= aCollectNew->first;

            aNewCounterDataNodeNames[1] = sIgnoreListNodePath;
            aNewCounterDataNodeNames[1] += sPathSeparator;
            aNewCounterDataNodeNames[1] += sNewNodeName;
            aNewCounterDataNodeNames[1] += sCounterLocalPath;
            aNewCounterDataValues[1]    <<= aCollectNew->second;

            PutProperties(aNewCounterDataNodeNames, aNewCounterDataValues);
        }
    }
}

// -----------------------------------------------------------------------

void SvtHelpOptions_Impl::implLoadURLCounters()
{
    ::osl::MutexGuard aGuard(aIgnoreCounterSafety);

    Sequence< ::rtl::OUString > aNodeNames;
    Sequence< Any >             aURLs;
    Sequence< Any >             aCounters;

    implGetURLCounters(aNodeNames, aURLs, aCounters);
    sal_Int32 nKnownURLs = aURLs.getLength();

    const Any* pURLs = aURLs.getConstArray();
    const Any* pCounters = aCounters.getConstArray();

    ::rtl::OUString sCurrentURL;
    sal_Int32 nCurrentCounter;
    for (sal_Int32 i=0; i<nKnownURLs; ++i, ++pURLs, ++pCounters)
    {
        (*pURLs) >>= sCurrentURL;
        nCurrentCounter = 0;
        (*pCounters) >>= nCurrentCounter;
        aURLIgnoreCounters[sCurrentURL] = nCurrentCounter;
    }
}

// -----------------------------------------------------------------------

void SvtHelpOptions_Impl::Commit()
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

            case AGENT_ENABLED :
                pValues[nProp] <<= bHelpAgentEnabled;
                break;

            case AGENT_TIMEOUT:
                pValues[nProp] <<= nHelpAgentTimeoutPeriod;
                break;

            case AGENT_RETRYLIMIT:
                pValues[nProp] <<= nHelpAgentRetryLimit;
                break;

            case LOCALE:
                pValues[nProp] <<= ::rtl::OUString(aLocale);
                break;

            case SYSTEM:
                pValues[nProp] <<= ::rtl::OUString(aSystem);
                break;
            case STYLESHEET :
                pValues[nProp] <<= ::rtl::OUString(sHelpStyleSheet);
            break;

        }
    }

    PutProperties( aNames, aValues );

    implSaveURLCounters();
}

// -----------------------------------------------------------------------

void SvtHelpOptions_Impl::Notify( const Sequence<rtl::OUString>& aPropertyNames )
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
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtHelpOptions_Impl::ctor()");
        pOptions = new SvtHelpOptions_Impl;

        svtools::ItemHolder2::holdConfigItem(E_HELPOPTIONS);
    }
    pImp = pOptions;
}

// -----------------------------------------------------------------------

void SvtHelpOptions_Impl::resetAgentIgnoreURLCounter()
{
    ::osl::MutexGuard aGuard(aIgnoreCounterSafety);
    aURLIgnoreCounters.clear();
    SetModified();
}

// -----------------------------------------------------------------------

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

void SvtHelpOptions::SetExtendedHelp( sal_Bool b )
{
    pImp->SetExtendedHelp( b );
}

sal_Bool SvtHelpOptions::IsExtendedHelp() const
{
    return pImp->IsExtendedHelp();
}

void SvtHelpOptions::SetHelpTips( sal_Bool b )
{
    pImp->SetHelpTips( b );
}

sal_Bool SvtHelpOptions::IsHelpTips() const
{
    return pImp->IsHelpTips();
}

// -----------------------------------------------------------------------

void SvtHelpOptions::SetHelpAgentTimeoutPeriod( sal_Int32 _nSeconds )
{
    pImp->SetHelpAgentTimeoutPeriod( _nSeconds );
}

// -----------------------------------------------------------------------

sal_Int32 SvtHelpOptions::GetHelpAgentTimeoutPeriod( ) const
{
    return pImp->GetHelpAgentTimeoutPeriod( );
}

// -----------------------------------------------------------------------

void SvtHelpOptions::SetHelpAgentAutoStartMode( sal_Bool b )
{
    pImp->SetHelpAgentEnabled( b );
}

// -----------------------------------------------------------------------

sal_Bool SvtHelpOptions::IsHelpAgentAutoStartMode() const
{
    return pImp->IsHelpAgentEnabled();
}

// -----------------------------------------------------------------------

void SvtHelpOptions::resetAgentIgnoreURLCounter()
{
    pImp->resetAgentIgnoreURLCounter();
}

// -----------------------------------------------------------------------

void SvtHelpOptions::SetWelcomeScreen( sal_Bool b )
{
    pImp->SetWelcomeScreen( b );
}

sal_Bool SvtHelpOptions::IsWelcomeScreen() const
{
    return pImp->IsWelcomeScreen();
}

String SvtHelpOptions::GetSystem() const
{
    return pImp->GetSystem();
}

const String&   SvtHelpOptions::GetHelpStyleSheet()const
{
    return pImp->GetHelpStyleSheet();
}

void  SvtHelpOptions::SetHelpStyleSheet(const String& rStyleSheet)
{
    pImp->SetHelpStyleSheet(rStyleSheet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
