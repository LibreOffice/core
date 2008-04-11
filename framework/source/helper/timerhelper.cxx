/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: timerhelper.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <helper/timerhelper.hxx>
#include <threadhelp/resetableguard.hxx>
#include <vcl/svapp.hxx>

//........................................................................
namespace framework
{
//........................................................................

    //====================================================================
    //= OTimerHelper
    //====================================================================
    //--------------------------------------------------------------------
    OTimerHelper::OTimerHelper( const ::vos::TTimeValue& _rExpireTime  )
        : ThreadHelpBase ( &Application::GetSolarMutex()    )
        , OTimer         ( _rExpireTime                     )
    {
    }

    //--------------------------------------------------------------------
    void SAL_CALL OTimerHelper::onShot()
    {
        ResetableGuard aGuard( m_aLock );
        if( m_pListener != NULL )
        {
            m_pListener->timerExpired();
        }
    }

//........................................................................
}   // namespace framework
//........................................................................

