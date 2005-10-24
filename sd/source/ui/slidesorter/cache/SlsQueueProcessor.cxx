/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsQueueProcessor.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:41:45 $
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

#include "SlsQueueProcessor.hxx"
#include "SlsCacheConfiguration.hxx"

namespace sd { namespace slidesorter { namespace cache {


//===== QueueProcessorBase  ===================================================

QueueProcessorBase::QueueProcessorBase (void)
    : mnTimeBetweenHighPriorityRequests (10/*ms*/),
      mnTimeBetweenLowPriorityRequests (100/*ms*/),
      mnTimeBetweenRequestsWhenNotIdle (1000/*ms*/)
{
    // Look into the configuration if there for overriding values.
    ::com::sun::star::uno::Any aTimeBetweenReqeusts;
    aTimeBetweenReqeusts = CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TimeBetweenHighPriorityRequests")));
    if (aTimeBetweenReqeusts.has<sal_Int32>())
        aTimeBetweenReqeusts >>= mnTimeBetweenHighPriorityRequests;

    aTimeBetweenReqeusts = CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TimeBetweenLowPriorityRequests")));
    if (aTimeBetweenReqeusts.has<sal_Int32>())
        aTimeBetweenReqeusts >>= mnTimeBetweenLowPriorityRequests;

    aTimeBetweenReqeusts = CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TimeBetweenRequestsDuringShow")));
    if (aTimeBetweenReqeusts.has<sal_Int32>())
        aTimeBetweenReqeusts >>= mnTimeBetweenRequestsWhenNotIdle;

    maTimer.SetTimeoutHdl (LINK(this,QueueProcessorBase,ProcessRequest));
    maTimer.SetTimeout (mnTimeBetweenHighPriorityRequests);
}




void QueueProcessorBase::Start (int nPriorityClass)
{
    if ( ! maTimer.IsActive())
    {
        if (nPriorityClass == 0)
            maTimer.SetTimeout (mnTimeBetweenHighPriorityRequests);
        else
            maTimer.SetTimeout (mnTimeBetweenLowPriorityRequests);
        maTimer.Start();
    }
}




void QueueProcessorBase::Stop (void)
{
    if (maTimer.IsActive())
        maTimer.Stop();
}




IMPL_LINK(QueueProcessorBase, ProcessRequest, Timer*, pTimer)
{
    ProcessRequest();
    return 1;
}




} } } // end of namespace ::sd::slidesorter::cache
