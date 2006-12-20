/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_thread.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:23:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_THREAD_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_THREAD_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include <cstddef>
#include <new>

#ifndef _THREAD_HXX_
#include "osl/thread.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include "salhelper/simplereferenceobject.hxx"
#endif

/// @HTML

namespace dp_gui {

/**
   A safe encapsulation of <code>osl::Thread</code>.
*/
class Thread: public salhelper::SimpleReferenceObject, private osl::Thread {
public:
    Thread();

    /**
       Launch the thread.

       <p>This function must be called at most once.</p>
    */
    void launch();

    static void * operator new(std::size_t size) throw (std::bad_alloc);

    static void operator delete(void * p) throw ();

protected:
    virtual ~Thread();

    /**
       The main function executed by the thread.

       <p>Any exceptions terminate the thread and are effectively ignored.</p>
    */
    virtual void execute() = 0;

private:
    Thread(Thread &); // not defined
    void operator =(Thread &); // not defined

    virtual void SAL_CALL run();

    virtual void SAL_CALL onTerminated();
};

}

#endif
