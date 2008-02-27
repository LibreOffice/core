#ifndef ADDITIONALFUNC_HXX
#define ADDITIONALFUNC_HXX

#ifndef _registertestfunction_h
#include "cppunit/autoregister/registertestfunction.h"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

extern "C" void SAL_CALL RegisterAdditionalFunctions(FktRegFuncPtr _pFunc);

#define NOADDITIONAL \
void RegisterAdditionalFunctions(FktRegFuncPtr){}

namespace {

enum T2_OSType {
    T2_OS_WNT32 = 1,
    T2_OS_UNIX,
    T2_OS_OS2
};

inline T2_OSType getOSType()
{
#if defined WNT
    return T2_OS_WNT32;
#elif defined OS2
    return T2_OS_OS2;
#else
    return T2_OS_UNIX;
#endif
}

}

#define TESTSHL2_UNO_BOOTSTRAP(file)                                                    \
void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)                                  \
{                                                                                       \
   com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory> xMS;     \
   com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xComponentContext; \
   try {                                                                                \
       if (strlen(#file) > 0) {                                                         \
           ::rtl::OUString iniFile(rtl::OUString::createFromAscii(#file));              \
           if (getOSType() == T2_OS_WNT32 || getOSType() == T2_OS_OS2) {                \
               iniFile += ::rtl::OUString::createFromAscii(".ini");                     \
           } else {                                                                     \
               iniFile += ::rtl::OUString::createFromAscii("rc");                       \
           }                                                                            \
           xComponentContext = cppu::defaultBootstrap_InitialComponentContext(iniFile); \
       } else {                                                                         \
           xComponentContext = cppu::defaultBootstrap_InitialComponentContext();        \
       }                                                                                \
       xMS.set(xComponentContext->getServiceManager(), com::sun::star::uno::UNO_QUERY); \
       comphelper::setProcessServiceFactory(xMS);                                       \
   }                                                                                    \
   catch (::com::sun::star::uno::Exception e ) {                                        \
       rtl::OString aError;                                                             \
       aError = rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);           \
       printf("Error at UNO bootstrap time caught: %s\nMust quit.\n", aError.getStr()); \
       exit(1);                                                                         \
   }                                                                                    \
}

#endif
