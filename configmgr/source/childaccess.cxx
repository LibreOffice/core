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

#include "sal/config.h"

#include <cassert>
#include <vector>

#include "com/sun/star/container/XChild.hpp"
#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppu/unotype.hxx"
#include "cppuhelper/queryinterface.hxx"
#include "cppuhelper/weak.hxx"
#include "comphelper/servicehelper.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "access.hxx"
#include "childaccess.hxx"
#include "components.hxx"
#include "data.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "lock.hxx"
#include "modifications.hxx"
#include "node.hxx"
#include "path.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"
#include "setnode.hxx"
#include "type.hxx"

namespace configmgr {

namespace
{
    class theChildAccessUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theChildAccessUnoTunnelId > {};
}

css::uno::Sequence< sal_Int8 > ChildAccess::getTunnelId()
{
    return theChildAccessUnoTunnelId::get().getSeq();
}

ChildAccess::ChildAccess(
    Components & components, rtl::Reference< RootAccess > const & root,
    rtl::Reference< Access > const & parent, OUString const & name,
    rtl::Reference< Node > const & node):
    Access(components), m_rRoot(root), m_rParent(parent), m_sName(name), m_rNode(node),
    m_bInTransaction(false)
{
    m_pLock = lock();
    assert(root.is() && parent.is() && node.is());
}

ChildAccess::ChildAccess(
    Components & components, rtl::Reference< RootAccess > const & root,
    rtl::Reference< Node > const & node):
    Access(components), m_rRoot(root), m_rNode(node), m_bInTransaction(false)
{
    m_pLock = lock();
    assert(root.is() && node.is());
}

Path ChildAccess::getAbsolutePath()
{
    assert(getParentAccess().is());
    Path path(getParentAccess()->getAbsolutePath());
    path.push_back(m_sName);
    return path;
}

Path ChildAccess::getRelativePath()
{
    Path path;
    rtl::Reference< Access > parent(getParentAccess());
    if (parent.is())
    {
        path = parent->getRelativePath();
    }
    path.push_back(m_sName);
    return path;
}

OUString ChildAccess::getRelativePathRepresentation()
{
    OUStringBuffer path;
    rtl::Reference< Access > parent(getParentAccess());
    if (parent.is())
    {
        path.append(parent->getRelativePathRepresentation());
        if (!path.isEmpty())
        {
            path.append('/');
        }
    }
    path.append(Data::createSegment(m_rNode->getTemplateName(), m_sName));
    return path.makeStringAndClear();
}

rtl::Reference< Node > ChildAccess::getNode()
{
    return m_rNode;
}

bool ChildAccess::isFinalized()
{
    return m_rNode->getFinalized() != Data::NO_LAYER ||
        (m_rParent.is() && m_rParent->isFinalized());
}

OUString ChildAccess::getNameInternal()
{
    return m_sName;
}

rtl::Reference< RootAccess > ChildAccess::getRootAccess()
{
    return m_rRoot;
}

rtl::Reference< Access > ChildAccess::getParentAccess()
{
    return m_rParent;
}

void ChildAccess::acquire() throw ()
{
    Access::acquire();
}

void ChildAccess::release() throw ()
{
    Access::release();
}

css::uno::Reference< css::uno::XInterface > ChildAccess::getParent()
    throw (css::uno::RuntimeException)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*m_pLock);
    checkLocalizedPropertyAccess();
    return static_cast< cppu::OWeakObject * >(m_rParent.get());
}

void ChildAccess::setParent(css::uno::Reference< css::uno::XInterface > const &)
    throw (css::lang::NoSupportException, css::uno::RuntimeException)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*m_pLock);
    checkLocalizedPropertyAccess();
    throw css::lang::NoSupportException(
        "setParent", static_cast< cppu::OWeakObject * >(this));
}

sal_Int64 ChildAccess::getSomething(
    css::uno::Sequence< sal_Int8 > const & aIdentifier)
    throw (css::uno::RuntimeException)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*m_pLock);
    checkLocalizedPropertyAccess();
    return aIdentifier == getTunnelId()
        ? reinterpret_cast< sal_Int64 >(this) : 0;
}

void ChildAccess::bind(
    rtl::Reference< RootAccess > const & root,
    rtl::Reference< Access > const & parent, OUString const & name)
    throw ()
{
    assert(!m_rParent.is() && root.is() && parent.is() && !name.isEmpty());
    m_rRoot = root;
    m_rParent = parent;
    m_sName = name;
}

void ChildAccess::unbind() throw ()
{
    assert(m_rParent.is());
    m_rParent->releaseChild(m_sName);
    m_rParent.clear();
    m_bInTransaction = true;
}

void ChildAccess::committed()
{
    m_bInTransaction = false;
}

