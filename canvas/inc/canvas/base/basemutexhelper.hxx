/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basemutexhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:41:36 $
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

#ifndef INCLUDED_CANVAS_BASEMUTEXHELPER_HXX
#define INCLUDED_CANVAS_BASEMUTEXHELPER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif


/* Definition of the BaseMutexHelper class */

namespace canvas
{
    /** Base class, deriving from ::comphelper::OBaseMutex and
        initializing its own baseclass with m_aMutex.

        This is necessary to make the CanvasBase, GraphicDeviceBase,
        etc. classes freely combinable - letting them perform this
        initialization would prohibit deriving e.g. CanvasBase from
        GraphicDeviceBase.
     */
    template< class Base > class BaseMutexHelper : public Base
    {
    public:
        typedef Base BaseType;

        /** Construct BaseMutexHelper

            This method is the whole purpose of this template:
            initializing a base class with the provided m_aMutex
            member (the WeakComponentImplHelper templates need that,
            as they require the lifetime of the mutex to extend
            theirs).
         */
        BaseMutexHelper() :
            BaseType( m_aMutex )
        {
        }

protected:
        mutable ::osl::Mutex m_aMutex;
    };
}

#endif /* INCLUDED_CANVAS_BASEMUTEXHELPER_HXX */
