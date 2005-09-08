/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autostyl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:13:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_AUTOSTYL_HXX
#define SC_AUTOSTYL_HXX

#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class ScDocShell;
class ScRange;

class ScAutoStyleList
{
private:
    ScDocShell*     pDocSh;
    Timer           aTimer;
    Timer           aInitTimer;
    ULONG           nTimerStart;
    List            aEntries;
    List            aInitials;

    void    ExecuteEntries();
    void    AdjustEntries(ULONG nDiff);
    void    StartTimer(ULONG nNow);
    DECL_LINK( TimerHdl, Timer* );
    DECL_LINK( InitHdl, Timer* );

public:
            ScAutoStyleList(ScDocShell* pShell);
            ~ScAutoStyleList();

    void    AddInitial( const ScRange& rRange, const String& rStyle1,
                        ULONG nTimeout, const String& rStyle2 );
    void    AddEntry( ULONG nTimeout, const ScRange& rRange, const String& rStyle );

    void    ExecuteAllNow();
};



#endif

