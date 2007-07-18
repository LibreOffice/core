/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pausethreadstarting.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:31:19 $
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

#ifndef _PAUSETHREADSTARTING_HXX
#include <pausethreadstarting.hxx>
#endif

#ifndef _SWTHREADMANAGER_HXX
#include <swthreadmanager.hxx>
#endif

/** Helper class to pause starting of threads during existence of an instance
    of this class

    OD 2007-03-16 #i73788#

    @author OD
*/
SwPauseThreadStarting::SwPauseThreadStarting()
    : mbPausedThreadStarting( false )
{
    if ( SwThreadManager::ExistsThreadManager() &&
         !SwThreadManager::GetThreadManager().StartingOfThreadsSuspended() )
    {
        SwThreadManager::GetThreadManager().SuspendStartingOfThreads();
        mbPausedThreadStarting = true;
    }
}

SwPauseThreadStarting::~SwPauseThreadStarting()
{
    if ( mbPausedThreadStarting )
    {
        SwThreadManager::GetThreadManager().ResumeStartingOfThreads();
    }
}
