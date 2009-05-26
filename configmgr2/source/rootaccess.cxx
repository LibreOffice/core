/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/util/ChangesSet.hpp"
#include "com/sun/star/util/XChangesListener.hpp"
#include "cppu/unotype.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "components.hxx"
#include "lock.hxx"
#include "node.hxx"
#include "rootaccess.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

RootAccess::RootAccess(
    rtl::OUString const & path, rtl::OUString const & locale, bool update):
    path_(path), locale_(locale), update_(update) {}

rtl::OUString RootAccess::getLocale() const {
    return locale_; //TODO: handle locale_ == ""
}

bool RootAccess::isUpdate() const {
    return update_;
}

RootAccess::~RootAccess() {}

rtl::Reference< Node > RootAccess::getNode() {
    if (!node_.is()) {
        node_ = Components::singleton().resolvePath(path_);
        if (!node_.is()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot find ")) +
                 path_),
                static_cast< cppu::OWeakObject * >(this));
        }
    }
    return node_;
}

rtl::Reference< RootAccess > RootAccess::getRoot() {
    return this;
}

void RootAccess::addChangesListener(
    css::uno::Reference< css::util::XChangesListener > const & aListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    rBHelper.addListener(
        cppu::UnoType< css::util::XChangesListener >::get(), aListener);
}

void RootAccess::removeChangesListener(
    css::uno::Reference< css::util::XChangesListener > const & aListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    rBHelper.removeListener(
        cppu::UnoType< css::util::XChangesListener >::get(), aListener);
}

void RootAccess::commitChanges()
    throw (css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY|IS_UPDATE));
    //TODO
}

sal_Bool RootAccess::hasPendingChanges() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_ANY|IS_UPDATE));
    osl::MutexGuard g(lock);
    return false;//TODO
}

css::util::ChangesSet RootAccess::getPendingChanges()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY|IS_UPDATE));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

}
