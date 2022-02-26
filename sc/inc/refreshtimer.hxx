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

#include <sal/config.h>

#include <vcl/timer.hxx>
#include "scdllapi.h"
#include <memory>
#include <mutex>

class ScRefreshTimerControl
{
    std::recursive_mutex    aMutex;
    sal_uInt16              nBlockRefresh;

public:
    ScRefreshTimerControl() : nBlockRefresh(0) {}
    void SetAllowRefresh( bool b );
    bool IsRefreshAllowed() const { return !nBlockRefresh; }
    std::recursive_mutex& GetMutex() { return aMutex; }
};

class ScRefreshTimer : public AutoTimer
{
    std::unique_ptr<ScRefreshTimerControl> const * ppControl;

public:
    ScRefreshTimer();
    ScRefreshTimer( sal_Int32 nSeconds );
    ScRefreshTimer( const ScRefreshTimer& r );
    virtual ~ScRefreshTimer() override;

    ScRefreshTimer& operator=( const ScRefreshTimer& r );
    bool operator==( const ScRefreshTimer& r ) const;
    bool operator!=( const ScRefreshTimer& r ) const;

    void SetRefreshControl( std::unique_ptr<ScRefreshTimerControl> const * pp );
    void SetRefreshHandler( const Link<Timer *, void>& rLink );
    sal_Int32 GetRefreshDelaySeconds() const;
    void StopRefreshTimer();

    void SetRefreshDelay( sal_Int32 nSeconds );
    SC_DLLPUBLIC virtual void Invoke() override;

private:
    void Launch();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
