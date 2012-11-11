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

#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#endif

#include <WindowsAccessBridgeAdapter.h>

#include <wtypes.h>
#include <rtl/process.h>
#include <tools/link.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/sysdata.hxx>
#include <uno/current_context.hxx>
#include <uno/environment.h>
#include <uno/mapping.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

#include "jvmaccess/unovirtualmachine.hxx"

#include "jvmaccess/virtualmachine.hxx"

#include <osl/diagnose.h>

using ::com::sun::star::uno::Mapping;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using namespace ::com::sun::star::accessibility;

long VCLEventListenerLinkFunc(void * pInst, void * pData);

//------------------------------------------------------------------------
// global vatiables
//------------------------------------------------------------------------

Link g_aEventListenerLink(NULL, VCLEventListenerLinkFunc);

rtl::Reference< jvmaccess::UnoVirtualMachine > g_xUnoVirtualMachine;
typelib_InterfaceTypeDescription * g_pTypeDescription = NULL;
Mapping g_unoMapping;

jclass g_jcWindowsAccessBridgeAdapter = NULL;
jmethodID g_jmRegisterTopWindow = 0;
jmethodID g_jmRevokeTopWindow = 0;

//------------------------------------------------------------------------
// functions
//------------------------------------------------------------------------

SAL_DLLPUBLIC_EXPORT jint JNICALL JNI_OnLoad(JavaVM *, void *)
{
    return JNI_VERSION_1_2;
}

SAL_DLLPUBLIC_EXPORT jbyteArray JNICALL
Java_org_openoffice_accessibility_WindowsAccessBridgeAdapter_getProcessID(JNIEnv *pJNIEnv, jclass clazz)
{
    // Initialize global class and method references
    g_jcWindowsAccessBridgeAdapter =
        static_cast< jclass > (pJNIEnv->NewGlobalRef(clazz));
    if (NULL == g_jcWindowsAccessBridgeAdapter) {
        return 0; /* jni error occurred */
    }
    g_jmRegisterTopWindow =
        pJNIEnv->GetStaticMethodID(clazz, "registerTopWindow", "(ILcom/sun/star/accessibility/XAccessible;)V");
    if (0 == g_jmRegisterTopWindow) {
        return 0; /* jni error occurred */
    }
    g_jmRevokeTopWindow =
        pJNIEnv->GetStaticMethodID(clazz, "revokeTopWindow", "(ILcom/sun/star/accessibility/XAccessible;)V");
    if (0 == g_jmRevokeTopWindow) {
        return 0; /* jni error occurred */
    }

    // Use the special protocol of XJavaVM.getJavaVM:  If the passed in
    // process ID has an extra 17th byte of value one, the returned any
    // contains a pointer to a jvmaccess::UnoVirtualMachine, instead of
    // the underlying JavaVM pointer:
    jbyte processID[17];
    rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8 *> (processID));
    // #i51265# we need a jvmaccess::UnoVirtualMachine pointer for the
    // uno_getEnvironment() call later.
    processID[16] = 1;

    // Copy the result into a java byte[] and return.
    jbyteArray jbaProcessID = pJNIEnv->NewByteArray(17);
    pJNIEnv->SetByteArrayRegion(jbaProcessID, 0, 17, processID);
    return jbaProcessID;
}

SAL_DLLPUBLIC_EXPORT jboolean JNICALL
Java_org_openoffice_accessibility_WindowsAccessBridgeAdapter_createMapping(JNIEnv *, jclass, jlong pointer)
{
    uno_Environment * pJava_environment = NULL;
    uno_Environment * pUno_environment = NULL;

    try {
        // We get a non-refcounted pointer to a jvmaccess::VirtualMachine
        // from the XJavaVM service (the pointer is guaranteed to be valid
        // as long as our reference to the XJavaVM service lasts), and
        // convert the non-refcounted pointer into a refcounted one
        // immediately:
        g_xUnoVirtualMachine = reinterpret_cast< jvmaccess::UnoVirtualMachine * >(pointer);

        if ( g_xUnoVirtualMachine.is() )
        {
            OUString sJava(RTL_CONSTASCII_USTRINGPARAM("java"));
            uno_getEnvironment(&pJava_environment, sJava.pData, g_xUnoVirtualMachine.get());

            OUString sCppu_current_lb_name(RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME));
            uno_getEnvironment(&pUno_environment, sCppu_current_lb_name.pData, NULL);

            if ( pJava_environment && pUno_environment )
            {
                g_unoMapping = Mapping(pUno_environment, pJava_environment);
                getCppuType((::com::sun::star::uno::Reference< XAccessible > *) 0).getDescription((typelib_TypeDescription **) & g_pTypeDescription);
            }

            if ( pJava_environment )
            {
                // release java environment
                pJava_environment->release(pJava_environment);
                pJava_environment = NULL;
            }

            if ( pUno_environment )
            {
                // release uno environment
                pUno_environment->release(pUno_environment);
                pUno_environment = NULL;
            }
        }
    }

    catch (const RuntimeException &)
    {
        OSL_TRACE("RuntimeException caught while initializing the mapping");
    }

    if ( (0 != g_jmRegisterTopWindow) && (0 != g_jmRevokeTopWindow) )
    {
        ::Application::AddEventListener(g_aEventListenerLink);
    }
    return JNI_TRUE;
}

