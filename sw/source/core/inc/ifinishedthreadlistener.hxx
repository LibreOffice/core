/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ifinishedthreadlistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:33:25 $
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
#ifndef _IFINISHEDTHREADLISTENER_HXX
#define _IFINISHEDTHREADLISTENER_HXX

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

/** interface class to listen on the finish of a thread

    OD 2007-03-30 #i73788#
    Note: The thread provides its ThreadID on the finish notification

    @author OD
*/
class IFinishedThreadListener
{
    public:

        inline virtual ~IFinishedThreadListener()
        {
        };

        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID ) = 0;

    protected:

        inline IFinishedThreadListener()
        {
        };
};
#endif
