/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: timerhelper.hxx,v $
 * $Revision: 1.5 $
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

#ifndef __FRAMEWORK_HELPER_TIMERHELPER_HXX_
#define __FRAMEWORK_HELPER_TIMERHELPER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <vos/timer.hxx>
#include <osl/mutex.hxx>

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

