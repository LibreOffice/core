/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localmultistratum.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:05:01 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALMULTISTRATUM_HXX_
#define CONFIGMGR_LOCALBE_LOCALMULTISTRATUM_HXX_

#ifndef CONFIGMGR_LOCALBE_LOCALSTRATUMBASE_HXX_
#include "localstratumbase.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XMULTILAYERSTRATUM_HPP_
#include <com/sun/star/configuration/backend/XMultiLayerStratum.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

namespace configmgr
{
    namespace localbe
    {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

typedef cppu::ImplInheritanceHelper1< LocalStratumBase,
                                       backend::XMultiLayerStratum
                                     > MultiStratumImplBase ;

/**
  Implements the MultiLayerStratum service for local file access.
  */
class LocalMultiStratum : public MultiStratumImplBase
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
