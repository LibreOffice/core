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

#ifndef INCLUDED_VCL_INC_SALTIMER_HXX
#define INCLUDED_VCL_INC_SALTIMER_HXX

#include <sal/config.h>
#include <vcl/dllapi.h>
#include "salwtype.hxx"
#include <iostream>

/*
 * note: there will be only a single instance of SalTimer
 * SalTimer originally had only static methods, but
 * this needed to be virtualized for the sal plugin migration
 */

class VCL_PLUGIN_PUBLIC SalTimer
{
    SALTIMERPROC        m_pProc;

public:
    SalTimer() : m_pProc( nullptr ) {}
    virtual ~SalTimer() COVERITY_NOEXCEPT_FALSE;

    // AutoRepeat and Restart
    virtual void            Start( sal_uInt64 nMS ) = 0;
    virtual void            Stop() = 0;

    // Callbacks (indepen in \sv\source\app\timer.cxx)
    void            SetCallback( SALTIMERPROC pProc )
    {
        m_pProc = pProc;
    }

    void            CallCallback()
    {
        if( m_pProc )
            m_pProc();
    }
};

class VersionedEvent
{
    /**
     * The "additional event data" members on macOS are integers, so we can't
     * use an unsigned integer and rely on the defined unsigned overflow in
     * InvalidateEvent().
     */
    sal_Int32 m_nEventVersion;
    bool      m_bIsValidVersion;

public:
    VersionedEvent() : m_nEventVersion( 0 ), m_bIsValidVersion( false ) {}

    sal_Int32 GetNextEventVersion()
    {
        InvalidateEvent();
        m_bIsValidVersion = true;
        return m_nEventVersion;
    }

    void InvalidateEvent()
    {
        if ( m_bIsValidVersion )
        {
            if ( m_nEventVersion == SAL_MAX_INT32 )
                m_nEventVersion = 0;
            else
                ++m_nEventVersion;
            m_bIsValidVersion = false;
        }
    }

    bool ExistsValidEvent() const
    {
        return m_bIsValidVersion;
    }

    bool IsValidEventVersion( const sal_Int32 nEventVersion ) const
    {
        return m_bIsValidVersion && nEventVersion == m_nEventVersion;
    }
};

#endif // INCLUDED_VCL_INC_SALTIMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
