/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localstratumbase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:07:34 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALSTRATUMBASE_HXX_
#define CONFIGMGR_LOCALBE_LOCALSTRATUMBASE_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDENTITIES_HPP_
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XUPDATABLELAYER_HPP_
#include <com/sun/star/configuration/backend/XUpdatableLayer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_CANNOTCONNECTEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif

namespace configmgr
{
    struct ServiceImplementationInfo;

    namespace localbe
    {
//------------------------------------------------------------------------------

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
//------------------------------------------------------------------------------


typedef cppu::WeakComponentImplHelper3<lang::XInitialization,
                                       backend::XBackendEntities,
                                       lang::XServiceInfo> LocalStratumImplBase ;

struct LocalStratumMutexHolder { osl::Mutex mMutex; };
//------------------------------------------------------------------------------
static const char kLocalDataSuffix[] = ".xcu";
//------------------------------------------------------------------------------

/**
  Helper for implementing the [Single|Multi]LayerStratum service for local file access.
  */
class LocalStratumBase : protected LocalStratumMutexHolder, public LocalStratumImplBase
{
protected :
    /**
      Service constructor from a service factory.

      @param xContext   component context
      */
    LocalStratumBase(const uno::Reference<uno::XComponentContext>& xContext) ;

    /** Destructor */
    ~LocalStratumBase() ;


public:
    // XInitialize
    virtual void SAL_CALL
        initialize( const uno::Sequence<uno::Any>& aParameters)
            throw (uno::RuntimeException, uno::Exception,
                   css::configuration::InvalidBootstrapFileException,
                   backend::CannotConnectException,
                   backend::BackendSetupException);


    // XBackendEntities
    virtual rtl::OUString SAL_CALL
        getOwnerEntity(  )
            throw (uno::RuntimeException);

    virtual rtl::OUString SAL_CALL
        getAdminEntity(  )
            throw (uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        supportsEntity( const rtl::OUString& aEntity )
            throw (backend::BackendAccessException, uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        isEqualEntity( const rtl::OUString& aEntity, const rtl::OUString& aOtherEntity )
            throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException);

    // XServiceInfo
    virtual rtl::OUString SAL_CALL
        getImplementationName(  )
            throw (uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        supportsService( const rtl::OUString& aServiceName )
            throw (uno::RuntimeException) ;

    virtual uno::Sequence<rtl::OUString> SAL_CALL
        getSupportedServiceNames(  )
            throw (uno::RuntimeException) ;

protected:
    rtl::OUString const & getBaseUrl() const
    { return mStrataDataUrl; }

    static rtl::OUString getDataSuffix()
    { return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(kLocalDataSuffix)); }
//------------------------------------------------------------------------------
    /// raise a NoSupportException for an attempt to update this layer
    void failReadonly();

    // helper for implementing the stratum getLayer[s] function[s]
    uno::Reference<backend::XLayer> SAL_CALL
        getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
            throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException);

    /**
      Builds a LocalFileLayer object given a subpath
      Since the LocalFileLayer implements the various
      interfaces a layer can be accessed as, a few methods
      need one.

      @param aSubpath   Subpath
      @return   local file layer
      @throws com::sun::star::lang::IllegalArgumentException
              if the subpath is invalid.
      */
    uno::Reference<backend::XLayer> createReadonlyFileLayer(const rtl::OUString& aSubpath)
        throw (lang::IllegalArgumentException) ;

    /**
      Builds a LocalFileLayer object given a subpath.
      Since the LocalFileLayer implements the various
      interfaces a layer can be accessed as, a few methods
      need one.

      @param aSubpath   subpath
      @return   local file layer
      @throws com::sun::star::lang::IllegalArgumentException
              if the subpath is invalid.
      */
    uno::Reference<backend::XUpdatableLayer> createUpdatableFileLayer(const rtl::OUString& aSubpath)
        throw (lang::IllegalArgumentException) ;

    /**
      Tells if a file is more recent than a given date.
      The date is formatted YYYYMMDDhhmmssZ.

      @param aSubpath       relative URL of the component to check
      @param aTimestamp     timestamp to check against
      @return   sal_True if the file is more recent, sal_False otherwise
      */
    sal_Bool isMoreRecent(const rtl::OUString& aSubpath,
                          const rtl::OUString& aTimestamp) ;

protected:
    /// Parses and adjusts the passed base URL
    virtual void adjustBaseURL(rtl::OUString& aBaseURL);

private:
    /**
      Retrieves the appropriate layer and sublayers base directories.

      @param aLayerUrl      layer base URL, filled on return
      @param aSubLayerUrl   sublayer base URL, filled on return
      */
    virtual void getLayerDirectories(rtl::OUString& aLayerUrl,
                                     rtl::OUString& aSubLayerUrl) const = 0;
    virtual const ServiceImplementationInfo * getServiceInfoData() const = 0;

private :
    /** Service factory */
    uno::Reference<lang::XMultiServiceFactory> mFactory ;
    /**
      Base of the strata data.
      */
    rtl::OUString mStrataDataUrl ;

} ;


} } // configmgr.localbe

#endif
