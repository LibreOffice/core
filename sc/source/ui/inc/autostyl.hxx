/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_AUTOSTYL_HXX
#define SC_AUTOSTYL_HXX

#include <vcl/timer.hxx>
#include <tools/list.hxx>
#include <tools/string.hxx>

class ScDocShell;
class ScRange;

class ScAutoStyleList
{
private:
    ScDocShell*     pDocSh;
    Timer           aTimer;
    Timer           aInitTimer;
    sal_uLong           nTimerStart;
    List            aEntries;
    List            aInitials;

    void    ExecuteEntries();
    void    AdjustEntries(sal_uLong nDiff);
    void    StartTimer(sal_uLong nNow);
    DECL_LINK( TimerHdl, Timer* );
    DECL_LINK( InitHdl, Timer* );

public:
            ScAutoStyleList(ScDocShell* pShell);
            ~ScAutoStyleList();

    void    AddInitial( const ScRange& rRange, const String& rStyle1,
                        sal_uLong nTimeout, const String& rStyle2 );
    void    AddEntry( sal_uLong nTimeout, const ScRange& rRange, const String& rStyle );

    void    ExecuteAllNow();
};



#endif

