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

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <map>
#include <set>
#include <vector>

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/reflection/XCompoundTypeDescription.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp>
#include <com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceTypeDescription2.hpp>
#include <com/sun/star/reflection/XStructTypeDescription.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <salhelper/simplereferenceobject.hxx>

using cppu::PropertySetMixinImpl;

namespace {

struct PropertyData {
    explicit PropertyData(
        css::beans::Property const & theProperty, bool thePresent):
        property(theProperty), present(thePresent) {}

    css::beans::Property property;
    bool present;
};

struct Data: public salhelper::SimpleReferenceObject {
    typedef std::map< OUString, PropertyData > PropertyMap;

    PropertyMap properties;

    PropertyMap::const_iterator get(
        css::uno::Reference< css::uno::XInterface > const & object,
        OUString const & name) const;

protected:
    void initProperties(
        css::uno::Reference< css::reflection::XTypeDescription > const & type,
        css::uno::Sequence< OUString > const & absentOptional,
        std::vector< OUString > * handleNames)
    {
        std::set<OUString> seen;
        initProperties(type, absentOptional, handleNames, &seen);
    }

private:
    void initProperties(
        css::uno::Reference< css::reflection::XTypeDescription > const & type,
        css::uno::Sequence< OUString > const & absentOptional,
        std::vector< OUString > * handleNames, std::set<OUString> * seen);

    static css::uno::Reference< css::reflection::XTypeDescription >
    resolveTypedefs(
        css::uno::Reference< css::reflection::XTypeDescription > const & type);
};

Data::PropertyMap::const_iterator Data::get(
    css::uno::Reference< css::uno::XInterface > const & object,
    OUString const & name) const
{
    PropertyMap::const_iterator i(properties.find(name));
    if (i == properties.end() || !i->second.present) {
        throw css::beans::UnknownPropertyException(name, object);
    }
    return i;
}

void Data::initProperties(
    css::uno::Reference< css::reflection::XTypeDescription > const & type,
    css::uno::Sequence< OUString > const & absentOptional,
    std::vector< OUString > * handleNames, std::set<OUString> * seen)
{
    css::uno::Reference< css::reflection::XInterfaceTypeDescription2 > ifc(
        resolveTypedefs(type), css::uno::UNO_QUERY_THROW);
    if (seen->insert(ifc->getName()).second) {
        css::uno::Sequence<
        css::uno::Reference< css::reflection::XTypeDescription > > bases(
            ifc->getBaseTypes());
        for (sal_Int32 i = 0; i < bases.getLength(); ++i) {
            initProperties(bases[i], absentOptional, handleNames, seen);
        }
        css::uno::Sequence<
        css::uno::Reference<
        css::reflection::XInterfaceMemberTypeDescription > > members(
            ifc->getMembers());
        OUString const * absentBegin = absentOptional.getConstArray();
        OUString const * absentEnd =
            absentBegin + absentOptional.getLength();
        for (sal_Int32 i = 0; i < members.getLength(); ++i) {
            if (members[i]->getTypeClass()
                == css::uno::TypeClass_INTERFACE_ATTRIBUTE)
            {
                css::uno::Reference<
                css::reflection::XInterfaceAttributeTypeDescription2 > attr(
                    members[i], css::uno::UNO_QUERY_THROW);
                sal_Int16 attrAttribs = 0;
                if (attr->isBound()) {
                    attrAttribs |= css::beans::PropertyAttribute::BOUND;
                }
                bool bSetUnknown = false;
                if (attr->isReadOnly()) {
                    attrAttribs |= css::beans::PropertyAttribute::READONLY;
                    bSetUnknown = true;
                }
                css::uno::Sequence<
                css::uno::Reference<
                css::reflection::XCompoundTypeDescription > > excs(
                    attr->getGetExceptions());
                bool bGetUnknown = false;
                //XXX  Special interpretation of getter/setter exceptions only
                // works if the specified exceptions are of the exact type, not
                // of a supertype:
                for (sal_Int32 j = 0; j < excs.getLength(); ++j) {
                    if ( excs[j]->getName() == "com.sun.star.beans.UnknownPropertyException" )
                    {
                        bGetUnknown = true;
                        break;
                    }
                }
                excs = attr->getSetExceptions();
                for (sal_Int32 j = 0; j < excs.getLength(); ++j) {
                    if ( excs[j]->getName() == "com.sun.star.beans.UnknownPropertyException" )
                    {
                        bSetUnknown = true;
                    } else if ( excs[j]->getName() == "com.sun.star.beans.PropertyVetoException" )
                    {
                        attrAttribs
                            |= css::beans::PropertyAttribute::CONSTRAINED;
                    }
                }
                if (bGetUnknown && bSetUnknown) {
                    attrAttribs |= css::beans::PropertyAttribute::OPTIONAL;
                }
                css::uno::Reference< css::reflection::XTypeDescription > t(
                    attr->getType());
                for (;;)
                {
                    t = resolveTypedefs(t);
                    sal_Int16 n;
                    if (t->getName().startsWith(
                            "com.sun.star.beans.Ambiguous<"))
                    {
                        n = css::beans::PropertyAttribute::MAYBEAMBIGUOUS;
                    } else if (t->getName().startsWith(
                                   "com.sun.star.beans.Defaulted<"))
                    {
                        n = css::beans::PropertyAttribute::MAYBEDEFAULT;
                    } else if (t->getName().startsWith(
                                   "com.sun.star.beans.Optional<"))
                    {
                        n = css::beans::PropertyAttribute::MAYBEVOID;
                    } else {
                        break;
                    }
                    if ((attrAttribs & n) != 0) {
                        break;
                    }
                    attrAttribs |= n;
                    css::uno::Sequence<
                    css::uno::Reference< css::reflection::XTypeDescription > >
                        args(
                            css::uno::Reference<
                            css::reflection::XStructTypeDescription >(
                                t, css::uno::UNO_QUERY_THROW)->
                            getTypeArguments());
                    if (args.getLength() != 1) {
                        throw css::uno::RuntimeException(
                            "inconsistent UNO type registry");
                    }
                    t = args[0];
                }
                std::vector< OUString >::size_type handles
                    = handleNames->size();
                if (handles > SAL_MAX_INT32) {
                    throw css::uno::RuntimeException(
                        "interface type has too many attributes");
                }
                OUString name(members[i]->getMemberName());
                if (!properties.emplace(
                            name,
                            PropertyData(
                                css::beans::Property(
                                    name, static_cast< sal_Int32 >(handles),
                                    css::uno::Type(
                                        t->getTypeClass(), t->getName()),
                                    attrAttribs),
                                (std::find(absentBegin, absentEnd, name)
                                 == absentEnd))).
                    second)
                {
                    throw css::uno::RuntimeException(
                        "inconsistent UNO type registry");
                }
                handleNames->push_back(name);
            }
        }
    }
}

css::uno::Reference< css::reflection::XTypeDescription > Data::resolveTypedefs(
    css::uno::Reference< css::reflection::XTypeDescription > const & type)
{
    css::uno::Reference< css::reflection::XTypeDescription > t(type);
    while (t->getTypeClass() == css::uno::TypeClass_TYPEDEF) {
        t = css::uno::Reference< css::reflection::XIndirectTypeDescription >(
            t, css::uno::UNO_QUERY_THROW)->getReferencedType();
    }
    return t;
}

class Info: public cppu::WeakImplHelper< css::beans::XPropertySetInfo > {
public:
    explicit Info(Data * data): m_data(data) {}

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL getProperties() override;

