/*************************************************************************
 *
 *  $RCSfile: dbloader.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-06 17:41:30 $
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
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _DBA_REGISTRATION_HELPER_HXX_
#include "registrationhelper.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

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
        return ::rtl::OUString::createFromAscii("openoffice.org.DBContentLoader");
    }
    static Sequence< ::rtl::OUString> getSupportedServiceNames_Static(void) throw(  );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    // XLoader
    virtual void SAL_CALL load( const Reference< XFrame > & _rFrame, const ::rtl::OUString& _rURL,
                                const Sequence< PropertyValue >& _rArgs,
                                const Reference< XLoadEventListener > & _rListener);
    virtual void SAL_CALL cancel(void);
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
extern "C" void SAL_CALL createRegistryInfo_DBContentLoader()
{
    static OMultiInstanceAutoRegistration< DBContentLoader > aAutoRegistration;
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
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii("stardiv.one.frame.FrameLoader");
    aSNS.getArray()[1] = ::rtl::OUString::createFromAscii("stardiv.one.frame.DBContentLoader");
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
        const Reference< XLoadEventListener > & rListener)
{
    m_xFrame    = rFrame;
    m_xListener = rListener;
    m_aURL      = rURL;
    m_aArgs     = rArgs;

    INetURLObject aParser(rURL);
    // ich benutze nicht maURL, sondern rURL, denn zwischen dem Constructor und diesem Load hier kann sich die ::com::sun::star::util::URL des Objektes
    // schon geaendert haben (zum Beispiel durch Umbenennen)
    Reference< XController >    xController;
    sal_Bool bSuccess = sal_True;
    if(aParser.GetMainURL().EqualsAscii(".component:DB/FormGridView"))
        xController = Reference< XController >(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("org.openoffice.dbaccess.OFormGridView")),UNO_QUERY);
    else if(aParser.GetMainURL().EqualsAscii(".component:DB/DataSourceBrowser"))// construct the control
        xController = Reference< XController >(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("org.openoffice.dbaccess.ODatabaseBrowser")),UNO_QUERY);
    else
        OSL_ENSHURE(0,"wrong dispatch url!");

    if(bSuccess = xController.is())
    {
        // and initialize
        try
        {
            Reference<XInitialization > xIni(xController,UNO_QUERY);
            Sequence< Any> aArgs(1);
            aArgs[0] <<= rFrame;
            xIni->initialize(aArgs);
        }
        catch(Exception&)
        {
            bSuccess = sal_False;
        }

    }

    if (bSuccess && rListener.is())
        rListener->loadFinished(this);
    else if (!bSuccess && rListener.is())
        rListener->loadCancelled(this);
}

// -----------------------------------------------------------------------
void DBContentLoader::cancel(void)
{
}
