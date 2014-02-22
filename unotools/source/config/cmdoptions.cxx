/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <unotools/cmdoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>

#include <itemholder1.hxx>

#include <algorithm>
#include <boost/unordered_map.hpp>

using namespace ::std                   ;
using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::beans ;

#define ROOTNODE_CMDOPTIONS                             OUString("Office.Commands/Execute")
#define PATHDELIMITER                                   OUString("/")

#define SETNODE_DISABLED                                OUString("Disabled")

#define PROPERTYNAME_CMD                                OUString("Command")

/*-****************************************************************************************************************
    @descr  support simple command option structures and operations on it
****************************************************************************************************************-*/
class SvtCmdOptions
{
    public:
        
        
        void Clear()
        {
            m_aCommandHashMap.clear();
        }

        bool HasEntries() const
        {
            return ( m_aCommandHashMap.size() > 0 );
        }

        bool Lookup( const OUString& aCmd ) const
        {
            CommandHashMap::const_iterator pEntry = m_aCommandHashMap.find( aCmd );
            return ( pEntry != m_aCommandHashMap.end() );
        }

        void AddCommand( const OUString& aCmd )
        {
            m_aCommandHashMap.insert( CommandHashMap::value_type( aCmd, 0 ) );
        }

        
        
        
        
        
        
        
        
        Sequence< OUString > GetList() const
        {
            sal_Int32               nCount = (sal_Int32)m_aCommandHashMap.size();
            sal_Int32               nIndex = 0;
            Sequence< OUString >    aList( nCount );

            CommandHashMap::const_iterator pEntry = m_aCommandHashMap.begin();
            while ( pEntry != m_aCommandHashMap.end() )
                aList[nIndex++] = pEntry->first;

            return aList;
        }

    private:
        class CommandHashMap : public ::boost::unordered_map< OUString      ,
                                                        sal_Int32           ,
                                                        OUStringHash        ,
                                                        ::std::equal_to< OUString >  >
        {
            public:
                inline void free()
                {
                    CommandHashMap().swap( *this );
                }
        };

        CommandHashMap m_aCommandHashMap;
};

typedef ::std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > > SvtFrameVector;

class SvtCommandOptions_Impl : public ConfigItem
{
    public:

         SvtCommandOptions_Impl();
        ~SvtCommandOptions_Impl();

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *

        virtual void Notify( const Sequence< OUString >& lPropertyNames );

