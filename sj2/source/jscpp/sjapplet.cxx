/*************************************************************************
 *
 *  $RCSfile: sjapplet.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:00:56 $
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

#include <cstdarg>
#include <jnihelp.hxx>

#include <sjapplet.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <unotools/processfactory.hxx>
#include <rtl/ustring>

#include <rtl/process.h>

#include <tools/debug.hxx>
#include <svtools/ownlist.hxx>


#include "sjapplet_impl.hxx"

using namespace ::rtl;
using namespace ::utl;
using namespace ::sj2;

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::java;

SjApplet2::SjApplet2()
  : _pImpl(new SjApplet2_Impl())
{
}

SjApplet2::~SjApplet2()
{
    delete _pImpl;
}

//=========================================================================
void SjApplet2::Init(Window * pParentWin, const INetURLObject & rDocBase, const SvCommandList & rCmdList)
{
    try {
        if(_pImpl)
            _pImpl->init(pParentWin, getProcessServiceFactory(), rDocBase, rCmdList);
    }
    catch(RuntimeException & runtimeException) {
#if OSL_DEBUG_LEVEL > 1
        OString message = OUStringToOString(runtimeException.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("sjapplet.cxx: SjApplet2::Init - exception occurred: %s\n", message.getStr());
#endif

        delete _pImpl;
        _pImpl = 0;
    }
}

//=========================================================================
void SjApplet2::setSizePixel( const Size & rSize )
{
    if(_pImpl)
        _pImpl->setSize(rSize);
}

void SjApplet2::appletRestart()
{
    if(_pImpl)
        _pImpl->restart();
}

void SjApplet2::appletReload()
{
    if(_pImpl)
        _pImpl->reload();
}

void SjApplet2::appletStart()
{
    if(_pImpl)
        _pImpl->start();
}

void SjApplet2::appletStop()
{
    if(_pImpl)
        _pImpl->stop();
}

void SjApplet2::appletClose()
{
    if(_pImpl)
        _pImpl->close();
}

// Fuer SO3, Wrapper fuer Applet liefern
SjJScriptAppletObject * SjApplet2::GetJScriptApplet()
{
    OSL_TRACE("SjApplet2::GetJScriptApplet\n");

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

// Settings are detected by the JavaVM service
// This function is not necessary anymore
void SjApplet2::settingsChanged(void)
{
//      Reference<XMultiServiceFactory> serviceManager(getProcessServiceFactory());

//      Reference<XJavaVM> xJavaVM(serviceManager->createInstance(OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine")), UNO_QUERY);

//      if(xJavaVM->isVMStarted())
//      {
//          Reference<XJavaThreadRegister_11> xJavaThreadRegister_11(xJavaVM, UNO_QUERY);

//          Sequence<sal_Int8> processID(16);
//          rtl_getGlobalProcessId((sal_uInt8 *)processID.getArray());
//          JavaVM * pJVM = *(JavaVM **)xJavaVM->getJavaVM(processID).getValue();
//          TKTThreadAttach jenv(pJVM, xJavaThreadRegister_11.get());

//          if( jenv.pEnv)
//          {
//    //            DBG_ERROR( "SjApplet2::settingsChanged not implemented" );
//  #ifdef _OLD_FEATURE

//              SjINetSettings aINetSettings;
//              GetpApp()->Property(aINetSettings);
//              SjJavaSettings aJSettings;
//              GetpApp()->Property(aJSettings);

//              SvCommandList aCmdList;
//              // Security Settings
//              switch ( aJSettings.GetNetAccess() )
//              {
//                  case NET_UNRESTRICTED:
//                      aCmdList.Append( String::CreateFromAscii("appletviewer.security.mode"),
//                                       String::CreateFromAscii("unrestricted") );
//                      break;

//                  case NET_NONE:
//                      aCmdList.Append( String::CreateFromAscii("appletviewer.security.mode"),
//                                       String::CreateFromAscii("none") );
//                      break;

//                  case NET_HOST:
//                      aCmdList.Append( String::CreateFromAscii("appletviewer.security.mode"),
//                                       String::CreateFromAscii("host") );
//                      break;
//              }
//              if ( aJSettings.IsSecurityEnabled() )
//                  aCmdList.Append( String::CreateFromAscii("stardiv.security.disableSecurity"),
//                                   String::CreateFromAscii("false") );
//              else
//                  aCmdList.Append( String::CreateFromAscii("stardiv.security.disableSecurity"),
//                                   String::CreateFromAscii("true") );

//              // HTTP settings
//              // http.proxyHost, http.proxyPort, ftp.proxyHost, ftp.proxyPort are changed within
//              // the JavaVM service.

//              // Ftp settings
//              if( aINetSettings.GetFtpProxy().Len() )
//                  aCmdList.Append( String::CreateFromAscii("ftpProxySet"), String::CreateFromAscii("true") );
//              else
//                  aCmdList.Append( String::CreateFromAscii("ftpProxySet"), String::CreateFromAscii("false") );


//              JRE_PropertyChanged(jenv.pEnv, aCmdList);
//  #endif //_OLD_FEATURE
//          }
//      }
}

