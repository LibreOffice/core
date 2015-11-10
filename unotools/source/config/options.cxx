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
#include <unotools/options.hxx>

using utl::detail::Options;
using utl::ConfigurationBroadcaster;

utl::ConfigurationListener::~ConfigurationListener() {}

ConfigurationBroadcaster::ConfigurationBroadcaster()
: mpList(nullptr)
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
    mpList->push_back( pListener );
}

void ConfigurationBroadcaster::RemoveListener( utl::ConfigurationListener* pListener )
{
    if ( mpList ) {
        for ( IMPL_ConfigurationListenerList::iterator it = mpList->begin();
              it != mpList->end();
              ++it
        ) {
            if ( *it == pListener ) {
                mpList->erase( it );
                break;
            }
        }
    }
}

void ConfigurationBroadcaster::NotifyListeners( sal_uInt32 nHint )
{
    if ( m_nBroadcastBlocked )
        m_nBlockedHint |= nHint;
    else
    {
        nHint |= m_nBlockedHint;
        m_nBlockedHint = 0;
        if ( mpList ) {
            for ( size_t n = 0; n < mpList->size(); n++ ) {
                (*mpList)[ n ]->ConfigurationChanged( this, nHint );
            }
        }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
