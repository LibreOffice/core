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

#include <vector>

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
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/sequenceasvector.hxx"
#include "cppu/unotype.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "access.hxx"
#include "childaccess.hxx"
#include "components.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedpropertyvaluenode.hxx"
#include "lock.hxx"
#include "node.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"
#include "setnode.hxx"
#include "type.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

css::uno::Type mapType(Type type) {
    switch (type) {
    default: // case TYPE_ERROR: //TODO: can happen?
        return cppu::UnoType< cppu::UnoVoidType >::get();
    case TYPE_NIL: //TODO: can happen?
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

Type mapType(css::uno::Any value) {
    switch (value.getValueType().getTypeClass()) {
    case css::uno::TypeClass_BOOLEAN:
        return TYPE_BOOLEAN;
    case css::uno::TypeClass_BYTE:
        return TYPE_SHORT;
    case css::uno::TypeClass_SHORT:
        return TYPE_SHORT;
    case css::uno::TypeClass_UNSIGNED_SHORT:
        return value.has< sal_Int16 >() ? TYPE_SHORT : TYPE_INT;
    case css::uno::TypeClass_LONG:
        return TYPE_INT;
    case css::uno::TypeClass_UNSIGNED_LONG:
        return value.has< sal_Int32 >() ? TYPE_INT : TYPE_LONG;
    case css::uno::TypeClass_HYPER:
        return TYPE_LONG;
    case css::uno::TypeClass_UNSIGNED_HYPER:
        return value.has< sal_Int64 >() ? TYPE_LONG : TYPE_ERROR;
    case css::uno::TypeClass_FLOAT:
    case css::uno::TypeClass_DOUBLE:
        return TYPE_DOUBLE;
    case css::uno::TypeClass_STRING:
        return TYPE_STRING;
    case css::uno::TypeClass_SEQUENCE: //TODO
        {
            rtl::OUString name(value.getValueType().getTypeName());
            if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]byte"))) {
                return TYPE_HEXBINARY;
            } else if (name.equalsAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("[]boolean")))
            {
                return TYPE_BOOLEAN_LIST;
            } else if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]short")))
            {
                return TYPE_SHORT_LIST;
            } else if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]long")))
            {
                return TYPE_INT_LIST;
            } else if (name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("[]hyper")))
            {
                return TYPE_LONG_LIST;
            } else if (name.equalsAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("[]double")))
            {
                return TYPE_DOUBLE_LIST;
            } else if (name.equalsAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("[]string")))
            {
                return TYPE_STRING_LIST;
            } else if (name.equalsAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("[][]byte")))
            {
                return TYPE_HEXBINARY_LIST;
            }
        }
        // fall through
    default:
        return TYPE_ERROR;
    }
}

}

bool Access::isValue() {
    osl::MutexGuard g(lock);
    rtl::Reference< Node > p(getNode());
    return dynamic_cast< PropertyNode * >(p.get()) != 0 ||
        dynamic_cast< LocalizedPropertyValueNode * >(p.get()) != 0 ||
        (dynamic_cast< LocalizedPropertyNode * >(p.get()) != 0 &&
         !Components::allLocales(getRootAccess()->getLocale()));
}

void Access::releaseChild(rtl::OUString const & name) {
    children_.erase(name);
}

Access::Access(): AccessBase(lock) {}

Access::~Access() {}

