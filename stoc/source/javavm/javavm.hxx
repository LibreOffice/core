/*************************************************************************
 *
 *  $RCSfile: javavm.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:17:37 $
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

#if !defined INCLUDED_STOC_JAVAVM_JAVAVM_HXX
#define INCLUDED_STOC_JAVAVM_JAVAVM_HXX

#include "jvmargs.hxx"

#include "com/sun/star/container/XContainerListener.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/java/XJavaThreadRegister_11.hpp"
#include "com/sun/star/java/XJavaVM.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/compbase5.hxx"
#include "osl/thread.hxx"
#include "rtl/ref.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace container { class XContainer; }
    namespace uno { class XComponentContext; }
} } }
namespace jvmaccess { class VirtualMachine; }

namespace stoc_javavm {

bool configureJava(const com::sun::star::uno::Reference<
                   com::sun::star::uno::XComponentContext>& xContext);
// The MS compiler needs a typedef here, so the JavaVirtualMachine ctor can call
// its base class ctor:
typedef
cppu::WeakComponentImplHelper5< com::sun::star::lang::XInitialization,
                                com::sun::star::lang::XServiceInfo,
                                com::sun::star::java::XJavaVM,
                                com::sun::star::java::XJavaThreadRegister_11,
                                com::sun::star::container::XContainerListener >
JavaVirtualMachine_Impl;

class JavaVirtualMachine: private osl::Mutex, public JavaVirtualMachine_Impl
{
public:
    explicit JavaVirtualMachine(
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext > const & rContext);

    // XInitialization
    virtual void SAL_CALL
    initialize(com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
                   rArguments)
        throw (com::sun::star::uno::Exception);

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
    supportsService(rtl::OUString const & rServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    // XJavaVM
    virtual com::sun::star::uno::Any SAL_CALL
    getJavaVM(com::sun::star::uno::Sequence< sal_Int8 > const & rProcessId)
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isVMStarted()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isVMEnabled()
        throw (com::sun::star::uno::RuntimeException);

    // XJavaThreadRegister_11
    virtual sal_Bool SAL_CALL isThreadAttached()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL registerThread()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL revokeThread()
        throw (com::sun::star::uno::RuntimeException);

    // XContainerListener
    virtual void SAL_CALL
    disposing(com::sun::star::lang::EventObject const & rSource)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    elementInserted(com::sun::star::container::ContainerEvent const & rEvent)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    elementRemoved(com::sun::star::container::ContainerEvent const & rEvent)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    elementReplaced(com::sun::star::container::ContainerEvent const & rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    JavaVirtualMachine(JavaVirtualMachine &); // not implemented
    void operator =(JavaVirtualMachine); // not implemented

    virtual ~JavaVirtualMachine();

    virtual void SAL_CALL disposing();

    JavaVM * createJavaVM(JVM const & jvm, JNIEnv ** pMainThreadEnv);
        // throws com::sun::star::uno::RuntimeException

    void registerConfigChangesListener();

    void setINetSettingsInVM(bool set_reset);

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        m_xContext;

    // the following are controlled by the 'this' mutex:
    bool m_bDisposed;
    rtl::Reference< jvmaccess::VirtualMachine > m_xVirtualMachine;
    JavaVM * m_pJavaVm;
        // stored as an instance member for backwards compatibility in getJavaVM
    bool m_bDontCreateJvm;
        // If the first creation of Java failed and this flag is set then the
        // next call to getJavaVM throws a RuntimException.  This is useful when
        // the second attempt to create Java might cause a crash.
    com::sun::star::uno::Reference< com::sun::star::container::XContainer >
        m_xInetConfiguration;
    com::sun::star::uno::Reference< com::sun::star::container::XContainer >
        m_xJavaConfiguration; // for Java settings

    osl::ThreadData m_aAttachGuards;
};

}

#endif // INCLUDED_STOC_JAVAVM_JAVAVM_HXX
