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

#include <strings.hxx>
#include <UITools.hxx>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdb/ReportDesign.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XModule.hpp>

#include <com/sun/star/sdbc/XDataSource.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/urlobj.hxx>
#include <unotools/fcm.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbaui;

namespace {

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
    sal_Bool                 SAL_CALL supportsService(const OUString& ServiceName) override;
    Sequence< OUString >     SAL_CALL getSupportedServiceNames() override;

    // XLoader
    virtual void SAL_CALL load( const Reference< XFrame > & _rFrame, const OUString& _rURL,
                                const Sequence< PropertyValue >& _rArgs,
                                const Reference< XLoadEventListener > & _rListener) override;
    virtual void SAL_CALL cancel() override;
};

}

DBContentLoader::DBContentLoader(const Reference< XComponentContext >& _rxContext)
    :m_xContext(_rxContext)
{

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_openoffice_comp_dbu_DBContentLoader_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new DBContentLoader(context));
}

// XServiceInfo
OUString SAL_CALL DBContentLoader::getImplementationName()
{
    return "org.openoffice.comp.dbu.DBContentLoader";
}

// XServiceInfo
sal_Bool SAL_CALL DBContentLoader::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > SAL_CALL DBContentLoader::getSupportedServiceNames()
{
    return { "com.sun.star.frame.FrameLoader", "com.sun.star.sdb.ContentLoader" };
}

void SAL_CALL DBContentLoader::load(const Reference< XFrame > & rFrame, const OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener)
{
    m_xListener = rListener;
    m_aArgs     = rArgs;

    static constexpr struct ServiceNameToImplName
    {
        OUString     sServiceName;
        const char*     pAsciiImplementationName;
    } aImplementations[] = {
        { URL_COMPONENT_FORMGRIDVIEW,      "org.openoffice.comp.dbu.OFormGridView"        },
        { URL_COMPONENT_DATASOURCEBROWSER, "org.openoffice.comp.dbu.ODatasourceBrowser"   },
        { URL_COMPONENT_QUERYDESIGN,       "org.openoffice.comp.dbu.OQueryDesign"         },
        { URL_COMPONENT_TABLEDESIGN,       "org.openoffice.comp.dbu.OTableDesign"         },
        { URL_COMPONENT_RELATIONDESIGN,    "org.openoffice.comp.dbu.ORelationDesign"      },
        { URL_COMPONENT_VIEWDESIGN,        "org.openoffice.comp.dbu.OViewDesign"          }
    };

    INetURLObject aParser( rURL );
    Reference< XController2 > xController;

    const OUString sComponentURL( aParser.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ) );
    for (const ServiceNameToImplName& aImplementation : aImplementations)
    {
        if ( sComponentURL == aImplementation.sServiceName )
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
            utl::ConnectModelController(xReportModel, xController);
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
            PropertyValue aFrame("Frame",0,Any(rFrame),PropertyState_DIRECT_VALUE);
            Sequence< Any > aInitArgs(m_aArgs.getLength()+1);

            Any* pBegin = aInitArgs.getArray();
            *pBegin <<= aFrame;
            std::transform(m_aArgs.begin(), m_aArgs.end(), ++pBegin,
                           [](auto& val) { return Any(val); });

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
            rFrame->setComponent( xController->getComponentWindow(), xController );
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