css::uno::Type Access::getElementType() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    rtl::Reference< Node > p(getNode());
    if (LocalizedPropertyNode * locprop =
        dynamic_cast< LocalizedPropertyNode * >(p.get()))
    {
        return mapType(locprop->getType());
    } else if (dynamic_cast< GroupNode * >(p.get()) != 0) {
        //TODO: Should a specific type be returned for a non-extensible group
        // with homogeneous members or for an extensible group that currently
        // has only homegeneous members?
        return cppu::UnoType< cppu::UnoVoidType >::get();
    } else if (dynamic_cast< SetNode * >(p.get()) != 0) {
        return cppu::UnoType< cppu::UnoVoidType >::get(); //TODO: correct?
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

sal_Bool Access::hasElements() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    rtl::Reference< Node > p(getNode());
    if (LocalizedPropertyNode * locprop =
        dynamic_cast< LocalizedPropertyNode * >(p.get()))
    {
        return !locprop->getMembers().empty();
    } else if (GroupNode * group = dynamic_cast< GroupNode * >(p.get())) {
        return !group->getMembers().empty();
    } else if (SetNode * set = dynamic_cast< SetNode * >(p.get())) {
        return !set->getMembers().empty();
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

rtl::Reference< Node > Access::getParentNode() {
    rtl::Reference< Access > parent(getParentAccess());
    return parent.is() ? parent->getNode() : rtl::Reference< Node >();
}

css::uno::Any Access::getByName(rtl::OUString const & aName)
    throw (
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    rtl::Reference< ChildAccess > child(getChild(aName));
    if (!child.is()) {
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
}

css::uno::Sequence< rtl::OUString > Access::getElementNames()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    std::vector< rtl::Reference< ChildAccess > > children(getAllChildren());
    comphelper::SequenceAsVector< rtl::OUString > names;
    for (std::vector< rtl::Reference< ChildAccess > >::iterator i(
             children.begin());
         i != children.end(); ++i)
    {
        names.push_back((*i)->getName());
    }
    return names.getAsConstList();
}

sal_Bool Access::hasByName(rtl::OUString const & aName)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    return getChild(aName).is();
}

css::uno::Any Access::getByHierarchicalName(rtl::OUString const & aName)
    throw (css::container::NoSuchElementException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    rtl::Reference< ChildAccess > child(getSubChild(aName));
    if (!child.is()) {
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
}

sal_Bool Access::hasByHierarchicalName(rtl::OUString const & aName)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    osl::MutexGuard g(lock);
    return getSubChild(aName).is();
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
    osl::MutexGuard g(lock);
    std::vector< rtl::Reference< ChildAccess > > children(getAllChildren());
    comphelper::SequenceAsVector< css::beans::Property > properties;
    for (std::vector< rtl::Reference< ChildAccess > >::iterator i(
             children.begin());
         i != children.end(); ++i)
    {
        properties.push_back((*i)->asProperty());
    }
    return properties.getAsConstList();
}

css::beans::Property Access::getPropertyByName(rtl::OUString const & aName)
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    rtl::Reference< ChildAccess > child(getChild(aName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asProperty();
}

sal_Bool Access::hasPropertyByName(rtl::OUString const & Name)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    return getChild(Name).is();
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

void Access::setName(rtl::OUString const & /*aName*/)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::beans::Property Access::getAsProperty() throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    return asProperty();
}

css::uno::Reference< css::beans::XPropertySetInfo > Access::getPropertySetInfo()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    return this;
}

void Access::setPropertyValue(
    rtl::OUString const & aPropertyName, css::uno::Any const & aValue)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    if (!getRootAccess()->isUpdate()) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr setPropertyValue on non-update access")),
            static_cast< cppu::OWeakObject * >(this));
    }
    rtl::Reference< ChildAccess > child(getChild(aPropertyName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    child->setProperty(aValue);
}

css::uno::Any Access::getPropertyValue(rtl::OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    rtl::Reference< ChildAccess > child(getChild(PropertyName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
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
    rtl::OUString const & aHierarchicalPropertyName,
    css::uno::Any const & aValue)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    if (!getRootAccess()->isUpdate()) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr setHierarchicalPropertyName on non-update"
                    " access")),
            static_cast< cppu::OWeakObject * >(this));
    }
    rtl::Reference< ChildAccess > child(getSubChild(aHierarchicalPropertyName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aHierarchicalPropertyName,
            static_cast< cppu::OWeakObject * >(this));
    }
    child->setProperty(aValue);
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

