/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbloader2.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 08:12:23 $
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

//#ifndef _SFXFRAME_HXX
//#include <sfx2/frame.hxx>
//#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDOCUMENTDATASOURCE_HPP_
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADEVENTLISTENER_HPP_
#include <com/sun/star/frame/XLoadEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXTENDEDFILTERDETECTION_HPP_
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _FLT_REGHELPER_HXX_
#include "flt_reghelper.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include <comphelper/documentconstants.hxx>
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen wg. SfxMedium
#include <sfx2/docfile.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_STORAGEHELPER_HXX
#include <comphelper/storagehelper.hxx>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

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
namespace css = ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
namespace css = ::com::sun::star;
using ::com::sun::star::awt::XWindow;

// -------------------------------------------------------------------------
namespace dbaxml
{

class DBTypeDetection : public ::cppu::WeakImplHelper2< XExtendedFilterDetection, XServiceInfo>
{
    Reference< XMultiServiceFactory > m_xServiceFactory;
public:
    DBTypeDetection(const Reference< XMultiServiceFactory >&);

    // XServiceInfo
    ::rtl::OUString                 SAL_CALL getImplementationName() throw(  );
    sal_Bool                        SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(  );
    Sequence< ::rtl::OUString >     SAL_CALL getSupportedServiceNames(void) throw(  );

    // static methods
    static ::rtl::OUString          getImplementationName_Static() throw(  )
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbflt.DBTypeDetection");
    }
    static Sequence< ::rtl::OUString> getSupportedServiceNames_Static(void) throw(  );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    virtual ::rtl::OUString SAL_CALL detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (::com::sun::star::uno::RuntimeException);
};
// -------------------------------------------------------------------------
DBTypeDetection::DBTypeDetection(const Reference< XMultiServiceFactory >& _rxFactory)
: m_xServiceFactory(_rxFactory)
{
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL DBTypeDetection::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        ::comphelper::SequenceAsHashMap aTemp(Descriptor);
        Reference< XInputStream > xInStream = aTemp.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream")), Reference< XInputStream >() );

        Reference<XPropertySet> xProp;
        if ( xInStream.is() )
        {
            xProp.set(::comphelper::OStorageHelper::GetStorageFromInputStream(xInStream,m_xServiceFactory),UNO_QUERY);
        }
        else
        {
            ::rtl::OUString sTemp = aTemp.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL")),::rtl::OUString());

            if ( sTemp.getLength() )
            {
                INetURLObject aURL(sTemp);
                xProp.set(::comphelper::OStorageHelper::GetStorageFromURL(sTemp,ElementModes::READ,m_xServiceFactory),UNO_QUERY);
            }
        }

        if ( xProp.is() )
        {
            ::rtl::OUString sMediaType;
            xProp->getPropertyValue( INFO_MEDIATYPE ) >>= sMediaType;
            if ( sMediaType.equalsAscii(MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII) )
                return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBase"));
            ::comphelper::disposeComponent(xProp);
        }
    } catch(Exception&){}
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL DBTypeDetection::Create( const Reference< XMultiServiceFactory >  & rSMgr )
{
    return *(new DBTypeDetection(rSMgr));
}
// -------------------------------------------------------------------------
// XServiceInfo
::rtl::OUString SAL_CALL DBTypeDetection::getImplementationName() throw(  )
{
    return getImplementationName_Static();
}
// -------------------------------------------------------------------------

// XServiceInfo
sal_Bool SAL_CALL DBTypeDetection::supportsService(const ::rtl::OUString& ServiceName) throw(  )
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
Sequence< ::rtl::OUString > SAL_CALL DBTypeDetection::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
// ORegistryServiceManager_Static
Sequence< ::rtl::OUString > DBTypeDetection::getSupportedServiceNames_Static(void) throw(  )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExtendedTypeDetection");
    return aSNS;
}
// -------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_DBTypeDetection()
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::DBTypeDetection > aAutoRegistration;
}
// -----------------------------------------------------------------------------

