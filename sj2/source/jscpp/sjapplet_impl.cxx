/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sjapplet_impl.cxx,v $
 * $Revision: 1.32 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "sjapplet_impl.hxx"

#ifndef OS2
#define HWND int
#define HMENU int
#define HDC int
#endif

#include "rtl/ustring.hxx"
#include <rtl/process.h>
#include <osl/mutex.hxx>

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <svtools/ownlist.hxx>

#ifdef QUARTZ
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <com/sun/star/java/XJavaVM.hpp>
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "jvmaccess/classpath.hxx"

#if defined(UNX) && !defined(QUARTZ)
#define Time xlib_time
#define Window xlib_window
#define Font xlib_font
#define Cursor xlib_cursor
#define KeyCode xlib_keycode

#include "X11/Xlib.h"
#undef Time
#undef Window
#undef Font
#undef Cursor
#undef KeyCode
#endif

using namespace ::rtl;
using namespace ::osl;
#ifdef SOLAR_JAVA
using namespace ::com::sun::star::java;
#endif // SOLAR_JAVA
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

namespace css = ::com::sun::star;
#ifdef SOLAR_JAVA

static void testJavaException(JNIEnv * pEnv)  throw(com::sun::star::uno::RuntimeException)
{
    jthrowable jtThrowable = pEnv->ExceptionOccurred();
    if(jtThrowable) { // is it a java exception ?
#if OSL_DEBUG_LEVEL > 1
        pEnv->ExceptionDescribe();
#endif
        pEnv->ExceptionClear();

          jclass jcThrowable = pEnv->FindClass("java/lang/Throwable");
          jmethodID jmThrowable_getMessage = pEnv->GetMethodID(jcThrowable, "getMessage", "()Ljava/lang/String;");

          jstring jsMessage = (jstring)pEnv->CallObjectMethod(jtThrowable, jmThrowable_getMessage);

          rtl::OUString ouMessage;
        if(jsMessage) {
            const jchar * jcMessage = pEnv->GetStringChars(jsMessage, NULL);
            ouMessage = rtl::OUString(jcMessage);
            pEnv->ReleaseStringChars(jsMessage, jcMessage);
        }

          throw css::uno::RuntimeException(ouMessage, css::uno::Reference<XInterface>());
    }
}
#else // !SOLAR_JAVA
static void throwException() throw(com::sun::star::uno::RuntimeException)
{
    throw css::uno::RuntimeException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Java applets not supported.")), css::uno::Reference<XInterface>());
}
#endif

#ifdef SOLAR_JAVA
#if defined (UNX) && !defined(QUARTZ)
struct EmbeddedWindow {
    jobject _joWindow;

    EmbeddedWindow(JNIEnv * pEnv, SystemEnvData const * pEnvData) throw(com::sun::star::uno::RuntimeException);
    void dispose(JNIEnv * pEnv);
};

