/*************************************************************************
 *
 *  $RCSfile: localsinglestratum.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 13:24:22 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_
#define CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDENTITIES_HPP_
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif // _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDENTITIES_HPP_
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_CANNOTCONNECTEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase4.hxx>
#endif // _CPPUHELPER_COMPBASE4_HXX_

namespace configmgr
{
    struct ServiceImplementationInfo;

    namespace localbe
    {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

typedef cppu::WeakComponentImplHelper4<backend::XSingleLayerStratum,
                                       lang::XInitialization,
                                       backend::XBackendEntities,
                                       lang::XServiceInfo> SingleBackendBase ;

/**
  Implements the SingleLayerStratum service for local file access.
  */
class LocalSingleStratumBase : public SingleBackendBase {
    public :
        /**
          Service constructor from a service factory.

          @param xContext   component context
          */
        LocalSingleStratumBase(const uno::Reference<uno::XComponentContext>& xContext) ;

        /** Destructor */
        ~LocalSingleStratumBase(void) ;


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

        // XSingleLayerStratum
        virtual uno::Reference<backend::XLayer> SAL_CALL
            getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
            getUpdatableLayer( const rtl::OUString& aLayerId )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        lang::NoSupportException,
                        uno::RuntimeException) ;

    protected:
        rtl::OUString const & getBaseUrl() const
        { return mStrataDataUrl; }

    protected:
        /**
          Retrieves the appropriate layer and sublayers base directories.

          @param aLayerUrl      layer base URL, filled on return
          @param aSubLayerUrl   sublayer base URL, filled on return
          */
        virtual void getLayerDirectories(rtl::OUString& aLayerUrl,
                                         rtl::OUString& aSubLayerUrl) const = 0;
    private:
        virtual const ServiceImplementationInfo * getServiceInfoData() const = 0;

    private :
        /** Service factory */
        uno::Reference<lang::XMultiServiceFactory> mFactory ;
        /** Mutex for resources protection */
        osl::Mutex mMutex ;
        /**
          Base of the strata data.
          */
        rtl::OUString mStrataDataUrl ;


        /**
          Builds a LocalFileLayer object given a layer id.
          Since the LocalFileLayer implements the various
          interfaces a layer can be accessed as, a few methods
          need one. This method handles the layer id mapping
          and the existence or not of sublayers.

          @param aLayerId   layer id
          @return   local file layer
          @throws com::sun::star::lang::IllegalArgumentException
                  if the layer id is invalid.
          */
        uno::Reference<backend::XLayer> createReadonlyFileLayer(const rtl::OUString& aLayerId)
            throw (lang::IllegalArgumentException) ;

        /**
          Builds a LocalFileLayer object given a layer id.
          Since the LocalFileLayer implements the various
          interfaces a layer can be accessed as, a few methods
          need one. This method handles the layer id mapping
          and the existence or not of sublayers.

          @param aLayerId   layer id
          @return   local file layer
          @throws com::sun::star::lang::IllegalArgumentException
                  if the layer id is invalid.
          */
        uno::Reference<backend::XUpdatableLayer> createUpdatableFileLayer(const rtl::OUString& aLayerId)
            throw (lang::IllegalArgumentException) ;

        /**
          Tells if a file is more recent than a given date.
          The date is formatted YYYYMMDDhhmmssZ.

          @param aComponent     URL of the component to check
          @param aTimestamp     timestamp to check against
          @return   sal_True if the file is more recent, sal_False otherwise
          */
        sal_Bool isMoreRecent(const rtl::OUString& aComponent,
                              const rtl::OUString& aTimestamp) ;

} ;

class LocalSingleStratum : public LocalSingleStratumBase
{
public:
    LocalSingleStratum(const uno::Reference<uno::XComponentContext>& xContext)
        : LocalSingleStratumBase(xContext)
        {};

private:
    virtual void getLayerDirectories(rtl::OUString& aLayerUrl, rtl::OUString& aSubLayerUrl) const;
    virtual const ServiceImplementationInfo * getServiceInfoData() const;
};

class LocalDataStratum : public LocalSingleStratumBase
{
public:
    LocalDataStratum(const uno::Reference<uno::XComponentContext>& xContext)
        : LocalSingleStratumBase(xContext)
        {};

private:
    virtual void getLayerDirectories(rtl::OUString& aLayerUrl, rtl::OUString& aSubLayerUrl) const;
    virtual const ServiceImplementationInfo * getServiceInfoData() const;
};

class LocalReadonlyStratum : public LocalSingleStratumBase
{
public:
    LocalReadonlyStratum(const uno::Reference<uno::XComponentContext>& xContext)
        : LocalSingleStratumBase(xContext)
        {};

    // XSingleLayerStratum - readonly implementation
    virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
        getUpdatableLayer( const rtl::OUString& aLayerId )
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    lang::NoSupportException,
                    uno::RuntimeException) ;

private:
    virtual void getLayerDirectories(rtl::OUString& aLayerUrl, rtl::OUString& aSubLayerUrl) const;
    virtual const ServiceImplementationInfo * getServiceInfoData() const;
};

class LocalResourceStratum : public LocalSingleStratumBase
{
public:
    LocalResourceStratum(const uno::Reference<uno::XComponentContext>& xContext)
        : LocalSingleStratumBase(xContext)
        {};

    // XSingleLayerStratum - readonly implementation
    virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
        getUpdatableLayer( const rtl::OUString& aLayerId )
            throw (backend::BackendAccessException,
                    lang::IllegalArgumentException,
                    lang::NoSupportException,
                    uno::RuntimeException) ;

private:
    virtual void getLayerDirectories(rtl::OUString& aLayerUrl, rtl::OUString& aSubLayerUrl) const;
    virtual const ServiceImplementationInfo * getServiceInfoData() const;
};



} } // configmgr.localbe

#endif // CONFIGMGR_LOCALBE_LOCALSINGLESTRATUM_HXX_
