/*************************************************************************
 *
 *  $RCSfile: backendaccess.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:46 $
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

#ifndef CONFIGMGR_BACKEND_BACKENDACCESS_HXX_
#define CONFIGMGR_BACKEND_BACKENDACCESS_HXX_

#ifndef CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX
#include "mergeddataprovider.hxx"
#endif // CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX

#ifndef CONFIGMGR_BACKEND_MERGEDCOMPONENTDATA_HXX
#include "mergedcomponentdata.hxx"
#endif // CONFIGMGR_BACKEND_MERGEDCOMPONENTDATA_HXX

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <drafts/com/sun/star/configuration/backend/XLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMA_HPP_
#include <drafts/com/sun/star/configuration/backend/XSchema.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMA_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <drafts/com/sun/star/configuration/backend/XBackend.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace configmgr { namespace backend {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
//namespace backenduno = css::configuration::backend ;
namespace backenduno = drafts::com::sun::star::configuration::backend ;

/**
  Implementation of IMergedDataProvider handling the access
  to the configuration data.
  */
class BackendAccess : public IMergedDataProvider {
    public :
        /**
          Constructor using an XBackend implementation and a
          service factory.

          @param xBackend   backend used for access to data
          @param xContext   uno context for instantiation of services
          */
        BackendAccess(  const uno::Reference<backenduno::XBackend>& xBackend,
                        const uno::Reference<uno::XComponentContext>& xContext) ;
        /** Destructor */
        ~BackendAccess(void) ;

        // IMergedDataProvider
        virtual ComponentResult getNodeData(const ComponentRequest& aRequest,
                                            ITemplateDataProvider* aTemplateProvider,
                                            INodeDataListener *aListener = NULL)
            CFG_UNO_THROW_ALL() ;
        virtual void removeRequestListener(INodeDataListener *aListener)
            CFG_NOTHROW() {}
        virtual void updateNodeData(const UpdateRequest& aUpdate)
            CFG_UNO_THROW_ALL() ;
        virtual NodeResult getDefaultData(const NodeRequest& aRequest)
            CFG_UNO_THROW_ALL() ;
        virtual TemplateResult getTemplateData(const TemplateRequest& aRequest)
            CFG_UNO_THROW_ALL() ;
        virtual bool isStrippingDefaults(void) CFG_NOTHROW() { return false ; }

    protected :

    private :
        /** Factory used for service invocation */
        uno::Reference<lang::XMultiServiceFactory> mFactory ;
        /** Backend being accessed */
        uno::Reference<backenduno::XBackend> mBackend ;

        /**
          Retrieves the schema and layers associated to a NodeRequest.

          @param aRequest   node request
          @param aSchema    reference to XSchema filled on return
          @param aLayers    sequence of XLayers filled on return
          */
        void getSchemaAndLayers(const NodeRequest& aRequest,
                uno::Reference<backenduno::XSchema>& aSchema,
                uno::Sequence<uno::Reference<backenduno::XLayer> >& aLayers) ;
} ;

} } // configmgr.backend

#endif // CONFIGMGR_BACKEND_BACKENDACCESS_HXX_
