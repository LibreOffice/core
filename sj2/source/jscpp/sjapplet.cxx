/*************************************************************************
 *
 *  $RCSfile: sjapplet.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:54:03 $
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

#include <jnihelp.hxx>

#include <sjapplet.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <unotools/processfactory.hxx>
#include <rtl/ustring>

#include <rtl/process.h>

#include <jni.h>

using namespace ::rtl;
using namespace ::com::sun::star::lang;
using namespace ::utl;

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <svtools/ownlist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/syschild.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::java;
using namespace ::sj2;

#include <com/sun/star/java/XJavaVM.hpp>

struct SjApplet2ImplStruct {
    JavaVM *    _pJVM;
    jobject     _joAppletExecutionContext;
    jobject     _joFrame;
    jclass      _jcAppletExecutionContext;

    Reference<XJavaVM>                  _xJavaVM;
    Reference<XJavaThreadRegister_11>   _xJavaThreadRegister_11;
};

SjApplet2::SjApplet2()
  : _pParentWin(NULL),
    _pImpl( new SjApplet2ImplStruct() )
{
}

SjApplet2::~SjApplet2()
{
    if (_pImpl->_joAppletExecutionContext)
    {
        TKTThreadAttach jenv( _pImpl->_pJVM,
                              _pImpl->_xJavaThreadRegister_11.get()
                            );

        jenv.pEnv->DeleteGlobalRef( _pImpl->_joAppletExecutionContext );
        jenv.pEnv->DeleteGlobalRef( _pImpl->_joFrame );
        jenv.pEnv->DeleteGlobalRef( _pImpl->_jcAppletExecutionContext );
    }

    delete _pImpl;
}

//=========================================================================
void SjApplet2::Init( Window * pParentWin, const INetURLObject & rDocBase, const SvCommandList & rCmdList )
{
    Reference<XMultiServiceFactory> serviceManager(getProcessServiceFactory());

    _pImpl->_xJavaVM = Reference<XJavaVM> (serviceManager->createInstance(OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine")), UNO_QUERY);
    _pImpl->_xJavaThreadRegister_11 = Reference<XJavaThreadRegister_11>(_pImpl->_xJavaVM, UNO_QUERY);

    Sequence<sal_Int8> processID(16);
    rtl_getGlobalProcessId((sal_uInt8 *)processID.getArray());
      Any aVMPtr = _pImpl->_xJavaVM->getJavaVM(processID);
    if( sizeof( _pImpl->_pJVM ) == sizeof( sal_Int32 ) )
    {
        // 32 bit system
        sal_Int32 nP = 0;
        aVMPtr >>= nP;
        _pImpl->_pJVM = (JavaVM*)nP;
    }
    else if( sizeof( _pImpl->_pJVM ) == sizeof( sal_Int64 ) )
    {
        // 64 bit system
        sal_Int64 nP = 0;
        aVMPtr >>= nP;
        _pImpl->_pJVM = (JavaVM*)nP;
    }

    TKTThreadAttach jenv(_pImpl->_pJVM, _pImpl->_xJavaThreadRegister_11.get());

    // Java URL erzeugen
    String aURL = rDocBase.GetMainURL();
    if ( aURL.Len() )
    {
        //WorkAround, weil Java mit dem | nicht zurecht kommt
        if( rDocBase.GetProtocol() == INET_PROT_FILE
            && aURL.GetChar( (xub_StrLen)9 ) == INET_ENC_DELIM_TOKEN )
            aURL = aURL.Insert( INET_DELIM_TOKEN, (xub_StrLen)9 );
    }

    jclass jcURL = jenv.pEnv->FindClass("java/net/URL");
    jmethodID jmURL_rinit = jenv.pEnv->GetMethodID(jcURL, "<init>", "(Ljava/lang/String;)V");
    jobject joDocBase = jenv.pEnv->AllocObject(jcURL);
    jstring jsURL = jenv.pEnv->NewString( aURL.GetBuffer(), aURL.Len() );
    jenv.pEnv->CallVoidMethod(joDocBase, jmURL_rinit, jsURL);

    jclass jcHashtable = jenv.pEnv->FindClass("java/util/Hashtable");
    jmethodID jmHashtable_rinit = jenv.pEnv->GetMethodID(jcHashtable, "<init>", "()V");
    jmethodID jmHashtable_put = jenv.pEnv->GetMethodID(jcHashtable, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jobject joParameters = jenv.pEnv->AllocObject(jcHashtable);
    jenv.pEnv->CallVoidMethod(joParameters, jmHashtable_rinit);

    for( ULONG i = 0; i < rCmdList.Count(); i++ ) {
        const SvCommand & rCmd = rCmdList[i];
        String aCmd = rCmd.GetCommand();
        String aLoweredCmd = aCmd.ToLowerAscii();
        jstring jsCommand = jenv.pEnv->NewString( aLoweredCmd.GetBuffer(), aLoweredCmd.Len() );
        jstring jsArg = jenv.pEnv->NewString( rCmd.GetArgument().GetBuffer(), rCmd.GetArgument().Len() );
        jenv.pEnv->CallVoidMethod(joParameters, jmHashtable_put, jsCommand, jsArg);
    }


    _pParentWin = pParentWin;

#if defined(WNT) && defined(blblblblblblb)
    if (WINDOW_SYSTEMCHILDWINDOW == pParentWin->GetType())
    {
        const SystemChildData*  pCD = ((SystemChildWindow*) pParentWin)->GetSystemData();
        if ( pCD )
            // hier wird das C++-Wrapper-Objekt fuer ein Java-Objekt erzeugt
            pWindow = new sun_awt_windows_WEmbeddedFrame((INT32)pCD->hWnd);
    }
    if (!pWindow)
        pWindow = new sun_awt_windows_WEmbeddedFrame();
#else
        jclass jcFrame = jenv.pEnv->FindClass("java/awt/Frame");
        jmethodID jmFrame_rinit = jenv.pEnv->GetMethodID(jcFrame, "<init>", "()V");
        _pImpl->_joFrame = jenv.pEnv->AllocObject(jcFrame);
        _pImpl->_joFrame = jenv.pEnv->NewGlobalRef(_pImpl->_joFrame);
        jenv.pEnv->CallVoidMethod(_pImpl->_joFrame, jmFrame_rinit);
#endif

    jmethodID jmFrame_show = jenv.pEnv->GetMethodID(jcFrame, "show", "()V");
    jenv.pEnv->CallVoidMethod(_pImpl->_joFrame, jmFrame_show);

    _pImpl->_jcAppletExecutionContext = jenv.pEnv->FindClass("stardiv/applet/AppletExecutionContext");
    _pImpl->_jcAppletExecutionContext = (jclass)jenv.pEnv->NewGlobalRef( _pImpl->_jcAppletExecutionContext );
    jmethodID jmAppletExecutionContext_rinit = jenv.pEnv->GetMethodID(_pImpl->_jcAppletExecutionContext, "<init>", "(Ljava/net/URL;Ljava/util/Hashtable;Ljava/awt/Container;J)V");
    jmethodID jmAppletExecutionContext_init = jenv.pEnv->GetMethodID(_pImpl->_jcAppletExecutionContext, "init", "()V");
    jmethodID jmAppletExecutionContext_startUp = jenv.pEnv->GetMethodID(_pImpl->_jcAppletExecutionContext, "startUp", "()V");

    _pImpl->_joAppletExecutionContext = jenv.pEnv->AllocObject(_pImpl->_jcAppletExecutionContext);
    _pImpl->_joAppletExecutionContext = jenv.pEnv->NewGlobalRef(_pImpl->_joAppletExecutionContext);
    jenv.pEnv->CallVoidMethod(_pImpl->_joAppletExecutionContext, jmAppletExecutionContext_rinit, joDocBase, joParameters, _pImpl->_joFrame, (jlong)0);
    jenv.pEnv->CallVoidMethod(_pImpl->_joAppletExecutionContext, jmAppletExecutionContext_init);
    jenv.pEnv->CallVoidMethod(_pImpl->_joAppletExecutionContext, jmAppletExecutionContext_startUp);

//      pWindow->setVisible(TRUE);

//      pAppletExecutionContext = new stardiv_applet_AppletExecutionContext_Impl(&aDocBase, &aHashtable, pWindow, this);
//      pAppletExecutionContext->init();

//      pAppletExecutionContext->startUp();
}

//=========================================================================
void SjApplet2::setSizePixel( const Size & rSize )
{
//      pWindow->setSize(rSize.Width(), rSize.Height());
}

void SjApplet2::appletRestart()
{
    TKTThreadAttach jenv(_pImpl->_pJVM, _pImpl->_xJavaThreadRegister_11.get());
    jmethodID jmAppletExecutionContext_restart = jenv.pEnv->GetMethodID(_pImpl->_jcAppletExecutionContext, "restart", "()V");
    jenv.pEnv->CallVoidMethod(_pImpl->_joAppletExecutionContext, jmAppletExecutionContext_restart);
}

void SjApplet2::appletReload()
{
    TKTThreadAttach jenv(_pImpl->_pJVM, _pImpl->_xJavaThreadRegister_11.get());
    jmethodID jmAppletExecutionContext_reload = jenv.pEnv->GetMethodID(_pImpl->_jcAppletExecutionContext, "reload", "()V");
    jenv.pEnv->CallVoidMethod(_pImpl->_joAppletExecutionContext, jmAppletExecutionContext_reload);
}

void SjApplet2::appletStart()
{
    TKTThreadAttach jenv(_pImpl->_pJVM, _pImpl->_xJavaThreadRegister_11.get());
    jmethodID jmAppletExecutionContext_sendStart = jenv.pEnv->GetMethodID(_pImpl->_jcAppletExecutionContext, "sendStart", "()V");
    jenv.pEnv->CallVoidMethod(_pImpl->_joAppletExecutionContext, jmAppletExecutionContext_sendStart);
}

void SjApplet2::appletStop()
{
    TKTThreadAttach jenv(_pImpl->_pJVM, _pImpl->_xJavaThreadRegister_11.get());
    jmethodID jmAppletExecutionContext_sendStop = jenv.pEnv->GetMethodID(_pImpl->_jcAppletExecutionContext, "sendStop", "()V");
    jenv.pEnv->CallVoidMethod(_pImpl->_joAppletExecutionContext, jmAppletExecutionContext_sendStop);
}

void SjApplet2::appletClose()
{
    if(_pImpl->_joAppletExecutionContext)
    {
        TKTThreadAttach jenv(_pImpl->_pJVM, _pImpl->_xJavaThreadRegister_11.get());
        jmethodID jmAppletExecutionContext_shutdown = jenv.pEnv->GetMethodID(_pImpl->_jcAppletExecutionContext, "shutdown", "()V");
        jenv.pEnv->CallVoidMethod(_pImpl->_joAppletExecutionContext, jmAppletExecutionContext_shutdown);
    }

//      pWindow->dispose();

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

//      delete pWindow;
//      pWindow = NULL;
}

// Fuer SO3, Wrapper fuer Applet liefern
SjJScriptAppletObject * SjApplet2::GetJScriptApplet()
{
    return NULL;
}

#ifdef _OLD_FEATURE
class SjINetSettings : public ApplicationProperty
/*  [Beschreibung]

    Diese Klasse repr"asentiert die Internet Einstellungen von Java.
    Beim Initialisieren von Java werden, "uber die Methode
    <Application::Property(...)>, die Einstellungen abgefragt.
*/
{
    String  aHttpProxy;
    int     nHttpProxyPort;
    String  aFtpProxy;
    int     nFtpProxyPort;
    String  aFirewallProxy;
    int     nFirewallProxyPort;
public:
                    TYPEINFO();
                    SjINetSettings();

    void            SetHttpProxy( const String & rStr )
                    { aHttpProxy = rStr; }
    const String &  GetHttpProxy() const { return aHttpProxy; }
    void            SetHttpProxyPort( int n )
                    { nHttpProxyPort = n; }
    int             GetHttpProxyPort() const { return nHttpProxyPort; }

    void            SetFtpProxy( const String & rStr )
                    { aFtpProxy = rStr; }
    const String &  GetFtpProxy() const { return aFtpProxy; }
    void            SetFtpProxyPort( int n )
                    { nFtpProxyPort = n; }
    int             GetFtpProxyPort() const { return nFtpProxyPort; }

    void            SetFirewallProxy( const String & rStr )
                    { aFirewallProxy = rStr; }
    const String &  GetFirewallProxy() const { return aFirewallProxy; }
    void            SetFirewallProxyPort( int n )
                    { nFirewallProxyPort = n; }
    int             GetFirewallProxyPort() const { return nFirewallProxyPort; }
};

