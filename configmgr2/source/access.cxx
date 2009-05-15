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

#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XPropertiesChangeListener.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/beans/XVetoableChangeListener.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XContainerListener.hpp"
#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/ChangesSet.hpp"
#include "com/sun/star/util/XChangesListener.hpp"
#include "comphelper/sequenceasvector.hxx"
#include "cppu/unotype.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "access.hxx"
#include "components.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "lock.hxx"
#include "node.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"
#include "setnode.hxx"

#if !defined INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_13
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_13
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 13
#include "comphelper/implbase_var.hxx"
#undef COMPHELPER_IMPLBASE_INTERFACE_NUMBER
#endif

namespace configmgr {

namespace {

namespace css = com::sun::star;

css::uno::Type mapType(Type type) {
    switch (type) {
    case TYPE_NONE: //TODO: can happen?
        return cppu::UnoType< cppu::UnoVoidType >::get();
    case TYPE_BOOLEAN:
        return cppu::UnoType< sal_Bool >::get();
    case TYPE_SHORT:
        return cppu::UnoType< sal_Int16 >::get();
    case TYPE_INT:
        return cppu::UnoType< sal_Int32 >::get();
    case TYPE_LONG:
        return cppu::UnoType< sal_Int64 >::get();
    case TYPE_DOUBLE:
        return cppu::UnoType< double >::get();
    case TYPE_STRING:
        return cppu::UnoType< rtl::OUString >::get();
    case TYPE_HEXBINARY:
        return cppu::UnoType< css::uno::Sequence< sal_Int8 > >::get();
    case TYPE_ANY: //TODO: can happen?
        return cppu::UnoType< css::uno::Any >::get();
    case TYPE_BOOLEAN_LIST:
        return cppu::UnoType< css::uno::Sequence< sal_Bool > >::get();
    case TYPE_SHORT_LIST:
        return cppu::UnoType< css::uno::Sequence< sal_Int16 > >::get();
    case TYPE_INT_LIST:
        return cppu::UnoType< css::uno::Sequence< sal_Int32 > >::get();
    case TYPE_LONG_LIST:
        return cppu::UnoType< css::uno::Sequence< sal_Int64 > >::get();
    case TYPE_DOUBLE_LIST:
        return cppu::UnoType< css::uno::Sequence< double > >::get();
    case TYPE_STRING_LIST:
        return cppu::UnoType< css::uno::Sequence< rtl::OUString > >::get();
    case TYPE_HEXBINARY_LIST:
        return cppu::UnoType<
            css::uno::Sequence< css::uno::Sequence< sal_Int8 > > >::get();
    }
}

}

Access::Access(RootAccess const * root, Node * node):
    WeakComponentImplHelper13(*static_cast< osl::Mutex * >(this)), root_(root),
    node_(node)
{
    OSL_ASSERT(root != 0 && node != 0);
}

Access::Access(RootAccess const * root):
    WeakComponentImplHelper13(*static_cast< osl::Mutex * >(this)), root_(root),
    node_(0)
{
    OSL_ASSERT(root != 0);
}

Access::~Access() {}

Node * Access::getNode() {
    return node_;
}

css::uno::Any Access::getByHierarchicalName(rtl::OUString const & aName)
    throw (css::container::NoSuchElementException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    Node * p = Components::singleton().resolvePath(getNode(), aName);
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(p)) {
        return prop->getValue();
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(p))
    {
        rtl::OUString loc(root_->getLocale());
        return loc.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("*"))
            ? css::uno::makeAny(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(
                        new Access(root_, locprop))))
            : locprop->getValue(loc);
    } else if (GroupNode * group = dynamic_cast< GroupNode * >(p)) {
        return css::uno::makeAny(
            css::uno::Reference< css::uno::XInterface >(
                static_cast< cppu::OWeakObject * >(new Access(root_, group))));
    } else if (SetNode * set = dynamic_cast< SetNode * >(p)) {
        return css::uno::makeAny(
            css::uno::Reference< css::uno::XInterface >(
                static_cast< cppu::OWeakObject * >(new Access(root_, set))));
    } else {
        OSL_ASSERT(p == 0);
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
}

