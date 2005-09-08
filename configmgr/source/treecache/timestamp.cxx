/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: timestamp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:25:38 $
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

#include <stdio.h>

#include "timestamp.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
    const sal_uInt32 maxTimeValueSeconds    = 0xFF000000ul; // catches accidetally added values as well
    const sal_uInt32 maxTimeValueNanos      = 999999999ul;
    const sal_uInt32 minTimeValueSeconds    = 0ul;
    const sal_uInt32 minTimeValueNanos      = 0ul;
// -------------------------------------------------------------------------
    TimeStamp TimeStamp::never()
    {
        const ::TimeValue maxTimeValue = { maxTimeValueSeconds, maxTimeValueNanos };

        return TimeStamp( maxTimeValue );
    }
// -------------------------------------------------------------------------
    TimeStamp TimeStamp::always()
    {
        const ::TimeValue minTimeValue = { minTimeValueSeconds, minTimeValueNanos };

        return TimeStamp( minTimeValue );
    }
// -------------------------------------------------------------------------
    TimeStamp TimeStamp::getCurrentTime()
    {
        TimeStamp aResult;
        if (! ::osl_getSystemTime( &aResult.m_aTime ) )
            return never();
        return aResult;
    }
// -------------------------------------------------------------------------
} // namespace configmgr


