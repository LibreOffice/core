/*************************************************************************
 *
 *  $RCSfile: backendaccess.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cyrillem $ $Date: 2002-07-03 13:38:50 $
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
#include "backendaccess.hxx"
#endif // CONFIGMGR_BACKEND_BACKENDACCESS_HXX_

#ifndef CONFIGMGR_BACKEND_LAYERMERGE_HXX
#include "layermerge.hxx"
#endif // CONFIGMGR_BACKEND_LAYERMERGE_HXX

#ifndef CONFIGMGR_BACKEND_SCHEMABUILDER_HXX
#include "schemabuilder.hxx"
#endif // CONFIGMGR_BACKEND_SCHEMABUILDER_HXX

#ifndef CONFIGMGR_BACKEND_UPDATEDISPATCHER_HXX
#include "updatedispatch.hxx"
#endif // CONFIGMGR_BACKEND_UPDATEDISPATCHER_HXX

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XCOMPOSITELAYER_HPP_
#include <drafts/com/sun/star/configuration/backend/XCompositeLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XCOMPOSITELAYER_HPP_

namespace configmgr { namespace backend {

//==============================================================================

//------------------------------------------------------------------------------

BackendAccess::BackendAccess(
        const uno::Reference<backenduno::XBackend>& xBackend,
        const uno::Reference<lang::XMultiServiceFactory>& xFactory)
    : mFactory(xFactory), mBackend(xBackend)
{
}
//------------------------------------------------------------------------------

BackendAccess::~BackendAccess(void) {}
//------------------------------------------------------------------------------

static rtl::OUString findBestLocale(
        const uno::Sequence<rtl::OUString>& aLocales,
        const rtl::OUString& aWanted) {
    rtl::OUString fallback ;

    for (sal_Int32 i = 0 ; i < aLocales.getLength() ; ++ i) {
        if (aLocales [i].equals(aWanted)) { return aWanted ; }
        if (fallback.getLength() == 0) {
            sal_Int32 compLength = aWanted.getLength() ;

            if (aLocales [i].getLength() < compLength) {
                compLength = aLocales [i].getLength() ;
            }
            if (aLocales [i].compareTo(aWanted, compLength) == 0) {
                fallback = aLocales [i] ;
            }
        }
    }
    return fallback ;
}
//------------------------------------------------------------------------------

static NodeResult merge(
        const uno::Reference<lang::XMultiServiceFactory>& aFactory,
        MergedComponentData& aData,
        const uno::Sequence<uno::Reference<backenduno::XLayer> >& aLayers,
        sal_Int32 aNbLayers,
        const rtl::OUString& aLocale,
        const AbsolutePath& aRootPath)
{
    for (sal_Int32 i = 0 ; i < aNbLayers ; ++ i) {
        //TODO Reactivate once the method is implemented
        //promoteToDefault(aData) ;
        aLayers [i]->readData(new LayerMergeHandler(aFactory, aData, aLocale)) ;
        uno::Reference<backenduno::XCompositeLayer> compositeLayer(
                aLayers [i], uno::UNO_QUERY) ;

        if (compositeLayer.is()) {
            rtl::OUString bestLocale = findBestLocale(
                    compositeLayer->listSubLayerIds(), aLocale) ;

            if (bestLocale.getLength() > 0) {
                compositeLayer->readSubLayerData(new LayerMergeHandler(
                            aFactory, aData, aLocale), bestLocale) ;
            }
        }
    }
    NodeInstance retCode(aData.extractSchemaTree(), aRootPath) ;

    return NodeResult(retCode) ;
}
//------------------------------------------------------------------------------

NodeResult BackendAccess::getNodeData(const NodeRequest& aRequest,
                                      INodeDataListener *aListener)
    CFG_UNO_THROW_ALL()
{
    SchemaBuilder *schemaBuilder = new backend::SchemaBuilder() ;
    uno::Reference<backenduno::XSchemaHandler> schemaHandler = schemaBuilder ;
    uno::Sequence<uno::Reference<backenduno::XLayer> > layers ;
    uno::Reference<backenduno::XSchema> schema ;

    getSchemaAndLayers(aRequest, schema, layers) ;
    schema->readTemplates(schemaHandler) ;
    return merge(mFactory, schemaBuilder->result(), layers, layers.getLength(),
                 aRequest.getOptions().getLocale(), aRequest.getPath()) ;
}
//------------------------------------------------------------------------------

void BackendAccess::updateNodeData(const UpdateRequest& aUpdate)
    CFG_UNO_THROW_ALL()
{
    rtl::OUString entity = aUpdate.getOptions().getEntity() ;
    rtl::OUString component =
                    aUpdate.getUpdateRoot().getModuleName().toString() ;
    uno::Reference<backenduno::XUpdateHandler> handler ;

    if (entity.getLength() == 0) {
        handler = mBackend->getOwnUpdateHandler(component) ;
    }
    else { handler = mBackend->getUpdateHandler(component, entity) ; }
    UpdateDispatcher dispatcher(handler, aUpdate.getOptions().getLocale()) ;

    dispatcher.dispatchUpdate(aUpdate.getUpdateRoot().location(),
                              *aUpdate.getUpdateData()) ;
}
//------------------------------------------------------------------------------

NodeResult BackendAccess::getDefaultData(const NodeRequest& aRequest)
    CFG_UNO_THROW_ALL()
{
    SchemaBuilder *schemaBuilder = new backend::SchemaBuilder() ;
    uno::Reference<backenduno::XSchemaHandler> schemaHandler = schemaBuilder ;
    uno::Sequence<uno::Reference<backenduno::XLayer> > layers ;
    uno::Reference<backenduno::XSchema> schema ;

    getSchemaAndLayers(aRequest, schema, layers) ;
    schema->readComponent(schemaHandler) ;
    return merge(mFactory, schemaBuilder->result(), layers,
                 layers.getLength() - 1, aRequest.getOptions().getLocale(),
                 aRequest.getPath()) ;
}
//------------------------------------------------------------------------------

TemplateResult BackendAccess::getTemplateData(const TemplateRequest& aRequest)
    CFG_UNO_THROW_ALL()
{
    uno::Reference<backenduno::XSchema> schema =
        mBackend->getComponentSchema(aRequest.getComponentName().toString()) ;
    SchemaBuilder *schemaBuilder = new SchemaBuilder() ;
    uno::Reference<backenduno::XSchemaHandler> handler = schemaBuilder ;

    schema->readTemplates(handler) ;
    backenduno::TemplateIdentifier templateId ;

    templateId.Name = aRequest.getTemplateName().toString() ;
    templateId.Component = aRequest.getComponentName().toString() ;
    TemplateInstance retCode(
            schemaBuilder->result().extractTemplateNode(templateId),
            aRequest.getTemplateName(), aRequest.getComponentName()) ;

    return TemplateResult(retCode) ;
}
//------------------------------------------------------------------------------

void BackendAccess::getSchemaAndLayers(const NodeRequest& aRequest,
        uno::Reference<backenduno::XSchema>& aSchema,
        uno::Sequence<uno::Reference<backenduno::XLayer> >& aLayers) {
    rtl::OUString component = aRequest.getPath().getModuleName().toString() ;
    rtl::OUString entity = aRequest.getOptions().getEntity() ;

    aSchema = mBackend->getComponentSchema(component) ;
    if (entity.getLength() == 0) {
        // Use own entity instead
        aLayers = mBackend->listOwnLayers(component) ;
    }
    else {
        aLayers = mBackend->listLayers(component, entity) ;
    }
}
//------------------------------------------------------------------------------

} } // configmgr.backend
