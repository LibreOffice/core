/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <flt_reghelper.hxx>
#include "xmlservices.hxx"
#include <stringconstants.hxx>
#include <strings.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/sdb/application/DatabaseObjectContainer.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <com/sun/star/frame/XLoadable.hpp>

#include <comphelper/documentconstants.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/types.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/moduleoptions.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/svapp.hxx>

using namespace ::ucbhelper;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::ui::dialogs;
using ::com::sun::star::awt::XWindow;
using ::com::sun::star::sdb::application::NamedDatabaseObject;

namespace dbaxml
{

class DBTypeDetection : public ::cppu::WeakImplHelper< XExtendedFilterDetection, XServiceInfo>
{
    const Reference< XComponentContext >  m_aContext;

public:
    explicit DBTypeDetection(const Reference< XComponentContext >&);

    // XServiceInfo
    OUString                        SAL_CALL getImplementationName() override;
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) override;
    Sequence< OUString >            SAL_CALL getSupportedServiceNames() override;

    // static methods
    static OUString                 getImplementationName_Static() throw(  )
    {
        return OUString("org.openoffice.comp.dbflt.DBTypeDetection");
    }
    static Sequence< OUString> getSupportedServiceNames_Static() throw(  );
    static css::uno::Reference< css::uno::XInterface >
            Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

    virtual OUString SAL_CALL detect( css::uno::Sequence< css::beans::PropertyValue >& Descriptor ) override;
};

DBTypeDetection::DBTypeDetection(const Reference< XComponentContext >& _rxContext)
    :m_aContext( _rxContext )
{
}

OUString SAL_CALL DBTypeDetection::detect( css::uno::Sequence< css::beans::PropertyValue >& Descriptor )
{
    try
    {
        ::comphelper::NamedValueCollection aMedia( Descriptor );
        bool bStreamFromDescr = false;
        OUString sURL = aMedia.getOrDefault( "URL", OUString() );

        Reference< XInputStream > xInStream( aMedia.getOrDefault( "InputStream",  Reference< XInputStream >() ) );
        Reference< XPropertySet > xStorageProperties;
        if ( xInStream.is() )
        {
            bStreamFromDescr = true;
            xStorageProperties.set( ::comphelper::OStorageHelper::GetStorageFromInputStream(
                xInStream, m_aContext ), UNO_QUERY );
        }
        else
        {
            OUString sSalvagedURL( aMedia.getOrDefault( "SalvagedFile", OUString() ) );

            OUString sFileLocation( sSalvagedURL.isEmpty() ? sURL : sSalvagedURL );
            if ( !sFileLocation.isEmpty() )
            {
                xStorageProperties.set( ::comphelper::OStorageHelper::GetStorageFromURL(
                    sFileLocation, ElementModes::READ, m_aContext ), UNO_QUERY );
            }
        }

        if ( xStorageProperties.is() )
        {
            OUString sMediaType;
            xStorageProperties->getPropertyValue( INFO_MEDIATYPE ) >>= sMediaType;
            if ( sMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII || sMediaType == MIMETYPE_VND_SUN_XML_BASE_ASCII )
            {
                if ( bStreamFromDescr && !sURL.startsWith( "private:stream" ) )
                {
                    // After fixing of the i88522 issue ( use the new file locking for database files ) the stream from the type detection can be used further
                    // for now the file should be reopened to have read/write access
                    aMedia.remove( OUString(  "InputStream" ) );
                    aMedia.remove( OUString(  "Stream" ) );
                    aMedia >>= Descriptor;
                    try
                    {
                        ::comphelper::disposeComponent(xStorageProperties);
                        if ( xInStream.is() )
                            xInStream->closeInput();
                    }
                    catch( Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }
                }

                return OUString("StarBase");
            }
            ::comphelper::disposeComponent(xStorageProperties);
        }
    } catch(Exception&){}
    return OUString();
}

Reference< XInterface > DBTypeDetection::Create( const Reference< XMultiServiceFactory >  & rSMgr )
{
    return *(new DBTypeDetection( comphelper::getComponentContext(rSMgr) ));
}

