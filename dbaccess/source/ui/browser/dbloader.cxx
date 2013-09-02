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

#include "dbu_reghelper.hxx"
#include "dbustrings.hrc"
#include "UITools.hxx"

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
#include <cppuhelper/implbase2.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
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

class DBContentLoader : public ::cppu::WeakImplHelper2< XFrameLoader, XServiceInfo>
{
private:
    OUString                     m_aURL;
    Sequence< PropertyValue>            m_aArgs;
    Reference< XLoadEventListener >     m_xListener;
    Reference< XFrame >                 m_xFrame;
    Reference< XComponentContext >      m_xContext;
public:
    DBContentLoader(const Reference< XComponentContext >&);
    ~DBContentLoader();

    // XServiceInfo
    OUString                 SAL_CALL getImplementationName() throw(  );
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw(  );
    Sequence< OUString >     SAL_CALL getSupportedServiceNames(void) throw(  );

    // static methods
    static OUString          getImplementationName_Static() throw(  )
    {
        return OUString("org.openoffice.comp.dbu.DBContentLoader");
    }
    static Sequence< OUString> getSupportedServiceNames_Static(void) throw(  );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    // XLoader
    virtual void SAL_CALL load( const Reference< XFrame > & _rFrame, const OUString& _rURL,
                                const Sequence< PropertyValue >& _rArgs,
                                const Reference< XLoadEventListener > & _rListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(void) throw();
};

DBG_NAME(DBContentLoader)

DBContentLoader::DBContentLoader(const Reference< XComponentContext >& _rxContext)
    :m_xContext(_rxContext)
{
    DBG_CTOR(DBContentLoader,NULL);

}

DBContentLoader::~DBContentLoader()
{

    DBG_DTOR(DBContentLoader,NULL);
}

extern "C" void SAL_CALL createRegistryInfo_DBContentLoader()
{
    static ::dbaui::OMultiInstanceAutoRegistration< DBContentLoader > aAutoRegistration;
}

Reference< XInterface > SAL_CALL DBContentLoader::Create( const Reference< XMultiServiceFactory >  & rSMgr )
{
    return *(new DBContentLoader(comphelper::getComponentContext(rSMgr)));
}

// XServiceInfo
OUString SAL_CALL DBContentLoader::getImplementationName() throw(  )
{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool SAL_CALL DBContentLoader::supportsService(const OUString& ServiceName) throw(  )
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pBegin  = aSNL.getConstArray();
    const OUString * pEnd    = pBegin + aSNL.getLength();
    for( ; pBegin != pEnd; ++pBegin)
        if( *pBegin == ServiceName )
            return sal_True;
    return sal_False;
}

// XServiceInfo
Sequence< OUString > SAL_CALL DBContentLoader::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > DBContentLoader::getSupportedServiceNames_Static(void) throw(  )
{
    Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[0] = OUString("com.sun.star.frame.FrameLoader");
    aSNS.getArray()[1] = OUString("com.sun.star.sdb.ContentLoader");
    return aSNS;
}

void SAL_CALL DBContentLoader::load(const Reference< XFrame > & rFrame, const OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener) throw(::com::sun::star::uno::RuntimeException)
{
    m_xFrame    = rFrame;
    m_xListener = rListener;
    m_aURL      = rURL;
    m_aArgs     = rArgs;

    struct ServiceNameToImplName
    {
        const sal_Char*     pAsciiServiceName;
        const sal_Char*     pAsciiImplementationName;
        ServiceNameToImplName( const sal_Char* _pService, const sal_Char* _pImpl )
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

    const OUString sComponentURL( aParser.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
    for ( size_t i=0; i < sizeof( aImplementations ) / sizeof( aImplementations[0] ); ++i )
    {
        if ( sComponentURL.equalsAscii( aImplementations[i].pAsciiServiceName ) )
        {
            xController.set( m_xContext->getServiceManager()->
               createInstanceWithContext( OUString::createFromAscii( aImplementations[i].pAsciiImplementationName ), m_xContext), UNO_QUERY_THROW );
            break;
        }
    }

    // if a data source browser is loaded without its tree pane, then we assume it to be a
    // table data view, effectively. In this case, we need to adjust the module identifier.
    // #i85879#
    ::comphelper::NamedValueCollection aLoadArgs( rArgs );

    if  ( sComponentURL == URL_COMPONENT_DATASOURCEBROWSER )
    {
        sal_Bool bDisableBrowser =  ( sal_False == aLoadArgs.getOrDefault( "ShowTreeViewButton", sal_True ) )   // compatibility name
                                ||  ( sal_False == aLoadArgs.getOrDefault( (OUString)PROPERTY_ENABLE_BROWSER, sal_True ) );

        if ( bDisableBrowser )
        {
            try
            {
                Reference< XModule > xModule( xController, UNO_QUERY_THROW );
                xModule->setIdentifier( OUString( "com.sun.star.sdb.TableDataView" ) );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    if ( sComponentURL == URL_COMPONENT_REPORTDESIGN )
    {
        sal_Bool bPreview = aLoadArgs.getOrDefault( "Preview", sal_False );
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

    sal_Bool bSuccess = xController.is();
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
            xDataSource.set( getDataSourceByName( sDataSourceName, NULL, m_xContext, &aError ) );
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
            PropertyValue aFrame(OUString("Frame"),0,makeAny(rFrame),PropertyState_DIRECT_VALUE);
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
                DBG_UNHANDLED_EXCEPTION();
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

void DBContentLoader::cancel(void) throw()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
