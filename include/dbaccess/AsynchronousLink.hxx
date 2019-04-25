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

#pragma once

#include <osl/mutex.hxx>
#include <tools/link.hxx>

struct ImplSVEvent;

namespace dbaui
{

    // a helper for multi-threaded handling of async events

    /** handles asynchronous links which may be called in multi-threaded environments
        If you use an instance of this class as member of your own class, it will handle
        several crucial points for you (for instance the case that somebody posts the
        event while another thread tries to delete this event in the _destructor_ of the
        class).
    */
    class OAsynchronousLink final
    {
        Link<void*,void>    m_aHandler;
        ::osl::Mutex        m_aEventSafety;
        ::osl::Mutex        m_aDestructionSafety;
        ImplSVEvent *       m_nEventId;
        DECL_LINK(OnAsyncCall, void*, void);

    public:
        /** constructs the object
            @param      _rHandler           The link to be called asynchronously
        */
        OAsynchronousLink( const Link<void*,void>& _rHandler );
        ~OAsynchronousLink();

        bool    IsRunning() const { return m_nEventId != nullptr; }

        void Call( void* _pArgument = nullptr );
        void CancelCall();
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