// XServiceInfo
OUString SAL_CALL DBTypeDetection::getImplementationName()
{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool SAL_CALL DBTypeDetection::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > SAL_CALL DBTypeDetection::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > DBTypeDetection::getSupportedServiceNames_Static() throw(  )
{
    Sequence<OUString> aSNS { "com.sun.star.document.ExtendedTypeDetection" };
    return aSNS;
}

} // namespace dbaxml

extern "C" void createRegistryInfo_DBTypeDetection()
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::DBTypeDetection > aAutoRegistration;
}

namespace dbaxml
{

class DBContentLoader : public ::cppu::WeakImplHelper< XFrameLoader, XServiceInfo>
{
private:
    const Reference< XComponentContext >  m_aContext;
    Reference< XFrameLoader >           m_xMySelf;
    OUString                            m_sCurrentURL;
    ImplSVEvent * m_nStartWizard;

    DECL_LINK( OnStartTableWizard, void*, void );
public:
    explicit DBContentLoader(const Reference< XComponentContext >&);

    // XServiceInfo
    OUString                        SAL_CALL getImplementationName() override;
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) override;
    Sequence< OUString >            SAL_CALL getSupportedServiceNames() override;

    // static methods
    static OUString                 getImplementationName_Static() throw(  )
    {
        return OUString("org.openoffice.comp.dbflt.DBContentLoader2");
    }
    static Sequence< OUString > getSupportedServiceNames_Static() throw(  );
    static css::uno::Reference< css::uno::XInterface >
            Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

    // XLoader
    virtual void SAL_CALL load( const Reference< XFrame > & _rFrame, const OUString& _rURL,
                                const Sequence< PropertyValue >& _rArgs,
                                const Reference< XLoadEventListener > & _rListener) override;
    virtual void SAL_CALL cancel() override;

private:
    bool impl_executeNewDatabaseWizard( Reference< XModel > const & _rxModel, bool& _bShouldStartTableWizard );
};


DBContentLoader::DBContentLoader(const Reference< XComponentContext >& _rxFactory)
    :m_aContext( _rxFactory )
    ,m_nStartWizard(nullptr)
{

}

Reference< XInterface > DBContentLoader::Create( const Reference< XMultiServiceFactory >  & rSMgr )
{
    return *(new DBContentLoader( comphelper::getComponentContext(rSMgr) ));
}

