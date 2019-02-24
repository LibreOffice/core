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

#include <dbu_reghelper.hxx>
#include <stringconstants.hxx>
#include <strings.hxx>
#include <uiservices.hxx>
#include <UITools.hxx>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/sdb/ReportDesign.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XModule.hpp>

#include <com/sun/star/sdbc/XDataSource.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace dbaui;

class DBContentLoader : public ::cppu::WeakImplHelper< XFrameLoader, XServiceInfo>
{
private:
    Sequence< PropertyValue>            m_aArgs;
    Reference< XLoadEventListener >     m_xListener;
    Reference< XComponentContext >      m_xContext;
public:
    explicit DBContentLoader(const Reference< XComponentContext >&);

    // XServiceInfo
    OUString                 SAL_CALL getImplementationName() override;
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) override;
    Sequence< OUString >     SAL_CALL getSupportedServiceNames() override;

    // static methods
    static OUString          getImplementationName_Static() throw(  )
    {
        return OUString("org.openoffice.comp.dbu.DBContentLoader");
    }
    static Sequence< OUString> getSupportedServiceNames_Static() throw(  );
    static css::uno::Reference< css::uno::XInterface >
            Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

    // XLoader
    virtual void SAL_CALL load( const Reference< XFrame > & _rFrame, const OUString& _rURL,
                                const Sequence< PropertyValue >& _rArgs,
                                const Reference< XLoadEventListener > & _rListener) override;
    virtual void SAL_CALL cancel() override;
};


DBContentLoader::DBContentLoader(const Reference< XComponentContext >& _rxContext)
    :m_xContext(_rxContext)
{

}

extern "C" void createRegistryInfo_DBContentLoader()
{
    static ::dbaui::OMultiInstanceAutoRegistration< DBContentLoader > aAutoRegistration;
}

Reference< XInterface > DBContentLoader::Create( const Reference< XMultiServiceFactory >  & rSMgr )
{
    return *(new DBContentLoader(comphelper::getComponentContext(rSMgr)));
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
    Sequence< OUString > aSNS( 2 );
    aSNS[0] = "com.sun.star.frame.FrameLoader";
    aSNS[1] = "com.sun.star.sdb.ContentLoader";
    return aSNS;
}

