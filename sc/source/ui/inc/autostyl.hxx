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

#ifndef INCLUDED_SC_SOURCE_UI_INC_AUTOSTYL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_AUTOSTYL_HXX

#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <rtl/ustring.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>

class ScDocShell;
class ScRange;
struct ScAutoStyleInitData;

struct ScAutoStyleData
{
    sal_uLong nTimeout;
    ScRange   aRange;
    OUString  aStyle;

    ScAutoStyleData( sal_uLong nT, const ScRange& rR, const OUString& rT ) :
        nTimeout(nT), aRange(rR), aStyle(rT) {}
};

class ScAutoStyleList
{
private:

    ScDocShell*     pDocSh;
    Timer           aTimer;
    Idle            aInitIdle;
    sal_uLong       nTimerStart;
    std::vector<ScAutoStyleData>           aEntries;
    boost::ptr_vector<ScAutoStyleInitData> aInitials;

    void    ExecuteEntries();
    void    AdjustEntries(sal_uLong nDiff);
    void    StartTimer(sal_uLong nNow);
    DECL_LINK_TYPED( TimerHdl, Timer*, void );
    DECL_LINK_TYPED( InitHdl, Idle*, void );

public:
            ScAutoStyleList(ScDocShell* pShell);
            ~ScAutoStyleList();

    void    AddInitial( const ScRange& rRange, const OUString& rStyle1,
                        sal_uLong nTimeout, const OUString& rStyle2 );
    void    AddEntry( sal_uLong nTimeout, const ScRange& rRange, const OUString& rStyle );

    void    ExecuteAllNow();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
