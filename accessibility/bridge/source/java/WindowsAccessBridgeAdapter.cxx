/*************************************************************************
 *
 *  $RCSfile: WindowsAccessBridgeAdapter.cxx,v $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <WindowsAccessBridgeAdapter.h>

#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>

#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif

#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include <uno/current_context.hxx>
#endif

#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifndef _JVMACCESS_UNOVIRTUALMACHINE_HXX_
#include "jvmaccess/unovirtualmachine.hxx"
#endif

#ifndef _JVMACCESS_VIRTUALMACHINE_HXX_
#include "jvmaccess/virtualmachine.hxx"
#endif

#include <osl/diagnose.h>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
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

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    return JNI_VERSION_1_2;
}

JNIEXPORT jbyteArray JNICALL
Java_org_openoffice_accessibility_WindowsAccessBridgeAdapter_getProcessID(JNIEnv *pJNIEnv, jclass clazz)
{
    // Initialize global class and method references
    g_jcWindowsAccessBridgeAdapter =
        reinterpret_cast< jclass > (pJNIEnv->NewGlobalRef(clazz));
    if (NULL == g_jcWindowsAccessBridgeAdapter) {
        return 0; /* jni error occured */
    }
    g_jmRegisterTopWindow =
        pJNIEnv->GetStaticMethodID(clazz, "registerTopWindow", "(ILcom/sun/star/accessibility/XAccessible;)V");
    if (0 == g_jmRegisterTopWindow) {
        return 0; /* jni error occured */
    }
    g_jmRevokeTopWindow =
        pJNIEnv->GetStaticMethodID(clazz, "revokeTopWindow", "(ILcom/sun/star/accessibility/XAccessible;)V");
    if (0 == g_jmRevokeTopWindow) {
        return 0; /* jni error occured */
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

JNIEXPORT jboolean JNICALL
Java_org_openoffice_accessibility_WindowsAccessBridgeAdapter_createMapping(JNIEnv *pJNIEnv, jclass clazz, jlong pointer)
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
                getCppuType((Reference< XAccessible > *) 0).getDescription((typelib_TypeDescription **) & g_pTypeDescription);
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

    catch ( RuntimeException e)
    {
        OSL_TRACE("RuntimeException caught while initializing the mapping");
    }

    if ( (0 != g_jmRegisterTopWindow) && (0 != g_jmRevokeTopWindow) )
    {
        ::Application::AddEventListener(g_aEventListenerLink);
    }
    return JNI_TRUE;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
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
        Reference< XAccessible > xAccessible;

        // Test for combo box - drop down floating windows first
        Window * pParentWindow = pWindow->GetParent();

        if ( pParentWindow )
        {
            try
            {
                // The parent window of a combo box floating window should have the role COMBO_BOX
                Reference< XAccessible > xParentAccessible(pParentWindow->GetAccessible());
                if ( xParentAccessible.is() )
                {
                    Reference< XAccessibleContext > xParentAC(xParentAccessible->getAccessibleContext());
                    if ( xParentAC.is() && (AccessibleRole::COMBO_BOX == xParentAC->getAccessibleRole()) )
                    {
                        // O.k. - this is a combo box floating window corresponding to the child of role LIST of the parent.
                        // Let's not rely on a specific child order, just search for the child with the role LIST
                        sal_Int32 nCount = xParentAC->getAccessibleChildCount();
                        for ( sal_Int32 n = 0; (n < nCount) && !xAccessible.is(); n++)
                        {
                            Reference< XAccessible > xChild = xParentAC->getAccessibleChild(n);
                            if ( xChild.is() )
                            {
                                Reference< XAccessibleContext > xChildAC = xChild->getAccessibleContext();
                                if ( xChildAC.is() && (AccessibleRole::LIST == xChildAC->getAccessibleRole()) )
                                {
                                    xAccessible = xChild;
                                }
                            }
                        }
                    }
                }
            }
            catch (::com::sun::star::uno::RuntimeException e)
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

                    // Clear any exception that might have been occured.
                    if (pJNIEnv->ExceptionCheck()) {
                        pJNIEnv->ExceptionClear();
                    }
                }
            }
        }
    }
}

long VCLEventListenerLinkFunc(void * pInst, void * pData)
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
