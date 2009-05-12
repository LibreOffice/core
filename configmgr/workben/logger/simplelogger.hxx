#ifndef CONFIGMGR_SIMPLELOGGER_HXX_
#define CONFIGMGR_SIMPLELOGGER_HXX_

#include "simplelogger.hxx"
#include <com/sun/star/util/logging/XLogger.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>

#include <stdio.h>

namespace logger {

namespace uno = com::sun::star::uno ;
namespace lang = com::sun::star::lang ;
namespace logging = com::sun::star::util::logging ;
using rtl::OUString;

typedef cppu::WeakImplHelper2< logging::XLogger,
                               lang::XServiceInfo> LoggerBase ;


/**
  Class implementing the Logger service
  */
class SimpleLogger : public LoggerBase
{
public :
    /**
      Service constructor from a service factory.

      @param xContext   component context
      */
    explicit
    SimpleLogger( const uno::Reference<uno::XComponentContext>& xContext, char const * name) ;

    /** Destructor  */
    ~SimpleLogger() ;

   // XLogger
    virtual uno::Reference< logging::XLogger > SAL_CALL
        getLogger( const OUString& name )
            throw (uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getLevel(  ) throw (uno::RuntimeException);
    virtual OUString SAL_CALL getName(  ) throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL isLoggable( sal_Int32 level ) throw (uno::RuntimeException);

    virtual void SAL_CALL
        logp( sal_Int32 level, const OUString& sourceClass, const OUString& sourceMethod, const OUString& msg )
            throw (uno::RuntimeException);

   // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException) ;

    virtual sal_Bool SAL_CALL supportsService( const OUString& aServiceName)
        throw (uno::RuntimeException) ;

    virtual uno::Sequence<OUString> SAL_CALL
        getSupportedServiceNames(void) throw (uno::RuntimeException) ;

     /**
      Provides the implementation name.

      @return   implementation name
      */
    static OUString SAL_CALL getImplementationName_static() ;
    /**
      Provides the list of supported services.

      @return   list of service names
      */
    static uno::Sequence<OUString> SAL_CALL getSupportedServiceNames_static() ;

private:
    /** Component Context */
    uno::Reference<uno::XComponentContext> mContext ;
    OUString mName;
    FILE * mOutput;
    sal_Int32 mLevel;
} ;

}  // namespace logger

#endif
