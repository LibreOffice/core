/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mutex.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2006-06-02 12:42:22 $
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

#include <vos/diagnose.hxx>
#include <vos/object.hxx>
#include <vos/mutex.hxx>

using namespace vos;

/////////////////////////////////////////////////////////////////////////////
//
//  class Mutex
//

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OMutex, vos), VOS_NAMESPACE(OMutex, vos), VOS_NAMESPACE(OObject, vos), 0);

IMutex& OMutex::getGlobalMutex()
{
    static OMutex theGlobalMutex;

    return theGlobalMutex;
}

OMutex::OMutex()
{
    m_Impl= osl_createMutex();
}

OMutex::~OMutex()
{
    osl_destroyMutex(m_Impl);
}

void OMutex::acquire()
{
    osl_acquireMutex(m_Impl);
}

sal_Bool OMutex::tryToAcquire()
{
    return osl_tryToAcquireMutex(m_Impl);
}

void OMutex::release()
{
    osl_releaseMutex(m_Impl);
}