EmbeddedWindow::EmbeddedWindow(JNIEnv * pEnv, SystemEnvData const * pEnvData)
    throw(com::sun::star::uno::RuntimeException) : _joWindow(0)
{
    // ensure that all operations for the window contained in pEnvData
    // have been processed by the Xserver since java will access that
    // window by a separate connection to the Xserver
    XSync( (Display*)pEnvData->pDisplay, False);
    try
    {   //java < 1.5
        jclass jcToolkit = pEnv->FindClass("java/awt/Toolkit");                         testJavaException(pEnv);
        jmethodID jmToolkit_getDefaultToolkit = pEnv->GetStaticMethodID(
            jcToolkit, "getDefaultToolkit", "()Ljava/awt/Toolkit;" );   testJavaException(pEnv);
        pEnv->CallStaticObjectMethod(jcToolkit, jmToolkit_getDefaultToolkit);           testJavaException(pEnv);

        jclass jcMotifAppletViewer = pEnv->FindClass(
            "sun/plugin/navig/motif/MotifAppletViewer");
        if(pEnv->ExceptionOccurred())
        {
            pEnv->ExceptionClear();
            jcMotifAppletViewer = pEnv->FindClass(
                "sun/plugin/viewer/MNetscapePluginContext");                testJavaException(pEnv);
        }

        jclass jcClassLoader = pEnv->FindClass("java/lang/ClassLoader");                testJavaException(pEnv);
        jmethodID jmClassLoader_loadLibrary = pEnv->GetStaticMethodID(
            jcClassLoader, "loadLibrary",
            "(Ljava/lang/Class;Ljava/lang/String;Z)V" );                testJavaException(pEnv);
        jstring jsplugin = pEnv->NewStringUTF("javaplugin_jni");                        testJavaException(pEnv);
        pEnv->CallStaticVoidMethod(jcClassLoader, jmClassLoader_loadLibrary,
                                   jcMotifAppletViewer, jsplugin, JNI_FALSE);                  testJavaException(pEnv);

        jmethodID jmMotifAppletViewer_getWidget = pEnv->GetStaticMethodID(
            jcMotifAppletViewer, "getWidget", "(IIIII)I" );             testJavaException(pEnv);
        jint ji_widget = pEnv->CallStaticIntMethod(jcMotifAppletViewer,
                        jmMotifAppletViewer_getWidget, pEnvData->aWindow,
                        0, 0, 1, 1);                                                testJavaException(pEnv);
        jclass jcFrame  = pEnv->FindClass("sun/awt/motif/MEmbeddedFrame");              testJavaException(pEnv);
        jmethodID jmFrame_rinit = pEnv->GetMethodID(jcFrame, "<init>", "(J)V" );        testJavaException(pEnv);
        jobject joFrame = pEnv->AllocObject(jcFrame);                                   testJavaException(pEnv);
        pEnv->CallVoidMethod(joFrame, jmFrame_rinit, (jlong)ji_widget);                 testJavaException(pEnv);
        _joWindow = pEnv->NewGlobalRef(joFrame);
    }
    catch (RuntimeException & )
    {
    }

    if (_joWindow)
        return;
    //try Java > 1.5
#ifdef SOLARIS
    jclass jcFrame  = pEnv->FindClass("sun/awt/motif/MEmbeddedFrame"); testJavaException(pEnv);
    jobject joFrame = pEnv->AllocObject(jcFrame); testJavaException(pEnv);
    jmethodID jmFrame_rinit = pEnv->GetMethodID(jcFrame, "<init>", "(JZ)V" ); testJavaException(pEnv);
    pEnv->CallVoidMethod(joFrame, jmFrame_rinit, (jlong) pEnvData->aWindow, JNI_FALSE); testJavaException(pEnv);
    _joWindow = pEnv->NewGlobalRef(joFrame);
#elif LINUX
    jclass jcFrame  = pEnv->FindClass("sun/awt/X11/XEmbeddedFrame"); testJavaException(pEnv);
    jobject joFrame = pEnv->AllocObject(jcFrame); testJavaException(pEnv);
    jmethodID jmFrame_rinit = pEnv->GetMethodID(jcFrame, "<init>", "(J)V" ); testJavaException(pEnv);

    pEnv->CallVoidMethod(joFrame, jmFrame_rinit, (jlong) pEnvData->aWindow); testJavaException(pEnv);
    _joWindow = pEnv->NewGlobalRef(joFrame);
#endif
}

#else // UNX && !QUARTZ

struct EmbeddedWindow {
jobject _joWindow;

EmbeddedWindow(JNIEnv * pEnv, SystemEnvData const * pEnvData) throw(com::sun::star::uno::RuntimeException);
void dispose(JNIEnv * pEnv);
};


#ifdef WNT
EmbeddedWindow::EmbeddedWindow(JNIEnv * pEnv, SystemEnvData const * pEnvData) throw(com::sun::star::uno::RuntimeException)
{
jclass jcFrame = pEnv->FindClass("sun/awt/windows/WEmbeddedFrame");     testJavaException(pEnv);
jmethodID jmFrame_rinit = pEnv->GetMethodID(jcFrame, "<init>", "(I)V"); testJavaException(pEnv);

jobject joFrame = pEnv->AllocObject(jcFrame);                           testJavaException(pEnv);
pEnv->CallVoidMethod(joFrame, jmFrame_rinit, (jint)pEnvData->hWnd);     testJavaException(pEnv);

_joWindow = pEnv->NewGlobalRef(joFrame);
}

