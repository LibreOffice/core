/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UpdateRequest.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:47:58 $
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

#include "precompiled_sd.hxx"

#include "UpdateRequest.hxx"

#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;

namespace sd { namespace framework {

UpdateRequest::UpdateRequest (void)
    throw()
    : UpdateRequestInterfaceBase(MutexOwner::maMutex)
{
}




UpdateRequest::~UpdateRequest (void) throw()
{
}




void SAL_CALL UpdateRequest::execute (const Reference<XConfiguration>& rxConfiguration)
    throw (RuntimeException)
{
    (void)rxConfiguration;
    // Do nothing here.  The configuration is updated when the request queue
    // becomes empty.
}




OUString SAL_CALL UpdateRequest::getName (void)
    throw (RuntimeException)
{
    return OUString::createFromAscii("UpdateRequest");
}




void SAL_CALL UpdateRequest::setName (const OUString& rsName)
    throw (RuntimeException)
{
    (void)rsName;
    // Ignored.
}

} } // end of namespace sd::framework

