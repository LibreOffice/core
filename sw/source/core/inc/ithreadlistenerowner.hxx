/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ithreadlistenerowner.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:33:36 $
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
#ifndef _ITHREADLISTENEROWNER_HXX
#define _ITHREADLISTENEROWNER_HXX

#ifndef _IFINISHEDTHREADLISTENER_HXX
#include <ifinishedthreadlistener.hxx>
#endif

#include <boost/weak_ptr.hpp>

/** interface class of the owner of a thread listener

    OD 2007-01-30 #i73788#

    @author OD
*/
class IThreadListenerOwner
{
    public:

        inline virtual ~IThreadListenerOwner()
        {
        };

        virtual boost::weak_ptr< IFinishedThreadListener > GetThreadListenerWeakRef() = 0;

        virtual void NotifyAboutFinishedThread( const oslInterlockedCount nThreadID ) = 0;

    protected:

        inline IThreadListenerOwner()
        {
        };
};
#endif