        /*-****************************************************************************************************
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *

        virtual void Commit();

        /*-****************************************************************************************************
            @short      base implementation of public interface for "SvtDynamicMenuOptions"!
            @descr      These class is used as static member of "SvtDynamicMenuOptions" ...
                        => The code exist only for one time and isn't duplicated for every instance!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *

        bool                HasEntries  (   SvtCommandOptions::CmdOption    eOption     ) const;
        bool                Lookup      (   SvtCommandOptions::CmdOption    eCmdOption, const OUString& ) const;
        void EstablisFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);

    private:

        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent oue module tree
            @descr      These methods return the current list of key names! We need it to get needed values from our
                        configuration management and support dynamical menu item lists!

            @seealso    -

            @param      "nDisabledCount"    ,   returns count of menu entries for "new"
            @return     A list of configuration key names is returned.

            @onerror    -
        *

        Sequence< OUString > impl_GetPropertyNames();

    private:
        SvtCmdOptions  m_aDisabledCommands;
        SvtFrameVector m_lFrames;
};




SvtCommandOptions_Impl::SvtCommandOptions_Impl()
    
    :   ConfigItem( ROOTNODE_CMDOPTIONS )
    
{
    
    
    Sequence< OUString >    lNames              = impl_GetPropertyNames ();
    Sequence< Any >         lValues             = GetProperties         ( lNames         );

    
    
    
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtCommandOptions_Impl::SvtCommandOptions_Impl()\nI miss some values of configuration keys!\n" );

    
    
    sal_Int32   nItem     = 0 ;
    OUString    sCmd          ;

    
    for( nItem=0; nItem < lNames.getLength(); ++nItem )
    {
        
        lValues[nItem] >>= sCmd;
        m_aDisabledCommands.AddCommand( sCmd );
    }

/*TODO: Not used in the moment! see Notify() ...
    
    
    Sequence< OUString > aNotifySeq( 1 );
    aNotifySeq[0] = "Disabled";
    EnableNotification( aNotifySeq, true );
}




SvtCommandOptions_Impl::~SvtCommandOptions_Impl()
{
    
    if( IsModified() )
    {
        Commit();
    }
}




void SvtCommandOptions_Impl::Notify( const Sequence< OUString >& )
{
    MutexGuard aGuard( SvtCommandOptions::GetOwnStaticMutex() );

    Sequence< OUString >    lNames   = impl_GetPropertyNames ();
    Sequence< Any >         lValues  = GetProperties         ( lNames         );

    
    
    
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtCommandOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );

    
    
    sal_Int32   nItem     = 0 ;
    OUString    sCmd          ;

    m_aDisabledCommands.Clear();

    
    for( nItem=0; nItem < lNames.getLength(); ++nItem )
    {
        
        lValues[nItem] >>= sCmd;
        m_aDisabledCommands.AddCommand( sCmd );
    }

    
    
    for (SvtFrameVector::const_iterator pIt  = m_lFrames.begin();
                                        pIt != m_lFrames.end()  ;
                                      ++pIt                     )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame(pIt->get(), ::com::sun::star::uno::UNO_QUERY);
        if (xFrame.is())
            xFrame->contextChanged();
    }
}




void SvtCommandOptions_Impl::Commit()
{
    OSL_FAIL( "SvtCommandOptions_Impl::Commit()\nNot implemented yet!\n" );
}




bool SvtCommandOptions_Impl::HasEntries( SvtCommandOptions::CmdOption eOption ) const
{
    if ( eOption == SvtCommandOptions::CMDOPTION_DISABLED )
        return m_aDisabledCommands.HasEntries();
    else
        return false;
}




bool SvtCommandOptions_Impl::Lookup( SvtCommandOptions::CmdOption eCmdOption, const OUString& aCommand ) const
{
    switch( eCmdOption )
    {
        case SvtCommandOptions::CMDOPTION_DISABLED:
        {
            return m_aDisabledCommands.Lookup( aCommand );
        }
        default:
            DBG_ASSERT( false, "SvtCommandOptions_Impl::Lookup()\nUnknown option type given!\n" );
    }

    return false;
}




void SvtCommandOptions_Impl::EstablisFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame)
{
    
    
    
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > xWeak(xFrame);
    SvtFrameVector::const_iterator pIt = ::std::find(m_lFrames.begin(), m_lFrames.end(), xWeak);
    if (pIt == m_lFrames.end())
        m_lFrames.push_back(xWeak);
}




Sequence< OUString > SvtCommandOptions_Impl::impl_GetPropertyNames()
{
    
    Sequence< OUString > lDisabledItems      = GetNodeNames( SETNODE_DISABLED, utl::CONFIG_NAME_LOCAL_PATH );

    OUString aSetNode( SETNODE_DISABLED );
    aSetNode += PATHDELIMITER;

    OUString aCommandKey( PATHDELIMITER );
    aCommandKey += PROPERTYNAME_CMD;

    
    for (sal_Int32 i=0; i<lDisabledItems.getLength(); ++i )
    {
        OUStringBuffer aBuffer( 32 );
        aBuffer.append( aSetNode );
        aBuffer.append( lDisabledItems[i] );
        aBuffer.append( aCommandKey );
        lDisabledItems[i] = aBuffer.makeStringAndClear();
    }

    
    return lDisabledItems;
}






SvtCommandOptions_Impl*     SvtCommandOptions::m_pDataContainer = NULL  ;
sal_Int32                   SvtCommandOptions::m_nRefCount      = 0     ;




SvtCommandOptions::SvtCommandOptions()
{
    
    MutexGuard aGuard( GetOwnStaticMutex() );
    
    ++m_nRefCount;
    
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtCommandOptions_Impl;
        ItemHolder1::holdConfigItem(E_CMDOPTIONS);
    }
}




SvtCommandOptions::~SvtCommandOptions()
{
    
    MutexGuard aGuard( GetOwnStaticMutex() );
    
    --m_nRefCount;
    
    
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}




bool SvtCommandOptions::HasEntries( CmdOption eOption ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->HasEntries( eOption );
}




bool SvtCommandOptions::Lookup( CmdOption eCmdOption, const OUString& aCommandURL ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->Lookup( eCmdOption, aCommandURL );
}




void SvtCommandOptions::EstablisFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->EstablisFrameCallback(xFrame);
}

namespace
{
    class theCommandOptionsMutex : public rtl::Static<osl::Mutex, theCommandOptionsMutex>{};
}




Mutex& SvtCommandOptions::GetOwnStaticMutex()
{
    return theCommandOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
