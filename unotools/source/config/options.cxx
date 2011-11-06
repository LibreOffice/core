/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include "sal/config.h"
#include <tools/list.hxx>
#include <unotools/options.hxx>

namespace utl
{
    DECLARE_LIST( IMPL_ConfigurationListenerList, ConfigurationListener* )
}

using utl::detail::Options;
using utl::ConfigurationBroadcaster;

ConfigurationBroadcaster::ConfigurationBroadcaster()
: mpList(0)
, m_nBroadcastBlocked( 0 )
, m_nBlockedHint( 0 )
{
}

ConfigurationBroadcaster::~ConfigurationBroadcaster()
{
    delete mpList;
}

void ConfigurationBroadcaster::AddListener( utl::ConfigurationListener* pListener )
{
    if ( !mpList )
        mpList = new IMPL_ConfigurationListenerList;
    mpList->Insert( pListener );
}

void ConfigurationBroadcaster::RemoveListener( utl::ConfigurationListener* pListener )
{
    if ( mpList )
        mpList->Remove( pListener );
}

void ConfigurationBroadcaster::NotifyListeners( sal_uInt32 nHint )
{
    if ( m_nBroadcastBlocked )
        m_nBlockedHint |= nHint;
    else
    {
        nHint |= m_nBlockedHint;
        m_nBlockedHint = 0;
        if ( mpList )
            for ( sal_uInt32 n=0; n<mpList->Count(); n++ )
                mpList->GetObject(n)->ConfigurationChanged( this, nHint );
    }
}

void ConfigurationBroadcaster::BlockBroadcasts( bool bBlock )
{
    if ( bBlock )
        ++m_nBroadcastBlocked;
    else if ( m_nBroadcastBlocked )
    {
        if ( --m_nBroadcastBlocked == 0 )
            NotifyListeners( 0 );
    }
}

Options::Options()
{
}

Options::~Options()
{
}

void Options::ConfigurationChanged( ConfigurationBroadcaster*, sal_uInt32 nHint )
{
    NotifyListeners( nHint );
}