class DBContentLoader : public ::cppu::WeakImplHelper2< XFrameLoader, XServiceInfo>
{
private:
    Reference< XMultiServiceFactory >   m_xServiceFactory;
    Reference< XFrameLoader >           m_xMySelf;
    ::rtl::OUString                     m_sCurrentURL;
    sal_Int32                           m_nStartWizard;

    DECL_LINK( OnStartTableWizard, void* );
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
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbflt.DBContentLoader2");
    }
    static Sequence< ::rtl::OUString> getSupportedServiceNames_Static(void) throw(  );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    // XLoader
    virtual void SAL_CALL load( const Reference< XFrame > & _rFrame, const ::rtl::OUString& _rURL,
                                const Sequence< PropertyValue >& _rArgs,
                                const Reference< XLoadEventListener > & _rListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(void) throw();

private:
    sal_Bool impl_executeNewDatabaseWizard( Reference< XModel >& _rxModel, sal_Bool& _bShouldStartTableWizard );
};
DBG_NAME(DBContentLoader)

DBContentLoader::DBContentLoader(const Reference< XMultiServiceFactory >& _rxFactory)
    :m_xServiceFactory(_rxFactory)
    ,m_nStartWizard(0)
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
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.frame.FrameLoader");
    return aSNS;
}

// -----------------------------------------------------------------------
namespace
{
    sal_Bool lcl_urlAllowsInteraction( Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rURL )
    {
        bool bDoesAllow = sal_False;
        try
        {
            Reference< XURLTransformer > xTransformer(
                _rxORB->createInstance(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) ) ),
                UNO_QUERY
            );
            OSL_ENSURE( xTransformer.is(), "lcl_urlAllowsInteraction: could not create an URLTransformer!" );
            if ( xTransformer.is() )
            {
                URL aURL;
                aURL.Complete = _rURL;
                xTransformer->parseStrict( aURL );
                bDoesAllow = aURL.Arguments.equalsAscii( "Interactive" );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "lcl_urlAllowsInteraction: caught an exception while analyzing the URL!" );
        }
        return bDoesAllow;
    }

    Reference< XWindow > lcl_getTopMostWindow( Reference< XMultiServiceFactory >& _rxORB )
    {
        Reference< XWindow > xWindow;
        // get the top most window
        Reference < XFramesSupplier > xDesktop( _rxORB->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ), UNO_QUERY );
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

// -----------------------------------------------------------------------
sal_Bool DBContentLoader::impl_executeNewDatabaseWizard( Reference< XModel >& _rxModel, sal_Bool& _bShouldStartTableWizard )
{
    Sequence< Any > aWizardArgs(2);
    aWizardArgs[0] <<= PropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow")),
                    0,
                    makeAny( lcl_getTopMostWindow( m_xServiceFactory ) ),
                    PropertyState_DIRECT_VALUE);

    aWizardArgs[1] <<= PropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InitialSelection")),
                    0,
                    makeAny( _rxModel ),
                    PropertyState_DIRECT_VALUE);

    // create the dialog
    Reference< XExecutableDialog > xAdminDialog(
        m_xServiceFactory->createInstanceWithArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DatabaseWizardDialog")),aWizardArgs), UNO_QUERY);

    // execute it
    ::rtl::OUString sExistingDocumentToOpen;
    if ( !xAdminDialog.is() || ( RET_OK != xAdminDialog->execute() ) )
        return sal_False;

    Reference<XPropertySet> xProp(xAdminDialog,UNO_QUERY);
    sal_Bool bSuccess = sal_False;
    xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OpenDatabase"))) >>= bSuccess;
    xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartTableWizard"))) >>= _bShouldStartTableWizard;
    return bSuccess;
}