#elif defined QUARTZ
EmbeddedWindow::EmbeddedWindow(JNIEnv * pEnv, SystemEnvData const * pEnvData) throw(com::sun::star::uno::RuntimeException)
{
  /* The WNT code (above) that this code derives from, may be using quite old
     ways of interacting with native windows. More modern approaches seems to
     point towards JAWT_* and com.apple.eawt */
#if 0
// FIXME: this is not going to work on cocoa
jclass jcFrame = pEnv->FindClass("apple/awt/CEmbeddedFrame");     testJavaException(pEnv);
jmethodID jmFrame_rinit = pEnv->GetMethodID(jcFrame, "<init>", "(I)V"); testJavaException(pEnv);

jobject joFrame = pEnv->AllocObject(jcFrame);                           testJavaException(pEnv);
pEnv->CallVoidMethod(joFrame, jmFrame_rinit, (jint)pEnvData->pView);     testJavaException(pEnv);

_joWindow = pEnv->NewGlobalRef(joFrame);
#endif
}

#else

EmbeddedWindow::EmbeddedWindow(JNIEnv * pEnv, SystemEnvData const * pEnvData) throw(com::sun::star::uno::RuntimeException)
{
jclass jcFrame = pEnv->FindClass("java/awt/Frame");                     testJavaException(pEnv);
jmethodID jmFrame_rinit = pEnv->GetMethodID(jcFrame, "<init>", "()V");  testJavaException(pEnv);
jobject joFrame = pEnv->AllocObject(jcFrame);                                   testJavaException(pEnv);
pEnv->CallVoidMethod(joFrame, jmFrame_rinit);                           testJavaException(pEnv);

_joWindow = pEnv->NewGlobalRef(joFrame);
}

#endif

#endif // UNX && !QUARTZ

void EmbeddedWindow::dispose(JNIEnv * pEnv)
{
jclass jcWindow = pEnv->FindClass("java/awt/Window");                        testJavaException(pEnv);
jmethodID jmWindow_dispose = pEnv->GetMethodID(jcWindow, "dispose", "()V" ); testJavaException(pEnv);

pEnv->CallVoidMethod(_joWindow, jmWindow_dispose);                           testJavaException(pEnv);
pEnv->DeleteGlobalRef(_joWindow);

_joWindow = 0;
}
#endif // SOLAR_JAVA


#ifdef SOLAR_JAVA
SjApplet2_Impl::SjApplet2_Impl()  throw(com::sun::star::uno::RuntimeException)
    :
    _joAppletExecutionContext(0),
    _jcAppletExecutionContext(0)

{}
#else // !SOLAR_JAVA
SjApplet2_Impl::SjApplet2_Impl()  throw(com::sun::star::uno::RuntimeException)
{}
#endif


SjApplet2_Impl::~SjApplet2_Impl() throw()
{
#ifdef SOLAR_JAVA
    if (_joAppletExecutionContext)
    {
        try
        {
            if ( ! _virtualMachine.is())
                return;
            jvmaccess::VirtualMachine::AttachGuard vmAttachGuard(_virtualMachine);
            JNIEnv * pEnv = vmAttachGuard.getEnvironment();
            _pEmbeddedWindow->dispose(pEnv);
            delete _pEmbeddedWindow;

            pEnv->DeleteGlobalRef(_joAppletExecutionContext);
            pEnv->DeleteGlobalRef(_jcAppletExecutionContext);
        }
        catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
        {
        }
    }
#endif
}

