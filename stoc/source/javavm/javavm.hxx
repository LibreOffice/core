/*************************************************************************
 *
 *  $RCSfile: javavm.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jl $ $Date: 2002-07-23 14:07:21 $
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
#ifndef _JVM_JAVAVM_HXX_
#define _JVM_JAVAVM_HXX_
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COM_SUN_STAR_JAVA_XJAVAVM_HPP_
#include <com/sun/star/java/XJavaVM.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_XJAVATHREADREGISTER_11_HPP_
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif
#ifndef _UNO_ENVIRONMENT_HXX_
#include <uno/environment.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef __VMHOLDER
#include <bridges/java/jvmcontext.hxx>
#endif
//#include <jni.h>
#include "jvmargs.hxx"

using namespace ::cppu;
using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::java;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

namespace stoc_javavm
{

Mutex* javavm_getMutex();
class OCreatorThread;

class JavaVirtualMachine_Impl : public WeakImplHelper4< XJavaVM, XJavaThreadRegister_11,
              XServiceInfo, XContainerListener >
{
    OCreatorThread                *  pCreatorThread;
    uno_Environment               * _pJava_environment;
    JavaVMContext                 * _pVMContext;

    Reference<XComponentContext>        _xCtx;
    Reference<XMultiComponentFactory > _xSMgr;

    // For Inet settings
    Reference<XInterface> _xConfigurationAccess;
    // for Java settings
    Reference<XInterface> _xConfigurationAccess2;
    Module    _javaLib;

    void registerConfigChangesListener();

    sal_Bool m_bInteractionAbort;
    sal_Bool m_bInteractionRetry;

public:
    JavaVirtualMachine_Impl(const Reference<XComponentContext> & xCtx) throw();
    ~JavaVirtualMachine_Impl() throw();


    // XJavaVM
    virtual Any      SAL_CALL getJavaVM(const Sequence<sal_Int8> & processID)   throw(RuntimeException);
    virtual sal_Bool SAL_CALL isVMStarted(void)                                 throw( RuntimeException);
    virtual sal_Bool SAL_CALL isVMEnabled(void)                                 throw( RuntimeException);

    // XJavaThreadRegister_11
    virtual sal_Bool SAL_CALL isThreadAttached(void) throw(RuntimeException);
    virtual void     SAL_CALL registerThread(void)   throw(RuntimeException);
    virtual void     SAL_CALL revokeThread(void)     throw(RuntimeException);

    // XServiceInfo
    virtual OUString           SAL_CALL getImplementationName()                      throw(RuntimeException);
    virtual sal_Bool           SAL_CALL supportsService(const OUString& ServiceName) throw(RuntimeException);
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames(void)               throw(RuntimeException);

    // XContainerListener
    virtual void SAL_CALL elementInserted( const ContainerEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL elementRemoved( const ContainerEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL elementReplaced( const ContainerEvent& Event ) throw (RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    JavaVM *                createJavaVM(const JVM & jvm) throw(RuntimeException);
    void                    disposeJavaVM() throw();


    //callback for the InteractionAbort object
    void selectAbort();
    //callback for the InteractionRetry object
    void selectRetry();
    enum SelectedAction {action_abort,action_retry};
protected:
    void setINetSettingsInVM( sal_Bool set_reset);

    SelectedAction doClientInteraction(Any& exception);

};

} // end namespace
#endif