void SAL_CALL DBContentLoader::load(const Reference< XFrame > & rFrame, const OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener)
{
    m_xListener = rListener;
    m_aArgs     = rArgs;

    static const struct ServiceNameToImplName
    {
        const char*     pAsciiServiceName;
        const char*     pAsciiImplementationName;
        ServiceNameToImplName( const char* _pService, const char* _pImpl )
            :pAsciiServiceName( _pService )
            ,pAsciiImplementationName( _pImpl )
        {
        }
    } aImplementations[] = {
        ServiceNameToImplName( URL_COMPONENT_FORMGRIDVIEW,      "org.openoffice.comp.dbu.OFormGridView"        ),
        ServiceNameToImplName( URL_COMPONENT_DATASOURCEBROWSER, "org.openoffice.comp.dbu.ODatasourceBrowser"   ),
        ServiceNameToImplName( URL_COMPONENT_QUERYDESIGN,       "org.openoffice.comp.dbu.OQueryDesign"         ),
        ServiceNameToImplName( URL_COMPONENT_TABLEDESIGN,       "org.openoffice.comp.dbu.OTableDesign"         ),
        ServiceNameToImplName( URL_COMPONENT_RELATIONDESIGN,    "org.openoffice.comp.dbu.ORelationDesign"      ),
        ServiceNameToImplName( URL_COMPONENT_VIEWDESIGN,        "org.openoffice.comp.dbu.OViewDesign"          )
    };

    INetURLObject aParser( rURL );
    Reference< XController2 > xController;

    const OUString sComponentURL( aParser.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ) );
    for (const ServiceNameToImplName& aImplementation : aImplementations)
    {
        if ( sComponentURL.equalsAscii( aImplementation.pAsciiServiceName ) )
        {
            xController.set( m_xContext->getServiceManager()->
               createInstanceWithContext( OUString::createFromAscii( aImplementation.pAsciiImplementationName ), m_xContext), UNO_QUERY_THROW );
            break;
        }
    }

    // if a data source browser is loaded without its tree pane, then we assume it to be a
    // table data view, effectively. In this case, we need to adjust the module identifier.
    // #i85879#
    ::comphelper::NamedValueCollection aLoadArgs( rArgs );

    if  ( sComponentURL == URL_COMPONENT_DATASOURCEBROWSER )
    {
        bool bDisableBrowser =  !aLoadArgs.getOrDefault( "ShowTreeViewButton", true )   // compatibility name
                                ||  !aLoadArgs.getOrDefault( PROPERTY_ENABLE_BROWSER, true );

        if ( bDisableBrowser )
        {
            try
            {
                Reference< XModule > xModule( xController, UNO_QUERY_THROW );
                xModule->setIdentifier( "com.sun.star.sdb.TableDataView" );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
    }

    if ( sComponentURL == URL_COMPONENT_REPORTDESIGN )
    {
        bool bPreview = aLoadArgs.getOrDefault( "Preview", false );
        if ( bPreview )
        {   // report designs cannot be previewed
            if ( rListener.is() )
                rListener->loadCancelled( this );
            return;
        }
        Reference< XModel > xReportModel( aLoadArgs.getOrDefault( "Model", Reference< XModel >() ) );
        if ( xReportModel.is() )
        {
            xController.set( ReportDesign::create( m_xContext ) );
            xController->attachModel( xReportModel );
            xReportModel->connectController( xController.get() );
            xReportModel->setCurrentController( xController.get() );
        }
    }

    bool bSuccess = xController.is();
    Reference< XModel > xDatabaseDocument;
    if ( bSuccess )
    {
        Reference< XDataSource > xDataSource    ( aLoadArgs.getOrDefault( "DataSource",       Reference< XDataSource >() ) );
        OUString          sDataSourceName( aLoadArgs.getOrDefault( "DataSourceName",   OUString()          ) );
        Reference< XConnection > xConnection    ( aLoadArgs.getOrDefault( "ActiveConnection", Reference< XConnection >() ) );
        if ( xDataSource.is() )
        {
            xDatabaseDocument.set( getDataSourceOrModel( xDataSource ), UNO_QUERY );
        }
        else if ( !sDataSourceName.isEmpty() )
        {
            ::dbtools::SQLExceptionInfo aError;
            xDataSource.set( getDataSourceByName( sDataSourceName, nullptr, m_xContext, &aError ) );
            xDatabaseDocument.set( getDataSourceOrModel( xDataSource ), UNO_QUERY );
        }
        else if ( xConnection.is() )
        {
            Reference< XChild > xAsChild( xConnection, UNO_QUERY );
            if ( xAsChild.is() )
            {
                OSL_ENSURE( Reference< XDataSource >( xAsChild->getParent(), UNO_QUERY ).is(),
                    "DBContentLoader::load: a connection whose parent is no data source?" );
                xDatabaseDocument.set( getDataSourceOrModel( xAsChild->getParent() ), UNO_QUERY );
            }
        }

        // init controller
        SolarMutexGuard aGuard;
        try
        {
            Reference<XInitialization > xIni(xController,UNO_QUERY);
            PropertyValue aFrame("Frame",0,makeAny(rFrame),PropertyState_DIRECT_VALUE);
            Sequence< Any > aInitArgs(m_aArgs.getLength()+1);

            Any* pBegin = aInitArgs.getArray();
            Any* pEnd   = pBegin + aInitArgs.getLength();
            *pBegin <<= aFrame;
            const PropertyValue* pIter      = m_aArgs.getConstArray();
            for(++pBegin;pBegin != pEnd;++pBegin,++pIter)
            {
                *pBegin <<= *pIter;
            }

            xIni->initialize(aInitArgs);
        }
        catch(const Exception&)
        {
            // Does this need to be shown to the user?
            bSuccess = false;
            try
            {
                ::comphelper::disposeComponent( xController );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
    }

    // assign controller and frame
    if ( bSuccess )
    {
        if ( xController.is() && rFrame.is() )
        {
            rFrame->setComponent( xController->getComponentWindow(), xController.get() );
            xController->attachFrame(rFrame);
        }

        if ( rListener.is() )
            rListener->loadFinished( this );
    }
    else
        if ( rListener.is() )
            rListener->loadCancelled( this );
}

void DBContentLoader::cancel()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