    virtual css::beans::Property SAL_CALL getPropertyByName(
        OUString const & name) override;

    virtual sal_Bool SAL_CALL hasPropertyByName(OUString const & name) override;

private:
    rtl::Reference< Data > m_data;
};

css::uno::Sequence< css::beans::Property > Info::getProperties()
{
    assert(m_data->properties.size() <= SAL_MAX_INT32);
    css::uno::Sequence< css::beans::Property > s(
        static_cast< sal_Int32 >(m_data->properties.size()));
    sal_Int32 n = 0;
    for (const auto& rEntry : m_data->properties)
    {
        if (rEntry.second.present) {
            s[n++] = rEntry.second.property;
        }
    }
    s.realloc(n);
    return s;
}

css::beans::Property Info::getPropertyByName(OUString const & name)
{
    return m_data->get(static_cast< cppu::OWeakObject * >(this), name)->
        second.property;
}

sal_Bool Info::hasPropertyByName(OUString const & name)
{
    Data::PropertyMap::iterator i(m_data->properties.find(name));
    return i != m_data->properties.end() && i->second.present;
}

typedef
std::multiset< css::uno::Reference< css::beans::XPropertyChangeListener > >
BoundListenerBag;

}

class PropertySetMixinImpl::BoundListeners::Impl {
public:
    BoundListenerBag specificListeners;
    BoundListenerBag unspecificListeners;
    css::beans::PropertyChangeEvent event;
};

PropertySetMixinImpl::BoundListeners::BoundListeners(): m_impl(new Impl) {}

PropertySetMixinImpl::BoundListeners::~BoundListeners() {
    delete m_impl;
}

void PropertySetMixinImpl::BoundListeners::notify() const {
    for (const auto& rxListener : m_impl->specificListeners)
    {
        try {
            rxListener->propertyChange(m_impl->event);
        } catch (css::lang::DisposedException &) {}
    }
    for (const auto& rxListener : m_impl->unspecificListeners)
    {
        try {
            rxListener->propertyChange(m_impl->event);
        } catch (css::lang::DisposedException &) {}
    }
}

