#include <cstdarg>


#include <jni.h>


#include <unotools/processfactory.hxx>

#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

class Window;
class INetURLObject;
class SvCommandList;
class Size;

struct SjApplet2_Impl {
    Window              * _pParentWin;

    JavaVM *    _pJVM;
    jobject     _joAppletExecutionContext;
    jclass      _jcAppletExecutionContext;

    jmethodID _jmAppletExecutionContext_resize;
    jmethodID _jmAppletExecutionContext_restart;
    jmethodID _jmAppletExecutionContext_reload;
    jmethodID _jmAppletExecutionContext_sendStart;
    jmethodID _jmAppletExecutionContext_sendStop;
    jmethodID _jmAppletExecutionContext_shutdown;

    com::sun::star::uno::Reference<com::sun::star::java::XJavaVM>                   _xJavaVM;
    com::sun::star::uno::Reference<com::sun::star::java::XJavaThreadRegister_11>    _xJavaThreadRegister_11;

    SjApplet2_Impl() throw(com::sun::star::uno::RuntimeException);
    ~SjApplet2_Impl() throw();

    void init(Window * pParentWin,
              const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> & smgr,
              const INetURLObject & rDocBase,
              const SvCommandList & rCmdList) throw(com::sun::star::uno::RuntimeException);

    void setSize(const Size & rSize) throw(com::sun::star::uno::RuntimeException);
    void restart() throw(com::sun::star::uno::RuntimeException);
    void reload() throw(com::sun::star::uno::RuntimeException);
    void start() throw(com::sun::star::uno::RuntimeException);
    void stop() throw(com::sun::star::uno::RuntimeException);
    void close() throw(com::sun::star::uno::RuntimeException);


    void testJavaException(JNIEnv * pEnv) throw(com::sun::star::uno::RuntimeException);
};