sal_Bool Access::hasByHierarchicalName(rtl::OUString const & aName)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    return Components::singleton().resolvePath(getNode(), aName) != 0;
}

void Access::addContainerListener(
    css::uno::Reference< css::container::XContainerListener > const & xListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    rBHelper.addListener(
        cppu::UnoType< css::container::XContainerListener >::get(), xListener);
}

void Access::removeContainerListener(
    css::uno::Reference< css::container::XContainerListener > const & xListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    rBHelper.removeListener(
        cppu::UnoType< css::container::XContainerListener >::get(), xListener);
}

rtl::OUString Access::getExactName(rtl::OUString const & /*aApproximateName*/)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::uno::Sequence< css::beans::Property > Access::getProperties()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::beans::Property Access::getPropertyByName(rtl::OUString const & aName)
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    Node * p = getNode()->getMember(aName);
    if (p == 0) {
/*SB*/fprintf(stderr,"Unknown A <%s>\n",rtl::OUStringToOString(aName,RTL_TEXTENCODING_UTF8).getStr());
        throw css::beans::UnknownPropertyException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    css::uno::Type type;
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(p)) {
        type = mapType(prop->getType());
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(p))
    {
        if (root_->getLocale().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("*"))) {
            type = cppu::UnoType< css::uno::XInterface >::get();
                //TODO: correct?
        } else {
            type = mapType(locprop->getType());
        }
    } else {
        OSL_ASSERT(
            dynamic_cast< GroupNode * >(p) != 0 ||
            dynamic_cast< SetNode * >(p) != 0);
        type = cppu::UnoType< css::uno::XInterface >::get(); //TODO: correct?
    }
    return css::beans::Property(
        aName, -1, type,
        (css::beans::PropertyAttribute::BOUND | //TODO: correct for group/set?
         css::beans::PropertyAttribute::CONSTRAINED));
        //TODO: MAYBEVOID, READONLY, MAYBEDEFAULT, REMOVEABLE
}

sal_Bool Access::hasPropertyByName(rtl::OUString const & Name)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    return hasByName(Name);
}

rtl::OUString Access::getHierarchicalName() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

rtl::OUString Access::composeHierarchicalName(
    rtl::OUString const & /*aRelativeName*/)
    throw (
        css::lang::IllegalArgumentException, css::lang::NoSupportException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

rtl::OUString Access::getName() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

void Access::setName(rtl::OUString const & /*aName*/)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::uno::Reference< css::beans::XPropertySetInfo > Access::getPropertySetInfo()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    return this;
}

void Access::setPropertyValue(
    rtl::OUString const & /*aPropertyName*/, css::uno::Any const & /*aValue*/)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    //TODO
}

css::uno::Any Access::getPropertyValue(rtl::OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    Node * p = getNode()->getMember(PropertyName);
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(p)) {
        return prop->getValue();
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(p))
    {
        rtl::OUString loc(root_->getLocale());
        return loc.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("*"))
            ? css::uno::makeAny(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(
                        new Access(root_, locprop))))
            : locprop->getValue(loc);
    } else if (GroupNode * group = dynamic_cast< GroupNode * >(p)) {
        return css::uno::makeAny(
            css::uno::Reference< css::uno::XInterface >(
                static_cast< cppu::OWeakObject * >(new Access(root_, group))));
    } else if (SetNode * set = dynamic_cast< SetNode * >(p)) {
        return css::uno::makeAny(
            css::uno::Reference< css::uno::XInterface >(
                static_cast< cppu::OWeakObject * >(new Access(root_, set))));
    } else {
/*SB*/fprintf(stderr,"Unknown B <%s>\n",rtl::OUStringToOString(PropertyName,RTL_TEXTENCODING_UTF8).getStr());
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
}

void Access::addPropertyChangeListener(
    rtl::OUString const & /*aPropertyName*/, //TODO
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        xListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.addListener(
        cppu::UnoType< css::beans::XPropertyChangeListener >::get(), xListener);
}

