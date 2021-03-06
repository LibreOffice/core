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

#include <address.hxx>
#include <vector>
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>

class ScDocShell;
class ScRange;

struct ScAutoStyleData
{
    sal_uLong nTimeout;
    ScRange   aRange;
    OUString  aStyle;

    ScAutoStyleData( sal_uLong nT, const ScRange& rR, const OUString& rT ) :
        nTimeout(nT), aRange(rR), aStyle(rT) {}
};
struct ScAutoStyleInitData
{
    ScRange     aRange;
    OUString    aStyle1;
    sal_uLong   nTimeout;
    OUString    aStyle2;

    ScAutoStyleInitData( const ScRange& rR, const OUString& rSt1, sal_uLong nT, const OUString& rSt2 ) :
        aRange(rR), aStyle1(rSt1), nTimeout(nT), aStyle2(rSt2) {}
};


class ScAutoStyleList
{
private:

    ScDocShell*     pDocSh;
    Timer           aTimer;
    Idle            aInitIdle;
    sal_uLong       nTimerStart;
    std::vector<ScAutoStyleData>     aEntries;
    std::vector<ScAutoStyleInitData> aInitials;

    void    ExecuteEntries();
    void    AdjustEntries(sal_uLong nDiff);
    void    StartTimer(sal_uLong nNow);
    DECL_LINK( TimerHdl, Timer*, void );
    DECL_LINK( InitHdl, Timer*, void );

public:
            ScAutoStyleList(ScDocShell* pShell);
            ~ScAutoStyleList();

    void    AddInitial( const ScRange& rRange, const OUString& rStyle1,
                        sal_uLong nTimeout, const OUString& rStyle2 );
    void    AddEntry( sal_uLong nTimeout, const ScRange& rRange, const OUString& rStyle );

    void    ExecuteAllNow();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