class PropertySetMixinImpl::Impl: public Data {
public:
    Impl(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        Implements theImplements,
        css::uno::Sequence< OUString > const & absentOptional,
        css::uno::Type const & type);

    OUString const & translateHandle(
        css::uno::Reference< css::uno::XInterface > const & object,
        sal_Int32 handle) const;

    void setProperty(
        css::uno::Reference< css::uno::XInterface > const & object,
        OUString const & name, css::uno::Any const & value,
        bool isAmbiguous, bool isDefaulted, sal_Int16 illegalArgumentPosition)
        const;

    css::uno::Any getProperty(
        css::uno::Reference< css::uno::XInterface > const & object,
        OUString const & name, css::beans::PropertyState * state) const;

    PropertySetMixinImpl::Implements implements;
    css::uno::Sequence< OUString > handleMap;

    typedef std::map< OUString, BoundListenerBag > BoundListenerMap;

    typedef
    std::multiset< css::uno::Reference< css::beans::XVetoableChangeListener > >
    VetoListenerBag;

    typedef std::map< OUString, VetoListenerBag > VetoListenerMap;

    mutable osl::Mutex mutex;
    BoundListenerMap boundListeners;
    VetoListenerMap vetoListeners;
    bool disposed;

private:
    css::uno::Reference< css::reflection::XIdlClass > getReflection(
        OUString const & typeName) const;

    static css::uno::Any wrapValue(
        css::uno::Reference< css::uno::XInterface > const & object,
        css::uno::Any const & value,
        css::uno::Reference< css::reflection::XIdlClass > const & type,
        bool wrapAmbiguous, bool isAmbiguous, bool wrapDefaulted,
        bool isDefaulted, bool wrapOptional);

    css::uno::Reference< css::uno::XComponentContext > const & m_context;
    css::uno::Type m_type;
    css::uno::Reference< css::reflection::XIdlClass > m_idlClass;
};

PropertySetMixinImpl::Impl::Impl(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    Implements theImplements,
    css::uno::Sequence< OUString > const & absentOptional,
    css::uno::Type const & type):
    implements(theImplements), disposed(false), m_context(context),
    m_type(type)
{
    assert(context.is());
    assert(
        (implements
         & ~(IMPLEMENTS_PROPERTY_SET | IMPLEMENTS_FAST_PROPERTY_SET
             | IMPLEMENTS_PROPERTY_ACCESS))
        == 0);
    m_idlClass = getReflection(m_type.getTypeName());
    css::uno::Reference< css::reflection::XTypeDescription > ifc;
    try {
        ifc.set(
            css::uno::Reference< css::container::XHierarchicalNameAccess >(
                m_context->getValueByName(
                    "/singletons/com.sun.star.reflection."
                    "theTypeDescriptionManager"),
                css::uno::UNO_QUERY_THROW)->getByHierarchicalName(
                    m_type.getTypeName()),
            css::uno::UNO_QUERY_THROW);
    } catch (css::container::NoSuchElementException & e) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "unexpected com.sun.star.container.NoSuchElementException: "
             + e.Message,
             nullptr, anyEx );
    }
    std::vector< OUString > handleNames;
    initProperties(ifc, absentOptional, &handleNames);
    std::vector< OUString >::size_type size = handleNames.size();
    assert(size <= SAL_MAX_INT32);
    handleMap.realloc(static_cast< sal_Int32 >(size));
    std::copy(handleNames.begin(), handleNames.end(), handleMap.getArray());
}

OUString const & PropertySetMixinImpl::Impl::translateHandle(
    css::uno::Reference< css::uno::XInterface > const & object,
    sal_Int32 handle) const
{
    if (handle < 0 || handle >= handleMap.getLength()) {
        throw css::beans::UnknownPropertyException(
            "bad handle " + OUString::number(handle), object);
    }
    return handleMap[handle];
}