void SjApplet2_Impl::init(
    Window * pParentWin,
    css::uno::Reference< css::uno::XComponentContext > const & context,
    const INetURLObject & rDocBase, const SvCommandList & rCmdList)
    throw(com::sun::star::uno::RuntimeException)
{
#ifdef SOLAR_JAVA
    _pParentWin = pParentWin;


    // Java URL erzeugen
    rtl::OUString url = rDocBase.GetMainURL(INetURLObject::DECODE_TO_IURI);

    if(!url.getLength())
        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///"));

    if (url.getLength()) {
        //WorkAround, weil Java mit dem | nicht zurecht kommt
        if(rDocBase.GetProtocol() == INET_PROT_FILE && url.pData->buffer[9] == INET_ENC_DELIM_TOKEN) {
            rtl::OUString tmp = url.copy(0, 9);
            tmp += String(INET_DELIM_TOKEN);
            tmp += url.copy(10);

            url = tmp;
        }
    }


#if OSL_DEBUG_LEVEL > 1
    rtl::OString tmp = rtl::OUStringToOString(url, RTL_TEXTENCODING_ASCII_US);
    OSL_TRACE("SjApplet2_Impl::init - mainUrl: %s\n", tmp.getStr());
#endif

    css::uno::Reference<XJavaVM> _xJavaVM = css::uno::Reference<XJavaVM>(
        (css::uno::Reference< css::lang::XMultiComponentFactory >(
            context->getServiceManager(), css::uno::UNO_QUERY_THROW)->
         createInstanceWithContext(
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(
                     "com.sun.star.java.JavaVirtualMachine")),
             context)),
        UNO_QUERY);
    Sequence<sal_Int8> processID(17);
    rtl_getGlobalProcessId((sal_uInt8 *)processID.getArray());
    processID[16] = 0;

    OSL_ENSURE(sizeof (sal_Int64)
                    >= sizeof (jvmaccess::VirtualMachine *),
                "Pointer cannot be represented as sal_Int64");
    sal_Int64 nPointer = reinterpret_cast< sal_Int64 >(
        static_cast< jvmaccess::VirtualMachine * >(0));
    _xJavaVM->getJavaVM(processID) >>= nPointer;
    _virtualMachine = reinterpret_cast< jvmaccess::VirtualMachine * >(nPointer);
    if (!_virtualMachine.is())
        return;

    try
    {
        jvmaccess::VirtualMachine::AttachGuard vmAttachGuard(_virtualMachine);

        JNIEnv * pEnv = vmAttachGuard.getEnvironment();

        _jcAppletExecutionContext = jvmaccess::ClassPath::loadClass(
            context, pEnv,
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "vnd.sun.star.expand:$ORIGIN/classes/classes.jar")),
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "stardiv.applet.AppletExecutionContext")));
        testJavaException(pEnv);
        _jcAppletExecutionContext = (jclass) pEnv->NewGlobalRef(_jcAppletExecutionContext );                               testJavaException(pEnv);

        jclass jcURL = pEnv->FindClass("java/net/URL");                                                                   testJavaException(pEnv);
        jmethodID jmURL_rinit = pEnv->GetMethodID(jcURL, "<init>", "(Ljava/lang/String;)V");                              testJavaException(pEnv);
        jobject joDocBase = pEnv->AllocObject(jcURL);                                                                     testJavaException(pEnv);
        jstring jsURL = pEnv->NewString(url.getStr(), url.getLength());                                                   testJavaException(pEnv);
        pEnv->CallVoidMethod(joDocBase, jmURL_rinit, jsURL);                                                              testJavaException(pEnv);

        jclass jcHashtable = pEnv->FindClass("java/util/Hashtable");                                                      testJavaException(pEnv);
        jmethodID jmHashtable_rinit = pEnv->GetMethodID(jcHashtable, "<init>", "()V");                                    testJavaException(pEnv);
        jmethodID jmHashtable_put = pEnv->GetMethodID(jcHashtable, "put",
                                                      "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");        testJavaException(pEnv);
        jobject joParameters = pEnv->AllocObject(jcHashtable);                                                            testJavaException(pEnv);
        pEnv->CallVoidMethod(joParameters, jmHashtable_rinit);                                                            testJavaException(pEnv);

        for(sal_uInt32 i = 0; i < rCmdList.Count(); ++i) {
            const SvCommand & rCmd = rCmdList[i];
            String aCmd = rCmd.GetCommand();
            String aLoweredCmd = aCmd.ToLowerAscii();

#if OSL_DEBUG_LEVEL > 1
            rtl::OUString command = aCmd;
            rtl::OUString value = rCmd.GetArgument();
            rtl::OString cmd_tmp = rtl::OUStringToOString(command, RTL_TEXTENCODING_ASCII_US);
            rtl::OString value_tmp = rtl::OUStringToOString(value, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE("command: %s=%s", cmd_tmp.getStr(), value_tmp.getStr());
#endif
            jstring jsCommand = pEnv->NewString(aLoweredCmd.GetBuffer(), aLoweredCmd.Len());                              testJavaException(pEnv);
            jstring jsArg = pEnv->NewString(rCmd.GetArgument().GetBuffer(), rCmd.GetArgument().Len());                    testJavaException(pEnv);
            pEnv->CallObjectMethod(joParameters, jmHashtable_put, jsCommand, jsArg);                                      testJavaException(pEnv);
        }

        SystemEnvData const * pEnvData  = ((SystemChildWindow *)pParentWin)->GetSystemData();
        _pEmbeddedWindow = new EmbeddedWindow(pEnv, pEnvData);

        jmethodID jmAppletExecutionContext_rinit = pEnv->GetMethodID(_jcAppletExecutionContext, "<init>",
                                                                     "(Ljava/net/URL;Ljava/util/Hashtable;Ljava/awt/Container;J)V");  testJavaException(pEnv);
        jmethodID jmAppletExecutionContext_init = pEnv->GetMethodID(_jcAppletExecutionContext, "init", "()V");            testJavaException(pEnv);
        jmethodID jmAppletExecutionContext_startUp = pEnv->GetMethodID(_jcAppletExecutionContext, "startUp", "()V");      testJavaException(pEnv);

        _joAppletExecutionContext = pEnv->AllocObject(_jcAppletExecutionContext);                                         testJavaException(pEnv);
        _joAppletExecutionContext = pEnv->NewGlobalRef(_joAppletExecutionContext);                                        testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_rinit,
                             joDocBase, joParameters, _pEmbeddedWindow->_joWindow, (jlong)0);                             testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_init);                                   testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_startUp);                                testJavaException(pEnv);

    }
    catch(jvmaccess::VirtualMachine::AttachGuard::CreationException & )
    {
        throw RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "sjapplet_impl.cxx: Could not create jvmaccess::"
                              "VirtualMachine::AttachGuard!")), 0);
    }

