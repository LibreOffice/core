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

class Idle;
struct ImplSVEvent;

namespace svtools {

class SVT_DLLPUBLIC AsynchronLink
{
    Link<void*,void> _aLink;
    ImplSVEvent*     _nEventId;
    Idle*            _pIdle;
    bool             _bInCall;
    bool*            _pDeleted;
    void*            _pArg;
    ::osl::Mutex*    _pMutex;

    DECL_DLLPRIVATE_LINK_TYPED( HandleCall_Idle, Idle*, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleCall_PostUserEvent, void*, void );
    SVT_DLLPRIVATE void Call_Impl( void* pArg );

public:
    AsynchronLink( const Link<void*,void>& rLink )
        : _aLink( rLink )
        , _nEventId( nullptr )
        , _pIdle( nullptr )
        , _bInCall( false )
        , _pDeleted( nullptr )
        , _pArg( nullptr )
        , _pMutex( nullptr )
    {}
    AsynchronLink()
        : _nEventId( nullptr )
        , _pIdle( nullptr )
        , _bInCall( false )
        , _pDeleted( nullptr )
        , _pArg( nullptr )
        , _pMutex( nullptr )
    {}
    ~AsynchronLink();

    void CreateMutex();
    void operator=( const Link<void*,void>& rLink ) { _aLink = rLink; }
    void Call( void* pObj, bool bAllowDoubles = false, bool bUseTimer = false );
    void ClearPendingCall( );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