void PropertySetMixinImpl::Impl::setProperty(
    css::uno::Reference< css::uno::XInterface > const & object,
    OUString const & name, css::uno::Any const & value, bool isAmbiguous,
    bool isDefaulted, sal_Int16 illegalArgumentPosition) const
{
    PropertyMap::const_iterator i(properties.find(name));
    if (i == properties.end()) {
        throw css::beans::UnknownPropertyException(name, object);
    }
    if ((isAmbiguous
         && ((i->second.property.Attributes
              & css::beans::PropertyAttribute::MAYBEAMBIGUOUS)
             == 0))
        || (isDefaulted
            && ((i->second.property.Attributes
                 & css::beans::PropertyAttribute::MAYBEDEFAULT)
                == 0)))
    {
        throw css::lang::IllegalArgumentException(
            ("flagging as ambiguous/defaulted non-ambiguous/defaulted property "
             + name),
            object, illegalArgumentPosition);
    }
    css::uno::Reference< css::reflection::XIdlField2 > f(
        m_idlClass->getField(name), css::uno::UNO_QUERY_THROW);
    css::uno::Any o(object->queryInterface(m_type));
    css::uno::Any v(
        wrapValue(
            object, value,
            (css::uno::Reference< css::reflection::XIdlField2 >(
                m_idlClass->getField(name), css::uno::UNO_QUERY_THROW)->
             getType()),
            ((i->second.property.Attributes
              & css::beans::PropertyAttribute::MAYBEAMBIGUOUS)
             != 0),
            isAmbiguous,
            ((i->second.property.Attributes
              & css::beans::PropertyAttribute::MAYBEDEFAULT)
             != 0),
            isDefaulted,
            ((i->second.property.Attributes
              & css::beans::PropertyAttribute::MAYBEVOID)
             != 0)));
    try {
        f->set(o, v);
    } catch (css::lang::IllegalArgumentException & e) {
        if (e.ArgumentPosition == 1) {
            throw css::lang::IllegalArgumentException(
                e.Message, object, illegalArgumentPosition);
        } else {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                "unexpected com.sun.star.lang.IllegalArgumentException: "
                 + e.Message,
                object, anyEx );
        }
    } catch (css::lang::IllegalAccessException &) {
        //TODO  Clarify whether PropertyVetoException is the correct exception
        // to throw when trying to set a read-only property:
        throw css::beans::PropertyVetoException(
            "cannot set read-only property " + name, object);
    } catch (css::lang::WrappedTargetRuntimeException & e) {
        //FIXME  A WrappedTargetRuntimeException from XIdlField2.get is not
        // guaranteed to originate directly within XIdlField2.get (and thus have
        // the expected semantics); it might also be passed through from lower
        // layers.
        if (e.TargetException.isExtractableTo(
                cppu::UnoType<css::beans::UnknownPropertyException>::get())
            && ((i->second.property.Attributes
                 & css::beans::PropertyAttribute::OPTIONAL)
                != 0))
        {
            throw css::beans::UnknownPropertyException(name, object);
        } else if (e.TargetException.isExtractableTo(
                       cppu::UnoType<css::beans::PropertyVetoException>::get())
                   && ((i->second.property.Attributes
                        & css::beans::PropertyAttribute::CONSTRAINED)
                       != 0))
        {
            css::beans::PropertyVetoException exc;
            e.TargetException >>= exc;
            if (exc.Message.isEmpty() )
                throw css::beans::PropertyVetoException("Invalid " + name, object);
            else
                throw exc;
        } else {
            throw css::lang::WrappedTargetException(
                e.Message, object, e.TargetException);
        }
    }
}

