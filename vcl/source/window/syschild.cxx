/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <rtl/process.h>
#include <rtl/ref.hxx>

#include <tools/rc.h>

#include <vcl/window.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>

#include <window.h>
#include <salinst.hxx>
#include <salframe.hxx>
#include <salobj.hxx>
#include <svdata.hxx>

#if HAVE_FEATURE_JAVA
#include <jni.h>
#endif

#include <comphelper/processfactory.hxx>

#if HAVE_FEATURE_JAVA
#include <jvmaccess/virtualmachine.hxx>
#include <com/sun/star/java/JavaVirtualMachine.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

using namespace ::com::sun::star;

long ImplSysChildProc( void* pInst, SalObject* /* pObject */,
                       sal_uInt16 nEvent, const void* /* pEvent */ )
{
    SystemChildWindow* pWindow = (SystemChildWindow*)pInst;
    long nRet = 0;

    ImplDelData aDogTag( pWindow );
    switch ( nEvent )
    {
        case SALOBJ_EVENT_GETFOCUS:
            // get focus, such that all handlers are called,
            // as if this window gets the focus assuring
            // that the frame does not steal it
            pWindow->ImplGetFrameData()->mbSysObjFocus = true;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = true;
            pWindow->ToTop( TOTOP_NOGRABFOCUS );
            if( aDogTag.IsDead() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = false;
            pWindow->ImplGetFrameData()->mbInSysObjFocusHdl = true;
            pWindow->GrabFocus();
            if( aDogTag.IsDead() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjFocusHdl = false;
            break;

        case SALOBJ_EVENT_LOSEFOCUS:
            // trigger a LoseFocus which matches the status
            // of the window with matching Activate-Status
            pWindow->ImplGetFrameData()->mbSysObjFocus = false;
            if ( !pWindow->ImplGetFrameData()->mnFocusId )
            {
                pWindow->ImplGetFrameData()->mbStartFocusState = true;
                pWindow->ImplGetFrameData()->mnFocusId = Application::PostUserEvent( LINK( pWindow->ImplGetFrameWindow(), vcl::Window, ImplAsyncFocusHdl ) );
            }
            break;

        case SALOBJ_EVENT_TOTOP:
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = true;
            if ( !Application::GetFocusWindow() || pWindow->HasChildPathFocus() )
                pWindow->ToTop( TOTOP_NOGRABFOCUS );
            else
                pWindow->ToTop();
            if( aDogTag.IsDead() )
                break;
            pWindow->GrabFocus();
            if( aDogTag.IsDead() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = false;
            break;
    }

    return nRet;
}

void SystemChildWindow::ImplInitSysChild( vcl::Window* pParent, WinBits nStyle, SystemWindowData *pData, bool bShow )
{
    mpWindowImpl->mpSysObj = ImplGetSVData()->mpDefInst->CreateObject( pParent->ImplGetFrame(), pData, bShow );

    Window::ImplInit( pParent, nStyle, NULL );

    // we do not paint if it is the right SysChild
    if ( GetSystemData() )
    {
        mpWindowImpl->mpSysObj->SetCallback( this, ImplSysChildProc );
        SetParentClipMode( PARENTCLIPMODE_CLIP );
        SetBackground();
    }
}

SystemChildWindow::SystemChildWindow( vcl::Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SYSTEMCHILDWINDOW )
{
    ImplInitSysChild( pParent, nStyle, NULL );
}

SystemChildWindow::SystemChildWindow( vcl::Window* pParent, WinBits nStyle, SystemWindowData *pData, bool bShow ) :
    Window( WINDOW_SYSTEMCHILDWINDOW )
{
    ImplInitSysChild( pParent, nStyle, pData, bShow );
}

SystemChildWindow::~SystemChildWindow()
{
    Hide();
    if ( mpWindowImpl->mpSysObj )
    {
        ImplGetSVData()->mpDefInst->DestroyObject( mpWindowImpl->mpSysObj );
        mpWindowImpl->mpSysObj = NULL;
    }
}

const SystemEnvData* SystemChildWindow::GetSystemData() const
{
    if ( mpWindowImpl->mpSysObj )
        return mpWindowImpl->mpSysObj->GetSystemData();
    else
        return NULL;
}

void SystemChildWindow::EnableEraseBackground( bool bEnable )
{
    if ( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->EnableEraseBackground( bEnable );
}

void SystemChildWindow::ImplTestJavaException( void* pEnv )
{
#if HAVE_FEATURE_JAVA
    JNIEnv*     pJavaEnv = reinterpret_cast< JNIEnv* >( pEnv );
    jthrowable  jtThrowable = pJavaEnv->ExceptionOccurred();

    if( jtThrowable )
    { // is it a java exception ?
#if OSL_DEBUG_LEVEL > 1
        pJavaEnv->ExceptionDescribe();
#endif // OSL_DEBUG_LEVEL > 1
        pJavaEnv->ExceptionClear();

        jclass          jcThrowable = pJavaEnv->FindClass("java/lang/Throwable");
        jmethodID       jmThrowable_getMessage = pJavaEnv->GetMethodID(jcThrowable, "getMessage", "()Ljava/lang/String;");
        jstring         jsMessage = static_cast<jstring>( pJavaEnv->CallObjectMethod(jtThrowable, jmThrowable_getMessage) );
        OUString        ouMessage;

        if(jsMessage)
        {
            const jchar * jcMessage = pJavaEnv->GetStringChars(jsMessage, NULL);
            ouMessage = OUString(jcMessage);
            pJavaEnv->ReleaseStringChars(jsMessage, jcMessage);
        }

        throw uno::RuntimeException(ouMessage);
    }
#else
    (void)pEnv;
#endif // HAVE_FEATURE_JAVA
}

void SystemChildWindow::SetForwardKey( bool bEnable )
{
    if ( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetForwardKey( bEnable );
}

sal_IntPtr SystemChildWindow::GetParentWindowHandle( bool bUseJava )
{
    sal_IntPtr nRet = 0;

    (void)bUseJava;
#if defined WNT
    nRet = reinterpret_cast< sal_IntPtr >( GetSystemData()->hWnd );
#elif defined MACOSX
    // FIXME: this is wrong
    nRet = reinterpret_cast< sal_IntPtr >( GetSystemData()->mpNSView );
#elif defined ANDROID
    // Nothing
#elif defined IOS
    // Nothing
#elif defined UNX
    if( !bUseJava )
    {
        nRet = (sal_IntPtr) GetSystemData()->aWindow;
    }
#if HAVE_FEATURE_JAVA
    else
    {
        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        if( GetSystemData()->aWindow > 0 )
        {
            try
            {
                    ::rtl::Reference< ::jvmaccess::VirtualMachine > xVM;
                    uno::Reference< java::XJavaVM >                 xJavaVM = java::JavaVirtualMachine::create(xContext);;
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
                                ImplTestJavaException(pEnv);

                                jmethodID jmToolkit_getDefaultToolkit = pEnv->GetStaticMethodID( jcToolkit, "getDefaultToolkit", "()Ljava/awt/Toolkit;" );
                                ImplTestJavaException(pEnv);

                                pEnv->CallStaticObjectMethod(jcToolkit, jmToolkit_getDefaultToolkit);
                                ImplTestJavaException(pEnv);

                                jclass jcMotifAppletViewer = pEnv->FindClass("sun/plugin/navig/motif/MotifAppletViewer");
                                if( pEnv->ExceptionOccurred() )
                                {
                                    pEnv->ExceptionClear();

                                    jcMotifAppletViewer = pEnv->FindClass( "sun/plugin/viewer/MNetscapePluginContext");
                                    ImplTestJavaException(pEnv);
                                }

                                jclass jcClassLoader = pEnv->FindClass("java/lang/ClassLoader");
                                ImplTestJavaException(pEnv);

                                jmethodID jmClassLoader_loadLibrary = pEnv->GetStaticMethodID( jcClassLoader, "loadLibrary", "(Ljava/lang/Class;Ljava/lang/String;Z)V");
                                ImplTestJavaException(pEnv);

                                jstring jsplugin = pEnv->NewStringUTF("javaplugin_jni");
                                ImplTestJavaException(pEnv);

                                pEnv->CallStaticVoidMethod(jcClassLoader, jmClassLoader_loadLibrary, jcMotifAppletViewer, jsplugin, JNI_FALSE);
                                ImplTestJavaException(pEnv);

                                jmethodID jmMotifAppletViewer_getWidget = pEnv->GetStaticMethodID( jcMotifAppletViewer, "getWidget", "(IIIII)I" );
                                ImplTestJavaException(pEnv);

                                const Size aSize( GetOutputSizePixel() );
                                jint ji_widget = pEnv->CallStaticIntMethod( jcMotifAppletViewer, jmMotifAppletViewer_getWidget,
                                        GetSystemData()->aWindow, 0, 0, aSize.Width(), aSize.Height() );
                                ImplTestJavaException(pEnv);

                                nRet = static_cast< sal_IntPtr >( ji_widget );
                        }
                        catch( uno::RuntimeException& )
                        {
                        }

                        if( !nRet )
                            nRet = static_cast< sal_IntPtr >( GetSystemData()->aWindow );
                    }
            }
            catch( ... )
            {
            }
        }
    }
#endif // HAVE_FEATURE_JAVA
#endif

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