void Access::removePropertyChangeListener(
    rtl::OUString const & /*aPropertyName*/, //TODO
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.removeListener(
        cppu::UnoType< css::beans::XPropertyChangeListener >::get(), aListener);
}

void Access::addVetoableChangeListener(
    rtl::OUString const & /*PropertyName*/, //TODO
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.addListener(
        cppu::UnoType< css::beans::XVetoableChangeListener >::get(), aListener);
}

void Access::removeVetoableChangeListener(
    rtl::OUString const & /*PropertyName*/, //TODO
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.removeListener(
        cppu::UnoType< css::beans::XVetoableChangeListener >::get(), aListener);
}

void Access::setPropertyValues(
    css::uno::Sequence< rtl::OUString > const & /*aPropertyNames*/,
    css::uno::Sequence< css::uno::Any > const & /*aValues*/)
    throw (
        css::beans::PropertyVetoException, css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::uno::Sequence< css::uno::Any > Access::getPropertyValues(
    css::uno::Sequence< rtl::OUString > const & /*aPropertyNames*/)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

void Access::addPropertiesChangeListener(
    css::uno::Sequence< rtl::OUString > const & /*aPropertyNames*/, //TODO
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        xListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.addListener(
        cppu::UnoType< css::beans::XPropertiesChangeListener >::get(),
        xListener);
}

void Access::removePropertiesChangeListener(
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        xListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.removeListener(
        cppu::UnoType< css::beans::XPropertiesChangeListener >::get(),
        xListener);
}

void Access::firePropertiesChangeEvent(
    css::uno::Sequence< rtl::OUString > const & /*aPropertyNames*/,
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        /*xListener*/)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::uno::Reference< css::beans::XHierarchicalPropertySetInfo >
Access::getHierarchicalPropertySetInfo() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

void Access::setHierarchicalPropertyValue(
    rtl::OUString const & /*aHierarchicalPropertyName*/,
    css::uno::Any const & /*aValue*/)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    //TODO
}

css::uno::Any Access::getHierarchicalPropertyValue(
    rtl::OUString const & /*aHierarchicalPropertyName*/)
    throw (
        css::beans::UnknownPropertyException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

void Access::setHierarchicalPropertyValues(
    css::uno::Sequence< rtl::OUString > const & /*aHierarchicalPropertyNames*/,
    css::uno::Sequence< css::uno::Any > const & /*Values*/)
    throw (
        css::beans::PropertyVetoException, css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::uno::Sequence< css::uno::Any > Access::getHierarchicalPropertyValues(
    css::uno::Sequence< rtl::OUString > const & /*aHierarchicalPropertyNames*/)
    throw (
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

void Access::addChangesListener(
    css::uno::Reference< css::util::XChangesListener > const & aListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET|IS_ROOT));
    rBHelper.addListener(
        cppu::UnoType< css::util::XChangesListener >::get(), aListener);
}

void Access::removeChangesListener(
    css::uno::Reference< css::util::XChangesListener > const & aListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET|IS_ROOT));
    rBHelper.removeListener(
        cppu::UnoType< css::util::XChangesListener >::get(), aListener);
}

css::uno::Type Access::getElementType() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    Node * p = getNode();
    if (dynamic_cast< GroupNode * >(p) != 0) {
        //TODO: Should a specific type be returned for a non-extensible group
        // with homogeneous members or for an extensible group that currently
        // has only homegeneous members?
        return cppu::UnoType< cppu::UnoVoidType >::get();
    } else if (dynamic_cast< SetNode * >(p) != 0) {
        if(true)abort();*(char*)0=0;throw 0;//TODO
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            0);
    }
}

sal_Bool Access::hasElements() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    Node * p = getNode();
    if (GroupNode * group = dynamic_cast< GroupNode * >(p)) {
        return !group->getMembers().empty();
    } else if (SetNode * set = dynamic_cast< SetNode * >(p)) {
        return !set->getMembers().empty();
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            0);
    }
}