css::uno::Any PropertySetMixinImpl::Impl::getProperty(
    css::uno::Reference< css::uno::XInterface > const & object,
    OUString const & name, css::beans::PropertyState * state) const
{
    PropertyMap::const_iterator i(properties.find(name));
    if (i == properties.end()) {
        throw css::beans::UnknownPropertyException(name, object);
    }
    css::uno::Reference< css::reflection::XIdlField2 > field(
        m_idlClass->getField(name), css::uno::UNO_QUERY_THROW);
    css::uno::Any value;
    try {
        value = field->get(object->queryInterface(m_type));
    } catch (css::lang::IllegalArgumentException & e) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "unexpected com.sun.star.lang.IllegalArgumentException: "
             + e.Message,
             object, anyEx );
    } catch (css::lang::WrappedTargetRuntimeException & e) {
        //FIXME  A WrappedTargetRuntimeException from XIdlField2.get is not
        // guaranteed to originate directly within XIdlField2.get (and thus have
        // the expected semantics); it might also be passed through from lower
        // layers.
        if (e.TargetException.isExtractableTo(
                cppu::UnoType<css::beans::UnknownPropertyException>::get())
            && ((i->second.property.Attributes
                 & css::beans::PropertyAttribute::OPTIONAL)
                != 0))
        {
            throw css::beans::UnknownPropertyException(name, object);
        } else {
            throw css::lang::WrappedTargetException(
                e.Message, object, e.TargetException);
        }
    }
    bool undoAmbiguous
        = ((i->second.property.Attributes
            & css::beans::PropertyAttribute::MAYBEAMBIGUOUS)
           != 0);
    bool undoDefaulted
        = ((i->second.property.Attributes
            & css::beans::PropertyAttribute::MAYBEDEFAULT)
           != 0);
    bool undoOptional
        = ((i->second.property.Attributes
            & css::beans::PropertyAttribute::MAYBEVOID)
           != 0);
    bool isAmbiguous = false;
    bool isDefaulted = false;
    while (undoAmbiguous || undoDefaulted || undoOptional) {
        if (undoAmbiguous
            && value.getValueTypeName().startsWith(
                "com.sun.star.beans.Ambiguous<"))
        {
            css::uno::Reference< css::reflection::XIdlClass > ambiguous(
                getReflection(value.getValueTypeName()));
            try {
                if (!(css::uno::Reference< css::reflection::XIdlField2 >(
                          ambiguous->getField("IsAmbiguous"),
                          css::uno::UNO_QUERY_THROW)->get(value)
                      >>= isAmbiguous))
                {
                    throw css::uno::RuntimeException(
                        ("unexpected type of com.sun.star.beans.Ambiguous"
                         " IsAmbiguous member"),
                        object);
                }
                value = css::uno::Reference< css::reflection::XIdlField2 >(
                    ambiguous->getField("Value"), css::uno::UNO_QUERY_THROW)->
                    get(value);
            } catch (css::lang::IllegalArgumentException & e) {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw css::lang::WrappedTargetRuntimeException(
                    "unexpected com.sun.star.lang.IllegalArgumentException: "
                     + e.Message,
                     object, anyEx );
            }
            undoAmbiguous = false;
        } else if (undoDefaulted
                   && value.getValueTypeName().startsWith(
                       "com.sun.star.beans.Defaulted<"))
        {
            css::uno::Reference< css::reflection::XIdlClass > defaulted(
                getReflection(value.getValueTypeName()));
            try {

                if (!(css::uno::Reference< css::reflection::XIdlField2 >(
                          defaulted->getField("IsDefaulted"),
                          css::uno::UNO_QUERY_THROW)->get(value)
                      >>= isDefaulted))
                {
                    throw css::uno::RuntimeException(
                        ("unexpected type of com.sun.star.beans.Defaulted"
                         " IsDefaulted member"),
                        object);
                }
                value = css::uno::Reference< css::reflection::XIdlField2 >(
                    defaulted->getField("Value"), css::uno::UNO_QUERY_THROW)->
                    get(value);
            } catch (css::lang::IllegalArgumentException & e) {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw css::lang::WrappedTargetRuntimeException(
                    "unexpected com.sun.star.lang.IllegalArgumentException: "
                     + e.Message,
                    object, anyEx );
            }
            undoDefaulted = false;
        } else if (undoOptional
                   && value.getValueTypeName().startsWith(
                       "com.sun.star.beans.Optional<"))
        {
            css::uno::Reference< css::reflection::XIdlClass > optional(
                getReflection(value.getValueTypeName()));
            try {
                bool present = false;
                if (!(css::uno::Reference< css::reflection::XIdlField2 >(
                          optional->getField("IsPresent"),
                          css::uno::UNO_QUERY_THROW)->get(value)
                      >>= present))
                {
                    throw css::uno::RuntimeException(
                        ("unexpected type of com.sun.star.beans.Optional"
                         " IsPresent member"),
                        object);
                }
                if (!present) {
                    value.clear();
                    break;
                }
                value = css::uno::Reference< css::reflection::XIdlField2 >(
                    optional->getField("Value"), css::uno::UNO_QUERY_THROW)->
                    get(value);
            } catch (css::lang::IllegalArgumentException & e) {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw css::lang::WrappedTargetRuntimeException(
                    "unexpected com.sun.star.lang.IllegalArgumentException: "
                     + e.Message,
                     object, anyEx );
            }
            undoOptional = false;
        } else {
            throw css::uno::RuntimeException(
                "unexpected type of attribute " + name, object);
        }
    }
    if (state != nullptr) {
        //XXX  If isAmbiguous && isDefaulted, arbitrarily choose AMBIGUOUS_VALUE
        // over DEFAULT_VALUE:
        *state = isAmbiguous
            ? css::beans::PropertyState_AMBIGUOUS_VALUE
            : isDefaulted
            ? css::beans::PropertyState_DEFAULT_VALUE
            : css::beans::PropertyState_DIRECT_VALUE;
    }
    return value;
}

css::uno::Reference< css::reflection::XIdlClass >
PropertySetMixinImpl::Impl::getReflection(OUString const & typeName) const
{
    return css::uno::Reference< css::reflection::XIdlClass >(
        css::reflection::theCoreReflection::get(m_context)->forName(typeName),
        css::uno::UNO_SET_THROW);
}

