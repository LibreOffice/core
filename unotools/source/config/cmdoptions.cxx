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

#include <sal/log.hxx>
#include <unotools/cmdoptions.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <cppuhelper/weakref.hxx>

#include "itemholder1.hxx"

#include <algorithm>
#include <unordered_map>

using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

constexpr OUStringLiteral ROOTNODE_CMDOPTIONS        = u"Office.Commands/Execute";
#define PATHDELIMITER                                   "/"

#define SETNODE_DISABLED                                "Disabled"

#define PROPERTYNAME_CMD                                "Command"

namespace {

/*-****************************************************************************************************************
    @descr  support simple command option structures and operations on it
****************************************************************************************************************-*/
class SvtCmdOptions
{
    public:

        // the only way to free memory!
        void Clear()
        {
            m_aCommandHashMap.clear();
        }

        bool HasEntries() const
        {
            return ( !m_aCommandHashMap.empty() );
        }

        bool Lookup( const OUString& aCmd ) const
        {
            CommandHashMap::const_iterator pEntry = m_aCommandHashMap.find( aCmd );
            return ( pEntry != m_aCommandHashMap.end() );
        }

        void AddCommand( const OUString& aCmd )
        {
            m_aCommandHashMap.emplace( aCmd, 0 );
        }

    private:
        typedef std::unordered_map<OUString, sal_Int32>
            CommandHashMap;

        CommandHashMap m_aCommandHashMap;
};

std::mutex& GetOwnStaticMutex()
{
    static std::mutex theCommandOptionsMutex;
    return theCommandOptionsMutex;
}

}

typedef ::std::vector< css::uno::WeakReference< css::frame::XFrame > > SvtFrameVector;

class SvtCommandOptions_Impl : public ConfigItem
{
    public:

         SvtCommandOptions_Impl();
        virtual ~SvtCommandOptions_Impl() override;

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      This method is called from the ConfigManager before the application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& lPropertyNames ) override;

        /*-****************************************************************************************************
            @short      base implementation of public interface for "SvtDynamicMenuOptions"!
            @descr      These class is used as static member of "SvtDynamicMenuOptions" ...
                        => The code exist only for one time and isn't duplicated for every instance!
        *//*-*****************************************************************************************************/

        bool                HasEntriesDisabled() const;
        bool                LookupDisabled( const OUString& ) const;
        void EstablishFrameCallback(const css::uno::Reference< css::frame::XFrame >& xFrame);

    private:

        virtual void ImplCommit() override;

        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent our module tree
            @descr      This method returns the current list of key names! We need it to get needed values from our
                        configuration management and support dynamical menu item lists!
            @param      "nDisabledCount", returns count of menu entries for "new"
            @return     A list of configuration key names is returned.
        *//*-*****************************************************************************************************/

        Sequence< OUString > impl_GetPropertyNames();

    private:
        SvtCmdOptions  m_aDisabledCommands;
        SvtFrameVector m_lFrames;
};

//  constructor

SvtCommandOptions_Impl::SvtCommandOptions_Impl()
    // Init baseclasses first
    :   ConfigItem( ROOTNODE_CMDOPTIONS )
    // Init member then...
{
    // Get names and values of all accessible menu entries and fill internal structures.
    // See impl_GetPropertyNames() for further information.
    Sequence< OUString >    lNames              = impl_GetPropertyNames ();
    Sequence< Any >         lValues             = GetProperties         ( lNames         );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtCommandOptions_Impl::SvtCommandOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    // Attention: List for names and values have an internal construction pattern!
    sal_Int32   nItem     = 0;
    OUString    sCmd;

    // Get names/values for disabled commands.
    for( nItem=0; nItem < lNames.getLength(); ++nItem )
    {
        // Currently only one value
        lValues[nItem] >>= sCmd;
        m_aDisabledCommands.AddCommand( sCmd );
    }

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys! */
    Sequence<OUString> aNotifySeq { u"Disabled"_ustr };
    EnableNotification( aNotifySeq, true );
}

//  destructor

SvtCommandOptions_Impl::~SvtCommandOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

//  public method