class SjJavaSettings : public ApplicationProperty
/*  [Beschreibung]

    Diese Klasse repr"asentiert die allgemeinen Java Einstellungen.
    Beim Initialisieren von Java werden, "uber die Methode
    <Application::Property(...)>, die Einstellungen abgefragt.

    JavaHomeDir ist das Installationsverzeichnis von Java.
*/
{
    String          aClassPath;
    String          aJavaHomeDir;
    SjNetAccess     eNetAccess;
    UINT32          nNativeStackSize;   // Stack size for native threads
    UINT32          nJavaStackSize;     // Stack size for Java threads
    UINT32          nMinHeapSize;       // Minimum heap size (default 0 -> Java-Default)
    UINT32          nMaxHeapSize;       // Maximum heap size (default 0 -> JavaDefault)

    UINT32          nVerifyMode;        // controls whether Java byte code should be verified:
                                        // 0 -- none,
                                        // 1 -- remotely loaded code,
                                        // 2 -- all code.

    BOOL            bEnableClassGC;     // default set to TRUE
    BOOL            bEnableVerboseGC;   // default set to FALSE
    BOOL            bEnableAsyncGC;     // default set to TRUE
    BOOL            bVerbose;           // Switch the verbose mode of the VM
    BOOL            bDebugging;         // Enables or disable the debugging VM
    UINT32          nDebugPort;         // Set the debug port. Only valid with bDebugging = TRUE
    BOOL            bSecurity;          // TRUE, sandbox security enabled. FALSE, no security. Default set to TRUE.

public:

                    TYPEINFO();
                    SjJavaSettings();

    void            SetClassPath( const String & rStr )
                    { aClassPath = rStr; }
    const String &  GetClassPath() const { return aClassPath; }

    void            SetJavaHomeDir( const String & rStr )
                    { aJavaHomeDir = rStr; }
    const String &  GetJavaHomeDir() const { return aJavaHomeDir; }

    void            SetNetAccess( SjNetAccess eAcc )
                    { eNetAccess = eAcc; }
    SjNetAccess     GetNetAccess() const { return eNetAccess; }

                    // nSize == 0 -> Systemdefault
    void            SetMinHeapSize( UINT32 nSize )
                    { nMinHeapSize = nSize; }
    UINT32          GetMinHeapSize() const { return nMinHeapSize; }

                    // nSize == 0 -> Systemdefault
    void            SetMaxHeapSize( UINT32 nSize )
                    { nMaxHeapSize = nSize; }
    UINT32          GetMaxHeapSize() const { return nMaxHeapSize; }

                    // nSize == 0 -> Systemdefault
    void            SetNativeStackSize( UINT32 nSize )
                    { nNativeStackSize = nSize; }
    UINT32          GetNativeStackSize() const { return nNativeStackSize; }

                    // nSize == 0 -> Systemdefault
    void            SetJavaStackSize( UINT32 nSize )
                    { nJavaStackSize = nSize; }
    UINT32          GetJavaStackSize() const { return nJavaStackSize; }

                    // default = 2 -> remotely loaded code
    void            SetVerifyMode( UINT32 nSize )
                    { nVerifyMode = nSize; }
    UINT32          GetVerifyMode() const { return nVerifyMode; }

    void            EnableClassGC( BOOL bEnable )
                    { bEnableClassGC = bEnable; }
    UINT32          IsClassGCEnabled() const { return bEnableClassGC; }

    void            EnableVerboseGC( BOOL bEnable )
                    { bEnableVerboseGC = bEnable; }
    UINT32          IsVerboseGCEnabled() const { return bEnableVerboseGC; }

    void            EnableAsyncGC( BOOL bEnable )
                    { bEnableAsyncGC = bEnable; }
    UINT32          IsAsyncGCEnabled() const { return bEnableAsyncGC; }

    void            EnableVerboseVM( BOOL bEnable )
                    { bVerbose = bEnable; }
    UINT32          IsVerboseVMEnabled() const { return bVerbose; }

    void            EnableDebugging( BOOL bEnable )
                    { bDebugging = bEnable; }
    UINT32          IsDebuggingEnabled() const { return bDebugging; }

    void            SetDebugPort( UINT32 nPort )
                    { nDebugPort = nPort; }
    UINT32          GetDebugPort() const { return nDebugPort; }


    void            EnableSecurity( BOOL bEnable )
                    { bSecurity = bEnable; }
    BOOL            IsSecurityEnabled() const { return bSecurity; }

};