css::uno::Any PropertySetMixinImpl::Impl::wrapValue(
    css::uno::Reference< css::uno::XInterface > const & object,
    css::uno::Any const & value,
    css::uno::Reference< css::reflection::XIdlClass > const & type,
    bool wrapAmbiguous, bool isAmbiguous, bool wrapDefaulted, bool isDefaulted,
    bool wrapOptional)
{
    assert(wrapAmbiguous || !isAmbiguous);
    assert(wrapDefaulted || !isDefaulted);
    if (wrapAmbiguous
        && type->getName().startsWith("com.sun.star.beans.Ambiguous<"))
    {
        css::uno::Any strct;
        type->createObject(strct);
        try {
            css::uno::Reference< css::reflection::XIdlField2 > field(
                type->getField("Value"), css::uno::UNO_QUERY_THROW);
            field->set(
                strct,
                wrapValue(
                    object, value, field->getType(), false, false,
                    wrapDefaulted, isDefaulted, wrapOptional));
            css::uno::Reference< css::reflection::XIdlField2 >(
                type->getField("IsAmbiguous"), css::uno::UNO_QUERY_THROW)->set(
                    strct, css::uno::Any(isAmbiguous));
        } catch (css::lang::IllegalArgumentException & e) {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                "unexpected com.sun.star.lang.IllegalArgumentException: "
                 + e.Message,
                 object, anyEx );
        } catch (css::lang::IllegalAccessException & e) {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                "unexpected com.sun.star.lang.IllegalAccessException: "
                 + e.Message,
                 object, anyEx );
        }
        return strct;
    }
    if (wrapDefaulted
               && type->getName().startsWith("com.sun.star.beans.Defaulted<"))
    {
        css::uno::Any strct;
        type->createObject(strct);
        try {
            css::uno::Reference< css::reflection::XIdlField2 > field(
                type->getField("Value"), css::uno::UNO_QUERY_THROW);
            field->set(
                strct,
                wrapValue(
                    object, value, field->getType(), wrapAmbiguous, isAmbiguous,
                    false, false, wrapOptional));
            css::uno::Reference< css::reflection::XIdlField2 >(
                type->getField("IsDefaulted"), css::uno::UNO_QUERY_THROW)->set(
                    strct, css::uno::Any(isDefaulted));
        } catch (css::lang::IllegalArgumentException & e) {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                "unexpected com.sun.star.lang.IllegalArgumentException: "
                 + e.Message,
                 object, anyEx );
        } catch (css::lang::IllegalAccessException & e) {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                "unexpected com.sun.star.lang.IllegalAccessException: "
                 + e.Message,
                 object, anyEx );
        }
        return strct;
    }
    if (wrapOptional
               && type->getName().startsWith("com.sun.star.beans.Optional<"))
    {
        css::uno::Any strct;
        type->createObject(strct);
        bool present = value.hasValue();
        try {
            css::uno::Reference< css::reflection::XIdlField2 >(
                type->getField("IsPresent"), css::uno::UNO_QUERY_THROW)->set(
                    strct, css::uno::Any(present));
            if (present) {
                css::uno::Reference< css::reflection::XIdlField2 > field(
                    type->getField("Value"), css::uno::UNO_QUERY_THROW);
                field->set(
                    strct,
                    wrapValue(
                        object, value, field->getType(), wrapAmbiguous,
                        isAmbiguous, wrapDefaulted, isDefaulted, false));
            }
        } catch (css::lang::IllegalArgumentException & e) {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                "unexpected com.sun.star.lang.IllegalArgumentException: "
                 + e.Message,
                 object, anyEx );
        } catch (css::lang::IllegalAccessException & e) {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                "unexpected com.sun.star.lang.IllegalAccessException: "
                 + e.Message,
                object, anyEx );
        }
        return strct;
    }
    if (wrapAmbiguous || wrapDefaulted || wrapOptional) {
        throw css::uno::RuntimeException(
            "unexpected type of attribute", object);
    }
    return value;
}

PropertySetMixinImpl::PropertySetMixinImpl(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    Implements implements,
    css::uno::Sequence< OUString > const & absentOptional,
    css::uno::Type const & type)
{
    m_impl = new Impl(context, implements, absentOptional, type);
    m_impl->acquire();
}

PropertySetMixinImpl::~PropertySetMixinImpl() {
    m_impl->release();
}

void PropertySetMixinImpl::checkUnknown(OUString const & propertyName) {
    if (!propertyName.isEmpty()) {
        m_impl->get(
            static_cast< css::beans::XPropertySet * >(this), propertyName);
    }
}

