/*************************************************************************
 *
 *  $RCSfile: singlebackendadapter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cyrillem $ $Date: 2002-05-27 17:11:14 $
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

#ifndef ECOMP_LDAPBE_SINGLEBACKENDADAPTER_HXX_
#include "singlebackendadapter.hxx"
#endif // ECOMP_LDAPBE_SINGLEBACKENDADAPTER_HXX_

namespace ecomp { namespace ldapbe {

//==============================================================================

SingleBackendAdapter::SingleBackendAdapter(
        const uno::Reference<uno::XComponentContext>& aContext)
: BackendBase(mMutex), mContext(aContext) {
}
//------------------------------------------------------------------------------

SingleBackendAdapter::~SingleBackendAdapter(void) {}
//------------------------------------------------------------------------------

void SAL_CALL SingleBackendAdapter::initialize(
        const uno::Sequence<uno::Any>& aParameters) {
    static const rtl::OUString kSingleBackend(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.SingleBackend")) ;

    mBackend = uno::Reference<backend::XSingleBackend>::query(
        mContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                kSingleBackend, aParameters, mContext)) ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XSchema>
SAL_CALL SingleBackendAdapter::getComponentSchema(
        const rtl::OUString& aComponent) {
    return mBackend->getSchema(aComponent) ;
}
//------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backend::XLayer> >
SAL_CALL SingleBackendAdapter::listOwnLayers(const rtl::OUString& aComponent) {
    return listLayers(aComponent, mBackend->getOwnId()) ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XUpdateHandler>
SAL_CALL SingleBackendAdapter::getOwnUpdateHandler(
                                            const rtl::OUString& aComponent) {
    return getUpdateHandler(aComponent, mBackend->getOwnId()) ;
}
//------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backend::XLayer> >
SAL_CALL SingleBackendAdapter::listLayers(const rtl::OUString& aComponent,
                                          const rtl::OUString& aEntity) {
    return mBackend->getLayers(mBackend->listLayerIds(aComponent, aEntity),
                               rtl::OUString()) ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XUpdateHandler>
SAL_CALL SingleBackendAdapter::getUpdateHandler(const rtl::OUString& aComponent,
                                                const rtl::OUString& aEntity) {
    static const rtl::OUString kUpdateMerger(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.backend.UpdateMerger")) ;
    uno::Sequence<uno::Any> arguments(1) ;

    arguments [0] <<= mBackend->getUpdatableLayer(
                                        mBackend->getUpdateLayerId(aComponent,
                                                                   aEntity)) ;
    return uno::Reference<backend::XUpdateHandler>::query(
        mContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            kUpdateMerger, arguments, mContext)) ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kImplementationName(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.configuration.backend.SingleBackendAdapter")) ;

rtl::OUString SAL_CALL SingleBackendAdapter::getName(void) {
    return kImplementationName ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL SingleBackendAdapter::getImplementationName(void) {
    return getName() ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kBackendServiceName(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.backend.Backend")) ;

sal_Bool SAL_CALL SingleBackendAdapter::supportsService(
                                        const rtl::OUString& aServiceName) {
    return aServiceName.equals(kBackendServiceName) ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL SingleBackendAdapter::getServices(void) {
    return uno::Sequence<rtl::OUString>(
                                    &kBackendServiceName, 1) ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL SingleBackendAdapter::getSupportedServiceNames(void) {
    return getServices() ;
}
//------------------------------------------------------------------------------

} } // ecomp.ldapbe