css::uno::Any Access::getByName(rtl::OUString const & aName)
    throw (
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    Node * p = getNode()->getMember(aName);
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(p)) {
        return prop->getValue();
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(p))
    {
        rtl::OUString loc(root_->getLocale());
        return loc.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("*"))
            ? css::uno::makeAny(
                css::uno::Reference< css::uno::XInterface >(
                    static_cast< cppu::OWeakObject * >(
                        new Access(root_, locprop))))
            : locprop->getValue(loc);
    } else if (GroupNode * group = dynamic_cast< GroupNode * >(p)) {
        return css::uno::makeAny(
            css::uno::Reference< css::uno::XInterface >(
                static_cast< cppu::OWeakObject * >(new Access(root_, group))));
    } else if (SetNode * set = dynamic_cast< SetNode * >(p)) {
        return css::uno::makeAny(
            css::uno::Reference< css::uno::XInterface >(
                static_cast< cppu::OWeakObject * >(new Access(root_, set))));
    } else {
        OSL_ASSERT(p == 0);
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Sequence< rtl::OUString > Access::getElementNames()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    Node * p = getNode();
    if (GroupNode * group = dynamic_cast< GroupNode * >(p)) {
        comphelper::SequenceAsVector< rtl::OUString > names;
        for (NodeMap::iterator i(group->getMembers().begin());
             i != group->getMembers().end(); ++i)
        {
            names.push_back(i->first);
        }
        return names.getAsConstList();
    } else if (SetNode * set = dynamic_cast< SetNode * >(p)) {
        comphelper::SequenceAsVector< rtl::OUString > names;
        for (NodeMap::iterator i(set->getMembers().begin());
             i != set->getMembers().end(); ++i)
        {
            names.push_back(i->first);
        }
        return names.getAsConstList();
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            0);
    }
}

sal_Bool Access::hasByName(rtl::OUString const & aName)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    return getNode()->getMember(aName) != 0;
}

void Access::replaceByName(
    rtl::OUString const & aName, css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET|IS_UPDATE));
    osl::MutexGuard g(lock);
    Node * p = getNode();
    if (dynamic_cast< GroupNode * >(p) != 0) {
        try {
            setPropertyValue(aName, aElement);
        } catch (css::beans::UnknownPropertyException & e) {
            throw css::container::NoSuchElementException(e.Message, e.Context);
        } catch (css::beans::PropertyVetoException & e) {
            css::uno::Any ex(cppu::getCaughtException());
            throw css::lang::WrappedTargetException(e.Message, e.Context, ex);
        }
    } else if (dynamic_cast< SetNode * >(p) != 0) {
        if(true)abort();*(char*)0=0;throw 0;//TODO
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            0);
    }
}

void Access::insertByName(
    rtl::OUString const & /*aName*/, css::uno::Any const & /*aElement*/)
    throw (
        css::lang::IllegalArgumentException,
        css::container::ElementExistException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_SET|IS_UPDATE));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

void Access::removeByName(rtl::OUString const & /*aName*/)
    throw (
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_SET|IS_UPDATE));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

void Access::commitChanges()
    throw (css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET|IS_ROOT|IS_UPDATE));
    //TODO
}

sal_Bool Access::hasPendingChanges() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET|IS_ROOT|IS_UPDATE));
    return false;//TODO
}

css::util::ChangesSet Access::getPendingChanges()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET|IS_ROOT|IS_UPDATE));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

#if OSL_DEBUG_LEVEL > 0
bool Access::thisIs(int what) {
    osl::MutexGuard g(lock);
    Node * p = getNode();
    return ((what & IS_GROUP) == 0 || dynamic_cast< GroupNode * >(p) != 0) &&
        ((what & IS_SET) == 0 || dynamic_cast< SetNode * >(p) != 0) &&
        ((what & IS_GROUP_OR_SET) == 0 || dynamic_cast< GroupNode * >(p) != 0 ||
         dynamic_cast< SetNode * >(p) != 0);
}
#endif

}
