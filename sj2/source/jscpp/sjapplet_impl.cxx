/*************************************************************************
 *
 *  $RCSfile: sjapplet_impl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kr $ $Date: 2001-07-30 10:26:37 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#include "sjapplet_impl.hxx"

//  #include <windows.h>

#define HWND int

#include <rtl/ustring>
#include <rtl/process.h>

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <svtools/ownlist.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>


#include <jnihelp.hxx>

using namespace ::rtl;
using namespace ::sj2;
using namespace ::utl;

using namespace ::com::sun::star::java;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

SjApplet2_Impl::SjApplet2_Impl()  throw(com::sun::star::uno::RuntimeException)
    :   _pJVM(NULL),
        _joAppletExecutionContext(0),
        _jcAppletExecutionContext(0)
{}

SjApplet2_Impl::~SjApplet2_Impl() throw() {
    if (_joAppletExecutionContext) {
        TKTThreadAttach jenv(_pJVM, _xJavaThreadRegister_11.get());

        jenv.pEnv->DeleteGlobalRef(_joAppletExecutionContext);
        jenv.pEnv->DeleteGlobalRef(_jcAppletExecutionContext);
    }
}

void SjApplet2_Impl::init(Window * pParentWin,
                          const Reference<XMultiServiceFactory> & smgr,
                          const INetURLObject & rDocBase,
                          const SvCommandList & rCmdList)
    throw(com::sun::star::uno::RuntimeException)
{
    _pParentWin = pParentWin;

    // Java URL erzeugen
    OUString url = OUString::createFromAscii("file:///");
//      OUString url = rDocBase.GetMainURL();

//      if (url.getLength()) {
//          //WorkAround, weil Java mit dem | nicht zurecht kommt
//          if(rDocBase.GetProtocol() == INET_PROT_FILE && url.pData->buffer[9] == INET_ENC_DELIM_TOKEN) {
//              OUString tmp = url.copy(0, 9);
//              tmp += OUString(INET_DELIM_TOKEN);
//              tmp += url.copy(10);

//              url = tmp;
//          }
//      }
    _xJavaVM = Reference<XJavaVM>(smgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.java.JavaVirtualMachine"))), UNO_QUERY);
    _xJavaThreadRegister_11 = Reference<XJavaThreadRegister_11>(_xJavaVM, UNO_QUERY);

    Sequence<sal_Int8> processID(16);
    rtl_getGlobalProcessId((sal_uInt8 *)processID.getArray());
      Any aVMPtr = _xJavaVM->getJavaVM(processID);
    if( sizeof(_pJVM) == sizeof(sal_Int32))
    {
        // 32 bit system
        sal_Int32 nP = 0;
        aVMPtr >>= nP;
        _pJVM = (JavaVM*)nP;
    }
    else if(sizeof(_pJVM) == sizeof(sal_Int64))
    {
        // 64 bit system
        sal_Int64 nP = 0;
        aVMPtr >>= nP;
        _pJVM = (JavaVM*)nP;
    }

    if(!_pJVM) // did we get a vm?
        throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("sjapplet_impl.cxx: couldn't get a java vm")), Reference<XInterface>());

      TKTThreadAttach jenv(_pJVM, _xJavaThreadRegister_11.get());

    _jcAppletExecutionContext = jenv.pEnv->FindClass("stardiv/applet/AppletExecutionContext");                             testJavaException(jenv.pEnv);
    _jcAppletExecutionContext = (jclass)jenv.pEnv->NewGlobalRef(_jcAppletExecutionContext );                               testJavaException(jenv.pEnv);

    _jmAppletExecutionContext_resize    = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "appletResize", "(II)V");      testJavaException(jenv.pEnv);
    _jmAppletExecutionContext_restart   = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "restart", "()V");             testJavaException(jenv.pEnv);
    _jmAppletExecutionContext_reload    = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "reload", "()V");              testJavaException(jenv.pEnv);
    _jmAppletExecutionContext_sendStart = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "sendStart", "()V");           testJavaException(jenv.pEnv);
    _jmAppletExecutionContext_sendStop  = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "sendStop", "()V");            testJavaException(jenv.pEnv);
    _jmAppletExecutionContext_shutdown  = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "shutdown", "()V");            testJavaException(jenv.pEnv);

    jclass jcURL = jenv.pEnv->FindClass("java/net/URL");                                                                   testJavaException(jenv.pEnv);
    jmethodID jmURL_rinit = jenv.pEnv->GetMethodID(jcURL, "<init>", "(Ljava/lang/String;)V");                              testJavaException(jenv.pEnv);
    jobject joDocBase = jenv.pEnv->AllocObject(jcURL);                                                                     testJavaException(jenv.pEnv);
    jstring jsURL = jenv.pEnv->NewString(url.getStr(), url.getLength());                                                   testJavaException(jenv.pEnv);
    jenv.pEnv->CallVoidMethod(joDocBase, jmURL_rinit, jsURL);                                                              testJavaException(jenv.pEnv);

    jclass jcHashtable = jenv.pEnv->FindClass("java/util/Hashtable");                                                      testJavaException(jenv.pEnv);
    jmethodID jmHashtable_rinit = jenv.pEnv->GetMethodID(jcHashtable, "<init>", "()V");                                    testJavaException(jenv.pEnv);
    jmethodID jmHashtable_put = jenv.pEnv->GetMethodID(jcHashtable, "put",
                                                       "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");        testJavaException(jenv.pEnv);
    jobject joParameters = jenv.pEnv->AllocObject(jcHashtable);                                                            testJavaException(jenv.pEnv);
    jenv.pEnv->CallVoidMethod(joParameters, jmHashtable_rinit);                                                            testJavaException(jenv.pEnv);

    for(sal_Int64 i = 0; i < rCmdList.Count(); ++i) {
        const SvCommand & rCmd = rCmdList[i];
        String aCmd = rCmd.GetCommand();
        String aLoweredCmd = aCmd.ToLowerAscii();

#ifdef DEBUG
        OUString command = aCmd;
        OUString value = rCmd.GetArgument();
        OString cmd_tmp = OUStringToOString(command, RTL_TEXTENCODING_ASCII_US);
        OString value_tmp = OUStringToOString(value, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("command: %s=%s", cmd_tmp.getStr(), value_tmp.getStr());
#endif

        jstring jsCommand = jenv.pEnv->NewString(aLoweredCmd.GetBuffer(), aLoweredCmd.Len());                              testJavaException(jenv.pEnv);
        jstring jsArg = jenv.pEnv->NewString(rCmd.GetArgument().GetBuffer(), rCmd.GetArgument().Len());                    testJavaException(jenv.pEnv);
        jenv.pEnv->CallObjectMethod(joParameters, jmHashtable_put, jsCommand, jsArg);                                      testJavaException(jenv.pEnv);
    }

    jclass jcFrame = 0;
    jobject joFrame = 0;

#if defined(WNT)
    if (WINDOW_SYSTEMCHILDWINDOW == pParentWin->GetType())
    {
        const SystemEnvData * pCD = ((SystemChildWindow*) pParentWin)->GetSystemData();
        if (pCD) {
            jcFrame = jenv.pEnv->FindClass("sun/awt/windows/WEmbeddedFrame");                                              testJavaException(jenv.pEnv);
            jmethodID jmFrame_rinit = jenv.pEnv->GetMethodID(jcFrame, "<init>", "(I)V");                                   testJavaException(jenv.pEnv);
            joFrame = jenv.pEnv->AllocObject(jcFrame);                                                                     testJavaException(jenv.pEnv);
            jenv.pEnv->CallVoidMethod(joFrame, jmFrame_rinit, (jint)pCD->hWnd);                                            testJavaException(jenv.pEnv);
        }
    }
#else
#if defined(TUNX)
    jcFrame = jenv.pEnv->FindClass("sun/awt/motif/MEmbeddedFrame");                                                        testJavaException(jenv.pEnv);
    jmethodID jmFrame_rinit = jenv.pEnv->GetMethodID(jcFrame, "<init>", "(J)V");                                           testJavaException(jenv.pEnv);
    joFrame = jenv.pEnv->AllocObject(jcFrame);                                                                             testJavaException(jenv.pEnv);
    jenv.pEnv->CallVoidMethod(joFrame, jmFrame_rinit, (jlong)pParentWin->GetSystemData()->pWidget);                        testJavaException(jenv.pEnv);
#else
    jcFrame = jenv.pEnv->FindClass("java/awt/Frame");                                                                      testJavaException(jenv.pEnv);
    jmethodID jmFrame_rinit = jenv.pEnv->GetMethodID(jcFrame, "<init>", "()V");                                            testJavaException(jenv.pEnv);
    joFrame = jenv.pEnv->AllocObject(jcFrame);                                                                             testJavaException(jenv.pEnv);
    jenv.pEnv->CallVoidMethod(joFrame, jmFrame_rinit);                                                                     testJavaException(jenv.pEnv);
#endif
#endif

    jmethodID jmFrame_show = jenv.pEnv->GetMethodID(jcFrame, "show", "()V");                                               testJavaException(jenv.pEnv);
    jenv.pEnv->CallVoidMethod(joFrame, jmFrame_show);                                                                      testJavaException(jenv.pEnv);

    jmethodID jmAppletExecutionContext_rinit = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "<init>",
                                                          "(Ljava/net/URL;Ljava/util/Hashtable;Ljava/awt/Container;J)V");  testJavaException(jenv.pEnv);
    jmethodID jmAppletExecutionContext_init = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "init", "()V");            testJavaException(jenv.pEnv);
    jmethodID jmAppletExecutionContext_startUp = jenv.pEnv->GetMethodID(_jcAppletExecutionContext, "startUp", "()V");      testJavaException(jenv.pEnv);

    _joAppletExecutionContext = jenv.pEnv->AllocObject(_jcAppletExecutionContext);                                         testJavaException(jenv.pEnv);
    _joAppletExecutionContext = jenv.pEnv->NewGlobalRef(_joAppletExecutionContext);                                        testJavaException(jenv.pEnv);
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_rinit, joDocBase, joParameters, joFrame,
                              (jlong)0);                                                                                   testJavaException(jenv.pEnv);
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_init);                                   testJavaException(jenv.pEnv);
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, jmAppletExecutionContext_startUp);                                testJavaException(jenv.pEnv);
}

void SjApplet2_Impl::setSize(const Size & rSize) throw(com::sun::star::uno::RuntimeException)
{
    TKTThreadAttach jenv(_pJVM, _xJavaThreadRegister_11.get());
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, _jmAppletExecutionContext_resize, (jint)rSize.Width(),
                              (jint)rSize.Height());                                                                       testJavaException(jenv.pEnv);
}

void SjApplet2_Impl::restart() throw(com::sun::star::uno::RuntimeException)
{
    TKTThreadAttach jenv(_pJVM, _xJavaThreadRegister_11.get());
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, _jmAppletExecutionContext_restart);                               testJavaException(jenv.pEnv);
}

void SjApplet2_Impl::reload() throw(com::sun::star::uno::RuntimeException)
{
    TKTThreadAttach jenv(_pJVM, _xJavaThreadRegister_11.get());
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, _jmAppletExecutionContext_reload);                                testJavaException(jenv.pEnv);
}

void SjApplet2_Impl::start() throw(com::sun::star::uno::RuntimeException)
{
    TKTThreadAttach jenv(_pJVM, _xJavaThreadRegister_11.get());
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, _jmAppletExecutionContext_sendStart);                             testJavaException(jenv.pEnv);
}

void SjApplet2_Impl::stop() throw(com::sun::star::uno::RuntimeException)
{
    TKTThreadAttach jenv(_pJVM, _xJavaThreadRegister_11.get());
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, _jmAppletExecutionContext_sendStop);                              testJavaException(jenv.pEnv);
}

void SjApplet2_Impl::close() throw(com::sun::star::uno::RuntimeException)
{
    TKTThreadAttach jenv(_pJVM, _xJavaThreadRegister_11.get());
    jenv.pEnv->CallVoidMethod(_joAppletExecutionContext, _jmAppletExecutionContext_shutdown);                              testJavaException(jenv.pEnv);

    if( _pParentWin )
    {
          WorkWindow*   pAppWin = Application::GetAppWindow();
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

void SjApplet2_Impl::testJavaException(JNIEnv * pEnv)  throw(com::sun::star::uno::RuntimeException)
{
    jthrowable jtThrowable = pEnv->ExceptionOccurred();
    if(jtThrowable) { // is it a java exception ?
        pEnv->ExceptionDescribe();
        pEnv->ExceptionClear();

        jclass jcThrowable = pEnv->FindClass("java/lang/Throwable");
        jmethodID jmThrowable_getMessage = pEnv->GetMethodID(jcThrowable, "getMessage", "()V");

        jstring jsMessage = (jstring)pEnv->CallObjectMethod(jtThrowable, jmThrowable_getMessage);

        const jchar * jcMessage = pEnv->GetStringChars(jsMessage, NULL);
        OUString ouMessage(jcMessage);
        pEnv->ReleaseStringChars(jsMessage, jcMessage);

        throw RuntimeException(ouMessage, Reference<XInterface>());
    }
}