// XServiceInfo
OUString SAL_CALL DBContentLoader::getImplementationName()
{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool SAL_CALL DBContentLoader::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > SAL_CALL DBContentLoader::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > DBContentLoader::getSupportedServiceNames_Static() throw(  )
{
    Sequence<OUString> aSNS { "com.sun.star.frame.FrameLoader" };
    return aSNS;
}

namespace
{
    bool lcl_urlAllowsInteraction( const Reference<XComponentContext> & _rContext, const OUString& _rURL )
    {
        bool bDoesAllow = false;
        try
        {
            Reference< XURLTransformer > xTransformer( URLTransformer::create(_rContext) );
            URL aURL;
            aURL.Complete = _rURL;
            xTransformer->parseStrict( aURL );
            bDoesAllow = aURL.Arguments == "Interactive";
        }
        catch( const Exception& )
        {
            OSL_FAIL( "lcl_urlAllowsInteraction: caught an exception while analyzing the URL!" );
        }
        return bDoesAllow;
    }

    Reference< XWindow > lcl_getTopMostWindow( const Reference<XComponentContext> & _rxContext )
    {
        Reference< XWindow > xWindow;
        // get the top most window
        Reference < XDesktop2 > xDesktop = Desktop::create(_rxContext);
        Reference < XFrame > xActiveFrame = xDesktop->getActiveFrame();
        if ( xActiveFrame.is() )
        {
            xWindow = xActiveFrame->getContainerWindow();
            Reference<XFrame> xFrame = xActiveFrame;
            while ( xFrame.is() && !xFrame->isTop() )
                xFrame.set(xFrame->getCreator(),UNO_QUERY);

            if ( xFrame.is() )
                xWindow = xFrame->getContainerWindow();
        }
        return xWindow;
    }
}

bool DBContentLoader::impl_executeNewDatabaseWizard( Reference< XModel > const & _rxModel, bool& _bShouldStartTableWizard )
{
    Sequence<Any> aWizardArgs(comphelper::InitAnyPropertySequence(
    {
        {"ParentWindow", Any(lcl_getTopMostWindow( m_aContext ))},
        {"InitialSelection", Any(_rxModel)}
    }));

    // create the dialog
    Reference< XExecutableDialog > xAdminDialog( m_aContext->getServiceManager()->createInstanceWithArgumentsAndContext("com.sun.star.sdb.DatabaseWizardDialog", aWizardArgs, m_aContext), UNO_QUERY_THROW);

    // execute it
    if ( RET_OK != xAdminDialog->execute() )
        return false;

    Reference<XPropertySet> xProp(xAdminDialog,UNO_QUERY);
    bool bSuccess = false;
    xProp->getPropertyValue("OpenDatabase") >>= bSuccess;
    xProp->getPropertyValue("StartTableWizard") >>= _bShouldStartTableWizard;
    return bSuccess;
}

void SAL_CALL DBContentLoader::load(const Reference< XFrame > & rFrame, const OUString& _rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener)
{
    // first check if preview is true, if so return with out creating a controller. Preview is not supported
    ::comphelper::NamedValueCollection aMediaDesc( rArgs );
    bool bPreview = aMediaDesc.getOrDefault( "Preview", false );
    if ( bPreview )
    {
        if (rListener.is())
            rListener->loadCancelled(this);
        return;
    }

    Reference< XModel > xModel       = aMediaDesc.getOrDefault( "Model", Reference< XModel >() );
    OUString            sSalvagedURL = aMediaDesc.getOrDefault( "SalvagedFile", _rURL );

    bool bCreateNew = false;        // does the URL denote the private:factory URL?
    bool bStartTableWizard = false; // start the table wizard after everything was loaded successfully?

    bool bSuccess = true;

    // If there's no interaction handler in the media descriptor, put one.
    // By definition, loading via loadComponentFromURL (and thus via the content loader here)
    // is allowed to raise UI. To not burden every place inside the document with creating
    // a default handler, we simply ensure there is one.
    // If a handler is present in the media descriptor, even if it is NULL, we will
    // not touch it.
    if ( !aMediaDesc.has( "InteractionHandler" ) )
    {
       Reference< XInteractionHandler2 > xHandler( InteractionHandler::createWithParent(m_aContext, nullptr) );
       aMediaDesc.put( "InteractionHandler", xHandler );
    }

    // it's allowed to pass an existing document
    Reference< XOfficeDatabaseDocument > xExistentDBDoc;
    xModel.set( aMediaDesc.getOrDefault( "Model", xExistentDBDoc ), UNO_QUERY );
    aMediaDesc.remove( "Model" );

    // also, it's allowed to specify the type of view which should be created
    OUString sViewName = aMediaDesc.getOrDefault( "ViewName", OUString( "Default" ) );
    aMediaDesc.remove( "ViewName" );

    sal_Int32 nInitialSelection = -1;
    if ( !xModel.is() )
    {
        Reference< XDatabaseContext > xDatabaseContext( DatabaseContext::create(m_aContext) );

        OUString sFactoryName = SvtModuleOptions().GetFactoryEmptyDocumentURL(SvtModuleOptions::EFactory::DATABASE);
        bCreateNew = sFactoryName.match(_rURL);

        Reference< XDocumentDataSource > xDocumentDataSource;
        bool bNewAndInteractive = false;
        if ( bCreateNew )
        {
            bNewAndInteractive = lcl_urlAllowsInteraction( m_aContext, _rURL );
            xDocumentDataSource.set( xDatabaseContext->createInstance(), UNO_QUERY_THROW );
        }
        else
        {
            ::comphelper::NamedValueCollection aCreationArgs;
            aCreationArgs.put( OUString(INFO_POOLURL), sSalvagedURL );
            xDocumentDataSource.set( xDatabaseContext->createInstanceWithArguments( aCreationArgs.getWrappedNamedValues() ), UNO_QUERY_THROW );
        }

        xModel.set( xDocumentDataSource->getDatabaseDocument(), UNO_QUERY );

        if ( bCreateNew && xModel.is() )
        {
            if ( bNewAndInteractive )
            {
                bSuccess = impl_executeNewDatabaseWizard( xModel, bStartTableWizard );
            }
            else
            {
                try
                {
                    Reference< XLoadable > xLoad( xModel, UNO_QUERY_THROW );
                    xLoad->initNew();
                    bSuccess = true;
                }
                catch( const Exception& )
                {
                    bSuccess = false;
                }
            }

            // initially select the "Tables" category (will be done below)
            nInitialSelection = css::sdb::application::DatabaseObjectContainer::TABLES;
        }
    }

    if ( !xModel.is() )
    {
        if ( rListener.is() )
            rListener->loadCancelled(this);
        return;
    }

    if ( !bCreateNew )
    {
        // We need to XLoadable::load the document if it does not yet have an URL.
        // If it already *does* have an URL, then it was either passed in the arguments, or a previous incarnation
        // of that model existed before (which can happen if a model is closed, but an associated DataSource is kept
        // alive 'til loading the document again).
        bool bNeedLoad = xModel->getURL().isEmpty();
        try
        {
            aMediaDesc.put( "FileName", _rURL );
            Sequence< PropertyValue > aResource( aMediaDesc.getPropertyValues() );

            if ( bNeedLoad )
            {
                Reference< XLoadable > xLoad( xModel, UNO_QUERY_THROW );
                xLoad->load( aResource );
            }

            // always attach the resource, even if the document has not been freshly loaded
            xModel->attachResource( _rURL, aResource );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
            bSuccess = false;
        }
    }

    if ( bSuccess )
    {
        try
        {
            Reference< XModel2 > xModel2( xModel, UNO_QUERY_THROW );
            Reference< XController2 > xController( xModel2->createViewController( sViewName, Sequence< PropertyValue >(), rFrame ), UNO_QUERY_THROW );

            xController->attachModel( xModel );
            xModel->connectController( xController.get() );
            rFrame->setComponent( xController->getComponentWindow(), xController.get() );
            xController->attachFrame( rFrame );
            xModel->setCurrentController( xController.get() );

            bSuccess = true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
            bSuccess = false;
        }
    }

    if (bSuccess)
    {
        if ( rListener.is() )
            rListener->loadFinished(this);

        if ( nInitialSelection != -1 )
        {
            Reference< css::view::XSelectionSupplier >  xDocView( xModel->getCurrentController(), UNO_QUERY );
            if ( xDocView.is() )
            {
                NamedDatabaseObject aSelection;
                aSelection.Type = nInitialSelection;
                xDocView->select( makeAny( aSelection ) );
            }
        }

        if ( bStartTableWizard )
        {
            // reset the data of the previous async drop (if any)
            if ( m_nStartWizard )
                Application::RemoveUserEvent(m_nStartWizard);
            m_sCurrentURL = xModel->getURL();
            m_xMySelf = this;
            m_nStartWizard = Application::PostUserEvent(LINK(this, DBContentLoader, OnStartTableWizard));
        }
    }
    else
    {
        if ( rListener.is() )
            rListener->loadCancelled( this );
    }

    if ( !bSuccess )
        ::comphelper::disposeComponent(xModel);
}

void DBContentLoader::cancel()
{
}

IMPL_LINK_NOARG( DBContentLoader, OnStartTableWizard, void*, void )
{
    m_nStartWizard = nullptr;
    try
    {
        Sequence<Any> aWizArgs(comphelper::InitAnyPropertySequence(
        {
            {"DatabaseLocation", Any(m_sCurrentURL)}
        }));
        SolarMutexGuard aGuard;
        Reference< XJobExecutor > xTableWizard( m_aContext->getServiceManager()->createInstanceWithArgumentsAndContext("com.sun.star.wizards.table.CallTableWizard", aWizArgs, m_aContext), UNO_QUERY);
        if ( xTableWizard.is() )
            xTableWizard->trigger("start");
    }
    catch(const Exception&)
    {
        OSL_FAIL("caught an exception while starting the table wizard!");
    }
    m_xMySelf = nullptr;
}

}

extern "C" void createRegistryInfo_DBContentLoader2()
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::DBContentLoader > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
