/*************************************************************************
 *
 *  $RCSfile: javachild.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 10:46:36 $
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

#include <jni.h>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <unohelp.hxx>
#include <rtl/process.h>
#include <rtl/ref.hxx>
#include <jvmaccess/virtualmachine.hxx>
#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#include <window.hxx>
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
#endif
#include "javachild.hxx"
#include <svdata.hxx>
#include <sysdata.hxx>

using namespace ::com::sun::star;

// -------------------
// - JavaChildWindow -
// -------------------

JavaChildWindow::JavaChildWindow( Window* pParent, WinBits nStyle ) :
    SystemChildWindow( pParent, nStyle )
{
}

// -----------------------------------------------------------------------

JavaChildWindow::JavaChildWindow( Window* pParent, const ResId& rResId ) :
    SystemChildWindow( pParent, rResId )
{
}

// -----------------------------------------------------------------------

JavaChildWindow::~JavaChildWindow()
{
}

// -----------------------------------------------------------------------

void JavaChildWindow::implTestJavaException( void* pEnv )
{
    JNIEnv*     pJavaEnv = reinterpret_cast< JNIEnv* >( pEnv );
    jthrowable  jtThrowable = pJavaEnv->ExceptionOccurred();

    if( jtThrowable )
    { // is it a java exception ?
#if OSL_DEBUG_LEVEL > 1
        pJavaEnv->ExceptionDescribe();
#endif
        pJavaEnv->ExceptionClear();

        jclass          jcThrowable = pJavaEnv->FindClass("java/lang/Throwable");
        jmethodID       jmThrowable_getMessage = pJavaEnv->GetMethodID(jcThrowable, "getMessage", "()Ljava/lang/String;");
        jstring         jsMessage = (jstring) pJavaEnv->CallObjectMethod(jtThrowable, jmThrowable_getMessage);
        ::rtl::OUString ouMessage;

        if(jsMessage)
        {
            const jchar * jcMessage = pJavaEnv->GetStringChars(jsMessage, NULL);
            ouMessage = ::rtl::OUString(jcMessage);
            pJavaEnv->ReleaseStringChars(jsMessage, jcMessage);
        }

        throw uno::RuntimeException(ouMessage, uno::Reference<uno::XInterface>());
    }
}

// -----------------------------------------------------------------------

sal_Int32 JavaChildWindow::getParentWindowHandleForJava()
{
    sal_Int32 nRet = 0;

#if defined WNT
    nRet = reinterpret_cast< sal_Int32 >( GetSystemData()->hWnd );
#elif defined UNX
    uno::Reference< lang::XMultiServiceFactory > xFactory( vcl::unohelper::GetMultiServiceFactory() );

    if( xFactory.is() && ( GetSystemData()->aWindow > 0 ) )
    {
        JavaVM*                                         pJVM;
        ::rtl::Reference< ::jvmaccess::VirtualMachine > xVM;
        uno::Reference< java::XJavaVM >                 xJavaVM( xFactory->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.java.JavaVirtualMachine") ) ), uno::UNO_QUERY );
        uno::Sequence< sal_Int8 >                       aProcessID( 17 );

        rtl_getGlobalProcessId( (sal_uInt8*) aProcessID.getArray() );
        aProcessID[ 16 ] = 0;
        OSL_ENSURE(sizeof (sal_Int64) >= sizeof (jvmaccess::VirtualMachine *), "Pointer cannot be represented as sal_Int64");
        sal_Int64 nPointer = reinterpret_cast< sal_Int64 >( static_cast< jvmaccess::VirtualMachine * >(0));
        xJavaVM->getJavaVM(aProcessID) >>= nPointer;
        xVM = reinterpret_cast< jvmaccess::VirtualMachine * >(nPointer);

        if( xVM.is() )
        {
            try
            {
                ::jvmaccess::VirtualMachine::AttachGuard    aVMAttachGuard( xVM );
                JNIEnv*                                     pEnv = aVMAttachGuard.getEnvironment();

                jclass jcToolkit = pEnv->FindClass("java/awt/Toolkit");
                implTestJavaException(pEnv);

                jmethodID jmToolkit_getDefaultToolkit = pEnv->GetStaticMethodID( jcToolkit, "getDefaultToolkit", "()Ljava/awt/Toolkit;" );
                implTestJavaException(pEnv);

                pEnv->CallStaticObjectMethod(jcToolkit, jmToolkit_getDefaultToolkit);
                implTestJavaException(pEnv);

                jclass jcMotifAppletViewer = pEnv->FindClass("sun/plugin/navig/motif/MotifAppletViewer");
                if( pEnv->ExceptionOccurred() )
                {
                    pEnv->ExceptionClear();

                    jcMotifAppletViewer = pEnv->FindClass( "sun/plugin/viewer/MNetscapePluginContext");
                    implTestJavaException(pEnv);
                }

                jclass jcClassLoader = pEnv->FindClass("java/lang/ClassLoader");
                implTestJavaException(pEnv);

                jmethodID jmClassLoader_loadLibrary = pEnv->GetStaticMethodID( jcClassLoader, "loadLibrary", "(Ljava/lang/Class;Ljava/lang/String;Z)V");
                implTestJavaException(pEnv);

                jstring jsplugin = pEnv->NewStringUTF("javaplugin_jni");
                implTestJavaException(pEnv);

                pEnv->CallStaticVoidMethod(jcClassLoader, jmClassLoader_loadLibrary, jcMotifAppletViewer, jsplugin, JNI_FALSE);
                implTestJavaException(pEnv);

                jmethodID jmMotifAppletViewer_getWidget = pEnv->GetStaticMethodID( jcMotifAppletViewer, "getWidget", "(IIIII)I" );
                implTestJavaException(pEnv);

                const Size aSize( GetOutputSizePixel() );
                jint ji_widget = pEnv->CallStaticIntMethod( jcMotifAppletViewer, jmMotifAppletViewer_getWidget,
                                                            GetSystemData()->aWindow, 0, 0, aSize.Width(), aSize.Height() );
                implTestJavaException(pEnv);

                nRet = (sal_Int32) ji_widget;
            }
            catch( ::jvmaccess::VirtualMachine::AttachGuard::CreationException& )
            {
                throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "JavaChildWindow::getJavaWindowHandle: Could not create jvmaccess::VirtualMachine::AttachGuard!")), 0);
            }
        }
    }
#else
    // TBD
#endif

    return nRet;
}
