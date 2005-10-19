/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: delayevent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 11:50:09 $
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

#include "delayevent.hxx"
#include "osl/diagnose.h"

namespace presentation {
namespace internal {

bool Delay::fire()
{
    OSL_ASSERT( isCharged() );
    if (isCharged()) {
        mbWasFired = true;
        maFunc();
        maFunc.clear(); // early release of payload
    }
    return true;
}

bool Delay::isCharged() const
{
    return !mbWasFired;
}

double Delay::getActivationTime( double nCurrentTime ) const
{
    return nCurrentTime + mnTimeout;
}

void Delay::dispose()
{
    mbWasFired = true;
    maFunc.clear(); // release of payload
}

} // namespace internal
} // namespace presentation

