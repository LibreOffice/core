/*************************************************************************
 *
 *  $RCSfile: dbloader2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:17:52 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef _COM_SUN_STAR_FRAME_XFRAMELOADER_HPP_
#include <com/sun/star/frame/XFrameLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADEVENTLISTENER_HPP_
#include <com/sun/star/frame/XLoadEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
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
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
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

using namespace ::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::registry;

// -------------------------------------------------------------------------
namespace dbaxml
{

class DBTypeDetection : public ::cppu::WeakImplHelper2< XExtendedFilterDetection, XServiceInfo>
{
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
{
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL DBTypeDetection::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (::com::sun::star::uno::RuntimeException)
{

    ::comphelper::SequenceAsHashMap aTemp(Descriptor);
    ::rtl::OUString sTemp = aTemp.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL")),::rtl::OUString());

    if ( sTemp.getLength() )
    {
        INetURLObject aURL(sTemp);
        if ( aURL.GetExtension().EqualsIgnoreCaseAscii("odb") )
            return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBase"));
    }
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
};

DBContentLoader::DBContentLoader(const Reference< XMultiServiceFactory >& _rxFactory)
    :m_xServiceFactory(_rxFactory)
{
}
// -------------------------------------------------------------------------

DBContentLoader::~DBContentLoader()
{
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
void SAL_CALL DBContentLoader::load(const Reference< XFrame > & rFrame, const ::rtl::OUString& rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener) throw(::com::sun::star::uno::RuntimeException)
{
    m_xFrame    = rFrame;
    m_xListener = rListener;
    m_aURL      = rURL;
    m_aArgs     = rArgs;

    Reference< XSingleServiceFactory > xDatabaseContext(m_xServiceFactory->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
    Reference<XComponent> xComponent;
    sal_Bool bCreateNew = sal_False;
    sal_Bool bInteractive = sal_False;
    if ( xDatabaseContext.is() )
    {
        bCreateNew = rURL.match(SvtModuleOptions().GetFactoryEmptyDocumentURL(SvtModuleOptions::E_DATABASE));
        Sequence<Any> aCreationArgs;
        if ( !bCreateNew )
        {
            aCreationArgs.realloc(1);
            aCreationArgs[0] <<= NamedValue(INFO_POOLURL,makeAny(rURL));
        }
        else
            bInteractive = rURL.match(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("?Interactive")), SvtModuleOptions().GetFactoryEmptyDocumentURL(SvtModuleOptions::E_DATABASE).getLength() );
        xComponent.set(xDatabaseContext->createInstanceWithArguments(aCreationArgs),UNO_QUERY);
    }


    sal_Bool bSuccess = sal_True;
    Reference<XModel> xModel(xComponent,UNO_QUERY);
    if ( !bCreateNew && xModel.is() && !xModel->getURL().getLength() )
    {
        try
        {
            // ich benutze nicht maURL, sondern rURL, denn zwischen dem Constructor und diesem Load hier kann sich die ::com::sun::star::util::URL des Objektes
            // schon geaendert haben (zum Beispiel durch Umbenennen)
            const PropertyValue* pValue =::std::find_if(rArgs.getConstArray(),
                                                        rArgs.getConstArray() + rArgs.getLength(),
                                                        ::std::bind2nd(::comphelper::TPropertyValueEqualFunctor(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StatusIndicator"))));

            Sequence<Any> aFilterArgs;
            if ( pValue && pValue != (rArgs.getConstArray() + rArgs.getLength()) )
            {
                Reference<XStatusIndicator> xStatusIndicator(pValue->Value,UNO_QUERY);

                // set progress range and start status indicator
                sal_Int32 nProgressRange(1000000);
                if (xStatusIndicator.is())
                {
                    xStatusIndicator->start(::rtl::OUString(), nProgressRange);
                    aFilterArgs.realloc( 1 );
                    Any *pArgs = aFilterArgs.getArray();
                    *pArgs++ <<= xStatusIndicator;
                }
            }

            Reference<XImporter> xImporter(m_xServiceFactory->createInstanceWithArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.sdb.DBFilter")),aFilterArgs),UNO_QUERY);

            xImporter->setTargetDocument(xComponent);
            Reference<XFilter> xFilter(xImporter,UNO_QUERY);

            m_aArgs.realloc(m_aArgs.getLength()+1);
            m_aArgs[m_aArgs.getLength()-1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FileName"));
            m_aArgs[m_aArgs.getLength()-1].Value <<= rURL;

            xModel->attachResource(rURL,m_aArgs);

            xFilter->filter(m_aArgs);
        }
        catch(Exception&)
        {
            bSuccess = sal_False;
        }
    }

    Reference< XController >    xController(m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.OApplicationController"))),UNO_QUERY);

    if ( bSuccess && (bSuccess = ( xController.is() && xModel.is() )) )
    {
        xController->attachModel(xModel);
        xModel->connectController( xController );
        xModel->setCurrentController(xController);

        ::vos::OGuard aGuard(Application::GetSolarMutex());

        // and initialize
        try
        {
            Reference<XInitialization > xIni(xController,UNO_QUERY);
            PropertyValue aProp(::rtl::OUString::createFromAscii("Frame"),0,makeAny(rFrame),PropertyState_DIRECT_VALUE);
            Sequence< Any > aArgs(m_aArgs.getLength() + 2);

            Any* pArgIter = aArgs.getArray();
            Any* pEnd   = pArgIter + aArgs.getLength();
            *pArgIter++ <<= aProp;

            aProp.Name = URL_INTERACTIVE;
            aProp.Value <<= bInteractive;
            *pArgIter++ <<= aProp;

            const PropertyValue* pIter      = m_aArgs.getConstArray();
            for(++pArgIter;pArgIter != pEnd;++pArgIter,++pIter)
            {
                *pArgIter <<= *pIter;
            }

            xIni->initialize(aArgs);
        }
        catch(Exception&)
        {
            bSuccess = sal_False;
        }
    }

    if (bSuccess && rListener.is())
    {
        if ( xController.is() && rFrame.is() )
            xController->attachFrame(rFrame);
        rListener->loadFinished(this);
    }
    else if (!bSuccess && rListener.is())
        rListener->loadCancelled(this);
}

// -----------------------------------------------------------------------
void DBContentLoader::cancel(void) throw()
{
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
