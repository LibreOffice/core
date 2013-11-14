/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/accessibility/XMSAAService.hpp>

#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <vcl/svapp.hxx>

using namespace ::rtl; // for OUString
using namespace ::com::sun::star; // for odk interfaces
using namespace ::com::sun::star::uno; // for basic types
using namespace ::com::sun::star::accessibility;

using namespace ::com::sun::star::awt;

typedef sal_Int32 HWND;

#include "AccTopWindowListener.hxx"
#include "g_msacc.hxx"

extern void FreeTopWindowListener();
extern long GetMSComPtr(long hWnd, long lParam, long wParam);
extern void handleWindowOpened_impl( long pAcc);


namespace my_sc_impl
{

extern Sequence< OUString > SAL_CALL  getSupportedServiceNames_MSAAServiceImpl();
extern OUString SAL_CALL getImplementationName_MSAAServiceImpl();
extern Reference< XInterface > SAL_CALL create_MSAAServiceImpl(
        Reference< XComponentContext > const & xContext )
    SAL_THROW( () );
/**
   * Method that returns the service name.
   * @param
   * @return Name sequence.
   */
static Sequence< OUString > getSupportedServiceNames_MSAAServiceImpl()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        //      MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.MSAAService"));
            pNames = &seqNames;
        }
    }
    return *pNames;
}

/**
   * Method that returns the service name.
   * @param
   * @return Name sequence.
   */
static OUString getImplementationName_MSAAServiceImpl()
{
    static OUString *pImplName = 0;
    if( ! pImplName )
    {
        //      MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pImplName )
        {
            static OUString implName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.my_sc_implementation.MSAAService") );
            pImplName = &implName;
        }
    }
    return *pImplName;
}

class MSAAServiceImpl : public ::cppu::WeakImplHelper3<
            XMSAAService, lang::XServiceInfo, lang::XInitialization >
{
    OUString m_arg;
public:
    // focus on three given interfaces,
    // no need to implement XInterface, XTypeProvider, XWeak
    MSAAServiceImpl ();
    virtual ~MSAAServiceImpl( void );
    // XInitialization will be called upon createInstanceWithArguments[AndContext]()
    virtual void SAL_CALL initialize( Sequence< Any > const & args )
    throw (Exception);
    // XMSAAService
    virtual sal_Int32 SAL_CALL getAccObjectPtr (long hWnd, long lParam, long wParam)
    throw (RuntimeException);
    virtual void SAL_CALL handleWindowOpened(sal_Int32)
    throw (RuntimeException);
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
    throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw (RuntimeException);
};

/**
   * Implemention of XInitialization.
   * @param
   * @return.
   */
void MSAAServiceImpl::initialize( Sequence< Any > const & args ) throw (Exception)
{
    if (1 != args.getLength())
    {
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("give a string instanciating this component!") ),
            (::cppu::OWeakObject *)this, // resolve to XInterface reference
            0 ); // argument pos
    }
    if (! (args[ 0 ] >>= m_arg))
    {
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no string given as argument!") ),
            (::cppu::OWeakObject *)this, // resolve to XInterface reference
            0 ); // argument pos
    }
}

/**
   * Implemention of getAccObjectPtr.
   * @param
   * @return Com interface.
   */
sal_Int32 MSAAServiceImpl::getAccObjectPtr ( long hWnd, long lParam, long wParam) throw (RuntimeException)
{
    return GetMSComPtr(hWnd, lParam, wParam);
}

/**
   * Implemention of handleWindowOpened,the method will be invoked when a top window
   * opened and AT starts up.
   * @param
   * @return
   */
void MSAAServiceImpl::handleWindowOpened( sal_Int32 pAcc)
{
    handleWindowOpened_impl(pAcc);
}

/**
   * Implemention of XServiceInfo.
   * @param
   * @return Implementataion name.
   */
OUString MSAAServiceImpl::getImplementationName() throw (RuntimeException)
{
    // unique implementation name
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.my_sc_impl.MSAAService") );
}

/**
   * Implemention of XServiceInfo,return support service name.
   * @param Service name.
   * @return If the service name is supported.
   */
sal_Bool MSAAServiceImpl::supportsService( OUString const & serviceName ) throw (RuntimeException)
{
    // this object only supports one service, so the test is simple
    return serviceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.accessibility.MSAAService") );
}

/**
   * Implemention of XServiceInfo,return all service names.
   * @param.
   * @return service name sequence.
   */
Sequence< OUString > MSAAServiceImpl::getSupportedServiceNames() throw (RuntimeException)
{
    return getSupportedServiceNames_MSAAServiceImpl();
}

/**
   * Static method that can create an entity of our MSAA Service
   * @param xContext No use here.
   * @return The object interface.
   */
Reference< XInterface > SAL_CALL create_MSAAServiceImpl( Reference< XComponentContext > const & /*xContext*/ ) SAL_THROW( () )
{
    MSAAServiceImpl* xxx = new MSAAServiceImpl();
    //return static_cast< lang::XTypeProvider * >(  xxx );
    Reference< XMSAAService > p( xxx );
    return p;
}

/**
   * Constructor.
   * @param
   * @return
   */
MSAAServiceImpl::MSAAServiceImpl()
{
    Reference< XExtendedToolkit > xToolkit =
        Reference< XExtendedToolkit >(Application::GetVCLToolkit(), UNO_QUERY);

    if(xToolkit.is())
    {
        AccTopWindowListener *accListener;
        accListener = new AccTopWindowListener();
        g_pTop = accListener;
        Reference< XTopWindowListener> x(accListener);
        xToolkit->addTopWindowListener(x);
    }
}

/**
   * Static method that can create an entity of our MSAA Service
   * @param Destructor
   * @return
   */
MSAAServiceImpl::~MSAAServiceImpl()
{

    // As all folders and streams contain references to their parents,
    // we must remove these references so that they will be deleted when
    // the hash_map of the root folder is cleared, releasing all subfolders
    // and substreams which in turn release theirs, etc. When xRootFolder is
    // released when this destructor completes, the folder tree should be
    // deleted fully (and automagically).
    FreeTopWindowListener();


}

}

/* shared lib exports implemented without helpers in service_impl1.cxx */
namespace my_sc_impl
{
static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            create_MSAAServiceImpl, getImplementationName_MSAAServiceImpl,
            getSupportedServiceNames_MSAAServiceImpl, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_MSAAServiceImpl, getImplementationName_MSAAServiceImpl,
            getSupportedServiceNames_MSAAServiceImpl, ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        sal_Char const ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }
    void * SAL_CALL component_getFactory(
        sal_Char const * implName, lang::XMultiServiceFactory * xMgr,
        registry::XRegistryKey * xRegistry )
    {
        return ::cppu::component_getFactoryHelper(
                   implName, xMgr, xRegistry, ::my_sc_impl::s_component_entries );
    }
}
