/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "dbu_reghelper.hxx"
#include "dbustrings.hrc"
#include "UITools.hxx"

/** === begin UNO includes === **/
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
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/frame/XModule.hpp>
/** === end UNO includes === **/

#include <com/sun/star/sdbc/XDataSource.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/componentcontext.hxx>
#include <cppuhelper/implbase2.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <sal/macros.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using ::com::sun::star::sdbc::XDataSource;
using namespace dbaui;

class DBContentLoader : public ::cppu::WeakImplHelper2< XFrameLoader, XServiceInfo>
{
private:
    ::rtl::OUString                     m_aURL;
    Sequence< PropertyValue>            m_aArgs;
    Reference< XLoadEventListener >     m_xListener;
    Reference< XFrame >                 m_xFrame;
    Reference< XMultiServiceFactory >   m_xServiceFactory;
public:
    DBContentLoader(const Reference< XMultiServiceFactory >&);
    ~DBContentLoader();

    // XServiceInfo
    ::rtl::OUString                 SAL_CALL getImplementationName() throw(  );
    sal_Bool                        SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(  );
    Sequence< ::rtl::OUString >     SAL_CALL getSupportedServiceNames(void) throw(  );

    // static methods
    static ::rtl::OUString          getImplementationName_Static() throw(  )
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.DBContentLoader"));
    }
    static Sequence< ::rtl::OUString> getSupportedServiceNames_Static(void) throw(  );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    // XLoader
    virtual void SAL_CALL load( const Reference< XFrame > & _rFrame, const ::rtl::OUString& _rURL,
                                const Sequence< PropertyValue >& _rArgs,
                                const Reference< XLoadEventListener > & _rListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(void) throw();
};
DBG_NAME(DBContentLoader)

DBContentLoader::DBContentLoader(const Reference< XMultiServiceFactory >& _rxFactory)
    :m_xServiceFactory(_rxFactory)
{
    DBG_CTOR(DBContentLoader,NULL);

}
// -------------------------------------------------------------------------

