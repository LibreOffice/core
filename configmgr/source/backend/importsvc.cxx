/*************************************************************************
 *
 *  $RCSfile: importsvc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:47 $
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

#include "importsvc.hxx"

#ifndef CONFIGMGR_BACKEND_IMPORTMERGEHANDLER_HXX
#include "importmergehandler.hxx"
#endif

#ifndef CONFIGMGR_BACKENDFACTORY_HXX_
#include "backendfactory.hxx"
#endif

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
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
// -----------------------------------------------------------------------------

AsciiServiceName const aMergeImporterServices[] =
{
    "com.sun.star.configuration.backend.MergeImporter",
    0,
    "com.sun.star.configuration.backend.Importer",
    0
};
const ServiceImplementationInfo aMergeImporterSI =
{
    "com.sun.star.comp.configuration.backend.MergeImporter",
    aMergeImporterServices, aMergeImporterServices + 2
};
// -----------------------------------------------------------------------------

const ServiceRegistrationInfo* getMergeImportServiceInfo()
{ return getRegistrationInfo(& aMergeImporterSI); }
// -----------------------------------------------------------------------------

MergeImportService::MergeImportService(CreationArg _xContext)
: ImportService(_xContext, &aMergeImporterSI)
{
}
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateMergeImporter
( CreationContext const& xContext )
{
    return * new MergeImportService( xContext );
}
// -----------------------------------------------------------------------------

MergeImportService::InputHandler MergeImportService::createImportHandler(Backend const & xBackend, OUString const & aEntity)
{
    if (!xBackend.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to import without a backend. No default backend could be created") );
        throw lang::NullPointerException(sMessage,*this);
    }

    InputHandler aHandler( new ImportMergeHandler(xBackend, ImportMergeHandler::merge, aEntity) );

    return aHandler;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

AsciiServiceName const aCopyImporterServices[] =
{
    "com.sun.star.configuration.backend.CopyImporter",
    0,
    "com.sun.star.configuration.backend.Importer",
    0
};
const ServiceImplementationInfo aCopyImporterSI =
{
    "com.sun.star.comp.configuration.backend.CopyImporter",
    aCopyImporterServices, aCopyImporterServices + 2
};
// -----------------------------------------------------------------------------

const ServiceRegistrationInfo* getCopyImportServiceInfo()
{ return getRegistrationInfo(& aCopyImporterSI); }
// -----------------------------------------------------------------------------

CopyImportService::CopyImportService(CreationArg _xContext)
: ImportService(_xContext, &aCopyImporterSI)
{
}
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateCopyImporter
( CreationContext const& xContext )
{
    return * new CopyImportService( xContext );
}
// -----------------------------------------------------------------------------

CopyImportService::InputHandler CopyImportService::createImportHandler(Backend const & xBackend, OUString const & aEntity)
{
    if (!xBackend.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to import without a backend. No default backend could be created") );
        throw lang::NullPointerException(sMessage,*this);
    }

    ImportMergeHandler::Mode aMode = m_bOverwrite ? ImportMergeHandler::copy : ImportMergeHandler::no_overwrite;
    InputHandler aHandler( new ImportMergeHandler(xBackend,aMode,aEntity) );

    return aHandler;
}
// -----------------------------------------------------------------------------

sal_Bool CopyImportService::setImplementationProperty(OUString const & aName, uno::Any const & aValue)
{
    if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Overwrite")))
    {
        return (aValue >>= m_bOverwrite);
    }

    return ImportService::setImplementationProperty(aName,aValue);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

ImportService::ImportService(CreationArg _xContext, ServiceInfoHelper const & aSvcInfo )
: m_aMutex()
, m_xContext(_xContext)
, m_xDestinationBackend()
, m_aServiceInfo(aSvcInfo)
{
    if (!m_xContext.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Importer: Unexpected NULL context"));
        throw lang::NullPointerException(sMessage,NULL);
    }
}
// -----------------------------------------------------------------------------

ImportService::~ImportService()
{}
// -----------------------------------------------------------------------------

ImportService::Backend ImportService::createDefaultBackend() const
{
    return BackendFactory::instance( m_xContext ).getUnoBackend();
}
// -----------------------------------------------------------------------------

sal_Bool ImportService::setImplementationProperty(OUString const & , uno::Any const & )
{
    return false;
}
// -----------------------------------------------------------------------------

// XInitialize

void SAL_CALL
    ImportService::initialize( const uno::Sequence< uno::Any >& aArguments )
        throw (uno::Exception, uno::RuntimeException)
{
    sal_Int16 const nCount = static_cast<sal_Int16>(aArguments.getLength());

    if (sal_Int32(nCount) != aArguments.getLength())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Too many arguments to initialize a Configuration Importer"));
        throw lang::IllegalArgumentException(sMessage,*this,0);
    }

    for (sal_Int16 i = 0; i < nCount; ++i)
    {
        beans::NamedValue aExtraArg;
        if (aArguments[i] >>= aExtraArg)
        {
            OSL_VERIFY( setImplementationProperty(aExtraArg.Name, aExtraArg.Value) );

            continue;
        }

        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Cannot use argument to initialize a Configuration Importer"
                                                        "- NamedValue expected"));
        throw lang::IllegalArgumentException(sMessage,*this,i+1);
    }
}

// -----------------------------------------------------------------------------

// XImportLayer

uno::Reference< backenduno::XBackend > SAL_CALL
    ImportService::getTargetBackend(  )
        throw (uno::RuntimeException)
{
    uno::Reference< backenduno::XBackend > xRet;
    {
        osl::MutexGuard aGuard(m_aMutex);
        xRet = m_xDestinationBackend;
    }

    if (!xRet.is())
    {
        xRet = createDefaultBackend();

        osl::MutexGuard aGuard(m_aMutex);
        if (!m_xDestinationBackend.is())
            m_xDestinationBackend = xRet;
    }
    return xRet;
}
// -----------------------------------------------------------------------------

void SAL_CALL
    ImportService::setTargetBackend( const uno::Reference< backenduno::XBackend >& aBackend )
        throw (lang::NullPointerException, uno::RuntimeException)
{
    if (!aBackend.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to set a NULL backend") );
        throw lang::NullPointerException(sMessage,*this);
    }

    osl::MutexGuard aGuard(m_aMutex);
    m_xDestinationBackend = aBackend;
}
// -----------------------------------------------------------------------------

void SAL_CALL
    ImportService::importLayer( const uno::Reference< backenduno::XLayer >& aLayer )
        throw ( lang::WrappedTargetException, lang::IllegalArgumentException,
                lang::NullPointerException, uno::RuntimeException)
{
    if (!aLayer.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to import a NULL layer") );
        throw lang::NullPointerException(sMessage,*this);
    }

    InputHandler aInputHandler = createImportHandler( getTargetBackend() );
    aLayer->readData( aInputHandler );

}
// -----------------------------------------------------------------------------

void SAL_CALL
    ImportService::importLayerForEntity( const uno::Reference< backenduno::XLayer >& aLayer, const OUString& aEntity )
        throw ( lang::WrappedTargetException, lang::IllegalArgumentException,
                lang::NullPointerException, uno::RuntimeException)
{
    if (!aLayer.is())
    {
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to import a NULL layer") );
        throw lang::NullPointerException(sMessage,*this);
    }

    InputHandler aInputHandler = createImportHandler( getTargetBackend(), aEntity );
    aLayer->readData( aInputHandler );
}
// -----------------------------------------------------------------------------

// XServiceInfo

OUString SAL_CALL
    ImportService::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName( );
}
// -----------------------------------------------------------------------------


sal_Bool SAL_CALL
    ImportService::supportsService( const OUString& ServiceName )
        throw (uno::RuntimeException)
{
    return getServiceInfo().supportsService( ServiceName );
}
// -----------------------------------------------------------------------------


uno::Sequence< ::rtl::OUString > SAL_CALL
    ImportService::getSupportedServiceNames(  )
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

