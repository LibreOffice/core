/*************************************************************************
 *
 *  $RCSfile: updatesvc.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-05-27 10:35:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "updatesvc.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif

#include <drafts/com/sun/star/configuration/backend/XUpdatableLayer.hpp>
#include <drafts/com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace backenduno = drafts::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

AsciiServiceName const aUpdateMergerServices[] =
{
    "com.sun.star.configuration.backend.LayerUpdateMerger",
    0
};
const ServiceInfo aUpdateMergerSI =
{
    "com.sun.star.comp.configuration.backend.LayerUpdateMerger",
    aUpdateMergerServices
};
// -----------------------------------------------------------------------------

const ServiceInfo* getUpdateMergerServiceInfo()
{ return & aUpdateMergerSI; }
// -----------------------------------------------------------------------------

inline
ServiceInfoHelper UpdateService::getServiceInfo()
{
    return & aUpdateMergerSI;
}
// -----------------------------------------------------------------------------


UpdateService::UpdateService(CreationArg _xServiceFactory)
: m_xServiceFactory(_xServiceFactory)
{
    if (!m_xServiceFactory.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Update Merger: Unexpected NULL context"));
        throw uno::RuntimeException(sMessage,NULL);
    }
}
// -----------------------------------------------------------------------------

// XInitialization

void SAL_CALL
    UpdateService::initialize( const uno::Sequence< uno::Any >& aArguments )
        throw (uno::Exception, uno::RuntimeException)
{
    sal_Int16 const nCount = static_cast<sal_Int16>(aArguments.getLength());

    if (sal_Int32(nCount) != aArguments.getLength())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Too many arguments to initialize a Configuration Update Merger"));
        throw lang::IllegalArgumentException(sMessage,*this,0);
    }

    for (sal_Int16 i = 0; i < nCount; ++i)
    {
        uno::Reference< backenduno::XUpdatableLayer > xUpdLayer;
        if (aArguments[i] >>= xUpdLayer)
        {
            if (xUpdLayer.is())
            {
                m_xLayerWriter = xUpdLayer->getWriteHandler();
                m_xLayerReader = xUpdLayer.get();
            }
            else
            {
                m_xLayerWriter.clear();
                m_xLayerReader.clear();
            }
            continue;
        }

        if (aArguments[i] >>= m_xLayerReader)
            continue;

        if (aArguments[i] >>= m_xLayerWriter)
            continue;

        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Cannot use argument to initialize a Configuration Update Merger"
                                                        "- XLayer, XLayerHandler or XUpdatableLayer expected"));
        throw lang::IllegalArgumentException(sMessage,*this,i);
    }
}

// -----------------------------------------------------------------------------

// XServiceInfo

::rtl::OUString SAL_CALL
    UpdateService::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName( );
}
// -----------------------------------------------------------------------------


sal_Bool SAL_CALL
    UpdateService::supportsService( const ::rtl::OUString& ServiceName )
        throw (uno::RuntimeException)
{
    return getServiceInfo().supportsService( ServiceName );
}
// -----------------------------------------------------------------------------


uno::Sequence< ::rtl::OUString > SAL_CALL
    UpdateService::getSupportedServiceNames(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getSupportedServiceNames( );
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

