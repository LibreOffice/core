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

#ifndef INCLUDED_SC_INC_REFRESHTIMER_HXX
#define INCLUDED_SC_INC_REFRESHTIMER_HXX

#include <sal/config.h>

#include <tools/solar.h>
#include <vcl/timer.hxx>
#include <osl/mutex.hxx>
#include "scdllapi.h"

class ScRefreshTimerControl
{
    ::osl::Mutex    aMutex;
    sal_uInt16      nBlockRefresh;

public:
    ScRefreshTimerControl() : nBlockRefresh(0) {}
    void SetAllowRefresh( bool b );
    bool IsRefreshAllowed() const { return !nBlockRefresh; }
    ::osl::Mutex& GetMutex() { return aMutex; }
};

class ScRefreshTimer : public AutoTimer
{
    ScRefreshTimerControl * const * ppControl;

public:
    ScRefreshTimer();
    ScRefreshTimer( sal_uLong nSeconds );
    ScRefreshTimer( const ScRefreshTimer& r );
    virtual ~ScRefreshTimer();

    ScRefreshTimer& operator=( const ScRefreshTimer& r );
    bool operator==( const ScRefreshTimer& r ) const;
    bool operator!=( const ScRefreshTimer& r ) const;

    void SetRefreshControl( ScRefreshTimerControl * const * pp );
    void SetRefreshHandler( const Link<Timer *, void>& rLink );
    sal_uLong GetRefreshDelay() const;
    void StopRefreshTimer();

    void SetRefreshDelay( sal_uLong nSeconds );
    SC_DLLPUBLIC virtual void Invoke() override;

private:
    void Launch();
};

#endif // INCLUDED_SC_INC_REFRESHTIMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