void PropertySetMixinImpl::prepareSet(
    OUString const & propertyName, css::uno::Any const & oldValue,
    css::uno::Any const & newValue, BoundListeners * boundListeners)
{
    Impl::PropertyMap::const_iterator it(m_impl->properties.find(propertyName));
    assert(it != m_impl->properties.end());
    Impl::VetoListenerBag specificVeto;
    Impl::VetoListenerBag unspecificVeto;
    {
        osl::MutexGuard g(m_impl->mutex);
        if (m_impl->disposed) {
            throw css::lang::DisposedException(
                "disposed", static_cast< css::beans::XPropertySet * >(this));
        }
        if ((it->second.property.Attributes
             & css::beans::PropertyAttribute::CONSTRAINED)
            != 0)
        {
            Impl::VetoListenerMap::const_iterator i(
                m_impl->vetoListeners.find(propertyName));
            if (i != m_impl->vetoListeners.end()) {
                specificVeto = i->second;
            }
            i = m_impl->vetoListeners.find("");
            if (i != m_impl->vetoListeners.end()) {
                unspecificVeto = i->second;
            }
        }
        if ((it->second.property.Attributes
             & css::beans::PropertyAttribute::BOUND)
            != 0)
        {
            assert(boundListeners != nullptr);
            Impl::BoundListenerMap::const_iterator i(
                m_impl->boundListeners.find(propertyName));
            if (i != m_impl->boundListeners.end()) {
                boundListeners->m_impl->specificListeners = i->second;
            }
            i = m_impl->boundListeners.find("");
            if (i != m_impl->boundListeners.end()) {
                boundListeners->m_impl->unspecificListeners = i->second;
            }
        }
    }
    if ((it->second.property.Attributes
         & css::beans::PropertyAttribute::CONSTRAINED)
        != 0)
    {
        css::beans::PropertyChangeEvent event(
            static_cast< css::beans::XPropertySet * >(this), propertyName,
            false, it->second.property.Handle, oldValue, newValue);
        for (auto& rxVetoListener : specificVeto)
        {
            try {
                rxVetoListener->vetoableChange(event);
            } catch (css::lang::DisposedException &) {}
        }
        for (auto& rxVetoListener : unspecificVeto)
        {
            try {
                rxVetoListener->vetoableChange(event);
            } catch (css::lang::DisposedException &) {}
        }
    }
    if ((it->second.property.Attributes & css::beans::PropertyAttribute::BOUND)
        != 0)
    {
        assert(boundListeners != nullptr);
        boundListeners->m_impl->event = css::beans::PropertyChangeEvent(
            static_cast< css::beans::XPropertySet * >(this), propertyName,
            false, it->second.property.Handle, oldValue, newValue);
    }
}

void PropertySetMixinImpl::dispose() {
    Impl::BoundListenerMap boundListeners;
    Impl::VetoListenerMap vetoListeners;
    {
        osl::MutexGuard g(m_impl->mutex);
        boundListeners.swap(m_impl->boundListeners);
        vetoListeners.swap(m_impl->vetoListeners);
        m_impl->disposed = true;
    }
    css::lang::EventObject event(
        static_cast< css::beans::XPropertySet * >(this));
    for (auto& rEntry : boundListeners)
    {
        for (auto& rxBoundListener : rEntry.second)
        {
            rxBoundListener->disposing(event);
        }
    }
    for (auto& rEntry : vetoListeners)
    {
        for (auto& rxVetoListener : rEntry.second)
        {
            rxVetoListener->disposing(event);
        }
    }
}

css::uno::Any PropertySetMixinImpl::queryInterface(css::uno::Type const & type)
{
    if ((m_impl->implements & IMPLEMENTS_PROPERTY_SET) != 0
         && type == css::beans::XPropertySet::static_type())
    {
        css::uno::Reference< css::uno::XInterface > ifc(
            static_cast< css::beans::XPropertySet * >(this));
        return css::uno::Any(&ifc, type);
    }
    if ((m_impl->implements & IMPLEMENTS_FAST_PROPERTY_SET) != 0
               && type == css::beans::XFastPropertySet::static_type())
    {
        css::uno::Reference< css::uno::XInterface > ifc(
            static_cast< css::beans::XFastPropertySet * >(this));
        return css::uno::Any(&ifc, type);
    }
    if ((m_impl->implements & IMPLEMENTS_PROPERTY_ACCESS) != 0
               && type == css::beans::XPropertyAccess::static_type())
    {
        css::uno::Reference< css::uno::XInterface > ifc(
            static_cast< css::beans::XPropertyAccess * >(this));
        return css::uno::Any(&ifc, type);
    }
    return css::uno::Any();
}

css::uno::Reference< css::beans::XPropertySetInfo >
PropertySetMixinImpl::getPropertySetInfo()
{
    return new Info(m_impl);
}

void PropertySetMixinImpl::setPropertyValue(
    OUString const & propertyName, css::uno::Any const & value)
{
    m_impl->setProperty(
        static_cast< css::beans::XPropertySet * >(this), propertyName, value,
        false, false, 1);
}

css::uno::Any PropertySetMixinImpl::getPropertyValue(
    OUString const & propertyName)
{
    return m_impl->getProperty(
        static_cast< css::beans::XPropertySet * >(this), propertyName, nullptr);
}

