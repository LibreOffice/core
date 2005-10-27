/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: broadcasthelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-27 17:22:26 $
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

#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#define _COMPHELPER_BROADCASTHELPER_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

//... namespace comphelper .......................................................
namespace comphelper
{
//.........................................................................

    //==================================================================================
    //= OMutexAndBroadcastHelper - a class which holds a Mutex and a OBroadcastHelper;
    //=                 needed because when deriving from OPropertySetHelper,
    //=                 the OBroadcastHelper has to be initialized before
    //=                 the OPropertySetHelper
    //==================================================================================
    class OMutexAndBroadcastHelper
    {
    protected:
        ::osl::Mutex                m_aMutex;
        ::cppu::OBroadcastHelper    m_aBHelper;

    public:
        OMutexAndBroadcastHelper() : m_aBHelper( m_aMutex ) { }

        ::osl::Mutex&                   GetMutex()                  { return m_aMutex; }
        ::cppu::OBroadcastHelper&       GetBroadcastHelper()        { return m_aBHelper; }
        const ::cppu::OBroadcastHelper& GetBroadcastHelper() const  { return m_aBHelper; }

    };

    // base class for all classes who are derived from OPropertySet and from OComponent
    // @deprecated, you should use cppu::BaseMutex instead (cppuhelper/basemutex.hxx)

    class OBaseMutex
    {
    protected:
        mutable ::osl::Mutex m_aMutex;
    };
}
#endif // _COMPHELPER_BROADCASTHELPER_HXX_