SAL_DLLPUBLIC_EXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *)
{
    ::Application::RemoveEventListener(g_aEventListenerLink);

    if ( NULL != g_jcWindowsAccessBridgeAdapter )
    {
        JNIEnv * pJNIEnv;
        if ( ! jvm->GetEnv((void **) &pJNIEnv, JNI_VERSION_1_2) )
        {
            pJNIEnv->DeleteGlobalRef(g_jcWindowsAccessBridgeAdapter);
            g_jcWindowsAccessBridgeAdapter = NULL;
        }
    }

    if ( NULL != g_pTypeDescription )
    {
        typelib_typedescription_release( reinterpret_cast< typelib_TypeDescription * > (g_pTypeDescription) );
        g_pTypeDescription = NULL;
    }

    g_unoMapping.clear();
    g_xUnoVirtualMachine.clear();
}

HWND GetHWND(Window * pWindow)
{
    const SystemEnvData * pEnvData = pWindow->GetSystemData();
    if (pEnvData != NULL)
    {
        return pEnvData->hWnd;
    }
    return (HWND) -1;
}

void handleWindowEvent(Window * pWindow, bool bShow)
{
    if ( pWindow && pWindow->IsTopWindow() )
    {
        ::com::sun::star::uno::Reference< XAccessible > xAccessible;

        // Test for combo box - drop down floating windows first
        Window * pParentWindow = pWindow->GetParent();

        if ( pParentWindow )
        {
            try
            {
                // The parent window of a combo box floating window should have the role COMBO_BOX
                ::com::sun::star::uno::Reference< XAccessible > xParentAccessible(pParentWindow->GetAccessible());
                if ( xParentAccessible.is() )
                {
                    ::com::sun::star::uno::Reference< XAccessibleContext > xParentAC(xParentAccessible->getAccessibleContext());
                    if ( xParentAC.is() && (AccessibleRole::COMBO_BOX == xParentAC->getAccessibleRole()) )
                    {
                        // O.k. - this is a combo box floating window corresponding to the child of role LIST of the parent.
                        // Let's not rely on a specific child order, just search for the child with the role LIST
                        sal_Int32 nCount = xParentAC->getAccessibleChildCount();
                        for ( sal_Int32 n = 0; (n < nCount) && !xAccessible.is(); n++)
                        {
                            ::com::sun::star::uno::Reference< XAccessible > xChild = xParentAC->getAccessibleChild(n);
                            if ( xChild.is() )
                            {
                                ::com::sun::star::uno::Reference< XAccessibleContext > xChildAC = xChild->getAccessibleContext();
                                if ( xChildAC.is() && (AccessibleRole::LIST == xChildAC->getAccessibleRole()) )
                                {
                                    xAccessible = xChild;
                                }
                            }
                        }
                    }
                }
            }
            catch (const ::com::sun::star::uno::RuntimeException &)
            {
                // Ignore show events that throw DisposedExceptions in getAccessibleContext(),
                // but keep revoking these windows in hide(s).
                if (bShow)
                    return;
            }
        }

        // We have to rely on the fact that Window::GetAccessible()->getAccessibleContext() returns a valid XAccessibleContext
        // also for other menus than menubar or toplevel popup window. Otherwise we had to traverse the hierarchy to find the
        // context object to this menu floater. This makes the call to Window->IsMenuFloatingWindow() obsolete.
        if ( ! xAccessible.is() )
            xAccessible = pWindow->GetAccessible();

        if ( xAccessible.is() && g_unoMapping.is() )
        {
            jobject * joXAccessible = reinterpret_cast < jobject * > (g_unoMapping.mapInterface(
                xAccessible.get(), g_pTypeDescription));

            if ( NULL != joXAccessible )
            {
                jvmaccess::VirtualMachine::AttachGuard aGuard(g_xUnoVirtualMachine->getVirtualMachine());
                JNIEnv * pJNIEnv = aGuard.getEnvironment();

                if ( NULL != pJNIEnv )
                {
                    // g_jmRegisterTopWindow and g_jmRevokeTopWindow are ensured to be != 0 - otherwise
                    // the event listener would not have been attached.
                    pJNIEnv->CallStaticVoidMethod(g_jcWindowsAccessBridgeAdapter,
                        (bShow) ? g_jmRegisterTopWindow : g_jmRevokeTopWindow,
                        (jint) GetHWND(pWindow), joXAccessible );

                    // Clear any exception that might have been occurred.
                    if (pJNIEnv->ExceptionCheck()) {
                        pJNIEnv->ExceptionClear();
                    }
                }
            }
        }
    }
}

long VCLEventListenerLinkFunc(void *, void * pData)
{
    ::VclSimpleEvent const * pEvent = (::VclSimpleEvent const *) pData;

    switch (pEvent->GetId())
    {
    case VCLEVENT_WINDOW_SHOW:
        handleWindowEvent(((::VclWindowEvent const *) pEvent)->GetWindow(), true);
        break;
    case VCLEVENT_WINDOW_HIDE:
        handleWindowEvent(((::VclWindowEvent const *) pEvent)->GetWindow(), false);
        break;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
