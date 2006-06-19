/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localstratumbase.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:25:59 $
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

#include "localstratumbase.hxx"

#ifndef CONFIGMGR_LOCALBE_LOCALFILEHELPER_HXX_
#include "localfilehelper.hxx"
#endif

#ifndef CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_
#include "localfilelayer.hxx"
#endif // CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_

#ifndef _CONFIGMGR_OSLSTREAM_HXX_
#include "oslstream.hxx"
#endif // _CONFIGMGR_OSLSTREAM_HXX_

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif // CONFIGMGR_SERVICEINFOHELPER_HXX_

#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif
#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NOSUPPORTEXCEPTION_HPP_
#include <com/sun/star/lang/NoSupportException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_INSUFFICIENTACCESSRIGHTSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#include <memory>

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalStratumBase::LocalStratumBase(const uno::Reference<uno::XComponentContext>& xContext)
: LocalStratumImplBase(mMutex)
, mFactory(xContext->getServiceManager(),uno::UNO_QUERY)
{
}
//------------------------------------------------------------------------------

LocalStratumBase::~LocalStratumBase()
{}

//------------------------------------------------------------------------------
void SAL_CALL LocalStratumBase::initialize(const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception,
           css::configuration::InvalidBootstrapFileException,
           backend::CannotConnectException,
           backend::BackendSetupException)
{



    if (aParameters.getLength() == 0)
    {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "No parameters provided to local Stratum")),
                *this, 0) ;
    }


    for (sal_Int32 i = 0 ; i < aParameters.getLength() ; ++ i)
    {
        if (aParameters [i] >>= mStrataDataUrl )
        { break ; }

    }

    // get modified base for special layer implementations (e.g. resources)
    this->adjustBaseURL(mStrataDataUrl);

    validateFileURL(mStrataDataUrl, *this);
    implEnsureAbsoluteURL(mStrataDataUrl);
    normalizeURL(mStrataDataUrl,*this, true);

    if(FileHelper::fileExists(mStrataDataUrl))
    {
        checkIfDirectory(mStrataDataUrl, *this);
    }
}

//------------------------------------------------------------------------------

void LocalStratumBase::adjustBaseURL(rtl::OUString& )
{
}
//------------------------------------------------------------------------------

sal_Bool LocalStratumBase::isMoreRecent(const rtl::OUString& aFileUrl,
                                              const rtl::OUString& aTimestamp) {
    rtl::OUString layerUrl ;
    rtl::OUString subLayerUrl ;

    getLayerDirectories(layerUrl, subLayerUrl) ;

    return layerUrl.getLength() == 0 ||
            !BasicLocalFileLayer::getTimestamp(layerUrl + aFileUrl).equals( aTimestamp);
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL
    LocalStratumBase::getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{

    if (aLayerId.getLength() == 0){
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "LocalStratum:getLayer - no LayerId specified")),
                *this, 0) ;
    }
    rtl::OUString const componentSubPath = layeridToPath(aLayerId) + getDataSuffix();
    if (!isMoreRecent(componentSubPath, aTimestamp)) { return NULL ; }

    return createReadonlyFileLayer(componentSubPath);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

OUString SAL_CALL LocalStratumBase::getOwnerEntity()
    throw (uno::RuntimeException)
{
    return mStrataDataUrl ;
}
//------------------------------------------------------------------------------

OUString SAL_CALL LocalStratumBase::getAdminEntity()
    throw (uno::RuntimeException)
{
    return OUString();
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalStratumBase::supportsEntity( const OUString& aEntity )
    throw (backend::BackendAccessException, uno::RuntimeException)
{
    if(mStrataDataUrl.getLength() == 0)
    {
        return false;
    }
    if (aEntity.getLength() == 0)
    {
        return false;
    }
    return isEqualEntity(mStrataDataUrl,aEntity);
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalStratumBase::isEqualEntity(const OUString& aEntity, const OUString& aOtherEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (aEntity.getLength() == 0)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid empty entity."));

        throw lang::IllegalArgumentException(sMsg, *this, 1);
    }
    if (aOtherEntity.getLength() == 0)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid empty entity."));

        throw lang::IllegalArgumentException(sMsg, *this, 2);
    }
    OUString aNormalizedEntity(aEntity);
    normalizeURL(aNormalizedEntity,*this);

    OUString aNormalizedOther(aOtherEntity);
    normalizeURL(aNormalizedOther,*this);

    return aNormalizedEntity == aNormalizedOther;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uno::Reference<backend::XLayer>
    LocalStratumBase::createReadonlyFileLayer(const rtl::OUString& aSubpath)
         throw (lang::IllegalArgumentException)
{
    rtl::OUString layerPath ;
    rtl::OUString subLayerPath ;

    getLayerDirectories(layerPath, subLayerPath) ;
    return createReadonlyLocalFileLayer(mFactory, layerPath, aSubpath, subLayerPath) ;
}
//------------------------------------------------------------------------------
uno::Reference<backend::XUpdatableLayer>
    LocalStratumBase::createUpdatableFileLayer(const rtl::OUString& aSubpath)
         throw (lang::IllegalArgumentException)
{
    rtl::OUString layerPath ;
    rtl::OUString subLayerPath ;

    getLayerDirectories(layerPath, subLayerPath) ;
    return createUpdatableLocalFileLayer(mFactory, layerPath, aSubpath, subLayerPath) ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalStratumBase::getImplementationName()
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(getServiceInfoData()).getImplementationName() ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalStratumBase::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    return  ServiceInfoHelper(getServiceInfoData()).supportsService(aServiceName);
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL LocalStratumBase::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(getServiceInfoData()).getSupportedServiceNames() ;
}

// ---------------------------------------------------------------------------------------

void LocalStratumBase::failReadonly()
{
    rtl::OUStringBuffer aMessage;
    aMessage.appendAscii("Configurations - ")
            .appendAscii("Cannot get update access to layer: ")
            .appendAscii("Local file-based stratum at ")
            .append(this->getBaseUrl())
            .appendAscii(" is readonly.");
    throw lang::NoSupportException(aMessage.makeStringAndClear(),*this);
}
//------------------------------------------------------------------------------

} } // configmgr.localsinglestratum
