/*************************************************************************
 *
 *  $RCSfile: backendaccess.cxx,v $
 *
 *  $Revision: 1.15 $
 *  last change: $Author: rt $ $Date: 2003-10-06 14:44:32 $
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

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif // CONFIGMGR_MATCHLOCALE_HXX

#ifndef CONFIGMGR_BACKEND_LAYERMERGE_HXX
#include "layermerge.hxx"
#endif // CONFIGMGR_BACKEND_LAYERMERGE_HXX

#ifndef CONFIGMGR_BACKEND_SCHEMABUILDER_HXX
#include "schemabuilder.hxx"
#endif // CONFIGMGR_BACKEND_SCHEMABUILDER_HXX

#ifndef CONFIGMGR_BACKEND_UPDATEDISPATCHER_HXX
#include "updatedispatch.hxx"
#endif // CONFIGMGR_BACKEND_UPDATEDISPATCHER_HXX

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSchemaSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XCOMPOSITELAYER_HPP_
#include <com/sun/star/configuration/backend/XCompositeLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XCOMPOSITELAYER_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_MALFORMEDDATAEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_NOSUCHELEMENTEXCEPTION_HPP_
#include <com/sun/star/container/NoSuchElementException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDENTITIES_HPP_
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#define RTL_LOGFILE_OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())

#define OUSTR(txt)  OUString( RTL_CONSTASCII_USTRINGPARAM(txt) )


#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif // INCLUDED_VECTOR

#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

namespace configmgr { namespace backend {
//------------------------------------------------------------------------------

BackendAccess::BackendAccess(
        const uno::Reference<backenduno::XBackend>& xBackend,
        const uno::Reference<uno::XComponentContext>& xContext)
    : mFactory(xContext->getServiceManager(), uno::UNO_QUERY)
    , mBackend(xBackend)
    , mBinaryCache(xContext)
{
    OSL_ENSURE(mFactory.is(), "BackendAccess: Context has no ServiceManager (or it is missing an interface)");
    if (!mFactory.is())
        throw lang::NullPointerException(OUString::createFromAscii("BackendAccess: Context has no ServiceManager (or it is missing an interface)"), NULL);
    if (!xBackend.is())
        throw lang::NullPointerException(OUSTR("Configuration: Trying to create backend access without backend"),NULL);
    if (!uno::Reference<backenduno::XSchemaSupplier>::query(xBackend).is())
        throw lang::NullPointerException(OUSTR("Configuration: No backend for schemas available"),NULL);

    //Create Binary Cache
    uno::Reference<backenduno::XBackendEntities> xBackendEntities = uno::Reference<backenduno::XBackendEntities>( mBackend, uno::UNO_QUERY) ;
    OSL_ENSURE(xBackendEntities.is(),"Backend does not provide entity information");

    if ( xBackendEntities.is() ) mBinaryCache.setOwnerEntity(xBackendEntities->getOwnerEntity());
}
//------------------------------------------------------------------------------

BackendAccess::~BackendAccess() {}
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

void BackendAccess::merge(
        MergedComponentData& aData,
        const uno::Reference<backenduno::XLayer> * pLayers,
        sal_Int32 aNumLayers,
        RequestOptions const & aOptions,
        ITemplateDataProvider *aTemplateProvider)
    CFG_UNO_THROW_ALL()
{
    LayerMergeHandler * pMerger = new LayerMergeHandler(mFactory, aData, aTemplateProvider );
    uno::Reference<backenduno::XLayerHandler> xLayerMerger(pMerger);

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::merge()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "merging %d layers", int(aNumLayers) );

    OUString const aLocale = aOptions.getLocale();
    for (sal_Int32 i = 0 ; i < aNumLayers ; ++ i)
    {
        pMerger->prepareLayer() ;
        pLayers [i]->readData(xLayerMerger) ;

        uno::Reference<backenduno::XCompositeLayer> compositeLayer(
                pLayers [i], uno::UNO_QUERY) ;

        if (compositeLayer.is())
        {
            uno::Sequence<rtl::OUString> aSubLayerIds = compositeLayer->listSubLayerIds();
            if(localehelper::isAnyLocale(aLocale))
            {
                //Loop thru layers
                for (sal_Int32 j = 0; j < aSubLayerIds.getLength(); ++j)
                {
                    if(pMerger->prepareSublayer(aSubLayerIds[j]))
                    {
                        compositeLayer->readSubLayerData(xLayerMerger,aSubLayerIds[j]) ;
                    }
                }
            }
            else
            {
                rtl::OUString bestLocale = findBestLocale(aSubLayerIds, aLocale) ;

                if (pMerger->prepareSublayer(bestLocale) )
                {
                    compositeLayer->readSubLayerData(xLayerMerger, bestLocale) ;
                }
            }
        }
    }
}
//------------------------------------------------------------------------------

bool BackendAccess::readDefaultData( MergedComponentData & aComponentData,
                                        OUString const & aComponent,
                                        RequestOptions const & aOptions,
                                        bool bIncludeTemplates,
                                        const uno::Reference<backenduno::XLayer> * pLayers,
                                        sal_Int32 nNumLayers,
                                        ITemplateDataProvider *aTemplateProvider)
    CFG_UNO_THROW_ALL()
{
    RTL_LOGFILE_CONTEXT_AUTHOR(aLog1, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::readDefaultData()");

    bool bCacheHit = mBinaryCache.readComponentData(aComponentData, mFactory,
                                                    aComponent, aOptions.getEntity(), aOptions.getLocale(),
                                                    pLayers, nNumLayers, bIncludeTemplates);

    if (!bCacheHit)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog2, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::readDefaultData() - not in cache");

        {
            SchemaBuilder *schemaBuilder = new SchemaBuilder( aComponent, aComponentData, aTemplateProvider );
            uno::Reference<backenduno::XSchemaHandler> schemaHandler = schemaBuilder ;

            uno::Reference<backenduno::XSchema> schema = this->getSchema(aComponent);
            schema->readSchema(schemaHandler) ;
        }

        this->merge(aComponentData, pLayers, nNumLayers, aOptions, aTemplateProvider );
        promoteToDefault(aComponentData);

        if (mBinaryCache.isCacheEnabled(aOptions.getEntity()))
        {
            bool bWriteSuccess = mBinaryCache.writeComponentData( aComponentData, mFactory,
                                                                  aComponent, aOptions.getEntity(), aOptions.getLocale(),
                                                                  pLayers, nNumLayers );

            if (!bWriteSuccess)
            {
                OSL_TRACE("Configuration: Cache write failed: disabling binary cache");
                mBinaryCache.disableCache();
            }
        }
    }

    return aComponentData.hasSchema();
}
//------------------------------------------------------------------------------

ComponentResult BackendAccess::getNodeData(const ComponentRequest& aRequest,
                                           ITemplateDataProvider *_aTemplateProvider,
                                           INodeDataListener *aListener)
    CFG_UNO_THROW_ALL()
{
    rtl::OUString const component = aRequest.getComponentName().toString() ;
    ITemplateDataProvider * const aTemplateProvider = _aTemplateProvider ? _aTemplateProvider : this;

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::getNodeData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "request path: %s", RTL_LOGFILE_OU2A(component) );

    uno::Sequence<uno::Reference<backenduno::XLayer> > const layers =
        this->getLayers(component, aRequest.getOptions()) ;

    sal_Int32 const nNumUserLayers = 1;
    sal_Int32 const nNumDefaultLayers = layers.getLength() - nNumUserLayers;

    MergedComponentData aComponentData;

    if (!this->readDefaultData(aComponentData, component, aRequest.getOptions(), true,
                                layers.getConstArray(),nNumDefaultLayers,
                                aTemplateProvider))
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration: No data for request. Component \"");
        sMessage.append(component);
        sMessage.appendAscii("\" contains no data. ");

        throw com::sun::star::container::NoSuchElementException(sMessage.makeStringAndClear(),mBackend);
    }

    //Merge User layer
    merge(aComponentData,
            layers.getConstArray()+nNumDefaultLayers, nNumUserLayers,
            aRequest.getOptions(), aTemplateProvider );


    ComponentInstance retCode(aComponentData.extractSchemaTree(),
                              aComponentData.extractTemplatesTree(),
                              aRequest.getComponentName()) ;

    return ComponentResult(retCode) ;
}
//------------------------------------------------------------------------------

void BackendAccess::updateNodeData(const UpdateRequest& aUpdate)
    CFG_UNO_THROW_ALL()
{
    rtl::OUString entity = aUpdate.getOptions().getEntity() ;
    rtl::OUString component =
                    aUpdate.getUpdateRoot().getModuleName().toString() ;
    uno::Reference<backenduno::XUpdateHandler> handler ;

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::updateNodeData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "updating component: %s", RTL_LOGFILE_OU2A(component) );

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
    rtl::OUString const component = aRequest.getPath().getModuleName().toString() ;

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::getDefaultData()");
    RTL_LOGFILE_CONTEXT_TRACE1(aLog, "request path: %s", RTL_LOGFILE_OU2A(aRequest.getPath().toString()) );

    uno::Sequence<uno::Reference<backenduno::XLayer> > const layers =
        this->getLayers(component, aRequest.getOptions()) ;

    sal_Int32 const nNumUserLayers = 1;
    sal_Int32 const nNumDefaultLayers = layers.getLength() - nNumUserLayers;

    MergedComponentData aComponentData;

    if (!this->readDefaultData(aComponentData, component, aRequest.getOptions(), false,
                                layers.getConstArray(),nNumDefaultLayers,
                                this))
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration: No data for request. Component \"");
        sMessage.append(component);
        sMessage.appendAscii("\" contains no default data. ");

        throw com::sun::star::container::NoSuchElementException(sMessage.makeStringAndClear(),mBackend);
    }

    std::auto_ptr<ISubtree> aResultTree = aComponentData.extractSchemaTree();

    AbsolutePath aPath = aRequest.getPath();
    if( aPath.begin() != aPath.end())
    {
        for(AbsolutePath::Iterator it=aPath.begin()+1,endIt=aPath.end();it!=endIt; ++it)
        {
            std::auto_ptr<INode> aChild=aResultTree->removeChild(it->getName().toString());
            if(aChild.get()== NULL)
            {
                rtl::OUStringBuffer sMessage;
                sMessage.appendAscii("Configuration: No data for request. Element \"");
                sMessage.append(aPath.toString());
                sMessage.appendAscii("\" does not exist in the default data. ");

                throw com::sun::star::container::NoSuchElementException(sMessage.makeStringAndClear(),mBackend);
            }

            ISubtree *pChildAsSubtree = aChild->asISubtree();
            if(pChildAsSubtree == NULL)
            {
                OUString sMsg = OUString::createFromAscii("BackendAccess::getDefaultData - Node Expected, Found Property: ").concat(it->getName().toString());
                throw MalformedDataException(sMsg, mBackend, uno::Any());
            }
            aResultTree.reset(pChildAsSubtree);
            aChild.release();
        }
    }

    NodeInstance retCode(aResultTree, aRequest.getPath()) ;
    return NodeResult(retCode) ;
}
//------------------------------------------------------------------------------

TemplateResult BackendAccess::getTemplateData(const TemplateRequest& aRequest)
    CFG_UNO_THROW_ALL()
{
    rtl::OUString component = aRequest.getComponentName().toString();

    RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::backend::BackendAccess", "jb99855", "configmgr: BackendAccess::getTemplateData()");
    RTL_LOGFILE_CONTEXT_TRACE2(aLog, "requested template: %s/%s",
                                    RTL_LOGFILE_OU2A(aRequest.getComponentName().toString()) ,
                                    aRequest.isComponentRequest() ? "*" : RTL_LOGFILE_OU2A(aRequest.getComponentName().toString()) );

    MergedComponentData aComponentData;

    {
        SchemaBuilder *schemaBuilder = new SchemaBuilder( component, aComponentData ) ;
        uno::Reference<backenduno::XSchemaHandler> handler = schemaBuilder ;

        uno::Reference<backenduno::XSchema> schema = this->getSchema(component);

        schema->readTemplates(handler) ;
    }

    TemplateInstance::Data aResultData;
    if (aRequest.isComponentRequest())
    {
        aResultData.reset( aComponentData.extractTemplatesTree().release() );
    }
    else
    {
        backenduno::TemplateIdentifier templateId ;
        templateId.Name = aRequest.getTemplateName().toString() ;
        templateId.Component = aRequest.getComponentName().toString() ;

        aResultData = aComponentData.extractTemplateNode(templateId.Name);
    }

    TemplateInstance retCode(aResultData,aRequest.getTemplateName(), aRequest.getComponentName()) ;
    return TemplateResult(retCode) ;
}
//------------------------------------------------------------------------------

uno::Reference< backenduno::XSchema > BackendAccess::getSchema(const OUString& aComponent)
{
    uno::Reference< backenduno::XSchemaSupplier > xSchemaBackend(mBackend, uno::UNO_QUERY);
    OSL_ASSERT(xSchemaBackend.is());

    uno::Reference< backenduno::XSchema > xSchema = xSchemaBackend->getComponentSchema(aComponent) ;
    if (!xSchema.is())
    {
        rtl::OUStringBuffer sMessage;
        sMessage.appendAscii("Configuration: No data for request. Component \"");
        sMessage.append(aComponent);
        sMessage.appendAscii("\" is unknown. [No schema available]");

        throw com::sun::star::container::NoSuchElementException(sMessage.makeStringAndClear(),xSchemaBackend);
    }

    return xSchema;
}
//------------------------------------------------------------------------------

uno::Sequence< uno::Reference<backenduno::XLayer> > BackendAccess::getLayers(const OUString& aComponent,const RequestOptions& aOptions)
{
    rtl::OUString aEntity = aOptions.getEntity() ;

    if (aEntity.getLength() == 0)
    {
        // Use own entity instead
        return mBackend->listOwnLayers(aComponent) ;
    }
    else
    {
        return mBackend->listLayers(aComponent, aEntity) ;
    }
}
//------------------------------------------------------------------------------

} } // configmgr.backend
