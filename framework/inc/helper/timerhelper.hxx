/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: timerhelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:19:54 $
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

#ifndef __FRAMEWORK_HELPER_TIMERHELPER_HXX_
#define __FRAMEWORK_HELPER_TIMERHELPER_HXX_

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef _VOS_TIMER_HXX_
#include <vos/timer.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

//........................................................................
namespace framework
{
//........................................................................

    //====================================================================
    //= ITimerListener
    //====================================================================
    class ITimerListener
    {
    public:
        virtual void    timerExpired() = 0;
    };

    //====================================================================
    //= OTimerHelper
    //====================================================================
    /** implements a timer
    */
    class OTimerHelper : private ThreadHelpBase
                       , public ::vos::OTimer
    {
    protected:
        ITimerListener* m_pListener;

    public:
        OTimerHelper( const ::vos::TTimeValue& _rExpireTime );

        // the link set here will be called with m_rMutex acquired
        void            setListener( ITimerListener* _pListener )       { m_pListener = _pListener; }
        ITimerListener* getListener(                            ) const { return m_pListener;       }

    protected:
        virtual void SAL_CALL onShot();
    };

//........................................................................
}   // namespace framework
//........................................................................

#endif // _FRAMEWORK_HELPER_TIMERHELPER_HXX_