// -----------------------------------------------------------------------
void SAL_CALL DBContentLoader::load(const Reference< XFrame > & rFrame, const ::rtl::OUString& _rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener) throw(::com::sun::star::uno::RuntimeException)
{

    // first check if preview is true, if so return with out creating a controller. Preview is not supported
    ::comphelper::SequenceAsHashMap lDescriptor(rArgs);
    sal_Bool bPreview = lDescriptor.getUnpackedValueOrDefault(INFO_PREVIEW, sal_False );
    if ( bPreview )
    {
        if (rListener.is())
            rListener->loadCancelled(this);
        return;
    }
    Reference< XModel > xModel   = lDescriptor.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Model")), Reference< XModel >());

    ::rtl::OUString sSalvagedURL = lDescriptor.getUnpackedValueOrDefault(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SalvagedFile" ) ), _rURL );
    Sequence< PropertyValue > aLoadArgs( rArgs );

    sal_Bool bCreateNew = sal_False;        // does the URL denote the private:factory URL?
    sal_Bool bDidLoadExisting = sal_False;  // when it does, did we (the wizard) load an existing document instead
    sal_Bool bStartTableWizard = sal_False; // start the table wizard after everything was loaded successfully?

    sal_Bool bSuccess = sal_True;

    /* special mode: use already loaded model ...
        In such case no filter name will be selected and no URL will be given!
        Such informations are not neccessary. We have to create a new view only
        and call setComponent() at the corresponding frame. */
    if ( !xModel.is() )
    {
        Reference< XSingleServiceFactory > xDatabaseContext(m_xServiceFactory->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
        if ( xDatabaseContext.is() )
        {
            sal_Bool bInteractive = sal_False;

            ::rtl::OUString sFactoryName = SvtModuleOptions().GetFactoryEmptyDocumentURL(SvtModuleOptions::E_DATABASE);
            bCreateNew = sFactoryName.match(_rURL);
            Sequence<Any> aCreationArgs;
            if ( !bCreateNew )
            {
                aCreationArgs.realloc(1);
                aCreationArgs[0] <<= NamedValue( INFO_POOLURL, makeAny( sSalvagedURL ) );
            }
            else
                bInteractive = lcl_urlAllowsInteraction( m_xServiceFactory, _rURL );

            Reference< XDocumentDataSource > xDocumentDataSource;
            xDocumentDataSource.set(xDatabaseContext->createInstanceWithArguments(aCreationArgs),UNO_QUERY_THROW);
            xModel.set(xDocumentDataSource->getDatabaseDocument(),UNO_QUERY);
            if ( xModel.is() )
            {
                ::rtl::OUString sURL = xModel->getURL();
                if ( bCreateNew )
                    xModel->attachResource( sURL, aLoadArgs );

                if ( bInteractive )
                {
                    bSuccess = impl_executeNewDatabaseWizard( xModel, bStartTableWizard );
                    if ( sURL != xModel->getURL() )
                        bDidLoadExisting = sal_True;
                }
            }
        }
    }

    if ( !xModel.is() )
    {
        if ( rListener.is() )
            rListener->loadCancelled(this);
        return;
    }

    if ( !bCreateNew && !xModel->getURL().getLength() )
    {
        try
        {
            aLoadArgs.realloc(aLoadArgs.getLength()+1);
            aLoadArgs[aLoadArgs.getLength()-1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FileName"));
            aLoadArgs[aLoadArgs.getLength()-1].Value <<= _rURL;

            xModel->attachResource( _rURL, aLoadArgs );
        }
        catch(Exception&)
        {
            bSuccess = sal_False;
        }
    }

    Reference< XController > xController;
    if ( bSuccess )
    {
        xController.set(m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.OApplicationController"))),UNO_QUERY);

        bSuccess = xController.is();
        if ( bSuccess )
        {
            xController->attachModel(xModel);
            xModel->setCurrentController(xController);

            ::vos::OGuard aGuard(Application::GetSolarMutex());

            // and initialize
            try
            {
                Reference<XInitialization > xIni(xController,UNO_QUERY);
                Sequence< Any > aInitArgs( aLoadArgs.getLength() + 1 );

                Any* pArgIter = aInitArgs.getArray();
                Any* pEnd   = pArgIter + aInitArgs.getLength();
                *pArgIter++ <<= PropertyValue(
                            ::rtl::OUString::createFromAscii( "Frame" ),
                            0,
                            makeAny( rFrame ),
                            PropertyState_DIRECT_VALUE );

                const PropertyValue* pIter = aLoadArgs.getConstArray();
                for(++pArgIter;pArgIter != pEnd;++pArgIter,++pIter)
                {
                    *pArgIter <<= *pIter;
                }

                xIni->initialize(aInitArgs);
            }
            catch(Exception&)
            {
                bSuccess = sal_False;
            }
        }
    }

    if (bSuccess)
    {
        if ( xController.is() && rFrame.is() )
            xController->attachFrame(rFrame);
        try
        {
            Reference< css::container::XSet > xModelCollection(m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.GlobalEventBroadcaster"))),UNO_QUERY_THROW);
            xModelCollection->insert(css::uno::makeAny(xModel));

            Reference< css::document::XEventListener > xDocEventBroadcaster(xModel,UNO_QUERY_THROW);
            css::document::EventObject aEvent;
            aEvent.Source = xModel;
            ::std::vector< ::rtl::OUString > aEvents;
            if ( bCreateNew && !bDidLoadExisting )
            {
                aEvents.push_back(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnCreate")));
                aEvents.push_back(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnNew"))); // UI event
            }
            else
            {
                aEvents.push_back(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnLoadFinished")));
                aEvents.push_back(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnLoad"))); // UI event
            }

            std::vector< rtl::OUString >::iterator aIter = aEvents.begin();
            const std::vector< rtl::OUString >::iterator aEnd = aEvents.end();
            for (; aIter != aEnd; ++aIter)
            {
                aEvent.EventName = *aIter;
                xDocEventBroadcaster->notifyEvent(aEvent);
            }

            aEvent.EventName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OnViewCreated"));
            Reference< css::document::XEventListener > xGlobalDocEventBroadcaster(xModelCollection,UNO_QUERY_THROW);
            xGlobalDocEventBroadcaster->notifyEvent(aEvent);
        }
        catch(Exception)
        {
            OSL_ENSURE(0,"Could not add database model to global model collection and broadcast the events OnNew/OnLoad!");
        }
        if ( rListener.is() )
            rListener->loadFinished(this);

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

// -----------------------------------------------------------------------
void DBContentLoader::cancel(void) throw()
{
}
// -----------------------------------------------------------------------------
IMPL_LINK( DBContentLoader, OnStartTableWizard, void*, /*NOTINTERESTEDIN*/ )
{
    m_nStartWizard = 0;
    try
    {
        Sequence< Any > aWizArgs(1);
        PropertyValue aValue;
        aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DatabaseLocation"));
        aValue.Value <<= m_sCurrentURL;
        aWizArgs[0] <<= aValue;

        ::vos::OGuard aGuard(Application::GetSolarMutex());
        Reference< XJobExecutor > xTableWizard(m_xServiceFactory->createInstanceWithArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.wizards.table.CallTableWizard")),aWizArgs),UNO_QUERY);

        if ( xTableWizard.is() )
            xTableWizard->trigger(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("start")));
    }
    catch(const Exception&)
    {
        OSL_ENSURE(sal_False, "caught an exception while starting the table wizard!");
    }
    m_xMySelf = NULL;
    return 0L;
}
}
// -------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_DBContentLoader2()
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::DBContentLoader > aAutoRegistration;
}
// -------------------------------------------------------------------------
extern "C" void SAL_CALL writeDBLoaderInfo2(void* pRegistryKey)
{
    Reference< XRegistryKey> xKey(reinterpret_cast< XRegistryKey*>(pRegistryKey));

    // register content loader for dispatch
    ::rtl::OUString aImpl = ::rtl::OUString::createFromAscii("/");
    aImpl += ::dbaxml::DBContentLoader::getImplementationName_Static();

    ::rtl::OUString aImpltwo = aImpl;
    aImpltwo += ::rtl::OUString::createFromAscii("/UNO/Loader");
    Reference< XRegistryKey> xNewKey = xKey->createKey( aImpltwo );
    aImpltwo = aImpl;
    aImpltwo += ::rtl::OUString::createFromAscii("/Loader");
    Reference< XRegistryKey >  xLoaderKey = xKey->createKey( aImpltwo );
    xNewKey = xLoaderKey->createKey( ::rtl::OUString::createFromAscii("Pattern") );
    xNewKey->setAsciiValue( ::rtl::OUString::createFromAscii("private:factory/sdatabase") );
}
// -----------------------------------------------------------------------------