DBContentLoader::~DBContentLoader()
{

    DBG_DTOR(DBContentLoader,NULL);
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_DBContentLoader()
{
    static ::dbaui::OMultiInstanceAutoRegistration< DBContentLoader > aAutoRegistration;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL DBContentLoader::Create( const Reference< XMultiServiceFactory >  & rSMgr )
{
    return *(new DBContentLoader(rSMgr));
}
// -------------------------------------------------------------------------
// XServiceInfo
::rtl::OUString SAL_CALL DBContentLoader::getImplementationName() throw(  )
{
    return getImplementationName_Static();
}
// -------------------------------------------------------------------------

// XServiceInfo
sal_Bool SAL_CALL DBContentLoader::supportsService(const ::rtl::OUString& ServiceName) throw(  )
{
    Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString * pBegin  = aSNL.getConstArray();
    const ::rtl::OUString * pEnd    = pBegin + aSNL.getLength();
    for( ; pBegin != pEnd; ++pBegin)
        if( *pBegin == ServiceName )
            return sal_True;
    return sal_False;
}
// -------------------------------------------------------------------------
// XServiceInfo
Sequence< ::rtl::OUString > SAL_CALL DBContentLoader::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
// ORegistryServiceManager_Static
Sequence< ::rtl::OUString > DBContentLoader::getSupportedServiceNames_Static(void) throw(  )
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.FrameLoader"));
    aSNS.getArray()[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.ContentLoader"));
    return aSNS;
}
// -------------------------------------------------------------------------
extern "C" void SAL_CALL writeDBLoaderInfo(void* pRegistryKey)
{
    Reference< XRegistryKey> xKey(reinterpret_cast< XRegistryKey*>(pRegistryKey));

    // register content loader for dispatch
    ::rtl::OUString aImpl(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += DBContentLoader::getImplementationName_Static();

    ::rtl::OUString aImpltwo = aImpl;
    aImpltwo += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/Loader"));
    Reference< XRegistryKey> xNewKey = xKey->createKey( aImpltwo );
    aImpltwo = aImpl;
    aImpltwo += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/Loader"));
    Reference< XRegistryKey >  xLoaderKey = xKey->createKey( aImpltwo );
    xNewKey = xLoaderKey->createKey( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Pattern")) );
    xNewKey->setAsciiValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".component:DB*")) );
}

// -----------------------------------------------------------------------
void SAL_CALL DBContentLoader::load(const Reference< XFrame > & rFrame, const ::rtl::OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener) throw(::com::sun::star::uno::RuntimeException)
{
    m_xFrame    = rFrame;
    m_xListener = rListener;
    m_aURL      = rURL;
    m_aArgs     = rArgs;

    ::comphelper::ComponentContext aContext( m_xServiceFactory );

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
        ServiceNameToImplName( URL_COMPONENT_FORMGRIDVIEW,          "org.openoffice.comp.dbu.OFormGridView"        ),
        ServiceNameToImplName( URL_COMPONENT_DATASOURCEBROWSER,     "org.openoffice.comp.dbu.ODatasourceBrowser"   ),
        ServiceNameToImplName( URL_COMPONENT_QUERYDESIGN,           "org.openoffice.comp.dbu.OQueryDesign"         ),
        ServiceNameToImplName( URL_COMPONENT_TABLEDESIGN,           "org.openoffice.comp.dbu.OTableDesign"         ),
        ServiceNameToImplName( URL_COMPONENT_RELATIONDESIGN,        "org.openoffice.comp.dbu.ORelationDesign"      ),
        ServiceNameToImplName( URL_COMPONENT_VIEWDESIGN,            "org.openoffice.comp.dbu.OViewDesign"          )
    };

    INetURLObject aParser( rURL );
    Reference< XController2 > xController;

    const ::rtl::OUString sComponentURL( aParser.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
    for ( size_t i=0; i < SAL_N_ELEMENTS( aImplementations ); ++i )
    {
        if ( sComponentURL.equalsAscii( aImplementations[i].pAsciiServiceName ) )
        {
            aContext.createComponent( aImplementations[i].pAsciiImplementationName, xController );
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
                                ||  ( sal_False == aLoadArgs.getOrDefault( (::rtl::OUString)PROPERTY_ENABLE_BROWSER, sal_True ) );

        if ( bDisableBrowser )
        {
            try
            {
                Reference< XModule > xModule( xController, UNO_QUERY_THROW );
                xModule->setIdentifier( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.TableDataView" ) ) );
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
            xController.set( m_xServiceFactory->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.ReportDesign" ) ) ), UNO_QUERY );
            if ( xController.is() )
            {
                xController->attachModel( xReportModel );
                xReportModel->connectController( xController.get() );
                xReportModel->setCurrentController( xController.get() );
            }
        }
    }

    sal_Bool bSuccess = xController.is();
    Reference< XModel > xDatabaseDocument;
    if ( bSuccess )
    {
        Reference< XDataSource > xDataSource    ( aLoadArgs.getOrDefault( "DataSource",       Reference< XDataSource >() ) );
        ::rtl::OUString          sDataSourceName( aLoadArgs.getOrDefault( "DataSourceName",   ::rtl::OUString()          ) );
        Reference< XConnection > xConnection    ( aLoadArgs.getOrDefault( "ActiveConnection", Reference< XConnection >() ) );
        if ( xDataSource.is() )
        {
            xDatabaseDocument.set( getDataSourceOrModel( xDataSource ), UNO_QUERY );
        }
        else if ( sDataSourceName.getLength() )
        {
            ::dbtools::SQLExceptionInfo aError;
            xDataSource.set( getDataSourceByName( sDataSourceName, NULL, m_xServiceFactory, &aError ) );
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
            PropertyValue aFrame(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Frame")),0,makeAny(rFrame),PropertyState_DIRECT_VALUE);
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

// -----------------------------------------------------------------------
void DBContentLoader::cancel(void) throw()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