TYPEINIT1( SjJavaSettings, ApplicationProperty )
SjJavaSettings::SjJavaSettings()
/*  [Beschreibung]

    Im Konstruktor werden die Einstellungen des Classpath und JavaHome
    auf die Environment Variablen "classpath" und "java_home" initialisiert.
*/
    : eNetAccess( NET_HOST )
    , nNativeStackSize( 0 )
    , nJavaStackSize( 0 )
    , nMinHeapSize( 0 )
    , nMaxHeapSize( 0 )
    , nVerifyMode( 1 )
    , bEnableClassGC( TRUE )
    , bEnableVerboseGC( FALSE )
    , bEnableAsyncGC( TRUE )
    , bDebugging( FALSE )
    , nDebugPort( 0 )
    , bSecurity( TRUE )
{
}

//=========================================================================
//=========================================================================
//=========================================================================
TYPEINIT1( SjINetSettings, ApplicationProperty )
SjINetSettings::SjINetSettings()
/*  [Beschreibung]

    Die Proxy-Servernamen werden auf "" und die Port auf 0 initialisiert.
*/
    : nHttpProxyPort( 0 )
    , nFtpProxyPort( 0 )
    , nFirewallProxyPort( 0 )
{
}
#endif // _OLD_FEATURE


/*
 * Java init function to invoke Java runtime using JNI invocation API.
 */

