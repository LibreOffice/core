#ifndef CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_
#define CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_

#include <com/sun/star/configuration/backend/XBackend.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/compbase4.hxx>

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr { namespace backend {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backenduno = css::configuration::backend ;

/* Class containing a reference to a service factory(XSingleComponentFactory)
   object and a platform backend (XSingleLayerStratum).
   The reference to the platform backend will be NULL until the platform backend
   is initialised
 */
class BackendRef
{
    uno::Reference<lang::XSingleComponentFactory>  mFactory;
    uno::Reference<backenduno::XSingleLayerStratum> mBackend;
public:
    explicit
    BackendRef(const uno::Reference<lang::XSingleComponentFactory>& aFactory)
    :mFactory(aFactory)
    ,mBackend()
    {}

    uno::Reference<backenduno::XSingleLayerStratum> getBackend(uno::Reference<uno::XComponentContext> const & xContext);
    void disposeBackend();
};

/**
  Class implementing the Backend service for system integration backend access.
  It creates the required backends and coordinates access to them.
  */
class SystemIntegrationManager : public cppu::WeakComponentImplHelper4< backenduno::XBackend, backenduno::XBackendChangesNotifier, lang::XInitialization, lang::XServiceInfo>
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
    /** build lookup up table
    */
    void buildLookupTable();

    /** get list of supported components
    */
    uno::Sequence<rtl::OUString> getSupportedComponents(const uno::Reference<lang::XSingleComponentFactory>& xFactory);

    /**
        get supporting backends from lookup table
    */
    std::vector< uno::Reference<backenduno::XSingleLayerStratum> > getSupportingBackends(const rtl::OUString& aComponent);

private :
    /** Mutex for resource protection */
    osl::Mutex mMutex ;
    /** Component Context */
    uno::Reference<uno::XComponentContext> mContext ;

    std::multimap<rtl::OUString, BackendRef> mPlatformBackends;
} ;

} }  // configmgr.backend

#endif // CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_
