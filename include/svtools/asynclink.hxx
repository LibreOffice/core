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

#ifndef INCLUDED_SVTOOLS_ASYNCLINK_HXX
#define INCLUDED_SVTOOLS_ASYNCLINK_HXX

#include <svtools/svtdllapi.h>
#include <tools/solar.h>
#include <tools/link.hxx>
#include <osl/mutex.hxx>

class Timer;

namespace svtools {

class SVT_DLLPUBLIC AsynchronLink
{
    Link   _aLink;
    sal_uLong  _nEventId;
    Timer* _pTimer;
    bool   _bInCall;
    bool*  _pDeleted;
    void*  _pArg;
    ::osl::Mutex* _pMutex;

    DECL_DLLPRIVATE_STATIC_LINK( AsynchronLink, HandleCall, void* );
    SVT_DLLPRIVATE void Call_Impl( void* pArg );

public:
    AsynchronLink( const Link& rLink )
        : _aLink( rLink )
        , _nEventId( 0 )
        , _pTimer( 0 )
        , _bInCall( false )
        , _pDeleted( 0 )
        , _pArg( 0 )
        , _pMutex( 0 )
    {}
    AsynchronLink()
        : _nEventId( 0 )
        , _pTimer( 0 )
        , _bInCall( false )
        , _pDeleted( 0 )
        , _pArg( 0 )
        , _pMutex( 0 )
    {}
    ~AsynchronLink();

    void CreateMutex();
    void operator=( const Link& rLink ) { _aLink = rLink; }
    void Call( void* pObj, bool bAllowDoubles = false, bool bUseTimer = false );
    void ClearPendingCall( );
    bool IsSet() const { return _aLink.IsSet(); }
    Link GetLink() const { return _aLink; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
