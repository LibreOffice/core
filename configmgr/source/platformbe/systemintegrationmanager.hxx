#ifndef CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_
#define CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <com/sun/star/configuration/backend/XBackend.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDSETUPEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESNOTIFIER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLECOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSINGLELAYERSTRATUM_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif // _CPPUHELPER_COMPBASE4_HXX_

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr { namespace backend {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backenduno = css::configuration::backend ;



typedef uno::Reference<lang::XSingleComponentFactory>   BackendFactory;
typedef uno::Reference<backenduno::XSingleLayerStratum> PlatformBackend;


/* Class containing a reference to a service factory(XSingleComponentFactory)
   object and a platform backend (XSingleLayerStratum).
   The reference to the platform backend will be NULL until the platform backend
   is initialised
 */
class BackendRef
{
    BackendFactory  mFactory;
    PlatformBackend mBackend;
public:
    explicit
    BackendRef(const BackendFactory& aFactory)
    :mFactory(aFactory)
    ,mBackend()
    {}

    PlatformBackend getBackend(uno::Reference<uno::XComponentContext> const & xContext);
    void disposeBackend();
};

typedef cppu::WeakComponentImplHelper4< backenduno::XBackend,
                                        backenduno::XBackendChangesNotifier,
                                        lang::XInitialization,
                                        lang::XServiceInfo> BackendBase ;


/**
  Class implementing the Backend service for system integration backend access.
  It creates the required backends and coordinates access to them.
  */
class SystemIntegrationManager : public BackendBase
{
public:
   /**
     Service constructor from a service factory.

     @param xContext   component context
     */
    explicit
    SystemIntegrationManager( const uno::Reference<uno::XComponentContext>& xContext) ;

    /** Destructor  */
    ~SystemIntegrationManager() ;

    // XBackend
    virtual uno::Sequence<uno::Reference<backenduno::XLayer> >
        SAL_CALL listOwnLayers(const rtl::OUString& aComponent)
            throw (backenduno::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;

    virtual uno::Reference<backenduno::XUpdateHandler>
        SAL_CALL getOwnUpdateHandler(const rtl::OUString& aComponent)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                lang::NoSupportException,
                uno::RuntimeException) ;

    virtual uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
        listLayers(const rtl::OUString& aComponent,
                   const rtl::OUString& aEntity)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                uno::RuntimeException) ;

    virtual uno::Reference<backenduno::XUpdateHandler> SAL_CALL
        getUpdateHandler(const rtl::OUString& aComponent,
                         const rtl::OUString& aEntity)
        throw (backenduno::BackendAccessException,
                lang::IllegalArgumentException,
                lang::NoSupportException,
                uno::RuntimeException) ;

    // XInitialize
    virtual void SAL_CALL initialize(const uno::Sequence<uno::Any>& aParameters)
        throw (uno::RuntimeException, uno::Exception,
               lang::IllegalArgumentException,
               backenduno::BackendSetupException) ;

   // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException) ;

    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& aServiceName)
        throw (uno::RuntimeException) ;

    virtual uno::Sequence<rtl::OUString> SAL_CALL
        getSupportedServiceNames(void) throw (uno::RuntimeException) ;

    // XBackendChangesNotifier
    virtual void SAL_CALL addChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& xListner,
                                              const rtl::OUString& aComponent)
        throw (uno::RuntimeException);


    virtual void SAL_CALL removeChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& xListner,
                                                 const rtl::OUString& aComponent)
        throw (uno::RuntimeException);

     /**
      Provides the implementation name.

      @return   implementation name
      */
    static rtl::OUString SAL_CALL getSystemIntegrationManagerName(void) ;
    /**
      Provides the list of supported services.

      @return   list of service names
      */
    static uno::Sequence<rtl::OUString> SAL_CALL getServiceNames(void) ;
protected:
// ComponentHelper
    virtual void SAL_CALL disposing();
private :
    typedef std::multimap<rtl::OUString, BackendRef> BackendFactoryList;
    typedef std::vector<PlatformBackend> PlatformBackendList;

    /** build lookup up table
    */
    void buildLookupTable();

    /** get list of supported components
    */
    uno::Sequence<rtl::OUString> getSupportedComponents(const BackendFactory& xFactory);

    /**
        get supporting backends from lookup table
    */
    PlatformBackendList getSupportingBackends(const rtl::OUString& aComponent);

private :
    /** Mutex for resource protection */
    osl::Mutex mMutex ;
    /** Component Context */
    uno::Reference<uno::XComponentContext> mContext ;

    BackendFactoryList mPlatformBackends;
} ;

} }  // configmgr.backend

#endif // CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_
