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


#include <svtools/asynclink.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <vcl/timer.hxx>
#include <vcl/svapp.hxx>


namespace svtools {

void AsynchronLink::Call( void* pObj, bool bAllowDoubles )
{
    if( !_aLink.IsSet() )
        return;

    _pArg = pObj;
    DBG_ASSERT( bAllowDoubles ||  !_nEventId, "Already made a call" );
    ClearPendingCall();
    std::scoped_lock aGuard(_aMutex);
    _nEventId = Application::PostUserEvent( LINK( this, AsynchronLink, HandleCall_PostUserEvent) );
}

AsynchronLink::~AsynchronLink()
{
    if( _nEventId )
    {
        Application::RemoveUserEvent( _nEventId );
    }
}

void AsynchronLink::ClearPendingCall()
{
    std::scoped_lock aGuard(_aMutex);
    if( _nEventId )
    {
        Application::RemoveUserEvent( _nEventId );
        _nEventId = nullptr;
    }
}

IMPL_LINK_NOARG( AsynchronLink, HandleCall_PostUserEvent, void*, void )
{
    {
        std::scoped_lock aGuard(_aMutex);
        _nEventId = nullptr;
        // need to release the lock before calling the client since
        // the client may call back into us
    }
    _aLink.Call( _pArg );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