void Access::replaceByName(
    rtl::OUString const & aName, css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP_OR_SET|IS_UPDATE));
    osl::MutexGuard g(lock);
    rtl::Reference< Node > p(getNode());
    if (dynamic_cast< GroupNode * >(p.get()) != 0) {
        try {
            setPropertyValue(aName, aElement);
        } catch (css::beans::UnknownPropertyException & e) {
            throw css::container::NoSuchElementException(e.Message, e.Context);
        } catch (css::beans::PropertyVetoException & e) {
            css::uno::Any ex(cppu::getCaughtException());
            throw css::lang::WrappedTargetException(e.Message, e.Context, ex);
        }
    } else if (dynamic_cast< SetNode * >(p.get()) != 0) {
        if(true)abort();*(char*)0=0;throw 0;//TODO
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

void Access::insertByName(
    rtl::OUString const & aName, css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::ElementExistException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_EXTGROUP_OR_SET|IS_UPDATE));
    osl::MutexGuard g(lock);
    rtl::Reference< Node > p(getNode());
    if (GroupNode * group = dynamic_cast< GroupNode * >(p.get())) {
        if (getChild(aName).is()) {
            throw css::container::ElementExistException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        Type type = mapType(aElement);
        if (type == TYPE_ERROR) {
            throw css::lang::IllegalArgumentException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "configmgr insertByName inappropriate group element")),
                static_cast< cppu::OWeakObject * >(this), 1);
        }
        rtl::Reference< ChildAccess > child(
            new ChildAccess(
                getRootAccess(), this, aName,
                new PropertyNode(type, true, aElement, true)));
        children_[aName] = child.get();
        child->setStatus(ChildAccess::STATUS_ADDED);
        //TODO notify change
    } else if (SetNode * set = dynamic_cast< SetNode * >(p.get())) {
        if (getChild(aName).is()) {
            throw css::container::ElementExistException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        rtl::Reference< ChildAccess > freeAcc;
        css::uno::Reference< css::lang::XUnoTunnel > tunnel;
        aElement >>= tunnel;
        if (tunnel.is()) {
            freeAcc.set(
                reinterpret_cast< ChildAccess * >(
                    tunnel->getSomething(ChildAccess::getTunnelId())));
        }
        if (!freeAcc.is() ||
            freeAcc->getStatus() != ChildAccess::STATUS_UNMODIFIED)
        {
            throw css::lang::IllegalArgumentException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "configmgr insertByName inappropriate set element")),
                static_cast< cppu::OWeakObject * >(this), 1);
        }
        rtl::OUString tmplName;
        if (GroupNode * freeGroup = dynamic_cast< GroupNode * >(
                freeAcc->getNode().get()))
        {
            tmplName = freeGroup->getTemplateName();
        } else if (SetNode * freeSet = dynamic_cast< SetNode * >(
                       freeAcc->getNode().get()))
        {
            tmplName = freeSet->getTemplateName();
        }
        if (!set->isValidTemplate(tmplName)) {
            throw css::lang::IllegalArgumentException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "configmgr insertByName inappropriate set element")),
                static_cast< cppu::OWeakObject * >(this), 1);
        }
        rtl::Reference< RootAccess > root(getRootAccess());
        rtl::Reference< ChildAccess > child(
            new ChildAccess(root, this, aName, freeAcc->getNode()));
        children_[aName] = child.get();
        child->setStatus(ChildAccess::STATUS_ADDED);
        freeAcc->bind(root, this); // must not throw
        //TODO notify change
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

void Access::removeByName(rtl::OUString const & aName)
    throw (
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_EXTGROUP_OR_SET|IS_UPDATE));
    osl::MutexGuard g(lock);
    rtl::Reference< Node > p(getNode());
    if (dynamic_cast< GroupNode * >(p.get()) != 0) {
        rtl::Reference< ChildAccess > child(getChild(aName));
        if (!child.is()) {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        rtl::Reference< PropertyNode > prop(
            dynamic_cast< PropertyNode * >(child->getNode().get()));
        if (!(prop.is() && prop->isExtension())) {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        child->setStatus(ChildAccess::STATUS_REMOVED);
        //TODO notify change
    } else if (dynamic_cast< SetNode * >(p.get()) != 0) {
        rtl::Reference< ChildAccess > child(getChild(aName));
        if (!child.is()) {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        child->setStatus(ChildAccess::STATUS_REMOVED);
/*TODO:
        ChildMap::iterator j(children_.find(aName));
        rtl::Reference< ChildAccess > oldChild;
        ChildMap newChildren;
        if (j != children_.end()) {
            oldChild = j->second;
            newChildren = children_;
            newChildren.erase(aName);
        }
        rtl::Reference< Node > removed(i->second);
        set->getMembers().erase(i);
        removed->unbind(); // this and following must not throw
        if (oldChild.is()) {
            children_.swap(newChildren);
            oldChild->unbind();
        }
*/
        //TODO notify change
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Reference< css::uno::XInterface > Access::createInstance()
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_SET|IS_UPDATE));
    osl::MutexGuard g(lock);
    rtl::OUString tmplName(
        dynamic_cast< SetNode * >(getNode().get())->getDefaultTemplateName());
    rtl::Reference< Node > p(Components::singleton().getTemplate(tmplName));
    if (!p.is()) {
        throw css::uno::Exception(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown template ")) +
             tmplName),
            static_cast< cppu::OWeakObject * >(this));
    }
    return static_cast< cppu::OWeakObject * >(
        new ChildAccess(getRootAccess(), p->clone()));
}

