/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localmultistratum.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONFIGMGR_LOCALBE_LOCALMULTISTRATUM_HXX_
#define CONFIGMGR_LOCALBE_LOCALMULTISTRATUM_HXX_

#include "localstratumbase.hxx"
#include <com/sun/star/configuration/backend/XMultiLayerStratum.hpp>
#include <cppuhelper/implbase1.hxx>

namespace configmgr
{
    namespace localbe
    {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

/**
  Implements the MultiLayerStratum service for local file access.
  */
class LocalMultiStratum : public cppu::ImplInheritanceHelper1< LocalStratumBase, backend::XMultiLayerStratum >
{
public :
    /**
      Service constructor from a service factory.

      @param xContext   component context
      */
    LocalMultiStratum(const uno::Reference<uno::XComponentContext>& xContext) ;

    /** Destructor */
    ~LocalMultiStratum() ;


    // XMultiLayerStratum
    virtual uno::Sequence< rtl::OUString > SAL_CALL
        listLayerIds( const rtl::OUString& aComponent, const rtl::OUString& aEntity )
            throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException);

    virtual rtl::OUString SAL_CALL
        getUpdateLayerId( const rtl::OUString& aComponent, const rtl::OUString& aEntity )
            throw (backend::BackendAccessException, lang::NoSupportException,
                    lang::IllegalArgumentException, uno::RuntimeException);

    virtual uno::Reference< backend::XLayer > SAL_CALL
        getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
            throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException);

    virtual uno::Sequence< uno::Reference< backend::XLayer > > SAL_CALL
        getLayers( const uno::Sequence< rtl::OUString >& aLayerIds, const rtl::OUString& aTimestamp )
            throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException);

    virtual uno::Sequence< uno::Reference< backend::XLayer > > SAL_CALL
        getMultipleLayers( const uno::Sequence< rtl::OUString >& aLayerIds, const uno::Sequence< rtl::OUString >& aTimestamps )
            throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException);

    virtual uno::Reference< backend::XUpdatableLayer > SAL_CALL
        getUpdatableLayer( const rtl::OUString& aLayerId )
            throw (backend::BackendAccessException, lang::NoSupportException,
                    lang::IllegalArgumentException, uno::RuntimeException);

private:
    virtual void getLayerDirectories(rtl::OUString& aLayerUrl, rtl::OUString& aSubLayerUrl) const;
    virtual const ServiceImplementationInfo * getServiceInfoData() const;
} ;


} } // configmgr.localbe

#endif
