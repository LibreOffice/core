/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lateinitthread.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:30:47 $
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

#ifndef __FILTER_CONFIG_LATEINITTHREAD_HXX_
#define __FILTER_CONFIG_LATEINITTHREAD_HXX_

//_______________________________________________
// includes

#include "filtercache.hxx"

#ifndef _SALHELPER_SINGLETONREF_HXX_
#include <salhelper/singletonref.hxx>
#endif

#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

//_______________________________________________

/** @short      implements a thread, which will update the
                global filter cache of an office, after its
                startup was finished.

    @descr      Its started by a LateInitListener instance ...

    @see        LateInitListener

    @attention  The filter cache will be blocked during this thrad runs!
 */
class LateInitThread : public ::osl::Thread
{
    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  initialize new instance of this class.
         */
        LateInitThread();

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~LateInitThread();

        //---------------------------------------

        /** @short  thread function.
         */
        virtual void SAL_CALL run();
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_LATEINITTHREAD_HXX_