#else // !SOLAR_JAVA
    throwException();
#endif
}

void SjApplet2_Impl::setSize(const Size & rSize) throw(com::sun::star::uno::RuntimeException)
{

#ifdef SOLAR_JAVA
    try
    {
        if ( ! _virtualMachine.is())
            return;
        jvmaccess::VirtualMachine::AttachGuard vmAttachGuard(_virtualMachine);
        JNIEnv * pEnv = vmAttachGuard.getEnvironment();

        _pParentWin->SetSizePixel(rSize);

        jmethodID jmAppletExecutionContext_resize = pEnv->GetMethodID(
            _jcAppletExecutionContext, "appletResize", "(II)V");
        testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext,
                             jmAppletExecutionContext_resize, (jint)rSize.Width(),
                             (jint)rSize.Height());
        testJavaException(pEnv);
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        throw RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "SjApplet2_Impl::setSize, Could not create jvmaccess::"
                              "VirtualMachine::AttachGuard!")), 0);
    }
#else // !SOLAR_JAVA
    throwException();
#endif
}

void SjApplet2_Impl::restart() throw(com::sun::star::uno::RuntimeException)
{
#ifdef SOLAR_JAVA
    try
    {
        if ( ! _virtualMachine.is())
            return;
        jvmaccess::VirtualMachine::AttachGuard vmAttachGuard(_virtualMachine);
        JNIEnv * pEnv = vmAttachGuard.getEnvironment();
        jmethodID jmAppletExecutionContext_restart = pEnv->GetMethodID(
            _jcAppletExecutionContext, "restart", "()V");
        testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_restart);
        testJavaException(pEnv);
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        throw RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "SjApplet2_Impl::restart, Could not create jvmaccess::"
                              "VirtualMachine::AttachGuard!")), 0);
    }
#else // !SOLAR_JAVA
    throwException();
#endif
}

