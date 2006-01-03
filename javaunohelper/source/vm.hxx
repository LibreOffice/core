/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vm.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-01-03 12:43:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_JAVAUNOHELPER_SOURCE_VM_HXX
#define INCLUDED_JAVAUNOHELPER_SOURCE_VM_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include "jni.h"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }
namespace jvmaccess { class UnoVirtualMachine; }

namespace javaunohelper {

::rtl::Reference< ::jvmaccess::UnoVirtualMachine > create_vm_access(
    JNIEnv * jni_env, jobject loader );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
install_vm_singleton(
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
    const & xContext,
    ::rtl::Reference< ::jvmaccess::UnoVirtualMachine > const & vm_access );

}

#endif