void ChildAccess::setNode(rtl::Reference< Node > const & node)
{
    m_rNode = node;
}

void ChildAccess::setProperty( css::uno::Any const & value,
                               Modifications * localModifications)
{
    assert(localModifications != 0);
    Type type = TYPE_ERROR;
    bool nillable = false;
    switch (m_rNode->kind())
    {
    case Node::KIND_PROPERTY:
        {
            PropertyNode * prop = dynamic_cast< PropertyNode * >(m_rNode.get());
            if(prop)
            {
                type = prop->getStaticType();
                nillable = prop->isNillable();
            }
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            OUString locale(getRootAccess()->getLocale());
            if (!Components::allLocales(locale))
            {
                rtl::Reference< ChildAccess > child(getChild(locale));
                if (child.is())
                {
                    child->setProperty(value, localModifications);
                }
                else
                {
                    insertLocalizedValueChild( locale, value, localModifications);
                }
                return;
            }
        }
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            LocalizedPropertyNode * locprop =
                dynamic_cast< LocalizedPropertyNode * >(getParentNode().get());
            if(locprop)
            {
                type = locprop->getStaticType();
                nillable = locprop->isNillable();
            }
        }
        break;
    default:
        break;
    }
    checkValue(value, type, nillable);
    getParentAccess()->markChildAsModified(this);
    m_changedValue.reset(new css::uno::Any(value));
    localModifications->add(getRelativePath());
}

css::uno::Any ChildAccess::asValue()
{
    if (m_changedValue.get() != 0)
    {
        return *m_changedValue;
    }
    switch (m_rNode->kind())
    {
    case Node::KIND_PROPERTY:
        {
            PropertyNode* propnode = dynamic_cast< PropertyNode * >(m_rNode.get());
            if(propnode)
            {
                return propnode->getValue(getComponents());
            }
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            OUString locale(getRootAccess()->getLocale());
            if (!Components::allLocales(locale))
            {
                rtl::Reference< ChildAccess > child(getChild("*" + locale));
                // As a last resort, return a nil value even though it may be
                // illegal for the given property:
                return child.is() ? child->asValue() : css::uno::Any();
            }
        }
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            LocalizedValueNode* locnode = dynamic_cast< LocalizedValueNode * >(m_rNode.get());
            if(locnode)
            {
                return locnode->getValue();
            }
        }
        break;
    default:
        break;
    }
    return css::uno::makeAny(
        css::uno::Reference< css::uno::XInterface >(
            static_cast< cppu::OWeakObject * >(this)));
}

void ChildAccess::commitChanges(bool valid, Modifications * globalModifications)
{
    assert(globalModifications != 0);
    commitChildChanges(valid, globalModifications);
    if (valid && m_changedValue.get() != 0)
    {
        Path path(getAbsolutePath());
        getComponents().addModification(path);
        globalModifications->add(path);
        switch (m_rNode->kind())
        {
        case Node::KIND_PROPERTY:
            {
                PropertyNode* propnode = dynamic_cast< PropertyNode * >(m_rNode.get());
                if(propnode)
                {
                    propnode->setValue( Data::NO_LAYER, *m_changedValue);
                }
            }
            break;
        case Node::KIND_LOCALIZED_VALUE:
            {
                LocalizedValueNode* locnode = dynamic_cast< LocalizedValueNode * >(m_rNode.get());
                if(locnode)
                {
                    locnode->setValue( Data::NO_LAYER, *m_changedValue);
                }
            }
            break;
        default:
            assert(false); // this cannot happen
            break;
        }
    }
    m_changedValue.reset();
}

ChildAccess::~ChildAccess()
{
    osl::MutexGuard g(*m_pLock);
    if (m_rParent.is())
    {
        m_rParent->releaseChild(m_sName);
    }
}

void ChildAccess::addTypes(std::vector< css::uno::Type > * types) const
{
    assert(types != 0);
    types->push_back(cppu::UnoType< css::container::XChild >::get());
    types->push_back(cppu::UnoType< css::lang::XUnoTunnel >::get());
}

void ChildAccess::addSupportedServiceNames( std::vector< OUString > * services)
{
    assert(services != 0);
    services->push_back(
        getParentNode()->kind() == Node::KIND_GROUP
        ? OUString("com.sun.star.configuration.GroupElement")
        : OUString("com.sun.star.configuration.SetElement"));
}

css::uno::Any ChildAccess::queryInterface(css::uno::Type const & aType)
    throw (css::uno::RuntimeException)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*m_pLock);
    checkLocalizedPropertyAccess();
    css::uno::Any res(Access::queryInterface(aType));
    return res.hasValue()
        ? res
        : cppu::queryInterface(
            aType, static_cast< css::container::XChild * >(this),
            static_cast< css::lang::XUnoTunnel * >(this));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