void JRE_PropertyChanged( JNIEnv * env, const SvCommandList & rCmdList )
{
    jclass pClass = env->FindClass("java/util/Properties");
    if( !pClass )
        return;

    jmethodID mCtor = env->GetMethodID( pClass, "<init>", "()V" );
    jobject pProps = env->NewObject( pClass, mCtor, NULL );

    char * pSignature = "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;";
    char * pMethodName = "put";
    jmethodID mPut = env->GetMethodID( pClass, pMethodName, pSignature );
    env->DeleteLocalRef( pClass );

    if( !mCtor || !pProps || ! mPut )
        return;

    for( ULONG i = 0; i < rCmdList.Count(); i++ )
    {
        const SvCommand & rCmd = rCmdList[i];
        jstring pCommand = env->NewString( rCmd.GetCommand().GetBuffer(), rCmd.GetCommand().Len() );
        jstring pArg = env->NewString( rCmd.GetArgument().GetBuffer(), rCmd.GetArgument().Len() );

        jobject hTmp = env->CallObjectMethod( pProps, mPut, pCommand, pArg );
        env->DeleteLocalRef( hTmp );
        env->DeleteLocalRef( pCommand );
        env->DeleteLocalRef( pArg );
    }

    pClass = env->FindClass("stardiv/controller/SjSettings");

    if( !pClass )
        return;

    jmethodID mid = env->GetStaticMethodID( pClass , "changeProperties", "(Ljava/util/Properties;)V");
    if( !mid )
        return;

    env->CallStaticVoidMethod( pClass, mid, pProps );
    env->DeleteLocalRef( pProps );
    env->DeleteLocalRef( pClass );
}