void SjApplet2_Impl::reload() throw(com::sun::star::uno::RuntimeException)
{
#ifdef SOLAR_JAVA
    try
    {
        if ( ! _virtualMachine.is())
            return;
        jvmaccess::VirtualMachine::AttachGuard vmAttachGuard(_virtualMachine);
        JNIEnv * pEnv = vmAttachGuard.getEnvironment();
        jmethodID jmAppletExecutionContext_reload = pEnv->GetMethodID(
            _jcAppletExecutionContext, "reload", "()V");
        testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_reload);
        testJavaException(pEnv);
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        throw RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "SjApplet2_Impl::reload, Could not create jvmaccess::"
                              "VirtualMachine::AttachGuard!")), 0);
    }
#else // !SOLAR_JAVA
    throwException();
#endif
}

void SjApplet2_Impl::start() throw(com::sun::star::uno::RuntimeException)
{
#ifdef SOLAR_JAVA
    try
    {
        if ( ! _virtualMachine.is())
            return;
        jvmaccess::VirtualMachine::AttachGuard vmAttachGuard(_virtualMachine);
        JNIEnv * pEnv = vmAttachGuard.getEnvironment();
        jmethodID jmAppletExecutionContext_sendStart = pEnv->GetMethodID(
            _jcAppletExecutionContext, "sendStart", "()V");
        testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_sendStart);
        testJavaException(pEnv);
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        throw RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "SjApplet2_Impl::restart, Could not create jvmaccess::"
                              "VirtualMachine::AttachGuard!")), 0);
    }
#else // !SOLAR_JAVA
    throwException();
#endif
}

void SjApplet2_Impl::stop() throw(com::sun::star::uno::RuntimeException)
{
#ifdef SOLAR_JAVA
    try
    {
        if ( ! _virtualMachine.is())
            return;
        jvmaccess::VirtualMachine::AttachGuard vmAttachGuard(_virtualMachine);
        JNIEnv * pEnv = vmAttachGuard.getEnvironment();
        jmethodID jmAppletExecutionContext_sendStop = pEnv->GetMethodID(
            _jcAppletExecutionContext, "sendStop", "()V");
        testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_sendStop);
        testJavaException(pEnv);
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        throw RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "SjApplet2_Impl::restart, Could not create jvmaccess::"
                              "VirtualMachine::AttachGuard!")), 0);
    }
#else // !SOLAR_JAVA
    throwException();
#endif
}

void SjApplet2_Impl::close() throw(com::sun::star::uno::RuntimeException)
{
#ifdef SOLAR_JAVA
    try
    {
        if ( ! _virtualMachine.is())
            return;
        jvmaccess::VirtualMachine::AttachGuard vmAttachGuard(_virtualMachine);
        JNIEnv * pEnv = vmAttachGuard.getEnvironment();

        jmethodID jmAppletExecutionContext_shutdown  = pEnv->GetMethodID(
            _jcAppletExecutionContext, "shutdown", "()V");
        testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_shutdown);
        testJavaException(pEnv);

        jmethodID jmWaitForDispose= pEnv->GetMethodID(
            _jcAppletExecutionContext,"waitForDispose","()V");
        testJavaException(pEnv);
        //blocks until the applet has destroyed itself and the container was disposed
        //(stardiv.applet.AppletExecutionContext.dispose)
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmWaitForDispose);

        // now that the applet is disposed, we dispose the AppletExecutionContext, that will end the thread
        // which dispatches the applet methods, such as init, start, stop , destroy.
        jmethodID jmAppletExecutionContext_dispose= pEnv->GetMethodID(
            _jcAppletExecutionContext,"dispose", "()V");
        testJavaException(pEnv);
        pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_dispose);
        testJavaException(pEnv);

        if( _pParentWin )
        {
            WorkWindow* pAppWin = Application::GetAppWindow();
            if(pAppWin)
            {
                while(_pParentWin->GetChildCount())
                {
                    Window* pChild = _pParentWin->GetChild(0);
                    pChild->Show( FALSE );
                    pChild->SetParent( pAppWin );
                }
            }
        }
    }
    catch (jvmaccess::VirtualMachine::AttachGuard::CreationException &)
    {
        throw RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "SjApplet2_Impl::close, Could not create jvmaccess::"
                              "VirtualMachine::AttachGuard!")), 0);
    }
#else // !SOLAR_JAVA
    throwException();
#endif
}