void SvtCommandOptions_Impl::Notify( const Sequence< OUString >& )
{
    std::unique_lock aGuard( GetOwnStaticMutex() );

    Sequence< OUString >    lNames   = impl_GetPropertyNames ();
    Sequence< Any >         lValues  = GetProperties         ( lNames         );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtCommandOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    // Attention: List for names and values have an internal construction pattern!
    sal_Int32   nItem     = 0;
    OUString    sCmd;

    m_aDisabledCommands.Clear();

    // Get names/values for disabled commands.
    for( nItem=0; nItem < lNames.getLength(); ++nItem )
    {
        // Currently only one value
        lValues[nItem] >>= sCmd;
        m_aDisabledCommands.AddCommand( sCmd );
    }

    // don't forget to update all existing frames and her might cached dispatch objects!
    // But look for already killed frames. We hold weak references instead of hard ones ...
    for (SvtFrameVector::iterator pIt  = m_lFrames.begin(); pIt != m_lFrames.end(); )
    {
        css::uno::Reference< css::frame::XFrame > xFrame(pIt->get(), css::uno::UNO_QUERY);
        if (xFrame.is())
        {
            aGuard.unlock(); // because we can call back into ourself
            xFrame->contextChanged();
            aGuard.lock();
            ++pIt;
        }
        else
            pIt = m_lFrames.erase(pIt);
    }
}

//  public method

void SvtCommandOptions_Impl::ImplCommit()
{
    SAL_WARN("unotools.config","SvtCommandOptions_Impl::ImplCommit(): Not implemented yet!");
}

//  public method

bool SvtCommandOptions_Impl::HasEntriesDisabled() const
{
    return m_aDisabledCommands.HasEntries();
}

//  public method

bool SvtCommandOptions_Impl::LookupDisabled( const OUString& aCommand ) const
{
    return m_aDisabledCommands.Lookup( aCommand );
}

//  public method

void SvtCommandOptions_Impl::EstablishFrameCallback(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    // check if frame already exists inside list
    // ignore double registrations
    // every frame must be notified one times only!
    css::uno::WeakReference< css::frame::XFrame > xWeak(xFrame);
    SvtFrameVector::const_iterator pIt = ::std::find(m_lFrames.begin(), m_lFrames.end(), xWeak);
    if (pIt == m_lFrames.end())
        m_lFrames.push_back(xWeak);
}

//  private method

Sequence< OUString > SvtCommandOptions_Impl::impl_GetPropertyNames()
{
    // First get ALL names of current existing list items in configuration!
    Sequence< OUString > lDisabledItems      = GetNodeNames( u"" SETNODE_DISABLED ""_ustr, utl::ConfigNameFormat::LocalPath );

    // Expand all keys
    for (OUString& rItem : asNonConstRange(lDisabledItems))
        rItem =  SETNODE_DISABLED PATHDELIMITER + rItem + PATHDELIMITER PROPERTYNAME_CMD;

    // Return result.
    return lDisabledItems;
}

namespace {

std::weak_ptr<SvtCommandOptions_Impl> g_pCommandOptions;

}

SvtCommandOptions::SvtCommandOptions()
{
    // Global access, must be guarded (multithreading!).
    std::unique_lock aGuard( GetOwnStaticMutex() );

    m_pImpl = g_pCommandOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtCommandOptions_Impl>();
        g_pCommandOptions = m_pImpl;
        aGuard.unlock(); // because holdConfigItem will call this constructor
        ItemHolder1::holdConfigItem(EItem::CmdOptions);
    }
}

SvtCommandOptions::~SvtCommandOptions()
{
    // Global access, must be guarded (multithreading!)
    std::unique_lock aGuard( GetOwnStaticMutex() );

    m_pImpl.reset();
}

//  public method

bool SvtCommandOptions::HasEntriesDisabled() const
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    return m_pImpl->HasEntriesDisabled();
}

//  public method

bool SvtCommandOptions::LookupDisabled( const OUString& aCommandURL ) const
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    return m_pImpl->LookupDisabled( aCommandURL );
}

//  public method

void SvtCommandOptions::EstablishFrameCallback(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    std::unique_lock aGuard( GetOwnStaticMutex() );
    m_pImpl->EstablishFrameCallback(xFrame);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