void PropertySetMixinImpl::addPropertyChangeListener(
    OUString const & propertyName,
    css::uno::Reference< css::beans::XPropertyChangeListener > const & listener)
{
    css::uno::Reference< css::beans::XPropertyChangeListener >(
        listener, css::uno::UNO_SET_THROW); // reject NULL listener
    checkUnknown(propertyName);
    bool disposed;
    {
        osl::MutexGuard g(m_impl->mutex);
        disposed = m_impl->disposed;
        if (!disposed) {
            m_impl->boundListeners[propertyName].insert(listener);
        }
    }
    if (disposed) {
        listener->disposing(
            css::lang::EventObject(
                static_cast< css::beans::XPropertySet * >(this)));
    }
}

void PropertySetMixinImpl::removePropertyChangeListener(
    OUString const & propertyName,
    css::uno::Reference< css::beans::XPropertyChangeListener > const & listener)
{
    assert(listener.is());
    checkUnknown(propertyName);
    osl::MutexGuard g(m_impl->mutex);
    Impl::BoundListenerMap::iterator i(
        m_impl->boundListeners.find(propertyName));
    if (i != m_impl->boundListeners.end()) {
        BoundListenerBag::iterator j(i->second.find(listener));
        if (j != i->second.end()) {
            i->second.erase(j);
        }
    }
}

void PropertySetMixinImpl::addVetoableChangeListener(
    OUString const & propertyName,
    css::uno::Reference< css::beans::XVetoableChangeListener > const & listener)
{
    css::uno::Reference< css::beans::XVetoableChangeListener >(
        listener, css::uno::UNO_SET_THROW); // reject NULL listener
    checkUnknown(propertyName);
    bool disposed;
    {
        osl::MutexGuard g(m_impl->mutex);
        disposed = m_impl->disposed;
        if (!disposed) {
            m_impl->vetoListeners[propertyName].insert(listener);
        }
    }
    if (disposed) {
        listener->disposing(
            css::lang::EventObject(
                static_cast< css::beans::XPropertySet * >(this)));
    }
}

void PropertySetMixinImpl::removeVetoableChangeListener(
    OUString const & propertyName,
    css::uno::Reference< css::beans::XVetoableChangeListener > const & listener)
{
    assert(listener.is());
    checkUnknown(propertyName);
    osl::MutexGuard g(m_impl->mutex);
    Impl::VetoListenerMap::iterator i(m_impl->vetoListeners.find(propertyName));
    if (i != m_impl->vetoListeners.end()) {
        Impl::VetoListenerBag::iterator j(i->second.find(listener));
        if (j != i->second.end()) {
            i->second.erase(j);
        }
    }
}

void PropertySetMixinImpl::setFastPropertyValue(
    sal_Int32 handle, css::uno::Any const & value)
{
    m_impl->setProperty(
        static_cast< css::beans::XPropertySet * >(this),
        m_impl->translateHandle(
            static_cast< css::beans::XPropertySet * >(this), handle),
        value, false, false, 1);
}

css::uno::Any PropertySetMixinImpl::getFastPropertyValue(sal_Int32 handle)
{
    return m_impl->getProperty(
        static_cast< css::beans::XPropertySet * >(this),
        m_impl->translateHandle(
            static_cast< css::beans::XPropertySet * >(this), handle),
        nullptr);
}

css::uno::Sequence< css::beans::PropertyValue >
PropertySetMixinImpl::getPropertyValues()
{
    css::uno::Sequence< css::beans::PropertyValue > s(
        m_impl->handleMap.getLength());
    sal_Int32 n = 0;
    for (sal_Int32 i = 0; i < m_impl->handleMap.getLength(); ++i) {
        try {
            s[n].Value = m_impl->getProperty(
                static_cast< css::beans::XPropertySet * >(this),
                m_impl->handleMap[i], &s[n].State);
        } catch (css::beans::UnknownPropertyException &) {
            continue;
        } catch (css::lang::WrappedTargetException & e) {
            throw css::lang::WrappedTargetRuntimeException(
                e.Message, static_cast< css::beans::XPropertySet * >(this),
                e.TargetException);
        }
        s[n].Name = m_impl->handleMap[i];
        s[n].Handle = i;
        ++n;
    }
    s.realloc(n);
    return s;
}

void PropertySetMixinImpl::setPropertyValues(
    css::uno::Sequence< css::beans::PropertyValue > const & props)
{
    for (sal_Int32 i = 0; i < props.getLength(); ++i) {
        if (props[i].Handle != -1
            && (props[i].Name
                != m_impl->translateHandle(
                    static_cast< css::beans::XPropertySet * >(this),
                    props[i].Handle)))
        {
            throw css::beans::UnknownPropertyException(
                ("name " + props[i].Name + " does not match handle "
                 + OUString::number(props[i].Handle)),
                static_cast< css::beans::XPropertySet * >(this));
        }
        m_impl->setProperty(
            static_cast< css::beans::XPropertySet * >(this), props[i].Name,
            props[i].Value,
            props[i].State == css::beans::PropertyState_AMBIGUOUS_VALUE,
            props[i].State == css::beans::PropertyState_DEFAULT_VALUE, 0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
