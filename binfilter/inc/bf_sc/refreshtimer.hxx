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

#ifndef SC_REFRESHTIMER_HXX
#define SC_REFRESHTIMER_HXX

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif
namespace binfilter {

#define SC_REFRESHTIMER_CONTROL_LIST 0
#if SC_REFRESHTIMER_CONTROL_LIST
class ScRefreshTimer;
DECLARE_LIST( ScRefreshTimerList, ScRefreshTimer* )
#endif

class ScRefreshTimerControl
{
private:
            ::vos::OMutex		aMutex;
            USHORT				nBlockRefresh;

public:
#if SC_REFRESHTIMER_CONTROL_LIST
            ScRefreshTimerList	aList;
#endif

                                ScRefreshTimerControl() : nBlockRefresh(0) {}

            void				SetAllowRefresh( BOOL b )
                                    {
                                        if ( b && nBlockRefresh )
                                            --nBlockRefresh;
                                        else if ( !b && nBlockRefresh < (USHORT)(~0) )
                                            ++nBlockRefresh;
                                    }
            BOOL				IsRefreshAllowed() const	{ return !nBlockRefresh; }
            ::vos::OMutex&		GetMutex()					{ return aMutex; }
};


class ScRefreshTimerProtector
{
private:
    ScRefreshTimerControl * const *	ppControl;
public:
                                ScRefreshTimerProtector( ScRefreshTimerControl * const *	 pp );
                                ~ScRefreshTimerProtector()
                                    {
                                        if ( ppControl && *ppControl )
                                            (*ppControl)->SetAllowRefresh( TRUE );
                                    }
};


class ScRefreshTimer : public AutoTimer
{
private:
    ScRefreshTimerControl * const *	ppControl;

            void				AppendToControl()
                                    {
#if SC_REFRESHTIMER_CONTROL_LIST
                                        if ( ppControl && *ppControl )
                                            (*ppControl)->aList.Insert( this, LIST_APPEND );
#endif
                                    }
            void				RemoveFromControl()
                                    {
#if SC_REFRESHTIMER_CONTROL_LIST
                                        if ( ppControl && *ppControl )
                                            (*ppControl)->aList.Remove( this );
#endif
                                    }

            void				Start()
                                    {
                                        if ( GetTimeout() )
                                            AutoTimer::Start();
                                    }

public:
                                ScRefreshTimer() : ppControl(0)
                                    { SetTimeout( 0 ); }
                                ScRefreshTimer( ULONG nSeconds ) : ppControl(0)
                                    {
                                        SetTimeout( nSeconds * 1000 );
                                        Start();
                                    }
                                ScRefreshTimer( const ScRefreshTimer& r )
                                    : AutoTimer( r ), ppControl(0)
                                    {}
    virtual						~ScRefreshTimer();

            ScRefreshTimer&		operator=( const ScRefreshTimer& r )
                                    {
                                        SetRefreshControl(0);
                                        AutoTimer::operator=( r );
                                        return *this;
                                    }

            BOOL				operator==( const ScRefreshTimer& r ) const
                                    { return GetTimeout() == r.GetTimeout(); }

            BOOL				operator!=( const ScRefreshTimer& r ) const
                                    { return !ScRefreshTimer::operator==( r ); }

            void				StartRefreshTimer()
                                    { Start(); }

            void				SetRefreshControl( ScRefreshTimerControl * const * pp )
                                    {
                                        RemoveFromControl();
                                        ppControl = pp;
                                        AppendToControl();
                                    }

            void				SetRefreshHandler( const Link& rLink )
                                    { SetTimeoutHdl( rLink ); }

            ULONG				GetRefreshDelay() const
                                    { return GetTimeout() / 1000; }

            void				StopRefreshTimer()
                                    { Stop(); }

    virtual	void				SetRefreshDelay( ULONG nSeconds );
};


} //namespace binfilter
#endif // SC_REFRESHTIMER_HXX