void SjApplet2::settingsChanged(void)
{
    Reference<XMultiServiceFactory> serviceManager(getProcessServiceFactory());

    Reference<XJavaVM> xJavaVM(serviceManager->createInstance(OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine")), UNO_QUERY);

    if(xJavaVM->isVMStarted())
    {
        Reference<XJavaThreadRegister_11> xJavaThreadRegister_11(xJavaVM, UNO_QUERY);

        Sequence<sal_Int8> processID(16);
        rtl_getGlobalProcessId((sal_uInt8 *)processID.getArray());
        JavaVM * pJVM = (JavaVM *)xJavaVM->getJavaVM(processID).getValue();
        TKTThreadAttach jenv(pJVM, xJavaThreadRegister_11.get());

        if( jenv.pEnv)
        {
            DBG_ERROR( "SjApplet2::settingsChanged not implemented" );
#ifdef _OLD_FEATURE

            SjINetSettings aINetSettings;
            GetpApp()->Property(aINetSettings);
            SjJavaSettings aJSettings;
            GetpApp()->Property(aJSettings);

            SvCommandList aCmdList;
            // Security Settings
            switch ( aJSettings.GetNetAccess() )
            {
                case NET_UNRESTRICTED:
                    aCmdList.Append( String::CreateFromAscii("appletviewer.security.mode"),
                                     String::CreateFromAscii("unrestricted") );
                    break;

                case NET_NONE:
                    aCmdList.Append( String::CreateFromAscii("appletviewer.security.mode"),
                                     String::CreateFromAscii("none") );
                    break;

                case NET_HOST:
                    aCmdList.Append( String::CreateFromAscii("appletviewer.security.mode"),
                                     String::CreateFromAscii("host") );
                    break;
            }
            if ( aJSettings.IsSecurityEnabled() )
                aCmdList.Append( String::CreateFromAscii("stardiv.security.disableSecurity"),
                                 String::CreateFromAscii("false") );
            else
                aCmdList.Append( String::CreateFromAscii("stardiv.security.disableSecurity"),
                                 String::CreateFromAscii("true") );

            // HTTP settings
            aCmdList.Append( String::CreateFromAscii("http.proxyHost"),
                             aINetSettings.GetHttpProxy());
            aCmdList.Append( String::CreateFromAscii("http.proxyPort"),
                             aINetSettings.GetHttpProxyPort());

            // Ftp settings
            if( aINetSettings.GetFtpProxy().Len() )
                aCmdList.Append( String::CreateFromAscii("ftpProxySet"), String::CreateFromAscii("true") );
            else
                aCmdList.Append( String::CreateFromAscii("ftpProxySet"), String::CreateFromAscii("false") );

            aCmdList.Append( String::CreateFromAscii("ftpProxyHost"), aINetSettings.GetFtpProxy());
            aCmdList.Append( String::CreateFromAscii("ftpProxyPort"), aINetSettings.GetFtpProxyPort());

            JRE_PropertyChanged(jenv.pEnv, aCmdList);
#endif //_OLD_FEATURE
        }
    }
}

