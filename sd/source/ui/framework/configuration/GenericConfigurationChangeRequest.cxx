/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GenericConfigurationChangeRequest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:30:25 $
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

#include "GenericConfigurationChangeRequest.hxx"

#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;

namespace sd { namespace framework {

GenericConfigurationChangeRequest::GenericConfigurationChangeRequest (
    const Reference<XResourceId>& rxResourceId,
    const Mode eMode) throw(::com::sun::star::lang::IllegalArgumentException)
    : GenericConfigurationChangeRequestInterfaceBase(MutexOwner::maMutex),
      mxResourceId(rxResourceId),
      meMode(eMode)
{
    if ( ! rxResourceId.is() || rxResourceId->getResourceURL().getLength()==0)
        throw ::com::sun::star::lang::IllegalArgumentException();
}




GenericConfigurationChangeRequest::~GenericConfigurationChangeRequest (void) throw()
{
}




void SAL_CALL GenericConfigurationChangeRequest::execute (
    const Reference<XConfiguration>& rxConfiguration)
    throw (RuntimeException)
{
    if (rxConfiguration.is())
    {
        switch (meMode)
        {
            case Activation:
                rxConfiguration->addResource(mxResourceId);
                break;

            case Deactivation:
                rxConfiguration->removeResource(mxResourceId);
                break;
        }
    }
}




OUString SAL_CALL GenericConfigurationChangeRequest::getName (void)
    throw (RuntimeException)
{
    return OUString::createFromAscii("GenericConfigurationChangeRequest ")
        + OUString::createFromAscii(meMode==Activation ? "activate " : "deactivate ")
        + FrameworkHelper::ResourceIdToString(mxResourceId);
}




void SAL_CALL GenericConfigurationChangeRequest::setName (const OUString& rsName)
    throw (RuntimeException)
{
    (void)rsName;
    // Ignored.
}

} } // end of namespace sd::framework