css::uno::Reference< css::uno::XInterface > Access::createInstanceWithArguments(
    css::uno::Sequence< css::uno::Any > const & aArguments)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_SET|IS_UPDATE));
    if (aArguments.getLength() != 0) {
        throw css::uno::Exception(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configuration SimpleSetUpdate createInstanceWithArguments"
                    " must not specify any arguments")),
            static_cast< cppu::OWeakObject * >(this));
    }
    return createInstance();
}

rtl::Reference< ChildAccess > Access::getChild(rtl::OUString const & name) {
    WeakChildMap::iterator i(children_.find(name));
    if (i != children_.end()) {
        rtl::Reference< ChildAccess > child;
        if (i->second->acquireCounting() > 1) {
            child.set(i->second); // must not throw
        }
        i->second->releaseNondeleting();
        if (child.is()) {
            return child->getStatus() == ChildAccess::STATUS_REMOVED
                ? rtl::Reference< ChildAccess >() : child;
        }
    }
    rtl::Reference< Node > node(getNode()->getMember(name));
    if (!node.is()) {
        return rtl::Reference< ChildAccess >();
    }
    rtl::Reference< ChildAccess > child(
        new ChildAccess(getRootAccess(), this, name, node));
    children_[name] = child.get();
    return child;
}

std::vector< rtl::Reference< ChildAccess > > Access::getAllChildren() {
    NodeMap * members;
    rtl::Reference< Node > p(getNode());
    if (LocalizedPropertyNode * locprop =
        dynamic_cast< LocalizedPropertyNode * >(p.get()))
    {
        members = &locprop->getMembers();
    } else if (GroupNode * group = dynamic_cast< GroupNode * >(p.get())) {
        members = &group->getMembers();
    } else if (SetNode * set = dynamic_cast< SetNode * >(p.get())) {
        members = &set->getMembers();
    } else {
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            static_cast< cppu::OWeakObject * >(this));
    }
    std::vector< rtl::Reference< ChildAccess > > vec;
    for (NodeMap::iterator i(members->begin()); i != members->end(); ++i) {
        if (modifiedChildren_.find(i->first) == modifiedChildren_.end()) {
            vec.push_back(getChild(i->first));
        }
    }
    for (HardChildMap::iterator i(modifiedChildren_.begin());
         i != modifiedChildren_.end(); ++i)
    {
        if (i->second->getStatus() != ChildAccess::STATUS_REMOVED) {
            vec.push_back(i->second);
        }
    }
    return vec;
}

rtl::Reference< ChildAccess > Access::getSubChild(rtl::OUString const & path) {
    sal_Int32 i = path.indexOf('/');
    rtl::OUString name;
    bool setElement;
    rtl::OUString templateName;
    if (!Components::parseSegment(
            i == -1 ? path : path.copy(0, i), &name, &setElement,
            &templateName))
    {
        return rtl::Reference< ChildAccess >();
    }
    rtl::Reference< ChildAccess > child(getChild(name));
    if (!child.is()) {
        return rtl::Reference< ChildAccess >();
    }
    if (setElement) {
        rtl::Reference< Node > p(getNode());
        SetNode * set = dynamic_cast< SetNode * >(p.get());
        LocalizedPropertyNode * locprop =
            Components::allLocales(getRootAccess()->getLocale())
            ? dynamic_cast< LocalizedPropertyNode * >(p.get()) : 0;
        if ((set == 0 && locprop == 0) ||
            (set != 0 && templateName.getLength() != 0 &&
             !set->isValidTemplate(templateName)) ||
            (locprop != 0 && templateName.getLength() != 0))
        {
            return rtl::Reference< ChildAccess >();
        }
    }
    // For backwards compatibility, ignore a final slash after non-value nodes:
    return child->isValue()
        ? (i == -1 ? child : rtl::Reference< ChildAccess >())
        : (i == -1 || i + 1 == path.getLength()
           ? child : child->getSubChild(path.copy(i + 1)));
}

