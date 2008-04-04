/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbloader.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:56:17 $
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
#include <com/sun/star/frame/XController.hpp>
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
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.DBContentLoader");
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
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.frame.FrameLoader");
    aSNS.getArray()[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.ContentLoader");
    return aSNS;
}
// -------------------------------------------------------------------------
extern "C" void SAL_CALL writeDBLoaderInfo(void* pRegistryKey)
{
    Reference< XRegistryKey> xKey(reinterpret_cast< XRegistryKey*>(pRegistryKey));

    // register content loader for dispatch
    ::rtl::OUString aImpl = ::rtl::OUString::createFromAscii("/");
    aImpl += DBContentLoader::getImplementationName_Static();

    ::rtl::OUString aImpltwo = aImpl;
    aImpltwo += ::rtl::OUString::createFromAscii("/UNO/Loader");
    Reference< XRegistryKey> xNewKey = xKey->createKey( aImpltwo );
    aImpltwo = aImpl;
    aImpltwo += ::rtl::OUString::createFromAscii("/Loader");
    Reference< XRegistryKey >  xLoaderKey = xKey->createKey( aImpltwo );
    xNewKey = xLoaderKey->createKey( ::rtl::OUString::createFromAscii("Pattern") );
    xNewKey->setAsciiValue( ::rtl::OUString::createFromAscii(".component:DB*") );
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
        ServiceNameToImplName( URL_COMPONENT_RELATIONDESIGN,        "org.openoffice.comp.dbu.ORelationDesign"      )
    };

    INetURLObject aParser( rURL );
    Reference< XController > xController;

    const ::rtl::OUString sComponentURL( aParser.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
    for ( size_t i=0; i < sizeof( aImplementations ) / sizeof( aImplementations[0] ); ++i )
    {
        if ( sComponentURL.equalsAscii( aImplementations[i].pAsciiServiceName ) )
        {
            aContext.createComponent( aImplementations[i].pAsciiImplementationName, xController );
            break;
        }
    }

    // if a data source browser is loaded without its tree pane, then we assume it to be a
    // table data view, effectively. In this case, we need to adjust the module identifier.
    // 2008-02-05 / i85879 / frank.schoenheit@sun.com
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
                xReportModel->connectController( xController );
                xReportModel->setCurrentController( xController );
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
            xDatabaseDocument.set( getDataSourceOrModel( getDataSourceByName_displayError( sDataSourceName, NULL, m_xServiceFactory, sal_False ) ), UNO_QUERY );
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
        ::vos::OGuard aGuard(Application::GetSolarMutex());
        try
        {
            Reference<XInitialization > xIni(xController,UNO_QUERY);
            PropertyValue aFrame(::rtl::OUString::createFromAscii("Frame"),0,makeAny(rFrame),PropertyState_DIRECT_VALUE);
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // assign controller and frame
    if ( bSuccess )
    {
        if ( xController.is() && rFrame.is() )
            xController->attachFrame(rFrame);

        if ( xDatabaseDocument.is() )
        {
            Reference< document::XEventListener > xGlobalDocEventBroadcaster(m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.GlobalEventBroadcaster"))),UNO_QUERY_THROW);
            document::EventObject aEvent( xDatabaseDocument, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnViewCreated")) );
            xGlobalDocEventBroadcaster->notifyEvent(aEvent);
            // TODO: is this correct? The newly created view is not a view for the database document, at least not in
            // the sense that its XController::getModel would return the database document
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

