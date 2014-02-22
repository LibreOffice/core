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


#include <tools/debug.hxx>

#include <svl/hint.hxx>
#include <svl/brdcst.hxx>

#include <svl/lstner.hxx>
#include <algorithm>

DBG_NAME(SfxListener)
TYPEINIT0(SfxListener);



SfxListener::SfxListener()
{
    DBG_CTOR(SfxListener, 0);
}



SfxListener::SfxListener( const SfxListener &rListener )
{
    DBG_CTOR(SfxListener, 0);

    for ( sal_uInt16 n = 0; n < rListener.aBCs.size(); ++n )
        StartListening( *rListener.aBCs[n] );
}



SfxListener::~SfxListener()
{
    DBG_DTOR(SfxListener, 0);

    
    for ( sal_uInt16 nPos = 0; nPos < aBCs.size(); ++nPos )
    {
        SfxBroadcaster *pBC = aBCs[nPos];
        pBC->RemoveListener(*this);
    }
}




void SfxListener::RemoveBroadcaster_Impl( SfxBroadcaster& rBroadcaster )
{
    DBG_CHKTHIS(SfxListener, 0);

    aBCs.erase( std::find( aBCs.begin(), aBCs.end(), &rBroadcaster ) );
}




bool SfxListener::StartListening( SfxBroadcaster& rBroadcaster, bool bPreventDups )
{
    DBG_CHKTHIS(SfxListener, 0);

    if ( !bPreventDups || !IsListening( rBroadcaster ) )
    {
        rBroadcaster.AddListener(*this);
        aBCs.push_back( &rBroadcaster );

        DBG_ASSERT( IsListening(rBroadcaster), "StartListening failed" );
        return true;
    }

    return false;
}




bool SfxListener::EndListening( SfxBroadcaster& rBroadcaster, bool bAllDups )
{
    DBG_CHKTHIS(SfxListener, 0);

    if ( !IsListening( rBroadcaster ) )
        return false;

    do
    {
        rBroadcaster.RemoveListener(*this);
        aBCs.erase( std::find( aBCs.begin(), aBCs.end(), &rBroadcaster ) );
    }
    while ( bAllDups && IsListening( rBroadcaster ) );
    return true;
}




void SfxListener::EndListeningAll()
{
    DBG_CHKTHIS(SfxListener, 0);

    
    while ( !aBCs.empty() )
    {
        SfxBroadcaster *pBC = aBCs.front();
        pBC->RemoveListener(*this);
        aBCs.pop_front();
    }
}


bool SfxListener::IsListening( SfxBroadcaster& rBroadcaster ) const
{
    return aBCs.end() != std::find( aBCs.begin(), aBCs.end(), &rBroadcaster );
}




#ifdef DBG_UTIL
void SfxListener::Notify( SfxBroadcaster& rBroadcaster, const SfxHint& )
#else
void SfxListener::Notify( SfxBroadcaster&, const SfxHint& )
#endif
{
    #ifdef DBG_UTIL
    DBG_ASSERT(aBCs.end() != std::find(aBCs.begin(), aBCs.end(), &rBroadcaster),
                "notification from unregistered broadcaster" );
    #endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
