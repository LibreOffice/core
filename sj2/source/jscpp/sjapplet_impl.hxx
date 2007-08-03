#include <cstdarg>

#ifdef SOLAR_JAVA
#include <jni.h>
#endif // SOLAR_JAVA

#ifdef SOLAR_JAVA
#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>
#else
#include <com/sun/star/uno/RuntimeException.hpp>
#endif // SOLAR_JAVA

#include "rtl/ref.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "jvmaccess/virtualmachine.hxx"

class Window;
class INetURLObject;
class SvCommandList;
class Size;
namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

#ifdef SOLAR_JAVA
struct EmbeddedWindow;
#endif // SOLAR_JAVA

struct SjApplet2_Impl {
#ifdef SOLAR_JAVA
    Window              * _pParentWin;

    rtl::Reference<jvmaccess::VirtualMachine> _virtualMachine;

    jobject     _joAppletExecutionContext;
    jclass      _jcAppletExecutionContext;

    EmbeddedWindow * _pEmbeddedWindow;
#endif // SOLAR_JAVA
    SjApplet2_Impl() throw(com::sun::star::uno::RuntimeException);
    ~SjApplet2_Impl() throw();

    void init(Window * pParentWin,
              com::sun::star::uno::Reference<
                  com::sun::star::uno::XComponentContext > const & context,
              const INetURLObject & rDocBase,
              const SvCommandList & rCmdList) throw(com::sun::star::uno::RuntimeException);

    void setSize(const Size & rSize) throw(com::sun::star::uno::RuntimeException);
    void restart() throw(com::sun::star::uno::RuntimeException);
    void reload() throw(com::sun::star::uno::RuntimeException);
    void start() throw(com::sun::star::uno::RuntimeException);
    void stop() throw(com::sun::star::uno::RuntimeException);
    void close() throw(com::sun::star::uno::RuntimeException);
};