css::beans::Property Access::asProperty() {
    css::uno::Type type;
    bool nillable;
    bool removable;
    rtl::Reference< Node > p(getNode());
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(p.get())) {
        type = mapType(prop->getType());
        nillable = prop->isNillable();
        removable = prop->isExtension();
    } else if (dynamic_cast< LocalizedPropertyValueNode * >(p.get()) != 0) {
        LocalizedPropertyNode * locprop =
            dynamic_cast< LocalizedPropertyNode * >(getParentNode().get());
        OSL_ASSERT(locprop != 0);
        type = mapType(locprop->getType());
        nillable = locprop->isNillable();
        removable = false; //TODO ???
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(p.get()))
    {
        if (Components::allLocales(getRootAccess()->getLocale())) {
            type = cppu::UnoType< css::uno::XInterface >::get();
                //TODO: correct?
            removable = false;
        } else {
            type = mapType(locprop->getType());
            removable = false; //TODO ???
        }
        nillable = locprop->isNillable();
    } else {
        OSL_ASSERT(
            dynamic_cast< GroupNode * >(p.get()) != 0 ||
            dynamic_cast< SetNode * >(p.get()) != 0);
        type = cppu::UnoType< css::uno::XInterface >::get(); //TODO: correct?
        nillable = false;
        removable = dynamic_cast< SetNode * >(getParentNode().get()) != 0;
    }
    return css::beans::Property(
        getName(), -1, type,
        (css::beans::PropertyAttribute::BOUND | //TODO: correct for group/set?
         css::beans::PropertyAttribute::CONSTRAINED |
         (nillable ? css::beans::PropertyAttribute::MAYBEVOID : 0) |
         (getRootAccess()->isUpdate()
          ? (removable ? css::beans::PropertyAttribute::REMOVEABLE : 0)
          : css::beans::PropertyAttribute::READONLY))); //TODO: MAYBEDEFAULT
}

void Access::setProperty(css::uno::Any const & value) {
    Type type = TYPE_ERROR;
    bool nillable = false;
    rtl::Reference< Node > p(getNode());
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(p.get())) {
        type = prop->getType();
        nillable = prop->isNillable();
    } else if (dynamic_cast< LocalizedPropertyValueNode * >(p.get()) != 0) {
        LocalizedPropertyNode * locprop =
            dynamic_cast< LocalizedPropertyNode * >(getParentNode().get());
        OSL_ASSERT(locprop != 0);
        type = locprop->getType();
        nillable = locprop->isNillable();
    } else if (LocalizedPropertyNode * locprop =
               dynamic_cast< LocalizedPropertyNode * >(p.get()))
    {
        if (!Components::allLocales(getRootAccess()->getLocale())) {
            type = locprop->getType();
            nillable = locprop->isNillable();
        }
    }
    bool ok;
    switch (type) {
    case TYPE_ERROR:
        ok = false;
        break;
    case TYPE_ANY:
        switch (mapType(value)) {
        case TYPE_ERROR:
            ok = false;
            break;
        case TYPE_NIL:
            ok = nillable;
            break;
        case TYPE_ANY:
            OSL_ASSERT(false);
            // fall through (cannot happen)
        default:
            ok = true;
            break;
        }
        break;
    case TYPE_NIL:
        OSL_ASSERT(false);
        // fall through (cannot happen)
    default:
        ok = value.hasValue() ? value.isExtractableTo(mapType(type)) : nillable;
        break;
    }
    if (!ok) {
        throw css::lang::IllegalArgumentException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr inappropriate setProperty")),
            static_cast< cppu::OWeakObject * >(this), -1);
    }
    dynamic_cast< ChildAccess * >(this)->setStatus(
        ChildAccess::STATUS_CHANGED, value);
    //TODO notify change
}

#if OSL_DEBUG_LEVEL > 0
bool Access::thisIs(int what) {
    osl::MutexGuard g(lock);
    rtl::Reference< Node > p(getNode());
    return !isValue() &&
        ((what & IS_GROUP) == 0 || dynamic_cast< GroupNode * >(p.get()) != 0) &&
        ((what & IS_SET) == 0 || dynamic_cast< SetNode * >(p.get()) != 0) &&
        ((what & IS_GROUP_OR_SET) == 0 ||
         dynamic_cast< GroupNode * >(p.get()) != 0 ||
         dynamic_cast< SetNode * >(p.get()) != 0) &&
        ((what & IS_EXTGROUP_OR_SET) == 0 ||
         (dynamic_cast< GroupNode * >(p.get()) != 0 &&
          dynamic_cast< GroupNode * >(p.get())->isExtensible()) ||
         dynamic_cast< SetNode * >(p.get()) != 0) &&
        ((what & IS_GROUP_MEMBER) == 0 ||
         dynamic_cast< GroupNode * >(getParentNode().get()) != 0) ||
        ((what & IS_SET_MEMBER) == 0 ||
         dynamic_cast< SetNode * >(getParentNode().get()) != 0) ||
         ((what & IS_UPDATE) == 0 || getRootAccess()->isUpdate());
}
#endif

}
