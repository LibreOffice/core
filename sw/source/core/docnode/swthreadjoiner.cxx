/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swthreadjoiner.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:56:16 $
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
#include "precompiled_sw.hxx"

#ifndef _SWTHREADJOINER_HXX
#include <swthreadjoiner.hxx>
#endif

#ifndef _FINALTHREADMANAGER_HXX
#include <finalthreadmanager.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

/** Testing

    @author OD
*/
using namespace ::com::sun::star;

uno::Reference< util::XJobManager > SwThreadJoiner::mpThreadJoiner;
osl::Mutex* SwThreadJoiner::mpGetJoinerMutex = new osl::Mutex();

uno::Reference< util::XJobManager >& SwThreadJoiner::GetThreadJoiner()
{
    osl::MutexGuard aGuard(*mpGetJoinerMutex);

    if ( !mpThreadJoiner.is() )
    {
        mpThreadJoiner =
            uno::Reference< util::XJobManager >(
                ::comphelper::getProcessServiceFactory()->createInstance(
                    ::rtl::OUString::createFromAscii("com.sun.star.util.JobManager" ) ),
                uno::UNO_QUERY );
    }

    return mpThreadJoiner;
}

void SwThreadJoiner::ReleaseThreadJoiner()
{
    mpThreadJoiner.clear();
}
