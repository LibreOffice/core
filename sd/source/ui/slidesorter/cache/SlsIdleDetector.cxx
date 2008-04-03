/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsIdleDetector.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:18:23 $
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

#include "SlsIdleDetector.hxx"

#include "view/SlideSorterView.hxx"
#include "tools/IdleDetection.hxx"
#include "Window.hxx"

namespace sd { namespace slidesorter { namespace cache {

IdleDetector::IdleDetector (
    const ::Window* pWindow,
    const bool bIsSuspendPreviewUpdatesDuringFullScreenPresentation)
    : mpWindow(pWindow),
      mbIsSuspendPreviewUpdatesDuringFullScreenPresentation(
          bIsSuspendPreviewUpdatesDuringFullScreenPresentation)
{
}




bool IdleDetector::operator() (void)
{
    // Determine whether the system is idle.
    sal_Int32 nIdleState (tools::IdleDetection::GetIdleState(mpWindow));
    if (nIdleState == tools::IdleDetection::IDET_IDLE)
        return true;
    else
        return false;
}


} } } // end of namespace ::sd::slidesorter::cache
